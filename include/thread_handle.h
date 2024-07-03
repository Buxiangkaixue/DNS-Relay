//
// Created by stellaura on 03/07/24.
//

#ifndef DNS_RELAY_THREAD_HANDLE_H
#define DNS_RELAY_THREAD_HANDLE_H

#include "CacheConcept.h"
#include "DNSQuery.h"
#include "FileDatabase.h"
#include "IP_Result.h"
#include "utility.h"

#include <arpa/inet.h>
#include <fmt/format.h>
#include <iostream>
#include <string>
#include <thread>

constexpr int BUFFER_SIZE = 512; // default inline

template <typename Cache>
  requires CacheConcept<Cache, std::string, IP_Result>
void handle_request(int sockfd, sockaddr_in client_addr, socklen_t addr_len,
                    std::vector<uint8_t> request, Cache &cache,
                    FileDatabase &file_database) {
  DNSQuery dns_query(cache, file_database);
  std::string domain_name = extract_domain_name(
      (const char *)request.data(), request.size()); // 正确的调用方式
  fmt::print("domain name: {}\n", domain_name);

  // 提取查询类型
  uint16_t qtype = ntohs(*(uint16_t *)(request.data() + request.size() - 4));
  fmt::print("query type: {}\n", qtype);

  auto ip_result = dns_query.dns_query(domain_name);
  print_dns_query_result(*ip_result);

  if (ip_result) {
    // 构建 DNS 响应包
    std::vector<uint8_t> response = build_dns_response(
        (const char *)request.data(), request.size(), *ip_result);

    // 发送 DNS 响应包
    sendto(sockfd, response.data(), response.size(), 0,
           (struct sockaddr *)&client_addr, addr_len);
  }
}

template <typename Cache>
  requires CacheConcept<Cache, std::string, IP_Result>
void handle_udp(int sockfd, Cache &cache, FileDatabase &file_database) {
  while (true) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    std::vector<uint8_t> buffer(BUFFER_SIZE);

    // 接收 DNS 查询
    ssize_t n = recvfrom(sockfd, buffer.data(), BUFFER_SIZE, 0,
                         (struct sockaddr *)&client_addr, &addr_len);
    if (n < 0) {
      perror("Receive failed");
      continue;
    }

    buffer.resize(n); // 调整 buffer 大小到实际接收的数据长度

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    std::cout << "Received UDP DNS query from " << client_ip << ":"
              << ntohs(client_addr.sin_port) << std::endl;

    // 创建新线程处理每个查询
    std::thread request_thread(handle_request<Cache>, sockfd, client_addr,
                               addr_len, std::move(buffer), std::ref(cache),
                               std::ref(file_database));
    request_thread.detach(); // 不等待线程结束
  }
}

#endif // DNS_RELAY_THREAD_HANDLE_H
