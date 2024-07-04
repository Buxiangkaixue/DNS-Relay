#include "FileDatabase.h"
#include "IP_Result.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <spdlog/spdlog.h>

FileDatabase::FileDatabase() {
  spdlog::info("FileDatabase: Default constructor called");
}

FileDatabase::FileDatabase(const std::string &filename) {
  spdlog::info("FileDatabase: Loading data from file {}", filename);

  std::ifstream file(filename);
  if (!file.is_open()) {
    spdlog::error("FileDatabase: Could not open file {}", filename);
    throw std::runtime_error("Could not open file");
  }

  std::string line;
  while (std::getline(file, line)) {
    spdlog::debug("FileDatabase: Reading line: {}", line);
    std::istringstream iss(line);
    std::string domain, type, ip;
    if (!(iss >> domain >> type >> ip)) {
      spdlog::warn("FileDatabase: Invalid line format: {}", line);
      continue;
    }

    if (type == "A") {
      database[domain].ipv4.push_back(ip);
      spdlog::debug("FileDatabase: Added IPv4 address {} for domain {}", ip, domain);
    } else if (type == "AAAA") {
      database[domain].ipv6.push_back(ip);
      spdlog::debug("FileDatabase: Added IPv6 address {} for domain {}", ip, domain);
    }
  }
  spdlog::info("FileDatabase: Finished loading data from file {}", filename);
}

std::optional<IP_Result> FileDatabase::get(const std::string &domain) const {
  spdlog::info("FileDatabase: Querying domain {}", domain);
  std::lock_guard<std::mutex> lock(mtx); // 使用互斥锁保护数据库访问
  auto it = database.find(domain);
  if (it != database.end()) {
    spdlog::info("FileDatabase: Found entry for domain {}", domain);
    return it->second;
  }
  spdlog::info("FileDatabase: No entry found for domain {}", domain);
  return std::nullopt;
}
