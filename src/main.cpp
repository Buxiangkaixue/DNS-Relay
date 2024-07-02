#include "DNSQuery.h"
#include "FileDatabase.h"
#include "LRUCache.h"
#include "utility.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <vector>

constexpr int PORT = 53;
constexpr int BUFFER_SIZE = 512;

int main() {
  //============================================
  // 初始化 socket 监听UDP端口
  int sockfd;
  struct sockaddr_in server_addr, client_addr;
  char buffer[BUFFER_SIZE];
  socklen_t addr_len = sizeof(client_addr);

  // 创建 UDP socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // 设置服务器地址
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(PORT);

  // 绑定 socket 到地址和端口
  if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Bind failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  std::cout << "Listening on 127.0.0.1:" << PORT << " for DNS queries..."
            << std::endl;

  // ================================
  // 初始化程序查询
  const std::string file_path =
      "/home/stellaura/Programs/c++/DNS-Relay/data/dnsrelay.txt";
  LRUCache<std::string, IP_Result> cache(3);
  FileDatabase file_database(file_path);
  DNSQuery dns_query(cache, file_database);
  std::optional<IP_Result> ip_result;
  while (true) {
    // 接收 DNS 查询
    ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                         (struct sockaddr *)&client_addr, &addr_len);
    if (n < 0) {
      perror("Receive failed");
      continue;
    }

    std::cout << "Received DNS query:" << std::endl;
    print_hex(buffer, n);

    auto domain_name = extract_domain_name(buffer, n);
    fmt::print("domain name: {}\n", domain_name);

    ip_result = dns_query.dns_query(domain_name);
    print_dns_query_result(*ip_result);

    if (ip_result) {
      // 构建 DNS 响应包
      std::vector<uint8_t> response = build_dns_response(buffer, n, *ip_result);

      // 发送 DNS 响应包
      sendto(sockfd, response.data(), response.size(), 0, (struct sockaddr *)&client_addr, addr_len);
    }

  }

  close(sockfd);
  return 0;
}
