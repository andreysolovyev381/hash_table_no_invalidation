//
// Created by Andrey Solovyev on 19.05.2024.
//


#include "../hash_table.hpp"
#include <random>

static constexpr int iter_count {100'000};

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> distrib(1, iter_count);

auto insertion = [](int value, auto &hash_table){
	int elem {distrib(gen)};
	hash_table.insert(elem);
	value += elem;
	return value / hash_table.size();
};
auto accessTo = [](auto & hash_table){
	int key {distrib(gen)};
	auto found = hash_table.find(key);
	return found == hash_table.end() ?  -1 : *found;
};


int main() {

	containers::hash_table::Set<int> ht;
	int elem {0}, sum{0};
	for (int i = 0; i != iter_count; ++i) {
		elem = distrib(gen);
		ht.insert(elem);
		sum += elem;
	}
	std::fprintf(stdout, "%zu", sum / ht.size());

	for (int i = 0; i != iter_count; ++i) {
		sum -= accessTo(ht);
	}
	std::fprintf(stdout, "%zu", sum / ht.size());

	return 0;
}
