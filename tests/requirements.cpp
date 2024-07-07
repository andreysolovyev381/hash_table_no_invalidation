//
// Created by Andrey Solovyev on 16/02/2023.
//

#include <gtest/gtest.h>
#include "../include/hash_table.hpp"


template <typename T>
struct S {
	T value {42};
	operator double () {return static_cast<double>(value);}
};

struct S1 {int value {42};};

struct SHasher {
	std::size_t operator () (const S1& s) const {
		return s.value;
	}
};
bool operator == (const S1& lhs, const S1& rhs) {
	return lhs.value == rhs.value;
}
struct SComparator {
	bool operator () (const S1& lhs, const S1& rhs) const {
		return lhs.value < rhs.value;
	}
};

using namespace requirements;

TEST (type_requirements, Hash) {
	bool result = hash::is_hash_v<S1, SHasher>;
	ASSERT_TRUE(result);
	result = hash::is_hash_v<S1, SComparator>;
	ASSERT_FALSE(result);
	result = hash::is_hash_v<S1, double>;
	ASSERT_FALSE(result);
	std::hash<int> hi;
	result = hash::is_hash_v<int, decltype(hi)>;
	ASSERT_TRUE(result);
}


TEST (type_requirements, Comparator) {
	bool result = cmp::is_comparator_v<S1, SComparator>;
	ASSERT_TRUE(result);
	result = cmp::is_comparator_v<S1, SHasher>;
	ASSERT_FALSE(result);
	result = cmp::is_comparator_v<S1, double>;
	ASSERT_FALSE(result);
	result = cmp::is_comparator_v<S1, std::less<S1>>;
	ASSERT_TRUE(result);
}
