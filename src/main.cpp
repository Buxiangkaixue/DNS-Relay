//
// Created by Leoevergarden on 2024/7/1.
//
#include "DNSResolver.h"
#include "utility.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <iostream>

int main() {
  fmt::print("baidu.com\n");
  auto ret_ip = resolve_hostname("baidu.com");
  print_dns_query_result(ret_ip);
}