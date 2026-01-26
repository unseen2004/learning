#ifndef SELECT_VARIANT_HPP
#define SELECT_VARIANT_HPP

#include <vector>
#include <algorithm>
#include <stdexcept>

template<typename T>
class SelectVariant {
public:
    T select(std::vector<T>& data, std::size_t k, std::size_t groupSize = 5);

private:
    std::size_t partition(std::vector<T>& arr, std::size_t left, std::size_t right, std::size_t pivotIndex);
    std::size_t medianOfMediansVariant(std::vector<T>& arr, std::size_t left, std::size_t right, std::size_t groupSize);
    T selectVariant(std::vector<T>& arr, std::size_t left, std::size_t right, std::size_t k, std::size_t groupSize);
};

template<typename T>
T SelectVariant<T>::select(std::vector<T>& data, std::size_t k, std::size_t groupSize) {
    if (data.empty() || k >= data.size()) {
        throw std::out_of_range("Invalid selection index");
    }
    return selectVariant(data, 0, data.size() - 1, k, groupSize);
}

template<typename T>
std::size_t SelectVariant<T>::partition(std::vector<T>& arr, std::size_t left, std::size_t right, std::size_t pivotIndex) {
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
std::size_t SelectVariant<T>::medianOfMediansVariant(std::vector<T>& arr, std::size_t left, std::size_t right, std::size_t groupSize) {
    if (right < left) return left;
    if (right - left < groupSize) {
        std::sort(arr.begin() + left, arr.begin() + right + 1);
        return left + (right - left) / 2;
    }

    std::size_t numMedians = 0;
    for (std::size_t i = left; i <= right; i += groupSize) {
        std::size_t subRight = std::min(right, i + groupSize - 1);
        std::sort(arr.begin() + i, arr.begin() + subRight + 1);
        std::size_t medianIndex = i + (subRight - i) / 2;
        std::swap(arr[left + numMedians], arr[medianIndex]);
        numMedians++;
    }

    T medianValue = selectVariant(arr, left, left + numMedians - 1, left + numMedians / 2, groupSize);

    for (std::size_t i = left; i < left + numMedians; ++i) {
        if (arr[i] == medianValue) {
            return i;
        }
    }
    return left + numMedians / 2;
}

template<typename T>
T SelectVariant<T>::selectVariant(std::vector<T>& arr, std::size_t left, std::size_t right, std::size_t k, std::size_t groupSize) {
    if (left == right) {
        return arr[left];
    }

    std::size_t pivotIndex = medianOfMediansVariant(arr, left, right, groupSize);
    std::size_t pivotNewIndex = partition(arr, left, right, pivotIndex);

    if (k == pivotNewIndex) {
        return arr[k];
    } else if (k < pivotNewIndex) {
        return selectVariant(arr, left, pivotNewIndex - 1, k, groupSize);
    } else {
        return selectVariant(arr, pivotNewIndex + 1, right, k, groupSize);
    }
}

#endif // SELECT_VARIANT_HPP