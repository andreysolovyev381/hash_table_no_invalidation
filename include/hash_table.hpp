//
// Created by Andrey Solovyev on 13.03.2024.
//

#pragma once

#include <bit>
#include <climits>

#include <cstddef>
#include <list>
#include <variant>

#include <functional>
#include <concepts>
#include <type_traits>
#include <memory_resource>
#include <stdexcept>
#include <utility>


namespace requirements {

	namespace hash {

		template<typename Type, typename MaybeHash, typename HashResult = std::size_t>
		concept IsHash = 
		std::is_same_v<typename std::hash<Type>, MaybeHash> ||
		(
			std::negation_v<std::is_same<HashResult, bool>> &&
			std::is_invocable_r_v<std::size_t, MaybeHash, std::add_const_t<std::decay_t<Type>>> &&
			std::is_copy_constructible_v<MaybeHash> &&
			std::is_move_constructible_v<MaybeHash>
		);

		template<typename Type, typename MaybeHash, typename HashResult = std::size_t>
		static inline constexpr bool is_hash_v{IsHash<Type, MaybeHash, HashResult> ? true : false};

		[[maybe_unused]] inline auto combine = [](std::size_t hash, std::size_t seed = 0) {
			static constexpr std::size_t magic_number {0x9e3779b9}; //boost hash_combine as a source
			seed ^= hash + magic_number + (seed << 6) + (seed >> 2);
			return seed;
		};

	}//!namespace hash

}//!namespace requirements

namespace containers {

	namespace pmr {
		    
		template <std::size_t Alignment>
		class AlignedMemoryResource final : public std::pmr::memory_resource {
		public:
			explicit AlignedMemoryResource(std::pmr::memory_resource* upstream)
				: upstream_resource(upstream) 
			{}

		protected:
			void* do_allocate(std::size_t bytes, std::size_t alignment) override {
				alignment = std::max(alignment, Alignment);
				void* p {upstream_resource->allocate(bytes, alignment)};
				if (!p) {
					throw std::bad_alloc();
				}
				return p;
			}

			void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override {
				upstream_resource->deallocate(p, bytes, alignment);
			}

			bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
				auto const* other_ptr {dynamic_cast<const AlignedMemoryResource*>(&other)};
				return other_ptr && upstream_resource->is_equal(*other_ptr->upstream_resource);
			}
		private:
			std::pmr::memory_resource* upstream_resource;
		};

		// static inline std::pmr::synchronized_pool_resource resource(std::pmr::get_default_resource());
		static inline std::pmr::unsynchronized_pool_resource resource(std::pmr::get_default_resource());
		// using allocator_type = std::pmr::polymorphic_allocator<std::byte>;
		// usage example
		// std::pmr::list<int> l(pmr::allocator_type{&pmr::resource});

