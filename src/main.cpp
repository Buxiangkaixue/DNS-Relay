#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <vector>
#include "DNS_utils.h"
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#define PORT 53
#define BUFFER_SIZE 512

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <hostname>" << std::endl;
        return 1;
    }

    std::string hostname = argv[1];
    fmt::print("Querying DNS for: {}\n", hostname);

    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    size_t query_size;

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Setup server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("8.8.8.8"); // Using Google DNS server

    // Construct DNS query
    construct_dns_query(hostname, buffer, query_size);

    // Send DNS query
    if (sendto(sockfd, buffer, query_size, 0, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Send failed");
        close(sockfd);
        return 1;
    }

    // Receive DNS response
    socklen_t addr_len = sizeof(server_addr);
    ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
    if (n < 0) {
        perror("Receive failed");
        close(sockfd);
        return 1;
    }

    std::cout << "Received DNS response:" << std::endl;
    print_hex(buffer, n);

    std::vector<std::string> ip_addresses = parse_dns_response(buffer, n);
    std::cout << "IP Addresses:" << std::endl;
    for (const auto &ip : ip_addresses) {
        std::cout << ip << std::endl;
    }

    close(sockfd);
    return 0;
}
