#pragma once
#include <string>
#include <cstdint>

struct Config {
  unsigned short port = 8080;
  unsigned threads = 0; // 0 -> hardware_concurrency
  std::string doc_root = "./public";

  // Cache
  unsigned cache_mem_mb = 128;

  // Limits
  std::size_t max_request_line = 8192;
  std::size_t max_header_bytes = 32 * 1024;

  // Timeouts (ms)
  int read_timeout_ms = 5000;
  int write_timeout_ms = 5000;
  int keepalive_timeout_ms = 10000;

  // RDMA (effective if compiled with ENABLE_RDMA)
  bool rdma_enable = false;
  std::string rdma_bind = "0.0.0.0";
  unsigned short rdma_port = 7471;
  int rdma_pollers = 1;

  // RDMA protocol/tuning
  int rdma_recv_bufs_per_conn = 64;
  int rdma_recv_buf_size = 4096;      // bytes per posted RECV
  int rdma_send_chunk = 32768;        // bytes per SEND chunk of body
  int rdma_max_outstanding_sends = 64;
};

Config parse_args(int argc, char** argv);