#include "config.h"
#include <ostream>
#include <ios>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/cstdfloat.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/operators.hpp>
#include <boost/type_erasure/free.hpp>
#include <boost/mpl/vector.hpp>
#include "include/analyze-float.h"

namespace mp = boost::multiprecision;
using namespace boost::multiprecision::literals;
namespace te = boost::type_erasure;

BOOST_TYPE_ERASURE_FREE((has_to_float_rec), to_float_rec, 1)

typedef te::any<
    boost::mpl::vector<
        has_to_float_rec<mp::cpp_int(te::_self)>,
        te::ostreamable<>,
        te::relaxed>,
   te::_self const&> anyfloat;

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
    EXPECT_THAT(to_float_rec(GetParam().input), GetParam().expectation);
}

INSTANTIATE_TEST_CASE_P(FTB,
                        FloatToBits,
                        ::testing::ValuesIn(bit_conversions));
