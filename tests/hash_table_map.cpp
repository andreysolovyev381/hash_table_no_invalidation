//
// Created by Andrey Solovyev on 20.03.2024.
//

#include <gtest/gtest.h>
#include "../include/hash_table.hpp"

#include <filesystem>
#include <fstream>
#include <string>

TEST(hash_table_map, empty) {
	::containers::hash_table::Map<int, int> hashTable;
	bool empty = hashTable.empty();
	ASSERT_TRUE(empty);
	hashTable.insert(1, 1);
	empty = hashTable.empty();
	ASSERT_FALSE(empty);
}

TEST(hash_table_map, size) {
	::containers::hash_table::Map<int, int> hashTable;
	std::size_t sz = hashTable.size();
	ASSERT_EQ(sz, 0u);
	hashTable.insert(1, 1);
	sz = hashTable.size();
	ASSERT_EQ(sz, 1u);
	hashTable.insert(2, 1);
	hashTable.insert(17, 1);
	sz = hashTable.size();
	ASSERT_EQ(sz, 3u);
}

TEST(hash_table_map, contains) {
	::containers::hash_table::Map<int, int> hashTable;
	bool contains = hashTable.contains(42);
	ASSERT_FALSE(contains);
	hashTable.insert(1, 1);
	contains = hashTable.contains(42);
	ASSERT_FALSE(contains);
	hashTable.insert(42, 1);
	contains = hashTable.contains(42);
	ASSERT_TRUE(contains);
}

TEST(hash_table_map, insert) {
	::containers::hash_table::Map<int, int> hashTable;
	hashTable.insert(1, 1);
	std::size_t sz = hashTable.size();
	ASSERT_EQ(sz, 1u);
	hashTable.insert(2, 1);
	hashTable.insert(17, 1);
	sz = hashTable.size();
	ASSERT_EQ(sz, 3u);
	hashTable.insert(17, 2);
	sz = hashTable.size(); //repetitive add
	ASSERT_EQ(sz, 3u);
}

TEST(hash_table_map, remove) {
	::containers::hash_table::Map<int, int> hashTable;
	hashTable.insert(1, 1);
	hashTable.insert(2, 1);
	hashTable.insert(17, 1);
	std::size_t sz = hashTable.size();
	ASSERT_EQ(sz, 3u);

	//remove non-existent
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

	//repetitive remove
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

	//remove from empty hash_table
	contains = hashTable.contains(2);
	ASSERT_FALSE(contains);
	hashTable.erase(2);
	contains = hashTable.contains(2);
	ASSERT_FALSE(contains);
	sz = hashTable.size();
	ASSERT_EQ(sz, 0u);
}

TEST(hash_table_map, elem_ptr) {
	::containers::hash_table::Map<int, int> hashTable;
	auto ptr1 = hashTable.insert(1, 1);
	hashTable.insert(2, 1);
	hashTable.insert(17, 1);
	auto ptr2 = hashTable.insert(1, 1);
	ASSERT_EQ(ptr1, ptr2);
}

TEST(hash_table_map, iterating_forward) {
	::containers::hash_table::Map<int, int> hashTable;
	std::vector<int> v {1, 2, 17};
	for (int i : v) {
		hashTable.insert(i, 1);
	}
	int i = 0;
	for (auto const& elem : hashTable) {
		ASSERT_EQ(elem.first, v[i++]);
		ASSERT_EQ(elem.second, 1);
	}
}

TEST(hash_table_map, iterating_backwards) {
	::containers::hash_table::Map<int, int> hashTable;
	std::vector<int> v {1, 2, 17};
	for (int i : v) {
		hashTable.insert(i, 1);
	}
	int i = 2;
	for (auto it = hashTable.crbegin(), ite = hashTable.crend(); it != ite; ++it) {
		ASSERT_EQ(it->first, v[i--]);
		ASSERT_EQ(it->second, 1);
	}
}

TEST(hash_table_map, no_invalidation) {
	::containers::hash_table::Map<int, int> hashTable;
	auto* addressBefore {&*(hashTable.insert(-1, 1))};
	for (int i = 0; i != 1'000'000; ++i) {
		hashTable.insert(i, 1);
	}
	auto* addressAfter {&*(hashTable.find(-1))};

	ASSERT_EQ(addressBefore, addressAfter);
}




