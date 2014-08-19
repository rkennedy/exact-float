#include "config.h"
#include <cstdlib>
#include <cmath>
#include <tuple>
#include <limits>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/dynamic_bitset.hpp>
#include "analyze-float.h"
#include "src/analyze-float.cpp"

using ::testing::Eq;
using ::testing::_;

int main(int argc, char* argv[])
{
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}

struct Expectations
{
    ::testing::Matcher<bool> negative;
    ::testing::Matcher<uint16_t> exponent;
    ::testing::Matcher<uint64_t> mantissa;
    ::testing::Matcher<float_type> type;
};

typedef std::map<long double, Expectations> long_double_expectations_t;
long_double_expectations_t const long_double_expectations {
    {0, {false, 0, 0, zero}},
    {1.0, {false, 16383, 0x8000000000000000ull, normal}},
    {-1, {true, 16383, 0x8000000000000000ull, normal}},
    // TODO {std::strtold("inf", NULL), {false, _, _, infinity}},
};

typedef std::map<double, Expectations> double_expectations_t;
double_expectations_t const double_expectations {
    {0, {false, 0, 0, zero}},
    {1.0, {false, 1023, 0, normal}},
    {-2, {true, 1024, 0, normal}},
    {std::strtod("inf", NULL), {false, _, _, infinity}},
};

typedef std::map<float, Expectations> single_expectations_t;
single_expectations_t const single_expectations {
    {0, {false, 0, 0, zero}},
    {1.0, {false, 127, 0, normal}},
    {-2, {true, 128, 0, normal}},
    {std::strtof("inf", NULL), {false, _, _, infinity}},
};

class LongDoubleClassificationTest: public ::testing::TestWithParam<long_double_expectations_t::value_type>
{
};

class DoubleClassificationTest: public ::testing::TestWithParam<double_expectations_t::value_type>
{
};

class SingleClassificationTest: public ::testing::TestWithParam<single_expectations_t::value_type>
{
};

TEST_P(LongDoubleClassificationTest, identify_values)
{
    auto const info(make_float_info(GetParam().first));
    EXPECT_THAT(info.negative, GetParam().second.negative);
    EXPECT_THAT(info.exponent, GetParam().second.exponent);
    EXPECT_THAT(info.mantissa, GetParam().second.mantissa);
    EXPECT_THAT(info.number_type, GetParam().second.type);
}

TEST_P(DoubleClassificationTest, identify_values)
{
    auto const info(make_float_info(GetParam().first));
    EXPECT_THAT(info.negative, GetParam().second.negative);
    EXPECT_THAT(info.exponent, GetParam().second.exponent);
    EXPECT_THAT(info.mantissa, GetParam().second.mantissa);
    EXPECT_THAT(info.number_type, GetParam().second.type);
}

TEST_P(SingleClassificationTest, identify_values)
{
    auto const info(make_float_info(GetParam().first));
    EXPECT_THAT(info.negative, GetParam().second.negative);
    EXPECT_THAT(info.exponent, GetParam().second.exponent);
    EXPECT_THAT(info.mantissa, GetParam().second.mantissa);
    EXPECT_THAT(info.number_type, GetParam().second.type);
}

INSTANTIATE_TEST_CASE_P(LongDoubleClassifications,
                        LongDoubleClassificationTest,
                        ::testing::ValuesIn(long_double_expectations));
INSTANTIATE_TEST_CASE_P(DoubleClassifications,
                        DoubleClassificationTest,
                        ::testing::ValuesIn(double_expectations));
INSTANTIATE_TEST_CASE_P(SingleClassifications,
                        SingleClassificationTest,
                        ::testing::ValuesIn(single_expectations));

TEST(BitsetOpsTest, test_find_last)
{
    bitset empty_bitset;
    EXPECT_THAT(find_last(empty_bitset), Eq(bitset::size_type(-1)));

    bitset one_bit_set(std::string("100"));
    EXPECT_THAT(find_last(one_bit_set), Eq(2u));

    bitset many_bits_set(std::string("1111111111111111111111111111111111"));
    EXPECT_THAT(find_last(many_bits_set), Eq(33u));
}

TEST(BitsetOpsTest, test_build_bitset)
{
    EXPECT_THAT(build_bitset(0, 0), Eq(bitset()));
    EXPECT_THAT(build_bitset(0xAAAAAAAAAAAAAAAAull, 64),
                Eq(bitset(std::string("1010101010101010101010101010101010101010101010101010101010101010"))));
}

TEST(BitsetOpsTest, test_find_last_int)
{
    EXPECT_THAT(find_last(0), Eq(size_t(-1)));
    EXPECT_THAT(find_last(1), Eq(size_t(0)));
    EXPECT_THAT(find_last(20), Eq(size_t(4)));
}

TEST(BitsetOpsTest, test_build_bitset_int)
{
    EXPECT_THAT(build_bitset(0), Eq(bitset()));
    EXPECT_THAT(build_bitset(1), Eq(bitset(std::string("1"))));
    EXPECT_THAT(build_bitset(20), Eq(bitset(std::string("10100"))));
}

