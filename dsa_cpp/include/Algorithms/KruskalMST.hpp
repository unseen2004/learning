#ifndef KRUSKALMST_HPP
#define KRUSKALMST_HPP

#include <vector>
#include <utility>
#include "UnionFind.hpp"

template<typename W, typename T>
void customSort(std::vector<std::pair<W, std::pair<T, T> > > &edges) {
    for (size_t i = 0; i < edges.size() - 1; ++i) {
        for (size_t j = 0; j < edges.size() - i - 1; ++j) {
            if (edges[j] > edges[j + 1]) {
                auto temp = edges[j];
                edges[j] = edges[j + 1];
                edges[j + 1] = temp;
            }
        }
    }
}

template<typename W, typename T>
std::vector<std::pair<T, T> > kruskalMST(std::vector<std::pair<W, std::pair<T, T> > > &edges, T n) {
    customSort(edges);
    UnionFind<T> uf(n);
    std::vector<std::pair<T, T> > mst;
    for (auto &[weight, edge]: edges) {
        T u = edge.first;
        T v = edge.second;
        if (uf.find(u) != uf.find(v)) {
            uf.unionSets(u, v);
            mst.push_back({u, v});
        }
    }
    return mst;
}

#endif // KRUSKALMST_HPP
