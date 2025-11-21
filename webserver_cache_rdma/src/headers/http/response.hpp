#pragma once
#include <string>
#include <unordered_map>
#include "../util/time.hpp"

struct HttpResponse {
  int status = 200;
  std::string reason = "OK";
  std::unordered_map<std::string, std::string> headers;

  std::string serialize_headers() const {
    std::string h;
    h.reserve(256);
    h += "HTTP/1.1 " + std::to_string(status) + " " + reason + "\r\n";
    if (headers.find("Date") == headers.end()) {
      h += "Date: " + now_http_date() + "\r\n";
    }
    for (const auto& kv : headers) {
      h += kv.first + ": " + kv.second + "\r\n";
    }
    h += "\r\n";
    return h;
  }
};