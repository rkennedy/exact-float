#include "config.h"
#include <cmath>
#include <ios>
#include <iostream>
#include <array>
#include <limits>
#include <locale>
#include <typeindex>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/utility/binary.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/variant.hpp>
#include <boost/format.hpp>
#include "float-literals.h"
#include "analyze-float.h"

using ::testing::Eq;
using ::testing::StrEq;
using ::testing::_;
using ::testing::ResultOf;
using namespace boost::multiprecision::literals;

using anyfloat = boost::variant<boost::float80_t, boost::float64_t, boost::float32_t>;

// We don't want Google Test to try printing a FloatInfo value with the default
// operator<< because that's one of the things that's being tested.
void PrintTo(FloatInfo const& info, std::ostream* os)
{
    *os << boost::format("{ neg: %|1|, exponent: %|2|, mantissa: %|3|, numtype: %|4| }") % info.negative % info.exponent % info.mantissa % info.number_type;
}

struct ConstructionCase
{
    anyfloat number;
    bool negative;
    mp::cpp_int exponent;
    mp::cpp_int mantissa;
    float_type number_type;
    std::type_index type;
};

class ConstructionTest: public ::testing::TestWithParam<ConstructionCase>
{
};

TEST_P(ConstructionTest, direct_construction)
{
    FloatInfo const info(
        GetParam().negative,
        GetParam().exponent,
        GetParam().mantissa,
        GetParam().number_type,
        GetParam().type);
    EXPECT_THAT(info.negative, GetParam().negative);
    EXPECT_THAT(info.exponent, GetParam().exponent);
    EXPECT_THAT(info.mantissa, GetParam().mantissa);
    EXPECT_THAT(info.number_type, GetParam().number_type);
}

struct get_float_info: public boost::static_visitor<FloatInfo>
{
    template <typename Float>
    FloatInfo operator()(Float const value) const {
        return FloatInfo(value);
    }
};

TEST_P(ConstructionTest, number_construction)
{
    FloatInfo const info {
        boost::apply_visitor(get_float_info(),
                             GetParam().number)
    };
    EXPECT_THAT(info.negative, GetParam().negative);
    EXPECT_THAT(info.exponent, GetParam().exponent);
    EXPECT_THAT(info.mantissa, GetParam().mantissa);
    EXPECT_THAT(info.number_type, GetParam().number_type);
}

TEST_P(ConstructionTest, equality)
{
    FloatInfo const field_info{GetParam().negative, GetParam().exponent, GetParam().mantissa, GetParam().number_type, GetParam().type};
    FloatInfo const number_info{ boost::apply_visitor(get_float_info(), GetParam().number) };
    EXPECT_THAT(field_info, Eq(number_info));
}

