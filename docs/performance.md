# Performance

## Benchmark Methodology
Benchmarks are written using custom timers (`std::chrono::steady_clock`). We measure:
1. End-to-end throughput (Orders processed per second).
2. Latency percentiles (Avg, P50, P90, P99, Max).

## Data Structure Comparisons
- **OrderBookMap**: Uses `std::deque` for FIFO ordering at a price level. Cancellations are `O(N)` where N is the number of orders at that price level because we must linearly scan the deque.
- **OrderBookOptimized**: Uses `std::list` for FIFO ordering and maintains a `std::unordered_map` mapping OrderId to the `std::list::iterator`. Cancellations are strictly `O(1)` amortized.

## Bottlenecks
- **Queue Contention**: With 16+ producers, the single `std::mutex` on the queue becomes the primary bottleneck.
- **Allocation Overhead**: Dynamic allocation (`std::list` node creation) is the next largest bottleneck. A custom object pool or intrusive list would drastically reduce this.
