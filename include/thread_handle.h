#ifndef DNS_RELAY_THREAD_HANDLE_H
#define DNS_RELAY_THREAD_HANDLE_H

#include "CacheConcept.h"
#include "DNSQuery.h"
#include "FileDatabase.h"
#include "IP_Result.h"
#include "SocketPool.h"
#include "ThreadPool.h"
#include "utility.h"

#include <arpa/inet.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>

constexpr int BUFFER_SIZE = 512; // default inline

template <typename Cache>
  requires CacheConcept<Cache, std::string, IP_Result>
void handle_request(int sockfd, sockaddr_in client_addr, socklen_t addr_len,
                    std::vector<uint8_t> request, Cache &cache,
                    FileDatabase &file_database, SocketPool &socket_pool) {
  DNSQuery dns_query(
      cache, file_database, socket_pool,
      std::vector<std::string>{"test0", "baidu.com", "bilibili.com"});
  std::string domain_name = extract_domain_name(
      reinterpret_cast<const char *>(request.data())); // 正确的调用方式
  spdlog::info("domain name: {}", domain_name);

  // 提取查询类型
  uint16_t qtype =
      ntohs(*reinterpret_cast<uint16_t *>(request.data() + request.size() - 4));
  spdlog::info("query type: {}", qtype);

  auto ip_result = dns_query.dns_query(domain_name);

  print_dns_query_result(ip_result);
  // 构建 DNS 响应包
  std::vector<uint8_t> response =
      build_dns_response(reinterpret_cast<const char *>(request.data()),
                         request.size(), ip_result);

  // 发送 DNS 响应包
  sendto(sockfd, response.data(), response.size(), 0,
         reinterpret_cast<struct sockaddr *>(&client_addr), addr_len);
}

template <typename Cache>
  requires CacheConcept<Cache, std::string, IP_Result>
void handle_udp(int sockfd, Cache &cache, FileDatabase &file_database,
                SocketPool &socket_pool, ThreadPool &thread_pool) {
  while (true) {
    sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);
    std::vector<uint8_t> buffer(BUFFER_SIZE);

    // 接收 DNS 查询
    auto n =
        recvfrom(sockfd, buffer.data(), BUFFER_SIZE, 0,
                 reinterpret_cast<struct sockaddr *>(&client_addr), &addr_len);
    if (n < 0) [[unlikely]] {
      perror("Receive failed");
      continue;
    }

    buffer.resize(
        static_cast<unsigned long>(n)); // 调整 buffer 大小到实际接收的数据长度

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    spdlog::info("Received UDP DNS query from {}:{}", client_ip,
                 ntohs(client_addr.sin_port));

    spdlog::debug("Received DNS query:");
    print_hex(reinterpret_cast<const char *>(buffer.data()),
              static_cast<size_t>(n));

    // 使用线程池处理每个查询
    thread_pool.enqueue(handle_request<Cache>, sockfd, client_addr, addr_len,
                        std::move(buffer), std::ref(cache),
                        std::ref(file_database), std::ref(socket_pool));
  }
}

#endif // DNS_RELAY_THREAD_HANDLE_H
