#ifndef B_TREE_HPP
#define B_TREE_HPP

#include <vector>
#include <iostream>
#include "Node.hpp"

template<typename T>
class b_tree {
private:
    struct b_node {
        std::vector<T> m_keys;
        std::vector<b_node *> m_children;
        bool m_is_leaf;
        int m_degree;

        b_node(int degree, bool is_leaf);
    };

    b_node *m_root;
    int m_degree;

public:
    b_tree(int degree);

    void insert(const T &key);

    void remove(const T &key);

    bool search(const T &key) const;

    void print() const;

private:
    void insert_non_full(b_node *current_node, const T &key);

    void split_child(b_node *parent, int index, b_node *child);

    void remove_key(b_node *current_node, const T &key);

    T get_predecessor(b_node *current_node);

    T get_successor(b_node *current_node);

    void fill_node(b_node *current_node, int index);

    void borrow_from_prev(b_node *current_node, int index);

    void borrow_from_next(b_node *current_node, int index);

    void merge_nodes(b_node *current_node, int index);

    void print_node(b_node *current_node, int level) const;

    bool search_node(b_node *current_node, const T &key) const;
};

template<typename T>
b_tree<T>::b_node::b_node(int degree, bool is_leaf)
    : m_degree(degree), m_is_leaf(is_leaf) {
}

template<typename T>
b_tree<T>::b_tree(int degree) : m_root(nullptr), m_degree(degree) {
}

template<typename T>
void b_tree<T>::insert(const T &key) {
    if (!m_root) {
        m_root = new b_node(m_degree, true);
        m_root->m_keys.push_back(key);
    } else {
        if (m_root->m_keys.size() == 2 * m_degree - 1) {
            b_node *new_root = new b_node(m_degree, false);
            new_root->m_children.push_back(m_root);
            split_child(new_root, 0, m_root);
            m_root = new_root;
            insert_non_full(m_root, key);
        } else {
            insert_non_full(m_root, key);
        }
    }
}

template<typename T>
void b_tree<T>::remove(const T &key) {
    if (!m_root) return;
    remove_key(m_root, key);
    if (m_root->m_keys.empty()) {
        b_node *old_root = m_root;
        m_root = m_root->m_is_leaf ? nullptr : m_root->m_children[0];
        delete old_root;
    }
}

template<typename T>
bool b_tree<T>::search(const T &key) const {
    return search_node(m_root, key);
}

template<typename T>
void b_tree<T>::print() const {
    if (m_root) print_node(m_root, 0);
}

template<typename T>
void b_tree<T>::insert_non_full(b_node *current_node, const T &key) {
    int i = current_node->m_keys.size() - 1;
    if (current_node->m_is_leaf) {
        current_node->m_keys.push_back(T());
        while (i >= 0 && key < current_node->m_keys[i]) {
            current_node->m_keys[i + 1] = current_node->m_keys[i];
            i--;
        }
        current_node->m_keys[i + 1] = key;
    } else {
        while (i >= 0 && key < current_node->m_keys[i]) i--;
        if (current_node->m_children[i + 1]->m_keys.size() == 2 * m_degree - 1) {
            split_child(current_node, i + 1, current_node->m_children[i + 1]);
            if (key > current_node->m_keys[i + 1]) i++;
        }
        insert_non_full(current_node->m_children[i + 1], key);
    }
}

template<typename T>
void b_tree<T>::split_child(b_node *parent, int index, b_node *child) {
    b_node *new_child = new b_node(child->m_degree, child->m_is_leaf);
    for (int j = 0; j < m_degree - 1; j++)
        new_child->m_keys.push_back(child->m_keys[j + m_degree]);
    if (!child->m_is_leaf) {
        for (int j = 0; j < m_degree; j++)
            new_child->m_children.push_back(child->m_children[j + m_degree]);
    }
    parent->m_children.insert(parent->m_children.begin() + index + 1, new_child);
    parent->m_keys.insert(parent->m_keys.begin() + index, child->m_keys[m_degree - 1]);
    child->m_keys.resize(m_degree - 1);
    if (!child->m_is_leaf) child->m_children.resize(m_degree);
}

