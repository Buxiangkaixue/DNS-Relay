#include "DNSQuery.h"
#include "FileDatabase.h"
#include "LRU_K_Cache.h"
#include "thread_handle.h"
#include "utility.h"

#include <arpa/inet.h>
#include <cstring>
#include <fmt/format.h>
#include <iostream>
#include <map>
#include <spdlog/spdlog.h>
#include <thread>
#include <unistd.h>
#include <vector>

constexpr int PORT = 53;

// 映射日志等级字符串到spdlog的等级
std::map<std::string, spdlog::level::level_enum> log_level_map = {
    {"trace", spdlog::level::trace}, {"debug", spdlog::level::debug},
    {"info", spdlog::level::info},   {"warn", spdlog::level::warn},
    {"error", spdlog::level::err},   {"critical", spdlog::level::critical},
    {"off", spdlog::level::off}};

void show_usage(const std::string &prog_name) {
  std::cerr << "Usage: " << prog_name << " [-l log_level] [-h]" << std::endl;
  std::cerr << "  -l log_level   Set log level (trace, debug, info, warn, "
               "error, critical, off)"
            << std::endl;
  std::cerr << "  -h             Show this help message" << std::endl;
}

int main(int argc, char *argv[]) {
  // ==========================
  // log level setting
  int opt;
  std::string log_level = "info"; // 默认日志等级为info

  // 解析命令行参数
  while ((opt = getopt(argc, argv, "l:h")) != -1) {
    switch (opt) {
    case 'l':
      log_level = optarg;
      break;
    case 'h':
      show_usage(argv[0]);
      return 0;
    default:
      show_usage(argv[0]);
      return 1;
    }
  }

  // 设置日志级别
  auto it = log_level_map.find(log_level);
  if (it != log_level_map.end()) {
    spdlog::set_level(it->second);
    spdlog::info("Log level set to {}", log_level);
  } else {
    std::cerr << "Invalid log level: " << log_level << std::endl;
    show_usage(argv[0]);
    return 1;
  }

  // ==========================
  // udp communication
  int udp_sockfd;
  struct sockaddr_in server_addr;

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
  if (bind(udp_sockfd, (const struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    perror("UDP bind failed");
    close(udp_sockfd);
    exit(EXIT_FAILURE);
  }

  std::cout << "Listening on 127.0.0.1:" << PORT << " for DNS queries..."
            << std::endl;

  // 初始化程序查询
  const std::string file_path = "../data/dnsrelay.txt";
  LRU_K_Cache<std::string, IP_Result, 2> cache(10);
  FileDatabase file_database(file_path);

  // 启动 UDP 处理线程
  std::thread udp_thread(handle_udp<LRU_K_Cache<std::string, IP_Result, 2>>,
                         udp_sockfd, std::ref(cache), std::ref(file_database));

  // 等待线程结束（实际上不会结束，因为handle_udp是一个无限循环）
  udp_thread.join();

  close(udp_sockfd);

  return 0;
}
