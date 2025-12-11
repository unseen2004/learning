#ifndef DOUBLELINKEDLIST_HPP
#define DOUBLELINKEDLIST_HPP

#include "Node.hpp"

template<typename T>
class DoubleLinkedList {
    Node<T> *m_head;
    Node<T> *m_tail;

public:
    DoubleLinkedList();

    ~DoubleLinkedList();

    void push(T val);

    void pop();

    bool search(T val);
};

template<typename T>
DoubleLinkedList<T>::DoubleLinkedList() : m_head(nullptr), m_tail(nullptr) {
}

template<typename T>
DoubleLinkedList<T>::~DoubleLinkedList() {
    while (m_tail) {
        pop();
    }
}

template<typename T>
void DoubleLinkedList<T>::push(T val) {
    Node<T> *newNode = new Node<T>(val, nullptr, nullptr);
    if (m_tail == nullptr) {
        m_head = m_tail = newNode;
    } else {
        m_tail->right = newNode;
        newNode->left = m_tail;
        m_tail = newNode;
    }
}

template<typename T>
void DoubleLinkedList<T>::pop() {
    if (m_tail == nullptr) {
        return;
    } else if (m_tail == m_head) {
        delete m_tail;
        m_tail = m_head = nullptr;
    } else {
        Node<T> *toDel = m_tail;
        m_tail = m_tail->left;
        m_tail->right = nullptr;
        delete toDel;
    }
}

template<typename T>
bool DoubleLinkedList<T>::search(T val) {
    Node<T> *tmp = m_head;
    while (tmp) {
        if (tmp->val == val) {
            return true;
        }
        tmp = tmp->right;
    }
    return false;
}

#endif // DOUBLELINKEDLIST_HPP