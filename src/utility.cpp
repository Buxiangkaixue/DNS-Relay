//
// Created by stellaura on 02/07/24.
//
#include <fmt/format.h>
#include <string>
#include <vector>

void print_dns_query_result(const std::pair<std::vector<std::string>, std::vector<std::string>>& ret_ip) {
  if (!ret_ip.first.empty()) {
    fmt::print("IPv4: ");
    for (auto ipv4 : ret_ip.first) {
      fmt::print("{}, ", ipv4);
    }
    fmt::print("\n");
  }
  if (!ret_ip.second.empty()) {
    fmt::print("IPv6: ");
    for (auto ipv6 : ret_ip.second) {
      fmt::print("{}, ", ipv6);
    }
    fmt::print("\n");
  }
}