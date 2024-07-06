//
// Created by stellaura on 06/07/24.
//

#ifndef DNS_RELAY_SOCKETPOOL_H
#define DNS_RELAY_SOCKETPOOL_H

#include <arpa/inet.h>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class SocketPool {
public:
  SocketPool(const std::string &dns_server, int pool_size);
  ~SocketPool();

  int get_socket();
  void release_socket(int sockfd);

private:
  std::queue<int> sockets_;
  std::mutex mutex_;
  std::condition_variable cond_var_;
};

#endif // DNS_RELAY_SOCKETPOOL_H
