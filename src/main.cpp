#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#define PORT 53
#define BUFFER_SIZE 512

void print_hex(const char *data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%02x ", (unsigned char)data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

void print_dns_query(const char *buffer, ssize_t len) {
    const char *query = buffer + 12;
    while (*query) {
        int length = *query;
        query++;
        for (int i = 0; i < length; ++i) {
            printf("%c", *query);
            query++;
        }
        if (*query) printf(".");
    }
    printf("\n");
}

void construct_dns_query(std::string hostname, char *buffer, size_t &query_size) {
    unsigned short id = htons(0x1234);
    unsigned short flags = htons(0x0100);
    unsigned short qdcount = htons(1);
    unsigned short ancount = 0;
    unsigned short nscount = 0;
    unsigned short arcount = 0;

    memcpy(buffer, &id, 2);
    memcpy(buffer + 2, &flags, 2);
    memcpy(buffer + 4, &qdcount, 2);
    memcpy(buffer + 6, &ancount, 2);
    memcpy(buffer + 8, &nscount, 2);
    memcpy(buffer + 10, &arcount, 2);

    query_size = 12;

    size_t pos = 0;
    while ((pos = hostname.find('.')) != std::string::npos) {
        buffer[query_size++] = pos;
        memcpy(buffer + query_size, hostname.c_str(), pos);
        query_size += pos;
        hostname = hostname.substr(pos + 1);
    }
    buffer[query_size++] = hostname.size();
    memcpy(buffer + query_size, hostname.c_str(), hostname.size());
    query_size += hostname.size();
    buffer[query_size++] = 0;

    unsigned short qtype = htons(1);
    unsigned short qclass = htons(1);
    memcpy(buffer + query_size, &qtype, 2);
    query_size += 2;
    memcpy(buffer + query_size, &qclass, 2);
    query_size += 2;
}

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

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("8.8.8.8");

    construct_dns_query(hostname, buffer, query_size);

    if (sendto(sockfd, buffer, query_size, 0, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Send failed");
        close(sockfd);
        return 1;
    }

    socklen_t addr_len = sizeof(server_addr);
    ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
    if (n < 0) {
        perror("Receive failed");
        close(sockfd);
        return 1;
    }

    std::cout << "Received DNS response:" << std::endl;
    print_hex(buffer, n);
    std::cout << "Domain name: ";
    print_dns_query(buffer, n);

    close(sockfd);
    return 0;
}
