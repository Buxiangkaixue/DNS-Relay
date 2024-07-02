//    cacheMap_[key] = cacheItemsList_.begin();

// Created by stellaura on 02/07/24.
//

#ifndef DNS_RELAY_LRUCACHE_H
#define DNS_RELAY_LRUCACHE_H

#include <list>
#include <stdexcept>
#include <unordered_map>

template <typename Key, typename Value> class LRUCache {
private:
  std::list<std::pair<Key, Value>> cacheItemsList_;
  std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator>
      cacheMap_;
  size_t capacity_;

public:
  LRUCache(size_t capacity) : capacity_(capacity) {}

  Value get(const Key &key) {
    auto it = cacheMap_.find(key);
    if (it == cacheMap_.end()) {
      throw std::runtime_error("Key not found");
    }
    cacheItemsList_.splice(cacheItemsList_.begin(), cacheItemsList_,
                           it->second);
    return it->second->second;
  }

  void put(const Key &key, const Value &value) {
    auto it = cacheMap_.find(key);
    if (it != cacheMap_.end()) {
      cacheItemsList_.splice(cacheItemsList_.begin(), cacheItemsList_,
                             it->second);
      it->second->second = value;
      return;
    }

    if (cacheItemsList_.size() == capacity_) {
      auto last = cacheItemsList_.end();
      --last;
      cacheMap_.erase(last->first);
      cacheItemsList_.pop_back();
    }

    cacheItemsList_.emplace_front(key, value);
    cacheMap_[key] = cacheItemsList_.begin();
  }
};

#endif // DNS_RELAY_LRUCACHE_H
