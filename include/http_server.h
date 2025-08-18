#pragma once

#include <string>

class Cache;
class HttpClient;

class HttpServer {
private:
    int port;
    int server_socket;
    Cache* cache;
    HttpClient* http_client;

public:
    HttpServer(int port, Cache* cache, HttpClient* client);
    ~HttpServer();
    
    bool start();
    void run();
    void stop();

private:
    bool create_socket();
    void handle_client_async(int client_socket);
    std::string parse_path_from_request(const std::string& request);
    std::string create_health_response();
    std::string create_error_response(int status_code, const std::string& message);
};