#ifndef PRIMMST_HPP
#define PRIMMST_HPP

#include <vector>
#include <iostream>
#include <limits>
#include "../DataStructures/Queue.hpp"

template<typename T>
class PrimMST {
private:
    std::vector<std::vector<std::pair<int, T> > > m_graph;
    std::vector<T> m_key;
    std::vector<int> m_parent;
    std::vector<bool> m_inMST;

public:
    PrimMST(const std::vector<std::vector<std::pair<int, T> > > &graph); // Constructor

    void run();

    void printMST(std::ostream &os); // Print the MST
};

template<typename T>
PrimMST<T>::PrimMST(const std::vector<std::vector<std::pair<int, T> > > &graph) : m_graph(graph) {
    int n = static_cast<int>(m_graph.size());
    m_key.resize(n, std::numeric_limits<T>::max());
    m_parent.resize(n, -1);
    m_inMST.resize(n, false);
}

template<typename T>
void PrimMST<T>::run() {
    Queue<std::pair<T, int> > pq;
    m_key[0] = 0;
    pq.enqueue({0, 0});

    while (!pq.is_empty()) {
        int u = pq.peek().second;
        pq.dequeue();

        if (m_inMST[u]) continue;

        m_inMST[u] = true;

        for (const auto &edge: m_graph[u]) {
            int v = edge.first;
            T weight = edge.second;

            if (!m_inMST[v] && weight < m_key[v]) {
                m_key[v] = weight;
                m_parent[v] = u;
                pq.enqueue({m_key[v], v});
            }
        }
    }
}

template<typename T>
void PrimMST<T>::printMST(std::ostream &os) {
    for (int i = 1; i < static_cast<int>(m_graph.size()); i++) {
        os << "Edge: " << m_parent[i] << " - " << i << " | Weight: " << m_key[i] << "\n";
    }
}

#endif // PRIMMST_HPP
