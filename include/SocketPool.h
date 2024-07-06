#ifndef SOCKETPOOL_H
#define SOCKETPOOL_H

#include <arpa/inet.h>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class SocketPool {
public:
  SocketPool(const std::string &dns_server, int pool_size);
  ~SocketPool();

  int getSocket();
  void releaseSocket(int sock);

private:
  int createSocket();

  std::string dns_server_;
  int pool_size_;
  std::queue<int> sockets_;
  std::mutex mutex_;
  std::condition_variable cond_var_;
};

#endif // SOCKETPOOL_H
