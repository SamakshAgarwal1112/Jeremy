#pragma once

#include <string>
#include <future>

class HttpClient {
private:
    std::string upstream_host;
    int upstream_port;

public:
    HttpClient(const std::string& host = "httpbin.org", int port = 80);
    
    std::future<std::string> fetch_async(const std::string& path);
    std::string fetch(const std::string& path);

private:
    std::string create_mock_response(const std::string& path);
};