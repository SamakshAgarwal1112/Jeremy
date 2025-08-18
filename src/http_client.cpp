#include "http_client.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

HttpClient::HttpClient(const std::string& host, int port) 
    : upstream_host(host), upstream_port(port) {}

std::future<std::string> HttpClient::fetch_async(const std::string& path) {
    return std::async(std::launch::async, [this, path]() {
        return this->fetch(path);
    });
}

std::string HttpClient::fetch(const std::string& path) {
    std::cout << "Fetching from upstream: " << upstream_host << ":" << upstream_port << path << std::endl;
    
    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return create_mock_response(path);
    }

    // Resolve hostname
    struct hostent* he = gethostbyname(upstream_host.c_str());
    if (!he) {
        std::cerr << "Hostname resolution failed for: " << upstream_host << std::endl;
        close(sock);
        return create_mock_response(path);
    }

    // Connect to upstream
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(upstream_port);
    memcpy(&server_addr.sin_addr, he->h_addr_list[0], he->h_length);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed to: " << upstream_host << ":" << upstream_port << std::endl;
        close(sock);
        return create_mock_response(path);
    }

    // Send HTTP request
    std::ostringstream request;
    request << "GET " << path << " HTTP/1.1\r\n"
            << "Host: " << upstream_host << "\r\n"
            << "User-Agent: AsyncCacheProxy/1.0\r\n"
            << "Connection: close\r\n\r\n";
    
    std::string req_str = request.str();
    if (send(sock, req_str.c_str(), req_str.length(), 0) < 0) {
        std::cerr << "Send failed" << std::endl;
        close(sock);
        return create_mock_response(path);
    }

    // Read response
    std::string response;
    char buffer[4096];
    int bytes_received;
    
    while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        response += buffer;
    }

    close(sock);
    
    if (response.empty()) {
        std::cerr << "No response from upstream" << std::endl;
        return create_mock_response(path);
    }
    
    return response;
}

std::string HttpClient::create_mock_response(const std::string& path) {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    
    std::ostringstream mock_response;
    std::ostringstream body;
    
    body << "{"
         << "\"message\":\"Mock upstream response\","
         << "\"path\":\"" << path << "\","
         << "\"timestamp\":" << timestamp << ","
         << "\"source\":\"async-cache-proxy\""
         << "}";
    
    std::string body_str = body.str();
    
    mock_response << "HTTP/1.1 200 OK\r\n"
                  << "Content-Type: application/json\r\n"
                  << "Content-Length: " << body_str.length() << "\r\n"
                  << "Cache-Control: max-age=300\r\n"
                  << "X-Proxy-Source: mock\r\n\r\n"
                  << body_str;
    
    return mock_response.str();
}