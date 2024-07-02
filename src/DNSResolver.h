//
// Created by 天牧 on 24-7-2.
//

// DNSResolver.h

#include <vector>
#include <string>

class DNSResolver {
public:
    // 解析域名到IP地址
    std::pair<std::vector<std::string>, std::vector<std::string>> resolve_hostname(const std::string &hostname);

    // 其他DNS解析相关的函数声明
};
