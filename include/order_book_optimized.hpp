#pragma once
#include "order_book_base.hpp"
#include <unordered_map>
#include <map>
#include <vector>
#include <list>
#include <iostream>

class OrderBookOptimized : public OrderBookBase {
public:
    std::vector<Trade> addOrder(Order order) override;
    bool cancelOrder(OrderId id) override;

    std::optional<Price> bestBid() const override;
    std::optional<Price> bestAsk() const override;
    Quantity bidQuantityAt(Price price) const override;
    Quantity askQuantityAt(Price price) const override;

    void printSnapshot() const override;
    bool validateInvariants() const override;
    
    std::string name() const override { return "OrderBookOptimized"; }

private:
    struct PriceLevel {
        Price price;
        Quantity total_quantity = 0;
        std::list<Order> orders;
    };

    // Use vector for fast price level lookup (conceptually), but for limit order books, 
    // keeping a sorted structure is needed. Here we use an indexed vector mapping from price offset if we assumed a dense price space.
    // However, since prices can be sparse, we use std::map under the hood or an array if max_price is bounded.
    // For this optimized version, we'll use an array of PriceLevel pointers for O(1) lookup if max_price is small enough,
    // OR we'll use a fast binary tree (std::map) but with an intrusive list for O(1) cancellations.
    // Let's stick to std::map<Price, PriceLevel> but with std::list and iterators in unordered_map for O(1) cancel.
    
    std::map<Price, PriceLevel, std::greater<Price>> bids_; // descending
    std::map<Price, PriceLevel, std::less<Price>> asks_;    // ascending
    
    struct OrderLoc {
        Side side;
        Price price;
        std::list<Order>::iterator it;
    };
    std::unordered_map<OrderId, OrderLoc> order_map_;

    std::vector<Trade> match(Order& order);
};
