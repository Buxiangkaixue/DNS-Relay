#ifndef DNS_RELAY_DNSQUERY_H
#define DNS_RELAY_DNSQUERY_H

#include "CacheConcept.h"
#include "DNSResolver.h"
#include "FileDatabase.h"
#include "IP_Result.h"

#include <optional>
#include <set>
#include <stdexcept>
#include <string>

template <typename Cache>
  requires CacheConcept<Cache, std::string, IP_Result>
class DNSQuery {
public:
  DNSQuery(Cache &cache, FileDatabase &database)
      : cache(cache), database(database) {
    blocked_domains = {"test0"};
  }

  std::optional<IP_Result> dns_query(const std::string &domain) {
    if (blocked_domains.find(domain) != blocked_domains.end()) {
      return std::nullopt; // 返回空以指示域名被屏蔽
    }

    // 从缓存中查找
    try {
      auto cacheResult = cache.get(domain);
      return cacheResult;
    } catch (const std::runtime_error &) {
      // 缓存未命中
    }

    // 如果数据库存在，先从数据库中查找
    auto ip_result = database.get(domain);
    if (ip_result) {
      cache.put(domain, *ip_result);
      return ip_result;
    }

    // 通过解析主机名获取结果
    auto resolvedResult = dns_resolve_hostname(domain);
    cache.put(domain, resolvedResult);
    return resolvedResult;
  }

private:
  Cache &cache;
  FileDatabase &database;
  std::set<std::string> blocked_domains;
};

#endif // DNS_RELAY_DNSQUERY_H
