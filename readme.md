## Hash table with *NO* invalidation of pointers and iterators

### Rational
Another project of mine required a **hash table that never invalidates** its pointers and iterators. I made one with a humble hope to be on par with `std::unordered`. Header only, single C++20 file, see ./include folder. See ./tests for more detailed usage examples. In general, more or less it reproduces `std::unordered` interface.

### Short usage example
```cpp
#include "include/hash_table.hpp"
...

    ::containers::hash_table::Set<int> hashSet;
    hashSet.insert(42);
    auto found = hashSet.find(42);
    if (found != hashSet.end()) {...}

    ::containers::hash_table::Map<int, int> hashMap;
    hashMap[1] = 42;
    hashMap.insert(std::pair{42, 42});
    for (auto const& [k, v] : hashMap) {...}

```

### Reference to other hash tables
There are many other hash tables, but I found that more or less all of them are focused on performance, sacrificing exactly what I need — persistence of pointers and iterators. However, there are some serious attempts. Consider reading the [results](https://martin.ankerl.com/2019/04/01/hashmap-benchmarks-01-overview/) of benchmark made by people behind `robin_hood::hash_table`. I also recommend this [reading](https://greg7mdp.github.io/parallel-hashmap/) from Greg Popovitch.

### Benchmark
See ./benchmark folder for code. 
Here are the results for 1 million ints I got **at my lower end M1 from early 2022**:

#### Insertion

| Metric | `std::unordered_set` | `hash_table::Set` | `std::unordered_map` | `hash_table::Map` |
|--------|----------------------|--------------------|----------------------|--------------------|
| Mean   | 29.76 ns             | 33.55 ns           | 46.07 ns             | 45.84 ns           |
| Median | 28.01 ns             | 33.78 ns           | 46.01 ns             | 45.82 ns           |
| Stddev | 3.29 ns              | 0.44 ns            | 0.79 ns              | 0.25 ns            |
| CV     | 11.05%               | 1.31%              | 1.72%                | 0.54%              |
| Throughput | 33.92 M/s        | 29.81 M/s          | 21.71 M/s            | 21.82 M/s          |

#### Access

| Metric | `std::unordered_set` | `hash_table::Set` | `std::unordered_map` | `hash_table::Map` |
|--------|----------------------|--------------------|----------------------|--------------------|
| Mean   | 25.12 ns             | 26.78 ns           | 25.10 ns             | 27.12 ns           |
| Median | 25.08 ns             | 26.74 ns           | 25.10 ns             | 26.95 ns           |
| Stddev | 0.31 ns              | 0.14 ns            | 0.23 ns              | 0.46 ns            |
| CV     | 1.23%                | 0.53%              | 0.93%                | 1.71%              |
| Throughput | 39.81 M/s        | 37.34 M/s          | 39.85 M/s            | 36.88 M/s          |

#### Erase

| Metric | `std::unordered_set` | `hash_table::Set` | `std::unordered_map` | `hash_table::Map` |
|--------|----------------------|--------------------|----------------------|--------------------|
| Mean   | 3.31 ns              | 16.16 ns           | 3.32 ns              | 16.43 ns           |
| Median | 3.30 ns              | 16.18 ns           | 3.31 ns              | 16.42 ns           |
| Stddev | 0.02 ns              | 0.76 ns            | 0.02 ns              | 0.56 ns            |
| CV     | 0.68%                | 4.70%              | 0.74%                | 3.42%              |
| Throughput | 302.23 M/s       | 62.00 M/s          | 301.47 M/s           | 60.92 M/s          |


### Implementation details
* Open addressing and linear probing as a collision resolution, in case anybody cares. 

* It is allowed to throw, you are the one who should catch. 

* Indeed, to nail down all the data, hash table should use a linked list as an underlying structure. The problem is that random memory placement turns out to be bad for cache locality. 
But thanks to [Bloomberg](https://github.com/bloomberg) and their contribution to committee work, we have `std:pmr` namespace and polymorphic allocators.
Long story short, this hash table is built upon `std::pmr::list` that allows to place list nodes in memory in an array-like fashion, thus making such a container more cache-friendly. One can see the root source of everything, [Pablo Halpern](https://github.com/phalpern)'s CppCon2017 [report](https://www.youtube.com/watch?v=v3dz-AKOVL8). 

* My hypothesis is that after some insert / remove cycles this hash table will deteriorate in its performance — "*pogrom is a pogrom*", list is a list, appearance of "holes" in that initial array-like placement is inevitable.

### License
MIT

### Disclosure
Despite heavy testing performed (see ./tests/test_11.txt), no guarantees of any kind are given whatsoever. Use it at your own risk.
