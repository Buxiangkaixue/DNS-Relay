#ifndef DNS_RELAY_LRU_K_CACHE_H
#define DNS_RELAY_LRU_K_CACHE_H

#include "LRU_Cache.h"
#include <stdexcept>

template <typename Key, typename Value, size_t K> class LRU_K_Cache {
private:
  LRU_Cache<Key, std::pair<Value, size_t>> tempCache_; // 包含访问计数
  LRU_Cache<Key, Value> permanentCache_;               // 持久缓存
  size_t capacity_;
  mutable std::shared_mutex mutex_;

  void tempKeyCountIncrease(const Key &key,
                            std::pair<Value, size_t> &valueCountPair) {
    valueCountPair.second += 1;
    spdlog::debug("In LRU_K_Cache: increase key count with key {} and count {}",
                  key, valueCountPair.second);
    if (valueCountPair.second >= K) {
      spdlog::info("In LRU_K_Cache: key {} has reached threshold {} and is "
                   "promoted to permanent cache",
                   key, K);
      permanentCache_.put(key, valueCountPair.first);
      tempCache_.remove(key);
    } else {
      tempCache_.put(key, valueCountPair);
    }
  }

public:
  LRU_K_Cache(size_t capacity)
      : capacity_(capacity), tempCache_(capacity),
        permanentCache_(capacity - capacity / 10) {
    spdlog::info("In LRU_K_Cache: created with capacity {}", capacity_);
  }

  Value get(const Key &key) {
    std::unique_lock lock(mutex_);
    spdlog::debug("In LRU_K_Cache: getting key {}", key);
    if (permanentCache_.contains(key)) {
      spdlog::debug("In LRU_K_Cache: key {} found in permanent cache", key);
      return permanentCache_.get(key);
    } else if (tempCache_.contains(key)) {
      spdlog::debug("In LRU_K_Cache: key {} found in temp cache", key);
      auto valueCountPair = tempCache_.get(key);
      tempKeyCountIncrease(key, valueCountPair);
      return valueCountPair.first;
    } else {
      spdlog::warn("In LRU_K_Cache: key {} not found in cache", key);
      throw std::runtime_error("Key not found");
    }
  }

  void put(const Key &key, const Value &value) {
    std::unique_lock lock(mutex_);
    spdlog::debug("In LRU_K_Cache: putting key {} with value {}", key, value);
    if (permanentCache_.contains(key)) {
      spdlog::debug(
          "In LRU_K_Cache: key {} already in permanent cache, updating value",
          key);
      permanentCache_.put(key, value);
    } else if (tempCache_.contains(key)) {
      spdlog::debug(
          "In LRU_K_Cache: key {} already in temp cache, updating value", key);
      auto valueCountPair = tempCache_.get(key);
      valueCountPair.first = value;
      tempKeyCountIncrease(key, valueCountPair);
    } else {
      spdlog::debug("In LRU_K_Cache: key {} not in cache, adding to temp cache",
                    key);
      if (tempCache_.size() + permanentCache_.size() >= capacity_) {
        spdlog::info("In LRU_K_Cache: cache is full, removing oldest key from "
                     "temp cache");
        tempCache_.remove_oldest();
      }
      tempCache_.put(key, {value, 1});
    }
  }
};

#endif // DNS_RELAY_LRU_K_CACHE_H
