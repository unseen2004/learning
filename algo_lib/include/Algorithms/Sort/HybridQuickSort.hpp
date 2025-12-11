#ifndef HYBRID_QUICKSORT_HPP
#define HYBRID_QUICKSORT_HPP

#include <vector>

template<typename T>
class HybridQuickSort {
public:
    void sort(std::vector<T>& data);

private:
    const int INSERTION_THRESHOLD = 16;
    void hybridQuickSort(std::vector<T>& arr, int left, int right);
    void insertionSort(std::vector<T>& arr, int left, int right);
};

template<typename T>
void HybridQuickSort<T>::sort(std::vector<T>& data) {
    if (data.empty()) return;
    hybridQuickSort(data, 0, data.size() - 1);
}

template<typename T>
void HybridQuickSort<T>::insertionSort(std::vector<T>& arr, int left, int right) {
    for (int i = left + 1; i <= right; ++i) {
        T key = arr[i];
        int j = i - 1;
        while (j >= left && key < arr[j]) {
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = key;
    }
}

template<typename T>
void HybridQuickSort<T>::hybridQuickSort(std::vector<T>& arr, int left, int right) {
    while (left < right) {
        if (right - left + 1 <= INSERTION_THRESHOLD) {
            insertionSort(arr, left, right);
            break;
        } else {
            T pivot = arr[right];
            int i = left - 1;
            for (int j = left; j < right; ++j) {
                if (arr[j] < pivot) {
                    ++i;
                    T temp = arr[i];
                    arr[i] = arr[j];
                    arr[j] = temp;
                }
            }
            T temp = arr[i + 1];
            arr[i + 1] = arr[right];
            arr[right] = temp;

            int p = i + 1;
            if (p - left < right - p) {
                hybridQuickSort(arr, left, p - 1);
                left = p + 1;
            } else {
                hybridQuickSort(arr, p + 1, right);
                right = p - 1;
            }
        }
    }
}

#endif // HYBRID_QUICKSORT_HPP