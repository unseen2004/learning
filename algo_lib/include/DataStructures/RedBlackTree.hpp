#ifndef REDBLACKTREE_HPP
#define REDBLACKTREE_HPP

#include <iostream>

enum Color { RED, BLACK };

template<typename T>
class RedBlackTree {
private:
    struct Node {
        T data;
        Node *left;
        Node *right;
        Node *parent;
        Color color;

        Node(T data);
    };

    Node *root;

public:
    RedBlackTree();

    ~RedBlackTree();

    void insert(T data);

    bool remove(T data);

    bool contains(T data) const;

    void printInOrder() const;

private:
    Node *bstInsert(Node *root, Node *newNode);

    void rotateLeft(Node *&root, Node *&pt);

    void rotateRight(Node *&root, Node *&pt);

    void fixInsert(Node *&pt);

    void deleteNode(Node *node);

    void fixDelete(Node *x, Node *x_parent);

    Node *search(Node *root, T data) const;

    Node *getMinNode(Node *node) const;

    void transplant(Node *u, Node *v);

    void printInOrder(Node *root) const;

    void clear(Node *node);

    Node *getSibling(Node *node);
};

template<typename T>
RedBlackTree<T>::Node::Node(T data)
    : data(data), left(nullptr), right(nullptr), parent(nullptr), color(RED) {
}

template<typename T>
RedBlackTree<T>::RedBlackTree() : root(nullptr) {
}

template<typename T>
RedBlackTree<T>::~RedBlackTree() {
    clear(root);
}

template<typename T>
void RedBlackTree<T>::insert(T data) {
    Node *newNode = new Node(data);
    root = bstInsert(root, newNode);
    fixInsert(newNode);
}

template<typename T>
bool RedBlackTree<T>::remove(T data) {
    Node *node = search(root, data);
    if (node) {
        deleteNode(node);
        return true;
    }
    return false;
}

template<typename T>
bool RedBlackTree<T>::contains(T data) const {
    return search(root, data) != nullptr;
}

template<typename T>
void RedBlackTree<T>::printInOrder() const {
    printInOrder(root);
    std::cout << std::endl;
}

template<typename T>
typename RedBlackTree<T>::Node *RedBlackTree<T>::bstInsert(Node *root, Node *newNode) {
    if (root == nullptr) {
        return newNode;
    }

    if (newNode->data < root->data) {
        root->left = bstInsert(root->left, newNode);
        root->left->parent = root;
    } else if (newNode->data > root->data) {
        root->right = bstInsert(root->right, newNode);
        root->right->parent = root;
    }

    return root;
}

template<typename T>
void RedBlackTree<T>::rotateLeft(Node *&root, Node *&pt) {
    Node *pt_right = pt->right;

    pt->right = pt_right->left;

    if (pt->right != nullptr)
        pt->right->parent = pt;

    pt_right->parent = pt->parent;

    if (pt->parent == nullptr)
        root = pt_right;
    else if (pt == pt->parent->left)
        pt->parent->left = pt_right;
    else
        pt->parent->right = pt_right;

    pt_right->left = pt;
    pt->parent = pt_right;
}

template<typename T>
void RedBlackTree<T>::rotateRight(Node *&root, Node *&pt) {
    Node *pt_left = pt->left;

    pt->left = pt_left->right;

    if (pt->left != nullptr)
        pt->left->parent = pt;

    pt_left->parent = pt->parent;

    if (pt->parent == nullptr)
        root = pt_left;
    else if (pt == pt->parent->left)
        pt->parent->left = pt_left;
    else
        pt->parent->right = pt_left;

    pt_left->right = pt;
    pt->parent = pt_left;
}

template<typename T>
void RedBlackTree<T>::fixInsert(Node *&pt) {
    Node *parent_pt = nullptr;
    Node *grand_parent_pt = nullptr;

    while ((pt != root) && (pt->color != BLACK) && (pt->parent->color == RED)) {
        parent_pt = pt->parent;
        grand_parent_pt = pt->parent->parent;

        if (parent_pt == grand_parent_pt->left) {
            Node *uncle_pt = grand_parent_pt->right;

            if (uncle_pt != nullptr && uncle_pt->color == RED) {
                grand_parent_pt->color = RED;
                parent_pt->color = BLACK;
                uncle_pt->color = BLACK;
                pt = grand_parent_pt;
            } else {
                if (pt == parent_pt->right) {
                    rotateLeft(root, parent_pt);
                    pt = parent_pt;
                    parent_pt = pt->parent;
                }

                rotateRight(root, grand_parent_pt);
                std::swap(parent_pt->color, grand_parent_pt->color);
                pt = parent_pt;
            }
        } else {
            Node *uncle_pt = grand_parent_pt->left;

            if (uncle_pt != nullptr && uncle_pt->color == RED) {
                grand_parent_pt->color = RED;
                parent_pt->color = BLACK;
                uncle_pt->color = BLACK;
                pt = grand_parent_pt;
            } else {
                if (pt == parent_pt->left) {
                    rotateRight(root, parent_pt);
                    pt = parent_pt;
                    parent_pt = pt->parent;
                }

                rotateLeft(root, grand_parent_pt);
                std::swap(parent_pt->color, grand_parent_pt->color);
                pt = parent_pt;
            }
        }
    }

    root->color = BLACK;
}

