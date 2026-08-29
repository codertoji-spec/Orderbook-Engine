#include <cassert>
#include <iostream>
#include "../include/order_book_optimized.hpp"

void runCancellationTests() {
    OrderBookOptimized book;
    
    book.addOrder(Order(1, Side::Buy, 100, 50, 1));
    assert(book.bestBid() == 100);
    
    bool cancelled = book.cancelOrder(1);
    assert(cancelled);
    assert(!book.bestBid().has_value());
    
    bool cancelled_again = book.cancelOrder(1);
    assert(!cancelled_again);
    
    std::cout << "runCancellationTests passed\n";
}