TEST(BitsetOpsTest, test_one_bit_add)
{
    EXPECT_THAT(one_bit_add(false, false, false),
                Eq(std::make_tuple(false, false)));
    EXPECT_THAT(one_bit_add(false, false, true),
                Eq(std::make_tuple(true, false)));
    EXPECT_THAT(one_bit_add(false, true, false),
                Eq(std::make_tuple(true, false)));
    EXPECT_THAT(one_bit_add(false, true, true),
                Eq(std::make_tuple(false, true)));
    EXPECT_THAT(one_bit_add(true, false, false),
                Eq(std::make_tuple(true, false)));
    EXPECT_THAT(one_bit_add(true, false, true),
                Eq(std::make_tuple(false, true)));
    EXPECT_THAT(one_bit_add(true, true, false),
                Eq(std::make_tuple(false, true)));
    EXPECT_THAT(one_bit_add(true, true, true),
                Eq(std::make_tuple(true, true)));
}

TEST(BitsetOpsTest, test_ont_bit_subtract)
{
    EXPECT_THAT(one_bit_subtract(false, false, false),
                Eq(std::make_tuple(false, false)));
    EXPECT_THAT(one_bit_subtract(false, false, true),
                Eq(std::make_tuple(true, true)));
    EXPECT_THAT(one_bit_subtract(false, true, false),
                Eq(std::make_tuple(true, true)));
    EXPECT_THAT(one_bit_subtract(false, true, true),
                Eq(std::make_tuple(false, true)));
    EXPECT_THAT(one_bit_subtract(true, false, false),
                Eq(std::make_tuple(true, false)));
    EXPECT_THAT(one_bit_subtract(true, false, true),
                Eq(std::make_tuple(false, false)));
    EXPECT_THAT(one_bit_subtract(true, true, false),
                Eq(std::make_tuple(false, false)));
    EXPECT_THAT(one_bit_subtract(true, true, true),
                Eq(std::make_tuple(true, true)));
}

TEST(BitsetOpsTest, test_multiply)
{
    EXPECT_THAT(Multiply(build_bitset(10), build_bitset(50)),
                Eq(build_bitset(500)));
    EXPECT_THAT(Multiply(build_bitset(50), build_bitset(2)),
                Eq(build_bitset(100)));
    EXPECT_THAT(Multiply(build_bitset(0), build_bitset(1)),
                Eq(bitset()));
    EXPECT_THAT(Multiply(build_bitset(0xffffffff), build_bitset(0xffffffff)),
                Eq(build_bitset(18446744065119617025u)));
    EXPECT_THAT(Multiply(bitset(), bitset()),
                Eq(bitset()));
}

TEST(BitsetOpsTest, test_subtract)
{
    EXPECT_THAT(Subtract(bitset(), bitset()),
                Eq(bitset()));
    EXPECT_THAT(Subtract(build_bitset(500), build_bitset(3)),
                Eq(build_bitset(497)));
    EXPECT_THAT(Subtract(build_bitset(15), build_bitset(15)),
                Eq(bitset()));
}

TEST(BitsetOpsTest, test_bitset_ge)
{
    EXPECT_PRED2(std::greater_equal<bitset>(),
                 build_bitset(2), build_bitset(1));
    EXPECT_PRED2(std::greater_equal<bitset>(),
                 build_bitset(10), build_bitset(9));
    EXPECT_PRED2(std::greater_equal<bitset>(),
                 build_bitset(20), build_bitset(2));
    EXPECT_PRED2(std::greater_equal<bitset>(),
                 bitset(std::string("00100")), bitset(std::string("100")));
    EXPECT_PRED2(std::not2(std::greater_equal<bitset>()),
                 bitset(), build_bitset(2));
}

TEST(BitsetOpsTest, test_divide)
{
    EXPECT_THAT(DivideAndRemainder(build_bitset(42), build_bitset(7)),
                Eq(std::make_tuple(build_bitset(6), 0u)));
    EXPECT_THAT(DivideAndRemainder(bitset(), build_bitset(45)),
                Eq(std::make_tuple(bitset(), 0u)));
    EXPECT_THAT(DivideAndRemainder(build_bitset(10235), build_bitset(10)),
                Eq(std::make_tuple(build_bitset(1023), 5u)));
}

TEST(BitsetOpsTest, test_minimize_mantissa)
{
    EXPECT_THAT(minimize_mantissa(bitset(std::string("011000")), 3),
                Eq(std::make_tuple(bitset(std::string("011000")), 3)));
    EXPECT_THAT(minimize_mantissa(bitset(std::string("0100")), -3),
                Eq(std::make_tuple(bitset(std::string("0001")), -1)));
    EXPECT_THAT(minimize_mantissa(bitset(std::string("110000")), -3),
                Eq(std::make_tuple(bitset(std::string("000110")), 0)));
}

TEST(BitsetOpsTest, test_remove_fraction)
{
    EXPECT_THAT(remove_fraction(build_bitset(2), -4),
                Eq(std::make_tuple(build_bitset((1*5*5*5*5) * 2), 0, -4)));
    EXPECT_THAT(remove_fraction(build_bitset(3), 0),
                Eq(std::make_tuple(build_bitset(3), 0, 0)));
    EXPECT_THAT(remove_fraction(build_bitset(4), 6),
                Eq(std::make_tuple(build_bitset(4), 6, 0)));
}

TEST(BitsetOpsTest, test_reduce_binary_exponent)
{
    EXPECT_THAT(reduce_binary_exponent(bitset(std::string("1100")), 4),
                Eq(bitset(std::string("11000000"))));
    EXPECT_THAT(reduce_binary_exponent(bitset(std::string("1100")), 1),
                Eq(bitset(std::string("11000"))));
    EXPECT_THAT(reduce_binary_exponent(bitset(std::string("110")), 0),
                Eq(bitset(std::string("110"))));
}
