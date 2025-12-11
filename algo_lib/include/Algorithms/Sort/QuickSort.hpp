#ifndef QUICKSORT_HPP
#define QUICKSORT_HPP

#include <vector>

template<typename T>
class QuickSort {
public:
    void sort(std::vector<T>& data);

private:
    void quickSort(std::vector<T>& data, int low, int high);
    int partition(std::vector<T>& data, int low, int high);
    void swap(T& a, T& b);
};

template<typename T>
void QuickSort<T>::sort(std::vector<T>& data) {
    quickSort(data, 0, data.size() - 1);
}

template<typename T>
void QuickSort<T>::quickSort(std::vector<T>& data, int low, int high) {
    if (low < high) {
        int pi = partition(data, low, high);
        quickSort(data, low, pi - 1);
        quickSort(data, pi + 1, high);
    }
}

template<typename T>
int QuickSort<T>::partition(std::vector<T>& data, int low, int high) {
    T pivot = data[high];
    int i = (low - 1);

    for (int j = low; j < high; ++j) {
        if (data[j] < pivot) {
            ++i;
            swap(data[i], data[j]);
        }
    }
    swap(data[i + 1], data[high]);
    return (i + 1);
}

template<typename T>
void QuickSort<T>::swap(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}

#endif // QUICKSORT_HPP