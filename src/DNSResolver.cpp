#include "IP_Result.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <spdlog/spdlog.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>


// DNS 头部结构
struct DNSHeader {
  uint16_t id; // Identification number

  uint8_t rd : 1;     // Recursion Desired
  uint8_t tc : 1;     // Truncated Message
  uint8_t aa : 1;     // Authoritative Answer
  uint8_t opcode : 4; // Purpose of message
  uint8_t qr : 1;     // Query/Response Flag

  uint8_t rcode : 4; // Response code
  uint8_t cd : 1;    // Checking Disabled
  uint8_t ad : 1;    // Authenticated Data
  uint8_t z : 1;     // Its Z! Reserved
  uint8_t ra : 1;    // Recursion Available

  uint16_t q_count;    // Number of question entries
  uint16_t ans_count;  // Number of answer entries
  uint16_t auth_count; // Number of authority entries
  uint16_t add_count;  // Number of resource entries
};

// DNS 质询结构
struct DNSQuestion {
  uint16_t qtype;
  uint16_t qclass;
};

// 将域名转换为DNS查询格式
void ChangetoDnsNameFormat(unsigned char *dns, const std::string &host) {
  int lock = 0, i;
  strcat((char *)host.c_str(), ".");

  for (i = 0; i < host.size(); i++) {
    if (host[i] == '.') {
      *dns++ = i - lock;
      for (; lock < i; lock++) {
        *dns++ = host[lock];
      }
      lock++;
    }
  }
  *dns++ = '\0';
}

// 解析DNS响应并提取IP地址
void ParseDNSResponse(unsigned char *buffer, int size, IP_Result &result) {
  DNSHeader *dns = nullptr;
  unsigned char *reader = nullptr;
  dns = (DNSHeader *)buffer;

  reader = &buffer[sizeof(DNSHeader) +
                   (strlen((const char *)&buffer[sizeof(DNSHeader)]) + 1) +
                   sizeof(DNSQuestion)];

  for (int i = 0; i < ntohs(dns->ans_count); i++) {
    reader = reader + 2;
    DNSQuestion *question = (DNSQuestion *)reader;
    reader = reader + sizeof(DNSQuestion);

    if (ntohs(question->qtype) == 1) {
      struct sockaddr_in a;
      memcpy(&a.sin_addr.s_addr, reader, sizeof(a.sin_addr.s_addr));
      result.ipv4.push_back(inet_ntoa(a.sin_addr));
      reader = reader + 4;
    } else if (ntohs(question->qtype) == 28) {
      char ip6[INET6_ADDRSTRLEN];
      inet_ntop(AF_INET6, reader, ip6, INET6_ADDRSTRLEN);
      result.ipv6.push_back(ip6);
      reader = reader + 16;
    }
  }
}

IP_Result dns_resolve_hostname(const std::string &hostname,
                               const std::string &dns_server) {
  unsigned char buf[65536], *qname;
  int s;

  struct sockaddr_in dest;
  struct DNSHeader *dns = nullptr;
  struct DNSQuestion *question = nullptr;

  // 创建UDP socket
  s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s < 0) {
    perror("Socket creation failed");
    exit(1);
  }

  dest.sin_family = AF_INET;
  dest.sin_port = htons(53);
  dest.sin_addr.s_addr = inet_addr(dns_server.c_str());

  // 设置DNS头部
  dns = (struct DNSHeader *)&buf;
  dns->id = (unsigned short)htons(getpid());
  dns->qr = 0;
  dns->opcode = 0;
  dns->aa = 0;
  dns->tc = 0;
  dns->rd = 1;
  dns->ra = 0;
  dns->z = 0;
  dns->ad = 0;
  dns->cd = 0;
  dns->rcode = 0;
  dns->q_count = htons(1);
  dns->ans_count = 0;
  dns->auth_count = 0;
  dns->add_count = 0;

  // 设置质询部分
  qname = &buf[sizeof(struct DNSHeader)];
  ChangetoDnsNameFormat(qname, hostname);
  question = (struct DNSQuestion *)&buf[sizeof(struct DNSHeader) +
                                        (strlen((const char *)qname) + 1)];
  question->qtype = htons(1);
  question->qclass = htons(1);

  // 发送DNS查询
  if (sendto(s, (char *)buf,
             sizeof(struct DNSHeader) + (strlen((const char *)qname) + 1) +
                 sizeof(struct DNSQuestion),
             0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
    perror("sendto failed");
    close(s);
    exit(1);
  }

  // 接收DNS响应
  int i = sizeof(dest);
  if (recvfrom(s, (char *)buf, 65536, 0, (struct sockaddr *)&dest,
               (socklen_t *)&i) < 0) {
    perror("recvfrom failed");
    close(s);
    exit(1);
  }

  // 关闭socket
  close(s);

  // 解析响应并提取IP地址
  IP_Result result;
  ParseDNSResponse(buf, sizeof(buf), result);

  return result;
}