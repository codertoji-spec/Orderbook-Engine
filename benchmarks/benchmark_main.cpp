#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include "../include/order_book_optimized.hpp"
#include "../include/matching_engine.hpp"

void producer(MatchingEngine& engine, size_t num_orders, int seed, uint64_t start_id) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<Price> price_dist(9000, 11000);
    std::uniform_int_distribution<Quantity> qty_dist(1, 100);
    std::uniform_int_distribution<int> side_dist(0, 1);

    for (size_t i = 0; i < num_orders; ++i) {
        Side side = side_dist(gen) == 0 ? Side::Buy : Side::Sell;
        Price price = price_dist(gen);
        Quantity qty = qty_dist(gen);
        Timestamp ts = static_cast<Timestamp>(std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
        
        engine.enqueueOrder(Order(start_id + i, side, price, qty, ts));
    }
}

int main(int argc, char* argv[]) {
    size_t num_orders = 1000000;
    int num_producers = 1;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--orders" && i + 1 < argc) {
            num_orders = std::stoull(argv[++i]);
        } else if (arg == "--producers" && i + 1 < argc) {
            num_producers = std::stoi(argv[++i]);
        }
    }

    std::cout << "==============================\n";
    std::cout << "Order Book Benchmark\n";
    std::cout << "==============================\n";
    std::cout << "Orders:          " << num_orders << "\n";
    std::cout << "Producers:       " << num_producers << "\n";
    std::cout << "Execution Mode:  MPSC\n\n";

    auto book = std::make_unique<OrderBookOptimized>();
    MatchingEngine engine(std::move(book));
    
    // Preallocate latencies to avoid reallocation overhead during matching
    // engine.metrics_.latencies_ns.reserve(num_orders); 

    engine.start();

    auto start_time = std::chrono::steady_clock::now();

    std::vector<std::thread> producers;
    size_t orders_per_producer = num_orders / num_producers;
    
    for (int i = 0; i < num_producers; ++i) {
        size_t count = orders_per_producer;
        if (i == num_producers - 1) {
            count += num_orders % num_producers;
        }
        producers.emplace_back(producer, std::ref(engine), count, 42 + i, i * orders_per_producer + 1);
    }

    for (auto& p : producers) {
        p.join();
    }

    while (engine.getMetrics().orders_processed.load() < num_orders) {
        std::this_thread::yield();
    }

    auto end_time = std::chrono::steady_clock::now();
    engine.stop();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    double seconds = static_cast<double>(duration) / 1000.0;
    double ops = seconds > 0 ? static_cast<double>(num_orders) / seconds : 0;

    std::cout << "Throughput:\n";
    std::cout << "    Orders/sec:  " << static_cast<uint64_t>(ops) << "\n\n";

    engine.getMetrics().printReport();

    std::cout << "\nTrades:\n";
    std::cout << "    Generated:   " << engine.getMetrics().trades_generated.load() << "\n";

    return 0;
}
