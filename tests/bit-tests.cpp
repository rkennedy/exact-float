#include "config.h"
#include <ostream>
#include <ios>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/cstdfloat.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include "include/analyze-float.h"

namespace mp = boost::multiprecision;
using namespace boost::multiprecision::literals;

template <typename Float>
struct ConversionCase
{
    Float input;
    mp::cpp_int expectation;
};

template <typename Float>
std::ostream& operator<<(std::ostream& os, ConversionCase<Float> const& cc)
{
    return os << cc.input << " -> " << std::hex << cc.expectation;
}

template <typename Float>
class FloatToBits: public ::testing::TestWithParam<ConversionCase<Float>>
{
};

#ifdef BOOST_FLOAT80_C
typedef FloatToBits<boost::float80_t> FloatToBits80;

ConversionCase<boost::float80_t> const extended_bit_conversions[] {
    {0.0l, 0x0_cppui},
    {-0.0l, 0x80000000000000000000_cppui},
    {1.0l, 0x3fff8000000000000000_cppui},
    {-1.0l, 0xbfff8000000000000000_cppui},
};

TEST_P(FloatToBits80, convert_extended)
{
    EXPECT_THAT(to_float_rec(GetParam().input), GetParam().expectation);
}

INSTANTIATE_TEST_CASE_P(FTB80,
                        FloatToBits80,
                        ::testing::ValuesIn(extended_bit_conversions));
#endif

#ifdef BOOST_FLOAT64_C
typedef FloatToBits<boost::float64_t> FloatToBits64;

ConversionCase<boost::float64_t> const double_bit_conversions[] {
    {0.0, 0x0_cppui},
    {-0.0, 0x8000000000000000_cppui},
    {1.0, 0x3ff0000000000000_cppui},
    {-1.0, 0xbff0000000000000_cppui},
};

TEST_P(FloatToBits64, convert_double)
{
    EXPECT_THAT(to_float_rec(GetParam().input), GetParam().expectation);
}

INSTANTIATE_TEST_CASE_P(FTB64,
                        FloatToBits64,
                        ::testing::ValuesIn(double_bit_conversions));
#endif

#ifdef BOOST_FLOAT32_C
typedef FloatToBits<boost::float32_t> FloatToBits32;

ConversionCase<boost::float32_t> const single_bit_conversions[] {
    {0.0f, 0x0_cppui},
    {-0.0f, 0x80000000_cppui},
    {1.0f, 0x3f800000_cppui},
    {-1.0f, 0xbf800000_cppui},
};

TEST_P(FloatToBits32, convert_single)
{
    EXPECT_THAT(to_float_rec(GetParam().input), GetParam().expectation);
}

INSTANTIATE_TEST_CASE_P(FTB32,
                        FloatToBits32,
                        ::testing::ValuesIn(single_bit_conversions));
#endif
