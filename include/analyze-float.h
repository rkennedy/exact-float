#ifndef ANALYZE_FLOAT_H
#define ANALYZE_FLOAT_H
#undef _GLIBCXX_DEBUG
#include <cstdint>
#include <string>
#include <bitset>
#include <boost/dynamic_bitset.hpp>

typedef boost::dynamic_bitset<std::uint32_t> bitset;

enum float_type { unknown, normal, zero, denormal, indefinite, infinity, quiet_nan, signaling_nan };

std::ostream& operator<<(std::ostream&, float_type);

typedef std::bitset<80> Extended;
typedef std::bitset<64> Double;
typedef std::bitset<32> Single;

template <typename Float>
struct float_traits
{
    static unsigned const mantissa_bits;
    static unsigned const exponent_bits;
    static signed const exponent_bias;
    static bool const implied_one;
    // typedef <unspecified> record_type;
    // int is_negative(record_type const&);
    // int get_exponent(record_type const&);
    // int get_mantissa(record_type const&);
};

template <>
struct float_traits<long double>
{
    static unsigned const mantissa_bits = 64;
    static unsigned const exponent_bits = 15;
    static signed const exponent_bias = 16383;
    static bool const implied_one = false;
    static bool const has_indefinite = true;
    typedef Extended record_type;
    static bool is_negative(record_type const& rec) {
        return rec[rec.size() - 1];
    }
    static uint16_t get_exponent(record_type const& rec) {
        return ((rec >> mantissa_bits) & record_type(0x7fff)).to_ullong();
    }
    static uint64_t get_mantissa(record_type const& rec) {
        return (rec & record_type(0xffffffffffffffffull)).to_ullong();
    }
    static uint64_t const quiet_mask = 0x4000000000000000ull;
    static uint64_t const mantissa_mask = 0x3FFFFFFFFFFFFFFFull;
    constexpr static char const* const article = "an";
    constexpr static char const* const name = "Extended";
};

template <>
struct float_traits<double>
{
    static unsigned const mantissa_bits = 52;
    static unsigned const exponent_bits = 11;
    static signed const exponent_bias = 1023;
    static bool const implied_one = true;
    static bool const has_indefinite = false;
    typedef Double record_type;
    static bool is_negative(record_type const& rec) {
        return rec[rec.size() - 1];
    }
    static uint16_t get_exponent(record_type const& rec) {
        return ((rec >> mantissa_bits) & record_type(0x7ff)).to_ullong();
    }
    static uint64_t get_mantissa(record_type const& rec) {
        return (rec & record_type(0x000fffffffffffffull)).to_ullong();
    }
    static uint64_t const quiet_mask = 0x0008000000000000ull;
    static uint64_t const mantissa_mask = -1;
    constexpr static char const* const article = "a";
    constexpr static char const* const name = "Double";
};

template <>
struct float_traits<float>
{
    static unsigned const mantissa_bits = 23;
    static unsigned const exponent_bits = 8;
    static signed const exponent_bias = 127;
    static bool const implied_one = true;
    static bool const has_indefinite = false;
    typedef Single record_type;
    static bool is_negative(record_type const& rec) {
        return rec[rec.size() - 1];
    }
    static uint16_t get_exponent(record_type const& rec) {
        return ((rec >> mantissa_bits) & record_type(0xff)).to_ullong();
    }
    static uint64_t get_mantissa(record_type const& rec) {
        return (rec & record_type(0x007fffff)).to_ullong();
    }
    static uint64_t const quiet_mask = 0x00400000ull;
    static uint64_t const mantissa_mask = -1;
    constexpr static char const* const article = "a";
    constexpr static char const* const name = "Single";
};

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
        negative(float_traits<Float>::is_negative(rec)),
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
};

template <typename Float>
FloatInfo<Float> make_float_info(Float f)
{
    return FloatInfo<Float>(f);
}

std::string
FloatingBinPointToDecStr(std::uint64_t Value, int ValBinExp, bool negative, char decimal_point = '.', char thousands_sep = ' ');
#endif
