//
// Created by Andrey Solovyev on 13.03.2024.
//

#pragma once

#include <list>
#include <functional>
#include <concepts>
#include <type_traits>
#include <tuple>
#include <memory_resource>

namespace requirements {

	namespace hash {

		template<typename Type, typename MaybeHash, typename HashResult = std::size_t>
		concept IsHash = (
				                 std::negation_v<std::is_same<HashResult, bool>> &&
				                 std::is_invocable_r_v<std::size_t, MaybeHash, std::add_const_t<std::decay_t<Type>>> &&
				                 std::is_copy_constructible_v<MaybeHash> &&
				                 std::is_move_constructible_v<MaybeHash>) ||
		                 std::is_same_v<typename std::hash<Type>, MaybeHash>;

		template<typename Type, typename MaybeHash, typename HashResult = std::size_t>
		static inline constexpr bool is_hash_v{IsHash<Type, MaybeHash, HashResult> ? true : false};

		[[maybe_unused]] inline auto combine = [](std::size_t hash, std::size_t seed = 0) {
			static constexpr std::size_t magic_number = 0x9e3779b9; //boost hash_combine as a source
			seed ^= hash + magic_number + (seed << 6) + (seed >> 2);
			return seed;
		};

	}//!namespace hash

	namespace cmp {

		template <typename Type, typename MaybeComparator>
		concept IsComparator = requires() {
			requires std::is_invocable_r_v<
					bool,
					MaybeComparator,
					std::add_const_t<std::decay_t<Type>>, std::add_const_t<std::decay_t<Type>>>;
			requires std::is_same_v <
					std::invoke_result_t<MaybeComparator, std::add_const_t<std::decay_t<Type>>, std::add_const_t<std::decay_t<Type>>>,
					bool > ;
		};

		template <typename Type, typename MaybeComparator>
		concept NotComparator = requires() {
			requires !IsComparator<Type, MaybeComparator>;
		};

		template <typename Type, typename MaybeComparator>
		static inline constexpr bool is_comparator_v { IsComparator<Type, MaybeComparator> ? true : false };

	}//!namespace comparator

}//!namespace requirements

namespace containers {

	namespace pmr {

		static inline std::pmr::synchronized_pool_resource resource(std::pmr::get_default_resource());
		using allocator_type = std::pmr::polymorphic_allocator<std::byte>;
		//	usage example
		//	std::pmr::list<int> l(pmr::allocator_type{&pmr::resource});

		struct AllocatorAwareObject {

		};

	}//!namespace details::pmr

	namespace hash_table {

		namespace details {
			namespace const_values {

				static constexpr std::size_t capacity {20};
				static constexpr std::size_t sz {0};
				static constexpr double maxFactor {0.3};

			}//!namespace details::const_values

			template<typename Type1, typename Type2>
			struct InternalPair {
				using FirstType = Type1;
				using SecondType = Type2;
				FirstType first;
				SecondType second;
				InternalPair(FirstType f, SecondType s)
						: first(std::move(f))
						, second(std::move(s))
				{}
				InternalPair(FirstType f)
				requires std::default_initializable<SecondType>
						: first(std::move(f))
						, second(SecondType{})
				{}
			};

			template <typename MappedType>
			concept IsMap = requires {
				std::same_as<MappedType, InternalPair<typename MappedType::FirstType, typename MappedType::SecondType>>;
			};

			template <typename MappedType>
			concept IsSet = !IsMap<MappedType>;

			template<typename T, typename Hasher, typename Equal>
			class HashTable {
			private:
				using MappedType = T;

				static constexpr auto getKeyType(){
					if constexpr (requires {requires IsMap<MappedType>;}) {
						return std::type_identity<typename MappedType::FirstType>{};
					}
					else if constexpr (requires {requires IsSet<MappedType>;}) {
						return std::type_identity<MappedType>{};
					}
					else {
						throw std::invalid_argument("can't recognized mapped type extracting Key Type");
					}
				}

				using KeyType = typename decltype(getKeyType())::type;

				static constexpr auto getValueType(){
					if constexpr (requires {requires IsMap<MappedType>;}) {
						return std::type_identity<typename MappedType::SecondType>{};
					}
					else if constexpr (requires {requires IsSet<MappedType>;}) {
						return std::type_identity<MappedType>{};
					}
					else {
						throw std::invalid_argument("can't recognized mapped type extracting Value Type");
					}
				}

				using ValueType = typename decltype(getValueType())::type;

				struct KeyExtractor {
					ValueType const &operator()(ValueType const& value) const
					requires IsSet<MappedType>
					{
						return value;
					}

					KeyType const &operator()(InternalPair<KeyType const, ValueType> const& value) const
					requires IsMap<MappedType>
					{
						return value.first;
					}
				};

				using Data = typename std::pmr::list<MappedType>;

				static constexpr auto getIterType(){
					if constexpr (requires {requires IsMap<MappedType>;}) {
						return std::type_identity<typename Data::iterator>{};
					}
					else if constexpr (requires {requires IsSet<MappedType>;}) {
						return std::type_identity<typename Data::const_iterator>{};
					}
					else {
						throw std::invalid_argument("can't recognized mapped type extracting Value Type");
					}
				}

			public:
				using Iter = typename decltype(getIterType())::type;
				using CIter = typename Data::const_iterator;
				using CRIter = typename Data::const_reverse_iterator;

			private:
				struct Access {
					using Bucket = typename std::vector<CIter>;
					using Buckets = typename std::vector<Bucket>;

