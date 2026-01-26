#ifndef RBBST_HPP
#define RBBST_HPP

#include <algorithm>
#include <stdexcept>

enum class Color { RED, BLACK };

template<typename T>
class RBBST {
private:
    struct Node {
        T key;
        Color color;
        Node* left;
        Node* right;
        Node* parent;

        Node(const T& k) : key(k), color(Color::RED),
                          left(nullptr), right(nullptr), parent(nullptr) {}
    };

    Node* root;
    Node* nil;  // Sentinel node

    void rotateLeft(Node* x);
    void rotateRight(Node* x);
    void fixInsert(Node* node);
    void fixDelete(Node* x);
    void transplant(Node* u, Node* v);
    Node* minimum(Node* node) const;
    Node* maximum(Node* node) const;
    int getHeight(Node* node) const;
    void clearTree(Node* node);

public:
    RBBST();
    ~RBBST();

    void insert(const T& key);
    bool remove(const T& key);
    bool contains(const T& key) const;
    bool isEmpty() const;
    int height() const;
    T getMin() const;
    T getMax() const;
};

template<typename T>
RBBST<T>::RBBST() {
    nil = new Node(T{});
    nil->color = Color::BLACK;
    nil->left = nullptr;
    nil->right = nullptr;
    root = nil;
}

template<typename T>
RBBST<T>::~RBBST() {
    clearTree(root);
    delete nil;
}

template<typename T>
void RBBST<T>::clearTree(Node* node) {
    if (node != nil) {
        clearTree(node->left);
        clearTree(node->right);
        delete node;
    }
}

