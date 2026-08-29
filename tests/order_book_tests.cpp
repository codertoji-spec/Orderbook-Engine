#include <cassert>
#include <iostream>
#include "../include/order_book_optimized.hpp"

void runOrderBookTests() {
    OrderBookOptimized book;
    
    // Add Buy
    book.addOrder(Order(1, Side::Buy, 100, 50, 1));
    assert(book.bestBid() == 100);
    assert(book.bidQuantityAt(100) == 50);
    
    // Add Sell
    book.addOrder(Order(2, Side::Sell, 105, 50, 2));
    assert(book.bestAsk() == 105);
    
    assert(book.validateInvariants());
    std::cout << "runOrderBookTests passed\n";
}
