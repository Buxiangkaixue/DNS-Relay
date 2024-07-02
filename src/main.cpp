#include "DNSResolver.h"
#include "utility.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <iostream>

// Assuming resolve_hostname and print_dns_query_result are defined elsewhere
// in DNSResolver.h and utility.h

int main(int argc, char *argv[]) {
    std::cerr << "Number of arguments: " << argc << std::endl;  // Debug print for argc

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <hostname>" << std::endl;
        return 1;
    }

    std::string hostname = argv[1];
    fmt::print("{}\n", hostname);
    auto ret_ip = resolve_hostname(hostname);
    print_dns_query_result(ret_ip);

    return 0;
}
