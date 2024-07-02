#ifndef LOCAL_DNS_DATABASE_H
#define LOCAL_DNS_DATABASE_H

#include <unordered_map>
#include <string>

class LocalDNSDatabase {
public:
    void load(const std::string& filename);
    std::string query(const std::string& domain) const;
    void update(const std::string& domain, const std::string& ip);
    size_t size() const;

private:
    std::unordered_map<std::string, std::string> database;
};

#endif // LOCAL_DNS_DATABASE_H