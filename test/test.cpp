//
// Created by Leoevergarden on 2024/7/1.
//
#include "add.h"
#include "sub.h"

#include <catch2/catch_all.hpp>
#include <fmt/format.h>
#include <iostream>
#include <spdlog/spdlog.h>

TEST_CASE("test for catch2", "[test]") {

  fmt::print("Hello, {}!\n", "world");
  std::cout << "fmt version: " << FMT_VERSION / 10000 << "."
            << FMT_VERSION / 100 % 100 << "." << FMT_VERSION % 100 << std::endl;
  std::cout << "spdlog version: " << SPDLOG_VER_MAJOR << "." << SPDLOG_VER_MINOR
            << "." << SPDLOG_VER_PATCH << std::endl;

  REQUIRE(3 == add(1, 2));
  REQUIRE(-1 == sub(1, 2));
}