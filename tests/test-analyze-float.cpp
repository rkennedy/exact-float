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

#ifdef BOOST_FLOAT80_C
TEST(TestClassifyNumbers, test_recognize_extendeds)
{
    EXPECT_THAT(get_float_type<boost::float80_t>(0, 0), zero);
    EXPECT_THAT(get_float_type<boost::float80_t>(0, 0x7fffffffffffffff_cppui), denormal);
    EXPECT_THAT(get_float_type<boost::float80_t>(0, 0x4aaaaaaaaaaaaaaa_cppui), denormal);
    EXPECT_THAT(get_float_type<boost::float80_t>(0, 0x8000000000000000_cppui), denormal);
    EXPECT_THAT(get_float_type<boost::float80_t>(0, 0xffffffffffffffff_cppui), denormal);

    EXPECT_THAT(get_float_type<boost::float80_t>(0x7fff, 0), infinity) << "actually invalid nowadays";
    EXPECT_THAT(get_float_type<boost::float80_t>(0x7fff, 0x3fffffffffffffff_cppui), signaling_nan) << "actually invalid nowadays";
    EXPECT_THAT(get_float_type<boost::float80_t>(0x7fff, 0x4000000000000000_cppui), signaling_nan) << "actually invalid nowadays";

    EXPECT_THAT(get_float_type<boost::float80_t>(0x7fff, 0x8000000000000000_cppui), infinity) << "formerly signaling_nan";
    EXPECT_THAT(get_float_type<boost::float80_t>(0x7fff, 0xbfffffffffffffff_cppui), signaling_nan);
    EXPECT_THAT(get_float_type<boost::float80_t>(0x7fff, 0x8000000000000001_cppui), signaling_nan);

    EXPECT_THAT(get_float_type<boost::float80_t>(0x7fff, 0xc000000000000000_cppui), indefinite);
    EXPECT_THAT(get_float_type<boost::float80_t>(0x7fff, 0xffffffffffffffff_cppui), quiet_nan);
    EXPECT_THAT(get_float_type<boost::float80_t>(0x7fff, 0xc000000000000001_cppui), quiet_nan);

    EXPECT_THAT(get_float_type<boost::float80_t>(0x7777, 0x0000000000000000_cppui), denormal) << "invalid nowadays";
    EXPECT_THAT(get_float_type<boost::float80_t>(0x4444, 0x7fffffffffffffff_cppui), denormal) << "invalid nowadays";
    EXPECT_THAT(get_float_type<boost::float80_t>(0x3333, 0x8000000000000000_cppui), normal);
    EXPECT_THAT(get_float_type<boost::float80_t>(0x3333, 0xffffffffffffffff_cppui), normal);
}
#endif

#ifdef BOOST_FLOAT64_C
TEST(TestClassifyNumbers, test_recognize_doubles)
{
    EXPECT_THAT(get_float_type<boost::float64_t>(0, 0), zero);
    EXPECT_THAT(get_float_type<boost::float64_t>(0, 0xfffffffffffff_cppui), denormal);
    EXPECT_THAT(get_float_type<boost::float64_t>(0, 0x8000000000000_cppui), denormal);
    EXPECT_THAT(get_float_type<boost::float64_t>(0, 0x0000000000001_cppui), denormal);

    EXPECT_THAT(get_float_type<boost::float64_t>(0x7ff, 0), infinity);
    EXPECT_THAT(get_float_type<boost::float64_t>(0x7ff, 0x8000000000000_cppui), quiet_nan);
    EXPECT_THAT(get_float_type<boost::float64_t>(0x7ff, 0xfffffffffffff_cppui), quiet_nan);
    EXPECT_THAT(get_float_type<boost::float64_t>(0x7ff, 0x0000000000001_cppui), signaling_nan);
    EXPECT_THAT(get_float_type<boost::float64_t>(0x7ff, 0x7ffffffffffff_cppui), signaling_nan);

    EXPECT_THAT(get_float_type<boost::float64_t>(0x777, 0), normal);
    EXPECT_THAT(get_float_type<boost::float64_t>(0x001, 0xfffffffffffff_cppui), normal);
    EXPECT_THAT(get_float_type<boost::float64_t>(0x400, 0x8000000000000_cppui), normal);
}
#endif

#ifdef BOOST_FLOAT32_C
TEST(TestClassifyNumbers, test_recognize_singles)
{
    EXPECT_THAT(get_float_type<boost::float32_t>(0, 0), zero);
    EXPECT_THAT(get_float_type<boost::float32_t>(0, 0x7fffff_cppui), denormal);
    EXPECT_THAT(get_float_type<boost::float32_t>(0, 0x400000_cppui), denormal);
    EXPECT_THAT(get_float_type<boost::float32_t>(0, 0x000001_cppui), denormal);

    EXPECT_THAT(get_float_type<boost::float32_t>(0xff, 0), infinity);
    EXPECT_THAT(get_float_type<boost::float32_t>(0xff, 0x400000_cppui), quiet_nan);
    EXPECT_THAT(get_float_type<boost::float32_t>(0xff, 0x7fffff_cppui), quiet_nan);
    EXPECT_THAT(get_float_type<boost::float32_t>(0xff, 0x3fffff_cppui), signaling_nan);
    EXPECT_THAT(get_float_type<boost::float32_t>(0xff, 0x000001_cppui), signaling_nan);

    EXPECT_THAT(get_float_type<boost::float32_t>(0x80, 0), normal);
    EXPECT_THAT(get_float_type<boost::float32_t>(0x01, 0), normal);
    EXPECT_THAT(get_float_type<boost::float32_t>(0x77, 0x7fffffff), normal);
    EXPECT_THAT(get_float_type<boost::float32_t>(0xfe, 0x7fffffff), normal);
}
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

