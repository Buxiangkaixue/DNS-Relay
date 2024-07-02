#include <iostream>
#include <vector>
#include <arpa/inet.h>
#include <cstring>
#include <string>

#define BUFFER_SIZE 512

void print_hex(const char *data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%02x ", (unsigned char)data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

std::vector<std::string> parse_dns_response(const char *buffer, ssize_t len) {
    std::vector<std::string> ip_addresses;

    // Skip the DNS header (12 bytes)
    const char *ptr = buffer + 12;

    // Skip the question section
    while (*ptr != 0) {
        ptr += (*ptr + 1);
    }
    ptr += 5; // Skip the null byte, QTYPE, and QCLASS

    // Parse the answer section
    for (int i = 0; i < 2; ++i) { // Assume there are 2 answers
        if ((ptr - buffer) >= len) {
            break;
        }
        ptr += 2; // Skip the name (compression pointer)
        ptr += 2; // Skip the type
        ptr += 2; // Skip the class
        ptr += 4; // Skip the TTL
        uint16_t rdlength = ntohs(*((uint16_t*)ptr));
        ptr += 2; // Skip the RDLENGTH

        if (rdlength == 4) { // IPv4 address
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, ptr, ip, INET_ADDRSTRLEN);
            ip_addresses.push_back(std::string(ip));
        }
        ptr += rdlength;
    }

    return ip_addresses;
}

void construct_dns_query(const std::string &hostname, char *buffer, size_t &query_size) {
    // DNS header fields
    unsigned short id = htons(0x1234);
    unsigned short flags = htons(0x0100);  // Standard query with recursion
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

    query_size = 12;  // Initial size is the size of the header

    // Query name
    std::string modifiable_hostname = hostname;  // Make a modifiable copy of hostname
    size_t pos = 0;
    while ((pos = modifiable_hostname.find('.')) != std::string::npos) {
        buffer[query_size++] = pos;
        memcpy(buffer + query_size, modifiable_hostname.c_str(), pos);
        query_size += pos;
        modifiable_hostname = modifiable_hostname.substr(pos + 1);
    }
    buffer[query_size++] = modifiable_hostname.size();
    memcpy(buffer + query_size, modifiable_hostname.c_str(), modifiable_hostname.size());
    query_size += modifiable_hostname.size();
    buffer[query_size++] = 0;

    // Query type and class
    unsigned short qtype = htons(1);  // A record
    unsigned short qclass = htons(1); // IN class
    memcpy(buffer + query_size, &qtype, 2);
    query_size += 2;
    memcpy(buffer + query_size, &qclass, 2);
    query_size += 2;
}
