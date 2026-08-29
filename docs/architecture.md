# Architecture

The system utilizes a Multiple-Producer, Single-Consumer (MPSC) architecture to eliminate contention on the order book itself.

1. **TCP Server**: Listens for incoming connections. Each connection creates a producer thread that parses messages and enqueues `Order` objects.
2. **Thread-Safe Queue**: Uses `std::mutex` and `std::condition_variable` to safely pass messages from multiple producers to the consumer.
3. **Matching Engine**: The single consumer thread. It pops orders from the queue and applies them to the order book.
4. **Order Book**: An optimized data structure (`std::map` of `std::list`s with a `std::unordered_map` index) owned exclusively by the matching engine.

# Ownership Model
The matching engine is the sole owner of the order book. Producer threads never touch the order book. This guarantees deterministic ordering (based on sequence numbers assigned when enqueued) and completely removes the need for locking at the price-level or book-level during matching.
