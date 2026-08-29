#pragma once
#include "types.hpp"

struct Order {
    OrderId id;
    Side side;
    Price price;
    Quantity quantity;
    Quantity remaining_quantity;
    Timestamp timestamp;
    SequenceNumber seq_num;
    ClientId client_id;

    Order() = default;
    Order(OrderId i, Side s, Price p, Quantity q, Timestamp t, SequenceNumber sq = 0, ClientId c = 0)
        : id(i), side(s), price(p), quantity(q), remaining_quantity(q), timestamp(t), seq_num(sq), client_id(c) {}
};
