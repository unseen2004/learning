#pragma once
#ifdef ENABLE_RDMA

#include <memory>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <mutex>
#include <unordered_set>

#include <rdma/rdma_cma.h>
#include <infiniband/verbs.h>

#include "../util/config.hpp"
#include "../cache/lru_cache.hpp"

namespace rdma_fast {

struct RDMAConfig {
  std::string bind_addr = "0.0.0.0";
  uint16_t port = 7471;
  int cq_depth = 512;
  int poller_threads = 1;
};

class Connection;

class RDMAServer {
public:
  RDMAServer(const RDMAConfig& cfg, const Config& app_cfg, std::shared_ptr<LRUCache> cache);
  ~RDMAServer();

  void start();
  void stop();

  // Dispatch from poller
  void handle_wc(const ibv_wc& wc);

private:
  void cm_event_loop_();
  void cq_poller_loop_();

  RDMAConfig cfg_;
  Config app_cfg_{};
  std::shared_ptr<LRUCache> cache_{};

  std::atomic<bool> running_{false};

  rdma_event_channel* ec_ = nullptr;
  rdma_cm_id* listen_id_ = nullptr;

  ibv_pd* pd_ = nullptr;
  ibv_comp_channel* comp_ch_ = nullptr;
  ibv_cq* cq_ = nullptr;

  std::thread cm_thread_;
  std::vector<std::thread> pollers_;

  // Track live connections to keep them alive
  std::mutex conns_mtx_;
  std::unordered_set<std::shared_ptr<Connection>> conns_;
};

} // namespace rdma_fast

#endif // ENABLE_RDMA