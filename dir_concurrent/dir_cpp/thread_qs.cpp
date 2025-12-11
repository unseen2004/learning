#include <future>
#include <algorithm>

template<typename T>
T partition(T first, T last) {
    auto pivot = *first;
    auto i = first + 1;
    auto j = last - 1;
    while (i <= j) {
        while (i <= j && *i <= pivot) ++i;
        while (i <= j && *j > pivot) --j;
        if (i < j) std::iter_swap(i, j);
    }
    std::iter_swap(i - 1, first);
    return i - 1;
}

template<typename T>
void qs(T first, T last) {
    if (first < last) {
        auto p = partition(first, last);
        if (last - first <= 10000) // can be changed for better optimization
        {
            qs(first, p);
            qs(p + 1, last);
        } else {
            auto t1 = std::async(std::launch::async, [first, p]() { qs(first, p); });
            auto t2 = std::async(std::launch::async, [p, last]() { qs(p + 1, last); });
            t1.wait();
            t2.wait();
        }
    }
}
