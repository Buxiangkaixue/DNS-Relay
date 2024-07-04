#include <iostream>
#include <map>
#include <spdlog/spdlog.h>
#include <string>

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

void initialize_logging(const std::string &log_level) {
  auto it = log_level_map.find(log_level);
  if (it != log_level_map.end()) {
    spdlog::set_level(it->second);
    spdlog::info("Log level set to {}", log_level);
  } else {
    std::cerr << "Invalid log level: " << log_level << std::endl;
    exit(EXIT_FAILURE);
  }
}