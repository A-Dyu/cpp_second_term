#include <cassert>
#include "big_integer.h"

typedef unsigned __int128 uint128_t;

big_integer big_integer::abs() const {
    return sign ? -(*this) : *this;
}

namespace {
    // Get digit of bigint after negating (if is_negated) without creating new bigint
    // (You need to go from digit 0 to digit.size() - 1 sequentially, c is a carry flag,
    // when it's first digit c should be equal to is_negated)
    uint32_t negate_digit(bool is_negated, uint32_t x, bool &c) {
        if (is_negated) {
            x ^= UINT32_MAX;
            x += c;
            if (x != 0) {
                c = false;
            }
        }
        return x;
    }

    // Returns useless digit for a given sign
    uint32_t udg(bool sign) {
        return sign ? UINT32_MAX : 0;
    }

    // Adds uint32 with carry flag
    void addc(uint32_t& a, uint32_t b, bool & c) {
        a += b + c;
        c = a < b + c || (b == UINT32_MAX && c);
    }
}

// Returns a digit if i is in range, or a useless digit if it's not.
uint32_t big_integer::get(size_t i) const {
    if (i < digits.size()) {
        return digits[i];
    } else {
        return udg(sign);
    }
}

void big_integer::add(big_integer const& b, bool is_negated) {
    if (!b.sign && b.digits.empty()) {
        return;
    }
    if (b.sign && b.digits.empty() && is_negated) {
        add(1, false);
        return;
    }
    convert(std::max(digits.size(), b.digits.size()));
    bool c = false;
    bool negate_c = is_negated;
    bool b_sign = b.sign ^ is_negated;
    for (size_t i = 0; i < digits.size(); i++) {
        uint32_t x = negate_digit(is_negated, b.get(i), negate_c);
        addc(digits[i], x, c);
    }
    if (negate_c) {
        digits.push_back(udg(sign));
        addc(digits.back(), negate_digit(is_negated, udg(b.sign), negate_c), c);
    }

    if (sign && b_sign) {
        if (!c) {
            digits.push_back(UINT32_MAX - 1);
        }
    } else if (c) {
        if (sign || b_sign) {
            sign = false;
        } else {
            digits.push_back(1);
        }
    } else {
        sign |= b_sign;
    }
    format();
}

// Converting digits up to size sz by adding useless digits
void big_integer::convert(size_t sz) {
    while (digits.size() < sz) {
        digits.push_back(sign ? UINT32_MAX : 0);
    }
}

// Delete useless digits
void big_integer::format() {
    while (!digits.empty() && digits.back() == (sign ? UINT32_MAX : 0)) {
        digits.pop_back();
    }
}

// Compare bigint with shifted prefix of other bigint (bigints are not negative)
bool big_integer::less(big_integer const &b, size_t ind) const {
    assert(!sign && !b.sign);
    for (size_t i = 1; i <= digits.size(); i++) {
        if (digits[digits.size() - i] != (ind - i < b.digits.size() ? b.digits[ind - i] : 0)) {
            return digits[digits.size() - i] > (ind - i < b.digits.size() ? b.digits[ind - i] : 0);
        }
    }
    return true;
}

namespace {
    uint32_t cast_64_down_to_32(uint64_t x) {
        return static_cast<uint32_t>(x & UINT32_MAX);
    }
}

// Subtracts a shifted prefix of other bigint from bigint (bigints are not negative)
void big_integer::diff(big_integer const & b, size_t ind) {
    assert(!sign && !b.sign);
    size_t start = digits.size() - ind;
    bool c = false;
    for (size_t i = 0; i < ind; ++i) {
        uint32_t x = digits[start + i];
        uint32_t y = (i < b.digits.size() ? b.digits[i] : 0);
        uint64_t res = (static_cast<uint64_t>(x) - y) - c;
        c = (static_cast<uint64_t>(y) + c > x);
        digits[start + i] = cast_64_down_to_32(res);
    }
}

