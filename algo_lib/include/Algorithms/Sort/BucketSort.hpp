#ifndef BUCKETSORT_HPP
#define BUCKETSORT_HPP

#include <vector>

template<typename T>
class BucketSort {
public:
    void sort(std::vector<T>& data);

private:
    void insertionSort(std::vector<T>& bucket);
};

template<typename T>
void BucketSort<T>::sort(std::vector<T>& data) {
    if (data.empty()) return;

    T maxValue = data[0];
    T minValue = data[0];
    for (const T& value : data) {
        if (value > maxValue) maxValue = value;
        if (value < minValue) minValue = value;
    }

    int bucketCount = data.size();
    std::vector<std::vector<T>> buckets(bucketCount);

    for (const T& value : data) {
        int bucketIndex = static_cast<int>((value - minValue) / (maxValue - minValue + 1) * bucketCount);
        buckets[bucketIndex].push_back(value);
    }

    data.clear();
    for (auto& bucket : buckets) {
        insertionSort(bucket);
        data.insert(data.end(), bucket.begin(), bucket.end());
    }
}

template<typename T>
void BucketSort<T>::insertionSort(std::vector<T>& bucket) {
    for (std::size_t i = 1; i < bucket.size(); ++i) {
        T key = bucket[i];
        int j = i - 1;
        while (j >= 0 && bucket[j] > key) {
            bucket[j + 1] = bucket[j];
            --j;
        }
        bucket[j + 1] = key;
    }
}

#endif // BUCKETSORT_HPP