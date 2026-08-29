#pragma once
#include <string>

class TcpClient {
public:
    TcpClient(const std::string& host, uint16_t port);
    ~TcpClient();

    bool connectToServer();
    void disconnect();
    
    bool sendCommand(const std::string& cmd, std::string& response);

private:
    std::string host_;
    uint16_t port_;
    int socket_;
};
