//
// Created by Andrey Solovyev on 20.03.2024.
//

#include <gtest/gtest.h>
#include "../include/hash_table.hpp"
#include <filesystem>
#include <fstream>
#include <string>

static bool new_log {true};

inline
auto log_error = [](std::string const& output_actual, std::string const& output_expected, auto& hash_table){
	std::filesystem::path const dumpFile {std::filesystem::path(CMAKE_SOURCE_DIR) / "tests/error.log"};
	std::fstream dump;
	if (new_log) {
		dump.open(dumpFile, std::ios::out | std::ios::trunc);
		new_log = false;
	}
	else {
		dump.open(dumpFile, std::ios::out | std::ios::app);
	}
	if (!dump.is_open()) {
		throw std::invalid_argument("Can't open a dump file");
	}
	dump << "Actual output:\n\t\t" << output_actual << '\n';
	dump << "Expected output:\n\t\t" << output_expected << '\n';

	dump
			<< "\tht size: " << hash_table.size()
			<< "; accessHelper size: " << hash_table.access.accessHelper.size()
			<< "\tPrinting AccessHelper:\n";
	for (std::size_t j = 0; j != hash_table.access.accessHelper.size(); ++j){
		if (hash_table.access.accessHelper[j].has_value()) {
			dump << "\t\tpos: " << j
			<< " key: " << (hash_table.access.accessHelper[j].value())->first << '\n';
		}
	}
};

inline
auto user_scenario_run = [](std::string input_string, std::string const& output_string, auto& hash_table){
	std::stringstream input(std::move(input_string));
	std::stringstream output_expected(output_string);

	std::string output;
	int iter_count;
	input >> iter_count;
	std::string cmd;

	int key, value;
	for (int i = 0; i != iter_count; ++i){

		input >> cmd >> key;

		if (key == 79477009) {
			std::cerr
			<< "\tht size: " << hash_table.size()
			<< "; accessHelper size: " << hash_table.access.accessHelper.size()
			<< "\tPrinting AccessHelper:\n";
			for (std::size_t j = 0; j != hash_table.access.accessHelper.size(); ++j){
				if (hash_table.access.accessHelper[j].has_value()) {
					std::cerr << "\t\tpos: " << j  << " key: ";
					std::cerr << (hash_table.access.accessHelper[j].value())->first << '\n';
				}
			}
		}

		if (cmd == "put") {
			input >> value;
			hash_table[key] = value;
		}
		else if (cmd == "get") {
			std::string output_line;
			auto found = hash_table.find(key);
			if (found == hash_table.end()) {
				output_line.append("None");
			}
			else {
				output_line.append(std::to_string(found->second));
			}
			output.append(output_line);
			output.append("\n");

			std::string expected_line;
			output_expected >> expected_line;
			if (output_line != expected_line) {
				log_error(output_line, expected_line, hash_table);
				return output;
			}
		}
		else if (cmd == "delete") {
			std::string output_line;
			auto found = hash_table.find(key);
			if (found == hash_table.end()) {
				output_line.append("None");
			}
			else {
				output_line.append(std::to_string(found->second));
			}
			output.append(output_line);
			output.append("\n");

			std::string expected_line;
			output_expected >> expected_line;
			if (output_line != expected_line) {
				log_error(output_line, expected_line, hash_table);
				return output;
			}
			hash_table.erase(found);
		}
	}

	return output;
};


