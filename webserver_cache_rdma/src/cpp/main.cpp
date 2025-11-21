#include <boost/asio.hpp>
#include <fmt/core.h>
#include <thread>
#include <vector>
#include <string>
#include <cstdlib>

#include "../headers/server.hpp"
#include "../headers/signals.hpp"
#include "../headers/util/config.hpp"
#include "../headers/util/metrics.hpp"
#include "../headers/cache/lru_cache.hpp"

#ifdef ENABLE_RDMA
#include "../headers/rdma/rdma_server.hpp"
#endif

int main(int argc, char** argv) {
  try {
    Config cfg = parse_args(argc, argv);
    if (cfg.threads == 0) {
      cfg.threads = std::max(1u, std::thread::hardware_concurrency());
    }

    fmt::print("[info] Starting webserver port={}, threads={}, doc_root='{}', mem_cache={} MB, timeouts: read={}ms write={}ms keepalive={}ms\n",
               cfg.port, cfg.threads, cfg.doc_root, cfg.cache_mem_mb,
               cfg.read_timeout_ms, cfg.write_timeout_ms, cfg.keepalive_timeout_ms);
#ifdef ENABLE_RDMA
    fmt::print("[info] RDMA: enabled={}, bind={}, port={}, pollers={}\n",
               (cfg.rdma_enable ? "true" : "false"), cfg.rdma_bind, cfg.rdma_port, cfg.rdma_pollers);
#endif

    auto shared_cache = std::make_shared<LRUCache>(static_cast<std::size_t>(cfg.cache_mem_mb) * 1024ull * 1024ull);

#ifdef ENABLE_RDMA
    std::unique_ptr<rdma_fast::RDMAServer> rdma_srv;
    if (cfg.rdma_enable) {
      rdma_fast::RDMAConfig rc;
      rc.bind_addr = cfg.rdma_bind;
      rc.port = cfg.rdma_port;
      rc.cq_depth = 512;
      rc.poller_threads = cfg.rdma_pollers;
      rdma_srv = std::make_unique<rdma_fast::RDMAServer>(rc, cfg, shared_cache);
      rdma_srv->start();
    }
#endif

    boost::asio::io_context ioc;

    SignalHandler sigs{ioc};
    sigs.register_signals();

    Metrics::instance().reset();

    Server server{ioc, cfg, shared_cache};
    server.start();

    std::vector<std::thread> workers;
    workers.reserve(cfg.threads);
    for (unsigned i = 0; i < cfg.threads; ++i) {
      workers.emplace_back([&ioc] {
        ioc.run();
      });
    }

    for (auto& t : workers) t.join();

#ifdef ENABLE_RDMA
    if (rdma_srv) rdma_srv->stop();
#endif

    fmt::print("[info] Webserver stopped\n");
    return 0;
  } catch (const std::exception& ex) {
    fmt::print(stderr, "[fatal] {}\n", ex.what());
    return 1;
  }
}