template<typename T>
void b_tree<T>::remove_key(b_node *current_node, const T &key) {
    int idx = 0;
    while (idx < current_node->m_keys.size() && key > current_node->m_keys[idx]) idx++;
    if (idx < current_node->m_keys.size() && key == current_node->m_keys[idx]) {
        if (current_node->m_is_leaf) {
            current_node->m_keys.erase(current_node->m_keys.begin() + idx);
        } else {
            if (current_node->m_children[idx]->m_keys.size() >= m_degree) {
                T pred = get_predecessor(current_node->m_children[idx]);
                current_node->m_keys[idx] = pred;
                remove_key(current_node->m_children[idx], pred);
            } else if (current_node->m_children[idx + 1]->m_keys.size() >= m_degree) {
                T succ = get_successor(current_node->m_children[idx + 1]);
                current_node->m_keys[idx] = succ;
                remove_key(current_node->m_children[idx + 1], succ);
            } else {
                merge_nodes(current_node, idx);
                remove_key(current_node->m_children[idx], key);
            }
        }
    } else {
        if (current_node->m_children[idx]->m_keys.size() < m_degree) fill_node(current_node, idx);
        if (idx > current_node->m_keys.size()) {
            remove_key(current_node->m_children[idx - 1], key);
        } else {
            remove_key(current_node->m_children[idx], key);
        }
    }
}

template<typename T>
T b_tree<T>::get_predecessor(b_node *current_node) {
    while (!current_node->m_is_leaf)
        current_node = current_node->m_children[current_node->m_keys.size()];
    return current_node->m_keys.back();
}

template<typename T>
T b_tree<T>::get_successor(b_node *current_node) {
    while (!current_node->m_is_leaf)
        current_node = current_node->m_children[0];
    return current_node->m_keys[0];
}

template<typename T>
void b_tree<T>::fill_node(b_node *current_node, int index) {
    if (index != 0 && current_node->m_children[index - 1]->m_keys.size() >= m_degree) {
        borrow_from_prev(current_node, index);
    } else if (index != current_node->m_keys.size() && current_node->m_children[index + 1]->m_keys.size() >= m_degree) {
        borrow_from_next(current_node, index);
    } else {
        if (index != current_node->m_keys.size()) {
            merge_nodes(current_node, index);
        } else {
            merge_nodes(current_node, index - 1);
        }
    }
}

template<typename T>
void b_tree<T>::borrow_from_prev(b_node *current_node, int index) {
    b_node *child = current_node->m_children[index];
    b_node *sibling = current_node->m_children[index - 1];
    child->m_keys.insert(child->m_keys.begin(), current_node->m_keys[index - 1]);
    if (!child->m_is_leaf) {
        child->m_children.insert(child->m_children.begin(), sibling->m_children.back());
    }
    current_node->m_keys[index - 1] = sibling->m_keys.back();
    sibling->m_keys.pop_back();
    if (!sibling->m_is_leaf) {
        sibling->m_children.pop_back();
    }
}

template<typename T>
void b_tree<T>::borrow_from_next(b_node *current_node, int index) {
    b_node *child = current_node->m_children[index];
    b_node *sibling = current_node->m_children[index + 1];
    child->m_keys.push_back(current_node->m_keys[index]);
    if (!child->m_is_leaf) {
        child->m_children.push_back(sibling->m_children[0]);
    }
    current_node->m_keys[index] = sibling->m_keys[0];
    sibling->m_keys.erase(sibling->m_keys.begin());
    if (!sibling->m_is_leaf) {
        sibling->m_children.erase(sibling->m_children.begin());
    }
}

template<typename T>
void b_tree<T>::merge_nodes(b_node *current_node, int index) {
    b_node *child = current_node->m_children[index];
    b_node *sibling = current_node->m_children[index + 1];
    child->m_keys.push_back(current_node->m_keys[index]);
    for (int i = 0; i < static_cast<int>(sibling->m_keys.size()); ++i) {
        child->m_keys.push_back(sibling->m_keys[i]);
    }
    if (!child->m_is_leaf) {
        for (int i = 0; i < static_cast<int>(sibling->m_children.size()); ++i) {
            child->m_children.push_back(sibling->m_children[i]);
        }
    }
    current_node->m_keys.erase(current_node->m_keys.begin() + index);
    current_node->m_children.erase(current_node->m_children.begin() + index + 1);
    delete sibling;
}

template<typename T>
void b_tree<T>::print_node(b_node *current_node, int level) const {
    if (!current_node) return;
    for (int i = 0; i < static_cast<int>(current_node->m_keys.size()); ++i) {
        print_node(current_node->m_children[i], level + 1);
        std::cout << std::string(level * 2, ' ') << current_node->m_keys[i] << std::endl;
    }
    print_node(current_node->m_children[current_node->m_keys.size()], level + 1);
}

template<typename T>
bool b_tree<T>::search_node(b_node *current_node, const T &key) const {
    if (!current_node) return false;
    int idx = 0;
    while (idx < static_cast<int>(current_node->m_keys.size()) && key > current_node->m_keys[idx]) idx++;
    if (idx < static_cast<int>(current_node->m_keys.size()) && key == current_node->m_keys[idx]) return true;
    if (current_node->m_is_leaf) return false;
    return search_node(current_node->m_children[idx], key);
}

#endif // B_TREE_HPP
