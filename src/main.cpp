//#include "DNSQuery.h"
//#include "FileDatabase.h"
//#include "LRUCache.h"
//#include "utility.h"
//
//#include <fmt/format.h>
//#include <spdlog/spdlog.h>
//
//#include <arpa/inet.h>
//#include <cstring>
//#include <iostream>
//#include <unistd.h>
//#include <vector>
//
//constexpr int PORT = 53;
//constexpr int BUFFER_SIZE = 512;
//
//int main() {
//  //============================================
//  // 初始化 socket 监听UDP端口
//  int sockfd;
//  struct sockaddr_in server_addr, client_addr;
//  char buffer[BUFFER_SIZE];
//  socklen_t addr_len = sizeof(client_addr);
//
//  // 创建 UDP socket
//  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
//    perror("Socket creation failed");
//    exit(EXIT_FAILURE);
//  }
//
//  // 设置服务器地址
//  memset(&server_addr, 0, sizeof(server_addr));
//  server_addr.sin_family = AF_INET;
//  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
//  server_addr.sin_port = htons(PORT);
//
//  // 绑定 socket 到地址和端口
//  if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) <
//      0) {
//    perror("Bind failed");
//    close(sockfd);
//    exit(EXIT_FAILURE);
//  }
//
//  std::cout << "Listening on 127.0.0.1:" << PORT << " for DNS queries..."
//            << std::endl;
//
//  // ================================
//  // 初始化程序查询
//  const std::string file_path =
//      "../data/dnsrelay.txt";
//  LRUCache<std::string, IP_Result> cache(3);
//  FileDatabase file_database(file_path);
//  DNSQuery dns_query(cache, file_database);
//  std::optional<IP_Result> ip_result;
//  while (true) {
//    // 接收 DNS 查询
//    ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
//                         (struct sockaddr *)&client_addr, &addr_len);
//    if (n < 0) {
//      perror("Receive failed");
//      continue;
//    }
//
//    std::cout << "Received DNS query:" << std::endl;
//    print_hex(buffer, n);
//
//    auto domain_name = extract_domain_name(buffer, n);
//    fmt::print("domain name: {}\n", domain_name);
//
//    ip_result = dns_query.dns_query(domain_name);
//    print_dns_query_result(*ip_result);
//
//    if (ip_result) {
//      // 构建 DNS 响应包
//      std::vector<uint8_t> response = build_dns_response(buffer, n, *ip_result);
//
//      // 发送 DNS 响应包
//      sendto(sockfd, response.data(), response.size(), 0, (struct sockaddr *)&client_addr, addr_len);
//    }
//
//  }
//
//  close(sockfd);
//  return 0;
//}
#include "DNSQuery.h"
#include "FileDatabase.h"
#include "LRUCache.h"
#include "utility.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

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
#include <thread>

constexpr int PORT = 53;
constexpr int BUFFER_SIZE = 512;

void handle_udp(int sockfd, LRUCache<std::string, IP_Result>& cache, FileDatabase& file_database) {
    struct sockaddr_in client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);
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

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        std::cout << "Received UDP DNS query from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;

        std::cout << "Received DNS query:" << std::endl;
        print_hex(buffer, n);

        auto domain_name = extract_domain_name(buffer, n);
        fmt::print("domain name: {}\n", domain_name);

        // 提取查询类型
        uint16_t qtype = ntohs(*(uint16_t*)(buffer + n - 4));
        fmt::print("query type: {}\n", qtype);

        ip_result = dns_query.dns_query(domain_name);
        print_dns_query_result(*ip_result);

        if (ip_result) {
            // 构建 DNS 响应包
            std::vector<uint8_t> response = build_dns_response(buffer, n, *ip_result);

            // 发送 DNS 响应包
            sendto(sockfd, response.data(), response.size(), 0, (struct sockaddr *)&client_addr, addr_len);
        }
    }
}

void handle_tcp(int sockfd, LRUCache<std::string, IP_Result>& cache, FileDatabase& file_database) {
    DNSQuery dns_query(cache, file_database);
    std::optional<IP_Result> ip_result;
    char buffer[BUFFER_SIZE];

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int new_sock = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len);
        if (new_sock < 0) {
            perror("Accept failed");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        std::cout << "Received TCP DNS query from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;

        ssize_t n = read(new_sock, buffer, BUFFER_SIZE);
        if (n < 0) {
            perror("Read failed");
            close(new_sock);
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
            write(new_sock, response.data(), response.size());
        }

        close(new_sock);
    }
}

int main() {
    int udp_sockfd, tcp_sockfd;
    struct sockaddr_in server_addr;

    // 创建 UDP socket
    if ((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 创建 TCP socket
    if ((tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("TCP socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    // 绑定 UDP socket 到地址和端口
    if (bind(udp_sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("UDP bind failed");
        close(udp_sockfd);
        exit(EXIT_FAILURE);
    }

    // 绑定 TCP socket 到地址和端口
    if (bind(tcp_sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("TCP bind failed");
        close(tcp_sockfd);
        exit(EXIT_FAILURE);
    }

    // 监听 TCP 连接
    if (listen(tcp_sockfd, 5) < 0) {
        perror("Listen failed");
        close(tcp_sockfd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Listening on 127.0.0.1:" << PORT << " for DNS queries..." << std::endl;

    // 初始化程序查询
    const std::string file_path = "../data/dnsrelay.txt";
    LRUCache<std::string, IP_Result> cache(3);
    FileDatabase file_database(file_path);

    // 启动 UDP 处理线程
    std::thread udp_thread(handle_udp, udp_sockfd, std::ref(cache), std::ref(file_database));

    // 启动 TCP 处理线程
    std::thread tcp_thread(handle_tcp, tcp_sockfd, std::ref(cache), std::ref(file_database));

    udp_thread.join();
    tcp_thread.join();

    close(udp_sockfd);
    close(tcp_sockfd);

    return 0;
}

