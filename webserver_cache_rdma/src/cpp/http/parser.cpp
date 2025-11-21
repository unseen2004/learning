#include "../../headers/http/parser.hpp"
#include "../../headers/http/headers.hpp"
#include <cctype>
#include <algorithm>

void HttpParser::reset() {
  buf_.clear();
}

ParseResult HttpParser::parse(const char *data, std::size_t n) {
  buf_.append(data, n);

  if (buf_.size() > (max_start_line_ + max_headers_bytes_ + 4)) {
    return {ParseState::BadRequest, {}};
  }

  HttpRequest req;
  if (!try_parse(req)) {
    if (buf_.size() > (max_start_line_ + max_headers_bytes_ + 4)) {
      return {ParseState::BadRequest, {}};
    }
    return {ParseState::Incomplete, {}};
  }
  return {ParseState::Done, req};
}

static bool is_token_char(char c) {
  static const std::string tspecials = "()<>@,;:\\\"/[]?={} \t";
  return c > 31 && c < 127 && tspecials.find(c) == std::string::npos;
}

bool HttpParser::try_parse(HttpRequest &out) {
  auto pos = buf_.find("\r\n\r\n");
  if (pos == std::string::npos) return false;

  std::string head = buf_.substr(0, pos);
  buf_.erase(0, pos + 4);

  std::vector<std::string> lines;
  std::size_t start = 0;
  while (true) {
    auto e = head.find("\r\n", start);
    if (e == std::string::npos) {
      lines.push_back(head.substr(start));
      break;
    }
    lines.push_back(head.substr(start, e - start));
    start = e + 2;
  }
  if (lines.empty()) return false;

  const std::string &rl = lines[0];
  if (rl.size() > max_start_line_) return false;

  auto s1 = rl.find(' ');
  auto s2 = rl.find(' ', s1 == std::string::npos ? 0 : s1 + 1);
  if (s1 == std::string::npos || s2 == std::string::npos) return false;

  out.method = rl.substr(0, s1);
  out.target = rl.substr(s1 + 1, s2 - s1 - 1);
  out.version = rl.substr(s2 + 1);

  if (out.method.empty() || out.target.empty() || out.version.find("HTTP/") != 0) return false;
  for (char c: out.method) if (!is_token_char(c)) return false;

  std::unordered_map<std::string, std::string> headers;
  std::size_t total_bytes = 0;
  for (std::size_t i = 1; i < lines.size(); ++i) {
    const std::string &line = lines[i];
    total_bytes += line.size();
    if (total_bytes > max_headers_bytes_) return false;

    if (line.empty()) continue;
    auto colon = line.find(':');
    if (colon == std::string::npos) return false;

    std::string name = line.substr(0, colon);
    std::string value = line.substr(colon + 1);
    auto ltrim = [](std::string &s) {
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    };
    auto rtrim = [](std::string &s) {
      s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    };
    ltrim(value);
    rtrim(value);
    std::string lname = header_lower(name);
    headers[lname] = value;
  }
  out.headers = std::move(headers);

  auto conn = out.header("connection");
  if (out.version == "HTTP/1.1") {
    out.keep_alive = !(conn == "close" || conn == "Close");
  } else {
    out.keep_alive = (conn == "keep-alive" || conn == "Keep-Alive");
  }

  return true;
}
