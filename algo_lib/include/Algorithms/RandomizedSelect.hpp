#ifndef RANDOMIZEDSELECT_HPP
#define RANDOMIZEDSELECT_HPP

#include <vector>
#include <random>
#include <stdexcept>

template<typename T>
class RandomizedSelect {
public:
    T select(std::vector<T>& data, std::size_t k);

private:
    std::mt19937 gen{std::random_device{}()};
    std::size_t randomizedPartition(std::vector<T>& arr, std::size_t left, std::size_t right);
    T randomizedSelect(std::vector<T>& arr, std::size_t left, std::size_t right, std::size_t k);
};

template<typename T>
T RandomizedSelect<T>::select(std::vector<T>& data, std::size_t k) {
    if (data.empty() || k >= data.size()) {
        throw std::out_of_range("Invalid selection index");
    }
    return randomizedSelect(data, 0, data.size() - 1, k);
}

template<typename T>
std::size_t RandomizedSelect<T>::randomizedPartition(std::vector<T>& arr, std::size_t left, std::size_t right) {
    std::uniform_int_distribution<std::size_t> dist(left, right);
    std::size_t pivotIndex = dist(gen);

    std::swap(arr[pivotIndex], arr[right]);

    T pivotValue = arr[right];
    std::size_t storeIndex = left;

    for (std::size_t i = left; i < right; ++i) {
        if (arr[i] < pivotValue) {
            std::swap(arr[storeIndex], arr[i]);
            storeIndex++;
        }
    }

    std::swap(arr[right], arr[storeIndex]);
    return storeIndex;
}

template<typename T>
T RandomizedSelect<T>::randomizedSelect(std::vector<T>& arr, std::size_t left, std::size_t right, std::size_t k) {
    if (left == right) {
        return arr[left];
    }

    std::size_t pivotNewIndex = randomizedPartition(arr, left, right);

    if (k == pivotNewIndex) {
        return arr[k];
    } else if (k < pivotNewIndex) {
        return randomizedSelect(arr, left, pivotNewIndex - 1, k);
    } else {
        return randomizedSelect(arr, pivotNewIndex + 1, right, k);
    }
}

#endif // RANDOMIZEDSELECT_HPP