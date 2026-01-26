#ifndef GRAPH_ALGORITHMS_HPP
#define GRAPH_ALGORITHMS_HPP

#include <vector>
#include <limits>
#include <queue>

#include "../DataStructures/Queue.hpp"

template<typename T>
std::vector<T> graphBFS(const std::vector<std::vector<T> > &graph, T source, T target) {
    std::vector<bool> seen(graph.size(), false);
    std::vector<T> prev(graph.size(), -1);
    seen[source] = true;
    Queue<T> q;
    q.enqueue(source);
    while (!q.is_empty()) {
        T curr = q.peek();
        q.dequeue();
        if (curr == target) break;
        for (size_t i = 0; i < graph[curr].size(); ++i) {
            if (graph[curr][i] == 0) continue;
            if (!seen[i]) {
                seen[i] = true;
                prev[i] = curr;
                q.enqueue(i);
            }
        }
    }
    std::vector<T> out;
    if (prev[target] == -1) return out;
    for (T at = target; at != -1; at = prev[at]) {
        out.push_back(at);
    }
    // Custom reverse implementation
    for (size_t i = 0; i < out.size() / 2; ++i) {
        T temp = out[i];
        out[i] = out[out.size() - 1 - i];
        out[out.size() - 1 - i] = temp;
    }
    return out;
}

template<typename T>
bool graphDFSUtil(const std::vector<std::vector<std::pair<T, T> > > &graph, T curr, T target, std::vector<bool> &seen,
                  std::vector<T> &path) {
    if (seen[curr]) return false;
    seen[curr] = true;
    path.push_back(curr);
    if (curr == target) return true;
    for (const auto &edge: graph[curr]) {
        if (graphDFSUtil(graph, edge.first, target, seen, path)) return true;
    }
    path.pop_back();
    return false;
}

template<typename T>
std::vector<T> graphDFS(const std::vector<std::vector<std::pair<T, T> > > &graph, T source, T target) {
    std::vector<bool> seen(graph.size(), false);
    std::vector<T> path;
    graphDFSUtil(graph, source, target, seen, path);
    return path;
}

template<typename T>
void dijkstraAlgorithm(T src, const std::vector<std::vector<std::pair<T, T> > > &graph) {
    T n = graph.size();
    std::vector<T> dist(n, std::numeric_limits<T>::max());
    std::vector<bool> seen(n, false);
    dist[src] = 0;
    using Pair = std::pair<T, T>;
    std::priority_queue<Pair, std::vector<Pair>, std::greater<> > pq;
    pq.push({0, src});
    while (!pq.empty()) {
        T currDist = pq.top().first;
        T node = pq.top().second;
        pq.pop();
        if (seen[node]) continue;
        seen[node] = true;
        for (const auto &edge: graph[node]) {
            T v = edge.first;
            T weight = edge.second;
            if (!seen[v] && dist[v] > currDist + weight) {
                dist[v] = currDist + weight;
                pq.push({dist[v], v});
            }
        }
    }
}

#endif // GRAPH_ALGORITHMS_HPP
