//
// Created by Andrey Solovyev on 29.06.2024.
//

#include <gtest/gtest.h>
#include "../include/hash_table.hpp"

template <typename T = std::nullptr_t>
struct Element {

	struct Deleted{};

	Element () = default;

	Element (T t) {
		value.template emplace<T>(std::move(t));
	}

	constexpr bool is_not_initialised() const {
		return std::holds_alternative<std::monostate>(value);
	}

	constexpr bool has_value() const {
		return std::holds_alternative<T>(value);
	}

	constexpr bool is_deleted() const {
		return std::holds_alternative<Deleted>(value);
	}

	T& get_value() {
		return std::get<T>(value);
	}

	void delete_value() {
		value.template emplace<Deleted>(Deleted{});
	}

	std::variant<std::monostate, T, Deleted> value;

};

TEST(dummy, elem) {
	Element elem_default;
	bool check = elem_default.is_not_initialised();
	ASSERT_TRUE(check);
	check = elem_default.has_value();
	ASSERT_FALSE(check);
	check = elem_default.is_deleted();
	ASSERT_FALSE(check);

	Element elem(42);
	check = elem.is_not_initialised();
	ASSERT_FALSE(check);
	check = elem.has_value();
	ASSERT_TRUE(check);
	check = elem.is_not_initialised();
	ASSERT_FALSE(check);

	elem.delete_value();
	check = elem.is_not_initialised();
	ASSERT_FALSE(check);
	check = elem.has_value();
	ASSERT_FALSE(check);
	check = elem.is_deleted();
	ASSERT_TRUE(check);
}

TEST(dummy, value) {
	Element elem(42);

	auto value = elem.get_value();
	ASSERT_EQ(value, 42);
}
TEST(dummy, fail_value1) {
	Element elem;
	[[maybe_unused]] auto value = elem.get_value();
}

TEST(dummy, fail_delete) {
	Element elem;
	elem.delete_value();
}
TEST(dummy, fail_value2) {
	Element elem;
	elem.delete_value();
	[[maybe_unused]] auto value = elem.get_value();
}