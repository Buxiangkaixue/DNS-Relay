#ifndef DNS_RELAY_DNSQUERY_H
#define DNS_RELAY_DNSQUERY_H

#include "CacheConcept.h"
#include "DNSResolver.h"
#include "FileDatabase.h"
#include "IP_Result.h"
#include "SocketPool.h"

#include <optional>
#include <set>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

template <typename Cache>
  requires CacheConcept<Cache, std::string, IP_Result>
class DNSQuery {
public:
  template <typename T>
    requires std::is_same_v<std::decay_t<T>, std::vector<std::string>>
  DNSQuery(Cache &cache, FileDatabase &database, SocketPool &socket_pool,
           T &&blocked_domains = {})
      : cache(cache), database(database), socket_pool(socket_pool),
        blocked_domains(std::forward<T>(blocked_domains).begin(),
                        std::forward<T>(blocked_domains).end()) {
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
    auto socket_id = socket_pool.get_socket();
    spdlog::info("use socket with id {}", socket_id);
    auto resolvedResult = dns_resolve_hostname(socket_id, domain);
    socket_pool.release_socket(socket_id);

    spdlog::info("In DNSQuery: Domain {} resolved via DNS", domain);
    spdlog::debug("In DNSQuery: Domain {} is put into Cache", domain);
    cache.put(domain, resolvedResult);
    return resolvedResult;
  }

private:
  Cache &cache;
  FileDatabase &database;
  std::set<std::string> blocked_domains;
  std::string dns_sever;
  SocketPool &socket_pool;
};

#endif // DNS_RELAY_DNSQUERY_H
