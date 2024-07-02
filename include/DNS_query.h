//
// Created by stellaura on 02/07/24.
//

#ifndef DNS_RELAY_DNS_QUERY_H
#define DNS_RELAY_DNS_QUERY_H

#include <string>
#include <vector>

std::pair<std::vector<std::string>, std::vector<std::string>> resolve_hostname(const std::string &hostname);

#endif // DNS_RELAY_DNS_QUERY_H
