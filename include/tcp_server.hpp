#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include "matching_engine.hpp"

class TcpServer {
public:
    TcpServer(uint16_t port, MatchingEngine& engine);
    ~TcpServer();

    void start();
    void stop();

private:
    void acceptLoop();
    void handleClient(int client_socket);

    uint16_t port_;
    MatchingEngine& engine_;
    int server_socket_;
    std::atomic<bool> running_{false};
    std::thread accept_thread_;
    std::vector<std::thread> client_threads_;
    std::mutex client_threads_mutex_;
};
