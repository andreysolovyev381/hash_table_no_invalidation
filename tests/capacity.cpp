//
// Created by Andrey Solovyev on 20.03.2024.
//

#include <gtest/gtest.h>
#include "../include/hash_table.hpp"
#include <string>

using CP = ::containers::hash_table::details::CapacityPolicy;
constexpr std::size_t initialCap {::containers::hash_table::details::const_values::initial_capacity};

TEST(capacity_policy, isPowerOfTwo) {
	using CP = ::containers::hash_table::details::CapacityPolicy;
	ASSERT_TRUE(CP::isPowerOfTwo(1));
	ASSERT_TRUE(CP::isPowerOfTwo(2));
	ASSERT_TRUE(CP::isPowerOfTwo(4));
	ASSERT_TRUE(CP::isPowerOfTwo(32));
	ASSERT_TRUE(CP::isPowerOfTwo(1024));
	ASSERT_FALSE(CP::isPowerOfTwo(0));
	ASSERT_FALSE(CP::isPowerOfTwo(3));
	ASSERT_FALSE(CP::isPowerOfTwo(5));
	ASSERT_FALSE(CP::isPowerOfTwo(6));
	ASSERT_FALSE(CP::isPowerOfTwo(100));
}

TEST(capacity_policy, defaultCapacityIsPowerOfTwo) {
	::containers::hash_table::details::CapacityPolicy cp {0, sizeof(int)};
	ASSERT_TRUE(::containers::hash_table::details::CapacityPolicy::isPowerOfTwo(cp.capacity()));
	ASSERT_EQ(cp.capacity(), 32u);
	ASSERT_EQ(cp.mask(), 31u);
}

TEST(capacity_policy, roundsUpNonPowerOfTwo) {
	::containers::hash_table::details::CapacityPolicy cp3 {3, sizeof(int)};
	ASSERT_EQ(cp3.capacity(), 4u);
	ASSERT_EQ(cp3.mask(), 3u);

	::containers::hash_table::details::CapacityPolicy cp5 {5, sizeof(int)};
	ASSERT_EQ(cp5.capacity(), 8u);
	ASSERT_EQ(cp5.mask(), 7u);

	::containers::hash_table::details::CapacityPolicy cp100 {100, sizeof(int)};
	ASSERT_EQ(cp100.capacity(), 128u);
	ASSERT_EQ(cp100.mask(), 127u);

	::containers::hash_table::details::CapacityPolicy cp1000 {1000, sizeof(int)};
	ASSERT_EQ(cp1000.capacity(), 1024u);
	ASSERT_EQ(cp1000.mask(), 1023u);
}

TEST(capacity_policy, preservesPowerOfTwo) {
	::containers::hash_table::details::CapacityPolicy cp4 {4, sizeof(int)};
	ASSERT_EQ(cp4.capacity(), 4u);

	::containers::hash_table::details::CapacityPolicy cp64 {64, sizeof(int)};
	ASSERT_EQ(cp64.capacity(), 64u);

	::containers::hash_table::details::CapacityPolicy cp256 {256, sizeof(int)};
	ASSERT_EQ(cp256.capacity(), 256u);
}

TEST(capacity_policy, grow) {
	::containers::hash_table::details::CapacityPolicy cp {0, sizeof(int)};
	ASSERT_EQ(cp.capacity(), 32u);

	cp.setCapacity(cp.capacity() << 1);
	ASSERT_EQ(cp.capacity(), 64u);
	ASSERT_EQ(cp.mask(), 63u);
	ASSERT_TRUE(::containers::hash_table::details::CapacityPolicy::isPowerOfTwo(cp.capacity()));

	cp.setCapacity(cp.capacity() << 1);
	ASSERT_EQ(cp.capacity(), 128u);
	ASSERT_EQ(cp.mask(), 127u);
	ASSERT_TRUE(::containers::hash_table::details::CapacityPolicy::isPowerOfTwo(cp.capacity()));
}

TEST(capacity_policy, maskCorrectness) {
	::containers::hash_table::details::CapacityPolicy cp {16, sizeof(int)};
	ASSERT_EQ(cp.mask(), cp.capacity() - 1);

	cp.setCapacity(cp.capacity() << 1);
	ASSERT_EQ(cp.mask(), cp.capacity() - 1);
}

TEST(hash_table_map, capacityDefaultIsPowerOfTwo) {
	::containers::hash_table::Map<int, int> hashTable;
	std::size_t cap {hashTable.capacity()};
	ASSERT_TRUE(::containers::hash_table::details::CapacityPolicy::isPowerOfTwo(cap));
	ASSERT_EQ(cap, 32u);
}

