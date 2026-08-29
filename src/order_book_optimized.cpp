#include "../include/order_book_optimized.hpp"
#include <algorithm>
#include <chrono>

std::vector<Trade> OrderBookOptimized::addOrder(Order order) {
    std::vector<Trade> trades = match(order);
    if (order.remaining_quantity > 0) {
        if (order.side == Side::Buy) {
            auto& level = bids_[order.price];
            level.price = order.price;
            level.orders.push_back(order);
            level.total_quantity += order.remaining_quantity;
            auto it = level.orders.end();
            --it;
            order_map_[order.id] = {order.side, order.price, it};
        } else {
            auto& level = asks_[order.price];
            level.price = order.price;
            level.orders.push_back(order);
            level.total_quantity += order.remaining_quantity;
            auto it = level.orders.end();
            --it;
            order_map_[order.id] = {order.side, order.price, it};
        }
    }
    return trades;
}

std::vector<Trade> OrderBookOptimized::match(Order& order) {
    std::vector<Trade> trades;
    auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(
                   std::chrono::steady_clock::now().time_since_epoch())
                   .count();

    if (order.side == Side::Buy) {
        while (order.remaining_quantity > 0 && !asks_.empty()) {
            auto best_ask_it = asks_.begin();
            if (order.price < best_ask_it->first) break;

            auto& level = best_ask_it->second;
            auto& orders_at_price = level.orders;
            
            while (order.remaining_quantity > 0 && !orders_at_price.empty()) {
                auto& resting_order = orders_at_price.front();
                Quantity fill_qty = std::min(order.remaining_quantity, resting_order.remaining_quantity);

                trades.push_back(Trade{
                    order.id, resting_order.id, best_ask_it->first, fill_qty, static_cast<Timestamp>(now)
                });

                order.remaining_quantity -= fill_qty;
                resting_order.remaining_quantity -= fill_qty;
                level.total_quantity -= fill_qty;

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
            if (order.price > best_bid_it->first) break;

            auto& level = best_bid_it->second;
            auto& orders_at_price = level.orders;
            
            while (order.remaining_quantity > 0 && !orders_at_price.empty()) {
                auto& resting_order = orders_at_price.front();
                Quantity fill_qty = std::min(order.remaining_quantity, resting_order.remaining_quantity);

                trades.push_back(Trade{
                    resting_order.id, order.id, best_bid_it->first, fill_qty, static_cast<Timestamp>(now)
                });

                order.remaining_quantity -= fill_qty;
                resting_order.remaining_quantity -= fill_qty;
                level.total_quantity -= fill_qty;

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

bool OrderBookOptimized::cancelOrder(OrderId id) {
    auto it = order_map_.find(id);
    if (it == order_map_.end()) return false;

    OrderLoc loc = it->second;
    order_map_.erase(it);

    if (loc.side == Side::Buy) {
        auto level_it = bids_.find(loc.price);
        if (level_it != bids_.end()) {
            level_it->second.total_quantity -= loc.it->remaining_quantity;
            level_it->second.orders.erase(loc.it);
            if (level_it->second.orders.empty()) {
                bids_.erase(level_it);
            }
            return true;
        }
    } else {
        auto level_it = asks_.find(loc.price);
        if (level_it != asks_.end()) {
            level_it->second.total_quantity -= loc.it->remaining_quantity;
            level_it->second.orders.erase(loc.it);
            if (level_it->second.orders.empty()) {
                asks_.erase(level_it);
            }
            return true;
        }
    }
    return false;
}

std::optional<Price> OrderBookOptimized::bestBid() const {
    if (bids_.empty()) return std::nullopt;
    return bids_.begin()->first;
}

std::optional<Price> OrderBookOptimized::bestAsk() const {
    if (asks_.empty()) return std::nullopt;
    return asks_.begin()->first;
}

Quantity OrderBookOptimized::bidQuantityAt(Price price) const {
    auto it = bids_.find(price);
    if (it == bids_.end()) return 0;
    return it->second.total_quantity;
}

Quantity OrderBookOptimized::askQuantityAt(Price price) const {
    auto it = asks_.find(price);
    if (it == asks_.end()) return 0;
    return it->second.total_quantity;
}

void OrderBookOptimized::printSnapshot() const {
    std::cout << "========== ORDER BOOK ==========\n\n";
    std::cout << "ASKS\nPrice\t\tQuantity\n";
    for (auto it = asks_.rbegin(); it != asks_.rend(); ++it) {
        std::cout << it->first << "\t\t" << it->second.total_quantity << "\n";
    }

    std::cout << "\n-------------------------------\n\n";
    std::cout << "BIDS\nPrice\t\tQuantity\n";
    for (auto it = bids_.begin(); it != bids_.end(); ++it) {
        std::cout << it->first << "\t\t" << it->second.total_quantity << "\n";
    }
    std::cout << "\n===============================\n";
    
    auto bbid = bestBid();
    auto bask = bestAsk();
    if (bbid) std::cout << "Best Bid: " << *bbid << "\n";
    if (bask) std::cout << "Best Ask: " << *bask << "\n";
    if (bbid && bask) std::cout << "Spread:   " << (*bask - *bbid) << "\n";
}

bool OrderBookOptimized::validateInvariants() const {
    auto bbid = bestBid();
    auto bask = bestAsk();
    if (bbid && bask) {
        if (*bbid >= *bask) return false;
    }
    
    size_t order_count = 0;
    for (const auto& [p, level] : bids_) {
        if (level.orders.empty()) return false;
        order_count += level.orders.size();
        Quantity q = 0;
        for (auto& o : level.orders) q += o.remaining_quantity;
        if (q != level.total_quantity) return false;
    }
    for (const auto& [p, level] : asks_) {
        if (level.orders.empty()) return false;
        order_count += level.orders.size();
        Quantity q = 0;
        for (auto& o : level.orders) q += o.remaining_quantity;
        if (q != level.total_quantity) return false;
    }
    if (order_map_.size() != order_count) return false;

    return true;
}
