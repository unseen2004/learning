#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "request.hpp"

enum class ParseState {
  Incomplete,
  Done,
  BadRequest
};

struct ParseResult {
  ParseState state;
  HttpRequest request;
};

class HttpParser {
public:
  HttpParser(std::size_t max_start_line, std::size_t max_headers_bytes)
    : max_start_line_(max_start_line), max_headers_bytes_(max_headers_bytes) {}

  ParseResult parse(const char* data, std::size_t n);
  void reset();

private:
  std::string buf_;
  std::size_t max_start_line_;
  std::size_t max_headers_bytes_;

  bool try_parse(HttpRequest& out);
};