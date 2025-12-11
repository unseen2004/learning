#ifndef STACK_HPP
#define STACK_HPP

#include <stdexcept>
#include "Node.hpp"

template <typename T>
class Stack {
    node<T>* m_front;
    int m_size;

public:
    Stack() : m_front(nullptr), m_size(0) {}
    ~Stack();

    void push(T data);
    void pop();
    T peek() const;
    [[nodiscard]] bool is_empty() const;
    [[nodiscard]] int size() const;
};

template <typename T>
Stack<T>::~Stack() {
    while (!is_empty()) {
        pop();
    }
}

template <typename T>
void Stack<T>::push(T data) {
    node<T>* new_node = new node<T>(data);
    new_node->next = m_front;
    m_front = new_node;
    ++m_size;
}

template <typename T>
void Stack<T>::pop() {
    if (is_empty()) {
        throw std::out_of_range("Stack is empty");
    }
    const node<T>* temp = m_front;
    m_front = m_front->next;
    delete temp;
    --m_size;
}

template <typename T>
T Stack<T>::peek() const {
    if (is_empty()) {
        throw std::out_of_range("Stack is empty");
    }
    return m_front->val;
}

template <typename T>
bool Stack<T>::is_empty() const {
    return m_size == 0;
}

template <typename T>
int Stack<T>::size() const {
    return m_size;
}

#endif // STACK_HPP