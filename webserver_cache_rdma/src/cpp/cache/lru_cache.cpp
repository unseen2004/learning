#include "../../headers/cache/lru_cache.hpp"

#include <mutex>

bool LRUCache::get(const std::string& key, Entry& out) {
  std::unique_lock lock(mtx_);
  auto it = map_.find(key);
  if (it == map_.end()) return false;
  lru_.splice(lru_.begin(), lru_, it->second);
  out = it->second->value;
  return true;
}

void LRUCache::put(const std::string& key, const Entry& e) {
  std::unique_lock lock(mtx_);
  auto it = map_.find(key);
  if (it != map_.end()) {
    used_bytes_ -= it->second->value.size;
    it->second->value = e;
    used_bytes_ += e.size;
    lru_.splice(lru_.begin(), lru_, it->second);
  } else {
    lru_.push_front(Node{key, e});
    map_[key] = lru_.begin();
    used_bytes_ += e.size;
  }
  evict_if_needed();
}

void LRUCache::evict_if_needed() {
  while (used_bytes_ > capacity_bytes_ && !lru_.empty()) {
    auto it = --lru_.end();
    used_bytes_ -= it->value.size;
    map_.erase(it->key);
    lru_.erase(it);
  }
}

std::size_t LRUCache::size_bytes() const {
  std::shared_lock lock(mtx_);
  return used_bytes_;
}

std::size_t LRUCache::items() const {
  std::shared_lock lock(mtx_);
  return map_.size();
}