TEST(hash_table_map, capacityCustomRoundsUp) {
	::containers::hash_table::Map<int, int> hashTable {50};
	std::size_t cap {hashTable.capacity()};
	ASSERT_TRUE(::containers::hash_table::details::CapacityPolicy::isPowerOfTwo(cap));
	ASSERT_EQ(cap, 64u);
}

TEST(hash_table_map, capacityCustomPreservesPowerOfTwo) {
	::containers::hash_table::Map<int, int> hashTable {128};
	std::size_t cap {hashTable.capacity()};
	ASSERT_EQ(cap, 128u);
}

TEST(hash_table_map, capacityGrowsAsPowerOfTwoOnRehash) {
	::containers::hash_table::Map<int, int> hashTable {4};
	std::size_t initialCap {hashTable.capacity()};
	ASSERT_EQ(initialCap, 4u);

	for (int i {0}; i < 20; ++i) {
		hashTable.insert(i, i);
	}

	std::size_t grownCap {hashTable.capacity()};
	ASSERT_TRUE(::containers::hash_table::details::CapacityPolicy::isPowerOfTwo(grownCap));
	ASSERT_GT(grownCap, initialCap);
}

TEST(hash_table_map, correctnessAfterRehash) {
	::containers::hash_table::Map<int, int> hashTable {4};

	for (int i {0}; i < 100; ++i) {
		hashTable.insert(i, i * 10);
	}

	std::size_t sz {hashTable.size()};
	ASSERT_EQ(sz, 100u);

	for (int i {0}; i < 100; ++i) {
		bool found {hashTable.contains(i)};
		ASSERT_TRUE(found);
	}
}

TEST(hash_table_set, capacityDefaultIsPowerOfTwo) {
	::containers::hash_table::Set<int> hashTable;
	std::size_t cap {hashTable.capacity()};
	ASSERT_TRUE(::containers::hash_table::details::CapacityPolicy::isPowerOfTwo(cap));
	ASSERT_EQ(cap, 32u);
}

TEST(hash_table_set, capacityCustomRoundsUp) {
	::containers::hash_table::Set<int> hashTable {50};
	std::size_t cap {hashTable.capacity()};
	ASSERT_EQ(cap, 64u);
}

TEST(hash_table_set, capacityGrowsAsPowerOfTwoOnRehash) {
	::containers::hash_table::Set<int> hashTable {4};
	std::size_t initialCap {hashTable.capacity()};
	ASSERT_EQ(initialCap, 4u);

	for (int i {0}; i < 20; ++i) {
		hashTable.insert(i);
	}

	std::size_t grownCap {hashTable.capacity()};
	ASSERT_TRUE(::containers::hash_table::details::CapacityPolicy::isPowerOfTwo(grownCap));
	ASSERT_GT(grownCap, initialCap);
}

TEST(hash_table_map, eraseWorksWithBitmask) {
	::containers::hash_table::Map<int, int> hashTable {4};

	for (int i {0}; i < 10; ++i) {
		hashTable.insert(i, i);
	}

	hashTable.erase(3);
	hashTable.erase(7);

	std::size_t sz {hashTable.size()};
	ASSERT_EQ(sz, 8u);
	ASSERT_FALSE(hashTable.contains(3));
	ASSERT_FALSE(hashTable.contains(7));
	ASSERT_TRUE(hashTable.contains(0));
	ASSERT_TRUE(hashTable.contains(9));
}

TEST(hash_table_map, insertAfterEraseWithBitmask) {
	::containers::hash_table::Map<int, int> hashTable {4};

	for (int i {0}; i < 10; ++i) {
		hashTable.insert(i, i);
	}

	for (int i {0}; i < 10; ++i) {
		hashTable.erase(i);
	}

	std::size_t sz {hashTable.size()};
	ASSERT_EQ(sz, 0u);

	for (int i {100}; i < 110; ++i) {
		hashTable.insert(i, i);
	}

	sz = hashTable.size();
	ASSERT_EQ(sz, 10u);

	for (int i {100}; i < 110; ++i) {
		ASSERT_TRUE(hashTable.contains(i));
	}
}

TEST(hash_table_map, copyPreservesCapacity) {
	::containers::hash_table::Map<int, int> hashTable {50};
	for (int i {0}; i < 10; ++i) {
		hashTable.insert(i, i);
	}

	::containers::hash_table::Map<int, int> copied {hashTable};
	ASSERT_EQ(copied.capacity(), hashTable.capacity());
	ASSERT_EQ(copied.size(), hashTable.size());

	for (int i {0}; i < 10; ++i) {
		ASSERT_TRUE(copied.contains(i));
	}
}

