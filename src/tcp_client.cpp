#include "../include/tcp_client.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#ifdef _WIN32
#define CLOSE_SOCKET closesocket
#else
#define CLOSE_SOCKET close
#endif

TcpClient::TcpClient(const std::string& host, uint16_t port)
    : host_(host), port_(port), socket_(-1) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

TcpClient::~TcpClient() {
    disconnect();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool TcpClient::connectToServer() {
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) return false;

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    
    if (inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) <= 0) {
        return false;
    }

    if (connect(socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        return false;
    }

    return true;
}

void TcpClient::disconnect() {
    if (socket_ >= 0) {
        CLOSE_SOCKET(socket_);
        socket_ = -1;
    }
}

bool TcpClient::sendCommand(const std::string& cmd, std::string& response) {
    if (socket_ < 0) return false;
    
    std::string full_cmd = cmd + "\n";
    if (send(socket_, full_cmd.c_str(), full_cmd.size(), 0) < 0) {
        return false;
    }

    char buffer[1024];
    auto bytes_read = recv(socket_, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        response = buffer;
        return true;
    }
    return false;
}
