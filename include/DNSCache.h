#ifndef DNS_RELAY_DNSCACHE_H
#define DNS_RELAY_DNSCACHE_H

#include <unordered_map>
#include <string>
#include <chrono>
#include <shared_mutex>

// 定义DNS记录结构
struct DNSRecord {
    std::string ip_address;  // IP地址
    std::chrono::steady_clock::time_point expiry;  // 过期时间
};

class DNSCache {
    std::unordered_map<std::string, DNSRecord> cache;
    mutable std::shared_mutex cache_mutex;  // 使用 shared_mutex
    size_t max_size;

public:
    // 构造函数，设置最大缓存大小和默认TTL
    DNSCache(size_t size, int ttl);

    // 将DNS记录注册到缓存中
    void registerRecord(const std::string& domain, const std::string& ip, int ttl);

    // 查询DNS记录
    std::string query(const std::string& domain) const;

    // 删除特定DNS记录
    void removeRecord(const std::string& domain);

    // 清理过期的记录
    void evictRecords();
};

#endif // DNS_RELAY_DNSCACHE_H