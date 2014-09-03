#include "config.h"
#include <ostream>
#include <ios>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/cstdfloat.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/variant.hpp>
#include "include/analyze-float.h"

namespace mp = boost::multiprecision;
using namespace boost::multiprecision::literals;

struct float_rec: public boost::static_visitor<mp::cpp_int>
{
    template <typename Float>
    mp::cpp_int operator()(Float const value) const {
        return to_float_rec(value);
    }
};

using anyfloat = boost::variant<boost::float80_t, boost::float64_t, boost::float32_t>;

struct ConversionCase
{
    anyfloat input;
    mp::cpp_int expectation;
};

std::ostream& operator<<(std::ostream& os, ConversionCase const& cc)
{
    return os << cc.input << " -> " << std::hex << cc.expectation;
}

class FloatToBits: public ::testing::TestWithParam<ConversionCase>
{
};

ConversionCase const bit_conversions[] {
#ifdef BOOST_FLOAT80_C
    {0.0l, 0x0_cppui},
    {-0.0l, 0x80000000000000000000_cppui},
    {1.0l, 0x3fff8000000000000000_cppui},
    {-1.0l, 0xbfff8000000000000000_cppui},
#endif
#ifdef BOOST_FLOAT64_C
    {0.0, 0x0_cppui},
    {-0.0, 0x8000000000000000_cppui},
    {1.0, 0x3ff0000000000000_cppui},
    {-1.0, 0xbff0000000000000_cppui},
#endif
#ifdef BOOST_FLOAT32_C
    {0.0f, 0x0_cppui},
    {-0.0f, 0x80000000_cppui},
    {1.0f, 0x3f800000_cppui},
    {-1.0f, 0xbf800000_cppui},
#endif
};

TEST_P(FloatToBits, test_convert)
{
    auto const rec = boost::apply_visitor(float_rec(), GetParam().input);
    EXPECT_THAT(rec, GetParam().expectation);
}

INSTANTIATE_TEST_CASE_P(FTB,
                        FloatToBits,
                        ::testing::ValuesIn(bit_conversions));
