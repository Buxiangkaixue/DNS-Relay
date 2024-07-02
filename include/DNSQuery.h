#ifndef DNS_RELAY_DNSQUERY_H
#define DNS_RELAY_DNSQUERY_H

#include "IP_Result.h"
#include "FileDatabase.h"
#include "LRUCache.h"

#include <string>
#include <optional>
#include <memory>

class DNSQuery {
public:
  DNSQuery(LRUCache<std::string, IP_Result>& cache, FileDatabase& database);

  std::optional<IP_Result> dns_query(const std::string& domain);

private:
  LRUCache<std::string, IP_Result>& cache;
  FileDatabase& database;
};

#endif // DNS_RELAY_DNSQUERY_H
