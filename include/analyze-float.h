#ifndef ANALYZE_FLOAT_H
#define ANALYZE_FLOAT_H
#undef _GLIBCXX_DEBUG
#include <cstdint>
#include <string>
#include <boost/detail/endian.hpp>
#include <boost/mpl/if.hpp>
#include <boost/dynamic_bitset.hpp>

typedef boost::dynamic_bitset<std::uint32_t> bitset;

enum float_type { unknown, normal, zero, denormal, indefinite, infinity, quiet_nan, signaling_nan };

std::ostream& operator<<(std::ostream&, float_type);

struct BigEndianExtended
{
    std::uint16_t exponent;
    std::uint64_t mantissa;
};
struct LittleEndianExtended
{
    std::uint64_t mantissa;
    std::uint16_t exponent;
};

typedef boost::mpl::if_c<BOOST_BYTE_ORDER == 1234, LittleEndianExtended, BigEndianExtended>::type Extended;
static_assert(sizeof(Extended) == sizeof(long double),
              "long double isn't a match for Extended");

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
        return rec.exponent & 0x8000;
    }
    static uint16_t get_exponent(record_type const& rec) {
        return rec.exponent & 0x7fff;
    }
    static uint64_t get_mantissa(record_type const& rec) {
        return rec.mantissa;
    }
    static uint16_t const max_exponent = 0x7fff;
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
    typedef std::uint64_t record_type;
    static bool is_negative(record_type const& rec) {
        return rec & 0x8000000000000000ull;
    }
    static uint16_t get_exponent(record_type const& rec) {
        return (rec & 0x7ff0000000000000ull) >> 52;
    }
    static uint64_t get_mantissa(record_type const& rec) {
        return rec & 0x000fffffffffffffull;
    }
    static uint16_t const max_exponent = 0x7ff;
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
    typedef std::uint32_t record_type;
    static bool is_negative(record_type const& rec) {
        return rec & 0x80000000;
    }
    static uint16_t get_exponent(record_type const& rec) {
        return (rec & 0x7f800000) >> 23;
    }
    static uint64_t get_mantissa(record_type const& rec) {
        return rec & 0x007fffff;
    }
    static uint16_t const max_exponent = 0xff;
    static uint64_t const quiet_mask = 0x00400000ull;
    static uint64_t const mantissa_mask = -1;
    constexpr static char const* const article = "a";
    constexpr static char const* const name = "Single";
};

template <typename Float>
union FloatRec
{
    Float val;
    typename float_traits<Float>::record_type rec;
    FloatRec(Float const value): val(value) { }
};
static_assert(sizeof(FloatRec<long double>) == sizeof(long double),
              "FloatRec has wrong size for long double");
static_assert(sizeof(FloatRec<double>) == sizeof(double),
              "FloatRec has wrong size for double");
static_assert(sizeof(FloatRec<float>) == sizeof(float),
              "FloatRec has wrong size for float");

template <typename Float>
struct FloatInfo
{
private:
    FloatRec<Float> helper;
public:
    bool negative;
    std::uint16_t exponent;
    std::uint64_t mantissa;
    float_type number_type;

    FloatInfo(Float const value):
        helper(value),
        negative(float_traits<Float>::is_negative(helper.rec)),
        exponent(float_traits<Float>::get_exponent(helper.rec)),
        mantissa(float_traits<Float>::get_mantissa(helper.rec))
    {
        if (exponent == float_traits<Float>::max_exponent)
            if (mantissa == 0)
                number_type = infinity;
            else {
                mantissa &= float_traits<Float>::mantissa_mask;
                if ((float_traits<Float>::get_mantissa(helper.rec) & float_traits<Float>::quiet_mask) == 0)
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
