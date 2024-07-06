#include "SocketPool.h"
#include "string.h"

SocketPool::SocketPool(const std::string &dns_server, int pool_size)
    : dns_server_(dns_server), pool_size_(pool_size) {
  for (int i = 0; i < pool_size_; ++i) {
    int sock = createSocket();
    if (sock != -1) {
      sockets_.push(sock);
    }
  }
}

SocketPool::~SocketPool() {
  std::lock_guard<std::mutex> lock(mutex_);
  while (!sockets_.empty()) {
    int sock = sockets_.front();
    sockets_.pop();
    close(sock);
  }
}

int SocketPool::getSocket() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (sockets_.empty()) {
    cond_var_.wait(lock);
  }
  int sock = sockets_.front();
  sockets_.pop();
  return sock;
}

void SocketPool::releaseSocket(int sock) {
  std::lock_guard<std::mutex> lock(mutex_);
  sockets_.push(sock);
  cond_var_.notify_one();
}

int SocketPool::createSocket() {
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock == -1) {
    std::cerr << "Failed to create socket" << std::endl;
    return -1;
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(53); // DNS typically uses port 53
  if (inet_pton(AF_INET, dns_server_.c_str(), &server_addr.sin_addr) <= 0) {
    std::cerr << "Invalid DNS server address" << std::endl;
    close(sock);
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    std::cerr << "Connection to DNS server failed" << std::endl;
    close(sock);
    return -1;
  }

  return sock;
}
