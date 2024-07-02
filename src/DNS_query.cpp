//
// Created by stellaura on 02/07/24.
//
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <vector>

std::pair<std::vector<std::string>, std::vector<std::string>>
resolve_hostname(const std::string &hostname) {
  addrinfo hints, *res, *p;
  int status;
  char ipstr[INET6_ADDRSTRLEN];

  std::vector<std::string> ipv4_addresses;
  std::vector<std::string> ipv6_addresses;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(hostname.c_str(), NULL, &hints, &res)) != 0) {
    std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
    return {ipv4_addresses, ipv6_addresses};
  }

  for (p = res; p != NULL; p = p->ai_next) {
    void *addr;
    std::string ipver;

    if (p->ai_family == AF_INET) { // IPv4
      sockaddr_in *ipv4 = (sockaddr_in *)p->ai_addr;
      addr = &(ipv4->sin_addr);
      ipver = "IPv4";
      inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
      ipv4_addresses.push_back(ipstr);
    } else { // IPv6
      sockaddr_in6 *ipv6 = (sockaddr_in6 *)p->ai_addr;
      addr = &(ipv6->sin6_addr);
      ipver = "IPv6";
      inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
      ipv6_addresses.push_back(ipstr);
    }
  }

  freeaddrinfo(res); // free the linked list

  return {ipv4_addresses, ipv6_addresses};
}

int main() {
  std::string hostname = "www.example.com";
  auto result = resolve_hostname(hostname);

  std::cout << "IPv4 addresses for " << hostname << ":\n";
  for (const auto &ip : result.first) {
    std::cout << "  " << ip << std::endl;
  }

  std::cout << "IPv6 addresses for " << hostname << ":\n";
  for (const auto &ip : result.second) {
    std::cout << "  " << ip << std::endl;
  }

  return 0;
}
