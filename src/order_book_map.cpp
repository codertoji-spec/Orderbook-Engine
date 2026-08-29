#include "../include/order_book_map.hpp"
#include <algorithm>
#include <chrono>

std::vector<Trade> OrderBookMap::addOrder(Order order) {
    std::vector<Trade> trades = match(order);
    if (order.remaining_quantity > 0) {
        if (order.side == Side::Buy) {
            bids_[order.price].push_back(order);
            order_map_[order.id] = {order.side, order.price};
        } else {
            asks_[order.price].push_back(order);
            order_map_[order.id] = {order.side, order.price};
        }
    }
    return trades;
}

std::vector<Trade> OrderBookMap::match(Order& order) {
    std::vector<Trade> trades;
    auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(
                   std::chrono::steady_clock::now().time_since_epoch())
                   .count();

    if (order.side == Side::Buy) {
        while (order.remaining_quantity > 0 && !asks_.empty()) {
            auto best_ask_it = asks_.begin();
            if (order.price < best_ask_it->first) break; // Buy price < best ask

            auto& orders_at_price = best_ask_it->second;
            while (order.remaining_quantity > 0 && !orders_at_price.empty()) {
                auto& resting_order = orders_at_price.front();
                Quantity fill_qty = std::min(order.remaining_quantity, resting_order.remaining_quantity);

                trades.push_back(Trade{
                    order.id, resting_order.id, best_ask_it->first, fill_qty, static_cast<Timestamp>(now)
                });

                order.remaining_quantity -= fill_qty;
                resting_order.remaining_quantity -= fill_qty;

                if (resting_order.remaining_quantity == 0) {
                    order_map_.erase(resting_order.id);
                    orders_at_price.pop_front();
                }
            }
            if (orders_at_price.empty()) {
                asks_.erase(best_ask_it);
            }
        }
    } else { // Sell
        while (order.remaining_quantity > 0 && !bids_.empty()) {
            auto best_bid_it = bids_.begin();
            if (order.price > best_bid_it->first) break; // Sell price > best bid

            auto& orders_at_price = best_bid_it->second;
            while (order.remaining_quantity > 0 && !orders_at_price.empty()) {
                auto& resting_order = orders_at_price.front();
                Quantity fill_qty = std::min(order.remaining_quantity, resting_order.remaining_quantity);

                trades.push_back(Trade{
                    resting_order.id, order.id, best_bid_it->first, fill_qty, static_cast<Timestamp>(now)
                });

                order.remaining_quantity -= fill_qty;
                resting_order.remaining_quantity -= fill_qty;

                if (resting_order.remaining_quantity == 0) {
                    order_map_.erase(resting_order.id);
                    orders_at_price.pop_front();
                }
            }
            if (orders_at_price.empty()) {
                bids_.erase(best_bid_it);
            }
        }
    }
    return trades;
}

bool OrderBookMap::cancelOrder(OrderId id) {
    auto it = order_map_.find(id);
    if (it == order_map_.end()) return false;

    OrderLoc loc = it->second;
    order_map_.erase(it);

    if (loc.side == Side::Buy) {
        auto level_it = bids_.find(loc.price);
        if (level_it != bids_.end()) {
            auto& dq = level_it->second;
            auto dq_it = std::find_if(dq.begin(), dq.end(), [id](const Order& o) { return o.id == id; });
            if (dq_it != dq.end()) {
                dq.erase(dq_it);
            }
            if (dq.empty()) {
                bids_.erase(level_it);
            }
            return true;
        }
    } else {
        auto level_it = asks_.find(loc.price);
        if (level_it != asks_.end()) {
            auto& dq = level_it->second;
            auto dq_it = std::find_if(dq.begin(), dq.end(), [id](const Order& o) { return o.id == id; });
            if (dq_it != dq.end()) {
                dq.erase(dq_it);
            }
            if (dq.empty()) {
                asks_.erase(level_it);
            }
            return true;
        }
    }
    return false;
}

std::optional<Price> OrderBookMap::bestBid() const {
    if (bids_.empty()) return std::nullopt;
    return bids_.begin()->first;
}

std::optional<Price> OrderBookMap::bestAsk() const {
    if (asks_.empty()) return std::nullopt;
    return asks_.begin()->first;
}

Quantity OrderBookMap::bidQuantityAt(Price price) const {
    auto it = bids_.find(price);
    if (it == bids_.end()) return 0;
    Quantity q = 0;
    for (const auto& o : it->second) q += o.remaining_quantity;
    return q;
}

Quantity OrderBookMap::askQuantityAt(Price price) const {
    auto it = asks_.find(price);
    if (it == asks_.end()) return 0;
    Quantity q = 0;
    for (const auto& o : it->second) q += o.remaining_quantity;
    return q;
}

void OrderBookMap::printSnapshot() const {
    std::cout << "========== ORDER BOOK ==========\n\n";
    std::cout << "ASKS\nPrice\t\tQuantity\n";
    
    for (auto it = asks_.rbegin(); it != asks_.rend(); ++it) {
        Quantity q = 0;
        for (const auto& o : it->second) q += o.remaining_quantity;
        std::cout << it->first << "\t\t" << q << "\n";
    }

    std::cout << "\n-------------------------------\n\n";
    std::cout << "BIDS\nPrice\t\tQuantity\n";
    for (auto it = bids_.begin(); it != bids_.end(); ++it) {
        Quantity q = 0;
        for (const auto& o : it->second) q += o.remaining_quantity;
        std::cout << it->first << "\t\t" << q << "\n";
    }
    std::cout << "\n===============================\n";
    
    auto bbid = bestBid();
    auto bask = bestAsk();
    if (bbid) std::cout << "Best Bid: " << *bbid << "\n";
    if (bask) std::cout << "Best Ask: " << *bask << "\n";
    if (bbid && bask) std::cout << "Spread:   " << (*bask - *bbid) << "\n";
}

bool OrderBookMap::validateInvariants() const {
    auto bbid = bestBid();
    auto bask = bestAsk();
    if (bbid && bask) {
        if (*bbid >= *bask) return false;
    }
    
    size_t order_count = 0;
    for (const auto& [p, dq] : bids_) {
        if (dq.empty()) return false;
        order_count += dq.size();
    }
    for (const auto& [p, dq] : asks_) {
        if (dq.empty()) return false;
        order_count += dq.size();
    }
    if (order_map_.size() != order_count) return false;

    return true;
}
