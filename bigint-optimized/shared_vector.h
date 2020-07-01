#pragma once

#include <algorithm>
#include "shared_pointer.h"

struct shared_vector {
private:
    static const size_t SMALL_SIZE = sizeof(shared_pointer*) / sizeof(uint32_t);
    bool is_big;
    size_t _size;
    union {
        uint32_t small_value[SMALL_SIZE];
        shared_pointer* data;
    };

    static void small_copy(uint32_t* a, const uint32_t* b, size_t copy_size) {
        for (size_t i = 0; i < copy_size; i++) {
            a[i] = b[i];
        }
    }

    void swap(shared_vector& other) {
        if (is_big) {
            if (other.is_big) {
                std::swap(data, other.data);
            } else {
                shared_pointer* _data = data;
                small_copy(small_value, other.small_value, other._size);
                other.data = _data;
            }
        } else {
            if (other.is_big) {
                shared_pointer* _data = other.data;
                small_copy(other.small_value, small_value, _size);
                data = _data;
            } else {
                std::swap(small_value, other.small_value);
            }
        }
        std::swap(_size, other._size);
        std::swap(is_big, other.is_big);
    }

    void free() {
        if (is_big) {
            unshare(data);
        }
    }

    void turn_big() {
        if (!is_big) {
            uint32_t _val[SMALL_SIZE];
            small_copy(_val, small_value, size());
            data = new shared_pointer();
            for (size_t i = 0; i < size(); i++) {
                data->push_back(_val[i]);
            }
            is_big = true;
        }
    }

public:
    shared_vector(): is_big(false), _size(0) {}
    shared_vector(shared_vector const& other): is_big(other.is_big), _size(other._size) {
        if (is_big) {
            share(data, other.data);
        } else {
            small_copy(small_value, other.small_value, _size);
        }
    }

    shared_vector& operator=(shared_vector const& other) {
        if (&other != this) {
            shared_vector safe(other);
            swap(safe);
        }
        return *this;
    }

    ~shared_vector() {
        if (is_big) {
            destroy(data);
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
            free();
            data->push_back(x);
        } else {
            small_value[_size++] = x;
        }
    }

    void pop_back() {
        if (is_big) {
            free();
            data->pop_back();
        } else {
            _size--;
        }
    }

    uint32_t const& operator[](size_t i) const {
        return is_big ? (*data)[i] : small_value[i];
    }

    uint32_t& operator[](size_t i) {
        free();
        return is_big ? (*data)[i] : small_value[i];
    }

    uint32_t const& back() const {
        return is_big ? data->back() : small_value[_size - 1];
    }

    uint32_t& back() {
        free();
        return is_big ? data->back() : small_value[_size - 1];
    }

    void resize(size_t n) {
        free();
        while (n > size()) {
            push_back(0);
        }
        while (n < size()) {
            pop_back();
        }
    }

    void reverse() {
        free();
        if (is_big) {
            data->reverse();
        } else {
            std::reverse(small_value, small_value + _size);
        }
    }
};