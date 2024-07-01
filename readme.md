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
There are many other hash tables, but I found that all of them are focused on performance, sacrificing exactly what I need — persistence of pointers and iterators. However, there are some serious attempts. Consider reading the [results](https://martin.ankerl.com/2019/04/01/hashmap-benchmarks-01-overview/) of benchmark made by people behind `robin_hood::hash_table`. I also recommend this [reading](https://greg7mdp.github.io/parallel-hashmap/) from Greg Popovitch.

### Benchmark
See ./benchmark folder for code. 
Here are the results for 1 million ints. 
```
------------------------------------------------------------------------------------------------
Benchmark                                                      Time             CPU   Iterations
------------------------------------------------------------------------------------------------
Insertion std::unordered_set          /min_time:5.000      0.144 us        0.143 us     50408238
Insertion containers::hash_table::Set /min_time:5.000      0.129 us        0.128 us     53205182
Access std::unordered_set             /min_time:5.000      0.072 us        0.072 us     96885052
Access containers::hash_table::Set    /min_time:5.000      0.067 us        0.066 us    105449693
Erase std::unordered_set              /min_time:5.000      0.009 us        0.009 us    618141538
Erase containers::hash_table::Set     /min_time:5.000      0.089 us        0.089 us     69120837

```

### Implementation details
* Open addressing and linear probing as a collision resolution, in case anybody cares. 

* It is allowed to throw, you are the one who should catch. 

* Indeed, to nail down all the data, hash table should use a linked list as an underlying structure. The problem is that random memory placement turns out to be bad for cache locality. 
But thanks to [Bloomberg](https://github.com/bloomberg) and their contribution to committee work, we have `std:pmr` namespace and polymorphic allocators.
Long story short, this hash table is built upon `std::pmr::list` that allows to place list nodes in memory in an array-like fashion, thus making such a container more cache-friendly.

  Another issue that comes from usage of `std::pmr` is a strong requirement to follow "rule of five". Specifically, see ./test/pmr.cpp — copy ctor and assignment fail if implemented by default. That is a reason why HashTable is a move-only. Implementation of allocator aware linked list may be a good solution here, ie see the root source of everything, [Pablo Halpern](https://github.com/phalpern)'s CppCon2017 [report](https://www.youtube.com/watch?v=v3dz-AKOVL8). But it is beyond my needs, so feel free to contribute :smirk:

* My hypothesis is that after some insert / remove cycles this hash table will deteriorate in performance — "pogrom is a pogrom", list is a list, appearance of "holes" in that initial array-like placement is inevitable.

### License
MIT

### Disclosure
Despite heavy testing performed (see ./tests/test_11.txt), no guarantees of any kind are given whatsoever. Use it at your own risk.
