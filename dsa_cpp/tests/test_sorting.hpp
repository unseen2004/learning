#ifndef TEST_SORTING_HPP
#define TEST_SORTING_HPP

#include <vector>
#include <iostream>
#include "../include/Algorithms/Sort/BucketSort.hpp"
#include "../include/Algorithms/Sort/InsertionSort.hpp"
#include "../include/Algorithms/Sort/QuickSort.hpp"
#include "../include/Algorithms/Sort/MergeSort.hpp"
#include "../include/Algorithms/Sort/TwoWayMergeSort.hpp"

inline void testBucketSort() {
    std::vector<int> data = {4, 2, 7, 1, 9, 3};
    BucketSort<int> sorter;
    sorter.sort(data);
    for (int i = 1; i < data.size(); ++i) {
        if (data[i - 1] > data[i]) {
            std::cerr << "BucketSort test failed!" << std::endl;
            return;
        }
    }
    std::cout << "BucketSort test passed!" << std::endl;
}

inline void testInsertionSort() {
    std::vector<int> data = {4, 2, 7, 1, 9, 3};
    InsertionSort<int> sorter;
    sorter.sort(data);
    for (int i = 1; i < data.size(); ++i) {
        if (data[i - 1] > data[i]) {
            std::cerr << "InsertionSort test failed!" << std::endl;
            return;
        }
    }
    std::cout << "InsertionSort test passed!" << std::endl;
}

inline void testQuickSort() {
    std::vector<int> data = {4, 2, 7, 1, 9, 3};
    QuickSort<int> sorter;
    sorter.sort(data);
    for (int i = 1; i < data.size(); ++i) {
        if (data[i - 1] > data[i]) {
            std::cerr << "QuickSort test failed!" << std::endl;
            return;
        }
    }
    std::cout << "QuickSort test passed!" << std::endl;
}

inline void testMergeSort() {
    std::vector<int> data = {4, 2, 7, 1, 9, 3};
    MergeSort<int> sorter;
    sorter.sort(data);
    for (int i = 1; i < data.size(); ++i) {
        if (data[i - 1] > data[i]) {
            std::cerr << "MergeSort test failed!" << std::endl;
            return;
        }
    }
    std::cout << "MergeSort test passed!" << std::endl;
}

inline void testTwoWayMergeSort() {
    std::vector<int> data = {4, 2, 7, 1, 9, 3};
    TwoWayMergeSort<int>::sort(data);
    for (int i = 1; i < data.size(); ++i) {
        if (data[i - 1] > data[i]) {
            std::cerr << "TwoWayMergeSort test failed!" << std::endl;
            return;
        }
    }
    std::cout << "TwoWayMergeSort test passed!" << std::endl;
}

inline void runSortingTests() {
    testBucketSort();
    testInsertionSort();
    testQuickSort();
    testMergeSort();
    testTwoWayMergeSort();
}

#endif // TEST_SORTING_HPP