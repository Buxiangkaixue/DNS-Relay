#include "DNSQuery.h"
#include "FileDatabase.h"
#include "LRU_Cache.h"
#include "utility.h"

#include <catch2/catch_all.hpp>
#include <fmt/format.h>
#include <iostream>
#include <spdlog/spdlog.h>

using namespace std::string_literals; // enables s-suffix for std::string
                                      // literals

TEST_CASE("test for catch2", "[test]") {

  fmt::print("Hello, {}!\n", "world");
  std::cout << "fmt version: " << FMT_VERSION / 10000 << "."
            << FMT_VERSION / 100 % 100 << "." << FMT_VERSION % 100 << std::endl;
  std::cout << "spdlog version: " << SPDLOG_VER_MAJOR << "." << SPDLOG_VER_MINOR
            << "." << SPDLOG_VER_PATCH << std::endl;
}

TEST_CASE("test for LRU Cache", "[test]") {
  LRU_Cache<int, std::string> cache(2);
  cache.put(1, "a"s);
  REQUIRE("a" == cache.get(1));
  cache.put(1, "b"s);
  REQUIRE("b" == cache.get(1));
  cache.put(2, "c"s);
  REQUIRE("c" == cache.get(2));
  cache.put(3, "d"s);
  REQUIRE("d" == cache.get(3));
  try {
    cache.get(1);
  } catch (std::runtime_error e) {
    REQUIRE(std::string("Key not found") == e.what());
    fmt::print("{}\n", e.what());
  }
}

TEST_CASE("test for Database File", "[file database]") {
  std::string file_path =
      "/home/stellaura/Programs/c++/DNS-Relay/data/dnsrelay.txt";
  FileDatabase file_database(file_path);
  print_dns_query_result(file_database.get("a.com").value());
  print_dns_query_result(file_database.get("b.com").value());
  REQUIRE(file_database.get("a.com")->ipv4 ==
          std::vector<std::string>{"10.1.1.1", "10.1.1.2", "192.168.0.1"});
  REQUIRE(file_database.get("b.com")->ipv4 ==
          std::vector<std::string>{"0.0.0.0"});
  REQUIRE(!file_database.get("c.com").has_value());
}

TEST_CASE("test dns query", "[dns_query]") {
  LRU_Cache<std::string, IP_Result> cache(3);
  std::string file_path =
      "/home/stellaura/Programs/c++/DNS-Relay/data/dnsrelay.txt";
  FileDatabase file_database(file_path);
  DNSQuery dns_query(cache, file_database, std::string("8.8.8.8"),
                     std::vector<std::string>());

  std::optional<IP_Result> ret;
  for (auto s : {"baidu.com", "a.com", "b.com"}) {
    fmt::print("=================================\n{}:\n", s);
    ret = dns_query.dns_query(s);
    print_dns_query_result(*ret);
  }
}

TEST_CASE("test IP_Result format") { fmt::print("{}", IP_Result()); }
