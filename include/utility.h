#pragma once

#include "IP_Result.h"

#include <string>
#include <vector>

void print_dns_query_result(const IP_Result &ip_ret);

void print_hex(const char *data, size_t len);

std::string extract_domain_name(const char *buffer);

std::vector<uint8_t> build_dns_response(const char *query, ssize_t query_len,
                                        const IP_Result &ip_result);
