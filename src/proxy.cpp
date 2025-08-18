#include "proxy.h"
#include "cache.h"
#include "http_client.h"
#include "http_server.h"
#include <iostream>
#include <chrono>

AsyncCacheProxy::AsyncCacheProxy(int port, int cache_ttl, 
                               const std::string& upstream_host, 
                               int upstream_port) 
    : running(false) {
    
    cache = std::make_unique<Cache>(cache_ttl);
    http_client = std::make_unique<HttpClient>(upstream_host, upstream_port);
    http_server = std::make_unique<HttpServer>(port, cache.get(), http_client.get());
    
    std::cout << "AsyncCacheProxy initialized:" << std::endl;
    std::cout << "  Port: " << port << std::endl;
    std::cout << "  Cache TTL: " << cache_ttl << " seconds" << std::endl;
    std::cout << "  Upstream: " << upstream_host << ":" << upstream_port << std::endl;
}

AsyncCacheProxy::~AsyncCacheProxy() {
    stop();
}

bool AsyncCacheProxy::start() {
    if (!http_server->start()) {
        std::cerr << "Failed to start HTTP server" << std::endl;
        return false;
    }
    
    running = true;
    cleanup_thread = std::thread([this]() {
        cleanup_expired_cache();
    });
    
    return true;
}

void AsyncCacheProxy::run() {
    if (!running) {
        std::cerr << "Proxy not started. Call start() first." << std::endl;
        return;
    }
    
    http_server->run();
}

void AsyncCacheProxy::stop() {
    if (running) {
        running = false;
        http_server->stop();
        
        if (cleanup_thread.joinable()) {
            cleanup_thread.join();
        }
        
        std::cout << "AsyncCacheProxy stopped" << std::endl;
    }
}

void AsyncCacheProxy::cleanup_expired_cache() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(60));
        
        if (running) {
            cache->clear_expired();
        }
    }
}