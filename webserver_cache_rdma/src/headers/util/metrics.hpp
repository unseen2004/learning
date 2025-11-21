#pragma once
#include <atomic>
#include <string>

struct Metrics {
  std::atomic<unsigned long long> requests_total{0};
  std::atomic<unsigned long long> responses_2xx{0};
  std::atomic<unsigned long long> responses_4xx{0};
  std::atomic<unsigned long long> responses_5xx{0};
  std::atomic<unsigned long long> cache_hits{0};
  std::atomic<unsigned long long> cache_misses{0};
  std::atomic<unsigned long long> bytes_served{0};

  // RDMA counters
  std::atomic<unsigned long long> rdma_reqs{0};
  std::atomic<unsigned long long> rdma_ok{0};
  std::atomic<unsigned long long> rdma_err{0};
  std::atomic<unsigned long long> rdma_bytes{0};

  static Metrics& instance() {
    static Metrics m;
    return m;
  }

  void reset() {
    requests_total = 0;
    responses_2xx = 0;
    responses_4xx = 0;
    responses_5xx = 0;
    cache_hits = 0;
    cache_misses = 0;
    bytes_served = 0;
    rdma_reqs = 0;
    rdma_ok = 0;
    rdma_err = 0;
    rdma_bytes = 0;
  }

  std::string render_text() const {
    return
      "requests_total " + std::to_string(requests_total.load()) + "\n" +
      "responses_2xx " + std::to_string(responses_2xx.load()) + "\n" +
      "responses_4xx " + std::to_string(responses_4xx.load()) + "\n" +
      "responses_5xx " + std::to_string(responses_5xx.load()) + "\n" +
      "cache_hits " + std::to_string(cache_hits.load()) + "\n" +
      "cache_misses " + std::to_string(cache_misses.load()) + "\n" +
      "bytes_served " + std::to_string(bytes_served.load()) + "\n" +
      "rdma_requests " + std::to_string(rdma_reqs.load()) + "\n" +
      "rdma_ok " + std::to_string(rdma_ok.load()) + "\n" +
      "rdma_err " + std::to_string(rdma_err.load()) + "\n" +
      "rdma_bytes " + std::to_string(rdma_bytes.load()) + "\n";
  }
};