#ifndef NODE_HPP
#define NODE_HPP

template<typename T>
struct node {
    T val;
    node<T> *next;

    node(T val) : val(val), next(nullptr) {
    }
};

template<typename T>
struct Node {
    T val;
    Node<T> *left;
    Node<T> *right;

    Node(T v, Node<T> *left = nullptr, Node<T> *right = nullptr)
        : val(v), left(left), right(right) {
    }
};

#endif // NODE_HPP