		template<typename T = std::byte>
		using allocator_type = std::pmr::polymorphic_allocator<T>;
		template<typename T>
		inline AlignedMemoryResource<std::bit_ceil(sizeof(T))> aligned_resource{&resource};
		// std::pmr::list<int> l(pmr::allocator_type<T>{&pmr::aligned_resource<T>});


	}//!namespace details::pmr

	namespace hash_table {

		namespace details {

			namespace const_values {

				constexpr inline std::size_t initial_capacity {1<<5};
				constexpr inline std::size_t max_type_sizeof {1<<10};
				constexpr inline double maxLoadFactor {0.5};
				constexpr inline double minLoadFactor {0.125};
				constexpr inline int maxEmplaceAttempts {5};

			}//!namespace details::const_values

			class CapacityPolicy {
			public:

				explicit CapacityPolicy(std::size_t requested, std::size_t typeSize)
					: capacity_ {computeInitial(requested, typeSize)}
					, mask_ {capacity_ - 1}
				{}

				std::size_t capacity() const { return capacity_; }
				std::size_t mask() const { return mask_; }

				void setCapacity(std::size_t newCapacity) noexcept {
					if (not isPowerOfTwo(newCapacity)) {
						newCapacity = std::bit_ceil(newCapacity);
					}
					capacity_ = newCapacity;
					mask_ = capacity_ - 1;
				}

				static constexpr bool isPowerOfTwo(std::size_t value) {
					return value != 0 && (value & (value - 1)) == 0;
				}
			private:
				std::size_t capacity_;
				std::size_t mask_;

			private:
				static constexpr std::size_t computeInitial(std::size_t requested, std::size_t typeSize) {
					if (typeSize > const_values::max_type_sizeof) {
						return 1ull;
					}
					if (requested == 0) {
						return const_values::initial_capacity;
					}
					return std::bit_ceil(requested);
				}

			};

			namespace requirements {

				enum class Type : std::uint8_t {
					Set = 0,
					Map = 1,
				};

				template<Type t>
				concept IsMapConcept = requires { requires t == Type::Map; };				
				template<Type t>
				static constexpr inline bool is_map_v {t == Type::Map};

				template<Type t>
				concept IsSetConcept = requires { requires t == Type::Set; };
				template<Type t>
				static constexpr inline bool is_set_v {t == Type::Set};

			}//!namespace details::requirements

			template<typename T, typename Hasher, typename KeyEqual, requirements::Type t>
			class HashTable {
			private:
				static constexpr requirements::Type type {t};

				static constexpr auto getKeyType(){
					if constexpr (requirements::is_map_v<type>) {
						return std::type_identity<typename T::first_type>{};
					}
					else if constexpr (requirements::is_set_v<type>) {
						return std::type_identity<T>{};
					}
					else {
						throw std::invalid_argument("can't recognized mapped type extracting Key Type");
					}
				}
				using KeyType = typename decltype(getKeyType())::type;

				static constexpr auto getMappedType(){
					if constexpr (requirements::is_map_v<type>) {
						return std::type_identity<typename T::second_type>{};
					}
					else if constexpr (requirements::is_set_v<type>) {
						return std::type_identity<T>{};
					}
					else {
						throw std::invalid_argument("can't recognized mapped type extracting Value Type");
					}
				}
				using MappedType = typename decltype(getMappedType())::type;

				struct KeyExtractor {
					KeyType const& operator()(std::pair<KeyType const, MappedType> const& value) const
					requires requirements::IsMapConcept<type>
					{
						return value.first;
					}

					MappedType const& operator()(MappedType const& value) const
					requires requirements::IsSetConcept<type>
					{
						return value;
					}
				};

				using Data = typename std::pmr::list<T>;

				static constexpr auto getIterType(){
					if constexpr (requirements::is_map_v<type>) {
						return std::type_identity<typename Data::iterator>{};
					}
					else if constexpr (requirements::is_set_v<type>) {
						return std::type_identity<typename Data::const_iterator>{};
					}
					else {
						throw std::invalid_argument("can't recognized mapped type extracting Iterator Type");
					}
				}

			public:
				using key_type = KeyType;
			protected:
				using mapped_type = MappedType;
			public:
				using value_type = T;
				using hasher = Hasher;
				using key_equal = KeyEqual;
			protected:
				using allocator_type = pmr::allocator_type<T>;
			public:
				using iterator = typename decltype(getIterType())::type;
				using const_iterator = typename Data::const_iterator;
				using reverse_iterator = typename Data::reverse_iterator;
				using const_reverse_iterator = typename Data::const_reverse_iterator;

			private:

				template <std::input_iterator IterType>
				struct Element final {
					enum class State : std::uint8_t { Free = 0, Occupied, Deleted };

					State state_ {State::Free};
					IterType iter_ {};

					Element () = default;
					Element (IterType data_) : state_ {State::Occupied}, iter_ {data_} {}

					bool is_free() const noexcept { return state_ == State::Free; }
					bool has_value() const noexcept { return state_ == State::Occupied; }
					bool is_deleted() const noexcept { return state_ == State::Deleted; }

					IterType& value() noexcept { return iter_; }
					IterType const& value() const noexcept { return iter_; }

					void emplace(IterType data_) noexcept { iter_ = data_; state_ = State::Occupied; }
					void reset() noexcept { state_ = State::Deleted; }
				};

				struct Access final {
					using AccessHelper = typename std::vector<Element<iterator>>;
					using AccessIter = typename AccessHelper::iterator;
					using AccessCIter = typename AccessHelper::const_iterator;

					std::pmr::memory_resource* memResourcePtr;
					Data &data;
					Data deadNodes;
					AccessHelper accessHelper;
					CapacityPolicy capacityPolicy;
					std::size_t sz;
					std::size_t deleted_count;

					Hasher hasher;
					KeyEqual equal;
					KeyExtractor keyExtractor;

					explicit Access(Data &data, std::pmr::memory_resource* res)
						: memResourcePtr(res)
						, data(data)
						, deadNodes(pmr::allocator_type<T>{res})
						, capacityPolicy {0, sizeof(T)}
						, sz {0}
						, deleted_count{0}
					{
						accessHelper.resize(capacityPolicy.capacity());
					}

					explicit Access(Data &data, std::size_t initialCapacity, std::pmr::memory_resource* res)
						: memResourcePtr(res)
						, data(data)
						, deadNodes(pmr::allocator_type<T>{res})
						, capacityPolicy {initialCapacity, sizeof(T)}
						, sz {0}
						, deleted_count{0}
					{
						accessHelper.resize(capacityPolicy.capacity());
					}

					AccessIter getElemIter(key_type const &key) {
						std::size_t const 
							cap{capacityPolicy.capacity()},
							mask {capacityPolicy.mask()};
					    std::size_t h {hasher(key) & mask};
					    std::size_t const step {h | 1};
					
					    auto check = [this, &key](std::size_t idx) __attribute__((always_inline)) -> bool {
					        return accessHelper[idx].is_free() ||
					               (accessHelper[idx].has_value() && equal(keyExtractor(*(accessHelper[idx].value())), key));
					    };
					
						for (std::size_t i {0}; i != cap; ++i) {
							if (check(h)) {
					            return accessHelper.begin() + h;
					        }
					        h = (h + step) & mask;
						}
					    return accessHelper.end();
					}

					AccessCIter getElemIter(key_type const &key) const {
						std::size_t const 
							cap{capacityPolicy.capacity()},
							mask {capacityPolicy.mask()};
					    std::size_t h {hasher(key) & mask};
					    std::size_t const step {h | 1};
					
					    auto check = [this, &key](std::size_t idx) __attribute__((always_inline)) -> bool {
					        return accessHelper[idx].is_free() ||
					               (accessHelper[idx].has_value() && equal(keyExtractor(*(accessHelper[idx].value())), key));
					    };
					
						for (std::size_t i {0}; i != cap; ++i) {
							if (check(h)) {
								return accessHelper.cbegin() + h;
							}
					        h = (h + step) & mask;
						}
					    return accessHelper.cend();
					}

					iterator find(key_type const &key) {
						AccessIter elemIter {getElemIter(key)};
						return contains(elemIter) ? elemIter->value() : data.end();
					}

					const_iterator find(key_type const &key) const {
						AccessCIter elemIter {getElemIter(key)};
						return contains(elemIter) ? elemIter->value() : data.end();
					}

					std::pair<iterator, bool> insert(T mappedValue){

						auto emplaceable = [this](AccessIter iter) -> bool {
							return iter != accessHelper.end() && iter->is_free();
						};

						auto place_to_data = [this](T mappedValue) -> iterator {
						    data.emplace_back(
						        std::make_obj_using_allocator<T>(
						            pmr::allocator_type<T>{memResourcePtr},
						            std::move(mappedValue)
						        )
						    );
						    ++sz;
						    return std::prev(data.end());
						};

						double const currLoadFactor {1.0 * (sz + deleted_count) / capacityPolicy.capacity()};
						if (currLoadFactor > const_values::maxLoadFactor) {
							rehashTo(capacityPolicy.capacity() << 1);
						}
						key_type const& key {keyExtractor(mappedValue)};
						AccessIter elemIter {getElemIter(key)};
						if (contains(elemIter)) {
							return {elemIter->value(), false};
						}
						else if (emplaceable(elemIter)) {
							elemIter->emplace(place_to_data(std::move(mappedValue)));
							return {elemIter->value(), true};
						}
						else {
							int attempts {const_values::maxEmplaceAttempts};
							while (attempts-- && !emplaceable(elemIter)){
								rehashTo(capacityPolicy.capacity() << 1);
								elemIter = getElemIter(key);
							}
							if (!emplaceable(elemIter)) {
								throw std::runtime_error("Unable to emplace after rehash, consider reducing const_values::maxLoadFactor");
							}
							elemIter->emplace(place_to_data(std::move(mappedValue)));
							return {elemIter->value(), true};
						}
					}
#if 0
					void erase(key_type const &key) {
						AccessIter elemIter {getElemIter(key)};
						if (!contains(elemIter)) {
							return;
						}

						std::size_t const mask 		{capacityPolicy.mask()};
						std::size_t idx_curr        {static_cast<std::size_t>(elemIter - accessHelper.begin())};
						std::size_t hash_curr       {hasher(key)};
						std::size_t const step      {(hash_curr & mask) | 1};
						std::size_t	idx_next        {(idx_curr + step) & mask};

						//we shouldn't perform more iterations, then capacity we have
						for (std::size_t count {0}; count != capacityPolicy.capacity(); ++count){
							//should remove, next elem is free, we are at the end of hash sequence
							if (accessHelper[idx_next].is_free()){
								break;
							}
							//should skip because next element was deleted before
							else if (accessHelper[idx_next].is_deleted()) {
								idx_next = (idx_next + step) & mask;
								continue;
							}
							//at this point we know that both values are presented
							key_type const& key_next {keyExtractor(*(accessHelper[idx_next].value()))};
							std::size_t hash_next   {hasher(key_next)};

							//should skip as it is collision of placement, hashes are different (ie 18 and 9)
							if (hash_next != hash_curr) {
								idx_next = (idx_next + step) & mask;
								continue;
							}
							//else finally we should swap
							else {
								std::swap(accessHelper[idx_next], accessHelper[idx_curr]);

								//keep it for next iteration
								hash_curr = hash_next;
								idx_curr = idx_next;

								idx_next = (idx_curr + step) & mask;
							}
						}

						//deleting element at the end of hash sequence
						// data.erase(accessHelper[idx_curr].value());
						deadNodes.splice(deadNodes.end(), data, accessHelper[idx_curr].value());
						accessHelper[idx_curr].reset();
						--sz;
						++deleted_count;
						tryShrink();
					}
#else
					void erase(key_type const &key) {
						AccessIter elemIter {getElemIter(key)};
						if (!contains(elemIter)) {
							return;
						}
						deadNodes.splice(deadNodes.end(), data, elemIter->value());
						// data.erase(elemIter->value());
						elemIter->reset();
						--sz;
						++deleted_count;
						tryShrink();
					}
#endif

					void erase(const_iterator cIter){
						key_type const& key {keyExtractor(*cIter)};
						erase(key);
					}


					void tryShrink() {
						std::size_t targetCapacity {capacityPolicy.capacity()};
						while (targetCapacity > const_values::initial_capacity && 1.0 * sz / (targetCapacity >> 1) <= const_values::maxLoadFactor) 
						{
							targetCapacity >>= 1;
						}
						if (targetCapacity < capacityPolicy.capacity()) {
							rehashTo(targetCapacity);
						}
					}

					void rehashTo(std::size_t newCapacity) {

						capacityPolicy.setCapacity(newCapacity);
						std::size_t const newMask {capacityPolicy.mask()};
						AccessHelper newAccessHelper(newCapacity);

						for (Element<iterator> &entry : accessHelper) {
							if (entry.has_value()) {
								std::size_t h {hasher(keyExtractor(*(entry.value()))) & newMask};
								std::size_t const step {h | 1};
								bool entryUpdated {false};

								for (std::size_t i {0}; i != newCapacity; ++i) {
									if (newAccessHelper[h].is_free()) {
										newAccessHelper[h] = entry;
										entryUpdated = true;
										break;
									}
									h = (h + step) & newMask;
								}
								if (!entryUpdated) {
									throw std::runtime_error("Failed to update element while rehashing");
								}
							}
						}
						std::swap(accessHelper, newAccessHelper);
						deleted_count = 0;
						deadNodes.clear();
					}

					bool contains(AccessCIter iter) const {
						return iter != accessHelper.end() && iter->has_value();
					}

					bool contains(key_type const& key) const {
						AccessCIter iter {getElemIter(key)};
						return contains(iter);
					}

					std::size_t bytesAllocated() const {
						return accessHelper.capacity() * sizeof(typename AccessHelper::value_type);
					}
				};

			public:

				virtual ~HashTable() = default;

				HashTable()
					// : data(pmr::allocator_type{&pmr::resource})
					: memResourcePtr (&pmr::aligned_resource<T>)
					, data(pmr::allocator_type<T>{memResourcePtr})
					, access(data, memResourcePtr)
				{}

				HashTable(std::size_t initialCapacity)
					// : data(pmr::allocator_type{&pmr::resource})
					: memResourcePtr (&pmr::aligned_resource<T>)
					, data(pmr::allocator_type<T>{memResourcePtr})
					, access(data, initialCapacity, memResourcePtr)
				{}

				HashTable(HashTable const& other)
				    // : data(pmr::allocator_type{&pmr::resource})
					: memResourcePtr (other.memResourcePtr)
					, data(pmr::allocator_type<T>{memResourcePtr})
				    , access(data, other.access.capacityPolicy.capacity(), memResourcePtr)
				{
					for (T const& item : other.data){
					    data.emplace_back(
					        std::make_obj_using_allocator<T>(
					            pmr::allocator_type<T>{memResourcePtr},
					            item
					        )
					    );
					}

				    std::unordered_map<T const*, iterator> iterMap;
				    iterMap.reserve(other.access.sz);
				
					const_iterator oldIt {other.data.cbegin()};
				    iterator newIt {data.begin()};
				    for (; oldIt != other.data.cend(); ++oldIt, ++newIt) {
				        iterMap[std::addressof(*oldIt)] = newIt;
					}
				
				    access.sz            = other.access.sz;
				    access.deleted_count = other.access.deleted_count;
				
				    for (std::size_t i {0}; i < other.access.capacityPolicy.capacity(); ++i) {
				        Element<iterator> const& elem {other.access.accessHelper[i]};
				        if (elem.has_value()) {
				            access.accessHelper[i].emplace(iterMap.at(std::addressof(*elem.value())));
						}
				        else if (elem.is_deleted()) {
				            access.accessHelper[i].reset();
						}
				    }
				}

				HashTable& operator=(HashTable const& other) {
				    if (this == &other) {
						return *this;
					}

					data.clear();
					for (T const& item : other.data) {
					    data.emplace_back(
					        std::make_obj_using_allocator<T>(
					            pmr::allocator_type<T>{memResourcePtr},
					            item
					        )
					    );
					}
				
				    access.accessHelper.assign(other.access.capacityPolicy.capacity(), {});
				    access.capacityPolicy = other.access.capacityPolicy;
				    access.sz            = other.access.sz;
				    access.deleted_count = other.access.deleted_count;
				
				    std::unordered_map<T const*, iterator> iterMap;
				    iterMap.reserve(other.access.sz);
				
					const_iterator oldIt {other.data.cbegin()};
				    iterator newIt {data.begin()};
				    for (; oldIt != other.data.cend(); ++oldIt, ++newIt) {
				        iterMap[std::addressof(*oldIt)] = newIt;
					}
				
				    for (std::size_t i {0}; i < other.access.capacityPolicy.capacity(); ++i) {
				        Element<iterator> const& elem {other.access.accessHelper[i]};
				        if (elem.has_value()) {
				            access.accessHelper[i].emplace(iterMap.at(std::addressof(*elem.value())));
						}
				        else if (elem.is_deleted()) {
				            access.accessHelper[i].reset();
						}
				    }

					return *this;
				}

				HashTable(HashTable&& other) noexcept
				    // : data(pmr::allocator_type{&pmr::resource})
					: memResourcePtr (other.memResourcePtr)
					, data(pmr::allocator_type<T>{memResourcePtr})
				    , access(data, 0, memResourcePtr)
				{
				    data.splice(data.end(), other.data);
					access.deadNodes.clear();
				    access.accessHelper = std::move(other.access.accessHelper);
				    access.capacityPolicy = other.access.capacityPolicy;
				    access.sz = other.access.sz;
				    access.deleted_count = other.access.deleted_count;
				}

				HashTable& operator=(HashTable&& other) noexcept 
				{
				    if (this == &other) {
				        return *this;
				    }
				    data.clear();
				    data.splice(data.end(), other.data);
					access.deadNodes.clear();					
				    access.accessHelper = std::move(other.access.accessHelper);
				    access.capacityPolicy = other.access.capacityPolicy;
				    access.sz = other.access.sz;
				    access.deleted_count = other.access.deleted_count;
				    return *this;
				}

				template<typename... Args>
				requires 
					(sizeof...(Args) > 0) &&
					std::constructible_from<T, Args...> && 
					(!std::same_as<T, std::remove_cvref_t<Args>> && ...)
				std::pair<iterator, bool> insert(Args&&... args) {
					return access.insert(T(std::forward<Args>(args)...));
				}

				std::pair<iterator, bool> insert(T value) {
					return access.insert(std::move(value));
				}

				iterator find(key_type const& key) 
				requires requirements::IsMapConcept<type>
					{ return access.find(key); }

				const_iterator find(key_type const& key) const { return access.find(key); }

				void erase(key_type const &key) { access.erase(key); }

				void erase(const_iterator const cIter) { access.erase(cIter); }

				bool contains(key_type const &key) const{ return access.contains(key); }

				const_iterator at(key_type const& key) const {
					const_iterator found {find(key)};
					if (found == this->cend()) {
						throw std::out_of_range("Hash table, method at() gets non-existent key");
					}
					return found;
				}

				std::size_t size() const{ return access.sz; }

				std::size_t capacity() const { return access.capacityPolicy.capacity(); }

				bool empty() const{ return access.sz == 0u; }

				std::size_t bytesAllocated() const { return access.bytesAllocated(); }

				iterator begin() requires requirements::IsMapConcept<type> { return data.begin(); }

				iterator end() requires requirements::IsMapConcept<type> { return data.end(); }

				const_iterator begin() const { return data.cbegin(); }

				const_iterator end() const { return data.cend(); }

				const_iterator cbegin() const { return data.cbegin(); }

				const_iterator cend() const { return data.cend(); }

				reverse_iterator rbegin() requires requirements::IsMapConcept<type> { return data.rbegin(); }

				reverse_iterator rend() requires requirements::IsMapConcept<type> { return data.rend(); }

				const_reverse_iterator rbegin() const { return data.crbegin(); }

				const_reverse_iterator rend() const { return data.crend(); }

				const_reverse_iterator crbegin() const { return data.crbegin(); }

				const_reverse_iterator crend() const { return data.crend(); }

			private:
				std::pmr::memory_resource* memResourcePtr;
				Data data;
				Access access;
			};

		}//!namespace details

		template <typename T, typename Hasher = std::hash<T>, typename KeyEqual = std::equal_to<T>>
		requires
		::requirements::hash::IsHash<T, Hasher, std::size_t> &&
		std::predicate<KeyEqual, T, T>
		struct Set final : public details::HashTable<T, Hasher, KeyEqual, details::requirements::Type::Set> 
		{
		private:
			using base_type = details::HashTable<T, Hasher, KeyEqual, details::requirements::Type::Set>;
		public:
			using key_type = typename base_type::key_type;
			//no mapped_type
			using value_type = typename base_type::value_type;
			using hasher = typename base_type::hasher;
			using key_equal = typename base_type::key_equal;
#if 0
//todo requires some exercises with ctor
//however:
// That's the root cause of your build failure: Map's internal std::pmr::list sees that the mapped type (Set) advertises allocator_type, assumes it's allocator-aware, and tries to call Set(Set&&, alloc const&) — which you never wrote and shouldn't need to write.
//Set, Map (HashTable) — remove public allocator_type. They're containers that manage their own memory_resource* internally. They're never meant to be constructed by someone else passing an allocator in. Exposing the typedef just tricks the PMR machinery into calling constructors that don't exist and shouldn't exist.
			using allocator_type = typename base_type::allocator_type;
#endif			
			using iterator = typename base_type::iterator;
			using const_iterator = typename base_type::const_iterator;
			using reverse_iterator = typename base_type::reverse_iterator;
			using const_reverse_iterator = typename base_type::const_reverse_iterator;

			using base_type::base_type;
		};

		template <typename Key, typename Value, typename Hasher = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
		requires
		::requirements::hash::IsHash<Key, Hasher, std::size_t> &&
		std::predicate<KeyEqual, Key, Key>
		struct Map final : public details::HashTable<std::pair<Key const, Value>, Hasher, KeyEqual, details::requirements::Type::Map> 
		{
		private:
			using base_type = details::HashTable<std::pair<Key const, Value>, Hasher, KeyEqual, details::requirements::Type::Map>;
		public:
			using key_type = typename base_type::key_type;
			using mapped_type = typename base_type::mapped_type;
			using value_type = typename base_type::value_type;
			using hasher = typename base_type::hasher;
			using key_equal = typename base_type::key_equal;
#if 0
//todo requires some exercises with ctor
			using allocator_type = typename base_type::allocator_type;
#endif			
			using iterator = typename base_type::iterator;
			using const_iterator = typename base_type::const_iterator;
			using reverse_iterator = typename base_type::reverse_iterator;
			using const_reverse_iterator = typename base_type::const_reverse_iterator;

			using base_type::base_type;

			Value& operator[](Key const& key)
			requires std::default_initializable<Value>
			{
				iterator found {this->find(key)};
				if (found != this->end()) {
					return found->second;
				}

				auto [inserted, ok] {this->insert(value_type{key, Value{}} )};
				return inserted->second;
			}
		};

	}//!namespace hash_table

}//!namespace containers
