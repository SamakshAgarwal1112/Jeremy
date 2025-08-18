#include "proxy.h"
#include <iostream>
#include <cstdlib>
#include <csignal>

AsyncCacheProxy* g_proxy = nullptr;

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ". Shutting down gracefully..." << std::endl;
    if (g_proxy) {
        g_proxy->stop();
    }
    exit(0);
}

int get_env_int(const char* name, int default_value) {
    const char* value = std::getenv(name);
    if (value) {
        try {
            return std::stoi(value);
        } catch (const std::exception& e) {
            std::cerr << "Invalid value for " << name << ": " << value 
                      << ". Using default: " << default_value << std::endl;
        }
    }
    return default_value;
}

std::string get_env_string(const char* name, const std::string& default_value) {
    const char* value = std::getenv(name);
    return value ? std::string(value) : default_value;
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    int port = get_env_int("SERVER_PORT", 8080);
    int cache_ttl = get_env_int("CACHE_TTL", 300);
    std::string upstream_host = get_env_string("UPSTREAM_HOST", "httpbin.org");
    int upstream_port = get_env_int("UPSTREAM_PORT", 80);
    
    try {
        
        AsyncCacheProxy proxy(port, cache_ttl, upstream_host, upstream_port);
        g_proxy = &proxy;
        
        if (!proxy.start()) {
            std::cerr << "Failed to start proxy" << std::endl;
            return 1;
        }
        
        proxy.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}