template<typename T>
void RedBlackTree<T>::deleteNode(Node *node) {
    Node *y = node;
    Node *x = nullptr;
    Node *x_parent = nullptr;
    Color original_color = y->color;

    if (node->left == nullptr) {
        x = node->right;
        transplant(node, node->right);
    } else if (node->right == nullptr) {
        x = node->left;
        transplant(node, node->left);
    } else {
        y = getMinNode(node->right);
        original_color = y->color;
        x = y->right;
        if (y->parent == node) {
            if (x) x->parent = y;
        } else {
            transplant(y, y->right);
            y->right = node->right;
            y->right->parent = y;
        }
        transplant(node, y);
        y->left = node->left;
        y->left->parent = y;
        y->color = node->color;
    }

    delete node;

    if (original_color == BLACK) {
        fixDelete(x, x_parent);
    }
}

template<typename T>
void RedBlackTree<T>::fixDelete(Node *x, Node *x_parent) {
    while (x != root && (x == nullptr || x->color == BLACK)) {
        if (x == x_parent->left) {
            Node *w = x_parent->right;
            if (w->color == RED) {
                w->color = BLACK;
                x_parent->color = RED;
                rotateLeft(root, x_parent);
                w = x_parent->right;
            }

            if ((w->left == nullptr || w->left->color == BLACK) &&
                (w->right == nullptr || w->right->color == BLACK)) {
                w->color = RED;
                x = x_parent;
                x_parent = x->parent;
            } else {
                if (w->right == nullptr || w->right->color == BLACK) {
                    if (w->left) w->left->color = BLACK;
                    w->color = RED;
                    rotateRight(root, w);
                    w = x_parent->right;
                }

                w->color = x_parent->color;
                x_parent->color = BLACK;
                if (w->right) w->right->color = BLACK;
                rotateLeft(root, x_parent);
                x = root;
            }
        } else {
            Node *w = x_parent->left;
            if (w->color == RED) {
                w->color = BLACK;
                x_parent->color = RED;
                rotateRight(root, x_parent);
                w = x_parent->left;
            }

            if ((w->left == nullptr || w->left->color == BLACK) &&
                (w->right == nullptr || w->right->color == BLACK)) {
                w->color = RED;
                x = x_parent;
                x_parent = x->parent;
            } else {
                if (w->left == nullptr || w->left->color == BLACK) {
                    if (w->right) w->right->color = BLACK;
                    w->color = RED;
                    rotateLeft(root, w);
                    w = x_parent->left;
                }

                w->color = x_parent->color;
                x_parent->color = BLACK;
                if (w->left) w->left->color = BLACK;
                rotateRight(root, x_parent);
                x = root;
            }
        }
    }

    if (x) x->color = BLACK;
}

template<typename T>
typename RedBlackTree<T>::Node *RedBlackTree<T>::search(Node *root, T data) const {
    if (root == nullptr || root->data == data)
        return root;

    if (data < root->data)
        return search(root->left, data);

    return search(root->right, data);
}

template<typename T>
typename RedBlackTree<T>::Node *RedBlackTree<T>::getMinNode(Node *node) const {
    while (node->left != nullptr)
        node = node->left;
    return node;
}

template<typename T>
void RedBlackTree<T>::transplant(Node *u, Node *v) {
    if (u->parent == nullptr)
        root = v;
    else if (u == u->parent->left)
        u->parent->left = v;
    else
        u->parent->right = v;

    if (v != nullptr)
        v->parent = u->parent;
}

template<typename T>
void RedBlackTree<T>::printInOrder(Node *root) const {
    if (root == nullptr)
        return;

    printInOrder(root->left);
    std::cout << root->data << " ";
    printInOrder(root->right);
}

template<typename T>
void RedBlackTree<T>::clear(Node *node) {
    if (!node) return;

    clear(node->left);
    clear(node->right);
    delete node;
}

template<typename T>
typename RedBlackTree<T>::Node *RedBlackTree<T>::getSibling(Node *node) {
    if (node->parent == nullptr)
        return nullptr;

    if (node == node->parent->left)
        return node->parent->right;

    return node->parent->left;
}

#endif // REDBLACKTREE_HPP
