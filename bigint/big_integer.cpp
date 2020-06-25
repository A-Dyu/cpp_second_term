#include "big_integer.h"

typedef unsigned __int128 uint128_t;

big_integer big_integer::abs() const {
    return sign ? -(*this) : *this;
}

// Converting dig up to size sz by adding useless digits
void big_integer::convert(size_t sz) {
    while (dig.size() < sz) {
        dig.push_back(sign ? UINT32_MAX : 0);
    }
}

// Delete useless digits
void big_integer::format() {
    while (!dig.empty() && dig.back() == (sign ? UINT32_MAX : 0)) {
        dig.pop_back();
    }
}

// Compare bigint with shifted suffix of other bigint
bool big_integer::less(big_integer const &b, size_t ind) const {
    for (size_t i = 1; i <= dig.size(); i++) {
        if (dig[dig.size() - i] != (ind - i < b.dig.size() ? b.dig[ind - i] : 0)) {
            return dig[dig.size() - i] > (ind - i < b.dig.size() ? b.dig[ind - i] : 0);
        }
    }
    return true;
}

// Subtracts a shifted suffix of other bigint from bigint
void big_integer::dif(big_integer const &b, size_t ind) {
    size_t start = dig.size() - ind;
    bool c = false;
    for (size_t i = 0; i < ind; ++i) {
        uint32_t x = dig[start + i];
        uint32_t y = (i < b.dig.size() ? b.dig[i] : 0);
        auto res = static_cast<uint64_t>(x - y - c);
        c = (y + c > x);
        dig[start + i] = static_cast<uint32_t>(res & UINT32_MAX);
    }
}

// Result of dividing a bigint by uint32
big_integer big_integer::div_short(uint32_t b) const {
    big_integer res;
    uint64_t c = 0;
    uint64_t x = 0;
    for (size_t i = 0; i < dig.size(); i++) {
        x = (c << 32) | dig[dig.size() - 1 - i];
        res.dig.push_back(static_cast<uint32_t>((x / b) & UINT32_MAX));
        c = x % b;
    }
    reverse(res.dig.begin(), res.dig.end());
    res.format();
    return res;
}

void big_integer::bit_op(big_integer b, const std::function<uint32_t(uint32_t, uint32_t)>& op) {
    convert(std::max(dig.size(), b.dig.size()));
    b.convert(std::max(dig.size(), b.dig.size()));
    for (size_t i = 0; i < dig.size(); i++) {
        dig[i] = op(dig[i], b.dig[i]);
    }
    sign = op(sign, b.sign);
    format();
}

namespace {
    uint32_t _or(uint32_t a, uint32_t b) {
        return a | b;
    }

    uint32_t _and(uint32_t a, uint32_t b) {
        return a & b;
    }

    uint32_t _xor(uint32_t a, uint32_t b) {
        return a ^ b;
    }
}

big_integer::big_integer(): sign(false) {}

big_integer::big_integer(uint32_t x) : sign(false) {
    if (x) {
        dig.push_back(x);
    }
}

big_integer::big_integer(int x): big_integer(static_cast<uint32_t>(std::abs(static_cast<long long>(x)))) {
    if (x < 0) {
        *this = -*this;
    }
}

// Building bigint by adding substrings
void big_integer::append_substr(std::string const& s) {
    uint32_t k = 1;
    for (size_t i = 0; i < s.size(); i++) {
        k *= 10;
    }
    *this *= k;
    *this += std::stoi(s);
}

big_integer::big_integer(std::string s) : big_integer() {
    for (size_t i = s[0] == '-'; i < s.size(); i += 8) {
        append_substr(s.substr(i, 8));
    }
    if (s[0] == '-') {
        (*this) = -(*this);
    }
}

std::string to_string(big_integer x) {
    bool f = x.sign;
    x = x.abs();
    std::string res;
    while (x > 0) {
        big_integer r = x % 10;
        uint32_t c = (r == 0 ? 0 : r.dig[0]);
        res.push_back('0' + c);
        x /= 10;
    }
    if (f) {
        res.push_back('-');
    }
    std::reverse(res.begin(), res.end());
    return res.empty() ? "0" : res;
}

