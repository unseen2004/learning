#pragma once
#ifdef ENABLE_RDMA
#include <infiniband/verbs.h>
#include <rdma/rdma_cma.h>
#include <memory>
#include <vector>
#include <mutex>
#include <deque>
#include <atomic>

#include "../util/config.hpp"
#include "../cache/lru_cache.hpp"

namespace rdma_fast {

class RDMAServer; // fwd

struct Buffer {
  char* data = nullptr;
  size_t size = 0;
  ibv_mr* mr = nullptr;

  Buffer() = default;
  Buffer(ibv_pd* pd, size_t n);
  ~Buffer();
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;
  Buffer(Buffer&&) noexcept = delete;
  Buffer& operator=(Buffer&&) noexcept = delete;
};

struct WorkBase {
  std::shared_ptr<class Connection> conn; // keep connection alive until completion
  Buffer* buf = nullptr;
  explicit WorkBase(std::shared_ptr<class Connection> c, Buffer* b) : conn(std::move(c)), buf(b) {}
  virtual ~WorkBase() = default;
};

struct RecvWork : WorkBase {
  using WorkBase::WorkBase;
};

struct SendWork : WorkBase {
  using WorkBase::WorkBase;
};

class Connection : public std::enable_shared_from_this<Connection> {
public:
  Connection(RDMAServer* srv,
             rdma_cm_id* id,
             ibv_pd* pd,
             ibv_cq* cq,
             const Config& cfg,
             std::shared_ptr<LRUCache> cache);
  ~Connection();

  // Setup RECVs and ready to accept
  bool init();

  // Post RECV buffers (called on init and after each completion)
  bool post_recvs(int count);

  // Called by poller on completions
  void on_recv_complete(RecvWork* w, uint32_t byte_len);
  void on_send_complete(SendWork* w);

  // Cleanup
  void close();

  uint32_t qp_num() const { return id_->qp ? id_->qp->qp_num : 0; }

private:
  // Protocol handling
  void handle_ping();
  void handle_get(const std::string& url_path);

  // Send helpers
  bool send_header(uint16_t status, uint64_t content_len, uint32_t chunk);
  bool send_body_chunks(const std::shared_ptr<std::vector<uint8_t>>& body, uint32_t chunk);

  // Flow control
  void try_post_more_sends_locked();

  // Utilities
  Buffer* make_buffer(size_t n);
  void reclaim(Buffer* b);

  RDMAServer* server_;
  rdma_cm_id* id_;
  ibv_pd* pd_;
  ibv_cq* cq_;
  Config cfg_;
  std::shared_ptr<LRUCache> cache_;

  std::mutex mtx_;
  bool closed_ = false;

  // Pools
  std::vector<std::unique_ptr<Buffer>> recv_pool_;
  int recv_inflight_ = 0;

  // Pending sends
  struct SendItem {
    std::unique_ptr<Buffer> buf;
  };
  std::deque<SendItem> send_queue_;
  int sends_inflight_ = 0;
};

} // namespace rdma_fast
#endif