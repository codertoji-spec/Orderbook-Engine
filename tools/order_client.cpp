#include <iostream>
#include <string>
#include "../include/tcp_client.hpp"

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    uint16_t port = 9000;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        }
    }

    TcpClient client(host, port);
    if (!client.connectToServer()) {
        std::cerr << "Failed to connect to " << host << ":" << port << "\n";
        return 1;
    }

    std::cout << "Connected to " << host << ":" << port << ". Type commands (e.g., NEW BUY 1 1000 10) or 'quit':\n";
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "quit" || line == "exit") break;
        if (line.empty()) continue;

        std::string resp;
        if (client.sendCommand(line, resp)) {
            std::cout << "Response: " << resp;
        } else {
            std::cerr << "Failed to send command.\n";
            break;
        }
    }

    return 0;
}
