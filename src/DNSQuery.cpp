#include "DNSQuery.h"
#include "DNSResolver.h"
#include "FileDatabase.h"
#include "LRUCache.h"

// 构造函数定义
DNSQuery::DNSQuery(LRUCache<std::string, IP_Result> &cache,
                   FileDatabase &database)
    : cache(cache), database(database) {}

std::optional<IP_Result> DNSQuery::dns_query(const std::string &domain) {

  // 如果数据库存在，先从数据库中查找
  auto ip_result = database.get(domain);
  if (ip_result) {
    cache.put(domain, *ip_result);
    return ip_result;
  }

  // 从缓存中查找
  try {
    auto cacheResult = cache.get(domain);
    return cacheResult;
  } catch (const std::runtime_error &) {
    // 缓存未命中
  }

  // 通过解析主机名获取结果
  auto resolvedResult = dns_resolve_hostname(domain);
  cache.put(domain, resolvedResult);
  return resolvedResult;
}
