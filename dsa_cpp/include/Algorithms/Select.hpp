#ifndef SELECT_HPP
#define SELECT_HPP

#include <vector>
#include <algorithm>
#include <stdexcept>

template<typename T>
class Select {
public:
    T select(std::vector<T>& data, std::size_t k);

private:
    std::size_t partition(std::vector<T>& arr, std::size_t left, std::size_t right, std::size_t pivotIndex);
    std::size_t medianOfMedians(std::vector<T>& arr, std::size_t left, std::size_t right);
    T deterministic_select(std::vector<T>& arr, std::size_t left, std::size_t right, std::size_t k);
};

template<typename T>
T Select<T>::select(std::vector<T>& data, std::size_t k) {
    if (data.empty() || k >= data.size()) {
        throw std::out_of_range("Invalid selection index");
    }
    return deterministic_select(data, 0, data.size() - 1, k);
}

template<typename T>
std::size_t Select<T>::partition(std::vector<T>& arr, std::size_t left, std::size_t right, std::size_t pivotIndex) {
    T pivotValue = arr[pivotIndex];
    std::swap(arr[pivotIndex], arr[right]);

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
std::size_t Select<T>::medianOfMedians(std::vector<T>& arr, std::size_t left, std::size_t right) {
    if (right - left < 5) {
        std::sort(arr.begin() + left, arr.begin() + right + 1);
        return left + (right - left) / 2;
    }

    std::size_t numMedians = 0;
    for (std::size_t i = left; i <= right; i += 5) {
        std::size_t subRight = std::min(right, i + 4);
        std::sort(arr.begin() + i, arr.begin() + subRight + 1);
        std::size_t medianIndex = i + (subRight - i) / 2;
        std::swap(arr[left + numMedians], arr[medianIndex]);
        ++numMedians;
    }

    T medianValue = deterministic_select(arr, left, left + numMedians - 1, numMedians / 2);

    for (std::size_t i = left; i < left + numMedians; ++i) {
        if (arr[i] == medianValue) {
            return i;
        }
    }

    return left + numMedians / 2;
}

template<typename T>
T Select<T>::deterministic_select(std::vector<T>& arr, std::size_t left, std::size_t right, std::size_t k) {
    if (left == right) {
        return arr[left];
    }

    std::size_t pivotIndex = medianOfMedians(arr, left, right);
    std::size_t pivotNewIndex = partition(arr, left, right, pivotIndex);

    if (k == pivotNewIndex) {
        return arr[k];
    } else if (k < pivotNewIndex) {
        return deterministic_select(arr, left, pivotNewIndex - 1, k);
    } else {
        return deterministic_select(arr, pivotNewIndex + 1, right, k);
    }
}

#endif // SELECT_HPP