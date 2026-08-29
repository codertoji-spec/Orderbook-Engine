#include "../include/tcp_server.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

// A simple macro for closing sockets across platforms
#ifdef _WIN32
#define CLOSE_SOCKET closesocket
#else
#define CLOSE_SOCKET close
#endif

TcpServer::TcpServer(uint16_t port, MatchingEngine& engine)
    : port_(port), engine_(engine), server_socket_(-1) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

TcpServer::~TcpServer() {
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

void TcpServer::start() {
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        std::cerr << "Failed to create socket\n";
        return;
    }

    int opt = 1;
#ifdef _WIN32
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);

    if (bind(server_socket_, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        std::cerr << "Failed to bind to port " << port_ << "\n";
        return;
    }

    if (listen(server_socket_, 10) < 0) {
        std::cerr << "Failed to listen\n";
        return;
    }

    running_ = true;
    accept_thread_ = std::thread(&TcpServer::acceptLoop, this);
    std::cout << "TCP Server started on port " << port_ << "\n";
}

void TcpServer::stop() {
    running_ = false;
    if (server_socket_ >= 0) {
        CLOSE_SOCKET(server_socket_);
        server_socket_ = -1;
    }
    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }
    std::lock_guard<std::mutex> lock(client_threads_mutex_);
    for (auto& t : client_threads_) {
        if (t.joinable()) t.join();
    }
}

void TcpServer::acceptLoop() {
    while (running_) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
#ifdef _WIN32
        int client_sock = accept(server_socket_, reinterpret_cast<struct sockaddr*>(&client_addr), (int*)&client_len);
#else
        int client_sock = accept(server_socket_, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len);
#endif

        if (client_sock < 0) {
            if (running_) std::cerr << "Accept failed\n";
            continue;
        }

        std::lock_guard<std::mutex> lock(client_threads_mutex_);
        client_threads_.emplace_back(&TcpServer::handleClient, this, client_sock);
    }
}

void TcpServer::handleClient(int client_socket) {
    char buffer[1024];
    while (running_) {
        auto bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0) {
            break;
        }
        buffer[bytes_read] = '\0';
        
        // Simple human readable parser
        // NEW BUY <id> <price> <qty>
        // CANCEL <id>
        std::string req(buffer);
        std::istringstream iss(req);
        std::string cmd;
        iss >> cmd;

        if (cmd == "NEW") {
            std::string side_str;
            OrderId id;
            Price price;
            Quantity qty;
            iss >> side_str >> id >> price >> qty;
            
            Side side = (side_str == "BUY") ? Side::Buy : Side::Sell;
            Timestamp ts = static_cast<Timestamp>(std::chrono::duration_cast<std::chrono::nanoseconds>(
                   std::chrono::steady_clock::now().time_since_epoch()).count());
            
            engine_.enqueueOrder(Order(id, side, price, qty, ts));
            
            std::string resp = "ACCEPTED order=" + std::to_string(id) + "\n";
            send(client_socket, resp.c_str(), resp.size(), 0);
        } else if (cmd == "CANCEL") {
            OrderId id;
            iss >> id;
            engine_.enqueueCancel(id);
            std::string resp = "CANCEL_REQUESTED order=" + std::to_string(id) + "\n";
            send(client_socket, resp.c_str(), resp.size(), 0);
        }
    }
    CLOSE_SOCKET(client_socket);
}
