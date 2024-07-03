#ifndef DNS_RELAY_LRU_KCACHE_H
#define DNS_RELAY_LRU_KCACHE_H

#include "LRUCache.h"

// LRU-KCache 类
template <typename Key, typename Value, size_t K>
class LRU_KCache {
private:
  LRUCache<Key, std::pair<Value, size_t>> tempCache_; // 包含访问计数
  LRUCache<Key, Value> permanentCache_;               // 持久缓存
  size_t capacity_;
  
  void promoteToPermanent(const Key &key, std::pair<Value, size_t> &valueCountPair) {
    valueCountPair.second += 1;
    if (valueCountPair.second >= K) {
      permanentCache_.put(key, valueCountPair.first);
      tempCache_.remove(key);
    } else {
      tempCache_.put(key, valueCountPair);
    }
  }

public:
  LRU_KCache(size_t capacity)
      : capacity_(capacity),
        tempCache_(capacity / 10),
        permanentCache_(capacity - capacity / 10) {}

  Value get(const Key &key) {
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
    if (permanentCache_.contains(key)) {
      permanentCache_.put(key, value);
    } else if (tempCache_.contains(key)) {
      auto valueCountPair = tempCache_.get(key);
      valueCountPair.first = value;
      promoteToPermanent(key, valueCountPair);
    } else {
      tempCache_.put(key, {value, 1});
    }
  }
};


#endif // DNS_RELAY_LRU_KCACHE_H
