#include "config.h"
#include <cstdlib>
#include <cmath>
#include <tuple>
#include <limits>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/lexical_cast.hpp>
#include <boost/utility/binary.hpp>
#include "analyze-float.h"
#include "src/analyze-float.cpp"

using ::testing::Eq;
using ::testing::StrEq;
using ::testing::_;
using ::testing::ResultOf;

int main(int argc, char* argv[])
{
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}

#ifdef BOOST_FLOAT80_C
TEST(ConvFloatRec, convert_extended)
{
    typedef float_traits<boost::float80_t>::record_type Float;
    EXPECT_THAT(to_float_rec(0.0l), Float());
    EXPECT_THAT(to_float_rec(-0.0l), Float("10000000000000000000000000000000000000000000000000000000000000000000000000000000"));
    EXPECT_THAT(to_float_rec(1.0l), Float("00111111111111111000000000000000000000000000000000000000000000000000000000000000"));
    EXPECT_THAT(to_float_rec(-1.0l), Float("10111111111111111000000000000000000000000000000000000000000000000000000000000000"));
}
#endif

TEST(ConvFloatRec, convert_double)
{
    typedef float_traits<boost::float64_t>::record_type Float;
    EXPECT_THAT(to_float_rec(0.0), Float());
    EXPECT_THAT(to_float_rec(-0.0), Float(0x8000000000000000ull));
    EXPECT_THAT(to_float_rec(1.0), Float(0x3ff0000000000000ull));
    EXPECT_THAT(to_float_rec(-1.0), Float(0xbff0000000000000ull));
}

TEST(ConvFloatRec, convert_single)
{
    typedef float_traits<boost::float32_t>::record_type Float;
    EXPECT_THAT(to_float_rec(0.0f), Float());
    EXPECT_THAT(to_float_rec(-0.0f), Float(0x80000000u));
    EXPECT_THAT(to_float_rec(1.0f), Float(0x3f800000u));
    EXPECT_THAT(to_float_rec(-1.0f), Float(0xbf800000u));
}

struct Expectations
{
    ::testing::Matcher<bool> negative;
    ::testing::Matcher<uint16_t> exponent;
    ::testing::Matcher<uint64_t> mantissa;
    ::testing::Matcher<float_type> type;
};

#ifdef BOOST_FLOAT80_C
typedef std::map<boost::float80_t, Expectations> long_double_expectations_t;
long_double_expectations_t const long_double_expectations {
    {0, {false, 0, 0, zero}},
    {1.0, {false, 16383, 0x8000000000000000ull, normal}},
    {-1, {true, 16383, 0x8000000000000000ull, normal}},
    // TODO {std::strtold("inf", NULL), {false, _, _, infinity}},
};

class LongDoubleClassificationTest: public ::testing::TestWithParam<long_double_expectations_t::value_type>
{
};
#endif

#ifdef BOOST_FLOAT64_C
typedef std::map<boost::float64_t, Expectations> double_expectations_t;
double_expectations_t const double_expectations {
    {0, {false, 0, 0, zero}},
    {1.0, {false, 1023, 0, normal}},
    {-2, {true, 1024, 0, normal}},
    {std::strtod("inf", NULL), {false, _, _, infinity}},
};

class DoubleClassificationTest: public ::testing::TestWithParam<double_expectations_t::value_type>
{
};
#endif

#ifdef BOOST_FLOAT32_C
typedef std::map<boost::float32_t, Expectations> single_expectations_t;
single_expectations_t const single_expectations {
    {0, {false, 0, 0, zero}},
    {1.0, {false, 127, 0, normal}},
    {-2, {true, 128, 0, normal}},
    {std::strtof("inf", NULL), {false, _, _, infinity}},
};

class SingleClassificationTest: public ::testing::TestWithParam<single_expectations_t::value_type>
{
};
#endif

#ifdef BOOST_FLOAT80_C
TEST_P(LongDoubleClassificationTest, identify_values)
{
    auto const info(exact(GetParam().first));
    EXPECT_THAT(info.negative, GetParam().second.negative);
    EXPECT_THAT(info.exponent, GetParam().second.exponent);
    EXPECT_THAT(info.mantissa, GetParam().second.mantissa);
    EXPECT_THAT(info.number_type, GetParam().second.type);
}

INSTANTIATE_TEST_CASE_P(LongDoubleClassifications,
                        LongDoubleClassificationTest,
                        ::testing::ValuesIn(long_double_expectations));
