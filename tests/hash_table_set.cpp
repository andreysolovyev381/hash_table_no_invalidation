//
// Created by Andrey Solovyev on 20.03.2024.
//

#include <gtest/gtest.h>
#include "../hash_table.hpp"

TEST(hash_table_set, empty) {
	::containers::hash_table::Set<int> hashTable;
	bool empty = hashTable.empty();
	ASSERT_TRUE(empty);
	hashTable.insert(1);
	empty = hashTable.empty();
	ASSERT_FALSE(empty);
}

TEST(hash_table_set, size) {
	::containers::hash_table::Set<int> hashTable;
	std::size_t sz = hashTable.size();
	ASSERT_EQ(sz, 0u);
	hashTable.insert(1);
	sz = hashTable.size();
	ASSERT_EQ(sz, 1u);
	hashTable.insert(2);
	hashTable.insert(17);
	sz = hashTable.size();
	ASSERT_EQ(sz, 3u);
}

TEST(hash_table_set, contains) {
	::containers::hash_table::Set<int> hashTable;
	bool contains = hashTable.contains(42);
	ASSERT_FALSE(contains);
	hashTable.insert(1);
	contains = hashTable.contains(42);
	ASSERT_FALSE(contains);
	hashTable.insert(42);
	contains = hashTable.contains(42);
	ASSERT_TRUE(contains);
}

TEST(hash_table_set, insert) {
	::containers::hash_table::Set<int> hashTable;
	hashTable.insert(1);
	std::size_t sz = hashTable.size();
	ASSERT_EQ(sz, 1u);
	hashTable.insert(2);
	hashTable.insert(17);
	sz = hashTable.size();
	ASSERT_EQ(sz, 3u);
	hashTable.insert(17);
	sz = hashTable.size(); //repetitive add
	ASSERT_EQ(sz, 3u);
}

TEST(hash_table_set, erase) {
	::containers::hash_table::Set<int> hashTable;
	hashTable.insert(1);
	hashTable.insert(2);
	hashTable.insert(17);
	std::size_t sz = hashTable.size();
	ASSERT_EQ(sz, 3u);

	//erase non-existent
	hashTable.erase(42);
	sz = hashTable.size();
	ASSERT_EQ(sz, 3u);

	bool contains = hashTable.contains(1);
	ASSERT_TRUE(contains);
	hashTable.erase(1);
	contains = hashTable.contains(1);
	ASSERT_FALSE(contains);
	sz = hashTable.size();
	ASSERT_EQ(sz, 2u);

	//repetitive erase
	contains = hashTable.contains(1);
	ASSERT_FALSE(contains);
	hashTable.erase(1);
	contains = hashTable.contains(1);
	ASSERT_FALSE(contains);
	sz = hashTable.size();
	ASSERT_EQ(sz, 2u);

	contains = hashTable.contains(17);
	ASSERT_TRUE(contains);
	hashTable.erase(17);
	contains = hashTable.contains(17);
	ASSERT_FALSE(contains);
	sz = hashTable.size();
	ASSERT_EQ(sz, 1u);

	contains = hashTable.contains(2);
	ASSERT_TRUE(contains);
	hashTable.erase(2);
	contains = hashTable.contains(2);
	ASSERT_FALSE(contains);
	sz = hashTable.size();
	ASSERT_EQ(sz, 0u);

	//erase from empty hash_table
	contains = hashTable.contains(2);
	ASSERT_FALSE(contains);
	hashTable.erase(2);
	contains = hashTable.contains(2);
	ASSERT_FALSE(contains);
	sz = hashTable.size();
	ASSERT_EQ(sz, 0u);
}

TEST(hash_table_set, elem_ptr) {
	::containers::hash_table::Set<int> hashTable;
	auto ptr1 = hashTable.insert(1);
	hashTable.insert(2);
	hashTable.insert(17);
	auto ptr2 = hashTable.insert(1);
	ASSERT_EQ(ptr1, ptr2);
}

TEST(hash_table_set, iterating_forward) {
	::containers::hash_table::Set<int> hashTable;
	std::vector<int> v {1, 2, 17};
	for (int i : v) {
		hashTable.insert(i);
	}
	int i = 0;
	for (auto const& elem : hashTable) {
		ASSERT_EQ(elem, v[i++]);
	}
}

TEST(hash_table_set, iterating_backwards) {
	::containers::hash_table::Set<int> hashTable;
	std::vector<int> v {1, 2, 17};
	for (int i : v) {
		hashTable.insert(i);
	}
	int i = 2;
	for (auto it = hashTable.crbegin(), ite = hashTable.crend(); it != ite; ++it) {
		ASSERT_EQ(*it, v[i--]);
	}
}

TEST(hash_table_set, no_invalidation) {
	::containers::hash_table::Set<int> hashTable;
	auto const* const ptrBefore {hashTable.insert(-1)};
	for (int i = 0; i != 1'000'000; ++i) {
		hashTable.insert(i);
	}
	auto const* const ptrAfter {&*hashTable.find(-1)};
	ASSERT_EQ(ptrBefore, ptrAfter);
}
