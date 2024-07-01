//
// Created by Leoevergarden on 2024/7/1.
//
#include "add.h"
#include "sub.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <iostream>

int main() {
  int a, b;
  std::cin >> a >> b;
  fmt::print("a + b = {},\na - b = {}\n", add(a, b), sub(a, b));
}