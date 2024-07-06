//
// Created by 天牧 on 24-7-2.
//

#ifndef DNS_RELAY_DNS_QUERY_H
#define DNS_RELAY_DNS_QUERY_H

#include "IP_Result.h"

#include <string>
#include <vector>

IP_Result dns_resolve_hostname(int sockfd, const std::string &hostname);

#endif // DNS_RELAY_DNS_QUERY_H
