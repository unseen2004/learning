#ifndef HEAP_HPP
#define HEAP_HPP

#include <vector>
#include <functional>

template<typename T>
class Heap {
private:
    int length;
    std::vector<T> data;
    bool isMinHeap;

public:
    Heap(bool minHeap = true);

    ~Heap();

    void insert(T n);

    T deleteTop();

private:
    int parent(int idx) const;

    int leftChild(int idx) const;

    int rightChild(int idx) const;

    void heapifyUp(int idx);

    void heapifyDown(int idx);

    bool compare(T a, T b) const;

    void swap(T &a, T &b);
};

template<typename T>
Heap<T>::Heap(bool minHeap) : length(0), isMinHeap(minHeap) {
}

template<typename T>
Heap<T>::~Heap() {
    while (length > 0) {
        deleteTop();
    }
}

template<typename T>
void Heap<T>::insert(T n) {
    data.push_back(n);
    heapifyUp(length);
    ++length;
}

template<typename T>
T Heap<T>::deleteTop() {
    if (length == 0) {
        return T();
    }

    T out = data[0];

    --length;
    if (length == 0) {
        data.pop_back();
        return out;
    }

    data[0] = data[length];
    data.pop_back();
    heapifyDown(0);

    return out;
}

template<typename T>
int Heap<T>::parent(int idx) const {
    return (idx - 1) / 2;
}

template<typename T>
int Heap<T>::leftChild(int idx) const {
    return 2 * idx + 1;
}

template<typename T>
int Heap<T>::rightChild(int idx) const {
    return 2 * idx + 2;
}

template<typename T>
void Heap<T>::heapifyUp(int idx) {
    if (idx == 0) return;

    int p = parent(idx);
    if (compare(data[p], data[idx])) {
        swap(data[p], data[idx]);
        heapifyUp(p);
    }
}

template<typename T>
void Heap<T>::heapifyDown(int idx) {
    int lIdx = leftChild(idx);
    int rIdx = rightChild(idx);
    int extreme = idx;

    if (lIdx < length && compare(data[extreme], data[lIdx])) {
        extreme = lIdx;
    }

    if (rIdx < length && compare(data[extreme], data[rIdx])) {
        extreme = rIdx;
    }

    if (extreme != idx) {
        swap(data[idx], data[extreme]);
        heapifyDown(extreme);
    }
}

template<typename T>
bool Heap<T>::compare(T a, T b) const {
    return isMinHeap ? std::greater<T>()(a, b) : std::less<T>()(a, b);
}

template<typename T>
void Heap<T>::swap(T &a, T &b) {
    T temp = a;
    a = b;
    b = temp;
}

#endif // HEAP_HPP
