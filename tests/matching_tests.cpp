#include <cassert>
#include <iostream>
#include "../include/order_book_optimized.hpp"

void runMatchingTests() {
    OrderBookOptimized book;
    
    // Resting sell
    book.addOrder(Order(1, Side::Sell, 100, 50, 1));
    
    // Incoming buy matching partially
    auto trades = book.addOrder(Order(2, Side::Buy, 105, 20, 2));
    
    assert(trades.size() == 1);
    assert(trades[0].quantity == 20);
    assert(trades[0].price == 100);
    
    assert(book.askQuantityAt(100) == 30);
    assert(book.bestAsk() == 100);
    assert(!book.bestBid().has_value());
    
    std::cout << "runMatchingTests passed\n";
}
