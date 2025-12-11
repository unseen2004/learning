#ifndef TEST_DATASTRUCTURES_HPP
#define TEST_DATASTRUCTURES_HPP

#include <iostream>
#include "../include/DataStructures/RedBlackTree.hpp"
#include "../include/DataStructures/Node.hpp"
#include "../include/DataStructures/Heap.hpp"
#include "../include/DataStructures/LRU_Cache.hpp"
#include "../include/DataStructures/Stack.hpp"
#include "../include/DataStructures/Queue.hpp"
#include "../include/DataStructures/BTree.hpp"
#include "../include/DataStructures/LinkedList.hpp"
#include "../include/DataStructures/DoubleLinkedList.hpp"
#include "../include/DataStructures/BST.hpp"
#include "../include/DataStructures/AVLTree.hpp"
#include "../include/DataStructures/Trie.hpp"

inline void testRedBlackTree() {
    RedBlackTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);
    if (tree.contains(10) && tree.contains(20) && tree.contains(30)) {
        std::cout << "RedBlackTree test passed!" << std::endl;
    } else {
        std::cerr << "RedBlackTree test failed!" << std::endl;
    }
}

inline void testHeap() {
    Heap<int> heap;
    heap.insert(10);
    heap.insert(20);
    heap.insert(5);
    if (heap.deleteTop() == 5 && heap.deleteTop() == 10 && heap.deleteTop() == 20) {
        std::cout << "Heap test passed!" << std::endl;
    } else {
        std::cerr << "Heap test failed!" << std::endl;
    }
}

inline void testLRUCache() {
    LRU_Cache<int, int> cache(2);
    cache.update(1, 1);
    cache.update(2, 2);
    if (cache.get(1) == 1 && cache.get(2) == 2) {
        std::cout << "LRU_Cache test passed!" << std::endl;
    } else {
        std::cerr << "LRU_Cache test failed!" << std::endl;
    }
}

inline void testStack() {
    Stack<int> stack;
    stack.push(10);
    stack.push(20);
    if (stack.peek() == 20 && stack.size() == 2) {
        stack.pop();
        if (stack.peek() == 10 && stack.size() == 1) {
            std::cout << "Stack test passed!" << std::endl;
        } else {
            std::cerr << "Stack test failed!" << std::endl;
        }
    } else {
        std::cerr << "Stack test failed!" << std::endl;
    }
}

inline void testQueue() {
    Queue<int> queue;
    queue.enqueue(10);
    queue.enqueue(20);
    if (queue.peek() == 10 && queue.size() == 2) {
        queue.dequeue();
        if (queue.peek() == 20 && queue.size() == 1) {
            std::cout << "Queue test passed!" << std::endl;
        } else {
            std::cerr << "Queue test failed!" << std::endl;
        }
    } else {
        std::cerr << "Queue test failed!" << std::endl;
    }
}

inline void testBTree() {
    b_tree<int> tree(3);
    tree.insert(10);
    tree.insert(20);
    tree.insert(5);
    if (tree.search(10) && tree.search(20) && tree.search(5)) {
        std::cout << "BTree test passed!" << std::endl;
    } else {
        std::cerr << "BTree test failed!" << std::endl;
    }
}

inline void testLinkedList() {
    LinkedList<int> list;
    list.add(10);
    list.add(20);
    list.pop();
    if (!list.search(20) && list.search(10)) {
        std::cout << "LinkedList test passed!" << std::endl;
    } else {
        std::cerr << "LinkedList test failed!" << std::endl;
    }
}

inline void testDoubleLinkedList() {
    DoubleLinkedList<int> list;
    list.push(10);
    list.push(20);
    list.pop();
    if (!list.search(20) && list.search(10)) {
        std::cout << "DoubleLinkedList test passed!" << std::endl;
    } else {
        std::cerr << "DoubleLinkedList test failed!" << std::endl;
    }
}

inline void testBST() {
    BST<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(5);
    if (tree.contains(10) && tree.contains(20) && tree.contains(5)) {
        std::cout << "BST test passed!" << std::endl;
    } else {
        std::cerr << "BST test failed!" << std::endl;
    }
}

inline void testAVLTree() {
    AVLTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(5);
    if (tree.contains(10) && tree.contains(20) && tree.contains(5)) {
        std::cout << "AVLTree test passed!" << std::endl;
    } else {
        std::cerr << "AVLTree test failed!" << std::endl;
    }
}

inline void testTrie() {
    Trie<char> trie;
    trie.insert("hello");
    trie.insert("world");
    if (trie.search("hello") && trie.search("world")) {
        std::cout << "Trie test passed!" << std::endl;
    } else {
        std::cerr << "Trie test failed!" << std::endl;
    }
}

inline void runDataStructureTests() {
    testRedBlackTree();
    testHeap();
    testLRUCache();
    testStack();
    testQueue();
    testBTree();
    testLinkedList();
    testDoubleLinkedList();
    testBST();
    testAVLTree();
    testTrie();
}

#endif // TEST_DATASTRUCTURES_HPP
