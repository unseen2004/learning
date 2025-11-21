#include "../headers/signals.hpp"
#include <fmt/core.h>

SignalHandler::SignalHandler(boost::asio::io_context& ioc)
  : ioc_(ioc), signals_(ioc, SIGINT, SIGTERM)
{}

void SignalHandler::register_signals() {
  signals_.async_wait([this](const boost::system::error_code& ec, int signo) {
    on_signal(ec, signo);
  });
}

void SignalHandler::on_signal(const boost::system::error_code& ec, int signo) {
  if (!ec) {
    fmt::print("[info] Caught signal {}, shutting down...\n", signo);
    ioc_.stop();
  }
}