TEST (dummy, dummy){
	std::string input{R"(427
	get -770587462
	put -417784488 -375047901
	delete -417784488
	delete -349338279
	delete 469379452
	get -154300281
	get -434102758
	put 870058585 -231130688
	get 870058585
	put 183977892 -645776324
	get 399511429
	put 277375817 -940326848
	get -411472110
	delete 183977892
	get 870058585
	get 277375817
	delete 604178333
	put 870058585 -28046951
	delete 870058585
	get 277375817
	get 756916525
	get 277375817
	delete -652718421
	get 473598345
	get 277375817
	delete 277375817
	delete -335300958
	delete -12531105
	get -103009723
	delete -449431436
	get -153724358
	delete -35274590
	get -857302278
	put 50235927 227173483
	delete 50235927
	get -91377434
	put -253769807 11558923
	put -253769807 138331360
	delete -908097326
	delete -253769807
	get -789910966
	put -586845099 -945916124
	put 258519532 120068394
	get 258519532
	get -686035463
	put -506002908 874712337
	get 258519532
	put -553641090 -374277320
	get -506002908
	get 258519532
	delete -553641090
	put -166673150 47891389
	get -586845099
	delete -166673150
	put -586845099 733342451
	get -721042485
	get 486579926
	delete -444412711
	delete -586845099
	get 685977994
	delete -506002908
	delete 258519532
	get -522584727
	get -685424747
	get -685265364
	get 434678081
	put 389317792 278723592
	delete 389317792
	put 5856858 9013616
	put 5856858 -867059684
	get 5856858
	get 5856858
	get 5856858
	put 704621729 -903649646
	get -185798884
	delete 897692163
	get 704621729
	put 5856858 -467053062
	get 5856858
	put 5856858 934815392
	get 836648244
	delete 704621729
	put 5856858 -937167872
	put 5856858 216724220
	get 5856858
	get 5856858
	delete 5856858
	delete -270060212
	delete 705822846
	delete -462909707
	get -571477378
	delete -191242119
	delete -475003033
	get 748108381
	get 632513585
	get 777467947
	put -580949329 825050733
	get -470390224
	get -580949329
	put 449943943 663417429
	get 449943943
	get -580949329
	put -580949329 -10246971
	delete -580949329
	put 293777839 15696785
	put 620415532 -282783201
	put 620415532 -893023286
	delete 309395964
	put 936914428 21026110
	get 449943943
	get 118893006
	delete 620415532
	put 936914428 -207577504
	get 936914428
	get 713124316
	get 89120165
	get 881157827
	delete 293777839
	get 936914428
	put 449943943 -279912649
	get 936914428
	put -913775926 870012380
	put 449943943 -573270277
	get -754498363
	delete 936914428
	get -171135386
	get 449943943
	get 269815068
	get -913775926
	get 449943943
	get -913775926
	get 653827263
	get 449943943
	delete -713187658
	put -913775926 -171271058
	put 449943943 -737961014
	put -913775926 -645068713
	get -913775926
	get -913775926
	put -913775926 -968065391
	delete 148947792
	get -475990591
	put 449943943 -84990416
	get -616909661
	delete -913775926
	delete 198467356
	delete 643099842
	put 449943943 -535011439
	get 495350263
	get 449943943
	get -691876579
	get 449943943
	get -508698297
	put 589147811 -925895934
	get 449943943
	get 449943943
	put 738537818 753505755
	delete -993751616
	put 738537818 47558065
	get 449943943
	delete 449943943
	put 527219394 -570399104
	put 527219394 980402949
	delete 60171543
	put 26491620 -89138885
	put 527219394 796389429
	get 589147811
	get 589147811
	delete -594489503
	delete 26491620
	delete 738537818
	get -142588169
	put 589147811 -798989379
	delete 589147811
	put 527219394 -227126516
	put 527219394 -167676979
	get 527219394
	put -727596649 -738278430
	get -727596649
	get 11389770
	delete -702906762
	get 527219394
	delete 527219394
	put -548419445 -725294866
	put -295422542 -862426639
	get -727596649
	get 560219486
	get -727596649
	get -548419445
	put -279205751 224658330
	delete -279205751
	delete -727596649
	get 936291732
	delete -796830808
	get 333392838
	get -548419445
	get -295422542
	get 168336854
	get -295422542
	put -391297907 753781468
	get -295422542
	get -391297907
	delete -548419445
	delete -414004835
	get -911895481
	put -391297907 62782627
	delete -610261804
	delete -391297907
	get -295422542
	get -234100821
	get -295422542
	get -295422542
	get -295422542
	delete -295422542
	delete 578797404
	get 230562571
	delete -291065710
	get -588765005
	delete -717197702
	get -141522678
	get -355494863
	put 873001603 348411035
	put 277119720 359060976
	put 277119720 -41921137
	delete -907919668
	get -162094837
	put 277119720 -745685261
	put 873001603 384956018
	put 873001603 866996113
	get 787223049
	delete 277119720
	put -396911999 186291587
	put 873001603 204855726
	put -396911999 445482918
	get 873001603
	put -396911999 -998428981
	put -396911999 847580157
	get 873001603
	get -421763289
	delete 616338548
	delete 873001603
	delete -396911999
	get -328477874
	delete -779959104
	get -311877751
	delete -499010253
	delete -634277783
	get -38875787
	put 991564910 839107814
	delete 991564910
	delete -77241814
	put -407521438 706375
	put -407521438 563092204
	delete -24982465
	delete -407521438
	get -409347888
	get 927961927
	delete 888818755
	get -383065889
	get -767094995
	get -926381030
	get 545558600
	get 241152215
	get 269027967
	get 753544735
	get 179387026
	get 40382229
	get 234996281
	get 480808441
	delete -917366148
	put 872529898 -730389723
	delete 220423338
	get 872529898
	get 61488156
	get 581584214
	put 872529898 -651792359
	put 924249098 -432338004
	get 924249098
	get 924249098
	get 924249098
	delete -590713756
	get 924249098
	get 872529898
	delete 924249098
	get 872529898
	put -832478070 -104319407
	get 872529898
	put -832478070 272100267
	get -832478070
	delete -873705838
	delete -870563404
	get -925997856
	put 237183358 690940
	get -439212011
	put 442552548 186518025
	get -306483069
	get 872529898
	delete 442552548
	get -832478070
	delete 237183358
	put 872529898 -861320997
	get 872529898
	get -832478070
	delete 872529898
	get 159499376
	delete -832478070
	get -862216032
	get -901136814
	get -222796393
	get -501454573
	get 354419882
	get 925911114
	get -523317285
	get 219822732
	put -268544723 267327675
	get -268544723
	delete -876992024
	put -140894386 568141815
	delete -268544723
	put -140894386 -956166079
	get -140894386
	put -140894386 -313851326
	get -100918184
	get -140894386
	get -140894386
	get -874567412
	get -785505659
	get -140894386
	put -140894386 344027981
	put -140894386 333036859
	delete -140894386
	delete -825019805
	get 32118719
	put -848987599 -763771130
	get -770948347
	get 845293171
	delete 10224247
	delete -848987599
	get 540889362
	delete 742813273
	get 569726705
	get -166071039
	get -455433004
	delete -48835097
	put -231128839 728399261
	delete -231128839
	get 752092286
	put 584229336 168244189
	get 229795264
	put 584229336 -172357937
	put 584229336 -21488205
	get -378043333
	put 584229336 -167058079
	get 584229336
	put 727528621 -822690886
	get 727528621
	put -473326088 657962982
	put -2369855 873361310
	delete 727528621
	delete 848607653
	get 584229336
	get -2369855
	delete -2369855
	get -105214355
	get 584229336
	delete -473326088
	get 584229336
	delete 584229336
	get 48487100
	delete -236222369
	get 395451584
	get 263319283
	get -747260258
	delete 567335959
	delete 616473618
	get -622982295
	get -651435918
	put -856913934 584280306
	put -856913934 379315050
	put -856913934 994033218
	get 575916816
	get -22706521
	get -856913934
	put -856913934 918280754
	put -856913934 809562437
	get -856913934
	get -856913934
	delete -856913934
	get -827417550
	delete 325562573
	delete 143310805
	put 422520138 498579295
	get 791388495
	put 747754828 -691571164
	get 747754828
	put 501565033 -817373313
	delete 747754828
	get 422520138
	get 422520138
	put 422520138 589004895
	get 857951860
	put 144941251 3964563
	put 422520138 -150209098
	get 422520138
	put -780131587 600879090
	get 501565033
	put 501565033 -531960774
	put 144941251 -146134808
	put 79477009 -875104251
	put 144941251 715679198
	get 79477009
	put 79477009 -613979734
	put 79477009 550054007
	get 42001169
	get -780131587
	delete -912238082
	delete 144941251
	put 422520138 81721041
	get 422520138
	put 422520138 -10063291
	get 422520138
	get 501565033
	put -277462819 782925174
	delete 422520138
	delete -277462819
	delete -780131587
	delete 79477009
)"},
	expected_output {R"(None
-375047901
None
None
None
None
-231130688
None
None
-645776324
-231130688
-940326848
None
-28046951
-940326848
None
-940326848
None
None
-940326848
-940326848
None
None
None
None
None
None
None
227173483
None
None
138331360
None
120068394
None
120068394
874712337
120068394
-374277320
-945916124
47891389
None
None
None
733342451
None
874712337
120068394
None
None
None
None
278723592
-867059684
-867059684
-867059684
None
None
-903649646
-467053062
None
-903649646
216724220
216724220
216724220
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
825050733
663417429
825050733
-10246971
None
663417429
None
-893023286
-207577504
None
None
None
15696785
-207577504
-207577504
None
-207577504
None
-573270277
None
870012380
-573270277
870012380
None
-573270277
None
-645068713
-645068713
None
None
None
-968065391
None
None
None
-535011439
None
-535011439
None
-535011439
-535011439
None
-535011439
-535011439
None
-925895934
-925895934
None
-89138885
47558065
None
-798989379
-167676979
-738278430
None
None
-167676979
-167676979
-738278430
None
-738278430
-725294866
224658330
-738278430
None
None
None
-725294866
-862426639
None
-862426639
-862426639
753781468
-725294866
None
None
None
62782627
-862426639
None
-862426639
-862426639
-862426639
-862426639
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
-745685261
204855726
204855726
None
None
204855726
847580157
None
None
None
None
None
None
839107814
None
None
563092204
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
-730389723
None
None
-432338004
-432338004
-432338004
None
-432338004
-651792359
-432338004
-651792359
-651792359
272100267
None
None
None
None
None
-651792359
186518025
272100267
690940
-861320997
272100267
-861320997
None
272100267
None
None
None
None
None
None
None
None
267327675
None
267327675
-956166079
None
-313851326
-313851326
None
None
-313851326
333036859
None
None
None
None
None
-763771130
None
None
None
None
None
None
728399261
None
None
None
-167058079
-822690886
-822690886
None
-167058079
873361310
873361310
None
-167058079
657962982
-167058079
-167058079
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
994033218
809562437
809562437
809562437
None
None
None
None
-691571164
-691571164
498579295
498579295
None
-150209098
-817373313
-875104251
None
600879090
None
715679198
81721041
-10063291
-531960774
-10063291
782925174
600879090
550054007
)"};
	containers::hash_table::Map<int, int> ht;
	ASSERT_EQ(user_scenario_run(input, expected_output, ht), expected_output);

}

