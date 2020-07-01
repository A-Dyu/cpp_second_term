#pragma once

#include <cstdint>
#include <vector>

struct shared_vector {
private:
    size_t ref_counter;
    std::vector<uint32_t> v;
public:
    shared_vector(): ref_counter(1), v() {}

    shared_vector(std::vector<uint32_t> const& other): ref_counter(1), v(other) {}

    shared_vector(const uint32_t* begin, const uint32_t* end): ref_counter(1), v(begin, end) {}

    static shared_vector* share(shared_vector* const p) {
        p->ref_counter++;
        return p;
    }

    static shared_vector* unshare(shared_vector* a) {
        if (a->ref_counter > 1) {
            a->ref_counter--;
            a = new shared_vector(a->v);
        }
        return a;
    }

    static void destroy(shared_vector* a) {
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

    void resize(size_t n) {
        v.resize(n);
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