					Data &data;
					Buckets buckets;
					std::size_t capacity {const_values::capacity};
					std::size_t sz {const_values::sz};
					Hasher hasher;
					Equal equal;
					double maxFactor {const_values::maxFactor};
					KeyExtractor keyExtractor;

					explicit Access(Data &data)
							: data(data)
					{
						buckets.resize(capacity);
					}

					Bucket & getBucket(KeyType const &key) {
						std::size_t const hashValue {hasher(key) % capacity};
						return buckets[hashValue];
					}

					Bucket const& getBucket(KeyType const &key) const {
						std::size_t const hashValue {hasher(key) % capacity};
						return buckets[hashValue];
					}

					MappedType const *insert(MappedType mappedValue){
						auto const& key {keyExtractor(mappedValue)};
						Bucket &bucket {getBucket(key)};
						for (auto const &elem: bucket) {
							if (equal(keyExtractor(*elem), key)) {
								return &(*elem);
							}
						}
						data.emplace_back(std::move(mappedValue));
						bucket.emplace_back(std::prev(data.end()));
						T const *res {&(data.back())};
						++sz;
						rehash();
						return res;
					}

					void erase(KeyType const &key){
						Bucket &bucket {getBucket(key)};
						for (auto it = bucket.begin(), ite = bucket.end(); it != ite; ++it) {
							if (equal(keyExtractor((*(*it))), key)) {
								data.erase(*it);
								bucket.erase(it);
								--sz;
								return;
							}
						}
					}

					void erase(CIter cIter){
						data.erase(cIter);
						auto const& key {keyExtractor(*cIter)};
						Bucket &bucket {getBucket(key)};
						for (auto it = bucket.begin(), ite = bucket.end(); it != ite; ++it) {
							if (equal(keyExtractor((*(*it))), key)) {
								bucket.erase(it);
								--sz;
								return;
							}
						}
					}

					CIter getElemIter(KeyType const &key) const {
						Bucket const& bucket {getBucket(key)};
						for (auto const &elem: bucket) {
							if (equal(keyExtractor(*elem), key)) {
								return elem;
							}
						}
						return data.end();
					}

					bool contains(KeyType const &key) const{
						return getElemIter(key) != data.end();
					}

					std::size_t size() const{
						return sz;
					}

					bool empty() const{
						return sz == 0u;
					}

					void rehash(){
						double const currFactor {1.0 * sz / capacity};
						if (currFactor < maxFactor) {
							return;
						}
						capacity *= 2u;
						Buckets newBuckets(capacity);
						for (auto const &bucket: buckets) {
							for (auto const &elem: bucket) {
								std::size_t const hashValue {hasher(keyExtractor(*elem)) % capacity};
								newBuckets[hashValue].emplace_back(elem);
							}
						}
						buckets = std::move(newBuckets);
					}
				};

			public:

				HashTable()
						: data(pmr::allocator_type{&pmr::resource})
						, access(data)
				{}

				virtual ~HashTable() = default;

				//todo
				// ctors and
				// assignment operators due to pmr?

				template<typename... Args>
				requires std::constructible_from<MappedType, Args...>
				MappedType const *insert(Args&&... args){
					return access.insert(MappedType(std::forward<Args>(args)...));
				}

				CIter find(KeyType const& key) { return access.getElemIter(key); }

				void erase(KeyType const &key) { access.erase(key); }

				void erase(CIter const cIter) { access.erase(cIter); }

				bool contains(KeyType const &key) const{ return access.contains(key); }

				CIter at(KeyType const& key) const {
					auto found = find(key);
					if (found == this->cend()) {
						throw std::out_of_range("Hash table, method at() gets non-existent key");
					}
					return found;
				}

				std::size_t size() const{ return access.size(); }

				bool empty() const{ return access.empty(); }

				Iter begin() { return data.begin(); }

				Iter end() { return data.end(); }

				CIter cbegin() const{ return data.cbegin(); }

				CIter cend() const{ return data.cend(); }

				CRIter rbegin() const{ return data.crbegin(); }

				CRIter rend() const{ return data.crend(); }

				CRIter crbegin() const{ return data.crbegin(); }

				CRIter crend() const{ return data.crend(); }

			private:
				Data data;
				Access access;
			};

		}//!namespace details

		template <typename T, typename Hasher = std::hash<T>, typename Equal = std::equal_to<T>>
		requires
		::requirements::hash::IsHash<T, Hasher, std::size_t> &&
		::requirements::cmp::IsComparator<T, Equal>
		struct Set final : public details::HashTable<T, Hasher, Equal> {
			using details::HashTable<T, Hasher, Equal>::HashTable;
		};

		template <typename Key, typename Value, typename Hasher = std::hash<Key>, typename Equal = std::equal_to<Key>>
		requires
		::requirements::hash::IsHash<Key, Hasher, std::size_t> &&
		::requirements::cmp::IsComparator<Key, Equal>
		struct Map final : public details::HashTable<details::InternalPair<Key const, Value>, Hasher, Equal> {
			using details::HashTable<details::InternalPair<Key const, Value>, Hasher, Equal>::HashTable;

			Value& operator[](Key const& key) {
				auto found = this->find(key);
				if (found != this->end()) {
					return const_cast<Value&>(found->second);
				}
				auto* mappedValue {this->insert(key)};
				return const_cast<Value&>(mappedValue->second);
			}
		};

	}//!namespace hash_table

}//!namespace containers
