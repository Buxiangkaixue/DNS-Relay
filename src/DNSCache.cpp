#include <unordered_map>
#include <string>
#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <iostream>
#include <vector>
#include <stdexcept>

// 定义DNS记录结构
struct DNSRecord {
    std::string ip_address;  // IP地址
    std::chrono::steady_clock::time_point expiry;  // 过期时间
};

class DNSCache {
    std::unordered_map<std::string, DNSRecord> cache;
    mutable std::shared_mutex cache_mutex;  // 使用 shared_mutex
    size_t max_size;
    std::chrono::seconds default_ttl;

public:
    // 构造函数，设置最大缓存大小和默认TTL
    DNSCache(size_t size, int ttl) : max_size(size), default_ttl(ttl) {}

    // 将DNS记录注册到缓存中
    void registerRecord(const std::string& domain, const std::string& ip, int ttl) {
        try {
            std::unique_lock<std::shared_mutex> lock(cache_mutex);  // 写操作使用 unique_lock
            auto expiry = std::chrono::steady_clock::now() + std::chrono::seconds(ttl);
            cache[domain] = {ip, expiry};

            // 确保不超过最大大小
            if (cache.size() > max_size) {
                evictRecords();
            }
            std::cout << "Registered: " << domain << " with IP " << ip << " till " << ttl << " seconds." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Exception in registerRecord: " << e.what() << std::endl;
        }
    }

    // 查询DNS记录
    std::string query(const std::string& domain) {
        try {
            std::shared_lock<std::shared_mutex> lock(cache_mutex);  // 读操作使用 shared_lock
            auto it = cache.find(domain);
            if (it != cache.end() && it->second.expiry > std::chrono::steady_clock::now()) {
                std::cout << "Query successful for: " << domain << " -> IP: " << it->second.ip_address << std::endl;
                return it->second.ip_address;
            }
            std::cout << "Query failed for: " << domain << std::endl;
            return "";  // 返回空字符串表示没有找到
        } catch (const std::exception& e) {
            std::cerr << "Exception in query: " << e.what() << std::endl;
            return "";
        }
    }

    // 删除特定DNS记录
    void removeRecord(const std::string& domain) {
        try {
            std::unique_lock<std::shared_mutex> lock(cache_mutex);  // 写操作使用 unique_lock
            if (cache.erase(domain) > 0) {
                std::cout << "Record removed: " << domain << std::endl;
            } else {
                std::cout << "No record found to remove for: " << domain << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception in removeRecord: " << e.what() << std::endl;
        }
    }

    // 清理过期的记录
    void evictRecords() {
        try {
            std::unique_lock<std::shared_mutex> lock(cache_mutex);  // 清理也是写操作
            auto now = std::chrono::steady_clock::now();
            for (auto it = cache.begin(); it != cache.end(); ) {
                if (it->second.expiry <= now) {
                    std::cout << "Evicting expired record: " << it->first << std::endl;
                    it = cache.erase(it);
                } else {
                    ++it;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception in evictRecords: " << e.what() << std::endl;
        }
    }
};
