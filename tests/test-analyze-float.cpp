#include "config.h"
#include <cstdlib>
#include <cmath>
#include <tuple>
#include <limits>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/utility/binary.hpp>
#include "analyze-float.h"
#include "src/analyze-float.cpp"

using ::testing::Eq;
using ::testing::StrEq;
using ::testing::_;
using ::testing::ResultOf;
using namespace boost::multiprecision::literals;

int main(int argc, char* argv[])
{
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}

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

struct Expectations
{
    ::testing::Matcher<bool> negative;
    ::testing::Matcher<mp::cpp_int> exponent;
    ::testing::Matcher<mp::cpp_int> mantissa;
    ::testing::Matcher<float_type> type;
};

std::ostream& operator<<(std::ostream& os, Expectations const& ex)
{
    os << "neg ";
    ex.negative.DescribeTo(&os);
    os << "; exp ";
    ex.exponent.DescribeTo(&os);
    os << "; man ";
    ex.mantissa.DescribeTo(&os);
    os << "; type ";
    ex.mantissa.DescribeTo(&os);
    return os;
}

#ifdef BOOST_FLOAT80_C
typedef std::map<boost::float80_t, Expectations> long_double_expectations_t;
long_double_expectations_t const long_double_expectations {
    {0.0l, {false, Eq(0), Eq(0), zero}},
    {1.0l, {false, Eq(16383), Eq(0x8000000000000000ull), normal}},
    {-1.0l, {true, Eq(16383), Eq(0x8000000000000000ull), normal}},
    // TODO {std::strtold("inf", NULL), {false, _, _, infinity}},
};

class LongDoubleClassificationTest: public ::testing::TestWithParam<long_double_expectations_t::value_type>
{
};

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
typedef std::map<boost::float64_t, Expectations> double_expectations_t;
double_expectations_t const double_expectations {
    {0, {false, Eq(0), Eq(0), zero}},
    {1.0, {false, Eq(1023), Eq(0), normal}},
    {-2, {true, Eq(1024), Eq(0), normal}},
    {std::strtod("inf", NULL), {false, _, _, infinity}},
};

class DoubleClassificationTest: public ::testing::TestWithParam<double_expectations_t::value_type>
{
};

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
    mp::cpp_int exponent;
    mp::cpp_int mantissa;
    float_type number_type;
    char const* expectation;
};

std::ostream& operator<<(std::ostream& os, SerializationParam const& sp)
{
    return os << "neg: " << std::boolalpha << sp.negative << "; exp: " << std::hex << std::setfill('0') << std::setw(4) << sp.exponent << "; man: " << std::setw(16) << sp.mantissa << "; type: " << sp.number_type << "; expecting \"" << sp.expectation << "\"";
}

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
    FloatInfo<boost::float80_t> const value(GetParam().negative, GetParam().exponent, GetParam().mantissa, GetParam().number_type);
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
