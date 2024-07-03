#ifndef FILEDATABASE_H
#define FILEDATABASE_H

#include <mutex>
#include "IP_Result.h"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class FileDatabase {
public:
  FileDatabase();
  FileDatabase(const std::string &filename);

  std::optional<IP_Result> get(const std::string &domain) const;

private:
  std::unordered_map<std::string, IP_Result> database;
  mutable std::mutex mtx; // 互斥锁，mutable 允许在 const 成员函数中锁定和解锁
};

#endif // FILEDATABASE_H
