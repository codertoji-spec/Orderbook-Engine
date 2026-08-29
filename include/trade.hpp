#pragma once
#include "types.hpp"

struct Trade {
    OrderId buyOrderId;
    OrderId sellOrderId;
    Price price;
    Quantity quantity;
    Timestamp timestamp;
};