#endif

#ifdef BOOST_FLOAT64_C
TEST_P(DoubleClassificationTest, identify_values)
{
    auto const info(exact(GetParam().first));
    EXPECT_THAT(info.negative, GetParam().second.negative);
    EXPECT_THAT(info.exponent, GetParam().second.exponent);
    EXPECT_THAT(info.mantissa, GetParam().second.mantissa);
    EXPECT_THAT(info.number_type, GetParam().second.type);
}

INSTANTIATE_TEST_CASE_P(DoubleClassifications,
                        DoubleClassificationTest,
                        ::testing::ValuesIn(double_expectations));
#endif

#ifdef BOOST_FLOAT32_T
TEST_P(SingleClassificationTest, identify_values)
{
    auto const info(exact(GetParam().first));
    EXPECT_THAT(info.negative, GetParam().second.negative);
    EXPECT_THAT(info.exponent, GetParam().second.exponent);
    EXPECT_THAT(info.mantissa, GetParam().second.mantissa);
    EXPECT_THAT(info.number_type, GetParam().second.type);
}

INSTANTIATE_TEST_CASE_P(SingleClassifications,
                        SingleClassificationTest,
                        ::testing::ValuesIn(single_expectations));
#endif

TEST(BitsetOpsTest, test_minimize_mantissa)
{
    EXPECT_THAT(minimize_mantissa(BOOST_BINARY(011000), 3),
                Eq(std::make_tuple(BOOST_BINARY(011000), 3)));
    EXPECT_THAT(minimize_mantissa(BOOST_BINARY(0100), -3),
                Eq(std::make_tuple(BOOST_BINARY(0001), -1)));
    EXPECT_THAT(minimize_mantissa(BOOST_BINARY(110000), -3),
                Eq(std::make_tuple(BOOST_BINARY(000110), 0)));
}

TEST(BitsetOpsTest, test_remove_fraction)
{
    EXPECT_THAT(remove_fraction(2, -4),
                Eq(std::make_tuple((1*5*5*5*5) * 2, 0, -4)));
    EXPECT_THAT(remove_fraction(3, 0),
                Eq(std::make_tuple(3, 0, 0)));
    EXPECT_THAT(remove_fraction(4, 6),
                Eq(std::make_tuple(4, 6, 0)));
}

TEST(BitsetOpsTest, test_reduce_binary_exponent)
{
    EXPECT_THAT(reduce_binary_exponent(BOOST_BINARY(1100), 4),
                Eq(BOOST_BINARY(11000000)));
    EXPECT_THAT(reduce_binary_exponent(BOOST_BINARY(1100), 1),
                Eq(BOOST_BINARY(11000)));
    EXPECT_THAT(reduce_binary_exponent(BOOST_BINARY(110), 0),
                Eq(BOOST_BINARY(110)));
}

struct SerializationParam
{
    bool negative;
    std::uint16_t exponent;
    std::uint64_t mantissa;
    float_type number_type;
    char const* expectation;
};

class Serialization: public ::testing::TestWithParam<SerializationParam>
{
public:
    std::ostringstream os;
    static std::string str(std::ostream const& s) {
        return dynamic_cast<std::ostringstream const&>(s).str();
    }
};

#ifdef BOOST_FLOAT80_C
TEST_P(Serialization, test_extended)
{
    auto const value = FloatInfo<boost::float80_t>(GetParam().negative, GetParam().exponent, GetParam().mantissa, GetParam().number_type);
    EXPECT_THAT(os << value, ResultOf(str, StrEq(GetParam().expectation)));
}

SerializationParam const extended_serializations[] = {
    {false, std::numeric_limits<boost::float80_t>::max_exponent - 1, BOOST_BINARY_ULL(
            10000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000), normal,
    "+ 1"},
    {true, std::numeric_limits<boost::float80_t>::max_exponent - 1, BOOST_BINARY_ULL(
            11000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000), normal,
    "- 1.5"},
    {false, std::numeric_limits<boost::float80_t>::max_exponent + 5, BOOST_BINARY_ULL(
            10101110 10010001 11101011 10000101 00011110 10111000 01010001 11101100), normal,
    "+ 87.28500 00000 00000 00333 06690 73875 46962 12708 95004 27246 09375"},
};

INSTANTIATE_TEST_CASE_P(ExtendedSerializations, Serialization,
                        ::testing::ValuesIn(extended_serializations));
#endif
