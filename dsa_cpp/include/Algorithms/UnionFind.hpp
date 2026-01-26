#ifndef UNIONFIND_HPP
#define UNIONFIND_HPP

#include <vector>

template<typename T>
class UnionFind {
private:
    std::vector<T> m_parent;
    std::vector<T> m_rank;

public:
    UnionFind(T n);

    T find(T u);

    void unionSets(T u, T v);
};

template<typename T>
UnionFind<T>::UnionFind(T n) {
    m_parent.resize(n);
    m_rank.resize(n, 0);
    for (T i = 0; i < n; ++i) m_parent[i] = i;
}

template<typename T>
T UnionFind<T>::find(T u) {
    if (m_parent[u] != u) m_parent[u] = find(m_parent[u]);
    return m_parent[u];
}

template<typename T>
void UnionFind<T>::unionSets(T u, T v) {
    T rootU = find(u);
    T rootV = find(v);
    if (rootU != rootV) {
        if (m_rank[rootU] > m_rank[rootV]) m_parent[rootV] = rootU;
        else if (m_rank[rootU] < m_rank[rootV]) m_parent[rootU] = rootV;
        else {
            m_parent[rootV] = rootU;
            m_rank[rootU]++;
        }
    }
}

#endif // UNIONFIND_HPP