template<typename T>
void RBBST<T>::rotateLeft(Node* x) {
    Node* y = x->right;
    x->right = y->left;

    if (y->left != nil) {
        y->left->parent = x;
    }

    y->parent = x->parent;

    if (x->parent == nullptr) {
        root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

template<typename T>
void RBBST<T>::rotateRight(Node* x) {
    Node* y = x->left;
    x->left = y->right;

    if (y->right != nil) {
        y->right->parent = x;
    }

    y->parent = x->parent;

    if (x->parent == nullptr) {
        root = y;
    } else if (x == x->parent->right) {
        x->parent->right = y;
    } else {
        x->parent->left = y;
    }

    y->right = x;
    x->parent = y;
}

template<typename T>
void RBBST<T>::fixInsert(Node* node) {
    Node* uncle;

    while (node->parent && node->parent->color == Color::RED) {
        if (node->parent == node->parent->parent->right) {
            uncle = node->parent->parent->left;

            if (uncle->color == Color::RED) {
                node->parent->color = Color::BLACK;
                uncle->color = Color::BLACK;
                node->parent->parent->color = Color::RED;
                node = node->parent->parent;
            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    rotateRight(node);
                }
                node->parent->color = Color::BLACK;
                node->parent->parent->color = Color::RED;
                rotateLeft(node->parent->parent);
            }
        } else {
            uncle = node->parent->parent->right;

            if (uncle->color == Color::RED) {
                node->parent->color = Color::BLACK;
                uncle->color = Color::BLACK;
                node->parent->parent->color = Color::RED;
                node = node->parent->parent;
            } else {
                if (node == node->parent->right) {
                    node = node->parent;
                    rotateLeft(node);
                }
                node->parent->color = Color::BLACK;
                node->parent->parent->color = Color::RED;
                rotateRight(node->parent->parent);
            }
        }
        if (node == root) {
            break;
        }
    }

    root->color = Color::BLACK;
}

template<typename T>
void RBBST<T>::insert(const T& key) {
    Node* newNode = new Node(key);
    newNode->left = nil;
    newNode->right = nil;

    Node* y = nullptr;
    Node* x = root;

    while (x != nil) {
        y = x;
        if (newNode->key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    newNode->parent = y;

    if (y == nullptr) {
        root = newNode;
    } else if (newNode->key < y->key) {
        y->left = newNode;
    } else {
        y->right = newNode;
    }

    if (newNode->parent == nullptr) {
        newNode->color = Color::BLACK;
        return;
    }

    if (newNode->parent->parent == nullptr) {
        return;
    }

    fixInsert(newNode);
}

template<typename T>
void RBBST<T>::transplant(Node* u, Node* v) {
    if (u->parent == nullptr) {
        root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

template<typename T>
typename RBBST<T>::Node* RBBST<T>::minimum(Node* node) const {
    while (node->left != nil) {
        node = node->left;
    }
    return node;
}

template<typename T>
typename RBBST<T>::Node* RBBST<T>::maximum(Node* node) const {
    while (node->right != nil) {
        node = node->right;
    }
    return node;
}

template<typename T>
void RBBST<T>::fixDelete(Node* x) {
    Node* sibling;

    while (x != root && x->color == Color::BLACK) {
        if (x == x->parent->left) {
            sibling = x->parent->right;

            if (sibling->color == Color::RED) {
                sibling->color = Color::BLACK;
                x->parent->color = Color::RED;
                rotateLeft(x->parent);
                sibling = x->parent->right;
            }

            if (sibling->left->color == Color::BLACK && sibling->right->color == Color::BLACK) {
                sibling->color = Color::RED;
                x = x->parent;
            } else {
                if (sibling->right->color == Color::BLACK) {
                    sibling->left->color = Color::BLACK;
                    sibling->color = Color::RED;
                    rotateRight(sibling);
                    sibling = x->parent->right;
                }

                sibling->color = x->parent->color;
                x->parent->color = Color::BLACK;
                sibling->right->color = Color::BLACK;
                rotateLeft(x->parent);
                x = root;
            }
        } else {
            sibling = x->parent->left;

            if (sibling->color == Color::RED) {
                sibling->color = Color::BLACK;
                x->parent->color = Color::RED;
                rotateRight(x->parent);
                sibling = x->parent->left;
            }

            if (sibling->right->color == Color::BLACK && sibling->left->color == Color::BLACK) {
                sibling->color = Color::RED;
                x = x->parent;
            } else {
                if (sibling->left->color == Color::BLACK) {
                    sibling->right->color = Color::BLACK;
                    sibling->color = Color::RED;
                    rotateLeft(sibling);
                    sibling = x->parent->left;
                }

                sibling->color = x->parent->color;
                x->parent->color = Color::BLACK;
                sibling->left->color = Color::BLACK;
                rotateRight(x->parent);
                x = root;
            }
        }
    }

    x->color = Color::BLACK;
}

template<typename T>
bool RBBST<T>::remove(const T& key) {
    Node* node = root;
    Node* z = nil;

    // Find the node to delete
    while (node != nil) {
        if (node->key == key) {
            z = node;
            break;
        }

        if (node->key < key) {
            node = node->right;
        } else {
            node = node->left;
        }
    }

    if (z == nil) {
        return false;  // Key not found
    }

    Node* x;
    Node* y = z;
    Color y_original_color = y->color;

    if (z->left == nil) {
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == nil) {
        x = z->left;
        transplant(z, z->left);
    } else {
        y = minimum(z->right);
        y_original_color = y->color;
        x = y->right;

        if (y->parent == z) {
            x->parent = y;
        } else {
            transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }

        transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    delete z;

    if (y_original_color == Color::BLACK) {
        fixDelete(x);
    }

    return true;
}

template<typename T>
bool RBBST<T>::contains(const T& key) const {
    Node* node = root;

    while (node != nil) {
        if (node->key == key) {
            return true;
        }

        if (node->key < key) {
            node = node->right;
        } else {
            node = node->left;
        }
    }

    return false;
}

template<typename T>
int RBBST<T>::getHeight(Node* node) const {
    if (node == nil) {
        return 0;
    }

    int leftHeight = getHeight(node->left);
    int rightHeight = getHeight(node->right);

    return std::max(leftHeight, rightHeight) + 1;
}

template<typename T>
int RBBST<T>::height() const {
    return getHeight(root);
}

template<typename T>
bool RBBST<T>::isEmpty() const {
    return root == nil;
}

template<typename T>
T RBBST<T>::getMin() const {
    if (isEmpty()) {
        throw std::runtime_error("Tree is empty");
    }
    return minimum(root)->key;
}

template<typename T>
T RBBST<T>::getMax() const {
    if (isEmpty()) {
        throw std::runtime_error("Tree is empty");
    }
    return maximum(root)->key;
}

#endif // RBBST_HPP