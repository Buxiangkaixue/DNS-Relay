#ifndef FILEDATABASE_H
#define FILEDATABASE_H

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
};

#endif // FILEDATABASE_H
