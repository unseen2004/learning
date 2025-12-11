#ifndef TRIE_HPP
#define TRIE_HPP

#include <unordered_map>
#include <string>
#include <iostream>

template<typename KeyType>
class TrieNode {
public:
    std::unordered_map<KeyType, TrieNode *> children;
    bool end_of_word;

    TrieNode() : end_of_word(false) {
    }

    ~TrieNode();
};

template<typename KeyType>
TrieNode<KeyType>::~TrieNode() {
    for (auto &pair: children) {
        delete pair.second;
    }
}

template<typename KeyType>
class Trie {
private:
    TrieNode<KeyType> *m_root;

    void print(TrieNode<KeyType> *node, std::basic_string<KeyType> prefix) const;

public:
    Trie();

    ~Trie();

    void insert(const std::basic_string<KeyType> &word);

    bool search(const std::basic_string<KeyType> &word) const;

    void erase(const std::basic_string<KeyType> &word);

    void print() const;
};

template<typename KeyType>
Trie<KeyType>::Trie() {
    m_root = new TrieNode<KeyType>();
}

template<typename KeyType>
Trie<KeyType>::~Trie() {
    delete m_root;
}

template<typename KeyType>
void Trie<KeyType>::insert(const std::basic_string<KeyType> &word) {
    TrieNode<KeyType> *node = m_root;
    for (const KeyType &key: word) {
        if (node->children.find(key) == node->children.end()) {
            node->children[key] = new TrieNode<KeyType>();
        }
        node = node->children[key];
    }
    node->end_of_word = true;
}

template<typename KeyType>
bool Trie<KeyType>::search(const std::basic_string<KeyType> &word) const {
    TrieNode<KeyType> *node = m_root;
    for (const KeyType &key: word) {
        if (node->children.find(key) == node->children.end()) {
            return false;
        }
        node = node->children[key];
    }
    return node->end_of_word;
}

template<typename KeyType>
void Trie<KeyType>::erase(const std::basic_string<KeyType> &word) {
    TrieNode<KeyType> *node = m_root;
    for (const KeyType &key: word) {
        if (node->children.find(key) == node->children.end()) {
            return;
        }
        node = node->children[key];
    }
    if (node->end_of_word) {
        node->end_of_word = false;
    }
}

template<typename KeyType>
void Trie<KeyType>::print() const {
    print(m_root, std::basic_string<KeyType>());
}

template<typename KeyType>
void Trie<KeyType>::print(TrieNode<KeyType> *node, std::basic_string<KeyType> prefix) const {
    if (node->end_of_word) {
        std::cout << prefix << std::endl;
    }
    for (const auto &pair: node->children) {
        print(pair.second, prefix + pair.first);
    }
}


#endif // TRIE_HPP
