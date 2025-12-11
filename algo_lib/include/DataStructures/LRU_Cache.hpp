#ifndef LRU_CACHE_HPP
#define LRU_CACHE_HPP

#include <unordered_map>
#include <stdexcept>
#include "Node.hpp"

template<typename K, typename V>
class LRU_Cache {
    using NodePtr = Node<V> *;

    int m_length;
    NodePtr m_head;
    NodePtr m_tail;
    const int m_capacity;
    std::unordered_map<K, NodePtr> look_up;
    std::unordered_map<NodePtr, K> reverse_look_up;

public:
    LRU_Cache(int capacity = 10);

    ~LRU_Cache();

    void update(K key, V value);

    V get(K key);

private:
    void detach(NodePtr node);

    void prepend(NodePtr node);

    void trimCache();
};

template<typename K, typename V>
LRU_Cache<K, V>::LRU_Cache(int capacity)
    : m_capacity(capacity), m_length(0), m_head(nullptr), m_tail(nullptr) {
}

template<typename K, typename V>
LRU_Cache<K, V>::~LRU_Cache() {
    NodePtr current = m_head;
    while (current) {
        NodePtr to_delete = current;
        current = current->right;
        delete to_delete;
    }
}

template<typename K, typename V>
void LRU_Cache<K, V>::update(K key, V value) {
    if (look_up.find(key) != look_up.end()) {
        NodePtr node = look_up[key];
        node->val = value;
        detach(node);
        prepend(node);
    } else {
        if (m_length == m_capacity) {
            trimCache();
        }
        NodePtr node = new Node<V>(value);
        prepend(node);
        look_up[key] = node;
        reverse_look_up[node] = key;
        ++m_length;
    }
}

template<typename K, typename V>
V LRU_Cache<K, V>::get(K key) {
    if (look_up.find(key) != look_up.end()) {
        NodePtr node = look_up[key];
        detach(node);
        prepend(node);
        return node->val;
    }
    throw std::out_of_range("Key not found");
}

template<typename K, typename V>
void LRU_Cache<K, V>::detach(NodePtr node) {
    if (node->right) {
        node->right->left = node->left;
    }
    if (node->left) {
        node->left->right = node->right;
    }
    if (node == m_head) {
        m_head = node->right;
    }
    if (node == m_tail) {
        m_tail = node->left;
    }
    node->right = nullptr;
    node->left = nullptr;
}

template<typename K, typename V>
void LRU_Cache<K, V>::prepend(NodePtr node) {
    if (!m_head) {
        m_head = m_tail = node;
        return;
    }
    node->right = m_head;
    m_head->left = node;
    m_head = node;
}

template<typename K, typename V>
void LRU_Cache<K, V>::trimCache() {
    if (!m_tail) return;

    NodePtr node_to_remove = m_tail;
    K key_to_remove = reverse_look_up[node_to_remove];

    detach(node_to_remove);

    look_up.erase(key_to_remove);
    reverse_look_up.erase(node_to_remove);

    delete node_to_remove;
    --m_length;
}

#endif // LRU_CACHE_HPP