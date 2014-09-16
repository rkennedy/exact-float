#include "config.h"
#include <utility>
#include <typeindex>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/cstdfloat.hpp>
#include <boost/variant.hpp>
#include "float-literals.h"

using anyfloat = boost::variant<boost::float80_t, boost::float64_t, boost::float32_t>;
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
