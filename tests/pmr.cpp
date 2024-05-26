//
// Created by Andrey Solovyev on 20.03.2024.
//

#include <gtest/gtest.h>
#include "../include/hash_table.hpp"

struct ObjectWithPmrField {
	std::pmr::list<int> l;

	ObjectWithPmrField()
	: l (containers::pmr::allocator_type {&containers::pmr::resource})
	{}

	virtual ~ObjectWithPmrField() = default;

	ObjectWithPmrField(ObjectWithPmrField const&) = delete;

	ObjectWithPmrField(ObjectWithPmrField &&) = default;

	ObjectWithPmrField& operator=(ObjectWithPmrField const&) = delete;

	ObjectWithPmrField& operator=(ObjectWithPmrField &&) = default;
};

#if 0
TEST(hash_table_pmr, copy_ctor) {
	ObjectWithPmrField first;
	ObjectWithPmrField second (first);

	bool same {std::is_same_v<std::decay_t<decltype(first)>, std::decay_t<decltype(second)>>};
	ASSERT_TRUE(same);

	same = std::is_same_v<std::decay_t<decltype(first.l)>, std::decay_t<decltype(second.l)>>;
	ASSERT_TRUE(same);

	same = std::is_same_v<std::decay_t<decltype(first.l.get_allocator())>, std::decay_t<decltype(second.l.get_allocator())>>;
	ASSERT_TRUE(same);

	same = first.l.get_allocator().resource()->is_equal(*second.l.get_allocator().resource());
	ASSERT_TRUE(same);
}

TEST(hash_table_pmr, copy_assignment) {
	ObjectWithPmrField first;
	ObjectWithPmrField second = first;

	bool same {std::is_same_v<std::decay_t<decltype(first)>, std::decay_t<decltype(second)>>};
	ASSERT_TRUE(same);

	same = std::is_same_v<std::decay_t<decltype(first.l)>, std::decay_t<decltype(second.l)>>;
	ASSERT_TRUE(same);

	same = std::is_same_v<std::decay_t<decltype(first.l.get_allocator())>, std::decay_t<decltype(second.l.get_allocator())>>;
	ASSERT_TRUE(same);

	same = first.l.get_allocator().resource()->is_equal(*second.l.get_allocator().resource());
	ASSERT_TRUE(same);
}
#endif

TEST(hash_table_pmr, move_ctor) {
	ObjectWithPmrField first, first_copy;
	ObjectWithPmrField second (std::move(first));

	bool same {std::is_same_v<std::decay_t<decltype(first_copy)>, std::decay_t<decltype(second)>>};
	ASSERT_TRUE(same);

	same = std::is_same_v<std::decay_t<decltype(first_copy.l)>, std::decay_t<decltype(second.l)>>;
	ASSERT_TRUE(same);

	same = std::is_same_v<std::decay_t<decltype(first_copy.l.get_allocator())>, std::decay_t<decltype(second.l.get_allocator())>>;
	ASSERT_TRUE(same);

	same = first_copy.l.get_allocator().resource()->is_equal(*second.l.get_allocator().resource());
	ASSERT_TRUE(same);
}

TEST(hash_table_pmr, move_assignment) {
	ObjectWithPmrField first, first_copy;
	ObjectWithPmrField second = std::move(first);

	bool same {std::is_same_v<std::decay_t<decltype(first_copy)>, std::decay_t<decltype(second)>>};
	ASSERT_TRUE(same);

	same = std::is_same_v<std::decay_t<decltype(first_copy.l)>, std::decay_t<decltype(second.l)>>;
	ASSERT_TRUE(same);

	same = std::is_same_v<std::decay_t<decltype(first_copy.l.get_allocator())>, std::decay_t<decltype(second.l.get_allocator())>>;
	ASSERT_TRUE(same);

	same = first_copy.l.get_allocator().resource()->is_equal(*second.l.get_allocator().resource());
	ASSERT_TRUE(same);
}
