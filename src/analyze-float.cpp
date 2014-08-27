#undef _GLIBCXX_DEBUG
#include "config.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <limits>
#include <algorithm>
#include <functional>
#include <numeric>
#include <tuple>
#include <boost/dynamic_bitset.hpp>
#include "analyze-float.h"

constexpr char const* const float_traits_base<long double>::article;
constexpr char const* const float_traits_base<long double>::name;
constexpr char const* const float_traits_base<double>::article;
constexpr char const* const float_traits_base<double>::name;
constexpr char const* const float_traits_base<float>::article;
constexpr char const* const float_traits_base<float>::name;

std::ostream& operator<<(std::ostream& os, float_type const type)
{
    switch (type)
    {
        case unknown:
            return os << "unknown";
        case normal:
            return os << "normal";
        case zero:
            return os << "zero";
        case denormal:
            return os << "denormal";
        case indefinite:
            return os << "indefinite";
        case infinity:
            return os << "infinity";
        case quiet_nan:
            return os << "quiet_nan";
        case signaling_nan:
            return os << "signaling_nan";
        default:
            assert(false);
    }
}

namespace {

bitset::size_type
find_last(bitset const& bitset)
{
    bitset::size_type result, pos;
    for (result = bitset.find_first();
         (pos = bitset.find_next(result)) != bitset::npos;
         result = pos)
        ;
    return result;
}

size_t
find_last(std::uint64_t value)
{
    if (!value)
        return -1;
    size_t result = 0;
    while (value >>= 1)
        ++result;
    return result;
}

} // namespace

namespace std {

template<>
struct greater_equal<::bitset>: public std::binary_function<::bitset, ::bitset, bool>
{
    greater_equal() { }
    bool operator()(::bitset const& A, ::bitset B) const {
        ::bitset::size_type const b_size = find_last(B);
        if (b_size == ::bitset::npos)
            return true;
        ::bitset::size_type const a_size = find_last(A);
        if (a_size == ::bitset::npos)
            return false;
        if (a_size > b_size)
            return true;
        if (b_size > a_size)
            return false;
        B.resize(A.size());
        return A >= B;
    }
};

} // namespace std