TEST(hash_table_map, hash_values_collision) {
	::containers::hash_table::Map<int, int> hashTable;
	hashTable.insert(799, 1); //with initial capacity 20 h == 19
	hashTable.insert(919, 1);//with initial capacity 20 h == 19 -> busy -> (19 + 19) / 20 == 18
	ASSERT_EQ(hashTable.size(), 2u);
	hashTable.erase(799); //erases slot 19
	ASSERT_EQ(hashTable.size(), 1u);
	hashTable.insert(919, 2); //if no precautions would place it recently freed slot 19
	ASSERT_EQ(hashTable.size(), 1u);
}

inline
auto user_scenario_run = [](std::string input_string, auto& hash_table){
	std::stringstream ss_input(std::move(input_string));

	std::string output;
	int iter_count;
	ss_input >> iter_count;
	std::string cmd;

	int key, value;
	for (int i = 0; i != iter_count; ++i){
		ss_input >> cmd >> key;
		if (cmd == "put") {
			ss_input >> value;
			hash_table[key] = value;
		}
		else if (cmd == "get") {
			auto found = hash_table.find(key);
			if (found == hash_table.end()) {
				output.append("None");
			}
			else {
				output.append(std::to_string(found->second));
			}
			output.append("\n");
		}
		else if (cmd == "delete") {
			auto found = hash_table.find(key);
			if (found == hash_table.end()) {
				output.append("None");
			}
			else {
				output.append(std::to_string(found->second));
			}
			output.append("\n");
			hash_table.erase(found);
		}
	}

	return output;
};

TEST(hash_table_map, user_test_1) {
	std::string
			input {R"(10
get 1
put 1 10
put 2 4
get 1
get 2
delete 2
get 2
put 1 5
get 1
delete 2
)"},
			expected_output {R"(None
10
4
4
None
5
None
)"};
	containers::hash_table::Map<int, int> ht;
	ASSERT_EQ(user_scenario_run(input, ht), expected_output);
}

TEST(hash_table_map, user_test_2) {
	std::string
			input {R"(8
get 9
delete 9
put 9 1
get 9
put 9 2
get 9
put 9 3
get 9
)"},
			expected_output {R"(None
None
1
2
3
)"};
	containers::hash_table::Map<int, int> ht;
	ASSERT_EQ(user_scenario_run(input, ht), expected_output);
}

TEST(hash_table_map, user_test_3) {
	std::string
			input {R"(15
put 20 27
get 20
put 20 21
get 20
get 20
get -1
get 20
get -3
delete 20
get -29
get -33
delete -29
get 16
get 14
put 29 39
)"},
			expected_output {R"(27
21
21
None
21
None
21
None
None
None
None
None
)"};
	containers::hash_table::Map<int, int> ht;
	ASSERT_EQ(user_scenario_run(input, ht), expected_output);
}

TEST(hash_table_map, user_test_4) {
	std::string
			input {R"(15
get 31
get -34
get 24
delete 11
delete 36
delete 21
get 29
get -30
put -7 6
put -7 -26
put -7 6
get -7
get -7
get -7
get -7
)"},
			expected_output {R"(None
None
None
None
None
None
None
None
6
6
6
6
)"};
	containers::hash_table::Map<int, int> ht;
	ASSERT_EQ(user_scenario_run(input, ht), expected_output);
}

TEST(hash_table_map, user_test_5) {
	std::string
			input {R"(15
get 9
get 37
get 30
get -18
get -5
put 15 23
delete 15
get 7
put 3 18
get 3
put 19 -17
get -12
get 19
get -39
get 39
)"},
			expected_output {R"(None
None
None
None
None
23
None
18
None
-17
None
None
)"};
	containers::hash_table::Map<int, int> ht;
	ASSERT_EQ(user_scenario_run(input, ht), expected_output);
}