TEST(hash_table_map, movePreservesCapacity) {
	::containers::hash_table::Map<int, int> hashTable {50};
	for (int i {0}; i < 10; ++i) {
		hashTable.insert(i, i);
	}
	std::size_t capBefore {hashTable.capacity()};
	std::size_t szBefore {hashTable.size()};

	::containers::hash_table::Map<int, int> moved {std::move(hashTable)};
	ASSERT_EQ(moved.capacity(), capBefore);
	ASSERT_EQ(moved.size(), szBefore);

	for (int i {0}; i < 10; ++i) {
		ASSERT_TRUE(moved.contains(i));
	}
}

TEST(capacity_shrink_map, noShrinkBelowInitialCapacity) {
	::containers::hash_table::Map<int, int> hashTable;
	hashTable.insert(1, 1);
	hashTable.erase(1);
	std::size_t cap {hashTable.capacity()};
	ASSERT_GE(cap, initialCap);
}

TEST(capacity_shrink_map, shrinkAfterBulkErase) {
	::containers::hash_table::Map<int, int> hashTable {4};

	for (int i {0}; i < 200; ++i) {
		hashTable.insert(i, i);
	}
	std::size_t peakCap {hashTable.capacity()};
	ASSERT_TRUE(CP::isPowerOfTwo(peakCap));

	for (int i {0}; i < 195; ++i) {
		hashTable.erase(i);
	}
	std::size_t shrunkCap {hashTable.capacity()};
	ASSERT_TRUE(CP::isPowerOfTwo(shrunkCap));
	ASSERT_LT(shrunkCap, peakCap);
	ASSERT_GE(shrunkCap, initialCap);
}

TEST(capacity_shrink_map, correctnessAfterShrink) {
	::containers::hash_table::Map<int, int> hashTable {4};

	for (int i {0}; i < 100; ++i) {
		hashTable.insert(i, i * 10);
	}

	for (int i {0}; i < 95; ++i) {
		hashTable.erase(i);
	}

	std::size_t sz {hashTable.size()};
	ASSERT_EQ(sz, 5u);

	for (int i {95}; i < 100; ++i) {
		ASSERT_TRUE(hashTable.contains(i));
		::containers::hash_table::Map<int, int>::const_iterator it {hashTable.at(i)};
		ASSERT_EQ(it->second, i * 10);
	}

	for (int i {0}; i < 95; ++i) {
		ASSERT_FALSE(hashTable.contains(i));
	}
}

TEST(capacity_shrink_map, shrinkStopsAtInitialCapacity) {
	::containers::hash_table::Map<int, int> hashTable;
	hashTable.insert(1, 1);
	hashTable.insert(2, 2);

	for (int i {0}; i < 50; ++i) {
		hashTable.insert(i + 100, i);
	}
	for (int i {0}; i < 50; ++i) {
		hashTable.erase(i + 100);
	}

	hashTable.erase(1);
	hashTable.erase(2);

	std::size_t cap {hashTable.capacity()};
	ASSERT_EQ(cap, initialCap);
}

TEST(capacity_shrink_map, insertAfterShrinkWorks) {
	::containers::hash_table::Map<int, int> hashTable {4};

	for (int i {0}; i < 100; ++i) {
		hashTable.insert(i, i);
	}
	for (int i {0}; i < 98; ++i) {
		hashTable.erase(i);
	}
	std::size_t shrunkCap {hashTable.capacity()};

	for (int i {1000}; i < 1100; ++i) {
		hashTable.insert(i, i);
	}

	std::size_t grownCap {hashTable.capacity()};
	ASSERT_TRUE(CP::isPowerOfTwo(grownCap));
	ASSERT_GT(grownCap, shrunkCap);

	ASSERT_TRUE(hashTable.contains(98));
	ASSERT_TRUE(hashTable.contains(99));
	for (int i {1000}; i < 1100; ++i) {
		ASSERT_TRUE(hashTable.contains(i));
	}
}

TEST(capacity_shrink_map, gradualEraseShrinks) {
	::containers::hash_table::Map<int, int> hashTable {4};

	for (int i {0}; i < 512; ++i) {
		hashTable.insert(i, i);
	}
	std::size_t peakCap {hashTable.capacity()};

	std::size_t prevCap {peakCap};
	int erased {0};
	bool didShrink {false};
	for (int i {0}; i < 510; ++i) {
		hashTable.erase(i);
		++erased;
		std::size_t currCap {hashTable.capacity()};
		if (currCap < prevCap) {
			didShrink = true;
			ASSERT_TRUE(CP::isPowerOfTwo(currCap));
		}
		prevCap = currCap;
	}
	ASSERT_TRUE(didShrink);
}

