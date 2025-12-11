#ifndef BST_HPP
#define BST_HPP

#include <iostream>
#include <stdexcept>
#include "Node.hpp"

template<typename T>
class BST {
private:
    Node<T> *m_root;

public:
    BST();

    ~BST();

    void insert(T value);

    bool remove(T value);

    bool contains(T value) const;

    T getMin() const;

    T getMax() const;

    int getHeight() const;

    void printInOrder() const;

private:
    Node<T> *insert(Node<T> *node, T value);

    Node<T> *remove(Node<T> *node, T value);

    bool contains(Node<T> *node, T value) const;

    Node<T> *getMinNode(Node<T> *node) const;

    Node<T> *getMaxNode(Node<T> *node) const;

    int getHeight(Node<T> *node) const;

    void printInOrder(Node<T> *node) const;

    void clear(Node<T> *node);
};

template<typename T>
BST<T>::BST() : m_root(nullptr) {
}

template<typename T>
BST<T>::~BST() {
    clear(m_root);
}

template<typename T>
void BST<T>::insert(T value) {
    m_root = insert(m_root, value);
}

template<typename T>
bool BST<T>::remove(T value) {
    if (contains(m_root, value)) {
        m_root = remove(m_root, value);
        return true;
    }
    return false;
}

template<typename T>
bool BST<T>::contains(T value) const {
    return contains(m_root, value);
}

template<typename T>
T BST<T>::getMin() const {
    if (m_root == nullptr) {
        throw std::runtime_error("The tree is empty.");
    }
    return getMinNode(m_root)->val;
}

template<typename T>
T BST<T>::getMax() const {
    if (m_root == nullptr) {
        throw std::runtime_error("The tree is empty.");
    }
    return getMaxNode(m_root)->val;
}

template<typename T>
int BST<T>::getHeight() const {
    return getHeight(m_root);
}

template<typename T>
void BST<T>::printInOrder() const {
    printInOrder(m_root);
}

template<typename T>
Node<T> *BST<T>::insert(Node<T> *node, T value) {
    if (node == nullptr) {
        return new Node<T>(value);
    }
    if (value < node->val) {
        node->left = insert(node->left, value);
    } else if (value > node->val) {
        node->right = insert(node->right, value);
    }
    return node;
}

template<typename T>
Node<T> *BST<T>::remove(Node<T> *node, T value) {
    if (node == nullptr) {
        return node;
    }
    if (value < node->val) {
        node->left = remove(node->left, value);
    } else if (value > node->val) {
        node->right = remove(node->right, value);
    } else {
        if (node->left == nullptr) {
            Node<T> *temp = node->right;
            delete node;
            return temp;
        } else if (node->right == nullptr) {
            Node<T> *temp = node->left;
            delete node;
            return temp;
        }
        Node<T> *temp = getMinNode(node->right);
        node->val = temp->val;
        node->right = remove(node->right, temp->val);
    }
    return node;
}

template<typename T>
bool BST<T>::contains(Node<T> *node, T value) const {
    if (node == nullptr) {
        return false;
    }
    if (value < node->val) {
        return contains(node->left, value);
    } else if (value > node->val) {
        return contains(node->right, value);
    }
    return true;
}

template<typename T>
Node<T> *BST<T>::getMinNode(Node<T> *node) const {
    Node<T> *current = node;
    while (current && current->left != nullptr) {
        current = current->left;
    }
    return current;
}

template<typename T>
Node<T> *BST<T>::getMaxNode(Node<T> *node) const {
    Node<T> *current = node;
    while (current && current->right != nullptr) {
        current = current->right;
    }
    return current;
}

template<typename T>
int BST<T>::getHeight(Node<T> *node) const {
    if (node == nullptr) {
        return 0;
    }
    int leftHeight = getHeight(node->left);
    int rightHeight = getHeight(node->right);
    return std::max(leftHeight, rightHeight) + 1;
}

template<typename T>
void BST<T>::printInOrder(Node<T> *node) const {
    if (node != nullptr) {
        printInOrder(node->left);
        std::cout << node->val << " ";
        printInOrder(node->right);
    }
}

template<typename T>
void BST<T>::clear(Node<T> *node) {
    if (node != nullptr) {
        clear(node->left);
        clear(node->right);
        delete node;
    }
}

#endif // BST_HPP
