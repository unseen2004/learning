#ifdef ENABLE_RDMA

#include "../../headers/rdma/rdma_server.hpp"
#include "../../headers/rdma/connection.hpp"
#include <fmt/core.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <fmt/format.h>


#include "../../headers/util/config.hpp"
#include "../../headers/cache/lru_cache.hpp"

template <>
struct fmt::formatter<ibv_wc_status> : fmt::formatter<int> {
    auto format(ibv_wc_status status, format_context& ctx) const {
        return formatter<int>::format(static_cast<int>(status), ctx);
    }
};

namespace rdma_fast {
  static sockaddr_in make_addr_(const std::string &ip, uint16_t port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    return addr;
  }

  RDMAServer::RDMAServer(const RDMAConfig &cfg, const Config &app_cfg, std::shared_ptr<LRUCache> cache)
    : cfg_(cfg), app_cfg_(app_cfg), cache_(std::move(cache)) {
  }

  RDMAServer::~RDMAServer() {
    stop();
  }

  void RDMAServer::start() {
    if (running_.exchange(true)) return;

    ec_ = rdma_create_event_channel();
    if (!ec_) throw std::runtime_error("rdma_create_event_channel failed");

    if (rdma_create_id(ec_, &listen_id_, nullptr, RDMA_PS_TCP))
      throw std::runtime_error("rdma_create_id failed");

    auto addr = make_addr_(cfg_.bind_addr, cfg_.port);
    if (rdma_bind_addr(listen_id_, reinterpret_cast<sockaddr *>(&addr)))
      throw std::runtime_error("rdma_bind_addr failed");
    if (rdma_listen(listen_id_, 64))
      throw std::runtime_error("rdma_listen failed");

    fmt::print("[rdma] Listening on {}:{} (cq_depth={}, pollers={})\n",
               cfg_.bind_addr, cfg_.port, cfg_.cq_depth, cfg_.poller_threads);

    cm_thread_ = std::thread([this] { cm_event_loop_(); });
    for (int i = 0; i < cfg_.poller_threads; ++i)
      pollers_.emplace_back([this] { cq_poller_loop_(); });
  }

  void RDMAServer::stop() {
    if (!running_.exchange(false)) return;

    if (cm_thread_.joinable()) cm_thread_.join();
    for (auto &t: pollers_) if (t.joinable()) t.join();
    pollers_.clear();

    {
      std::lock_guard<std::mutex> g(conns_mtx_);
      conns_.clear();
    }

    if (cq_) {
      ibv_destroy_cq(cq_);
      cq_ = nullptr;
    }
    if (comp_ch_) {
      ibv_destroy_comp_channel(comp_ch_);
      comp_ch_ = nullptr;
    }
    if (pd_) {
      ibv_dealloc_pd(pd_);
      pd_ = nullptr;
    }

    if (listen_id_) {
      rdma_destroy_id(listen_id_);
      listen_id_ = nullptr;
    }
    if (ec_) {
      rdma_destroy_event_channel(ec_);
      ec_ = nullptr;
    }

    fmt::print("[rdma] Stopped\n");
  }

