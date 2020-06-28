#pragma once

#include <algorithm>
#include "shared_pointer.h"

struct shared_vector {
    shared_vector(): state(0) {}
    shared_vector(shared_vector const& other): state(other.state) {
        if (state == 2) {
            data = other.data;
            data->ref_counter++;
        } else {
            value = other.value;
        }
    }

    shared_vector& operator=(shared_vector const& other) {
        state = other.state;
        if (state == 2) {
            data = other.data;
            data->ref_counter++;
        } else {
            value = other.value;
        }
        return *this;
    }

    ~shared_vector() {
        if (state == 2) {
            data->ref_counter--;
            if (data->ref_counter == 0) {
                delete(data);
            }
        }
    }

    size_t size() const {
        if (state == 2) {
            return data->v.size();
        } else {
            return state;
        }
    }

    bool empty() const {
        return state == 0;
    }

    void push_back(uint32_t x) {
        if (state == 2) {
            unshare();
            data->v.push_back(x);
        } else if (state == 1) {
            uint32_t _val = value;
            turn_state(2);
            data->v.push_back(x);
        } else {
            turn_state(1);
            value = x;
        }
    }

    void pop_back() {
        if (state == 2) {
            unshare();
            data->v.pop_back();
            if (data->v.size() == 1) {
                turn_state(1);
            }
        } else {
            turn_state(0);
        }
    }

    uint32_t const& operator[](size_t i) const {
        return state == 2 ? data->v[i] : value;
    }

    uint32_t& operator[](size_t i) {
        unshare();
        return state == 2 ? data->v[i] : value;
    }

    uint32_t const& back() const {
        return state == 2 ? data->v.back() : value;
    }

    void resize(size_t n) {
        unshare();
        if (n == 0) {
            turn_state(0);
        } else if (n == 1) {
            turn_state(1);
        } else {
            while (n > size()) {
                push_back(0);
            }
            while (n < size()) {
                pop_back();
            }
        }
    }

    void reverse() {
        unshare();
        if (state == 2) {
            std::reverse(data->v.begin(), data->v.end());
        }
    }



private:
    void unshare() {
        if (state == 2 && data->ref_counter > 1) {
            data->ref_counter--;
            data = new shared_pointer(data->v);
        }
    }

    void turn_state(char st) {
        if (state == 2 && st != 2) {
            unshare();
            int x = data->v[0];
            delete(data);
            value = x;
        }
        if (state != 2 && st == 2) {
            int _val = value;
            data = new shared_pointer();
            if (state == 1) {
                data->v.push_back(_val);
            }
        }
        state = st;
    }

    char state;
    union {
        uint32_t value;
        shared_pointer* data;
    };
};

