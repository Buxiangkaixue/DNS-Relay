#ifndef DNS_RELAY_IP_RESULT_H
#define DNS_RELAY_IP_RESULT_H

#include <string>
#include <vector>
#include <fmt/format.h>
#include <fmt/ranges.h>

class IP_Result {
public:
  IP_Result();
  IP_Result(const std::vector<std::string>& ipv4, const std::vector<std::string>& ipv6);

  std::vector<std::string> ipv4;
  std::vector<std::string> ipv6;
};

// 特化format
template <>
struct fmt::formatter<IP_Result> {
  // 解析格式化规范字符串（如果有的话）
  constexpr auto parse(fmt::format_parse_context &ctx) {
    // 这里我们可以忽略格式规范，因为默认的格式化就可以满足要求
    return ctx.begin();
  }

  // 格式化 IP_Result 对象
  template <typename FormatContext>
  auto format(const IP_Result &result, FormatContext &ctx) const {
    return fmt::format_to(ctx.out(), "IPv4: [{}], IPv6: [{}]",
                          fmt::join(result.ipv4, ", "),
                          fmt::join(result.ipv6, ", "));
  }
};

#endif // DNS_RELAY_IP_RESULT_H
