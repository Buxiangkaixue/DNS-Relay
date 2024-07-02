#ifndef DNS_RELAY_IP_RESULT_H
#define DNS_RELAY_IP_RESULT_H

#include <string>
#include <vector>

class IP_Result {
public:
  IP_Result();
  IP_Result(const std::vector<std::string>& ipv4, const std::vector<std::string>& ipv6);

  std::vector<std::string> ipv4;
  std::vector<std::string> ipv6;
};

#endif // DNS_RELAY_IP_RESULT_H