struct MinimizeCase
{
    mp::cpp_int mantissa;
    int bin_exp;
    ::testing::Matcher<mp::cpp_int> new_mantissa;
    ::testing::Matcher<int> new_bin_exp;
};

std::ostream& operator<<(std::ostream& os, MinimizeCase const& mincase)
{
    os << "man: " << mincase.mantissa << "; binexp: " << mincase.bin_exp << "; new mantissa ";
    mincase.new_mantissa.DescribeTo(&os);
    os << "; new exponent ";
    mincase.new_bin_exp.DescribeTo(&os);
    return os;
}

class TestMinimizeMantissa: public ::testing::TestWithParam<MinimizeCase>
{
};

TEST_P(TestMinimizeMantissa, test)
{
    auto const result = minimize_mantissa(GetParam().mantissa, GetParam().bin_exp);
    EXPECT_THAT(std::get<0>(result), GetParam().new_mantissa);
    EXPECT_THAT(std::get<1>(result), GetParam().new_bin_exp);
}

MinimizeCase const minimize_cases[] {
    {BOOST_BINARY(011000), 3, mp::cpp_int(BOOST_BINARY(11000)), 3},
    {BOOST_BINARY(0100), -3, mp::cpp_int(BOOST_BINARY(1)), -1},
    {BOOST_BINARY(110000), -3, mp::cpp_int(BOOST_BINARY(000110)), 0},
};

INSTANTIATE_TEST_CASE_P(MinimizeCases,
                        TestMinimizeMantissa,
                        ::testing::ValuesIn(minimize_cases));

struct RemoveFractionCase
{
    mp::cpp_int mantissa;
    int bin_exp;
    ::testing::Matcher<mp::cpp_int> new_mantissa;
    ::testing::Matcher<int> new_bin_exp;
    ::testing::Matcher<int> new_dec_exp;
};

std::ostream& operator<<(std::ostream& os, RemoveFractionCase const& rem)
{
    os << "man: " << rem.mantissa << "; exp: " << rem.bin_exp << "; new mantissa ";
    rem.new_mantissa.DescribeTo(&os);
    os << "; new bin exp ";
    rem.new_bin_exp.DescribeTo(&os);
    os << "; new dec exp ";
    rem.new_dec_exp.DescribeTo(&os);
    return os;
}

class TestRemoveFraction: public ::testing::TestWithParam<RemoveFractionCase>
{
};

TEST_P(TestRemoveFraction, test)
{
    auto const result = remove_fraction(GetParam().mantissa, GetParam().bin_exp);
    EXPECT_THAT(std::get<0>(result), GetParam().new_mantissa);
    EXPECT_THAT(std::get<1>(result), GetParam().new_bin_exp);
    EXPECT_THAT(std::get<2>(result), GetParam().new_dec_exp);
}

RemoveFractionCase const remove_cases[] {
    {2, -4, mp::cpp_int(5*5*5*5*2), 0, -4},
    {3, 0, mp::cpp_int(3), 0, 0},
    {4, 6, mp::cpp_int(4), 6, 0},
};

INSTANTIATE_TEST_CASE_P(RemoveCases,
                        TestRemoveFraction,
                        ::testing::ValuesIn(remove_cases));

struct ReduceCase
{
    mp::cpp_int mantissa;
    int bin_exp;
    ::testing::Matcher<mp::cpp_int> new_mantissa;
};

std::ostream& operator<<(std::ostream& os, ReduceCase const& redcase)
{
    os << "man: " << redcase.mantissa << "; exp: " << redcase.bin_exp << "; new mantissa ";
    redcase.new_mantissa.DescribeTo(&os);
    return os;
}

class TestReduceExponent: public ::testing::TestWithParam<ReduceCase>
{
};

TEST_P(TestReduceExponent, test)
{
    auto const result = reduce_binary_exponent(GetParam().mantissa, GetParam().bin_exp);
    EXPECT_THAT(result, GetParam().new_mantissa);
}

ReduceCase const reduce_cases[] {
    {BOOST_BINARY(1100), 4, mp::cpp_int(BOOST_BINARY(11000000))},
    {BOOST_BINARY(1100), 1, mp::cpp_int(BOOST_BINARY(11000))},
    {BOOST_BINARY(110), 0, mp::cpp_int(BOOST_BINARY(110))},
};

INSTANTIATE_TEST_CASE_P(ReduceCases,
                        TestReduceExponent,
                        ::testing::ValuesIn(reduce_cases));

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
