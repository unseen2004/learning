#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <string>
#include <deque>

#include "util/config.hpp"
#include "cache/lru_cache.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include "http/parser.hpp"

class Session : public std::enable_shared_from_this<Session> {
public:
  Session(boost::asio::ip::tcp::socket socket, const Config& cfg, std::shared_ptr<LRUCache> cache);
  void start();

private:
  void start_read();
  void on_read(boost::system::error_code ec, std::size_t n);

  void handle_next_in_queue();
  void handle_request_and_respond(const HttpRequest& req);
  void respond_with_error(int status, const std::string& message, bool keep_alive);

  void write_response(std::unique_ptr<std::string> head,
                      std::shared_ptr<const std::vector<uint8_t>> body,
                      bool keep_alive);

  void on_write(std::unique_ptr<std::string> head,
                std::shared_ptr<const std::vector<uint8_t>> body,
                bool keep_alive,
                boost::system::error_code ec,
                std::size_t n);

  void arm_idle_timer();
  void cancel_timers();
  void close();

  boost::asio::ip::tcp::socket socket_;
  Config cfg_;
  std::shared_ptr<LRUCache> cache_;

  std::vector<char> inbuf_;
  HttpParser parser_;

  std::deque<HttpRequest> pending_;
  bool writing_ = false;
  bool closing_after_ = false;

  boost::asio::steady_timer read_timer_;
  boost::asio::steady_timer write_timer_;
  boost::asio::steady_timer idle_timer_;

  bool closed_ = false;
};