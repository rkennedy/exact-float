#include "config.h"
#include <cmath>
#include <iostream>
#include <array>
#include <limits>
#include <typeindex>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/utility/binary.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/mpl/map.hpp>
#include <boost/mpl/map/map30.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/char.hpp>
#include <boost/variant.hpp>
#include "analyze-float.h"

using ::testing::Eq;
using ::testing::StrEq;
using ::testing::_;
using ::testing::ResultOf;
using namespace boost::multiprecision::literals;

using anyfloat = boost::variant<boost::float80_t, boost::float64_t, boost::float32_t>;

namespace
{

typedef boost::mpl::map22<
    boost::mpl::pair<boost::mpl::char_<'0'>, boost::mpl::int_<0>>,
    boost::mpl::pair<boost::mpl::char_<'1'>, boost::mpl::int_<1>>,
    boost::mpl::pair<boost::mpl::char_<'2'>, boost::mpl::int_<2>>,
    boost::mpl::pair<boost::mpl::char_<'3'>, boost::mpl::int_<3>>,
    boost::mpl::pair<boost::mpl::char_<'4'>, boost::mpl::int_<4>>,
    boost::mpl::pair<boost::mpl::char_<'5'>, boost::mpl::int_<5>>,
    boost::mpl::pair<boost::mpl::char_<'6'>, boost::mpl::int_<6>>,
    boost::mpl::pair<boost::mpl::char_<'7'>, boost::mpl::int_<7>>,
    boost::mpl::pair<boost::mpl::char_<'8'>, boost::mpl::int_<8>>,
    boost::mpl::pair<boost::mpl::char_<'9'>, boost::mpl::int_<9>>,
    boost::mpl::pair<boost::mpl::char_<'a'>, boost::mpl::int_<10>>,
    boost::mpl::pair<boost::mpl::char_<'b'>, boost::mpl::int_<11>>,
    boost::mpl::pair<boost::mpl::char_<'c'>, boost::mpl::int_<12>>,
    boost::mpl::pair<boost::mpl::char_<'d'>, boost::mpl::int_<13>>,
    boost::mpl::pair<boost::mpl::char_<'e'>, boost::mpl::int_<14>>,
    boost::mpl::pair<boost::mpl::char_<'f'>, boost::mpl::int_<15>>,
    boost::mpl::pair<boost::mpl::char_<'A'>, boost::mpl::int_<10>>,
    boost::mpl::pair<boost::mpl::char_<'B'>, boost::mpl::int_<11>>,
    boost::mpl::pair<boost::mpl::char_<'C'>, boost::mpl::int_<12>>,
    boost::mpl::pair<boost::mpl::char_<'D'>, boost::mpl::int_<13>>,
    boost::mpl::pair<boost::mpl::char_<'E'>, boost::mpl::int_<14>>,
    boost::mpl::pair<boost::mpl::char_<'F'>, boost::mpl::int_<15>>
    > char_to_nibble;

template <char... Chars>
struct build_float;

template <char C1, char C2, char... Chars>
struct build_float<C1, C2, Chars...>: build_float<Chars...>
{
    template <typename It>
    build_float(It b):
        build_float<Chars...>(b + 1)
    {
        *b = (boost::mpl::at<char_to_nibble, boost::mpl::char_<C1>>::type::value << 4) | boost::mpl::at<char_to_nibble, boost::mpl::char_<C2>>::type::value;
    }
};

template <>
struct build_float<>
{
    template <typename It>
    build_float(It)
    { }
};

template <char Zero, char X, char... Chars, typename It> void
to_float(It b)
{
    static_assert(Zero == '0', "Hex float literal must start with \"0x\"");
    static_assert(X == 'x' || X == 'X', "Hex float literal must start with \"0x\"");
    build_float<Chars...>{b};
}

typedef boost::mpl::map<
#ifdef BOOST_FLOAT80_C
    boost::mpl::pair<boost::mpl::int_<80>, boost::float80_t>,
#endif
#ifdef BOOST_FLOAT64_C
    boost::mpl::pair<boost::mpl::int_<64>, boost::float64_t>,
#endif
#ifdef BOOST_FLOAT32_C
    boost::mpl::pair<boost::mpl::int_<32>, boost::float32_t>,
#endif
    boost::mpl::pair<boost::mpl::void_, boost::mpl::void_>
    > m;
// sizeof...(Chars) - 2 is the number of characters in the hexadecimal portion
// of the literal. The extra two are the "0x" prefix.
template <char... Chars>
typename boost::mpl::at<m, boost::mpl::int_<(sizeof...(Chars) - 2) * 4>>::type operator"" _float()
{
    union {
        typename boost::mpl::at<m, boost::mpl::int_<(sizeof...(Chars) - 2) * 4>>::type f;
        std::array<unsigned char, (sizeof...(Chars) - 2) / 2> b;
    } u;
    to_float<Chars...>(u.b.rbegin());
    return u.f;
}

using HexFloatCase = std::pair<anyfloat, anyfloat>;
class HexFloatTest: public ::testing::TestWithParam<HexFloatCase>
{};

TEST_P(HexFloatTest, test_equality)
{
    ASSERT_THAT(GetParam().first, GetParam().second);
}

TEST_P(HexFloatTest, same_type)
{
    ASSERT_THAT(std::type_index(GetParam().first.type()), std::type_index(GetParam().second.type()));
}

HexFloatCase const hex_float_cases[] {
#ifdef BOOST_FLOAT80_C
    HexFloatCase{BOOST_FLOAT80_C(1.), 0x3fff8000000000000000_float},
    HexFloatCase{BOOST_FLOAT80_C(0.), 0x00000000000000000000_float},
    HexFloatCase{BOOST_FLOAT80_C(-1.), 0xbfff8000000000000000_float},
    HexFloatCase{BOOST_FLOAT80_C(-0.), 0x80000000000000000000_float},
#endif
#ifdef BOOST_FLOAT64_C
    HexFloatCase{BOOST_FLOAT64_C(1.), 0x3ff0000000000000_float},
    HexFloatCase{BOOST_FLOAT64_C(0.), 0x0000000000000000_float},
    HexFloatCase{BOOST_FLOAT64_C(-1.), 0xbff0000000000000_float},
    HexFloatCase{BOOST_FLOAT64_C(-0.), 0x8000000000000000_float},
#endif
#ifdef BOOST_FLOAT32_C
    HexFloatCase{BOOST_FLOAT32_C(1.), 0x3f800000_float},
    HexFloatCase{BOOST_FLOAT32_C(0.), 0x00000000_float},
    HexFloatCase{BOOST_FLOAT32_C(-1.), 0xbf800000_float},
    HexFloatCase{BOOST_FLOAT32_C(-0.), 0x80000000_float},
#endif
};

INSTANTIATE_TEST_CASE_P(
    HFT, HexFloatTest,
    ::testing::ValuesIn(hex_float_cases));

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
