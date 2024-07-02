//
// Created by stellaura on 02/07/24.
//

#ifndef DNS_RELAY_UTILITY_H
#define DNS_RELAY_UTILITY_H

#include <string>
#include <vector>

void print_dns_query_result(
    const std::pair<std::vector<std::string>, std::vector<std::string>>& ret_ip);

#endif // DNS_RELAY_UTILITY_H
