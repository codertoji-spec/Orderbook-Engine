#pragma once
#include "types.hpp"
#include <list>
#include "order.hpp"

struct OrderLocation {
    Side side;
    Price price;
    // We'll use this iterator to quickly remove from std::list or std::deque if we use a list-based implementation.
    // For now we just define the struct.
    bool valid = false;
};