TEST (dummy, dummy_1) {
	containers::hash_table::Map<int, int> hash_table (20);
	//to 7
	hash_table.insert(std::pair{7, 1});
	//to 9
	hash_table.insert(std::pair{9, 1});
	//to 16
	hash_table.insert(std::pair{16, 1});
	//to 18
	hash_table.insert(std::pair{29, 1});
	//should've gone to 7 but occupied, so to 16 but occupied, so to 5
	hash_table.insert(std::pair{49, 1});
	//should've gone to 7 but occupied, so to 16 but occupied, so to 5 but occupied, so to 14
	hash_table.insert(std::pair{69, 1});
	//should've gone to 7 but occupied, so to 16 but occupied, so to 5 but occupied, so to 14 but occupied, so to 3
	hash_table.insert(std::pair{89, 1});

	std::cerr
			<< "\tht size: " << hash_table.size()
			<< "; accessHelper size: " << hash_table.access.accessHelper.size()
			<< "\tPrinting AccessHelper:\n";
	for (std::size_t j = 0; j != hash_table.access.accessHelper.size(); ++j){
		if (hash_table.access.accessHelper[j].has_value()) {
			std::cerr << "\t\tpos: " << j
			<< " key: " << (hash_table.access.accessHelper[j].value())->first << '\n';
		}
	}

	hash_table.erase(9);

	std::cerr
			<< "\tht size: " << hash_table.size()
			<< "; accessHelper size: " << hash_table.access.accessHelper.size()
			<< "\tPrinting AccessHelper:\n";
	for (std::size_t j = 0; j != hash_table.access.accessHelper.size(); ++j){
		if (hash_table.access.accessHelper[j].has_value()) {
			std::cerr << "\t\tpos: " << j
			<< " key: " << (hash_table.access.accessHelper[j].value())->first << '\n';
		}
	}

	ASSERT_TRUE(true);
}


