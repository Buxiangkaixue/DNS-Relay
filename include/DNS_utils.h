#ifndef DNS_UTILS_H
#define DNS_UTILS_H

#include <vector>
#include <string>

void print_hex(const char *data, size_t len);
std::vector<std::string> parse_dns_response(const char *buffer, ssize_t len);
void construct_dns_query(const std::string &hostname, char *buffer, size_t &query_size);

#endif // DNS_UTILS_H
