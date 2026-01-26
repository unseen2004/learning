#ifndef FORDFULKERSON_HPP
#define FORDFULKERSON_HPP

#include <vector>
#include <limits>
#include <iostream>
#include "../DataStructures/Queue.hpp"

template<typename FlowType>
class FordFulkerson {
private:
    int m_numVertices;
    std::vector<std::vector<FlowType> > m_capacity;
    std::vector<std::vector<FlowType> > m_residual;

public:
    FordFulkerson(int numVertices);

    void addEdge(int u, int v, FlowType capacity);

    FlowType maxFlow(int source, int sink);

    void minCut(int source);

private:
    bool bfs(int source, int sink, std::vector<int> &parent);
};

template<typename FlowType>
FordFulkerson<FlowType>::FordFulkerson(int numVertices) : m_numVertices(numVertices) {
    m_capacity = std::vector<std::vector<FlowType> >(
        numVertices, std::vector<FlowType>(numVertices, static_cast<FlowType>(0))
    );
    m_residual = std::vector<std::vector<FlowType> >(
        numVertices, std::vector<FlowType>(numVertices, static_cast<FlowType>(0))
    );
}

template<typename FlowType>
void FordFulkerson<FlowType>::addEdge(int u, int v, FlowType capacity) {
    m_capacity[u][v] = capacity;
    m_residual[u][v] = capacity;
}

template<typename FlowType>
FlowType FordFulkerson<FlowType>::maxFlow(int source, int sink) {
    std::vector<int> parent(m_numVertices);
    FlowType maxFlow = static_cast<FlowType>(0);

    while (bfs(source, sink, parent)) {
        FlowType pathFlow = std::numeric_limits<FlowType>::max();

        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            pathFlow = std::min(pathFlow, m_residual[u][v]);
        }

        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            m_residual[u][v] -= pathFlow;
            m_residual[v][u] += pathFlow;
        }

        maxFlow += pathFlow;
    }

    return maxFlow;
}

template<typename FlowType>
void FordFulkerson<FlowType>::minCut(int source) {
    std::vector<bool> visited(m_numVertices, false);
    Queue<int> q;
    q.enqueue(source);
    visited[source] = true;

    while (!q.is_empty()) {
        int u = q.peek();
        q.dequeue();
        for (int v = 0; v < m_numVertices; v++) {
            if (m_residual[u][v] > 0 && !visited[v]) {
                visited[v] = true;
                q.enqueue(v);
            }
        }
    }

    for (int u = 0; u < m_numVertices; u++) {
        for (int v = 0; v < m_numVertices; v++) {
            if (visited[u] && !visited[v] && m_capacity[u][v] > 0) {
                std::cout << u << " - " << v << " | Capacity: " << m_capacity[u][v] << std::endl;
            }
        }
    }
}

template<typename FlowType>
bool FordFulkerson<FlowType>::bfs(int source, int sink, std::vector<int> &parent) {
    std::fill(parent.begin(), parent.end(), -1);
    std::vector<bool> visited(m_numVertices, false);
    Queue<int> q;
    q.enqueue(source);
    visited[source] = true;

    while (!q.is_empty()) {
        int u = q.peek();
        q.dequeue();

        for (int v = 0; v < m_numVertices; v++) {
            if (!visited[v] && m_residual[u][v] > 0) {
                q.enqueue(v);
                visited[v] = true;
                parent[v] = u;
                if (v == sink) return true;
            }
        }
    }

    return false;
}

#endif // FORDFULKERSON_HPP
