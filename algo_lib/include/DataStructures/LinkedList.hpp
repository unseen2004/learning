#ifndef LINKEDLIST_HPP
#define LINKEDLIST_HPP

#include "Node.hpp"

template<typename T>
class LinkedList {
    Node<T> *m_head;

public:
    LinkedList();

    ~LinkedList();

    void add(T val);

    void pop();

    bool search(T val);
};

template<typename T>
LinkedList<T>::LinkedList() : m_head(nullptr) {
}

template<typename T>
LinkedList<T>::~LinkedList() {
    while (m_head) {
        pop();
    }
}

template<typename T>
void LinkedList<T>::add(T val) {
    Node<T> *newNode = new Node<T>(val);
    if (m_head == nullptr) {
        m_head = newNode;
    } else {
        Node<T> *tmp = m_head;
        while (tmp->right) {
            tmp = tmp->right;
        }
        tmp->right = newNode;
    }
}

template<typename T>
void LinkedList<T>::pop() {
    if (m_head == nullptr) {
        return;
    } else if (m_head->right == nullptr) {
        delete m_head;
        m_head = nullptr;
    } else {
        Node<T> *tmp = m_head;
        while (tmp->right->right) {
            tmp = tmp->right;
        }
        delete tmp->right;
        tmp->right = nullptr;
    }
}

template<typename T>
bool LinkedList<T>::search(T val) {
    Node<T> *tmp = m_head;
    while (tmp) {
        if (tmp->val == val) {
            return true;
        }
        tmp = tmp->right;
    }
    return false;
}

#endif // LINKEDLIST_HPP