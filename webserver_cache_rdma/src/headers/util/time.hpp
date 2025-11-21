#pragma once
#include <string>
#include <chrono>
#include <ctime>

inline std::string format_http_date(std::time_t t) {
  char buf[64]{0};
  std::tm gm{};
#if defined(_WIN32)
  gmtime_s(&gm, &t);
#else
  gmtime_r(&t, &gm);
#endif
  std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &gm);
  return std::string(buf);
}

inline std::string now_http_date() {
  return format_http_date(std::time(nullptr));
}