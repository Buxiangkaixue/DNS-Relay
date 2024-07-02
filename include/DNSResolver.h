//
// Created by 天牧 on 24-7-2.
//

#ifndef DNS_RELAY_DNS_QUERY_H
#define DNS_RELAY_DNS_QUERY_H

#include <string>
#include <vector>

std::pair<std::vector<std::string>, std::vector<std::string>> resolve_hostname(const std::string &hostname);

#endif // DNS_RELAY_DNS_QUERY_H
