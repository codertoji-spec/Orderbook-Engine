#include <cassert>
#include <iostream>
#include <thread>
#include "../include/order_book_optimized.hpp"
#include "../include/matching_engine.hpp"

void runConcurrencyTests() {
    auto book = std::make_unique<OrderBookOptimized>();
    MatchingEngine engine(std::move(book));
    engine.start();

    auto producer = [&engine](uint64_t start_id) {
        for (uint64_t i = 0; i < 1000; ++i) {
            engine.enqueueOrder(Order(start_id + i, Side::Buy, 100, 10, i));
        }
    };

    std::thread t1(producer, 0);
    std::thread t2(producer, 1000);

    t1.join();
    t2.join();

    engine.stop();
    
    assert(engine.getMetrics().orders_processed.load() == 2000);
    
    std::cout << "runConcurrencyTests passed\n";
}
