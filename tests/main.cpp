//
// Created by Andrey Solovyev on 20.03.2024.
//

#include <gtest/gtest.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
	testing::InitGoogleTest(&argc, argv);
	testing::GTEST_FLAG(color) = "yes";
//	::testing::GTEST_FLAG(filter) = "hash_table_set.hash_values_collision*";
//	::testing::GTEST_FLAG(filter) = "hash_table_set.*";
//	::testing::GTEST_FLAG(filter) = "hash_table_map.*";
//	::testing::GTEST_FLAG(filter) = "dummy.*";
//	::testing::GTEST_FLAG(filter) = "dummy.dummy_2";
	::testing::GTEST_FLAG(filter) = "hash_table_map.user_test_11";
//	::testing::GTEST_FLAG(filter) = "hash_table_set.no_invalidation";
	auto res {RUN_ALL_TESTS()};
	return res;
}

//todo
// add initializer list to insert
// remove access from public
// add prime numbers or power of two as a capacity driver
// remove hash_table:: namespace from ::containers::hash_table::Set ?
// remove include option
// erase must be optimized

/*
	std::cerr
			<< "\tht size: " << hash_table.size()
			<< "; accessHelper size: " << hash_table.access.accessHelper.size()
			<< "\tPrinting AccessHelper:\n";
	for (std::size_t j = 0; j != hash_table.access.accessHelper.size(); ++j){
		if (hash_table.access.accessHelper[j].has_value()) {
			std::cerr << "\t\tpos: " << j
			          << " key: " << hash_table.access.accessHelper[j].value()->first << '\n';
		}
	}
 */