#include <iostream>
#include <cstring>
#include <string>
#include <stdexcept>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

std::string get_public_ip() {
    const char* server = "ifconfig.me";
    const int port = 80;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int err = getaddrinfo(server, std::to_string(port).c_str(), &hints, &res);
    if (err != 0) {
        close(sock);
        throw std::runtime_error("Failed to resolve hostname");
    }

    struct sockaddr_in* server_addr = (struct sockaddr_in*)res->ai_addr;
    
    if (connect(sock, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0) {
        freeaddrinfo(res);
        close(sock);
        throw std::runtime_error("Failed to connect to server");
    }

    std::string request = "GET /ip HTTP/1.1\r\n"
                          "Host: ifconfig.me\r\n"
                          "Connection: close\r\n\r\n";
    if (send(sock, request.c_str(), request.size(), 0) < 0) {
        freeaddrinfo(res);
        close(sock);
        throw std::runtime_error("Failed to send HTTP request");
    }

    char buffer[1024];
    std::string response;
    ssize_t bytes_received;
    while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        response.append(buffer, bytes_received);
    }

    if (bytes_received < 0) {
        freeaddrinfo(res);
        close(sock);
        throw std::runtime_error("Failed to receive HTTP response");
    }

    freeaddrinfo(res);
    close(sock);

    size_t ip_start = response.find("\r\n\r\n");
    if (ip_start == std::string::npos) {
        throw std::runtime_error("Invalid HTTP response format");
    }
    return response.substr(ip_start + 4);
}

bool server_check() {
    try {
        std::string public_ip = get_public_ip();
        std::vector<std::string> allowed_ips = {
            "172.232.115.25"
        };

        for (const auto& allowed_ip : allowed_ips) {
            if (public_ip == allowed_ip) {
                return true;
            }
        }

        std::cerr << "╒══════════════════════════════════╕" << std::endl;
        std::cerr << "│THE PULGIN POWERED BY VCH R&D TEAM│" << std::endl;
        std::cerr << "| You can't run Vortexiaa Sampvoice|" << std::endl;
        std::cerr << "└──────────────────────────────────┘" << std::endl;
        std::exit(EXIT_FAILURE);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    return false;
}
