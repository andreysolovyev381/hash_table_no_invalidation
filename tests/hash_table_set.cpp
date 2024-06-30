//
// Created by Andrey Solovyev on 20.03.2024.
//

#include <gtest/gtest.h>
#include "../include/hash_table.hpp"

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
	::containers::hash_table::Set<int> hashTable(10);
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
	auto iterBefore {hashTable.insert(-1)};
	for (int i = 0; i != 1'000'000; ++i) {
		hashTable.insert(i);
	}
	auto iterAfter {hashTable.find(-1)};
	ASSERT_EQ(*iterBefore, *iterAfter);
}

TEST(hash_table_set, hash_values_collision1) {
	::containers::hash_table::Set<int> hashTable;
	hashTable.insert(799); //with initial capacity 20 h == 19
	hashTable.insert(919);//with initial capacity 20 h == 19 -> busy -> (19 + 19) / 20 == 18
	ASSERT_EQ(hashTable.size(), 2u);
	hashTable.erase(799); //erases slot 19
	ASSERT_EQ(hashTable.size(), 1u);
	hashTable.insert(919); //if no precautions would place it recently freed slot 19
	ASSERT_EQ(hashTable.size(), 1u);
}

TEST (hash_table_set, hash_values_collision3) {

	/*
	 * With capacity set to 20
	 * All of the keys get step for linear probing that is equal to 9
	 * therefore four keys occupy 9, 18, 7, 16 slots in vector that keeps data
	 * then we try to remove the third one, that occupies elem with idx == 7
	 * then we try ot retrieve fourth pair, that occupies elem with idx == 16
	 */

	containers::hash_table::Set<int> ht(20);

	auto found = ht.find(79477009);
	ASSERT_EQ(found, ht.end());
	ht.insert(79477009);
	ASSERT_EQ(ht.size(), 1u);
	found = ht.find(79477009);
	ASSERT_NE(found, ht.end());

	found = ht.find(-614266467);
	ASSERT_EQ(found, ht.end());
	ht.insert(-614266467);
	ASSERT_EQ(ht.size(), 2u);
	found = ht.find(-614266467);
	ASSERT_NE(found, ht.end());

	found = ht.find(401991289);
	ASSERT_EQ(found, ht.end());
	ht.insert(401991289);
	ASSERT_EQ(ht.size(), 3u);
	found = ht.find(401991289);
	ASSERT_NE(found, ht.end());

	found = ht.find(428606529);
	ASSERT_EQ(found, ht.end());
	ht.insert(428606529);
	ASSERT_EQ(ht.size(), 4u);
	found = ht.find(428606529);
	ASSERT_NE(found, ht.end());

	found = ht.find(401991289);
	ASSERT_NE(found, ht.end());
	ht.erase(found);
	found = ht.find(401991289);
	ASSERT_EQ(found, ht.end());
	ASSERT_EQ(ht.size(), 3u);

	found = ht.find(428606529);
	ASSERT_NE(found, ht.end());
}


TEST (hash_table_set, hash_values_collision2) {

	/*
	 * Same as previous test, but removing 2nd elem out of 4, instead of
	 * removing 3rd
	 */

	containers::hash_table::Set<int> ht(20);

	auto found = ht.find(79477009);
	ASSERT_EQ(found, ht.end());
	ht.insert(79477009);
	ASSERT_EQ(ht.size(), 1u);
	found = ht.find(79477009);
	ASSERT_NE(found, ht.end());

	found = ht.find(-614266467);
	ASSERT_EQ(found, ht.end());
	ht.insert(-614266467);
	ASSERT_EQ(ht.size(), 2u);
	found = ht.find(-614266467);
	ASSERT_NE(found, ht.end());

	found = ht.find(401991289);
	ASSERT_EQ(found, ht.end());
	ht.insert(401991289);
	ASSERT_EQ(ht.size(), 3u);
	found = ht.find(401991289);
	ASSERT_NE(found, ht.end());

	found = ht.find(428606529);
	ASSERT_EQ(found, ht.end());
	ht.insert(428606529);
	ASSERT_EQ(ht.size(), 4u);
	found = ht.find(428606529);
	ASSERT_NE(found, ht.end());

	found = ht.find(-614266467);
	ASSERT_NE(found, ht.end());
	ht.erase(found);
	found = ht.find(-614266467);
	ASSERT_EQ(found, ht.end());
	ASSERT_EQ(ht.size(), 3u);

	found = ht.find(428606529);
	ASSERT_NE(found, ht.end());
}
