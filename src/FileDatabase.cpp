#include "FileDatabase.h"
#include "IP_Result.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

FileDatabase::FileDatabase() = default;

FileDatabase::FileDatabase(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open file");
  }

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string domain, type, ip;
    if (!(iss >> domain >> type >> ip)) {
      continue;
    }

    if (type == "A") {
      database[domain].ipv4.push_back(ip);
    } else if (type == "AAAA") {
      database[domain].ipv6.push_back(ip);
    }
  }
}

std::optional<IP_Result> FileDatabase::get(const std::string &domain) const {
  auto it = database.find(domain);
  if (it != database.end()) {
    return it->second;
  }
  return std::nullopt;
}
