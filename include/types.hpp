#pragma once
#include <cstdint>

enum class Side {
    Buy,
    Sell
};

enum class OrderType {
    Limit,
    Market
};

using OrderId = uint64_t;
using Price = int64_t;
using Quantity = uint64_t;
using Timestamp = uint64_t;
using SequenceNumber = uint64_t;
using ClientId = uint64_t;
