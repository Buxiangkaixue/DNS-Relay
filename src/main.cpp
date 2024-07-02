//
// Created by Leoevergarden on 2024/7/1.
//
#include "DNS_query.h"
#include "add.h"
#include "sub.h"
#include "utility.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <iostream>

int main() {
  int a, b;
  std::cin >> a >> b;
  fmt::print("a + b = {},\na - b = {}\n", add(a, b), sub(a, b));
  fmt::print("baidu.com\n");
  auto ret_ip = resolve_hostname("baidu.com");
  print_dns_query_result(ret_ip);
}