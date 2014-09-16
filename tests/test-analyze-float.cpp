#include "config.h"
#include <cmath>
#include <iostream>
#include <array>
#include <limits>
#include <typeindex>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/utility/enable_if.hpp>
#include <boost/utility/binary.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/mpl/map.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/bitwise.hpp>
#include <boost/mpl/char.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/long.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/insert.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/eval_if.hpp>
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

template <unsigned char B>
using uchar = boost::mpl::integral_c<unsigned char, B>;

struct invalid_char {};

template <char C>
using char_to_nibble =
    typename boost::mpl::eval_if_c<C == '0', uchar<0>,
    typename boost::mpl::eval_if_c<C == '1', uchar<1>,
    typename boost::mpl::eval_if_c<C == '2', uchar<2>,
    typename boost::mpl::eval_if_c<C == '3', uchar<3>,
    typename boost::mpl::eval_if_c<C == '4', uchar<4>,
    typename boost::mpl::eval_if_c<C == '5', uchar<5>,
    typename boost::mpl::eval_if_c<C == '6', uchar<6>,
    typename boost::mpl::eval_if_c<C == '7', uchar<7>,
    typename boost::mpl::eval_if_c<C == '8', uchar<8>,
    typename boost::mpl::eval_if_c<C == '9', uchar<9>,
    typename boost::mpl::eval_if_c<C == 'a' || C == 'A', uchar<10>,
    typename boost::mpl::eval_if_c<C == 'b' || C == 'B', uchar<11>,
    typename boost::mpl::eval_if_c<C == 'c' || C == 'C', uchar<12>,
    typename boost::mpl::eval_if_c<C == 'd' || C == 'D', uchar<13>,
    typename boost::mpl::eval_if_c<C == 'e' || C == 'E', uchar<14>,
    typename boost::mpl::eval_if_c<C == 'f' || C == 'F', uchar<15>,
    invalid_char>>>>>>>>>>>>>>>>::type;

template <char... Chars>
struct build_float;

template <char C1, char C2, char... Chars>
struct build_float<C1, C2, Chars...>
{
    using type = typename boost::mpl::push_back<
        typename build_float<Chars...>::type,
        typename boost::mpl::bitor_<
            typename boost::mpl::shift_left<
                char_to_nibble<C1>,
                boost::mpl::int_<4>
            >::type,
            char_to_nibble<C2>
        >::type
    >::type;
};

template <>
struct build_float<>
{
    using type = boost::mpl::vector<>;
};

template <size_t... Indices>
struct indices
{
    using type = indices<Indices..., sizeof...(Indices)>;
};

template <size_t N>
struct build_indices
{
    using type = typename build_indices<N - 1>::type::type;
};

template <>
struct build_indices<0>
{
    using type = indices<>;
};

#ifdef BOOST_FLOAT80_C
using m2 = boost::mpl::insert<boost::mpl::map<>, boost::mpl::pair<boost::mpl::long_<80>, boost::float80_t>>::type;
#else
using m2 = boost::mpl::map<>;
#endif
#ifdef BOOST_FLOAT64_C
using m1 = boost::mpl::insert<m2, boost::mpl::pair<boost::mpl::long_<64>, boost::float64_t>>::type;
#else
using m1 = m2;
#endif
#ifdef BOOST_FLOAT32_C
using m = boost::mpl::insert<m1, boost::mpl::pair<boost::mpl::long_<32>, boost::float32_t>>::type;
#else
using m = m1;
#endif

template <size_t Bits>
union hex_float
{
    using array_type = std::array<unsigned char, Bits / CHAR_BIT>;

    typename boost::mpl::at_c<m, Bits>::type f;
    array_type b;

    constexpr hex_float(array_type const& b):
        b(b)
    { }
};

template <char Zero, char X, char... Chars>
struct to_float
{
private:
    static_assert(Zero == '0', "Hex float literal must start with \"0x\"");
    static_assert(X == 'x' || X == 'X', "Hex float literal must start with \"0x\"");
    //static_assert(sizeof...(Chars) % 2 == 0, "Hex float literal must have even number of characters");
    using type = typename build_float<Chars...>::type;
    static constexpr size_t length = boost::mpl::size<type>::value;
    static_assert(length == sizeof...(Chars) / 2, "Length mismatch");
    using array_type = typename hex_float<length * CHAR_BIT>::array_type;

    template <size_t... Indices>
    static constexpr array_type make(indices<Indices...> const&) {
        static_assert(length == sizeof...(Indices), "Length mismatch");
        return array_type{{
            boost::mpl::at_c<type, Indices>::type::value...
        }};
    }
public:
    static constexpr std::array<unsigned char, length> as_array() {
        return make(typename build_indices<length>::type{});
    }
};

// sizeof...(Chars) - 2 is the number of characters in the hexadecimal portion
// of the literal. The extra two are the "0x" prefix.
template <char... Chars> constexpr
typename boost::enable_if<
    boost::mpl::has_key<
        m,
        boost::mpl::long_<(sizeof...(Chars) - 2) * CHAR_BIT / 2>
    >,
    typename boost::mpl::at<
        m,
        boost::mpl::long_<(sizeof...(Chars) - 2) * CHAR_BIT / 2>
    >::type
>::type operator"" _float()
{
    return hex_float<(sizeof...(Chars) - 2) / 2 * CHAR_BIT>{
        to_float<Chars...>::as_array()
    }.f;
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
