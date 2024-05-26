## Hash table with no invalidation of pointers and iterators

### Rational
Another project of mine required a hash table that never invalidates its pointers and iterators. I made one in the hope that it would be on par with `std::unordered`. Header only, single C++20 file, see ./include folder.


### Reference to other hash tables
There are many other hash tables, but I found that all of them are focused on performance, sacrificing exactly what I need—persistence of pointers and iterators. However, there are some serious attempts. Consider reading the [results](https://martin.ankerl.com/2019/04/01/hashmap-benchmarks-01-overview/) of benchmark made by people behind `robin_hood::hash_table`. I also recommend this [reading](https://greg7mdp.github.io/parallel-hashmap/) from Greg Popovic.

### Benchmark
See "benchmark" folder for code. 
Here are the results for 1 million ints. 
```bash
------------------------------------------------------------------------------------------------
Benchmark                                                      Time             CPU   Iterations
------------------------------------------------------------------------------------------------
Insertion std::unordered_set          /min_time:5.000      0.159 us        0.158 us     43200594
Insertion containers::hash_table::Set /min_time:5.000      0.131 us        0.131 us     48726121
Access std::unordered_set             /min_time:5.000      0.100 us        0.100 us     70115314
Access containers::hash_table::Set    /min_time:5.000      0.066 us        0.066 us    105866092
Erase std::unordered_set              /min_time:5.000      0.009 us        0.009 us    595622129
Erase containers::hash_table::Set     /min_time:5.000      0.033 us        0.032 us    211234684

```

### Implementation details
Indeed, to nail down all the data, one should use a linked list. The problem is that random memory placement turns out to be bad for cache locality.

But thanks to [Bloomberg](https://github.com/bloomberg) and their contribution to committee work, we have `std:pmr` namespace and polymorphic allocators.
Long story short, this hash table is built upon `std::pmr::list` that allows to place list nodes in memory in an array-like fashion, thus making such a container more cache-friendly.

My hypothesis is that after some insert / remove cycles this hash table will deteriorate in performance -- "pogrom is a pogrom", list is a list, appearance of "holes" in that initial array-like placement is inevitable.

### License
MIT