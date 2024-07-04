#include "IP_Result.h"
#include <spdlog/spdlog.h>

// IP_Result 构造函数的实现
IP_Result::IP_Result() = default;
IP_Result::IP_Result(const std::vector<std::string> &ipv4,
                     const std::vector<std::string> &ipv6)
    : ipv4(ipv4), ipv6(ipv6) {}

// 移动构造函数
IP_Result::IP_Result(IP_Result &&other) noexcept
    : ipv4(std::move(other.ipv4)), ipv6(std::move(other.ipv6)) {
  spdlog::trace("IP_Result move constructor called");
}

// 移动赋值运算符
IP_Result &IP_Result::operator=(IP_Result &&other) noexcept {
  if (this != &other) {
    ipv4 = std::move(other.ipv4);
    ipv6 = std::move(other.ipv6);
    spdlog::trace("IP_Result move assignment operator called");
  }
  return *this;
}
