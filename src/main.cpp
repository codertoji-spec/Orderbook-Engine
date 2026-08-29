#include <iostream>
#include <memory>
#include <csignal>
#include "../include/order_book_optimized.hpp"
#include "../include/matching_engine.hpp"
#include "../include/tcp_server.hpp"

std::unique_ptr<TcpServer> server;

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received. Shutting down...\n";
    if (server) {
        server->stop();
    }
}

int main(int argc, char* argv[]) {
    uint16_t port = 9000;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        }
    }

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    auto book = std::make_unique<OrderBookOptimized>();
    MatchingEngine engine(std::move(book));
    engine.start();

    server = std::make_unique<TcpServer>(port, engine);
    server->start();

    // Block main thread until server stops
    while (engine.isRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // You could print stats here periodically
    }

    server.reset();
    return 0;
}