big_integer big_integer::operator~() const{
    big_integer res = *this;
    for (unsigned int & d : res.dig) {
        d ^= UINT32_MAX;
    }
    res.sign ^= true;
    return res;
}

big_integer big_integer::operator-() const{
    big_integer res = *this;
    if (res.dig.empty()) {
        if (res.sign) {
            res.sign = false;
            res.dig.push_back(1);
        }
        return res;
    }
    res = ~res + 1;
    res.format();
    return res;
}

big_integer big_integer::operator+() const{
    return *this;
}

big_integer& big_integer::operator++() {
    return (*this) += 1;
}

big_integer& big_integer::operator--() {
    return (*this) -= 1;
}

big_integer const big_integer::operator++(int) {
    big_integer res = *this;
    *this += 1;
    return res;
}

big_integer const big_integer::operator--(int) {
    big_integer res = *this;
    *this -= 1;
    return res;
}


big_integer operator+(big_integer a, big_integer const& b) {
    return a += b;
}

big_integer operator-(big_integer const& a, big_integer const& b) {
    return a + (-b);
}

namespace {
    // Get digit of bigint after using abs without creating new bigint
    uint32_t abs_dig(bool sign, uint32_t x, bool &c) {
        if (sign) {
            x ^= UINT32_MAX;
            x += c;
            if (x != 0) {
                c = false;
            }
        }
        return x;
    }
}

big_integer operator*(big_integer const& a, big_integer const& b) {
    big_integer res;
    res.convert(a.dig.size() + b.dig.size());
    bool ca = a.sign;
    for (size_t i = 0; i < a.dig.size() + ca; i++) {
        uint32_t xa = (i < a.dig.size() ? abs_dig(a.sign, a.dig[i], ca) : ca);
        uint32_t c = 0;
        bool cb = b.sign;
        for (size_t j = 0; j < b.dig.size() + cb; j++) {
            uint64_t mul = static_cast<uint64_t>(xa) *
                            static_cast<uint64_t>(j < b.dig.size() ? abs_dig(b.sign, b.dig[j], cb) : cb) +
                             static_cast<uint64_t>(res.dig[i + j]) + static_cast<uint64_t>(c);
            res.dig[i + j] = mul;
            c = mul >> 32;
        }
        res.dig[i + b.dig.size()] += c;
    }
    res.format();
    if (a.sign ^ b.sign) {
        res = -res;
    }
    return res;
}

big_integer operator/(big_integer const& _a, big_integer const& _b) {
    big_integer a = _a.abs();
    big_integer b = _b.abs();
    big_integer res;
    big_integer dq;
    if (a < b) {
        return 0;
    }
    if (b.dig.size() == 1) {
        res = a.div_short(b.dig[0]);
        if (_a.sign ^ _b.sign) {
            res = -res;
        }
        return res;
    }
    a.dig.push_back(0);
    size_t m = b.dig.size() + 1;
    size_t n = a.dig.size();
    res.dig.resize(n - m + 1);
    uint32_t qt = 0;
    for (size_t i = m, j = res.dig.size() - 1; i <= n; i++, j--) {
        uint128_t x = (static_cast<uint128_t>(a.dig[a.dig.size() - 1]) << 64) |
                        (static_cast<uint128_t>(a.dig[a.dig.size() - 2]) << 32) |
                         (static_cast<uint128_t>(a.dig[a.dig.size() - 3]));
        uint128_t y = ((static_cast<uint128_t>(b.dig[b.dig.size() - 1]) << 32) | static_cast<uint128_t>(b.dig[b.dig.size() - 2]));
        qt = static_cast<uint32_t>(x / y);
        dq = b * qt;
        if (!a.less(dq.abs(), m)) {
            qt--;
            dq -= b;
        }
        res.dig[j] = qt;
        a.dif(dq.abs(), m);
        if (!a.dig.back()) {
            a.dig.pop_back();
        }
    }
    res.format();
    if (_a.sign ^ _b.sign) {
        res = -res;
    }
    return res;
}

big_integer operator%(big_integer const& a, big_integer const& b) {
    return a - (a / b) * b;
}

