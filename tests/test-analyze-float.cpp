#include "config.h"
#include <cmath>
#include <iostream>
#include <limits>
#include <typeindex>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/utility/binary.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/variant.hpp>
#include "analyze-float.h"

using ::testing::Eq;
using ::testing::StrEq;
using ::testing::_;
using ::testing::ResultOf;
using namespace boost::multiprecision::literals;

struct TypeCase
{
    mp::cpp_int exponent;
    mp::cpp_int mantissa;
    std::type_index type;
    ::testing::Matcher<float_type> expected_type;
};

std::ostream& operator<<(std::ostream& os, TypeCase const& typecase)
{
    os << "exp: " << std::hex << typecase.exponent << "; man: " << typecase.mantissa << "; type: " << typecase.type.name() << "; result ";
    typecase.expected_type.DescribeTo(&os);
    return os;
}

TypeCase const case_expectations[] {
#ifdef BOOST_FLOAT80_C
    { 0, 0, typeid(boost::float80_t), zero },
    { 0, 0x7fffffffffffffff_cppui, typeid(boost::float80_t), denormal },
    { 0, 0x4aaaaaaaaaaaaaaa_cppui, typeid(boost::float80_t), denormal },
    { 0, 0x8000000000000000_cppui, typeid(boost::float80_t), denormal },
    { 0, 0xffffffffffffffff_cppui, typeid(boost::float80_t), denormal },
    { 0x7fff, 0, typeid(boost::float80_t), infinity },
    { 0x7fff, 0x3fffffffffffffff_cppui, typeid(boost::float80_t), signaling_nan },
    { 0x7fff, 0x4000000000000000_cppui, typeid(boost::float80_t), signaling_nan },
    { 0x7fff, 0x8000000000000000_cppui, typeid(boost::float80_t), infinity },
    { 0x7fff, 0xbfffffffffffffff_cppui, typeid(boost::float80_t), signaling_nan },
    { 0x7fff, 0x8000000000000001_cppui, typeid(boost::float80_t), signaling_nan },
    { 0x7fff, 0xc000000000000000_cppui, typeid(boost::float80_t), indefinite },
    { 0x7fff, 0xffffffffffffffff_cppui, typeid(boost::float80_t), quiet_nan },
    { 0x7fff, 0xc000000000000001_cppui, typeid(boost::float80_t), quiet_nan },
    { 0x7777, 0x0000000000000000_cppui, typeid(boost::float80_t), denormal },
    { 0x4444, 0x7fffffffffffffff_cppui, typeid(boost::float80_t), denormal },
    { 0x3333, 0x8000000000000000_cppui, typeid(boost::float80_t), normal },
    { 0x3333, 0xffffffffffffffff_cppui, typeid(boost::float80_t), normal },
#endif
#ifdef BOOST_FLOAT64_C
    { 0, 0, typeid(boost::float64_t), zero },
    { 0, 0xfffffffffffff_cppui, typeid(boost::float64_t), denormal },
    { 0, 0x8000000000000_cppui, typeid(boost::float64_t), denormal },
    { 0, 0x0000000000001_cppui, typeid(boost::float64_t), denormal },

    { 0x7ff, 0, typeid(boost::float64_t), infinity },
    { 0x7ff, 0x8000000000000_cppui, typeid(boost::float64_t), quiet_nan },
    { 0x7ff, 0xfffffffffffff_cppui, typeid(boost::float64_t), quiet_nan },
    { 0x7ff, 0x0000000000001_cppui, typeid(boost::float64_t), signaling_nan },
    { 0x7ff, 0x7ffffffffffff_cppui, typeid(boost::float64_t), signaling_nan },

    { 0x777, 0, typeid(boost::float64_t), normal },
    { 0x001, 0xfffffffffffff_cppui, typeid(boost::float64_t), normal },
    { 0x400, 0x8000000000000_cppui, typeid(boost::float64_t), normal },
#endif
#ifdef BOOST_FLOAT32_C
    { 0, 0, typeid(boost::float32_t), zero },
    { 0, 0x7fffff_cppui, typeid(boost::float32_t), denormal },
    { 0, 0x400000_cppui, typeid(boost::float32_t), denormal },
    { 0, 0x000001_cppui, typeid(boost::float32_t), denormal },

    { 0xff, 0, typeid(boost::float32_t), infinity },
    { 0xff, 0x400000_cppui, typeid(boost::float32_t), quiet_nan },
    { 0xff, 0x7fffff_cppui, typeid(boost::float32_t), quiet_nan },
    { 0xff, 0x3fffff_cppui, typeid(boost::float32_t), signaling_nan },
    { 0xff, 0x000001_cppui, typeid(boost::float32_t), signaling_nan },

    { 0x80, 0, typeid(boost::float32_t), normal },
    { 0x01, 0, typeid(boost::float32_t), normal },
    { 0x77, 0x7fffffff, typeid(boost::float32_t), normal },
    { 0xfe, 0x7fffffff, typeid(boost::float32_t), normal },
#endif
};

class TestClassifyNumbers: public ::testing::TestWithParam<TypeCase>
{
};

TEST_P(TestClassifyNumbers, test)
{
    auto const result = get_float_type(GetParam().exponent, GetParam().mantissa, GetParam().type);
    EXPECT_THAT(result, GetParam().expected_type);
}

INSTANTIATE_TEST_CASE_P(Classify,
                        TestClassifyNumbers,
                        ::testing::ValuesIn(case_expectations));

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

using anyfloat = boost::variant<boost::float80_t, boost::float64_t, boost::float32_t>;

