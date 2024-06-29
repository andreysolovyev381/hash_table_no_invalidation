//
// Created by Andrey Solovyev on 20.03.2024.
//

#include <gtest/gtest.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
	testing::InitGoogleTest(&argc, argv);
	testing::GTEST_FLAG(color) = "yes";
//	::testing::GTEST_FLAG(filter) = "shit_test*.*";
	::testing::GTEST_FLAG(filter) = "dummy*.*";
	auto res {RUN_ALL_TESTS()};
	return res;
}