// Result of dividing a bigint by uint32
big_integer big_integer::div_short(uint32_t b) const {
    big_integer res;
    uint64_t c = 0;
    uint64_t x = 0;
    for (size_t i = 0; i < digits.size(); i++) {
        x = (c << 32) | digits[digits.size() - 1 - i];
        res.digits.push_back(cast_64_down_to_32(x / b));
        c = x % b;
    }
    reverse(res.digits.begin(), res.digits.end());
    res.format();
    return res;
}

void big_integer::bit_op(big_integer const& b, const std::function<uint32_t(uint32_t, uint32_t)>& op) {
    convert(std::max(digits.size(), b.digits.size()));
    for (size_t i = 0; i < digits.size(); i++) {
        digits[i] = op(digits[i], b.get(i));
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
    if (x != 0) {
        digits.push_back(x);
    }
}

big_integer::big_integer(int x): big_integer(static_cast<uint32_t>(std::abs(static_cast<long long>(x)))) {
    if (x < 0) {
        negate();
    }
}

// Building bigint by adding substrings (len <= 8)
void big_integer::append_substr(std::string const& s) {
    assert(s.length() <= 8);
    uint32_t k = 1;
    for (size_t i = 0; i < s.size(); i++) {
        k *= 10;
    }
    *this *= k;
    *this += std::stoi(s);
}

big_integer::big_integer(std::string const& s) : big_integer() {
    for (size_t i = s[0] == '-'; i < s.size(); i += 8) {
        append_substr(s.substr(i, 8));
    }
    if (s[0] == '-') {
        negate();
    }
}

std::string to_string(big_integer const& x) {
    big_integer x_abs = x.abs();
    std::string res;
    while (x_abs > 0) {
        big_integer r = x_abs % 10;
        uint32_t c = (r == 0 ? 0 : r.digits[0]);
        res.push_back('0' + c);
        x_abs /= 10;
    }
    if (x.sign) {
        res.push_back('-');
    }
    std::reverse(res.begin(), res.end());
    return res.empty() ? "0" : res;
}

void big_integer::tilde() {
    for (size_t i = 0; i < digits.size(); i++) {
        digits[i] ^= UINT32_MAX;
    }
    sign ^= true;
}

void big_integer::negate() {
    if (digits.empty()) {
        if (sign) {
            sign = false;
            digits.push_back(1);
        }
    } else {
        tilde();
        *this += 1;
        format();
    }
}

big_integer big_integer::operator~() const{
    big_integer res = *this;
    res.tilde();
    return res;
}

big_integer big_integer::operator-() const{
    big_integer res = *this;
    res.negate();
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

big_integer big_integer::operator++(int) {
    big_integer res = *this;
    *this += 1;
    return res;
}

big_integer big_integer::operator--(int) {
    big_integer res = *this;
    *this -= 1;
    return res;
}


big_integer operator+(big_integer a, big_integer const& b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const& b) {
    return a -= b;
}

big_integer operator*(big_integer const& a, big_integer const& b) {
    big_integer res;
    res.convert(a.digits.size() + b.digits.size());
    bool ca = a.sign;
    for (size_t i = 0; i < a.digits.size() + ca; i++) {
        uint32_t xa = (i < a.digits.size() ? negate_digit(a.sign, a.digits[i], ca) : ca);
        uint32_t c = 0;
        bool cb = b.sign;
        for (size_t j = 0; j < b.digits.size() + cb; j++) {
            uint64_t mul = static_cast<uint64_t>(xa) * (j < b.digits.size() ? negate_digit(b.sign, b.digits[j], cb) : cb) +
                           static_cast<uint64_t>(res.digits[i + j]) + c;
            res.digits[i + j] = mul;
            c = mul >> 32;
        }
        res.digits[i + b.digits.size()] = c;
    }
    res.format();
    if (a.sign ^ b.sign) {
        res.negate();
    }
    return res;
}

namespace {
    uint128_t shiftr128(uint32_t x, int shift) {
        return static_cast<uint128_t>(x) << shift;
    }
}

big_integer operator/(big_integer const& a, big_integer const& b) {
    big_integer a_abs = a.abs();
    big_integer b_abs = b.abs();
    big_integer res;
    big_integer dq;
    if (a_abs < b_abs) {
        return 0;
    }
    if (b_abs.digits.size() == 1) {
        res = a_abs.div_short(b_abs.digits[0]);
        if (a.sign ^ b.sign) {
            res.negate();
        }
        return res;
    }
    a_abs.digits.push_back(0);
    size_t m = b_abs.digits.size() + 1;
    size_t n = a_abs.digits.size();
    res.digits.resize(n - m + 1);
    uint32_t qt = 0;
    for (size_t i = m, j = res.digits.size() - 1; i <= n; i++, j--) {
        uint128_t x = shiftr128(a_abs.digits[a_abs.digits.size() - 1], 64) |
                      shiftr128(a_abs.digits[a_abs.digits.size() - 2], 32) |
                      shiftr128(a_abs.digits[a_abs.digits.size() - 3], 0);
        uint128_t y = shiftr128(b_abs.digits[b_abs.digits.size() - 1], 32) | shiftr128(b_abs.digits[b_abs.digits.size() - 2], 0);
        qt = static_cast<uint32_t>(std::min(static_cast<uint128_t>(UINT32_MAX), x / y));
        dq = b_abs * qt;
        if (!a_abs.less(dq.abs(), m)) {
            qt--;
            dq -= b_abs;
        }
        res.digits[j] = qt;
        a_abs.diff(dq.abs(), m);
        if (a_abs.digits.back() == 0) {
            a_abs.digits.pop_back();
        }
    }
    res.format();
    if (a.sign ^ b.sign) {
        res.negate();
    }
    return res;
}

big_integer operator%(big_integer const& a, big_integer const& b) {
    return a - (a / b) * b;
}

big_integer operator>>(big_integer const& a, int b) {
    big_integer res = a;
    int k = b / 32;
    b %= 32;
    for (size_t i = k; i < res.digits.size(); i++) {
        res.digits[i - k] = res.digits[i];
    }
    res.digits.resize(res.digits.size() - k);
    res.format();
    big_integer shifted_b = (static_cast<uint32_t>(1) << b);
    res /= shifted_b;
    if (res < 0 && res * shifted_b != a) {
        res--;
    }
    return res;
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
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

big_integer& big_integer::operator+=(big_integer const& x) {
    add(x, false);
    return *this;
}

big_integer& big_integer::operator-=(big_integer const& x) {
    add(x, true);
    return *this;
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

big_integer& big_integer::operator>>=(int b) {
    return *this = *this >> b;
}

big_integer& big_integer::operator<<=(int b) {
    int k = b / 32;
    b %= 32;
    convert(digits.size() + k);
    for (size_t i = digits.size(); i > k; i--) {
        digits[i - 1] = digits[i - k - 1];
    }
    for (size_t i = 0; i < (size_t) k; i++) {
        digits[i] = 0;
    }
    format();
    *this *= (static_cast<uint32_t>(1) << b);
    return *this;
}

bool operator==(big_integer const& a, big_integer const& b) {
    if (a.digits.size() != b.digits.size() || a.sign != b.sign) {
        return false;
    }
    for (size_t i = 0; i < a.digits.size(); i++) {
        if (a.digits[i] != b.digits[i]) {
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
    } else if (a.digits.size() != b.digits.size()) {
        return (a.digits.size() < b.digits.size()) ^ a.sign;
    }
    for (size_t i = a.digits.size(); i > 0; i--) {
        if (a.digits[i - 1] != b.digits[i - 1]) {
            return a.digits[i - 1] < b.digits[i - 1];
        }
    }
    return false;
}

bool operator>(big_integer const& a, big_integer const& b) {
    if (a.sign != b.sign) {
        return !a.sign;
    } else if (a.digits.size() != b.digits.size()) {
        return (a.digits.size() > b.digits.size()) ^ a.sign;
    }
    for (size_t i = a.digits.size(); i > 0; i--) {
        if (a.digits[i - 1] != b.digits[i - 1]) {
            return a.digits[i - 1] > b.digits[i - 1];
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