namespace {

std::tuple<bool /*sum*/, bool /*carry*/>
one_bit_add(bool const a, bool const b, bool carry)
{
    return std::make_tuple(
        a ^ b ^ carry,
        (a + b + carry) >= 2);
}

bitset
Multiply(bitset A, bitset const& B)
{
    bitset Product;
    bitset::size_type const last_b = find_last(B);
    if (last_b != bitset::npos)
        for (size_t i = 0; i <= last_b; ++i, A.push_back(0), A <<= 1) {
            if (!B[i])
                continue;
            if (Product.empty())
                Product = A;
            else {
                bitset::size_type const a_size = A.size();
                Product.resize(a_size);
                bool carry = false;
                for (size_t j = 0; j < a_size; ++j)
                    std::make_tuple(Product[j], std::ref(carry)) = one_bit_add(A[j], Product[j], carry);
                if (carry)
                    Product.push_back(1);
            }
        }
    return Product;
}

std::tuple<bool /*diff*/, bool /*borrow*/>
one_bit_subtract(bool const a, bool const b, bool borrow)
{
    return std::make_tuple(
        (a + b + borrow) & 1,
        (borrow && b) || (borrow && !a) || (b && !a));
}

bitset
Subtract(bitset A, bitset B)
{
    assert(std::greater_equal<bitset>()(A, B));
    size_t const nbits = find_last(A) + 1;
    B.resize(nbits);
    bitset Diff(nbits);
    bool borrow = false;
    for (size_t i = 0; i < nbits; ++i)
        std::make_tuple(Diff[i], std::ref(borrow)) = one_bit_subtract(A[i], B[i], borrow);
    assert(!borrow);
    Diff.resize(find_last(Diff) + 1);
    return Diff;
}

std::tuple<bitset /*Quotient*/, unsigned /*Remainder*/>
DivideAndRemainder(bitset Dividend, bitset Divisor)
{
    assert(Divisor.any()); // can't divide by zero
    size_t const nbits = std::max(Dividend.size(), Divisor.size());
    bitset Quotient(nbits);
    Dividend.resize(nbits);
    Divisor.resize(nbits);
    std::greater_equal<bitset> const ge;
    if (ge(Dividend, Divisor)) {
        size_t offset = find_last(Dividend) - find_last(Divisor);
        Divisor <<= offset;
        do {
            Quotient[offset] = ge(Dividend, Divisor);
            if (Quotient[offset])
                Dividend = Subtract(Dividend, Divisor);
            Divisor >>= 1;
        } while (offset-- != 0);
    }
    Quotient.resize(find_last(Quotient) + 1);
    return std::make_tuple(Quotient, Dividend.to_ulong());
}

bitset
build_bitset(std::uint64_t value, unsigned nBits)
{
    bitset result;
    while (nBits--) {
        result.push_back(value & 1);
        value >>= 1;
    }
    return result;
}

bitset
build_bitset(std::uint64_t value)
{
    return build_bitset(value, find_last(value) + 1);
}

// Repeatedly divide by 10 and use remainders to create decimal string
std::string
build_result(int DecExp, bitset Man, bool negative, char decimal_point, char thousands_sep)
{
    bitset const ten = build_bitset(10);
    char const DecDigits[11] = "0123456789";
    std::string result;
    do {
        if (!result.empty()) {
            if (DecExp == 0)
                result += decimal_point;
            else if (thousands_sep == ' ' && (DecExp % 5) == 0)
                result += ' ';
            else if (thousands_sep != '\0' && thousands_sep != ' ' && (DecExp % 3) == 0)
                result += thousands_sep;
        }
        unsigned Remainder;
        std::tie(Man, Remainder) = DivideAndRemainder(Man, ten);
        ++DecExp;
        assert(Remainder < 10);
        assert(Remainder >= 0);
        result += DecDigits[Remainder];
    } while (DecExp <= 0 || Man.any());
    result += negative ? " -" : " +";
    return std::string(result.rbegin(), result.rend());
}

/**
 * Reduce mantissa to minimum number of bits
 * That is, while mantissa is even, divide by 2 and increment binary
 * exponent. Stop if the exponent isn't negative. */
std::tuple<bitset, int /*BinExp*/>
minimize_mantissa(bitset const& Man, int BinExp)
{
    bitset::size_type const idx = Man.find_first();
    assert(idx != bitset::npos);
    // If assertion fails, result should be +/- 0.0
    int const adjustment = std::min<int>(idx, -BinExp);
    if (adjustment <= 0)
        return std::make_tuple(Man, BinExp);
    return std::make_tuple(Man >> adjustment, BinExp + adjustment);
}

/**
 * Repeatedly multiply by 10 until there is no more fraction. Decrement
 * the DecExp at the same time. Note that a multiply by 10 is the same
 * as multiply by 5 and increment of the BinExp exponent. Also note
 * that a multiply by 5 adds two or three bits to the number of
 * mantissa bits. */
std::tuple<bitset, int /*BinExp*/, int/*DecExp*/>
remove_fraction(bitset Man, int const BinExp)
{
    if (BinExp >= 0)
        return std::make_tuple(Man, BinExp, 0);

    std::vector<bitset> const multipliers(-BinExp, build_bitset(5));
    return std::make_tuple(
        std::accumulate(multipliers.begin(), multipliers.end(), Man, Multiply),
        0,
        BinExp);
}

// Finish reducing BinExp to 0 by shifting mantissa up
bitset
reduce_binary_exponent(bitset Man, int BinExp)
{
    assert(BinExp >= 0);
    Man.resize(Man.size() + BinExp);
    return Man << BinExp;
}

} // namespace

// Value = Mantissa * 2^BinExp * 10^DecExp
std::string
FloatingBinPointToDecStr(std::uint64_t Value, int BinExp, bool negative, char decimal_point /*= '.'*/, char thousands_sep /*= ' '*/)
{
    bitset Man = build_bitset(Value);

    std::tie(Man, BinExp) = minimize_mantissa(Man, BinExp);
    assert(Man.any());

    int DecExp;
    std::tie(Man, BinExp, DecExp) = remove_fraction(Man, BinExp);
    assert(DecExp <= 0);

    Man = reduce_binary_exponent(Man, BinExp);

    return build_result(DecExp, Man, negative, decimal_point, thousands_sep);
}
