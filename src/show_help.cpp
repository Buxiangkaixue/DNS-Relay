#include <iostream>
#include <string>

void show_usage(const std::string &prog_name) {
  std::cerr << "Usage: " << prog_name
            << " [-l log_level] [-t thread_count] [-d dns_server] [-h]" << std::endl;
  std::cerr << "  -l log_level   Set log level (trace, debug, info, warn, "
               "error, critical, off)"
            << std::endl;
  std::cerr << "  -t thread_count Set the number of threads in the thread pool"
            << std::endl;
  std::cerr << "  -d dns_server  Set the DNS server address" << std::endl;
  std::cerr << "  -h             Show this help message" << std::endl;
}
