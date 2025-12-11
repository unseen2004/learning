#ifndef DUALPIVOTQUICKSORT_HPP
#define DUALPIVOTQUICKSORT_HPP

#include <vector>

template<typename T>
class DualPivotQuickSort {
public:
    void sort(std::vector<T>& data);

private:
    void dualPivotQuicksort(std::vector<T>& arr, int left, int right);
};

template<typename T>
void DualPivotQuickSort<T>::sort(std::vector<T>& data) {
    if (data.empty()) return;
    dualPivotQuicksort(data, 0, data.size() - 1);
}

template<typename T>
void DualPivotQuickSort<T>::dualPivotQuicksort(std::vector<T>& arr, int left, int right) {
    if (left >= right) return;

    T p = arr[left], q = arr[right];
    if (q < p) {
        T temp = arr[left];
        arr[left] = arr[right];
        arr[right] = temp;
        p = arr[left];
        q = arr[right];
    }

    int lt = left + 1, gt = right - 1, i = left + 1;
    int s = 0, l = 0;

    while (i <= gt) {
        if (l > s) {
            if (q < arr[i]) {
                T temp = arr[i];
                arr[i] = arr[gt];
                arr[gt] = temp;
                gt--;
                l++;
            } else {
                if (arr[i] < p) {
                    T temp = arr[i];
                    arr[i] = arr[lt];
                    arr[lt] = temp;
                    lt++;
                    s++;
                }
                i++;
            }
        } else {
            if (arr[i] < p) {
                T temp = arr[i];
                arr[i] = arr[lt];
                arr[lt] = temp;
                lt++;
                s++;
                i++;
            } else {
                if (q < arr[i]) {
                    T temp = arr[i];
                    arr[i] = arr[gt];
                    arr[gt] = temp;
                    gt--;
                    l++;
                } else {
                    i++;
                }
            }
        }
    }

    lt--;
    gt++;

    T temp1 = arr[left];
    arr[left] = arr[lt];
    arr[lt] = temp1;

    T temp2 = arr[right];
    arr[right] = arr[gt];
    arr[gt] = temp2;

    dualPivotQuicksort(arr, left, lt - 1);
    dualPivotQuicksort(arr, lt + 1, gt - 1);
    dualPivotQuicksort(arr, gt + 1, right);
}

#endif // DUALPIVOTQUICKSORT_HPP