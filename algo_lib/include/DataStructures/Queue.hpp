#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <stdexcept>
#include "Node.hpp"

template<typename T>
class Queue {
    node<T> *m_front;
    node<T> *m_tail;
    int m_size;

public:
    Queue() : m_front(nullptr), m_tail(nullptr), m_size(0) {
    }

    ~Queue();

    void enqueue(T data);

    void dequeue();

    T peek() const;

    [[nodiscard]] int size() const;

    [[nodiscard]] bool is_empty() const;
};

template<typename T>
Queue<T>::~Queue() {
    while (!is_empty()) {
        dequeue();
    }
}

template<typename T>
void Queue<T>::enqueue(T data) {
    node<T> *new_node = new node<T>(data);
    if (is_empty()) {
        m_front = m_tail = new_node;
    } else {
        m_tail->next = new_node;
        m_tail = new_node;
    }
    ++m_size;
}

template<typename T>
void Queue<T>::dequeue() {
    if (is_empty()) {
        throw std::out_of_range("Queue is empty");
    }
    const node<T> *temp = m_front;
    m_front = m_front->next;
    delete temp;
    --m_size;
    if (is_empty()) {
        m_tail = nullptr;
    }
}

template<typename T>
T Queue<T>::peek() const {
    if (is_empty()) {
        throw std::out_of_range("Queue is empty");
    }
    return m_front->val;
}

template<typename T>
bool Queue<T>::is_empty() const {
    return m_size == 0;
}

template<typename T>
int Queue<T>::size() const {
    return m_size;
}

#endif // QUEUE_HPP