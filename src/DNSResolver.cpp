//
// Created by stellaura on 02/07/24.
//
#include "IP_Result.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <vector>

IP_Result dns_resolve_hostname(const std::string &hostname) {
  addrinfo hints; // 查询的配置文件 指定了查询的方式
  addrinfo *res;  // 查询的结果的返回位置 是一个链表

  // ipv6地址存储所需要的最小的字符长度，用作缓冲区长度，保证可以存储一个ipv6地址
  char ipstr[INET6_ADDRSTRLEN];

  std::vector<std::string> ipv4_addresses;
  std::vector<std::string> ipv6_addresses;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  // 查询的错误码返回
  int status;
  // 因为需要修改res指针的值，需要对指针取地址
  if ((status = getaddrinfo(hostname.c_str(), NULL, &hints, &res)) != 0) {
    std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
    return {ipv4_addresses, ipv6_addresses};
  }

  addrinfo *p;
  for (p = res; p != NULL; p = p->ai_next) {
    void *addr;

    if (p->ai_family == AF_INET) { // IPv4
      sockaddr_in *ipv4 = (sockaddr_in *)p->ai_addr;
      addr = &(ipv4->sin_addr);
      inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
      ipv4_addresses.push_back(ipstr);
    } else { // IPv6
      sockaddr_in6 *ipv6 = (sockaddr_in6 *)p->ai_addr;
      addr = &(ipv6->sin6_addr);
      inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
      ipv6_addresses.push_back(ipstr);
    }
  }

  freeaddrinfo(res); // free the linked list

  return {ipv4_addresses, ipv6_addresses};
}