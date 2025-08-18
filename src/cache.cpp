#include "cache.h"
#include <iostream>

Cache::Cache(int ttl) : ttl_seconds(ttl) {}

std::string Cache::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    auto it = cache_store.find(key);
    
    if (it != cache_store.end()) {
        if (std::chrono::steady_clock::now() < it->second.expiry) {
            std::cout << "Cache HIT for key: " << key << std::endl;
            return it->second.data;
        } else {
            cache_store.erase(it);
        }
    }
    
    std::cout << "Cache MISS for key: " << key << std::endl;
    return "";
}

void Cache::store(const std::string& key, const std::string& data) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    
    CacheEntry entry;
    entry.data = data;
    entry.expiry = std::chrono::steady_clock::now() + std::chrono::seconds(ttl_seconds);
    
    cache_store[key] = entry;
    std::cout << "Stored in cache: " << key << " (TTL: " << ttl_seconds << "s)" << std::endl;
}

void Cache::clear_expired() {
    std::lock_guard<std::mutex> lock(cache_mutex);
    auto now = std::chrono::steady_clock::now();
    
    for (auto it = cache_store.begin(); it != cache_store.end();) {
        if (now >= it->second.expiry) {
            std::cout << "Evicting expired cache entry: " << it->first << std::endl;
            it = cache_store.erase(it);
        } else {
            ++it;
        }
    }
}

size_t Cache::size() const {
    std::lock_guard<std::mutex> lock(cache_mutex);
    return cache_store.size();
}