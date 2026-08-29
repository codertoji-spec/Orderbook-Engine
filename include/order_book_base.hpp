#pragma once
#include "order.hpp"
#include "trade.hpp"
#include <vector>
#include <optional>
#include <string>

class OrderBookBase {
public:
    virtual ~OrderBookBase() = default;

    virtual std::vector<Trade> addOrder(Order order) = 0;
    virtual bool cancelOrder(OrderId id) = 0;

    virtual std::optional<Price> bestBid() const = 0;
    virtual std::optional<Price> bestAsk() const = 0;
    virtual Quantity bidQuantityAt(Price price) const = 0;
    virtual Quantity askQuantityAt(Price price) const = 0;

    virtual void printSnapshot() const = 0;
    virtual bool validateInvariants() const = 0;
    
    virtual std::string name() const = 0;
};
