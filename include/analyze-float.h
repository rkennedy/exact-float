#ifndef ANALYZE_FLOAT_H
#define ANALYZE_FLOAT_H
#undef _GLIBCXX_DEBUG
#include <cstdint>
#include <limits>
#include <string>
#include <bitset>
#include <boost/format.hpp>
#include <boost/cstdfloat.hpp>
#include <boost/multiprecision/cpp_int.hpp>

namespace mp = boost::multiprecision;

enum float_type { unknown, normal, zero, denormal, indefinite, infinity, quiet_nan, signaling_nan };

std::ostream& operator<<(std::ostream&, float_type);

template <typename Float>
struct float_traits_base
{
    static bool const implied_one;
    static unsigned const bits;
    static bool const has_indefinite;
    static char const* const article;
    static char const* const name;
};

#ifdef BOOST_FLOAT80_C
template <>
struct float_traits_base<boost::float80_t>
{
    static bool const implied_one = false;
    static unsigned const bits = 80;
    static bool const has_indefinite = true;
    constexpr static char const* const article = "an";
    constexpr static char const* const name = "Extended";
};
#endif

#ifdef BOOST_FLOAT64_C
template <>
struct float_traits_base<boost::float64_t>
{
    static bool const implied_one = true;
    static unsigned const bits = 64;
    static bool const has_indefinite = false;
    constexpr static char const* const article = "a";
    constexpr static char const* const name = "Double";
};
#endif

#ifdef BOOST_FLOAT32_C
template <>
struct float_traits_base<boost::float32_t>
{
    static bool const implied_one = true;
    static unsigned const bits = 32;
    static bool const has_indefinite = false;
    constexpr static char const* const article = "a";
    constexpr static char const* const name = "Single";
};
#endif

template <typename Float>
struct float_traits: float_traits_base<Float>
{
    typedef float_traits_base<Float> Base;
    typedef Float type;

    enum { mantissa_bits = std::numeric_limits<Float>::digits - Base::implied_one };
    enum { exponent_bits = Base::bits - 1 - mantissa_bits };
    enum { exponent_bias = std::numeric_limits<Float>::max_exponent - 1 };

    static mp::cpp_int get_exponent(mp::cpp_int const& rec) {
        auto const exponent_mask = (mp::cpp_int(1) << unsigned(exponent_bits)) - 1;
        return (rec >> unsigned(mantissa_bits)) & exponent_mask;
    }
    static mp::cpp_int get_mantissa(mp::cpp_int const& rec) {
        auto const mantissa_mask = (mp::cpp_int(1) << unsigned(mantissa_bits)) - 1;
        return rec & mantissa_mask;
    }
};

template <typename Float>
mp::cpp_int
to_float_rec(Float const value)
{
    mp::cpp_int result;
    auto const v = reinterpret_cast<unsigned char const*>(&value);
    // TODO: Handle endian variation
    for (auto i = 0u; i < float_traits<Float>::bits; ++i)
        if (v[i / CHAR_BIT] & (1u << (i % CHAR_BIT)))
            mp::bit_set(result, i);
    return result;
}

template <typename Float>
float_type
get_float_type(mp::cpp_int exponent, mp::cpp_int mantissa)
{
    auto const exponent_mask = (mp::cpp_int(1) << unsigned(float_traits<Float>::exponent_bits)) - 1;
    if (exponent == exponent_mask) {
        if (mantissa.is_zero())
            return infinity;
        bool const top_bit = mp::bit_test(mantissa, float_traits<Float>::mantissa_bits - 1);
        if (float_traits<Float>::implied_one)
            return top_bit ? quiet_nan : signaling_nan;
        // From here down, we know it's a float80_t
        assert(float_traits<Float>::has_indefinite);
        if (!top_bit)
            return signaling_nan;
        if (mp::bit_test(mantissa, float_traits<Float>::mantissa_bits - 2))
            return mp::lsb(mantissa) < float_traits<Float>::mantissa_bits - 2 ? quiet_nan : indefinite;
        return mp::lsb(mantissa) < float_traits<Float>::mantissa_bits - 1 ? signaling_nan : infinity;
    } else if (exponent.is_zero())
        return mantissa.is_zero() ? zero : denormal;
    else
        return (!float_traits<Float>::implied_one && !mp::bit_test(mantissa, float_traits<Float>::mantissa_bits - 1)) ? denormal : normal;
}

template <typename Float>
struct FloatInfo
{
private:
    mp::cpp_int rec;
public:
    bool negative;
    mp::cpp_int exponent;
    mp::cpp_int mantissa;
    float_type number_type;

    FloatInfo(Float const value):
        rec(to_float_rec(value)),
        negative(value < 0),
        exponent(float_traits<Float>::get_exponent(rec)),
        mantissa(float_traits<Float>::get_mantissa(rec)),
        number_type(get_float_type<Float>(exponent, mantissa))
    { }

    FloatInfo(bool negative, mp::cpp_int exponent, mp::cpp_int mantissa, float_type number_type):
        rec(), negative(negative), exponent(exponent), mantissa(mantissa), number_type(number_type)
    { }

    bool operator==(FloatInfo<Float> const& other) const {
        return negative == other.negative
            && exponent == other.exponent
            && mantissa == other.mantissa
            && number_type == other.number_type;
    }
};

template <typename Float>
FloatInfo<Float> exact(Float f)
{
    return FloatInfo<Float>(f);
}

std::string
FloatingBinPointToDecStr(mp::cpp_int Value, int ValBinExp, bool negative, char decimal_point = '.', char thousands_sep = ' ');

template <typename Float>
std::ostream&
operator<<(std::ostream& os, FloatInfo<Float> const& info)
{
    switch (info.number_type) {
        case normal: {
            unsigned const mantissa_offset = float_traits<Float>::mantissa_bits - 1 + float_traits<Float>::implied_one;
            mp::cpp_int const full_mantissa = (mp::cpp_int(1) << mantissa_offset) | info.mantissa;
            mp::cpp_int const adjusted_exponent = info.exponent - float_traits<Float>::exponent_bias - mantissa_offset;
            return os << FloatingBinPointToDecStr(full_mantissa, adjusted_exponent.convert_to<int>(), info.negative, '.', ' ');
        }
        case zero:
            return os << (info.negative ? "- 0" : "+ 0");
        case denormal:
            // TODO!
            return os << FloatingBinPointToDecStr(info.mantissa, -float_traits<Float>::exponent_bias - (float_traits<Float>::mantissa_bits - 2), info.negative, '.', ' ');
        case indefinite:
            return os << "Indefinite";
        case infinity:
            return os << (info.negative ? "- Infinity" : "+ Infinity");
        case quiet_nan:
            return os << boost::format("QNaN(%d)") % info.mantissa;
        case signaling_nan:
            return os << boost::format("SNaN(%d)") % info.mantissa;
        default:
            return os << "unknown-number-type";
    }
}
#endif
