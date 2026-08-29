#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <atomic>
#include "../include/order_book_optimized.hpp"
#include "../include/matching_engine.hpp"

std::atomic<bool> run_stress{true};

void producer(MatchingEngine& engine, int seed, uint64_t start_id) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<Price> price_dist(9000, 11000);
    std::uniform_int_distribution<Quantity> qty_dist(1, 100);
    std::uniform_int_distribution<int> side_dist(0, 1);
    std::uniform_int_distribution<int> action_dist(0, 9); // 0-8 NEW, 9 CANCEL

    uint64_t current_id = start_id;
    std::vector<OrderId> active_orders;

    while (run_stress) {
        int action = action_dist(gen);
        if (action < 9 || active_orders.empty()) {
            Side side = side_dist(gen) == 0 ? Side::Buy : Side::Sell;
            Price price = price_dist(gen);
            Quantity qty = qty_dist(gen);
            Timestamp ts = static_cast<Timestamp>(std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
            
            engine.enqueueOrder(Order(current_id, side, price, qty, ts));
            active_orders.push_back(current_id);
            current_id++;
        } else {
            std::uniform_int_distribution<size_t> idx_dist(0, active_orders.size() - 1);
            size_t idx = idx_dist(gen);
            engine.enqueueCancel(active_orders[idx]);
            active_orders[idx] = active_orders.back();
            active_orders.pop_back();
        }
        // Small yield to not completely overwhelm the queue in stress test if needed
        if (current_id % 1000 == 0) std::this_thread::yield();
    }
}

int main(int argc, char* argv[]) {
    int num_threads = 4;
    int duration_sec = 5;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--threads" && i + 1 < argc) {
            num_threads = std::stoi(argv[++i]);
        } else if (arg == "--duration" && i + 1 < argc) {
            duration_sec = std::stoi(argv[++i]);
        }
    }

    std::cout << "Starting stress test with " << num_threads << " threads for " << duration_sec << " seconds.\n";

    auto book = std::make_unique<OrderBookOptimized>();
    MatchingEngine engine(std::move(book));
    engine.start();

    std::vector<std::thread> producers;
    for (int i = 0; i < num_threads; ++i) {
        producers.emplace_back(producer, std::ref(engine), 42 + i, static_cast<uint64_t>(i + 1) * 1000000000ULL);
    }

    std::this_thread::sleep_for(std::chrono::seconds(duration_sec));
    run_stress = false;

    for (auto& p : producers) {
        p.join();
    }

    engine.stop();
    
    std::cout << "Stress test completed.\n";
    std::cout << "Orders processed: " << engine.getMetrics().orders_processed.load() << "\n";
    std::cout << "Cancellations:    " << engine.getMetrics().cancellations_processed.load() << "\n";
    std::cout << "Trades generated: " << engine.getMetrics().trades_generated.load() << "\n";

    return 0;
}
