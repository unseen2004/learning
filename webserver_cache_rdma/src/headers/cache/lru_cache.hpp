#pragma once
#include <unordered_map>
#include <list>
#include <memory>
#include <shared_mutex>
#include <vector>
#include <string>
#include <ctime>

class LRUCache {
public:
  struct Entry {
    std::shared_ptr<std::vector<uint8_t>> body;
    std::size_t size = 0;
    std::time_t last_modified = 0;
    std::string etag;
  };

  explicit LRUCache(std::size_t capacity_bytes)
    : capacity_bytes_(capacity_bytes) {}

  bool get(const std::string& key, Entry& out);
  void put(const std::string& key, const Entry& e);

  std::size_t size_bytes() const;
  std::size_t capacity_bytes() const { return capacity_bytes_; }
  std::size_t items() const;

private:
  struct Node {
    std::string key;
    Entry value;
  };

  mutable std::shared_mutex mtx_;
  std::size_t capacity_bytes_;
  std::size_t used_bytes_{0};

  std::list<Node> lru_; // front = most recent
  std::unordered_map<std::string, std::list<Node>::iterator> map_;

  void evict_if_needed();
};