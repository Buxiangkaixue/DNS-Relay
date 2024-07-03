#include "DNSQuery.h"
#include "FileDatabase.h"
#include "LRUCache.h"
#include "utility.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <thread>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

constexpr int PORT = 53;
constexpr int BUFFER_SIZE = 512;

void handle_request(int sockfd, sockaddr_in client_addr, socklen_t addr_len, std::vector<uint8_t> request, LRUCache<std::string, IP_Result>& cache, FileDatabase& file_database) {
    DNSQuery dns_query(cache, file_database);
    std::string domain_name = extract_domain_name((const char*)request.data(), request.size()); // 正确的调用方式
    fmt::print("domain name: {}\n", domain_name);

    // 提取查询类型
    uint16_t qtype = ntohs(*(uint16_t*)(request.data() + request.size() - 4));
    fmt::print("query type: {}\n", qtype);

    auto ip_result = dns_query.dns_query(domain_name);
    print_dns_query_result(*ip_result);

    if (ip_result) {
        // 构建 DNS 响应包
        std::vector<uint8_t> response = build_dns_response((const char*)request.data(), request.size(), *ip_result);

        // 发送 DNS 响应包
        sendto(sockfd, response.data(), response.size(), 0, (struct sockaddr *)&client_addr, addr_len);
    }
}


void handle_udp(int sockfd, LRUCache<std::string, IP_Result>& cache, FileDatabase& file_database) {
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        std::vector<uint8_t> buffer(BUFFER_SIZE);

        // 接收 DNS 查询
        ssize_t n = recvfrom(sockfd, buffer.data(), BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) {
            perror("Receive failed");
            continue;
        }

        buffer.resize(n);  // 调整 buffer 大小到实际接收的数据长度

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        std::cout << "Received UDP DNS query from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;

        // 创建新线程处理每个查询
        std::thread request_thread(handle_request, sockfd, client_addr, addr_len, std::move(buffer), std::ref(cache), std::ref(file_database));
        request_thread.detach();  // 不等待线程结束
    }
}

int main() {
    int udp_sockfd;
    struct sockaddr_in server_addr;

    // 创建 UDP socket
    if ((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP socket creation failed");
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

    std::cout << "Listening on 127.0.0.1:" << PORT << " for DNS queries..." << std::endl;

    // 初始化程序查询
    const std::string file_path = "../data/dnsrelay.txt";
    LRUCache<std::string, IP_Result> cache(3);
    FileDatabase file_database(file_path);

    // 启动 UDP 处理线程
    std::thread udp_thread(handle_udp, udp_sockfd, std::ref(cache), std::ref(file_database));

    // 等待线程结束（实际上不会结束，因为handle_udp是一个无限循环）
    udp_thread.join();

    close(udp_sockfd);

    return 0;
}