big_integer operator>>(big_integer const& _a, int b) {
    big_integer a = _a;
    int k = b / 32;
    b %= 32;
    for (size_t i = k; i < a.dig.size(); i++) {
        a.dig[i - k] = a.dig[i];
    }
    a.dig.resize(a.dig.size() - k);
    a.format();
    big_integer _b = (static_cast<uint32_t>(1) << b);
    a /= _b;
    if (a < 0 && a * _b != _a) {
        a--;
    }
    return a;
}

big_integer operator<<(big_integer a, int b) {
    int k = b / 32;
    b %= 32;
    a.convert(a.dig.size() + k);
    for (int i = (int)a.dig.size() - 1; i >= k; i--) {
        a.dig[i] = a.dig[i - k];
    }
    for (size_t i = 0; i < (size_t) k; i++) {
        a.dig[i] = 0;
    }
    a.format();
    a *= (static_cast<uint32_t>(1) << b);
    return a;
}

big_integer operator|(big_integer a, big_integer const& b) {
    return a |= b;
}

big_integer operator&(big_integer a, big_integer const& b) {
    return a &= b;
}

big_integer operator^(big_integer a, big_integer const& b) {
    return a ^= b;
}

big_integer& big_integer::operator+=(big_integer b) {
    big_integer& a = *this;
    a.convert(std::max(a.dig.size(), b.dig.size()));
    b.convert(std::max(a.dig.size(), b.dig.size()));
    bool c = false;
    for (size_t i = 0; i < a.dig.size(); i++) {
        a.dig[i] += b.dig[i] + c;
        c = a.dig[i] < b.dig[i] + c || (b.dig[i] == UINT32_MAX && c);
    }
    if (a.sign && b.sign) {
        if (!c) {
            a.dig.push_back(UINT32_MAX - 1);
        }
    } else if (c) {
        if (a.sign || b.sign) {
            a.sign = false;
        } else {
            a.dig.push_back(1);
        }
    } else {
        a.sign |= b.sign;
    }
    a.format();
    return a;
}

big_integer& big_integer::operator-=(big_integer const& x) {
    return *this += -x;
}

big_integer& big_integer::operator*=(big_integer const& x) {
    return *this = *this * x;
}

big_integer& big_integer::operator/=(big_integer const& x) {
    return *this = *this / x;
}

big_integer& big_integer::operator%=(big_integer const& x) {
    return *this = *this % x;
}

big_integer& big_integer::operator|=(big_integer const& x) {
    bit_op(x, _or);
    return *this;
}

big_integer& big_integer::operator&=(big_integer const& x) {
    bit_op(x, _and);
    return *this;
}

big_integer& big_integer::operator^=(big_integer const& x) {
    bit_op(x, _xor);
    return *this;
}

big_integer& big_integer::operator>>=(int x) {
    return *this = *this >> x;
}

big_integer& big_integer::operator<<=(int x) {
    return *this = *this << x;
}

bool operator==(big_integer const& a, big_integer const& b) {
    if (a.dig.size() != b.dig.size() || a.sign != b.sign) {
        return false;
    }
    for (size_t i = 0; i < a.dig.size(); i++) {
        if (a.dig[i] != b.dig[i]) {
            return false;
        }
    }
    return true;
}

bool operator!=(big_integer const& a, big_integer const& b) {
    return !(a == b);
}

bool operator<(big_integer const& a, big_integer const& b) {
    if (a.sign != b.sign) {
        return a.sign;
    } else if (a.dig.size() != b.dig.size()) {
        return (a.dig.size() < b.dig.size()) ^ a.sign;
    }
    for (int i = (int) a.dig.size() - 1; i >= 0; i--) {
        if (a.dig[i] != b.dig[i]) {
            return a.dig[i] < b.dig[i];
        }
    }
    return false;
}

bool operator>(big_integer const& a, big_integer const& b) {
    if (a.sign != b.sign) {
        return !a.sign;
    } else if (a.dig.size() != b.dig.size()) {
        return (a.dig.size() > b.dig.size()) ^ a.sign;
    }
    for (int i = (int) a.dig.size() - 1; i >= 0; i--) {
        if (a.dig[i] != b.dig[i]) {
            return a.dig[i] > b.dig[i];
        }
    }
    return false;
}

bool operator<=(big_integer const& a, big_integer const& b) {
    return !(a > b);
}

bool operator>=(big_integer const& a, big_integer const& b) {
    return !(a < b);
}





