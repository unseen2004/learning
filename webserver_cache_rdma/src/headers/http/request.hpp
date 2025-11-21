#pragma once
#include <string>
#include <unordered_map>

struct HttpRequest {
  std::string method;
  std::string target;
  std::string version;
  std::unordered_map<std::string, std::string> headers;
  bool keep_alive = true;

  std::string header(const std::string& name) const;
};