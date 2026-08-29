#pragma once
#include "order_book_base.hpp"
#include "thread_safe_queue.hpp"
#include "metrics.hpp"
#include <thread>
#include <atomic>
#include <memory>
#include <variant>

struct CancelOrderRequest {
    OrderId id;
};

using EngineMessage = std::variant<Order, CancelOrderRequest>;

class MatchingEngine {
public:
    MatchingEngine(std::unique_ptr<OrderBookBase> book);
    ~MatchingEngine();

    void start();
    void stop();

    void enqueueOrder(Order order);
    void enqueueCancel(OrderId id);

    bool isRunning() const { return running_; }

    Metrics& getMetrics() { return metrics_; }
    void printBookSnapshot() const {
        if (book_) book_->printSnapshot();
    }

private:
    void processLoop();

    std::unique_ptr<OrderBookBase> book_;
    ThreadSafeQueue<EngineMessage> queue_;
    std::thread worker_;
    std::atomic<bool> running_{false};
    
    Metrics metrics_;
};
