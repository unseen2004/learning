#pragma once
#include <boost/asio.hpp>

class SignalHandler {
public:
  explicit SignalHandler(boost::asio::io_context& ioc);
  void register_signals();

private:
  void on_signal(const boost::system::error_code& ec, int signo);

  boost::asio::io_context& ioc_;
  boost::asio::signal_set signals_;
};