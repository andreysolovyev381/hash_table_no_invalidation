//
// Created by Andrey Solovyev on 19.05.2024.
//

#include <benchmark/benchmark.h>

#include <algorithm>
#include <list>
#include <random>


static constexpr std::size_t cache_line_size {64u};
static constexpr std::size_t arbitrary_alignment {24u};
static constexpr int iter_count {100'000};
static constexpr int bound {50'000};


static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> distrib(1, iter_count);

static void RegularList(benchmark::State& state) {
	std::list<int> l;
	std::vector<std::vector<int>> vs;

	for (int i = 0; i != iter_count; ++i){
		vs.emplace_back(std::vector<int> {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17});
		l.push_back(distrib(gen));
	}

	for (auto _ : state) {
		auto count = std::count_if(l.begin(), l.end(), [](const auto elem){ return elem > bound; });
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(RegularList)->Name("Regular List")->Unit(benchmark::kMicrosecond)->MinTime(5);

static void PmrList1(benchmark::State& state) {
	std::pmr::unsynchronized_pool_resource upstream_resource(std::pmr::get_default_resource());
	memory::AlignedMemoryResource<arbitrary_alignment> aligned_resource(&upstream_resource);
	using allocator_type = std::pmr::polymorphic_allocator<std::byte>;
	std::pmr::list<int> l(allocator_type{&aligned_resource});
	std::vector<std::vector<int>> vs;

	for (int i = 0; i != iter_count; ++i){
		vs.emplace_back(std::vector<int> {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17});
		l.push_back(distrib(gen));
	}

	for (auto _ : state) {
		auto count = std::count_if(l.begin(), l.end(), [](const auto elem){ return elem > bound; });
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(PmrList1)->Name("Pmr List, using Aligned Memory Resource")->Unit(benchmark::kMicrosecond)->MinTime(5);


static void PmrList2(benchmark::State& state) {
	std::pmr::unsynchronized_pool_resource resource(std::pmr::get_default_resource());
	using allocator_type = std::pmr::polymorphic_allocator<std::byte>;
	std::pmr::list<int> l(allocator_type{&resource});
	std::vector<std::vector<int>> vs;

	for (int i = 0; i != iter_count; ++i){
		vs.emplace_back(std::vector<int> {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17});
		l.push_back(distrib(gen));
	}

	for (auto _ : state) {
		auto count = std::count_if(l.begin(), l.end(), [](const auto elem){ return elem > bound; });
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(PmrList2)->Name("Pmr List, using Standard Memory Resource")->Unit(benchmark::kMicrosecond)->MinTime(5);


BENCHMARK_MAIN();