TEST(hash_table_map, user_test_6) {
	std::string
			input {R"(100
get 964
get 591
get -390
get 331
delete 499
put 896 -538
get 896
get -353
put 896 337
get 245
get 896
get 896
delete 896
put 673 702
put 395 476
put 366 209
get -474
get 487
get 673
get 366
get 366
delete 395
delete 366
delete 910
put 673 -223
get 673
delete 673
get 187
delete 568
delete 908
get -386
delete 460
delete -203
get -14
put -837 -559
put -702 -386
delete -837
get -702
get 288
get -702
delete 135
put -702 -675
get -673
get -887
delete -702
get 434
delete 632
get 747
get -772
get -286
get 62
delete -813
delete -376
get -391
put -463 650
put -463 -427
get -463
get -463
get 756
get -463
get -463
delete 9
get -463
get -463
get -463
put -463 -745
delete 459
get 426
get -463
get -463
get -463
delete -463
delete 854
put -961 -259
get -961
get -697
delete -961
get 931
delete 635
get 576
delete 7
get 129
get 674
delete 268
get 637
delete -185
delete -304
put -181 -836
delete -181
get -655
delete 603
get -98
delete 98
get 522
get 824
get 136
get -945
delete 990
delete -909
delete -442
)"},
			expected_output {R"(None
None
None
None
None
-538
None
None
337
337
337
None
None
702
209
209
476
209
None
-223
-223
None
None
None
None
None
None
None
-559
-386
None
-386
None
None
None
-675
None
None
None
None
None
None
None
None
None
-427
-427
None
-427
-427
None
-427
-427
-427
None
None
-745
-745
-745
-745
None
-259
None
-259
None
None
None
None
None
None
None
None
None
None
-836
None
None
None
None
None
None
None
None
None
None
None
)"};
	containers::hash_table::Map<int, int> ht;
	ASSERT_EQ(user_scenario_run(input, ht), expected_output);
}

TEST(hash_table_map, user_test_7) {
	std::string
			input {R"(100
put -913 -610
delete -913
get 737
get -100
delete -117
delete -884
get -175
get 644
delete 155
get -581
get -925
delete -86
get -177
delete 772
delete -824
delete 984
delete 201
get -834
delete -878
get 309
get -510
delete -944
get 437
delete -47
delete -927
get 697
put -587 496
get -587
get 625
get 157
delete -587
get 968
delete 364
get -997
delete -861
get 598
get -868
get -207
get -607
get 499
get -219
delete 473
put 387 -708
get 387
get 186
get 387
get 387
delete 387
get 579
delete -782
get -64
get 460
get -460
delete 474
get 963
put 147 240
get 147
get 147
delete 147
get -445
get -717
put 799 -95
get -856
put 919 -827
delete 799
get 919
put 919 132
get 919
get 919
put 919 -682
get 175
put -206 407
delete -206
get 17
delete 183
get 919
get 919
get 919
delete 919
put 253 -570
put 253 -979
get 253
put 253 -923
get 253
delete 253
put -898 -686
put 537 576
get 537
get 537
put -803 -655
get -496
delete 537
get -803
get 511
get -898
delete -787
put -803 -356
get -803
put -803 330
delete 770
)"},
			expected_output {R"(-610
None
None
None
None
None
None
None
None
None
None
None
None
None
None
None
None
None
None
None
None
None
None
None
None
496
None
None
496
None
None
None
None
None
None
None
None
None
None
None
-708
None
-708
-708
-708
None
None
None
None
None
None
None
240
240
240
None
None
None
-95
-827
132
132
None
407
None
None
-682
-682
-682
-682
-979
-923
-923
576
576
None
576
-655
None
-686
None
-356
None
)"};
	containers::hash_table::Map<int, int> ht;
	ASSERT_EQ(user_scenario_run(input, ht), expected_output);
}

namespace requirements {
	template <typename buffer_t>
	concept buffer_concept = requires (buffer_t buffer, std::size_t sz) {
		requires std::is_constructible_v<buffer_t, std::size_t, char>;
		requires std::is_default_constructible_v<buffer_t>;
		buffer.data();
		buffer.resize(sz);
		buffer.size();
	};

	template <typename T>
	concept is_writable_concept = requires (T t, std::ostream os) {
		os << t;
	};

}//!namespace