TEST (dummy, dummy_2) {
	containers::hash_table::Map<int, int> hash_table (20);

	hash_table.insert(std::pair{18, 1});
	hash_table.insert(std::pair{9, 1});
	hash_table.insert(std::pair{29, 1});

	std::cerr
			<< "\tht size: " << hash_table.size()
			<< "; accessHelper size: " << hash_table.access.accessHelper.size()
			<< "\tPrinting AccessHelper:\n";
	for (std::size_t j = 0; j != hash_table.access.accessHelper.size(); ++j){
		if (hash_table.access.accessHelper[j].has_value()) {
			std::cerr << "\t\tpos: " << j
			          << " key: " << (hash_table.access.accessHelper[j].value())->first << '\n';
		}
	}

	hash_table.erase(18);

	auto found = hash_table.find(29);
	ASSERT_TRUE(found != hash_table.end());
}


int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
	testing::InitGoogleTest(&argc, argv);
	testing::GTEST_FLAG(color) = "yes";
//	::testing::GTEST_FLAG(filter) = "hash_table_set.hash_values_collision*";
//	::testing::GTEST_FLAG(filter) = "hash_table_set.*";
//	::testing::GTEST_FLAG(filter) = "hash_table_map.*";
//	::testing::GTEST_FLAG(filter) = "dummy.*";
	::testing::GTEST_FLAG(filter) = "dummy.dummy_2";
	auto res {RUN_ALL_TESTS()};
	return res;
}

//todo
// add initializer list to insert
// remove access from public
// add prime numbers or power of two as a capacity driver
// remove hash_table:: namespace from ::containers::hash_table::Set ?
