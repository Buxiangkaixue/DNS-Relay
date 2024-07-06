#include "IP_TYPE.h"

#include "spdlog/spdlog.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

class IP_Result {
public:
  IP_Result() = default;
  IP_Result(const std::vector<std::string> &ipv4,
            const std::vector<std::string> &ipv6)
      : ipv4_addresses(ipv4), ipv6_addresses(ipv6) {}

  std::vector<std::string> ipv4_addresses;
  std::vector<std::string> ipv6_addresses;
};

// 生成DNS查询包的函数
std::vector<uint8_t> create_dns_query(const std::string &domain,
                                      IP_TYPE ip_type) {
  std::vector<uint8_t> query;

  // 1. 标识符（2字节）
  query.push_back(0x12); // 标识符高字节，可以根据需要随机生成
  query.push_back(0x34); // 标识符低字节，可以根据需要随机生成

  // 2. 标志（2字节）
  query.push_back(0x01); // QR = 0, OpCode = 0, AA = 0, TC = 0, RD = 1
  query.push_back(0x00); // Z = 0, RCode = 0

  // 3. 问题计数（2字节）
  query.push_back(0x00); // 问题数（高字节）
  query.push_back(0x01); // 问题数（低字节）

  // 4. 回答计数（2字节）
  query.push_back(0x00); // 回答数（高字节）
  query.push_back(0x00); // 回答数（低字节）

  // 5. 授权计数（2字节）
  query.push_back(0x00); // 授权数（高字节）
  query.push_back(0x00); // 授权数（低字节）

  // 6. 附加计数（2字节）
  query.push_back(0x00); // 附加数（高字节）
  query.push_back(0x00); // 附加数（低字节）

  // 7. 查询名称（可变长度）
  size_t pos = 0, next_pos;
  while ((next_pos = domain.find('.', pos)) != std::string::npos) {
    query.push_back(static_cast<uint8_t>(next_pos - pos));
    query.insert(query.end(), domain.begin() + pos, domain.begin() + next_pos);
    pos = next_pos + 1;
  }
  query.push_back(static_cast<uint8_t>(domain.size() - pos));
  query.insert(query.end(), domain.begin() + pos, domain.end());
  query.push_back(0x00); // 结束标签

  // 8. 查询类型（2字节）
  query.push_back(0x00); // 类型高字节
  if (ip_type == IP_TYPE::IPv4) {
    query.push_back(0x01);
  } else if (ip_type == IP_TYPE::IPv6) {
    query.push_back(0x1c); // 类型低字节（A记录）
  }
  // 9. 查询类（2字节）
  query.push_back(0x00); // 类高字节
  query.push_back(0x01); // 类低字节（IN）

  return query;
}

std::vector<std::string> dns_resolve_hostname_(const std::string &hostname,
                                               const std::string &dns_server,
                                               IP_TYPE query_type) {
  spdlog::info("Resolving hostname: {} using DNS server: {}", hostname,
               dns_server);

  std::vector<uint8_t> query = create_dns_query(hostname, query_type);

  int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sockfd < 0) {
    spdlog::error("Socket creation failed");
    return {};
  }

  struct timeval tv;
  tv.tv_sec = 5; // 5 seconds timeout
  tv.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(53);
  inet_pton(AF_INET, dns_server.c_str(), &server_addr.sin_addr);

  if (sendto(sockfd, query.data(), query.size(), 0, (sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
    spdlog::error("Sendto failed");
    close(sockfd);
    return {};
  }

  uint8_t response[512];
  socklen_t server_addr_len = sizeof(server_addr);
  ssize_t resp_size = recvfrom(sockfd, response, sizeof(response), 0,
                               (sockaddr *)&server_addr, &server_addr_len);
  if (resp_size < 0) {
    spdlog::error("Recvfrom failed or timed out");
    close(sockfd);
    return {};
  }

  close(sockfd);

  std::vector<std::string> ip_addresses;

  int answer_count = (response[6] << 8) | response[7];
  size_t pos = 12;

  while (response[pos] != 0) {
    pos++;
  }
  pos += 5;

  for (int i = 0; i < answer_count; ++i) {
    pos += 2;
    uint16_t type = (response[pos] << 8) | response[pos + 1];
    pos += 8;
    uint16_t data_length = (response[pos] << 8) | response[pos + 1];
    pos += 2;

    if (type == 1 && data_length == 4) { // A record (IPv4)
      char ipv4[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &response[pos], ipv4, sizeof(ipv4));
      ip_addresses.push_back(ipv4);
    } else if (type == 28 && data_length == 16) { // AAAA record (IPv6)
      char ipv6[INET6_ADDRSTRLEN];
      inet_ntop(AF_INET6, &response[pos], ipv6, sizeof(ipv6));
      ip_addresses.push_back(ipv6);
    }
    pos += data_length;
  }

  if (query_type == IP_TYPE::IPv4) {
    spdlog::info("Resolved {}: {} IPv4 addresses", hostname,
                 ip_addresses.size());
  } else if (query_type == IP_TYPE::IPv6) {
    spdlog::info("Resolved {}: {} IPv6 addresses", hostname,
                 ip_addresses.size());
  }

  return ip_addresses;
}

IP_Result dns_resolve_hostname(const std::string &hostname,
                               const std::string &dns_server) {
  return {dns_resolve_hostname_(hostname, dns_server, IP_TYPE::IPv4),
          dns_resolve_hostname_(hostname, dns_server, IP_TYPE::IPv6)};
}