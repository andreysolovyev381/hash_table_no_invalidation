//
// Created by Andrey Solovyev on 20.03.2024.
//

#include <gtest/gtest.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
	testing::InitGoogleTest(&argc, argv);
	testing::GTEST_FLAG(color) = "yes";
	auto res {RUN_ALL_TESTS()};
	return res;
}

//todo
// add initializer list to insert
// currently find() returns const object
// capacity should create data elements, not just access vector?
