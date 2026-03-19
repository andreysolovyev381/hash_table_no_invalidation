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
static constexpr int rep_count {5};

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> distrib(1, iter_count);

auto insertion = [](int value, auto &ht){
	int key {distrib(gen)};
	if constexpr (requires { typename std::decay_t<decltype(ht)>::mapped_type; }) {
		if constexpr (std::is_same_v<std::decay_t<decltype(ht)>, containers::hash_table::Map<int,int>>) {
			ht.insert(key, distrib(gen));
		}
		else {
			ht.insert({key, distrib(gen)});
		}
	} else {
		ht.insert(key);
	}
	value += key;
	return value / ht.size();
};

template <typename HashTable>
static void Insertion(benchmark::State& state) {
	HashTable ht(iter_count);
	int res {1};
	for (auto _ : state) {
		res = insertion(res, ht);
		benchmark::DoNotOptimize(res);
		benchmark::ClobberMemory();
	}
	state.SetItemsProcessed(state.iterations());
	state.counters["ns/op"] = benchmark::Counter(
		state.iterations(),
		benchmark::Counter::kIsRate | benchmark::Counter::kInvert
	);
#ifndef __APPLE__
	state.counters["cycles"] = benchmark::Counter::kDefaults;
#endif
}

auto accessTo = [](auto &ht){
	int key {distrib(gen)};
	auto found = ht.find(key);
	if (found == ht.end()) return -1;
	if constexpr (requires { typename std::decay_t<decltype(ht)>::mapped_type; }) {
		return found->second;
	} else {
		return *found;
	}
};

template <typename HashTable>
static void Access(benchmark::State& state) {
	HashTable ht(iter_count);
	int res {1};
	for (int i = 0; i != iter_count; ++i){
		insertion(res, ht);
	}
	for (auto _ : state) {
		res = accessTo(ht);
		benchmark::DoNotOptimize(res);
		benchmark::ClobberMemory();
	}
	state.SetItemsProcessed(state.iterations());
	state.counters["ns/op"] = benchmark::Counter(
		state.iterations(),
		benchmark::Counter::kIsRate | benchmark::Counter::kInvert
	);
#ifndef __APPLE__
	state.counters["cycles"] = benchmark::Counter::kDefaults;
#endif
}

auto eraseFrom = [](auto &ht){
	int key {distrib(gen)};
	auto found = ht.find(key);
	int v {-1};
	if (found != ht.end()) {
		if constexpr (requires { typename std::decay_t<decltype(ht)>::mapped_type; }) {
			v = found->second;
		} else {
			v = *found;
		}
		ht.erase(found);
	}
	return v;
};

template <typename HashTable>
static void Erase(benchmark::State& state) {
	HashTable ht(iter_count);
	int res {1};
	for (int i = 0; i != iter_count; ++i){
		insertion(res, ht);
	}
	for (auto _ : state) {
		res = eraseFrom(ht);
		benchmark::DoNotOptimize(res);
		benchmark::ClobberMemory();
	}
	state.SetItemsProcessed(state.iterations());
	state.counters["ns/op"] = benchmark::Counter(
		state.iterations(),
		benchmark::Counter::kIsRate | benchmark::Counter::kInvert
	);
#ifndef __APPLE__
	state.counters["cycles"] = benchmark::Counter::kDefaults;
#endif
}

#define BENCHMARK_HT_PARAMS(NiceName)        \
->Repetitions(rep_count)                     \
->Unit(benchmark::kMicrosecond)              \
->DisplayAggregatesOnly(true)                \
->MinTime(test_time)                         \
->Name(NiceName)                             \
;

BENCHMARK_TEMPLATE(Insertion, std::unordered_set<int>)                 BENCHMARK_HT_PARAMS("Insertion std::unordered_set          ")
BENCHMARK_TEMPLATE(Insertion, containers::hash_table::Set<int>)        BENCHMARK_HT_PARAMS("Insertion containers::hash_table::Set ")
BENCHMARK_TEMPLATE(Insertion, std::unordered_map<int,int>)             BENCHMARK_HT_PARAMS("Insertion std::unordered_map          ")
BENCHMARK_TEMPLATE(Insertion, containers::hash_table::Map<int,int>)    BENCHMARK_HT_PARAMS("Insertion containers::hash_table::Map ")

BENCHMARK_TEMPLATE(Access, std::unordered_set<int>)                    BENCHMARK_HT_PARAMS("Access std::unordered_set             ")
BENCHMARK_TEMPLATE(Access, containers::hash_table::Set<int>)           BENCHMARK_HT_PARAMS("Access containers::hash_table::Set    ")
BENCHMARK_TEMPLATE(Access, std::unordered_map<int,int>)                BENCHMARK_HT_PARAMS("Access std::unordered_map             ")
BENCHMARK_TEMPLATE(Access, containers::hash_table::Map<int,int>)       BENCHMARK_HT_PARAMS("Access containers::hash_table::Map    ")

BENCHMARK_TEMPLATE(Erase, std::unordered_set<int>)                     BENCHMARK_HT_PARAMS("Erase std::unordered_set              ")
BENCHMARK_TEMPLATE(Erase, containers::hash_table::Set<int>)            BENCHMARK_HT_PARAMS("Erase containers::hash_table::Set     ")
BENCHMARK_TEMPLATE(Erase, std::unordered_map<int,int>)                 BENCHMARK_HT_PARAMS("Erase std::unordered_map              ")
BENCHMARK_TEMPLATE(Erase, containers::hash_table::Map<int,int>)        BENCHMARK_HT_PARAMS("Erase containers::hash_table::Map     ")


int main(int argc, char** argv) {
	::benchmark::Initialize(&argc, argv);
	::benchmark::RunSpecifiedBenchmarks();
	::benchmark::Shutdown();
	return 0;
}
