//
// Created by stellaura on 02/07/24.
//
#include "IP_Result.h"

#include <fmt/format.h>
#include <string>
#include <vector>
#include <arpa/inet.h>

void print_dns_query_result(const IP_Result& ip_ret) {
  if (!ip_ret.ipv4.empty()) {
    fmt::print("IPv4: ");
    for (auto ipv4 : ip_ret.ipv4) {
      fmt::print("{}, ", ipv4);
    }
    fmt::print("\n");
  }
  if (!ip_ret.ipv6.empty()) {
    fmt::print("IPv6: ");
    for (auto ipv6 : ip_ret.ipv6) {
      fmt::print("{}, ", ipv6);
    }
    fmt::print("\n");
  }
}


void print_hex(const char *data, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    printf("%02x ", (unsigned char)data[i]);
    if ((i + 1) % 16 == 0) printf("\n");
  }
  printf("\n");
}

std::string extract_domain_name(const char *buffer, ssize_t len) {
  std::string domain_name;
  const char *query = buffer + 12; // Skip the DNS header (12 bytes)

  while (*query) {
    int length = *query;
    query++;
    for (int i = 0; i < length; ++i) {
      domain_name += *query;
      query++;
    }
    if (*query) {
      domain_name += '.';
    }
  }

  return domain_name;
}

std::vector<uint8_t> build_dns_response(const char *query, ssize_t query_len, const IP_Result &ip_result) {
  std::vector<uint8_t> response(query, query + query_len); // Start with the original query
  response[2] = 0x81; // Set the response flags
  response[3] = 0x80; // Set the response flags

  // Calculate the number of answers
  uint16_t answer_count = ip_result.ipv4.size() + ip_result.ipv6.size();
  response[6] = (answer_count >> 8) & 0xFF;
  response[7] = answer_count & 0xFF;

  // Move the response pointer to the end of the query section
  size_t pos = query_len;

  // Add the IPv4 answers
  for (const auto &ip : ip_result.ipv4) {
    response.push_back(0xc0); // Name: pointer to offset 12 (start of the query)
    response.push_back(0x0c);

    response.push_back(0x00); // Type: A (IPv4 address)
    response.push_back(0x01);

    response.push_back(0x00); // Class: IN
    response.push_back(0x01);

    response.push_back(0x00); // TTL: 0 (short TTL for simplicity)
    response.push_back(0x00);
    response.push_back(0x00);
    response.push_back(0x3c); // TTL: 60 seconds

    response.push_back(0x00); // Data length: 4 bytes for an IPv4 address
    response.push_back(0x04);

    // Convert the IP address to binary form and append to the response
    in_addr addr;
    inet_pton(AF_INET, ip.c_str(), &addr);
    response.insert(response.end(), (uint8_t *)&addr, (uint8_t *)&addr + 4);
  }

  // Add the IPv6 answers
  for (const auto &ip : ip_result.ipv6) {
    response.push_back(0xc0); // Name: pointer to offset 12 (start of the query)
    response.push_back(0x0c);

    response.push_back(0x00); // Type: AAAA (IPv6 address)
    response.push_back(0x1c);

    response.push_back(0x00); // Class: IN
    response.push_back(0x01);

    response.push_back(0x00); // TTL: 0 (short TTL for simplicity)
    response.push_back(0x00);
    response.push_back(0x00);
    response.push_back(0x3c); // TTL: 60 seconds

    response.push_back(0x00); // Data length: 16 bytes for an IPv6 address
    response.push_back(0x10);

    // Convert the IP address to binary form and append to the response
    in6_addr addr;
    inet_pton(AF_INET6, ip.c_str(), &addr);
    response.insert(response.end(), (uint8_t *)&addr, (uint8_t *)&addr + 16);
  }

  return response;
}

