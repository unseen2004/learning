#include "../headers/server.hpp"
#include "../headers/session.hpp"
#include <fmt/core.h>

using boost::asio::ip::tcp;

Server::Server(boost::asio::io_context& ioc, const Config& cfg, std::shared_ptr<LRUCache> cache)
  : ioc_(ioc),
    acceptor_(ioc),
    cfg_(cfg),
    cache_(std::move(cache)) {

  tcp::endpoint ep(tcp::v4(), cfg.port);
  boost::system::error_code ec;
  acceptor_.open(ep.protocol(), ec);
  if (ec) throw std::runtime_error("acceptor open failed: " + ec.message());
  acceptor_.set_option(tcp::acceptor::reuse_address(true), ec);
  acceptor_.bind(ep, ec);
  if (ec) throw std::runtime_error("bind failed: " + ec.message());
  acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
  if (ec) throw std::runtime_error("listen failed: " + ec.message());
}

void Server::start() {
  fmt::print("[info] Listening on 0.0.0.0:{}\n", cfg_.port);
  do_accept();
}

void Server::do_accept() {
  acceptor_.async_accept(
    [this](boost::system::error_code ec, tcp::socket socket) {
      if (!ec) {
        try {
          auto ep = socket.remote_endpoint();
          fmt::print("[info] Accepted {}:{}\n", ep.address().to_string(), ep.port());
        } catch (...) {}
        std::make_shared<Session>(std::move(socket), cfg_, cache_)->start();
      } else {
        fmt::print(stderr, "[warn] accept error: {}\n", ec.message());
      }
      do_accept();
    }
  );
}