struct get_exact: public boost::static_visitor<FloatInfo>
{
    template <typename Float>
    FloatInfo operator()(Float const value) const {
        return exact(value);
    }
};

using expectations_t = std::map<anyfloat, Expectations>;
expectations_t const boost_float_expectations {
#ifdef BOOST_FLOAT80_C
    {BOOST_FLOAT80_C(0.), {false, Eq(0), Eq(0), zero}},
    {BOOST_FLOAT80_C(1.), {false, Eq(16383), Eq(0x8000000000000000ull), normal}},
    {BOOST_FLOAT80_C(-1.), {true, Eq(16383), Eq(0x8000000000000000ull), normal}},
    {std::strtold("inf", NULL), {false, _, _, infinity}},
#endif
#ifdef BOOST_FLOAT64_C
    {BOOST_FLOAT64_C(0.), {false, Eq(0), Eq(0), zero}},
    {BOOST_FLOAT64_C(1.), {false, Eq(1023), Eq(0), normal}},
    {BOOST_FLOAT64_C(-2.), {true, Eq(1024), Eq(0), normal}},
    {std::strtod("inf", NULL), {false, _, _, infinity}},
#endif
#ifdef BOOST_FLOAT32_T
    {BOOST_FLOAT32_C(0.), {false, 0, 0, zero}},
    {BOOST_FLOAT32_C(1.), {false, 127, 0, normal}},
    {BOOST_FLOAT32_C(-2.), {true, 128, 0, normal}},
    {std::strtof("inf", NULL), {false, _, _, infinity}},
#endif
};

class ClassificationTest: public ::testing::TestWithParam<expectations_t::value_type>
{
};

TEST_P(ClassificationTest, identify_values)
{
    auto const info(boost::apply_visitor(get_exact(), GetParam().first));
    EXPECT_THAT(info.negative, GetParam().second.negative);
    EXPECT_THAT(info.exponent, GetParam().second.exponent);
    EXPECT_THAT(info.mantissa, GetParam().second.mantissa);
    EXPECT_THAT(info.number_type, GetParam().second.type);
}

INSTANTIATE_TEST_CASE_P(Classifications,
                        ClassificationTest,
                        ::testing::ValuesIn(boost_float_expectations));

struct SerializationParam
{
    bool negative;
    mp::cpp_int exponent;
    mp::cpp_int mantissa;
    float_type number_type;
    std::type_index type;
    char const* expectation;
};

std::ostream& operator<<(std::ostream& os, SerializationParam const& sp)
{
    return os << sp.type.name() << ": neg: " << std::boolalpha << sp.negative << "; exp: " << std::hex << std::setfill('0') << std::setw(4) << sp.exponent << "; man: " << std::setw(16) << sp.mantissa << "; type: " << sp.number_type << "; expecting \"" << sp.expectation << "\"";
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
    FloatInfo const value(GetParam().negative, GetParam().exponent, GetParam().mantissa, GetParam().number_type, GetParam().type);
    EXPECT_THAT(os << value, ResultOf(str, StrEq(GetParam().expectation)));
}

SerializationParam const extended_serializations[] = {
#ifdef BOOST_FLOAT80_C
    {false, std::numeric_limits<boost::float80_t>::max_exponent - 1, BOOST_BINARY_ULL(
            10000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000), normal, typeid(boost::float80_t),
    "+ 1"},
    {true, std::numeric_limits<boost::float80_t>::max_exponent - 1, BOOST_BINARY_ULL(
            11000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000), normal, typeid(boost::float80_t),
    "- 1.5"},
    {false, std::numeric_limits<boost::float80_t>::max_exponent + 5, BOOST_BINARY_ULL(
            10101110 10010001 11101011 10000101 00011110 10111000 01010001 11101100), normal, typeid(boost::float80_t),
    "+ 87.28500 00000 00000 00333 06690 73875 46962 12708 95004 27246 09375"},
#endif

#ifdef BOOST_FLOAT64_C
    {false, std::numeric_limits<boost::float64_t>::max_exponent - 1, BOOST_BINARY_ULL(
            0000 00000000 00000000 00000000 00000000 00000000 00000000), normal, typeid(boost::float64_t),
    "+ 1"},
    {true, std::numeric_limits<boost::float64_t>::max_exponent - 1, BOOST_BINARY_ULL(
            1000 00000000 00000000 00000000 00000000 00000000 00000000), normal, typeid(boost::float64_t),
    "- 1.5"},
    {false, std::numeric_limits<boost::float64_t>::max_exponent + 5, BOOST_BINARY_ULL(
            0101 11010010 00111101 01110000 10100011 11010111 00001010), normal, typeid(boost::float64_t),
    "+ 87.28499 99999 99996 58939 48683 51519 10781 86035 15625"},
#endif

#ifdef BOOST_FLOAT32_C
    {false, std::numeric_limits<boost::float32_t>::max_exponent - 1, BOOST_BINARY_ULL(
            0000000 00000000 00000000), normal, typeid(boost::float32_t),
    "+ 1"},
    {true, std::numeric_limits<boost::float32_t>::max_exponent - 1, BOOST_BINARY_ULL(
            1000000 00000000 00000000), normal, typeid(boost::float32_t),
    "- 1.5"},
    {false, std::numeric_limits<boost::float32_t>::max_exponent + 5, BOOST_BINARY_ULL(
            0101110 10010001 11101100), normal, typeid(boost::float32_t),
    "+ 87.28500 36621 09375"},
#endif
};

INSTANTIATE_TEST_CASE_P(ExtendedSerializations, Serialization,
                        ::testing::ValuesIn(extended_serializations));
#endif
