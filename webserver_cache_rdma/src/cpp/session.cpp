#include "../headers/session.hpp"
#include <fmt/core.h>
#include <boost/asio/buffer.hpp>
#include <boost/asio/write.hpp>
#include <filesystem>
#include "../headers/fs/path_utils.hpp"
#include "../headers/fs/file_reader.hpp"
#include "../headers/http/mime.hpp"
#include "../headers/http/response.hpp"
#include "../headers/util/time.hpp"
#include "../headers/util/metrics.hpp"

using boost::asio::ip::tcp;

Session::Session(tcp::socket socket, const Config& cfg, std::shared_ptr<LRUCache> cache)
  : socket_(std::move(socket)),
    cfg_(cfg),
    cache_(std::move(cache)),
    inbuf_(8192),
    parser_(cfg.max_request_line, cfg.max_header_bytes),
    read_timer_(socket_.get_executor()),
    write_timer_(socket_.get_executor()),
    idle_timer_(socket_.get_executor())
{}

void Session::start() {
  arm_idle_timer();
  start_read();
}

void Session::start_read() {
  if (closing_after_) return;
  auto self = shared_from_this();
  read_timer_.expires_after(std::chrono::milliseconds(cfg_.read_timeout_ms));
  read_timer_.async_wait([self](const boost::system::error_code& ec) {
    if (!ec) {
      fmt::print("[info] read timeout, closing connection\n");
      self->close();
    }
  });

  socket_.async_read_some(boost::asio::buffer(inbuf_),
    [self](boost::system::error_code ec, std::size_t n) {
      self->on_read(ec, n);
    }
  );
}

void Session::on_read(boost::system::error_code ec, std::size_t n) {
  if (ec) {
    if (ec != boost::asio::error::operation_aborted) {
      // client closed or error
    }
    close();
    return;
  }

  boost::system::error_code ignore;
  read_timer_.cancel(ignore);
  idle_timer_.expires_after(std::chrono::milliseconds(cfg_.keepalive_timeout_ms));

  auto res = parser_.parse(inbuf_.data(), n);
  while (true) {
    if (res.state == ParseState::BadRequest) {
      pending_.clear();
      closing_after_ = true;
      respond_with_error(400, "Bad Request", false);
      return;
    } else if (res.state == ParseState::Incomplete) {
      break;
    } else {
      pending_.push_back(std::move(res.request));
      res = parser_.parse("", 0);
    }
  }

  if (!writing_) {
    handle_next_in_queue();
  }

  if (!closing_after_) {
    start_read();
  }
}

void Session::handle_next_in_queue() {
  if (pending_.empty() || writing_) return;
  const HttpRequest req = std::move(pending_.front());
  pending_.pop_front();
  handle_request_and_respond(req);
}

void Session::handle_request_and_respond(const HttpRequest& req) {
  writing_ = true;
  bool keep_alive = req.keep_alive;
  if (!keep_alive) {
    closing_after_ = true;
    pending_.clear();
  }

  if (req.method == "GET" && req.target == "/metrics") {
    auto body_str = Metrics::instance().render_text();
    auto body = std::make_shared<std::vector<uint8_t>>(body_str.begin(), body_str.end());

    HttpResponse resp;
    resp.status = 200;
    resp.reason = "OK";
    resp.headers["Content-Type"] = "text/plain; charset=utf-8";
    resp.headers["Content-Length"] = std::to_string(body->size());
    resp.headers["Connection"] = keep_alive ? "keep-alive" : "close";
    auto head = std::make_unique<std::string>(resp.serialize_headers());
    write_response(std::move(head), body, keep_alive);
    return;
  }

  if (!(req.method == "GET" || req.method == "HEAD")) {
    respond_with_error(405, "Method Not Allowed", keep_alive);
    return;
  }

  auto mapped = map_url_to_fs(cfg_.doc_root, req.target);
  if (!mapped.ok) {
    respond_with_error(400, mapped.error, keep_alive);
    return;
  }
  if (!mapped.exists) {
    respond_with_error(404, "Not Found", keep_alive);
    return;
  }

  const std::string& fs_path = mapped.fs_path;
  const std::string cache_key = mapped.cache_key;

  LRUCache::Entry entry;
  if (cache_->get(cache_key, entry)) {
    Metrics::instance().cache_hits.fetch_add(1, std::memory_order_relaxed);

    HttpResponse resp;
    resp.status = 200;
    resp.reason = "OK";
    resp.headers["Content-Type"] = mime_type(fs_path);
    resp.headers["Content-Length"] = std::to_string(entry.body->size());
    resp.headers["Connection"] = keep_alive ? "keep-alive" : "close";
    resp.headers["Last-Modified"] = format_http_date(entry.last_modified);
    resp.headers["ETag"] = entry.etag;

    auto head = std::make_unique<std::string>(resp.serialize_headers());
    auto body = (req.method == "HEAD") ? std::make_shared<std::vector<uint8_t>>() : entry.body;

    Metrics::instance().responses_2xx.fetch_add(1, std::memory_order_relaxed);
    Metrics::instance().bytes_served.fetch_add(body->size(), std::memory_order_relaxed);
    write_response(std::move(head), body, keep_alive);
    return;
  }
  Metrics::instance().cache_misses.fetch_add(1, std::memory_order_relaxed);

  auto fr = read_file(fs_path);
  if (!fr.ok) {
    respond_with_error(500, fr.error, keep_alive);
    return;
  }

  LRUCache::Entry new_entry;
  new_entry.body = std::make_shared<std::vector<uint8_t>>(std::move(fr.data));
  new_entry.size = new_entry.body->size();
  new_entry.last_modified = fr.last_modified;
  new_entry.etag = make_etag(new_entry.size, new_entry.last_modified);

  cache_->put(cache_key, new_entry);

  HttpResponse resp;
  resp.status = 200;
  resp.reason = "OK";
  resp.headers["Content-Type"] = mime_type(fs_path);
  resp.headers["Content-Length"] = std::to_string(new_entry.size);
  resp.headers["Connection"] = keep_alive ? "keep-alive" : "close";
  resp.headers["Last-Modified"] = format_http_date(new_entry.last_modified);
  resp.headers["ETag"] = new_entry.etag;

  auto head = std::make_unique<std::string>(resp.serialize_headers());
  auto body = (req.method == "HEAD") ? std::make_shared<std::vector<uint8_t>>() : new_entry.body;

  Metrics::instance().responses_2xx.fetch_add(1, std::memory_order_relaxed);
  Metrics::instance().bytes_served.fetch_add(body->size(), std::memory_order_relaxed);
  write_response(std::move(head), body, keep_alive);
}

