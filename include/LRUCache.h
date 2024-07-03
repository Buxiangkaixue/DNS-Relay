//    cacheMap_[key] = cacheItemsList_.begin();

// Created by stellaura on 02/07/24.
//

#ifndef DNS_RELAY_LRUCACHE_H
#define DNS_RELAY_LRUCACHE_H

#include <list>
#include <stdexcept>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>

template <typename Key, typename Value>
class LRUCache {
private:
    std::list<std::pair<Key, Value>> cacheItemsList_;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> cacheMap_;
    size_t capacity_;
    mutable std::shared_mutex mutex_;  // 添加 shared_mutex 用于控制读写访问

public:
    LRUCache(size_t capacity) : capacity_(capacity) {}

    Value get(const Key &key) {
        std::shared_lock<std::shared_mutex> lock(mutex_);  // 使用 shared_lock 来允许并发读取
        auto it = cacheMap_.find(key);
        if (it == cacheMap_.end()) {
            throw std::runtime_error("Key not found");
        }
        // 移动节点操作需要独占锁
        lock.unlock();
        std::unique_lock<std::shared_mutex> uniqueLock(mutex_);
        cacheItemsList_.splice(cacheItemsList_.begin(), cacheItemsList_, it->second);
        uniqueLock.unlock();
        std::shared_lock<std::shared_mutex> relock(mutex_);  // 重新加共享锁以返回值
        return it->second->second;
    }

    void put(const Key &key, const Value &value) {
        std::unique_lock<std::shared_mutex> lock(mutex_);  // 使用 unique_lock 来独占锁
        auto it = cacheMap_.find(key);
        if (it != cacheMap_.end()) {
            cacheItemsList_.splice(cacheItemsList_.begin(), cacheItemsList_, it->second);
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
