#ifndef TEST_ALGORITHMS_HPP
#define TEST_ALGORITHMS_HPP

#include <vector>
#include <iostream>
#include <sstream>
#include "../include/Algorithms/KruskalMST.hpp"
#include "../include/Algorithms/TreeAlgorithms.hpp"
#include "../include/Algorithms/PrimMST.hpp"
#include "../include/Algorithms/BloomFilter.hpp"
#include "../include/Algorithms/GraphAlgorithms.hpp"
#include "../include/Algorithms/FordFulkerson.hpp"
#include "../include/Algorithms/UnionFind.hpp"

inline void testKruskalMST() {
    std::vector<std::pair<int, std::pair<int, int>>> edges = {
        {1, {0, 1}}, {2, {1, 2}}, {3, {0, 2}}
    };
    auto mst = kruskalMST(edges, 3);
    if (mst.size() == 2) {
        std::cout << "KruskalMST test passed!" << std::endl;
    } else {
        std::cerr << "KruskalMST test failed!" << std::endl;
    }
}

inline void testTreeAlgorithms() {
    Node<int> *root = new Node<int>(1);
    root->left = new Node<int>(2);
    root->right = new Node<int>(3);
    std::vector<int> path = preOrderTraversal(root);
    if (path == std::vector<int>{1, 2, 3}) {
        std::cout << "TreeAlgorithms test passed!" << std::endl;
    } else {
        std::cerr << "TreeAlgorithms test failed!" << std::endl;
    }
}

inline void testPrimMST() {
    std::vector<std::vector<std::pair<int, int>>> graph = {
        {{1, 1}, {2, 2}}, {{0, 1}, {2, 3}}, {{0, 2}, {1, 3}}
    };
    PrimMST<int> prim(graph);
    prim.run();
    std::ostringstream os;
    prim.printMST(os);
    if (!os.str().empty()) {
        std::cout << "PrimMST test passed!" << std::endl;
    } else {
        std::cerr << "PrimMST test failed!" << std::endl;
    }
}

inline void testBloomFilter() {
    BloomFilter<int> bloom(100, 3);
    bloom.add(1);
    if (bloom.mightContain(1) && !bloom.mightContain(2)) {
        std::cout << "BloomFilter test passed!" << std::endl;
    } else {
        std::cerr << "BloomFilter test failed!" << std::endl;
    }
}

inline void testGraphAlgorithms() {
    std::vector<std::vector<int>> graph = {
        {0, 1, 0}, {1, 0, 1}, {0, 1, 0}
    };
    auto path = graphBFS(graph, 0, 2);
    if (path == std::vector<int>{0, 1, 2}) {
        std::cout << "GraphAlgorithms test passed!" << std::endl;
    } else {
        std::cerr << "GraphAlgorithms test failed!" << std::endl;
    }
}

inline void testFordFulkerson() {
    FordFulkerson<int> ff(4);
    ff.addEdge(0, 1, 1000);
    ff.addEdge(1, 2, 1);
    ff.addEdge(2, 3, 1000);
    ff.addEdge(0, 2, 1000);
    ff.addEdge(1, 3, 1000);
    int maxFlow = ff.maxFlow(0, 3);
    if (maxFlow == 2000) {
        std::cout << "FordFulkerson test passed!" << std::endl;
    } else {
        std::cerr << "FordFulkerson test failed!" << std::endl;
    }
}

inline void testUnionFind() {
    UnionFind<int> uf(5);
    uf.unionSets(0, 1);
    uf.unionSets(1, 2);
    if (uf.find(0) == uf.find(2) && uf.find(0) != uf.find(3)) {
        std::cout << "UnionFind test passed!" << std::endl;
    } else {
        std::cerr << "UnionFind test failed!" << std::endl;
    }
}

inline void runAlgorithmTests() {
    testKruskalMST();
    testTreeAlgorithms();
    testPrimMST();
    testBloomFilter();
    testGraphAlgorithms();
    testFordFulkerson();
    testUnionFind();
}

#endif // TEST_ALGORITHMS_HPP