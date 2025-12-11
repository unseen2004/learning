# C++ Algorithms & Data Structures Library

<p align="center">
  <img src="https://img.shields.io/badge/Library-C++20-blue?style=for-the-badge" alt="C++20" />
  <img src="https://img.shields.io/badge/CMake-3.20+-blue?style=for-the-badge" alt="CMake" />
</p>

---

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [License](#license)

---

## Overview
This repository provides a comprehensive C++ library implementing various algorithms and data structures. It is designed with flexibility in mind and offers two versions:
- **Default Branch:** Traditional header-only implementation using `.hpp` files.

## Features
- **Algorithms:**  
  - Bloom Filter  
  - Ford Fulkerson  
  - Graph Algorithms  
  - Minimum Spanning Trees (Kruskal, Prim)  
  - Sorting Algorithms: Bucket, Insertion, Merge, Quick, Two-Way Merge, Dual Pivot Quick, Hybrid Quick  
  - Tree Algorithms  
  - Union-Find
  - Select
  - Randomized Select
  - Select Variant
- **Data Structures:**  
  - AVL Tree  
  - Binary Search Tree (BST)  
  - B-Tree  
  - Doubly Linked List  
  - Heap  
  - Linked List  
  - LRU Cache  
  - Queue  
  - Red-Black Tree  
  - Stack  
  - Trie
  - Binomial Heap
  - RBBST
  - Splay

---


## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## How to Run

To compile and run the tests, you can use the following command from the `algo_lib` directory:

```bash
g++ -std=c++20 -Iinclude tests/main_test.cpp -o main_test
./main_test
```
