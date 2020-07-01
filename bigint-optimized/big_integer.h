#pragma once
#include <utility>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include "shared_so_vector.h"

struct big_integer {
private:
    shared_so_vector digits;
    bool sign;
public:
    big_integer();
    big_integer(const big_integer&) = default;
    big_integer(uint32_t);
    big_integer(int);
    explicit big_integer(std::string const&);
    big_integer& operator=(big_integer const&) = default;

    friend std::string to_string(big_integer const&);

    big_integer operator~() const;
    big_integer operator-() const;
    big_integer operator+() const;
    big_integer& operator++();
    big_integer operator++(int) ;
    big_integer& operator--();
    big_integer operator--(int);

    friend big_integer operator*(big_integer const&, big_integer const&);
    friend big_integer operator/(big_integer const&, big_integer const&);

    friend big_integer operator>>(big_integer const&, int);
    friend big_integer operator<<(big_integer, int);

    friend big_integer operator|(big_integer, big_integer const&);
    friend big_integer operator&(big_integer, big_integer const&);
    friend big_integer operator^(big_integer, big_integer const&);

    big_integer& operator+=(big_integer const&);
    big_integer& operator-=(big_integer const&);
    big_integer& operator*=(big_integer const&);
    big_integer& operator/=(big_integer const&);
    big_integer& operator%=(big_integer const&);
    big_integer& operator|=(big_integer const&);
    big_integer& operator&=(big_integer const&);
    big_integer& operator^=(big_integer const&);
    big_integer& operator >>=(int);
    big_integer& operator <<=(int);

    friend bool operator==(big_integer const&, big_integer const&);
    friend bool operator<(big_integer const&, big_integer const&);
    friend bool operator>(big_integer const&, big_integer const&);
    friend bool operator<=(big_integer const&, big_integer const&);
    friend bool operator>=(big_integer const&, big_integer const&);

private:
    void convert(size_t);
    void format();
    big_integer abs() const;
    bool less(big_integer const&, size_t) const;
    void diff(big_integer const&, size_t);
    big_integer div_short(uint32_t) const;
    void bit_op(big_integer const&, const std::function<uint32_t(uint32_t, uint32_t)>&);
    void append_substr(std::string const&);
    void tilde();
    void negate();
    uint32_t get(size_t) const;
    void add(big_integer const&, bool);
};

big_integer operator+(big_integer, big_integer const&);
big_integer operator-(big_integer, big_integer const&);
big_integer operator%(big_integer const&, big_integer const&);
bool operator!=(big_integer const&, big_integer const&);