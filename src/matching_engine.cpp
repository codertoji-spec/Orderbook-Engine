#include "../include/matching_engine.hpp"
#include <iostream>

MatchingEngine::MatchingEngine(std::unique_ptr<OrderBookBase> book)
    : book_(std::move(book)) {}

MatchingEngine::~MatchingEngine() {
    stop();
}

void MatchingEngine::start() {
    bool expected = false;
    if (running_.compare_exchange_strong(expected, true)) {
        worker_ = std::thread(&MatchingEngine::processLoop, this);
    }
}

void MatchingEngine::stop() {
    bool expected = true;
    if (running_.compare_exchange_strong(expected, false)) {
        queue_.shutdown();
        if (worker_.joinable()) {
            worker_.join();
        }
    }
}

void MatchingEngine::enqueueOrder(Order order) {
    queue_.push(order);
}

void MatchingEngine::enqueueCancel(OrderId id) {
    queue_.push(CancelOrderRequest{id});
}

void MatchingEngine::processLoop() {
    while (true) {
        auto msg_opt = queue_.wait_and_pop();
        if (!msg_opt) {
            break; // Shutdown triggered and queue is empty
        }
        
        auto start_time = std::chrono::steady_clock::now();
        
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Order>) {
                metrics_.orders_processed++;
                auto trades = book_->addOrder(arg);
                metrics_.trades_generated += trades.size();
                // Optionally handle trades (e.g., notify clients)
            } else if constexpr (std::is_same_v<T, CancelOrderRequest>) {
                bool cancelled = book_->cancelOrder(arg.id);
                if (cancelled) metrics_.cancellations_processed++;
            }
        }, *msg_opt);
        
        auto end_time = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
        metrics_.recordLatency(diff);
    }
}
