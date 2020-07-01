#pragma once

#include <cstdint>
#include <vector>

struct shared_pointer {
private:
    size_t ref_counter;
    std::vector<uint32_t> v;
public:
    shared_pointer(): ref_counter(1), v() {}

    shared_pointer(std::vector<uint32_t> const& other): ref_counter(1), v(other) {}

    friend void share(shared_pointer* &a, shared_pointer* const b) {
        a = b;
        a->ref_counter++;
    }

    friend void unshare(shared_pointer* &a) {
        if (a->ref_counter > 1) {
            a->ref_counter--;
            a = new shared_pointer(a->v);
        }
    }

    friend void destroy(shared_pointer* a) {
        a->ref_counter--;
        if (a->ref_counter == 0) {
            delete a;
        }
    }

    size_t size() {
        return v.size();
    }

    bool empty() {
        return v.empty();
    }

    void push_back(uint32_t x) {
        v.push_back(x);
    }

    void pop_back() {
        v.pop_back();
    }

    void reverse() {
        std::reverse(v.begin(), v.end());
    }

    uint32_t const& operator[](size_t i) const {
        return v[i];
    }

    uint32_t& operator[](size_t i) {
        return v[i];
    }

    uint32_t const& back() const {
        return v.back();
    }

    uint32_t& back() {
        return v.back();
    }
};


