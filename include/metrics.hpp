#pragma once
#include <atomic>
#include <cstdint>
#include <vector>
#include <mutex>
#include <algorithm>

struct Metrics {
    std::atomic<uint64_t> orders_processed{0};
    std::atomic<uint64_t> trades_generated{0};
    std::atomic<uint64_t> cancellations_processed{0};

    // For latency measurements
    std::mutex latency_mutex;
    std::vector<uint64_t> latencies_ns; // Not for production hot-path without pre-allocation

    void recordLatency(uint64_t ns) {
        std::lock_guard<std::mutex> lock(latency_mutex);
        latencies_ns.push_back(ns);
    }

    void printReport() {
        std::lock_guard<std::mutex> lock(latency_mutex);
        if (latencies_ns.empty()) return;
        
        std::sort(latencies_ns.begin(), latencies_ns.end());
        uint64_t sum = 0;
        for (auto l : latencies_ns) sum += l;
        
        uint64_t avg = sum / latencies_ns.size();
        uint64_t p50 = latencies_ns[latencies_ns.size() * 50 / 100];
        uint64_t p95 = latencies_ns[latencies_ns.size() * 95 / 100];
        uint64_t p99 = latencies_ns[latencies_ns.size() * 99 / 100];
        uint64_t max = latencies_ns.back();

        printf("Latency (ns): Avg: %llu, P50: %llu, P95: %llu, P99: %llu, Max: %llu\n",
               static_cast<unsigned long long>(avg), static_cast<unsigned long long>(p50), 
               static_cast<unsigned long long>(p95), static_cast<unsigned long long>(p99), 
               static_cast<unsigned long long>(max));
    }
};
