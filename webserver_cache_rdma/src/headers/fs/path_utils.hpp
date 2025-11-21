#pragma once
#include <string>

struct PathMapResult {
  bool ok = false;
  bool exists = false;
  std::string fs_path;
  std::string cache_key;
  std::string error;
};

PathMapResult map_url_to_fs(const std::string& doc_root, const std::string& url_path);