void Session::respond_with_error(int status, const std::string& message, bool keep_alive) {
  HttpResponse resp;
  resp.status = status;
  switch (status) {
    case 400: resp.reason = "Bad Request"; break;
    case 404: resp.reason = "Not Found"; break;
    case 405: resp.reason = "Method Not Allowed"; break;
    default: resp.reason = "Internal Server Error"; break;
  }
  std::string payload = fmt::format("{} {}\n", status, message);
  auto body = std::make_shared<std::vector<uint8_t>>(payload.begin(), payload.end());
  resp.headers["Content-Type"] = "text/plain; charset=utf-8";
  resp.headers["Content-Length"] = std::to_string(body->size());
  resp.headers["Connection"] = keep_alive ? "keep-alive" : "close";

  if (status >= 500)
    Metrics::instance().responses_5xx.fetch_add(1, std::memory_order_relaxed);
  else
    Metrics::instance().responses_4xx.fetch_add(1, std::memory_order_relaxed);

  auto head = std::make_unique<std::string>(resp.serialize_headers());
  write_response(std::move(head), body, keep_alive);
}

void Session::write_response(std::unique_ptr<std::string> head,
                             std::shared_ptr<const std::vector<uint8_t>> body,
                             bool keep_alive) {
  auto self = shared_from_this();

  write_timer_.expires_after(std::chrono::milliseconds(cfg_.write_timeout_ms));
  write_timer_.async_wait([self](const boost::system::error_code& ec) {
    if (!ec) {
      fmt::print("[info] write timeout, closing connection\n");
      self->close();
    }
  });

  std::array<boost::asio::const_buffer, 2> bufs {
    boost::asio::buffer(*head),
    body->empty() ? boost::asio::const_buffer{} : boost::asio::buffer(body->data(), body->size())
  };

  boost::asio::async_write(socket_, bufs,
    [self, head = std::move(head), body, keep_alive]
    (boost::system::error_code ec, std::size_t /*n*/) mutable {
      self->on_write(std::move(head), body, keep_alive, ec, 0);
    }
  );
}

void Session::on_write(std::unique_ptr<std::string> /*head*/,
                       std::shared_ptr<const std::vector<uint8_t>> /*body*/,
                       bool keep_alive,
                       boost::system::error_code ec,
                       std::size_t /*n*/) {
  boost::system::error_code ignore;
  write_timer_.cancel(ignore);

  if (ec) {
    close();
    return;
  }

  if (!keep_alive || closing_after_) {
    close();
    return;
  }

  writing_ = false;
  if (!pending_.empty()) {
    handle_next_in_queue();
  } else {
    start_read();
  }
}

void Session::arm_idle_timer() {
  auto self = shared_from_this();
  idle_timer_.expires_after(std::chrono::milliseconds(cfg_.keepalive_timeout_ms));
  idle_timer_.async_wait([self](const boost::system::error_code& ec){
    if (!ec) {
      fmt::print("[info] idle timeout, closing connection\n");
      self->close();
    }
  });
}

void Session::cancel_timers() {
  boost::system::error_code ig;
  read_timer_.cancel(ig);
  write_timer_.cancel(ig);
  idle_timer_.cancel(ig);
}

void Session::close() {
  if (closed_) return;
  closed_ = true;
  cancel_timers();
  boost::system::error_code ig;
  socket_.shutdown(tcp::socket::shutdown_both, ig);
  socket_.close(ig);
}