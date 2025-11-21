#pragma once
#include <fmt/core.h>

inline void log_info(const char* fmt, auto&&... args) {
  fmt::print(fmt, std::forward<decltype(args)>(args)...);
}
inline void log_warn(const char* fmt, auto&&... args) {
  fmt::print(stderr, fmt, std::forward<decltype(args)>(args)...);
}
inline void log_error(const char* fmt, auto&&... args) {
  fmt::print(stderr, fmt, std::forward<decltype(args)>(args)...);
}