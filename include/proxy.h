#pragma once

#include <memory>
#include <thread>

class Cache;
class HttpClient;
class HttpServer;

class AsyncCacheProxy {
private:
    std::unique_ptr<Cache> cache;
    std::unique_ptr<HttpClient> http_client;
    std::unique_ptr<HttpServer> http_server;
    std::thread cleanup_thread;
    bool running;

public:
    AsyncCacheProxy(int port = 8080, int cache_ttl = 300, 
                   const std::string& upstream_host = "httpbin.org", 
                   int upstream_port = 80);
    ~AsyncCacheProxy();
    
    bool start();
    void run();
    void stop();

private:
    void cleanup_expired_cache();
};