template<requirements::buffer_concept buffer_t = std::string>
buffer_t read_file(std::filesystem::path const &file_path) {
	std::ifstream in_file{file_path, std::ios::in | std::ios::binary};
	if (!in_file)
		throw std::runtime_error("Cannot open " + file_path.filename().string());

	buffer_t res(std::filesystem::file_size(file_path), '\0');;

	in_file.read(res.data(), res.size());

	if (!in_file)
		throw std::runtime_error("Could not read the full contents from " + file_path.filename().string());

	return res;
}

TEST(hash_table_map, user_test_11){
	std::filesystem::path const
		sourcePath {CMAKE_SOURCE_DIR},
		inputFile {sourcePath / "tests/test_11.txt"},
		expectedOutputFile {sourcePath / "tests/test_11.a"};
	std::string
		input {read_file(inputFile)},
		expected_output {read_file(expectedOutputFile)};

	containers::hash_table::Map<int, int> ht;
	std::string actual_output {user_scenario_run(input, ht)};

	std::stringstream a (actual_output), e(expected_output);
	std::string a_str, e_str;
	int count {0};
	while (true) {
		++count;
		bool a_read {a >> a_str};
		bool e_read {e >> e_str};
		if ((!a_read && e_read) || (a_read && !e_read)) {
			std::cerr << "Uneven read from a and e\n";
			std::cerr << (a_read ? a_str : e_str) << '\n';
			break;
		}
		else if (!a_read && !e_read) {
#if 0
			std::cerr << "End of both files\n";
#endif
			break;
		}
		else if (a_str != e_str) {
			std::cerr << "a:\t\"" << a_str << "\"\n";
			std::cerr << "e:\t\"" << e_str << "\"\n";
		}
	}
#if 0
	std::cerr << "Read " << count << " lines\n";
#endif

	// 75009 - total expected lines count in output
	// if we are here, it means that line by line we compared output
	ASSERT_TRUE(count == 75009) << count;
}

TEST (hash_table_map, hash_values_collision3) {

	/*
	 * With capacity set to 20
	 * All of the keys get hash for linear probing that is equal to 9
	 * therefore four keys occupy 9, 18, 7, 16 slots in vector that keeps data
	 * then we try to remove the third one, that occupies elem with idx == 7
	 * then we try ot retrieve fourth pair, that occupies elem with idx == 16
	 */

	containers::hash_table::Map<int, int> ht(20);

	ht[79477009] = 550054007;
	ASSERT_EQ(ht.size(), 1u);
	auto found = ht.find(79477009);
	ASSERT_EQ(found->first, 79477009);
	ASSERT_EQ(found->second, 550054007);

	ht[-614266467] = -394954512;
	ASSERT_EQ(ht.size(), 2u);
	found = ht.find(-614266467);
	ASSERT_EQ(found->first, -614266467);
	ASSERT_EQ(found->second, -394954512);

	ht[401991289] = -828392468;
	ASSERT_EQ(ht.size(), 3u);
	found = ht.find(401991289);
	ASSERT_EQ(found->first, 401991289);
	ASSERT_EQ(found->second, -828392468);

	ht[428606529] = 604479068;
	ASSERT_EQ(ht.size(), 4u);
	found = ht.find(428606529);
	ASSERT_EQ(found->first, 428606529);
	ASSERT_EQ(found->second, 604479068);

	found = ht.find(401991289);
	ASSERT_EQ(found->first, 401991289);
	ASSERT_EQ(found->second, -828392468);
	ASSERT_EQ(ht.size(), 4u);
	ht.erase(found);
	found = ht.find(401991289);
	ASSERT_EQ(found, ht.end());
	ASSERT_EQ(ht.size(), 3u);

	ASSERT_EQ(ht.size(), 3u);
	found = ht.find(428606529);
	ASSERT_NE(found, ht.end());
	ASSERT_EQ(found->first, 428606529);
	ASSERT_EQ(found->second, 604479068);

}

