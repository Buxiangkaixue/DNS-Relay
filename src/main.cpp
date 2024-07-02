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
  auto ret_ip = resolve_hostname("baidu.com");
  print_dns_query_result(ret_ip);
}