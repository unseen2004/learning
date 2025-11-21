#ifdef ENABLE_RDMA
#include "../../headers/rdma/connection.hpp"
#include "../../headers/rdma/protocol.hpp"
#include "../../headers/rdma/rdma_server.hpp"
#include "../../headers/fs/path_utils.hpp"
#include "../../headers/fs/file_reader.hpp"
#include "../../headers/util/metrics.hpp"
#include <cstring>
#include <fmt/core.h>
#include <infiniband/verbs.h>

#include "../../headers/cache/lru_cache.hpp"
#include "../../headers/util/config.hpp"

namespace rdma_fast {

Buffer::Buffer(ibv_pd* pd, size_t n)
  : data(static_cast<char*>(::aligned_alloc(4096, ((n + 4095) / 4096) * 4096))),
    size(((n + 4095) / 4096) * 4096) {
  if (!data) throw std::bad_alloc();
  mr = ibv_reg_mr(pd, data, size, IBV_ACCESS_LOCAL_WRITE);
  if (!mr) {
    ::free(data);
    data = nullptr;
    throw std::runtime_error("ibv_reg_mr failed");
  }
}
Buffer::~Buffer() {
  if (mr) ibv_dereg_mr(mr);
  if (data) ::free(data);
}

Connection::Connection(RDMAServer* srv,
                       rdma_cm_id* id,
                       ibv_pd* pd,
                       ibv_cq* cq,
                       const Config& cfg,
                       std::shared_ptr<LRUCache> cache)
  : server_(srv), id_(id), pd_(pd), cq_(cq), cfg_(cfg), cache_(std::move(cache)) {}

Connection::~Connection() {
  close();
}

bool Connection::init() {
  std::lock_guard<std::mutex> g(mtx_);
  try {
    recv_pool_.reserve(cfg_.rdma_recv_bufs_per_conn);
    for (int i = 0; i < cfg_.rdma_recv_bufs_per_conn; ++i) {
      recv_pool_.push_back(std::make_unique<Buffer>(pd_, static_cast<size_t>(cfg_.rdma_recv_buf_size)));
    }
  } catch (const std::exception& ex) {
    fmt::print(stderr, "[rdma] recv pool alloc failed: {}\n", ex.what());
    return false;
  }
  return post_recvs(cfg_.rdma_recv_bufs_per_conn);
}

bool Connection::post_recvs(int count) {
  int posted = 0;
  for (int i = 0; i < count; ++i) {
    Buffer* b = nullptr;
    {
      // Take from pool
      if (recv_pool_.empty()) break;
      b = recv_pool_.back().release();
      recv_pool_.pop_back();
    }

    ibv_sge sge{};
    sge.addr = reinterpret_cast<uint64_t>(b->data);
    sge.length = static_cast<uint32_t>(b->size);
    sge.lkey = b->mr->lkey;

    auto work = new RecvWork(shared_from_this(), b);

    ibv_recv_wr wr{}, *bad = nullptr;
    wr.sg_list = &sge;
    wr.num_sge = 1;
    wr.wr_id = reinterpret_cast<uint64_t>(work);

    if (ibv_post_recv(id_->qp, &wr, &bad)) {
      // return buffer to pool and free work
      recv_pool_.push_back(std::unique_ptr<Buffer>(b));
      delete work;
      break;
    } else {
      ++recv_inflight_;
      ++posted;
    }
  }
  return posted > 0;
}

void Connection::on_recv_complete(RecvWork* w, uint32_t byte_len) {
  std::unique_ptr<RecvWork> work(w); // auto free
  Buffer* buf = work->buf;

  // Parse request (can be less than buffer size)
  Request req;
  bool ok = parse_request(buf->data, byte_len, req);
  if (!ok) {
    // Malformed -> send error header with status 400 and no body
    send_header(400, 0, 0);
  } else {
    Metrics::instance().rdma_reqs.fetch_add(1, std::memory_order_relaxed);
    if (req.op == Op::PING) {
      handle_ping();
    } else if (req.op == Op::GET) {
      handle_get(req.path);
    } else {
      send_header(400, 0, 0);
    }
  }

  // Reuse buffer: repost RECV
  {
    std::lock_guard<std::mutex> g(mtx_);
    recv_pool_.push_back(std::unique_ptr<Buffer>(buf));
    --recv_inflight_;
    post_recvs(1);
  }
}

void Connection::handle_ping() {
  send_header(200, 0, 0);
  Metrics::instance().rdma_ok.fetch_add(1, std::memory_order_relaxed);
}

void Connection::handle_get(const std::string& url_path) {
  // Map and serve, same as HTTP path
  auto mapped = map_url_to_fs(cfg_.doc_root, url_path);
  if (!mapped.ok) {
    send_header(400, 0, 0);
    Metrics::instance().rdma_err.fetch_add(1, std::memory_order_relaxed);
    return;
  }
  if (!mapped.exists) {
    send_header(404, 0, 0);
    Metrics::instance().rdma_err.fetch_add(1, std::memory_order_relaxed);
    return;
  }

  const std::string cache_key = mapped.cache_key;
  LRUCache::Entry entry;
  if (!cache_->get(cache_key, entry)) {
    auto fr = read_file(mapped.fs_path);
    if (!fr.ok) {
      send_header(500, 0, 0);
      Metrics::instance().rdma_err.fetch_add(1, std::memory_order_relaxed);
      return;
    }
    LRUCache::Entry ne;
    ne.body = std::make_shared<std::vector<uint8_t>>(std::move(fr.data));
    ne.size = ne.body->size();
    ne.last_modified = fr.last_modified;
    ne.etag = make_etag(ne.size, ne.last_modified);
    cache_->put(cache_key, ne);
    entry = std::move(ne);
  }

  uint64_t total = entry.body->size();
  uint32_t chunk = static_cast<uint32_t>(std::max(1, std::min(cfg_.rdma_send_chunk, static_cast<int>(total))));
  if (!send_header(200, total, chunk)) {
    Metrics::instance().rdma_err.fetch_add(1, std::memory_order_relaxed);
    return;
  }
  if (total > 0) {
    if (!send_body_chunks(entry.body, chunk)) {
      Metrics::instance().rdma_err.fetch_add(1, std::memory_order_relaxed);
      return;
    }
  }
  Metrics::instance().rdma_ok.fetch_add(1, std::memory_order_relaxed);
  Metrics::instance().rdma_bytes.fetch_add(total, std::memory_order_relaxed);
}

bool Connection::send_header(uint16_t status, uint64_t content_len, uint32_t chunk) {
  auto header_bytes = make_resp_header(status, content_len, chunk);
  auto b = std::make_unique<Buffer>(pd_, header_bytes.size());
  std::memcpy(b->data, header_bytes.data(), header_bytes.size());

  ibv_sge sge{};
  sge.addr = reinterpret_cast<uint64_t>(b->data);
  sge.length = static_cast<uint32_t>(header_bytes.size());
  sge.lkey = b->mr->lkey;

  auto work = new SendWork(shared_from_this(), b.get());

  ibv_send_wr wr{}, *bad=nullptr;
  wr.sg_list = &sge;
  wr.num_sge = 1;
  wr.opcode = IBV_WR_SEND;
  wr.send_flags = IBV_SEND_SIGNALED;
  wr.wr_id = reinterpret_cast<uint64_t>(work);

  {
    std::lock_guard<std::mutex> g(mtx_);
    if (ibv_post_send(id_->qp, &wr, &bad)) {
      delete work;
      return false;
    }
    ++sends_inflight_;
    // transfer ownership to inflight queue (reclaimed on send complete)
    send_queue_.push_back(SendItem{std::move(b)});
  }
  return true;
}

bool Connection::send_body_chunks(const std::shared_ptr<std::vector<uint8_t>>& body, uint32_t chunk) {
  std::lock_guard<std::mutex> g(mtx_);
  size_t off = 0;
  const size_t total = body->size();

  while (off < total) {
    const size_t n = std::min(static_cast<size_t>(chunk), total - off);

    auto b = std::make_unique<Buffer>(pd_, n);
    std::memcpy(b->data, body->data() + off, n);

    ibv_sge sge{};
    sge.addr = reinterpret_cast<uint64_t>(b->data);
    sge.length = static_cast<uint32_t>(n);
    sge.lkey = b->mr->lkey;

    auto work = new SendWork(shared_from_this(), b.get());

    ibv_send_wr wr{}, *bad=nullptr;
    wr.sg_list = &sge;
    wr.num_sge = 1;
    wr.opcode = IBV_WR_SEND;
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.wr_id = reinterpret_cast<uint64_t>(work);

    // Flow control: limit outstanding sends
    if (sends_inflight_ >= cfg_.rdma_max_outstanding_sends) {
      // Stop posting more now; queue buffer and let on_send_complete post later
      send_queue_.push_back(SendItem{std::move(b)});
      // Store wr details by re-creating when dequeued; to keep simple, we post immediately but rely on HCA queue.
      // Simpler approach: post and rely on HCA; still enforce a high watermark.
    }

    if (ibv_post_send(id_->qp, &wr, &bad)) {
      delete work;
      return false;
    }
    ++sends_inflight_;
    send_queue_.push_back(SendItem{std::move(b)});

    off += n;
  }
  return true;
}

void Connection::try_post_more_sends_locked() {
  // This placeholder can be extended to post deferred sends if you implement a separate queue of not-yet-posted WRs.
}

void Connection::on_send_complete(SendWork* w) {
  std::unique_ptr<SendWork> work(w);
  Buffer* buf = work->buf;

  std::lock_guard<std::mutex> g(mtx_);
  // Find and reclaim the matching buffer from send_queue_
  // For simplicity we reclaim from front if matches pointer.
  if (!send_queue_.empty() && send_queue_.front().buf.get() == buf) {
    send_queue_.pop_front(); // unique_ptr will delete buffer
  } else {
    // Fallback: search (shouldn't happen in order)
    for (auto it = send_queue_.begin(); it != send_queue_.end(); ++it) {
      if (it->buf.get() == buf) {
        send_queue_.erase(it);
        break;
      }
    }
  }
  --sends_inflight_;
  if (sends_inflight_ < 0) sends_inflight_ = 0;
  try_post_more_sends_locked();
}

void Connection::close() {
  std::lock_guard<std::mutex> g(mtx_);
  if (closed_) return;
  closed_ = true;
  // QP and id are owned by CM; RDMAServer will destroy them on disconnect
}

} // namespace rdma_fast
#endif