//
// Created by Andrey Solovyev on 19.05.2024.
//

#include <benchmark/benchmark.h>

#include "../include/hash_table.hpp"
#include <random>
#include <unordered_set>
#include <unordered_map>

static constexpr int iter_count {1'000'000};
static constexpr int test_time {5};

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> distrib(1, iter_count);

auto insertion = [](int value, auto &hash_table){
	int elem {distrib(gen)};
	hash_table.insert(elem);
	value += elem;
	return value / hash_table.size();
};

template <typename HashTable>
static void Insertion(benchmark::State& state) {
	HashTable ht;
	int res {1};
	for (auto _ : state) {
		res = insertion(res, ht);
		benchmark::DoNotOptimize(res);
	}
}

BENCHMARK_TEMPLATE(Insertion, std::unordered_set<int>)->Name("Insertion std::unordered_set          ")->Unit(benchmark::kMicrosecond)->MinTime(test_time);
BENCHMARK_TEMPLATE(Insertion, containers::hash_table::Set<int>)->Name("Insertion containers::hash_table::Set ")->Unit(benchmark::kMicrosecond)->MinTime(test_time);


auto accessTo = [](auto & hash_table){
	int key {distrib(gen)};
	auto found = hash_table.find(key);
	return found == hash_table.end() ?  -1 : *found;
};

template <typename HashTable>
static void Access(benchmark::State& state) {
	HashTable ht;
	int res {1};
	for (int i = 0; i != iter_count; ++i){
		insertion(res, ht);
	}
	for (auto _ : state) {
		res = accessTo(ht);
		benchmark::DoNotOptimize(res);
	}
}

BENCHMARK_TEMPLATE(Access, std::unordered_set<int>)->Name("Access std::unordered_set             ")->Unit(benchmark::kMicrosecond)->MinTime(test_time);
BENCHMARK_TEMPLATE(Access, containers::hash_table::Set<int>)->Name("Access containers::hash_table::Set    ")->Unit(benchmark::kMicrosecond)->MinTime(test_time);


auto eraseFrom = [](auto & hash_table){
	int key {distrib(gen)};
	auto found = hash_table.find(key);
	int v;
	if (found != hash_table.end()) {
		v = *found;
		hash_table.erase(found);
	}

	return found == hash_table.end() ? -1 : v;
};

template <typename HashTable>
static void Erase(benchmark::State& state) {
	HashTable ht;
	int res {1};
	for (int i = 0; i != iter_count; ++i){
		insertion(res, ht);
	}
	for (auto _ : state) {
		res = eraseFrom(ht);
		benchmark::DoNotOptimize(res);
	}
}

BENCHMARK_TEMPLATE(Erase, std::unordered_set<int>)->Name("Erase std::unordered_set              ")->Unit(benchmark::kMicrosecond)->MinTime(test_time);
BENCHMARK_TEMPLATE(Erase, containers::hash_table::Set<int>)->Name("Erase containers::hash_table::Set     ")->Unit(benchmark::kMicrosecond)->MinTime(test_time);


int main(int argc, char** argv) {
	::benchmark::Initialize(&argc, argv);
	::benchmark::RunSpecifiedBenchmarks();
	::benchmark::Shutdown();
	return 0;
}
