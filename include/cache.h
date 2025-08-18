#pragma once

#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>

struct CacheEntry {
    std::string data;
    std::chrono::steady_clock::time_point expiry;
};

class Cache {
private:
    std::unordered_map<std::string, CacheEntry> cache_store;
    std::mutex cache_mutex;
    int ttl_seconds;

public:
    explicit Cache(int ttl = 300);
    
    std::string get(const std::string& key);
    void store(const std::string& key, const std::string& data);
    void clear_expired();
    size_t size() const;
};