#ifndef AVLTREE_HPP
#define AVLTREE_HPP

#include <iostream>
#include <unordered_map>
#include "Node.hpp"

template<typename T>
class AVLTree {
private:
    Node<T> *m_root;
    std::unordered_map<Node<T>*, int> heights;

public:
    AVLTree();

    ~AVLTree();

    void insert(T value);

    bool remove(T value);

    bool contains(T value) const;

    int getHeight() const;

    void printInOrder() const;

private:
    Node<T> *insert(Node<T> *node, T value);

    Node<T> *remove(Node<T> *node, T value);

    bool contains(Node<T> *node, T value) const;

    Node<T> *balance(Node<T> *node);

    int getBalanceFactor(Node<T> *node) const;

    Node<T> *rotateLeft(Node<T> *node);

    Node<T> *rotateRight(Node<T> *node);

    int getHeight(Node<T> *node) const;

    int max(int a, int b) const;

    void printInOrder(Node<T> *node) const;

    Node<T> *getMinNode(Node<T> *node) const;

    void clear(Node<T> *node);
};

template<typename T>
AVLTree<T>::AVLTree() : m_root(nullptr) {
}

template<typename T>
AVLTree<T>::~AVLTree() {
    clear(m_root);
}

template<typename T>
void AVLTree<T>::insert(T value) {
    m_root = insert(m_root, value);
}

template<typename T>
bool AVLTree<T>::remove(T value) {
    if (contains(value)) {
        m_root = remove(m_root, value);
        return true;
    }
    return false;
}

template<typename T>
bool AVLTree<T>::contains(T value) const {
    return contains(m_root, value);
}

template<typename T>
int AVLTree<T>::getHeight() const {
    return getHeight(m_root);
}

template<typename T>
void AVLTree<T>::printInOrder() const {
    printInOrder(m_root);
    std::cout << std::endl;
}

template<typename T>
Node<T> *AVLTree<T>::insert(Node<T> *node, T value) {
    if (!node) {
        Node<T> *newNode = new Node<T>(value);
        heights[newNode] = 1;
        return newNode;
    }

    if (value < node->val) {
        node->left = insert(node->left, value);
    } else if (value > node->val) {
        node->right = insert(node->right, value);
    } else {
        return node;
    }

    heights[node] = 1 + max(getHeight(node->left), getHeight(node->right));

    return balance(node);
}

template<typename T>
Node<T> *AVLTree<T>::remove(Node<T> *node, T value) {
    if (!node) return nullptr;

    if (value < node->val) {
        node->left = remove(node->left, value);
    } else if (value > node->val) {
        node->right = remove(node->right, value);
    } else {
        if (!node->left || !node->right) {
            Node<T> *temp = node->left ? node->left : node->right;
            if (!temp) {
                temp = node;
                node = nullptr;
            } else {
                *node = *temp;
                heights.erase(temp);
            }
            delete temp;
        } else {
            Node<T> *minNode = getMinNode(node->right);
            node->val = minNode->val;
            node->right = remove(node->right, minNode->val);
        }
    }

    if (!node) return nullptr;

    heights[node] = 1 + max(getHeight(node->left), getHeight(node->right));
    return balance(node);
}

template<typename T>
bool AVLTree<T>::contains(Node<T> *node, T value) const {
    if (!node) return false;

    if (value < node->val) {
        return contains(node->left, value);
    } else if (value > node->val) {
        return contains(node->right, value);
    }
    return true;
}

template<typename T>
Node<T> *AVLTree<T>::balance(Node<T> *node) {
    int balanceFactor = getBalanceFactor(node);

    if (balanceFactor < -1) {
        if (getBalanceFactor(node->left) > 0) {
            node->left = rotateLeft(node->left);
        }
        return rotateRight(node);
    }

    if (balanceFactor > 1) {
        if (getBalanceFactor(node->right) < 0) {
            node->right = rotateRight(node->right);
        }
        return rotateLeft(node);
    }

    return node;
}

template<typename T>
int AVLTree<T>::getBalanceFactor(Node<T> *node) const {
    if (!node) return 0;
    return getHeight(node->right) - getHeight(node->left);
}

template<typename T>
Node<T> *AVLTree<T>::rotateLeft(Node<T> *node) {
    Node<T> *newRoot = node->right;
    node->right = newRoot->left;
    newRoot->left = node;

    heights[node] = 1 + max(getHeight(node->left), getHeight(node->right));
    heights[newRoot] = 1 + max(getHeight(newRoot->left), getHeight(newRoot->right));

    return newRoot;
}

template<typename T>
Node<T> *AVLTree<T>::rotateRight(Node<T> *node) {
    Node<T> *newRoot = node->left;
    node->left = newRoot->right;
    newRoot->right = node;

    heights[node] = 1 + max(getHeight(node->left), getHeight(node->right));
    heights[newRoot] = 1 + max(getHeight(newRoot->left), getHeight(newRoot->right));

    return newRoot;
}

template<typename T>
int AVLTree<T>::getHeight(Node<T> *node) const {
    if (!node) return 0;
    auto it = heights.find(node);
    return it != heights.end() ? it->second : 0;
}

template<typename T>
int AVLTree<T>::max(int a, int b) const {
    return (a > b) ? a : b;
}

template<typename T>
void AVLTree<T>::printInOrder(Node<T> *node) const {
    if (!node) return;

    printInOrder(node->left);
    std::cout << node->val << " ";
    printInOrder(node->right);
}

template<typename T>
Node<T> *AVLTree<T>::getMinNode(Node<T> *node) const {
    while (node && node->left) {
        node = node->left;
    }
    return node;
}

template<typename T>
void AVLTree<T>::clear(Node<T> *node) {
    if (!node) return;

    clear(node->left);
    clear(node->right);
    heights.erase(node);
    delete node;
}

#endif // AVLTREE_HPP