#pragma once

#include <cstdint>
#include <vector>

struct shared_pointer {
    shared_pointer(): ref_counter(1), v() {}
    shared_pointer(std::vector<uint32_t> const& other): ref_counter(1), v(other) {}
public:
    size_t ref_counter;
    std::vector<uint32_t> v;
};


