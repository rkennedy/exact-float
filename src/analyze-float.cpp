#undef _GLIBCXX_DEBUG
#include "config.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <tuple>
#include <boost/multiprecision/cpp_int.hpp>
#include "analyze-float.h"

namespace mp = boost::multiprecision;

#ifdef BOOST_FLOAT80_C
constexpr char const* const float_traits_base<boost::float80_t>::article;
constexpr char const* const float_traits_base<boost::float80_t>::name;
#endif
#ifdef BOOST_FLOAT64_C
constexpr char const* const float_traits_base<boost::float64_t>::article;
constexpr char const* const float_traits_base<boost::float64_t>::name;
#endif
#ifdef BOOST_FLOAT32_C
constexpr char const* const float_traits_base<boost::float32_t>::article;
constexpr char const* const float_traits_base<boost::float32_t>::name;
#endif

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

// Repeatedly divide by 10 and use remainders to create decimal string
std::string
build_result(int DecExp, mp::cpp_int Man, bool negative, char decimal_point, char thousands_sep)
{
    mp::cpp_int const ten(10);
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
        mp::cpp_int Remainder;
        mp::divide_qr(Man, ten, Man, Remainder);
        ++DecExp;
        assert(Remainder < 10);
        assert(Remainder >= 0);
        result += DecDigits[int(Remainder)];
    } while (DecExp <= 0 || !Man.is_zero());
    result += negative ? " -" : " +";
    return std::string(result.rbegin(), result.rend());
}

/**
 * Reduce mantissa to minimum number of bits
 * That is, while mantissa is even, divide by 2 and increment binary
 * exponent. Stop if the exponent isn't negative. */
std::tuple<mp::cpp_int, int /*BinExp*/>
minimize_mantissa(mp::cpp_int const& Man, int const BinExp)
{
    assert(Man != 0);
    auto const idx = mp::lsb(Man);
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
std::tuple<mp::cpp_int, int /*BinExp*/, int/*DecExp*/>
remove_fraction(mp::cpp_int Man, int const BinExp)
{
    if (BinExp >= 0)
        return std::make_tuple(Man, BinExp, 0);

    return std::make_tuple(Man * mp::pow(mp::cpp_int(5), -BinExp), 0, BinExp);
}

// Finish reducing BinExp to 0 by shifting mantissa up
mp::cpp_int
reduce_binary_exponent(mp::cpp_int Man, int BinExp)
{
    assert(BinExp >= 0);
    return Man << BinExp;
}

} // namespace

// Value = Mantissa * 2^BinExp * 10^DecExp
std::string
FloatingBinPointToDecStr(mp::cpp_int Value, int BinExp, bool negative, char decimal_point /*= '.'*/, char thousands_sep /*= ' '*/)
{
    mp::cpp_int Man = Value;

    std::tie(Man, BinExp) = minimize_mantissa(Man, BinExp);
    assert(Man != 0);

    int DecExp;
    std::tie(Man, BinExp, DecExp) = remove_fraction(Man, BinExp);
    assert(DecExp <= 0);

    Man = reduce_binary_exponent(Man, BinExp);

    return build_result(DecExp, Man, negative, decimal_point, thousands_sep);
}
