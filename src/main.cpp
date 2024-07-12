#include "DNSQuery.h"
#include "FileDatabase.h"
#include "LRU_K_Cache.h"
#include "log_initialization.h"
#include "show_help.h"
#include "thread_handle.h"
#include "utility.h"

#include <arpa/inet.h>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <spdlog/spdlog.h>
#include <unistd.h>

constexpr int PORT = 53;

int initialize_udp_socket() {
  int udp_sockfd;
  sockaddr_in server_addr;

  // 创建 UDP socket
  if ((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("UDP socket creation failed");
    exit(EXIT_FAILURE);
  }

  // 设置服务器地址
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(PORT);

  // 绑定 UDP socket 到地址和端口
  if (bind(udp_sockfd, reinterpret_cast<const sockaddr *>(&server_addr),
           sizeof(server_addr)) < 0) {
    perror("UDP bind failed");
    close(udp_sockfd);
    exit(EXIT_FAILURE);
  }

  std::cout << "Listening on 127.0.0.1:" << PORT << " for DNS queries..."
            << std::endl;

  return udp_sockfd;
}

void handle_command_line_arguments(int argc, char *argv[],
                                   std::string &log_level, int &thread_count,
                                   std::string &dns_server) {
  int opt;
  while ((opt = getopt(argc, argv, "l:t:d:h")) != -1) {
    switch (opt) {
    case 'l':
      log_level = optarg;
      break;
    case 't':
      thread_count = std::stoi(optarg);
      break;
    case 'd':
      dns_server = optarg;
      break;
    case 'h':
      show_usage(argv[0]);
      exit(0);
    default:
      show_usage(argv[0]);
      exit(1);
    }
  }
}

std::filesystem::path getExecutableDir() {
  // 获取当前可执行文件的路径
  std::filesystem::path exePath =
      std::filesystem::read_symlink("/proc/self/exe");
  return exePath.parent_path();
}

int main(int argc, char *argv[]) {
  // 处理命令行参数
  std::string log_level = "info"; // 默认日志等级为info
  int thread_count = 4;           // 默认线程数量为4
  std::string dns_sever = "8.8.8.8";
  handle_command_line_arguments(argc, argv, log_level, thread_count, dns_sever);
  spdlog::info("Thread number is {}", thread_count);

  // 初始化日志
  initialize_logging(log_level);

  // 初始化 UDP
  int udp_sockfd = initialize_udp_socket();

  // 初始化程序查询
  std::filesystem::path file_path = getExecutableDir();
  file_path = file_path.parent_path().parent_path();
  spdlog::debug("project root path: {}", file_path.string());
  file_path = file_path / "data" / "dnsrelay.txt";
  LRU_K_Cache<std::string, IP_Result, 2> cache(10);
  FileDatabase file_database(file_path.string());

  // 创建线程池
  ThreadPool thread_pool(static_cast<size_t>(thread_count)); // 例如4个线程
  SocketPool socket_pool(dns_sever, thread_count);

  // 启动 UDP 处理
  handle_udp(udp_sockfd, cache, file_database, socket_pool, thread_pool);

  close(udp_sockfd);

  return 0;
}
