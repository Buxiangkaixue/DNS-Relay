#ifndef DNS_RELAY_DNSQUERY_H
#define DNS_RELAY_DNSQUERY_H

#include "CacheConcept.h"
#include "DNSResolver.h"
#include "FileDatabase.h"
#include "IP_Result.h"

#include <optional>
#include <set>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>

template <typename Cache>
  requires CacheConcept<Cache, std::string, IP_Result>
class DNSQuery {
public:
  DNSQuery(Cache &cache, FileDatabase &database,
           const std::vector<std::string> &blocked_domains = {})
      : cache(cache), database(database),
        blocked_domains(blocked_domains.begin(), blocked_domains.end()) {
    spdlog::info(
        "In DNSQuery: DNSQuery object created with initial blocked domains: {}",
        fmt::join(blocked_domains, ", "));
  }

  IP_Result dns_query(const std::string &domain) {
    spdlog::info("In DNSQuery: Querying domain {}", domain);
    if (blocked_domains.find(domain) != blocked_domains.end()) {
      spdlog::warn("In DNSQuery: Domain {} is blocked", domain);
      return IP_Result(); // 返回空以指示域名被屏蔽
    }

    // 如果数据库存在，先从数据库中查找
    auto ip_result = database.get(domain);
    if (ip_result) {
      spdlog::info("In DNSQuery: Domain {} found in database", domain);
      return *ip_result;
    }

    // 从缓存中查找
    try {
      auto cacheResult = cache.get(domain);
      spdlog::info("In DNSQuery: Domain {} found in cache", domain);
      return cacheResult;
    } catch (const std::runtime_error &) {
      // 缓存未命中
      spdlog::debug("In DNSQuery: Domain {} not found in cache", domain);
    }

    // 通过解析主机名获取结果
    auto resolvedResult = dns_resolve_hostname(domain);
    spdlog::info("In DNSQuery: Domain {} resolved via DNS", domain);
    spdlog::debug("In DNSQuery: Domain {} is put into Cache", domain);
    cache.put(domain, resolvedResult);
    return resolvedResult;
  }

private:
  Cache &cache;
  FileDatabase &database;
  std::set<std::string> blocked_domains;
};

#endif // DNS_RELAY_DNSQUERY_H
