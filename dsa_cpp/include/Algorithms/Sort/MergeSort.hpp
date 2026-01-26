#ifndef MERGESORT_HPP
#define MERGESORT_HPP

#include <vector>

template<typename T>
class MergeSort {
public:
    void sort(std::vector<T>& data);

private:
    void mergeSort(std::vector<T>& data, int left, int right);
    void merge(std::vector<T>& data, int left, int mid, int right);
};

template<typename T>
void MergeSort<T>::sort(std::vector<T>& data) {
    mergeSort(data, 0, data.size() - 1);
}

template<typename T>
void MergeSort<T>::mergeSort(std::vector<T>& data, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(data, left, mid);
        mergeSort(data, mid + 1, right);
        merge(data, left, mid, right);
    }
}

template<typename T>
void MergeSort<T>::merge(std::vector<T>& data, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<T> L(n1), R(n2);

    for (int i = 0; i < n1; ++i)
        L[i] = data[left + i];
    for (int j = 0; j < n2; ++j)
        R[j] = data[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            data[k] = L[i];
            ++i;
        } else {
            data[k] = R[j];
            ++j;
        }
        ++k;
    }

    while (i < n1) {
        data[k] = L[i];
        ++i;
        ++k;
    }

    while (j < n2) {
        data[k] = R[j];
        ++j;
        ++k;
    }
}

#endif // MERGESORT_HPP