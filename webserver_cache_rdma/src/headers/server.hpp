#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <string>

#include "util/config.hpp"
#include "cache/lru_cache.hpp"

class Server {
public:
  Server(boost::asio::io_context& ioc, const Config& cfg, std::shared_ptr<LRUCache> cache);
  void start();

  std::shared_ptr<LRUCache> cache() const { return cache_; }
  const Config& config() const { return cfg_; }

private:
  void do_accept();

  boost::asio::io_context& ioc_;
  boost::asio::ip::tcp::acceptor acceptor_;
  Config cfg_;
  std::shared_ptr<LRUCache> cache_;
};