ConstructionCase const construction_params[] {
#ifdef BOOST_FLOAT80_C
    { 0x00000000000000000000_float, false, 0x0000, 0x0000000000000000_cppui, zero, typeid(boost::float80_t) },
    { 0x00007fffffffffffffff_float, false, 0x0000, 0x7fffffffffffffff_cppui, denormal, typeid(boost::float80_t) },
    { 0x00004aaaaaaaaaaaaaaa_float, false, 0x0000, 0x4aaaaaaaaaaaaaaa_cppui, denormal, typeid(boost::float80_t) },
    { 0x00008000000000000000_float, false, 0x0000, 0x8000000000000000_cppui, denormal, typeid(boost::float80_t) },
    { 0x0000ffffffffffffffff_float, false, 0x0000, 0xffffffffffffffff_cppui, denormal, typeid(boost::float80_t) },
    { 0x7fff0000000000000000_float, false, 0x7fff, 0x0000000000000000_cppui, infinity, typeid(boost::float80_t) },
    { 0x7fff3fffffffffffffff_float, false, 0x7fff, 0x3fffffffffffffff_cppui, signaling_nan, typeid(boost::float80_t) },
    { 0x7fff4000000000000000_float, false, 0x7fff, 0x4000000000000000_cppui, signaling_nan, typeid(boost::float80_t) },
    { 0x7fff8000000000000000_float, false, 0x7fff, 0x8000000000000000_cppui, infinity, typeid(boost::float80_t) },
    { 0x7fffbfffffffffffffff_float, false, 0x7fff, 0xbfffffffffffffff_cppui, signaling_nan, typeid(boost::float80_t) },
    { 0x7fff8000000000000001_float, false, 0x7fff, 0x8000000000000001_cppui, signaling_nan, typeid(boost::float80_t) },
    { 0x7fffc000000000000000_float, false, 0x7fff, 0xc000000000000000_cppui, indefinite, typeid(boost::float80_t) },
    { 0x7fffffffffffffffffff_float, false, 0x7fff, 0xffffffffffffffff_cppui, quiet_nan, typeid(boost::float80_t) },
    { 0x7fffc000000000000001_float, false, 0x7fff, 0xc000000000000001_cppui, quiet_nan, typeid(boost::float80_t) },
    { 0x77770000000000000000_float, false, 0x7777, 0x0000000000000000_cppui, denormal, typeid(boost::float80_t) },
    { 0x44447fffffffffffffff_float, false, 0x4444, 0x7fffffffffffffff_cppui, denormal, typeid(boost::float80_t) },
    { 0x33338000000000000000_float, false, 0x3333, 0x8000000000000000_cppui, normal, typeid(boost::float80_t) },
    { 0x3333ffffffffffffffff_float, false, 0x3333, 0xffffffffffffffff_cppui, normal, typeid(boost::float80_t) },
#endif
#ifdef BOOST_FLOAT64_C
    { 0x0000000000000000_float, false, 0x000, 0x0000000000000_cppui, zero, typeid(boost::float64_t) },
    { 0x000fffffffffffff_float, false, 0x000, 0xfffffffffffff_cppui, denormal, typeid(boost::float64_t) },
    { 0x0008000000000000_float, false, 0x000, 0x8000000000000_cppui, denormal, typeid(boost::float64_t) },
    { 0x0000000000000001_float, false, 0x000, 0x0000000000001_cppui, denormal, typeid(boost::float64_t) },
    { 0x7ff0000000000000_float, false, 0x7ff, 0x0000000000000_cppui, infinity, typeid(boost::float64_t) },
    { 0x7ff8000000000000_float, false, 0x7ff, 0x8000000000000_cppui, quiet_nan, typeid(boost::float64_t) },
    { 0x7fffffffffffffff_float, false, 0x7ff, 0xfffffffffffff_cppui, quiet_nan, typeid(boost::float64_t) },
    { 0x7ff0000000000001_float, false, 0x7ff, 0x0000000000001_cppui, signaling_nan, typeid(boost::float64_t) },
    { 0x7ff7ffffffffffff_float, false, 0x7ff, 0x7ffffffffffff_cppui, signaling_nan, typeid(boost::float64_t) },
    { 0x7770000000000000_float, false, 0x777, 0x0000000000000_cppui, normal, typeid(boost::float64_t) },
    { 0x001fffffffffffff_float, false, 0x001, 0xfffffffffffff_cppui, normal, typeid(boost::float64_t) },
    { 0x4008000000000000_float, false, 0x400, 0x8000000000000_cppui, normal, typeid(boost::float64_t) },
#endif
#ifdef BOOST_FLOAT32_C
    { 0x00000000_float, false, 0x00, 0, zero, typeid(boost::float32_t) },
    { 0x007fffff_float, false, 0x00, 0x7fffff_cppui, denormal, typeid(boost::float32_t) },
    { 0x00400000_float, false, 0x00, 0x400000_cppui, denormal, typeid(boost::float32_t) },
    { 0x00000001_float, false, 0x00, 0x000001_cppui, denormal, typeid(boost::float32_t) },
    { 0x7f800000_float, false, 0xff, 0x000000_cppui, infinity, typeid(boost::float32_t) },
    { 0x7fc00000_float, false, 0xff, 0x400000_cppui, quiet_nan, typeid(boost::float32_t) },
    { 0x7fffffff_float, false, 0xff, 0x7fffff_cppui, quiet_nan, typeid(boost::float32_t) },
    { 0x7fbfffff_float, false, 0xff, 0x3fffff_cppui, signaling_nan, typeid(boost::float32_t) },
    { 0x7f800001_float, false, 0xff, 0x000001_cppui, signaling_nan, typeid(boost::float32_t) },
    { 0x40000000_float, false, 0x80, 0x000000_cppui, normal, typeid(boost::float32_t) },
    { 0x00800000_float, false, 0x01, 0x000000_cppui, normal, typeid(boost::float32_t) },
    { 0x3bffffff_float, false, 0x77, 0x7fffff_cppui, normal, typeid(boost::float32_t) },
    { 0x7f7fffff_float, false, 0xfe, 0x7fffff_cppui, normal, typeid(boost::float32_t) },
#endif
};

