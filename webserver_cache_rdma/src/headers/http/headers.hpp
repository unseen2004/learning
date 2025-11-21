#pragma once
#include <string>
#include <algorithm>

inline std::string header_lower(const std::string& s) {
  std::string out = s;
  std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
  return out;
}