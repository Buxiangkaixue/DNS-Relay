#ifndef DNS_RELAY_LRU_CACHE_H
#define DNS_RELAY_LRU_CACHE_H

#include <list>
#include <mutex>
#include <shared_mutex>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <unordered_map>

// 正确的前向声明，包含所有模板参数
template <typename Key, typename Value, size_t NUM_K> class LRU_K_Cache;

// 预先定义的 LRU_Cache 类
template <typename Key, typename Value> class LRU_Cache {
  // 设置LRU_K_Cache为友元类 从而在LRU_K中
  // temp移动到permanent时 可以避免copy
  // TODO 友元模板声明，确保只有相同 Key 和 Value 类型的 LRU_K_Cache 可以访问
  // 不知道如何写模板类的友元 可能现在require还不支持 友元模板类
  template <typename, typename, size_t NUM_K> friend class LRU_K_Cache;

private:
  std::list<std::pair<Key, Value>> cacheItemsList_;
  std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator>
      cacheMap_;
  size_t capacity_;
  mutable std::shared_mutex mutex_; // 保护数据结构的共享互斥锁

  Value &get_ref(const Key &key) {
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

public:
  LRU_Cache(size_t capacity) : capacity_(capacity) {
    spdlog::info("LRU_Cache created with capacity {}", capacity_);
  }

  Value get(const Key &key) {
    std::unique_lock lock(mutex_);
    return get_ref(key);
  }

  template <typename K, typename V>
    requires std::is_same_v<std::decay_t<K>, Key> &&
             std::is_same_v<std::decay_t<V>, Value>
  void put(K &&key, V &&value) {
    std::unique_lock lock(mutex_);
    auto it = cacheMap_.find(key);
    if (it != cacheMap_.end()) {
      cacheItemsList_.splice(cacheItemsList_.begin(), cacheItemsList_,
                             it->second);
      it->second->second = std::forward<V>(value);
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

    // 首先插入新元素
    cacheItemsList_.emplace_front(std::forward<K>(key), std::forward<V>(value));
    // 然后更新哈希表，使其指向新插入的元素
    cacheMap_[cacheItemsList_.front().first] = cacheItemsList_.begin();
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
