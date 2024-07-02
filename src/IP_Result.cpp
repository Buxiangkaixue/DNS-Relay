#include "IP_Result.h"

IP_Result::IP_Result() = default;
IP_Result::IP_Result(const std::vector<std::string> &ipv4,
                     const std::vector<std::string> &ipv6)
    : ipv4(ipv4), ipv6(ipv6) {}
