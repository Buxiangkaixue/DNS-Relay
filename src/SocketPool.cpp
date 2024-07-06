#include "SocketPool.h"
#include <cstring>
#include <iostream>

SocketPool::SocketPool(const std::string &dns_server, int pool_size) {
  for (int i = 0; i < pool_size; ++i) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
      perror("Socket creation failed");
      continue;
    }

    struct timeval tv;
    tv.tv_sec = 5; // 5 seconds timeout
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(53);
    inet_pton(AF_INET, dns_server.c_str(), &server_addr.sin_addr);

    {
      std::lock_guard<std::mutex> lock(mutex_);
      sockets_.push(sockfd);
    }
  }
}

SocketPool::~SocketPool() {
  std::lock_guard<std::mutex> lock(mutex_);
  while (!sockets_.empty()) {
    close(sockets_.front());
    sockets_.pop();
  }
}

int SocketPool::get_socket() {
  std::unique_lock<std::mutex> lock(mutex_);
  cond_var_.wait(lock, [this] { return !sockets_.empty(); });
  int sockfd = sockets_.front();
  sockets_.pop();
  return sockfd;
}

void SocketPool::release_socket(int sockfd) {
  std::lock_guard<std::mutex> lock(mutex_);
  sockets_.push(sockfd);
  cond_var_.notify_one();
}
