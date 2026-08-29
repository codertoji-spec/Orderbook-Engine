#include <iostream>
#include "include/order_book_optimized.hpp"

int main() {
    OrderBookOptimized book;

    std::cout << "--- STEP 1: PEOPLE PLACE ORDERS ---
";
    std::cout << "Alice wants to SELL 10 shares at $105
";
    book.addOrder(Order(1, Side::Sell, 105, 10, 1));
    
    std::cout << "Bob wants to BUY 50 shares at $100

";
    book.addOrder(Order(2, Side::Buy, 100, 50, 2));

    std::cout << "Current State of the Exchange:
";
    book.printSnapshot();

    std::cout << "
--- STEP 2: A NEW BUYER ARRIVES ---
";
    std::cout << "Charlie is willing to BUY 5 shares at $105!
";
    std::cout << "(This should instantly match with Alice's sell order)

";
    
    auto trades = book.addOrder(Order(3, Side::Buy, 105, 5, 3));

    std::cout << "--- STEP 3: TRADES GENERATED! ---
";
    for (const auto& trade : trades) {
        std::cout << "TRADE EXECUTED: Buyer #" << trade.buyOrderId 
                  << " bought " << trade.quantity << " shares from Seller #" 
                  << trade.sellOrderId << " at $ " << trade.price << "
";
    }

    std::cout << "
--- STEP 4: FINAL STATE OF EXCHANGE ---
";
    book.printSnapshot();

    return 0;
}
