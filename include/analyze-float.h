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
};

template <>
struct float_traits<long double>
{
    static unsigned const mantissa_bits = 64;
    static unsigned const exponent_bits = 15;
    static signed const exponent_bias = 16383;
    static bool const implied_one = false;
    typedef Extended record_type;
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
    typedef std::uint64_t record_type;
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
    typedef std::uint32_t record_type;
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

    FloatInfo(Float const value);
};

std::string
FloatingBinPointToDecStr(std::uint64_t Value, int ValBinExp, bool negative, char decimal_point = '.', char thousands_sep = ' ');
#endif
