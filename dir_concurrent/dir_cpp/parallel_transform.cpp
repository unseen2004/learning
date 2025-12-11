#include <algorithm>
#include <iostream>
#include <vector>
#include <thread>

template<typename iterator, typename F>
void transform(iterator begin, iterator end, F &&f) {
    auto size = std::distance(begin, end);
    if (size < 10000)
        transform(begin, end, begin, std::forward<F>(f));
    else {
        std::vector<std::thread> threads;
        int thread_count = std::thread::hardware_concurrency();
        auto first = begin;
        auto last = begin;
        size /= thread_count;
        for (int i{0}; i < thread_count; ++i) {
            first = last;
            if (i == thread_count - 1)
                last = end;
            else
                std::advance(last, size);

            threads.emplace_back([first, last, &f]() { transform(first, last, first, std::forward<F>(f)); });
            for (auto &t: threads) {
                t.join();
            }
        }
    }
}

template<typename iterator, typename F>
void process(iterator begin, iterator end, F &&f) {
    auto size = std::distance(begin, end);
    if (size < 10000)
        transform(begin, end, begin, std::forward<F>(f));
    else {
        std::vector<std::thread> threads;
        int thread_count = std::thread::hardware_concurrency();
        std::vector<typename std::iterator_traits<iterator>::value_type> mins(thread_count);
        auto first = begin;
        auto last = begin;
        size /= thread_count;
        for (int i{0}; i < thread_count; ++i) {
            first = last;
            if (i == thread_count - 1)
                last = end;
            else
                std::advance(last, size);

            threads.emplace_back([first, last, &f, &r=mins[i]]() {
                r = transfom(first, last, first, std::forward<F>(f));
            });
            for (auto &t: threads) {
                t.join();
            }
            return std::forward<F>(f)(std::begin(mins), std::end(mins));
        }
    }
}
