#ifndef DNS_RELAY_LRU_CACHE_H
#define DNS_RELAY_LRU_CACHE_H

#include <spdlog/spdlog.h>

#include <list>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <unordered_map>

// 预先定义的 LRU_Cache 类
template <typename Key, typename Value> class LRU_Cache {
private:
  std::list<std::pair<Key, Value>> cacheItemsList_;
  std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator>
      cacheMap_;
  size_t capacity_;
  mutable std::shared_mutex mutex_; // 保护数据结构的共享互斥锁

public:
  LRU_Cache(size_t capacity) : capacity_(capacity) {
    spdlog::info("LRU_Cache created with capacity {}", capacity_);
  }

  Value get(const Key &key) {
    std::unique_lock lock(mutex_);
    auto it = cacheMap_.find(key);
    if (it == cacheMap_.end()) {
      spdlog::warn("Key {} not found in cache", key);
      throw std::runtime_error("Key not found");
    }
    cacheItemsList_.splice(cacheItemsList_.begin(), cacheItemsList_,
                           it->second);
    spdlog::debug("Key {} accessed, value {}", key, it->second->second);
    return it->second->second;
  }

  void put(const Key &key, const Value &value) {
    std::unique_lock lock(mutex_);
    auto it = cacheMap_.find(key);
    if (it != cacheMap_.end()) {
      cacheItemsList_.splice(cacheItemsList_.begin(), cacheItemsList_,
                             it->second);
      it->second->second = value;
      spdlog::debug("Key {} updated with value {}", key, value);
      return;
    }

    if (cacheItemsList_.size() == capacity_) {
      auto last = cacheItemsList_.end();
      --last;
      spdlog::debug("Cache is full, removing oldest key {}", last->first);
      cacheMap_.erase(last->first);
      cacheItemsList_.pop_back();
    }

    cacheItemsList_.emplace_front(key, value);
    cacheMap_[key] = cacheItemsList_.begin();
    spdlog::debug("Key {} inserted with value {}", key, value);
  }

  size_t size() const {
    std::shared_lock lock(mutex_);
    size_t size = cacheItemsList_.size();
    spdlog::info("Cache size requested, current size {}", size);
    return size;
  }

  bool contains(const Key &key) const {
    std::shared_lock lock(mutex_);
    bool found = cacheMap_.find(key) != cacheMap_.end();
    spdlog::debug("Cache contains key {}: {}", key, found);
    return found;
  }

  void remove(const Key &key) {
    std::unique_lock lock(mutex_);
    auto it = cacheMap_.find(key);
    if (it != cacheMap_.end()) {
      cacheItemsList_.erase(it->second);
      cacheMap_.erase(it);
      spdlog::debug("Key {} removed from cache", key);
    } else {
      spdlog::warn("Key {} not found in cache, cannot remove", key);
    }
  }

  // 避免重复上锁 不能调用 remove 函数
  void remove_oldest() {
    std::unique_lock lock(mutex_);
    if (!cacheItemsList_.empty()) {
      auto oldest = cacheItemsList_.back();
      cacheItemsList_.pop_back();
      cacheMap_.erase(oldest.first);
      spdlog::debug("Oldest key {} removed from cache", oldest.first);
    } else {
      spdlog::warn("Cache is empty, cannot remove oldest key");
    }
  }
};

#endif // DNS_RELAY_LRU_CACHE_H
