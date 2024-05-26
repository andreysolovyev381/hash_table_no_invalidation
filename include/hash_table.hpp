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
#include <stdexcept>
#include <optional>

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
			static constexpr std::size_t magic_number {0x9e3779b9}; //boost hash_combine as a source
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

	}//!namespace details::pmr

	namespace hash_table {

		namespace details {

			namespace const_values {

				static constexpr std::size_t initial_capacity {20};
				static constexpr double maxLoadFactor {0.5};

			}//!namespace details::const_values

			namespace requirements {

				template<typename MappedType>
				concept IsMapConcept = std::same_as<MappedType,
						std::pair<typename MappedType::first_type, typename MappedType::second_type>>;

				template<typename MappedType>
				concept IsSetConcept = !IsMapConcept<MappedType>;

			}//!namespace details::requirements

			template<typename T, typename Hasher, typename Equal>
			class HashTable {
			private:
				using MappedType = T;

				static constexpr auto getKeyType(){
					if constexpr (requires {requires requirements::IsMapConcept<MappedType>;}) {
						return std::type_identity<typename MappedType::first_type>{};
					}
					else if constexpr (requires {requires requirements::IsSetConcept<MappedType>;}) {
						return std::type_identity<MappedType>{};
					}
					else {
						throw std::invalid_argument("can't recognized mapped type extracting Key Type");
					}
				}

				using KeyType = typename decltype(getKeyType())::type;

				static constexpr auto getValueType(){
					if constexpr (requires {requires requirements::IsMapConcept<MappedType>;}) {
						return std::type_identity<typename MappedType::second_type>{};
					}
					else if constexpr (requires {requires requirements::IsSetConcept<MappedType>;}) {
						return std::type_identity<MappedType>{};
					}
					else {
						throw std::invalid_argument("can't recognized mapped type extracting Value Type");
					}
				}

				using ValueType = typename decltype(getValueType())::type;

				struct KeyExtractor {
					KeyType const& operator()(std::pair<KeyType const, ValueType> const& value) const
					requires requirements::IsMapConcept<MappedType>
					{
						return value.first;
					}

					ValueType const& operator()(ValueType const& value) const
					requires requirements::IsSetConcept<MappedType>
					{
						return value;
					}
				};

				using Data = typename std::pmr::list<MappedType>;

				static constexpr auto getIterType(){
					if constexpr (requires {requires requirements::IsMapConcept<MappedType>;}) {
						return std::type_identity<typename Data::iterator>{};
					}
					else if constexpr (requires {requires requirements::IsSetConcept<MappedType>;}) {
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
					using AccessHelper = typename std::vector<std::optional<CIter>>;
					using AccessIter = typename AccessHelper::iterator;
					using AccessCIter = typename AccessHelper::const_iterator;

					Data &data;
					AccessHelper accessHelper;
					std::size_t capacity {const_values::initial_capacity};
					std::size_t sz {0};

					Hasher hasher;
					Equal equal;
					KeyExtractor keyExtractor;

					explicit Access(Data &data)
							: data(data)
					{
						accessHelper.resize(capacity);
					}

					AccessIter getElemIter(KeyType const &key) {
						std::size_t h {hasher(key) % capacity};
						std::size_t step {h | 1};

						for (std::size_t i = 0; i != capacity; ++i) {
							if (!accessHelper[h].has_value() || equal(keyExtractor(*(accessHelper[h].value())), key)) {
								return accessHelper.begin() + h;
							}
							h = (h + step) % capacity;
						}
						return accessHelper.end();
					}

					AccessCIter getElemIter(KeyType const &key) const {
						std::size_t h {hasher(key) % capacity};
						std::size_t step {h};
						step |= 1;

						for (std::size_t i = 0; i != capacity; ++i) {
							if (!accessHelper[h].has_value() || equal(keyExtractor(*(accessHelper[h].value())), key)) {
								return accessHelper.cbegin() + h;
							}
							h = (h + step) % capacity;
						}
						return accessHelper.cend();
					}

					CIter find(KeyType const &key) {
						AccessIter elemIter {getElemIter(key)};
						return contains(elemIter) ? elemIter->value() : data.end();
					}

					CIter find(KeyType const &key) const {
						AccessCIter elemIter {getElemIter(key)};
						return contains(elemIter) ? elemIter->value() : data.cend();
					}

					CIter emplace(MappedType mappedValue){
						data.emplace_back(std::move(mappedValue));
						++sz;
						return std::prev(data.end());
					}

					CIter insert(MappedType mappedValue){
						double const currLoadFactor {1.0 * sz / capacity};
						if (currLoadFactor > const_values::maxLoadFactor) {
							rehash();
						}
						auto const& key {keyExtractor(mappedValue)};
						AccessIter elemIter {getElemIter(key)};
						if (contains(elemIter)) {
							return elemIter->value();
						}
						else if (emplaceable(elemIter)) {
							*elemIter = emplace(std::move(mappedValue));
							return elemIter->value();
						}
						else {
							rehash();
							elemIter = getElemIter(key);
							if (!emplaceable(elemIter)) {
								throw std::runtime_error("Unable to emplace after rehash, consider reducing const_values::maxLoadFactor");
							}
							*elemIter = emplace(std::move(mappedValue));
							return elemIter->value();
						}
					}

					void erase(KeyType const &key) {
						AccessIter elemIter {getElemIter(key)};
						if (!contains(elemIter)) {
							return;
						}

						data.erase(elemIter->value());
						/*
						https://en.cppreference.com/w/cpp/iterator/distance
						Complexity
						Linear.
						However, if InputIt additionally meets the requirements of LegacyRandomAccessIterator, complexity is constant.
						*/
						std::size_t h = std::distance(accessHelper.begin(), elemIter);
						std::size_t step {h | 1};

						accessHelper[h].reset();
						--sz;

						for (std::size_t i = 0; i != capacity; ++i) {
							h = (h + step) % capacity;
							if (!accessHelper[h].has_value()) {
								break;
							}

							CIter elem {*accessHelper[h]};
							accessHelper[h].reset();

							std::size_t newH {hasher(keyExtractor(*elem)) % capacity};
							std::size_t newStep {newH | 1};

							for (std::size_t j = 0; j != capacity; ++j) {
								if (!accessHelper[newH].has_value()) {
									accessHelper[newH] = elem;
									break;
								}
								newH = (newH + newStep) % capacity;
							}
						}
					}

					void erase(CIter cIter){
						auto const& key {keyExtractor(*cIter)};
						erase(key);
					}

					void rehash() {
						std::size_t const new_capacity {capacity * 2u};
						AccessHelper newAccessHelper(new_capacity);

						for (auto &entry : accessHelper) {
							if (entry.has_value()) {
								std::size_t h {hasher(keyExtractor(*entry.value())) % new_capacity};
								std::size_t step {h | 1};
								bool entryUpdated {false};

								for (std::size_t i = 0; i != new_capacity; ++i) {
									if (!newAccessHelper[h].has_value()) {
										newAccessHelper[h] = entry.value();
										entryUpdated = true;
										break;
									}
									h = (h + step) % new_capacity;
								}
								if (!entryUpdated) {
									throw std::runtime_error("Failed to update element while rehashing");
								}
							}
						}
						std::swap(accessHelper, newAccessHelper);
						capacity = new_capacity;
					}

					bool contains(AccessCIter iter) const {
						return iter != accessHelper.end() && iter->has_value();
					}

					bool contains(KeyType const& key) const {
						AccessCIter iter {getElemIter(key)};
						return contains(iter);
					}

					bool emplaceable(AccessCIter iter) const {
						return iter != accessHelper.end() && !iter->has_value();
					}

				};

			public:

				HashTable()
						: data(pmr::allocator_type{&pmr::resource})
						, access(data)
				{}

				virtual ~HashTable() = default;

				HashTable(HashTable const&) = delete;

				HashTable(HashTable &&) = default;

				HashTable& operator=(HashTable const&) = delete;

				HashTable& operator=(HashTable &&) = default;

				template<typename... Args>
				requires std::constructible_from<MappedType, Args...>
				CIter insert(Args&&... args){
					return access.insert(MappedType(std::forward<Args>(args)...));
				}

				CIter find(KeyType const& key) { return access.find(key); }

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

				std::size_t size() const{ return access.sz; }

				bool empty() const{ return access.sz == 0u; }

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
		struct Map final : public details::HashTable<std::pair<Key const, Value>, Hasher, Equal> {
			using details::HashTable<std::pair<Key const, Value>, Hasher, Equal>::HashTable;

			Value& operator[](Key const& key) {
				auto found = this->find(key);
				if (found != this->end()) {
					return const_cast<Value&>(found->second);
				}
				static_assert(std::is_default_constructible_v<Value>, "hash table operator[] requires default constructible Value");
				auto inserted {this->insert(key, Value{})};
				return const_cast<Value&>(inserted->second);
			}
		};

	}//!namespace hash_table

}//!namespace containers