TEST(capacity_shrink_set, shrinkAfterBulkErase) {
	::containers::hash_table::Set<int> hashTable {4};

	for (int i {0}; i < 200; ++i) {
		hashTable.insert(i);
	}
	std::size_t peakCap {hashTable.capacity()};

	for (int i {0}; i < 195; ++i) {
		hashTable.erase(i);
	}
	std::size_t shrunkCap {hashTable.capacity()};
	ASSERT_TRUE(CP::isPowerOfTwo(shrunkCap));
	ASSERT_LT(shrunkCap, peakCap);

	for (int i {195}; i < 200; ++i) {
		ASSERT_TRUE(hashTable.contains(i));
	}
}

TEST(capacity_bytes_map, bytesProportionalToCapacity) {
	::containers::hash_table::Map<int, int> h1 {32};
	::containers::hash_table::Map<int, int> h2 {64};

	std::size_t bytes32 {h1.bytesAllocated()};
	std::size_t bytes64 {h2.bytesAllocated()};

	ASSERT_GT(bytes32, 0u);
	ASSERT_EQ(bytes64, bytes32 * 2);
}

TEST(capacity_bytes_map, bytesGrowOnRehash) {
	::containers::hash_table::Map<int, int> hashTable {4};
	std::size_t bytesBefore {hashTable.bytesAllocated()};

	for (int i {0}; i < 20; ++i) {
		hashTable.insert(i, i);
	}
	std::size_t bytesAfter {hashTable.bytesAllocated()};

	ASSERT_GT(bytesAfter, bytesBefore);
}

TEST(capacity_bytes_map, bytesShrinkOnErase) {
	::containers::hash_table::Map<int, int> hashTable {4};

	for (int i {0}; i < 200; ++i) {
		hashTable.insert(i, i);
	}
	std::size_t peakBytes {hashTable.bytesAllocated()};

	for (int i {0}; i < 195; ++i) {
		hashTable.erase(i);
	}
	std::size_t shrunkBytes {hashTable.bytesAllocated()};

	ASSERT_LT(shrunkBytes, peakBytes);
}

TEST(capacity_bytes_map, bytesMatchCapacityTimesElementSize) {
	::containers::hash_table::Map<int, int> hashTable {64};

	for (int i {0}; i < 10; ++i) {
		hashTable.insert(i, i);
	}

	std::size_t cap {hashTable.capacity()};
	std::size_t bytes {hashTable.bytesAllocated()};

	ASSERT_EQ(bytes % cap, 0u);
	std::size_t elementSize {bytes / cap};
	ASSERT_GT(elementSize, 0u);

	for (int i {10}; i < 100; ++i) {
		hashTable.insert(i, i);
	}

	std::size_t cap2 {hashTable.capacity()};
	std::size_t bytes2 {hashTable.bytesAllocated()};
	ASSERT_EQ(bytes2 / cap2, elementSize);
}

TEST(capacity_bytes_map, bytesFullCycleGrowAndShrink) {
	::containers::hash_table::Map<int, int> hashTable {4};
	std::size_t initialBytes {hashTable.bytesAllocated()};

	for (int i {0}; i < 500; ++i) {
		hashTable.insert(i, i);
	}
	std::size_t peakBytes {hashTable.bytesAllocated()};
	ASSERT_GT(peakBytes, initialBytes);

	for (int i {0}; i < 499; ++i) {
		hashTable.erase(i);
	}
	std::size_t finalBytes {hashTable.bytesAllocated()};
	ASSERT_LT(finalBytes, peakBytes);

	std::size_t finalCap {hashTable.capacity()};
	ASSERT_GE(finalCap, initialCap);
	ASSERT_TRUE(CP::isPowerOfTwo(finalCap));
	ASSERT_EQ(hashTable.size(), 1u);
	ASSERT_TRUE(hashTable.contains(499));
}

TEST(capacity_bytes_set, bytesGrowAndShrink) {
	::containers::hash_table::Set<int> hashTable {4};
	std::size_t initialBytes {hashTable.bytesAllocated()};

	for (int i {0}; i < 300; ++i) {
		hashTable.insert(i);
	}
	std::size_t peakBytes {hashTable.bytesAllocated()};
	ASSERT_GT(peakBytes, initialBytes);

	for (int i {0}; i < 298; ++i) {
		hashTable.erase(i);
	}
	std::size_t shrunkBytes {hashTable.bytesAllocated()};
	ASSERT_LT(shrunkBytes, peakBytes);
}

TEST(capacity_bytes_map, differentValueSizesAffectElementSize) {
	::containers::hash_table::Map<int, int> smallMap {64};
	::containers::hash_table::Map<int, std::array<char, 256>> bigMap {64};

	std::size_t smallElementSize {smallMap.bytesAllocated() / smallMap.capacity()};
	std::size_t bigElementSize {bigMap.bytesAllocated() / bigMap.capacity()};

	ASSERT_EQ(smallElementSize, bigElementSize);
}