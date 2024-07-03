#ifndef DNS_RELAY_LRU_CACHE_H
#define DNS_RELAY_LRU_CACHE_H

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
  LRU_Cache(size_t capacity) : capacity_(capacity) {}

  Value get(const Key &key) {
    std::unique_lock lock(mutex_);
    auto it = cacheMap_.find(key);
    if (it == cacheMap_.end()) {
      throw std::runtime_error("Key not found");
    }
    cacheItemsList_.splice(cacheItemsList_.begin(), cacheItemsList_,
                           it->second);
    return it->second->second;
  }

  void put(const Key &key, const Value &value) {
    std::unique_lock lock(mutex_);
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

  size_t size() const {
    std::shared_lock lock(mutex_);
    return cacheItemsList_.size();
  }

  bool contains(const Key &key) const {
    std::shared_lock lock(mutex_);
    return cacheMap_.find(key) != cacheMap_.end();
  }

  void remove(const Key &key) {
    std::unique_lock lock(mutex_);
    auto it = cacheMap_.find(key);
    if (it != cacheMap_.end()) {
      cacheItemsList_.erase(it->second);
      cacheMap_.erase(it);
    }
  }

  // 避免重复上锁 不能调用 remove 函数
  void remove_oldest() {
    std::unique_lock lock(mutex_);
    if (!cacheItemsList_.empty()) {
      auto oldest = cacheItemsList_.back();
      cacheItemsList_.pop_back();
      cacheMap_.erase(oldest.first);
    }
  }
};

#endif // DNS_RELAY_LRU_CACHE_H
