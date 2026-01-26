#ifndef SPLAY_HPP
#define SPLAY_HPP

#include <algorithm>
#include <stdexcept>

template<typename T>
class Splay {
private:
    struct Node {
        T key;
        Node* left;
        Node* right;
        Node* parent;

        Node(const T& k) : key(k), left(nullptr), right(nullptr), parent(nullptr) {}
    };

    Node* root;

    void rotateLeft(Node* x);
    void rotateRight(Node* x);
    void splay(Node* x);
    Node* findNode(const T& key) const;
    Node* subtreeMin(Node* x) const;
    Node* subtreeMax(Node* x) const;
    int getHeight(Node* node) const;
    void clearTree(Node* node);

public:
    Splay() : root(nullptr) {}
    ~Splay() { clearTree(root); }

    void insert(const T& key);
    bool remove(const T& key);
    bool contains(const T& key);
    bool isEmpty() const { return root == nullptr; }
    int height() const { return getHeight(root); }
    T getMin() const;
    T getMax() const;
};

template<typename T>
void Splay<T>::rotateLeft(Node* x) {
    Node* y = x->right;
    x->right = y->left;

    if (y->left) {
        y->left->parent = x;
    }

    y->parent = x->parent;

    if (!x->parent) {
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
void Splay<T>::rotateRight(Node* x) {
    Node* y = x->left;
    x->left = y->right;

    if (y->right) {
        y->right->parent = x;
    }

    y->parent = x->parent;

    if (!x->parent) {
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
void Splay<T>::splay(Node* x) {
    while (x->parent) {
        if (!x->parent->parent) {
            // Zig step
            if (x == x->parent->left) {
                rotateRight(x->parent);
            } else {
                rotateLeft(x->parent);
            }
        } else if (x == x->parent->left && x->parent == x->parent->parent->left) {
            // Zig-zig step
            rotateRight(x->parent->parent);
            rotateRight(x->parent);
        } else if (x == x->parent->right && x->parent == x->parent->parent->right) {
            // Zig-zig step
            rotateLeft(x->parent->parent);
            rotateLeft(x->parent);
        } else if (x == x->parent->left && x->parent == x->parent->parent->right) {
            // Zig-zag step
            rotateRight(x->parent);
            rotateLeft(x->parent);
        } else {
            // Zig-zag step
            rotateLeft(x->parent);
            rotateRight(x->parent);
        }
    }
}

template<typename T>
typename Splay<T>::Node* Splay<T>::findNode(const T& key) const {
    Node* x = root;
    while (x) {
        if (key < x->key) {
            x = x->left;
        } else if (key > x->key) {
            x = x->right;
        } else {
            return x; // Found
        }
    }
    return nullptr; // Not found
}

template<typename T>
void Splay<T>::insert(const T& key) {
    Node* z = root;
    Node* p = nullptr;

    while (z) {
        p = z;
        if (key < z->key) {
            z = z->left;
        } else if (key > z->key) {
            z = z->right;
        } else {
            splay(z); // Duplicate key, just splay it
            return;
        }
    }

    Node* node = new Node(key);
    node->parent = p;

    if (!p) {
        root = node;
    } else if (key < p->key) {
        p->left = node;
    } else {
        p->right = node;
    }

    splay(node);
}

template<typename T>
bool Splay<T>::remove(const T& key) {
    Node* node = findNode(key);
    if (!node) return false;

    splay(node);

    if (!node->left) {
        // No left child
        root = node->right;
        if (root) root->parent = nullptr;
    } else if (!node->right) {
        // No right child
        root = node->left;
        root->parent = nullptr;
    } else {
        // Both children exist
        Node* leftSubtree = node->left;
        leftSubtree->parent = nullptr;

        root = node->right;
        root->parent = nullptr;

        // Find the minimum in right subtree
        Node* minRight = subtreeMin(root);
        splay(minRight);

        // Connect the left subtree
        minRight->left = leftSubtree;
        leftSubtree->parent = minRight;
    }

    delete node;
    return true;
}

template<typename T>
bool Splay<T>::contains(const T& key) {
    Node* node = findNode(key);
    if (!node) return false;

    splay(node);
    return true;
}

template<typename T>
typename Splay<T>::Node* Splay<T>::subtreeMin(Node* x) const {
    while (x->left) {
        x = x->left;
    }
    return x;
}

template<typename T>
typename Splay<T>::Node* Splay<T>::subtreeMax(Node* x) const {
    while (x->right) {
        x = x->right;
    }
    return x;
}

template<typename T>
T Splay<T>::getMin() const {
    if (isEmpty()) {
        throw std::runtime_error("Tree is empty");
    }
    return subtreeMin(root)->key;
}

template<typename T>
T Splay<T>::getMax() const {
    if (isEmpty()) {
        throw std::runtime_error("Tree is empty");
    }
    return subtreeMax(root)->key;
}

template<typename T>
int Splay<T>::getHeight(Node* node) const {
    if (!node) return 0;

    int leftHeight = getHeight(node->left);
    int rightHeight = getHeight(node->right);

    return std::max(leftHeight, rightHeight) + 1;
}

template<typename T>
void Splay<T>::clearTree(Node* node) {
    if (!node) return;

    clearTree(node->left);
    clearTree(node->right);
    delete node;
}

#endif // SPLAY_HPP