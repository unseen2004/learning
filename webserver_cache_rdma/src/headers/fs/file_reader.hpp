#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <ctime>

struct FileReadResult {
  bool ok = false;
  std::vector<uint8_t> data;
  std::time_t last_modified = 0;
  std::string error;
};

FileReadResult read_file(const std::string& path);

inline std::string make_etag(std::size_t size, std::time_t mtime) {
  return "W/\"" + std::to_string(size) + "-" + std::to_string(static_cast<long long>(mtime)) + "\"";
}