INSTANTIATE_TEST_CASE_P(
    Construction,
    ConstructionTest,
    ::testing::ValuesIn(construction_params));

struct SerializationParam
{
    anyfloat value;
    char const* expectation;
};

std::ostream& operator<<(std::ostream& os, SerializationParam const& sp)
{
    return os << boost::format("%|1| (%|2|); expecting \"%|1|\"") % sp.value % std::type_index(sp.value.type()).name() % sp.expectation;
}

class Serialization: public ::testing::TestWithParam<SerializationParam>
{
public:
    std::ostringstream os;
    static std::string str(std::ostream const& s) {
        return dynamic_cast<std::ostringstream const&>(s).str();
    }
    void SetUp() override {
        os.imbue(std::locale::classic());
    }
};

TEST_P(Serialization, test)
{
    FloatInfo const value {
        boost::apply_visitor(get_float_info(),
                             GetParam().value)
    };
    EXPECT_THAT(os << value, ResultOf(str, StrEq(GetParam().expectation)));
}

SerializationParam const serializations[] = {
#ifdef BOOST_FLOAT80_C
    { BOOST_FLOAT80_C(1.), "1" },
    { BOOST_FLOAT80_C(-1.5), "-1.5" },
    { BOOST_FLOAT80_C(87.285), "87.28500" "00000" "00000" "00333" "06690" "73875" "46962" "12708" "95004" "27246" "09375" },
    { BOOST_FLOAT80_C(0.0625), "0.0625" },
#endif

#ifdef BOOST_FLOAT64_C
    { BOOST_FLOAT64_C(1.), "1" },
    { BOOST_FLOAT64_C(-1.5), "-1.5" },
    { BOOST_FLOAT64_C(87.285), "87.28499" "99999" "99996" "58939" "48683" "51519" "10781" "86035" "15625" },
    { BOOST_FLOAT64_C(0.0625), "0.0625" },
#endif

#ifdef BOOST_FLOAT32_C
    { BOOST_FLOAT32_C(1.), "1" },
    { BOOST_FLOAT32_C(-1.5), "-1.5" },
    { BOOST_FLOAT32_C(87.285), "87.28500" "36621" "09375" },
    { BOOST_FLOAT32_C(0.0625), "0.0625" },
#endif
};

INSTANTIATE_TEST_CASE_P(Serializations, Serialization,
                        ::testing::ValuesIn(serializations));

TEST_F(Serialization, honor_showpos_for_positive)
{
    FloatInfo const value { 1.5 };
    EXPECT_THAT(os << std::showpos << value,
                ResultOf(str, StrEq("+1.5")));
}

TEST_F(Serialization, no_print_pos_sign_on_negative)
{
    // If the whole and fractional parts are printed as standalone numbers,
    // then showpos might cause them to incorrectly include signs of their
    // own.
    FloatInfo const value { -1.5 };
    EXPECT_THAT(os << std::showpos << value,
                ResultOf(str, StrEq("-1.5")));
}

TEST_F(Serialization, ignore_noshowpos_for_negative)
{
    FloatInfo const value { -1.5 };
    EXPECT_THAT(os << std::noshowpos << value,
                ResultOf(str, StrEq("-1.5")));
}

TEST_F(Serialization, restore_showpos)
{
    FloatInfo const value { 1.5 };
    EXPECT_THAT(os << std::showpos << value << value,
                ResultOf(str, StrEq("+1.5+1.5")));
}

TEST_F(Serialization, restore_fill)
{
    FloatInfo const value { -1.0625 };
    EXPECT_THAT(os << std::setfill('$') << value << std::setw(5) << -1,
                ResultOf(str, StrEq("-1.0625$$$-1")));
}

TEST_F(Serialization, honor_locale_decimal_separator)
{
    struct test_punct: std::numpunct<char>
    {
        char_type do_decimal_point() const override {
            return ':';
        }
    };
    FloatInfo const value { 1.5 };
    os.imbue(std::locale(os.getloc(), new test_punct()));
    EXPECT_THAT(os << value,
                ResultOf(str, StrEq("1:5")));
}
