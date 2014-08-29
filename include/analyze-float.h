#ifndef ANALYZE_FLOAT_H
#define ANALYZE_FLOAT_H
#undef _GLIBCXX_DEBUG
#include <cstdint>
#include <limits>
#include <string>
#include <bitset>
#include <boost/dynamic_bitset.hpp>
#include <boost/format.hpp>
#include <boost/cstdfloat.hpp>

typedef boost::dynamic_bitset<std::uint32_t> bitset;

enum float_type { unknown, normal, zero, denormal, indefinite, infinity, quiet_nan, signaling_nan };

std::ostream& operator<<(std::ostream&, float_type);

template <typename Float>
struct float_traits_base
{
    static bool const implied_one;
    static unsigned const bits;
    static bool const has_indefinite;
    static uint64_t const quiet_mask;
    static uint64_t const mantissa_mask;
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
    static uint64_t const quiet_mask = 0x4000000000000000ull;
    static uint64_t const mantissa_mask = 0x3FFFFFFFFFFFFFFFull;
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
    static uint64_t const quiet_mask = 0x0008000000000000ull;
    static uint64_t const mantissa_mask = -1;
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
    static uint64_t const quiet_mask = 0x00400000ull;
    static uint64_t const mantissa_mask = -1;
    constexpr static char const* const article = "a";
    constexpr static char const* const name = "Single";
};
#endif

template <typename Float>
struct float_traits: float_traits_base<Float>
{
    typedef float_traits_base<Float> Base;
    typedef Float type;

    static unsigned const mantissa_bits = std::numeric_limits<Float>::digits - Base::implied_one;
    static unsigned const exponent_bits = Base::bits - 1 - mantissa_bits;
    static unsigned const exponent_bias = std::numeric_limits<Float>::max_exponent - 1;
    typedef std::bitset<Base::bits> record_type;

    static uint16_t get_exponent(record_type const& rec) {
        auto const exponent_mask = std::bitset<exponent_bits>().flip().to_ullong();
        return ((rec >> mantissa_bits) & record_type(exponent_mask)).to_ullong();
    }
    static uint64_t get_mantissa(record_type const& rec) {
        auto const mantissa_mask = std::bitset<mantissa_bits>().flip().to_ullong();
        return (rec & record_type(mantissa_mask)).to_ullong();
    }
};

template <typename T>
bool
is_negative(T const& rec) {
    return rec[rec.size() - 1];
}

template <typename Float>
typename float_traits<Float>::record_type
to_float_rec(Float const value)
{
    typename float_traits<Float>::record_type result;
    auto const v = reinterpret_cast<unsigned char const*>(&value);
    // TODO: Handle endian variation
    for (auto i = 0u; i < result.size(); ++i)
        result[i] = v[i / CHAR_BIT] & (1u << (i % CHAR_BIT));
    return result;
}

template <typename Float>
struct FloatInfo
{
private:
    typename float_traits<Float>::record_type rec;
public:
    bool negative;
    std::uint16_t exponent;
    std::uint64_t mantissa;
    float_type number_type;

    FloatInfo(Float const value):
        rec(to_float_rec(value)),
        negative(is_negative(rec)),
        exponent(float_traits<Float>::get_exponent(rec)),
        mantissa(float_traits<Float>::get_mantissa(rec))
    {
        std::uint16_t const max_exponent = (1 << float_traits<Float>::exponent_bits) - 1;
        if (exponent == max_exponent)
            if (mantissa == 0)
                number_type = infinity;
            else {
                mantissa &= float_traits<Float>::mantissa_mask;
                if ((float_traits<Float>::get_mantissa(rec) & float_traits<Float>::quiet_mask) == 0)
                    number_type = signaling_nan;
                else if (mantissa == 0)
                    number_type = (float_traits<Float>::has_indefinite && mantissa == 0) ? indefinite : quiet_nan;
            }
        else if (exponent == 0)
            number_type = mantissa == 0 ? zero : denormal;
        else
            number_type = normal;
    }

    FloatInfo(bool negative, std::uint16_t exponent, std::uint64_t mantissa, float_type number_type):
        rec(), negative(negative), exponent(exponent), mantissa(mantissa), number_type(number_type)
    { }
};

template <typename Float>
FloatInfo<Float> exact(Float f)
{
    return FloatInfo<Float>(f);
}

std::string
FloatingBinPointToDecStr(std::uint64_t Value, int ValBinExp, bool negative, char decimal_point = '.', char thousands_sep = ' ');

template <typename Float>
std::ostream&
operator<<(std::ostream& os, FloatInfo<Float> const& info)
{
    switch (info.number_type) {
        case normal: {
            std::uint64_t const full_mantissa = (static_cast<std::uint64_t>(1) << (float_traits<Float>::mantissa_bits - 1 + float_traits<Float>::implied_one)) | info.mantissa;
            return os << FloatingBinPointToDecStr(full_mantissa, info.exponent - float_traits<Float>::exponent_bias - (float_traits<Float>::mantissa_bits - 1 + float_traits<Float>::implied_one), info.negative, '.', ' ');
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
