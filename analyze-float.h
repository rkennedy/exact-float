#ifndef ANALYZE_FLOAT_H
#define ANALYZE_FLOAT_H
#undef _GLIBCXX_DEBUG
#include <boost/cstdint.hpp>
#include <string>
#include <boost/static_assert.hpp>
#include <boost/detail/endian.hpp>
#include <boost/mpl/if.hpp>
#include <boost/dynamic_bitset.hpp>

typedef boost::dynamic_bitset<uint32_t> bitset;

enum float_type { unknown, normal, zero, denormal, indefinite, infinity, quiet_nan, signaling_nan };

struct BigEndianExtended
{
	uint16_t exponent;
	uint64_t mantissa;
};
struct LittleEndianExtended
{
	uint64_t mantissa;
	uint16_t exponent;
};

typedef boost::mpl::if_c<BOOST_BYTE_ORDER == 1234, LittleEndianExtended, BigEndianExtended>::type Extended;
BOOST_STATIC_ASSERT(sizeof(Extended) == sizeof(long double));

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
};

template <>
struct float_traits<double>
{
	static unsigned const mantissa_bits = 52;
	static unsigned const exponent_bits = 11;
	static signed const exponent_bias = 1023;
	static bool const implied_one = true;
	typedef uint64_t record_type;
};

template <>
struct float_traits<float>
{
	static unsigned const mantissa_bits = 23;
	static unsigned const exponent_bits = 8;
	static signed const exponent_bias = 127;
	static bool const implied_one = true;
	typedef uint32_t record_type;
};

template <typename Float>
union FloatRec
{
	Float val;
	typename float_traits<Float>::record_type rec;
	FloatRec(Float const value): val(value) { }
};
BOOST_STATIC_ASSERT(sizeof(FloatRec<long double>) == sizeof(long double));
BOOST_STATIC_ASSERT(sizeof(FloatRec<double>) == sizeof(double));
BOOST_STATIC_ASSERT(sizeof(FloatRec<float>) == sizeof(float));

template <typename Float>
struct FloatInfo
{
private:
	FloatRec<Float> helper;
public:
	bool negative;
	uint16_t exponent;
	uint64_t mantissa;
	float_type number_type;

	FloatInfo(Float const value);
};

std::string
FloatingBinPointToDecStr(uint64_t Value, int ValBinExp, bool negative, char decimal_point = '.', char thousands_sep = ' ');
#endif
