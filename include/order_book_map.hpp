#pragma once
#include "order_book_base.hpp"
#include <map>
#include <deque>
#include <unordered_map>
#include <iostream>

class OrderBookMap : public OrderBookBase {
public:
    std::vector<Trade> addOrder(Order order) override;
    bool cancelOrder(OrderId id) override;

    std::optional<Price> bestBid() const override;
    std::optional<Price> bestAsk() const override;
    Quantity bidQuantityAt(Price price) const override;
    Quantity askQuantityAt(Price price) const override;

    void printSnapshot() const override;
    bool validateInvariants() const override;
    
    std::string name() const override { return "OrderBookMap"; }

private:
    std::map<Price, std::deque<Order>, std::greater<Price>> bids_; // descending
    std::map<Price, std::deque<Order>, std::less<Price>> asks_;    // ascending
    
    struct OrderLoc {
        Side side;
        Price price;
    };
    std::unordered_map<OrderId, OrderLoc> order_map_;

    std::vector<Trade> match(Order& order);
};
