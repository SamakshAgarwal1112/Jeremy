#include "http_server.h"
#include "cache.h"
#include "http_client.h"
#include <iostream>
#include <thread>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

HttpServer::HttpServer(int port, Cache* cache, HttpClient* client) 
    : port(port), server_socket(-1), cache(cache), http_client(client) {}

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::start() {
    return create_socket();
}

void HttpServer::run() {
    if (server_socket < 0) {
        std::cerr << "Server socket not initialized" << std::endl;
        return;
    }

    std::cout << "Async Cache Proxy started on port " << port << std::endl;
    std::cout << "Health check available at: http://localhost:" << port << "/health" << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }
        
        handle_client_async(client_socket);
    }
}

void HttpServer::stop() {
    if (server_socket >= 0) {
        close(server_socket);
        server_socket = -1;
    }
}

bool HttpServer::create_socket() {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Failed to set socket options" << std::endl;
        close(server_socket);
        return false;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to bind socket to port " << port << std::endl;
        close(server_socket);
        return false;
    }

    if (listen(server_socket, 10) < 0) {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(server_socket);
        return false;
    }

    return true;
}

void HttpServer::handle_client_async(int client_socket) {
    std::thread([this, client_socket]() {
        char buffer[4096];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            close(client_socket);
            return;
        }
        
        buffer[bytes_received] = '\0';
        std::string request(buffer);
        std::string path = parse_path_from_request(request);
        
        std::cout << "Handling request for: " << path << std::endl;
        
        std::string response;
        
        if (path == "/health") {
            response = create_health_response();
        } else {
            std::string cached_response = cache->get(path);
            
            if (!cached_response.empty()) {
                response = cached_response;
            } else {
                auto future_response = http_client->fetch_async(path);
                std::string upstream_response = future_response.get();
                
                cache->store(path, upstream_response);
                response = upstream_response;
            }
        }
        
        send(client_socket, response.c_str(), response.length(), 0);
        close(client_socket);
    }).detach();
}

std::string HttpServer::parse_path_from_request(const std::string& request) {
    size_t start = request.find("GET ");
    if (start == std::string::npos) return "/";
    
    start += 4; // Length of "GET "
    size_t end = request.find(" HTTP", start);
    if (end == std::string::npos) return "/";
    
    return request.substr(start, end - start);
}

std::string HttpServer::create_health_response() {
    std::ostringstream body;
    body << "{"
         << "\"status\":\"healthy\","
         << "\"cache_size\":" << cache->size() << ","
         << "\"timestamp\":" << std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()
         << "}";
    
    std::string body_str = body.str();
    
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: application/json\r\n"
             << "Content-Length: " << body_str.length() << "\r\n"
             << "Cache-Control: no-cache\r\n\r\n"
             << body_str;
    
    return response.str();
}

std::string HttpServer::create_error_response(int status_code, const std::string& message) {
    std::ostringstream body;
    body << "{\"error\":\"" << message << "\",\"status\":" << status_code << "}";
    
    std::string body_str = body.str();
    
    std::ostringstream response;
    response << "HTTP/1.1 " << status_code << " Error\r\n"
             << "Content-Type: application/json\r\n"
             << "Content-Length: " << body_str.length() << "\r\n\r\n"
             << body_str;
    
    return response.str();
}