  void RDMAServer::cm_event_loop_() {
    while (running_) {
      rdma_cm_event *ev = nullptr;
      if (rdma_get_cm_event(ec_, &ev)) {
        if (!running_) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      }
      auto event = ev->event;
      rdma_cm_id *id = ev->id;
      rdma_ack_cm_event(ev);

      if (event == RDMA_CM_EVENT_CONNECT_REQUEST) {
        if (!pd_) {
          ibv_context *ctx = id->verbs;
          pd_ = ibv_alloc_pd(ctx);
          if (!pd_) {
            fmt::print(stderr, "[rdma] ibv_alloc_pd failed\n");
            rdma_reject(id, nullptr, 0);
            continue;
          }
          comp_ch_ = ibv_create_comp_channel(ctx);
          if (!comp_ch_) {
            fmt::print(stderr, "[rdma] ibv_create_comp_channel failed\n");
            rdma_reject(id, nullptr, 0);
            continue;
          }
          cq_ = ibv_create_cq(ctx, cfg_.cq_depth, nullptr, comp_ch_, 0);
          if (!cq_) {
            fmt::print(stderr, "[rdma] ibv_create_cq failed\n");
            rdma_reject(id, nullptr, 0);
            continue;
          }
          ibv_req_notify_cq(cq_, 0);
        }

        ibv_qp_init_attr qp_attr{};
        qp_attr.send_cq = cq_;
        qp_attr.recv_cq = cq_;
        qp_attr.qp_type = IBV_QPT_RC;
        qp_attr.cap.max_send_wr = 1024;
        qp_attr.cap.max_recv_wr = 1024;
        qp_attr.cap.max_send_sge = 1;
        qp_attr.cap.max_recv_sge = 1;

        if (rdma_create_qp(id, pd_, &qp_attr)) {
          fmt::print(stderr, "[rdma] rdma_create_qp failed\n");
          rdma_reject(id, nullptr, 0);
          continue;
        }

        auto conn = std::make_shared<Connection>(this, id, pd_, cq_, app_cfg_, cache_);
        if (!conn->init()) {
          fmt::print(stderr, "[rdma] connection init failed\n");
          rdma_destroy_qp(id);
          rdma_reject(id, nullptr, 0);
          continue;
        }

        rdma_conn_param param{};
        param.initiator_depth = 1;
        param.responder_resources = 1;
        param.rnr_retry_count = 7;

        if (rdma_accept(id, &param)) {
          fmt::print(stderr, "[rdma] rdma_accept failed\n");
          rdma_destroy_qp(id);
          continue;
        }

        {
          std::lock_guard<std::mutex> g(conns_mtx_);
          conns_.insert(conn);
        }

        fmt::print("[rdma] Accepted connection qp_num={}\n", conn->qp_num());
      } else if (event == RDMA_CM_EVENT_DISCONNECTED) {
        // Find and remove the connection (shared_ptr will clean up)
        std::lock_guard<std::mutex> g(conns_mtx_);
        for (auto it = conns_.begin(); it != conns_.end();) {
          if ((*it)->qp_num() == (id->qp ? id->qp->qp_num : 0)) {
            (*it)->close();
            it = conns_.erase(it);
          } else {
            ++it;
          }
        }
        if (id->qp) rdma_destroy_qp(id);
        rdma_destroy_id(id);
        fmt::print("[rdma] Disconnected\n");
      }
    }
  }

  void RDMAServer::cq_poller_loop_() {
    while (running_) {
      ibv_cq *cq = nullptr;
      void *cq_ctx = nullptr;
      int rc = ibv_get_cq_event(comp_ch_, &cq, &cq_ctx);
      if (rc) {
        if (!running_) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        continue;
      }
      ibv_ack_cq_events(cq, 1);
      ibv_req_notify_cq(cq, 0);

      while (running_) {
        ibv_wc wc{};
        int n = ibv_poll_cq(cq, 32, &wc);
        if (n < 0) {
          fmt::print(stderr, "[rdma] ibv_poll_cq error\n");
          break;
        }
        if (n == 0) break;

        if (wc.status != IBV_WC_SUCCESS) {
          fmt::print(stderr, "[rdma] CQE status {} wr_id {}\n", wc.status, wc.wr_id);
          // Free work item if present
          auto *base = reinterpret_cast<WorkBase *>(wc.wr_id);
          delete base;
          continue;
        }

        if (wc.opcode == IBV_WC_RECV) {
          auto *w = reinterpret_cast<RecvWork *>(wc.wr_id);
          w->conn->on_recv_complete(w, wc.byte_len);
        } else if (wc.opcode == IBV_WC_SEND) {
          auto *w = reinterpret_cast<SendWork *>(wc.wr_id);
          w->conn->on_send_complete(w);
        } else {
          // Ignore other opcodes for this protocol
          auto *base = reinterpret_cast<WorkBase *>(wc.wr_id);
          delete base;
        }
      }
    }
  }
} // namespace rdma_fast

#endif // ENABLE_RDMA
