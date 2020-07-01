#pragma once

#include <algorithm>
#include "shared_vector.h"

struct shared_so_vector {
private:
    static constexpr size_t SMALL_SIZE = sizeof(shared_vector*) / sizeof(uint32_t);
    bool is_big;
    size_t _size;
    union {
        uint32_t small_value[SMALL_SIZE];
        shared_vector* data;
    };

    void swap(shared_so_vector& other) {
        if (is_big) {
            if (other.is_big) {
                std::swap(data, other.data);
            } else {
                shared_vector* _data = data;
                std::copy_n(other.small_value, other._size, small_value);
                other.data = _data;
            }
        } else {
            if (other.is_big) {
                shared_vector* _data = other.data;
                std::copy_n(small_value, _size, other.small_value);
                data = _data;
            } else {
                std::swap(small_value, other.small_value);
            }
        }
        std::swap(_size, other._size);
        std::swap(is_big, other.is_big);
    }

    void ensure_unique() {
        if (is_big) {
            data = shared_vector::unshare(data);
        }
    }

    void turn_big() {
        if (!is_big) {
            uint32_t _val[SMALL_SIZE];
            std::copy_n(small_value, size(), _val);
            data = new shared_vector(small_value, small_value + _size);
            is_big = true;
        }
    }

public:
    shared_so_vector(): is_big(false), _size(0) {}
    shared_so_vector(shared_so_vector const& other): is_big(other.is_big), _size(other._size) {
        if (is_big) {
            data = shared_vector::share(other.data);
        } else {
            std::copy_n(other.small_value, _size, small_value);
        }
    }

    shared_so_vector& operator=(shared_so_vector const& other) {
        if (&other != this) {
            shared_so_vector safe(other);
            swap(safe);
        }
        return *this;
    }

    ~shared_so_vector() {
        if (is_big) {
            shared_vector::destroy(data);
        }
    }

    size_t size() const {
        return is_big ? data->size() : _size;
    }

    bool empty() const {
        return size() == 0;
    }

    void push_back(uint32_t x) {
        if (size() == SMALL_SIZE) {
            turn_big();
        }
        if (is_big) {
            ensure_unique();
            data->push_back(x);
        } else {
            small_value[_size++] = x;
        }
    }

    void pop_back() {
        if (is_big) {
            ensure_unique();
            data->pop_back();
        } else {
            _size--;
        }
    }

    uint32_t const& operator[](size_t i) const {
        return is_big ? (*data)[i] : small_value[i];
    }

    uint32_t& operator[](size_t i) {
        ensure_unique();
        return is_big ? (*data)[i] : small_value[i];
    }

    uint32_t const& back() const {
        return is_big ? data->back() : small_value[_size - 1];
    }

    uint32_t& back() {
        ensure_unique();
        return is_big ? data->back() : small_value[_size - 1];
    }

    void resize(size_t n) {
        ensure_unique();
        if (n > SMALL_SIZE) {
            turn_big();
        }
        if (is_big) {
            data->resize(n);
        } else {
            while (n > size()) {
                small_value[_size++] = 0;
            }
            _size = n;
        }
    }

    void reverse() {
        ensure_unique();
        if (is_big) {
            data->reverse();
        } else {
            std::reverse(small_value, small_value + _size);
        }
    }
};
