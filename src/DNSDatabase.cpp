//
// Created by 天牧 on 24-7-2.
//

#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>

class LocalDNSDatabase {
    std::unordered_map<std::string, std::string> database;

public:
    // 加载本地数据库
    void load(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            throw std::runtime_error("无法打开文件：" + filename);
        }

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string domain, ip;
            if (!(iss >> domain >> ip)) {
                continue; // 解析错误，跳过当前行
            }
            database[domain] = ip;
        }
    }

    // 查询域名对应的IP地址
    std::string query(const std::string& domain) const {
        auto it = database.find(domain);
        if (it != database.end()) {
            return it->second; // 找到对应IP
        }
        return ""; // 未找到，返回空字符串
    }

    // 添加或更新DNS记录
    void update(const std::string& domain, const std::string& ip) {
        database[domain] = ip;
    }

    // 获取数据库大小
    size_t size() const {
        return database.size();
    }
};