TEST (hash_table_map, hash_values_collision2) {

	/*
	 * Same as previous test, but removing 2nd elem out of 4, instead of
	 * removing 3rd
	 * therefore provoking a hash sequence breach
	 */

	containers::hash_table::Map<int, int> ht(20);

	ht[79477009] = 550054007;
	ASSERT_EQ(ht.size(), 1u);
	auto found = ht.find(79477009);
	ASSERT_EQ(found->first, 79477009);
	ASSERT_EQ(found->second, 550054007);

	ht[-614266467] = -394954512;
	ASSERT_EQ(ht.size(), 2u);
	found = ht.find(-614266467);
	ASSERT_EQ(found->first, -614266467);
	ASSERT_EQ(found->second, -394954512);

	ht[401991289] = -828392468;
	ASSERT_EQ(ht.size(), 3u);
	found = ht.find(401991289);
	ASSERT_EQ(found->first, 401991289);
	ASSERT_EQ(found->second, -828392468);

	ht[428606529] = 604479068;
	ASSERT_EQ(ht.size(), 4u);
	found = ht.find(428606529);
	ASSERT_EQ(found->first, 428606529);
	ASSERT_EQ(found->second, 604479068);

	found = ht.find(-614266467);
	ASSERT_EQ(found->first, -614266467);
	ASSERT_EQ(found->second, -394954512);
	ASSERT_EQ(ht.size(), 4u);
	ht.erase(found);
	found = ht.find(-614266467);
	ASSERT_EQ(found, ht.end());
	ASSERT_EQ(ht.size(), 3u);

	ASSERT_EQ(ht.size(), 3u);
	found = ht.find(428606529);
	ASSERT_NE(found, ht.end());
	ASSERT_EQ(found->first, 428606529);
	ASSERT_EQ(found->second, 604479068);

}

TEST (hash_table_map, hash_values_collision_sequence_breach1) {
	/*
	 * In this test hash sequence breach happens exactly to the reasons of the same hash values
	 * hash % 20 == 16 has the same idx as hash % 20 == 9 (inserted forth time)
	 * */

	containers::hash_table::Map<int, int> ht (20);
	//goes to idx 7
	ht.insert(std::pair{7, 1});
	//to 9
	ht.insert(std::pair{9, 1});
	//to 16
	ht.insert(std::pair{16, 1});
	//to 18
	ht.insert(std::pair{29, 1});
	//should've gone to 7 but occupied, so to 16 but occupied, so to 5
	ht.insert(std::pair{49, 1});
	//should've gone to 7 but occupied, so to 16 but occupied, so to 5 but occupied, so to 14
	ht.insert(std::pair{69, 1});
	//should've gone to 7 but occupied, so to 16 but occupied, so to 5 but occupied, so to 14 but occupied, so to 3
	ht.insert(std::pair{89, 1});

	ASSERT_EQ(ht.size(), 7u);
	ht.erase(9);
	ASSERT_EQ(ht.size(), 6u);

	auto found = ht.find(29);
	ASSERT_NE(found, ht.end());
	ASSERT_EQ(found->first, 29);

	found = ht.find(49);
	ASSERT_NE(found, ht.end());
	ASSERT_EQ(found->first, 49);

	found = ht.find(69);
	ASSERT_NE(found, ht.end());
	ASSERT_EQ(found->first, 69);

	found = ht.find(89);
	ASSERT_NE(found, ht.end());
	ASSERT_EQ(found->first, 89);

	found = ht.find(7);
	ASSERT_NE(found, ht.end());
	ASSERT_EQ(found->first, 7);

	found = ht.find(9);
	ASSERT_EQ(found, ht.end());

}


TEST (hash_table_map, hash_values_collision_sequence_breach2) {
	/*
    * In this test hash sequence breach happens because if deletion on intermediate
    * element - 18
    * if not processed correctly it would result in being blind for existence
    * of element 29
    * */

	containers::hash_table::Map<int, int> ht (20);

	ht.insert(std::pair{18, 1});
	ht.insert(std::pair{9, 1});
	ht.insert(std::pair{29, 1});

	ASSERT_EQ(ht.size(), 3u);
	ht.erase(18);
	ASSERT_EQ(ht.size(), 2u);

	auto found = ht.find(9);
	ASSERT_NE(found, ht.end());
	ASSERT_EQ(found->first, 9);

	found = ht.find(29);
	ASSERT_NE(found, ht.end());
	ASSERT_EQ(found->first, 29);

	found = ht.find(18);
	ASSERT_EQ(found, ht.end());
}
