#include "../../headers/util/config.hpp"
#include <fmt/core.h>
#include <string>
#include <cstdlib>

static void print_usage(const char* argv0) {
  fmt::print(
    "Usage: {} [--port N] [--threads N] [--doc-root PATH]\n"
    "            [--cache.mem-mb N]\n"
    "            [--read-timeout-ms N] [--write-timeout-ms N] [--keepalive-timeout-ms N]\n"
    "            [--max-request-line N] [--max-header-bytes N]\n"
    "            [--rdma.enable] [--rdma.bind IP] [--rdma.port N] [--rdma.pollers N]\n"
    "            [--rdma.recv-bufs N] [--rdma.recv-size N] [--rdma.send-chunk N] [--rdma.max-sends N]\n",
    argv0
  );
}

Config parse_args(int argc, char** argv) {
  Config cfg;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    auto next = [&](int& i) -> std::string { return (i + 1 < argc) ? std::string(argv[++i]) : std::string(); };

    if (arg == "--port" && i + 1 < argc) cfg.port = static_cast<unsigned short>(std::stoi(next(i)));
    else if (arg == "--threads" && i + 1 < argc) cfg.threads = static_cast<unsigned>(std::stoul(next(i)));
    else if (arg == "--doc-root" && i + 1 < argc) cfg.doc_root = next(i);
    else if (arg == "--cache.mem-mb" && i + 1 < argc) cfg.cache_mem_mb = static_cast<unsigned>(std::stoul(next(i)));
    else if (arg == "--read-timeout-ms" && i + 1 < argc) cfg.read_timeout_ms = std::stoi(next(i));
    else if (arg == "--write-timeout-ms" && i + 1 < argc) cfg.write_timeout_ms = std::stoi(next(i));
    else if (arg == "--keepalive-timeout-ms" && i + 1 < argc) cfg.keepalive_timeout_ms = std::stoi(next(i));
    else if (arg == "--max-request-line" && i + 1 < argc) cfg.max_request_line = static_cast<std::size_t>(std::stoull(next(i)));
    else if (arg == "--max-header-bytes" && i + 1 < argc) cfg.max_header_bytes = static_cast<std::size_t>(std::stoull(next(i)));
    else if (arg == "--rdma.enable") cfg.rdma_enable = true;
    else if (arg == "--rdma.bind" && i + 1 < argc) cfg.rdma_bind = next(i);
    else if (arg == "--rdma.port" && i + 1 < argc) cfg.rdma_port = static_cast<unsigned short>(std::stoi(next(i)));
    else if (arg == "--rdma.pollers" && i + 1 < argc) cfg.rdma_pollers = std::stoi(next(i));
    else if (arg == "--rdma.recv-bufs" && i + 1 < argc) cfg.rdma_recv_bufs_per_conn = std::stoi(next(i));
    else if (arg == "--rdma.recv-size" && i + 1 < argc) cfg.rdma_recv_buf_size = std::stoi(next(i));
    else if (arg == "--rdma.send-chunk" && i + 1 < argc) cfg.rdma_send_chunk = std::stoi(next(i));
    else if (arg == "--rdma.max-sends" && i + 1 < argc) cfg.rdma_max_outstanding_sends = std::stoi(next(i));
    else if (arg == "--help" || arg == "-h") {
      print_usage(argv[0]);
      std::exit(0);
    }
  }
  return cfg;
}