#ifndef DNS_RELAY_LRU_K_CACHE_H
#define DNS_RELAY_LRU_K_CACHE_H

#include "LRU_Cache.h"
#include <stdexcept>

template <typename Key, typename Value, size_t K>
class LRU_K_Cache {
private:
  LRU_Cache<Key, std::pair<Value, size_t>> tempCache_; // 包含访问计数
  LRU_Cache<Key, Value> permanentCache_;               // 持久缓存
  size_t capacity_;
  mutable std::shared_mutex mutex_;

  void promoteToPermanent(const Key &key,
                          std::pair<Value, size_t> &valueCountPair) {
    valueCountPair.second += 1;
    if (valueCountPair.second >= K) {
      permanentCache_.put(key, valueCountPair.first);
      tempCache_.remove(key);
    } else {
      tempCache_.put(key, valueCountPair);
    }
  }

public:
  LRU_K_Cache(size_t capacity)
      : capacity_(capacity), tempCache_(capacity),
        permanentCache_(capacity - capacity / 10) {}

  Value get(const Key &key) {
    std::unique_lock lock(mutex_);
    if (permanentCache_.contains(key)) {
      return permanentCache_.get(key);
    } else if (tempCache_.contains(key)) {
      auto valueCountPair = tempCache_.get(key);
      promoteToPermanent(key, valueCountPair);
      return valueCountPair.first;
    } else {
      throw std::runtime_error("Key not found");
    }
  }

  void put(const Key &key, const Value &value) {
    std::unique_lock lock(mutex_);
    if (permanentCache_.contains(key)) {
      permanentCache_.put(key, value);
    } else if (tempCache_.contains(key)) {
      auto valueCountPair = tempCache_.get(key);
      valueCountPair.first = value;
      promoteToPermanent(key, valueCountPair);
    } else {
      if (tempCache_.size() + permanentCache_.size() >= capacity_) {
        tempCache_.remove_oldest();
      }
      tempCache_.put(key, {value, 1});
    }
  }
};

#endif // DNS_RELAY_LRU_K_CACHE_H
