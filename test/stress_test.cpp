#include <arpa/inet.h>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

// 配置常量
constexpr int PORT = 53;
constexpr int BUFFER_SIZE = 512;
constexpr int NUM_REQUESTS = 10000; // 总请求数量
constexpr int CONCURRENCY = 50;     // 并发线程数量

// 生成DNS查询包的函数
std::vector<uint8_t> create_dns_query(const std::string &domain) {
  std::vector<uint8_t> query;

  // 1. 标识符（2字节）
  query.push_back(0x12); // 标识符高字节，可以根据需要随机生成
  query.push_back(0x34); // 标识符低字节，可以根据需要随机生成

  // 2. 标志（2字节）
  query.push_back(0x01); // QR = 0, OpCode = 0, AA = 0, TC = 0, RD = 1
  query.push_back(0x00); // Z = 0, RCode = 0

  // 3. 问题计数（2字节）
  query.push_back(0x00); // 问题数（高字节）
  query.push_back(0x01); // 问题数（低字节）

  // 4. 回答计数（2字节）
  query.push_back(0x00); // 回答数（高字节）
  query.push_back(0x00); // 回答数（低字节）

  // 5. 授权计数（2字节）
  query.push_back(0x00); // 授权数（高字节）
  query.push_back(0x00); // 授权数（低字节）

  // 6. 附加计数（2字节）
  query.push_back(0x00); // 附加数（高字节）
  query.push_back(0x00); // 附加数（低字节）

  // 7. 查询名称（可变长度）
  size_t pos = 0, next_pos;
  while ((next_pos = domain.find('.', pos)) != std::string::npos) {
    query.push_back(static_cast<uint8_t>(next_pos - pos));
    query.insert(query.end(), domain.begin() + pos, domain.begin() + next_pos);
    pos = next_pos + 1;
  }
  query.push_back(static_cast<uint8_t>(domain.size() - pos));
  query.insert(query.end(), domain.begin() + pos, domain.end());
  query.push_back(0x00); // 结束标签

  // 8. 查询类型（2字节）
  query.push_back(0x00); // 类型高字节
  query.push_back(0x01); // 类型低字节（A记录）

  // 9. 查询类（2字节）
  query.push_back(0x00); // 类高字节
  query.push_back(0x01); // 类低字节（IN）

  return query;
}

// 发送DNS请求的函数
void send_dns_request(int thread_id, int num_requests, int &success_count,
                      int &failure_count,
                      const std::vector<std::vector<uint8_t>> &requests) {
  int sockfd;
  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE];

  // 创建 UDP socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("UDP socket creation failed");
    return;
  }

  // 设置服务器地址
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(PORT);

  // 随机数生成
  std::srand(std::time(nullptr));

  for (int i = 0; i < num_requests; ++i) {
    // 从请求列表中随机选择一个请求
    const std::vector<uint8_t> &request =
        requests[std::rand() % requests.size()];

    // 发送 DNS 查询
    ssize_t n =
        sendto(sockfd, request.data(), request.size(), 0,
               (const struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0) {
      perror("Send failed");
      ++failure_count;
      continue;
    }

    // 接收 DNS 响应
    socklen_t len = sizeof(server_addr);
    n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                 (struct sockaddr *)&server_addr, &len);
    if (n < 0) {
      perror("Receive failed");
      ++failure_count;
    } else {
      ++success_count;
    }
  }

  close(sockfd);
}

int main() {
  std::vector<std::thread> threads;
  int total_success = 0;
  int total_failure = 0;
  auto start_time = std::chrono::high_resolution_clock::now();

  // 初始化DNS请求列表
  std::vector<std::vector<uint8_t>> requests = {
      create_dns_query("microsoft.com"), create_dns_query("google.com"),
      create_dns_query("facebook.com"),  create_dns_query("apple.com"),
      create_dns_query("amazon.com"),    create_dns_query("twitter.com")};

  for (int i = 0; i < CONCURRENCY; ++i) {
    threads.emplace_back(send_dns_request, i, NUM_REQUESTS / CONCURRENCY,
                         std::ref(total_success), std::ref(total_failure),
                         std::ref(requests));
  }

  for (auto &thread : threads) {
    thread.join();
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = end_time - start_time;

  std::cout << "Total requests: " << NUM_REQUESTS << std::endl;
  std::cout << "Successful requests: " << total_success << std::endl;
  std::cout << "Failed requests: " << total_failure << std::endl;
  std::cout << "Total time: " << duration.count() << " seconds" << std::endl;
  std::cout << "Requests per second: " << NUM_REQUESTS / duration.count()
            << std::endl;

  return 0;
}
