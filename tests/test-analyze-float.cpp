#include "config.h"
#include <tuple>
#include <gtest/gtest.h>
#include <boost/dynamic_bitset.hpp>
#include "analyze-float.h"
#include "src/analyze-float.cpp"

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

template <typename T>
class CommonFloatInfoTest: public ::testing::Test
{
};

typedef ::testing::Types<long double, double, float> FloatTypes;
TYPED_TEST_CASE(CommonFloatInfoTest, FloatTypes);

TYPED_TEST(CommonFloatInfoTest, Zero)
{
    FloatInfo<TypeParam> const info(0.0);
    EXPECT_FALSE(info.negative);
    EXPECT_EQ(info.exponent, 0);
    EXPECT_EQ(info.mantissa, 0u);
    EXPECT_EQ(info.number_type, zero);
}

TEST(FloatInfoTest, extended_one)
{
    FloatInfo<long double> const info(1.0);
    EXPECT_FALSE(info.negative);
    EXPECT_EQ(info.exponent, 16383);
    EXPECT_EQ(info.mantissa, 0x8000000000000000ull);
    EXPECT_EQ(info.number_type, normal);
}

TEST(FloatInfoTest, double_one)
{
    FloatInfo<double> const info(1.0);
    EXPECT_FALSE(info.negative);
    EXPECT_EQ(info.exponent, 1023);
    EXPECT_EQ(info.mantissa, 0u);
    EXPECT_EQ(info.number_type, normal);
}

TEST(FloatInfoTest, single_one)
{
    FloatInfo<float> const info(1.0);
    EXPECT_FALSE(info.negative);
    EXPECT_EQ(info.exponent, 127);
    EXPECT_EQ(info.mantissa, 0u);
    EXPECT_EQ(info.number_type, normal);
}

TEST(FloatInfoTest, negative_extended_one)
{
    FloatInfo<long double> const info(-1);
    EXPECT_TRUE(info.negative);
    EXPECT_EQ(info.exponent, 16383);
    EXPECT_EQ(info.mantissa, 0x8000000000000000ull);
    EXPECT_EQ(info.number_type, normal);
}

TEST(FloatInfoTest, negative_two_double)
{
    FloatInfo<double> const info(-2.0);
    EXPECT_TRUE(info.negative);
    EXPECT_EQ(info.exponent, 1024);
    EXPECT_EQ(info.mantissa, 0u);
    EXPECT_EQ(info.number_type, normal);
}

TEST(BitsetOpsTest, test_find_last)
{
    bitset empty_bitset;
    EXPECT_EQ(find_last(empty_bitset), bitset::size_type(-1));

    bitset one_bit_set(std::string("100"));
    EXPECT_EQ(find_last(one_bit_set), 2u);

    bitset many_bits_set(std::string("1111111111111111111111111111111111"));
    EXPECT_EQ(find_last(many_bits_set), 33u);
}

TEST(BitsetOpsTest, test_build_bitset)
{
    EXPECT_EQ(bitset(), build_bitset(0, 0));

    EXPECT_EQ(bitset(std::string("1010101010101010101010101010101010101010101010101010101010101010")), build_bitset(0xAAAAAAAAAAAAAAAAull, 64));
}

TEST(BitsetOpsTest, test_find_last_int)
{
    EXPECT_EQ(size_t(-1), find_last(0));
    EXPECT_EQ(size_t(0), find_last(1));
    EXPECT_EQ(size_t(4), find_last(20));
}

TEST(BitsetOpsTest, test_build_bitset_int)
{
    EXPECT_EQ(bitset(), build_bitset(0));
    EXPECT_EQ(bitset(std::string("1")), build_bitset(1));
    EXPECT_EQ(bitset(std::string("10100")), build_bitset(20));
}

TEST(BitsetOpsTest, test_one_bit_add)
{
    EXPECT_EQ(one_bit_add(false, false, false), std::make_tuple(false, false));
    EXPECT_EQ(one_bit_add(false, false, true), std::make_tuple(true, false));
    EXPECT_EQ(one_bit_add(false, true, false), std::make_tuple(true, false));
    EXPECT_EQ(one_bit_add(false, true, true), std::make_tuple(false, true));
    EXPECT_EQ(one_bit_add(true, false, false), std::make_tuple(true, false));
    EXPECT_EQ(one_bit_add(true, false, true), std::make_tuple(false, true));
    EXPECT_EQ(one_bit_add(true, true, false), std::make_tuple(false, true));
    EXPECT_EQ(one_bit_add(true, true, true), std::make_tuple(true, true));
}

TEST(BitsetOpsTest, test_ont_bit_subtract)
{
    EXPECT_EQ(one_bit_subtract(false, false, false), std::make_tuple(false, false));
    EXPECT_EQ(one_bit_subtract(false, false, true), std::make_tuple(true, true));
    EXPECT_EQ(one_bit_subtract(false, true, false), std::make_tuple(true, true));
    EXPECT_EQ(one_bit_subtract(false, true, true), std::make_tuple(false, true));
    EXPECT_EQ(one_bit_subtract(true, false, false), std::make_tuple(true, false));
    EXPECT_EQ(one_bit_subtract(true, false, true), std::make_tuple(false, false));
    EXPECT_EQ(one_bit_subtract(true, true, false), std::make_tuple(false, false));
    EXPECT_EQ(one_bit_subtract(true, true, true), std::make_tuple(true, true));
}

TEST(BitsetOpsTest, test_multiply)
{
    EXPECT_EQ(Multiply(build_bitset(10), build_bitset(50)), build_bitset(500));
    EXPECT_EQ(Multiply(build_bitset(50), build_bitset(2)), build_bitset(100));
    EXPECT_EQ(Multiply(build_bitset(0), build_bitset(1)), bitset());
    EXPECT_EQ(Multiply(build_bitset(0xffffffff), build_bitset(0xffffffff)), build_bitset(18446744065119617025u));
    EXPECT_EQ(Multiply(bitset(), bitset()), bitset());
}

TEST(BitsetOpsTest, test_subtract)
{
    EXPECT_EQ(Subtract(bitset(), bitset()), bitset());
    EXPECT_EQ(Subtract(build_bitset(500), build_bitset(3)), build_bitset(497));
    EXPECT_EQ(Subtract(build_bitset(15), build_bitset(15)), bitset());
}

TEST(BitsetOpsTest, test_bitset_ge)
{
    EXPECT_PRED2(std::greater_equal<bitset>(), build_bitset(2), build_bitset(1));
    EXPECT_PRED2(std::greater_equal<bitset>(), build_bitset(10), build_bitset(9));
    EXPECT_PRED2(std::greater_equal<bitset>(), build_bitset(20), build_bitset(2));
    EXPECT_PRED2(std::greater_equal<bitset>(), bitset(std::string("00100")), bitset(std::string("100")));
    EXPECT_PRED2(std::not2(std::greater_equal<bitset>()), bitset(), build_bitset(2));
}

TEST(BitsetOpsTest, test_divide)
{
    EXPECT_EQ(DivideAndRemainder(build_bitset(42), build_bitset(7)), std::make_tuple(build_bitset(6), 0u));

    EXPECT_EQ(DivideAndRemainder(bitset(), build_bitset(45)), std::make_tuple(bitset(), 0u));

    EXPECT_EQ(DivideAndRemainder(build_bitset(10235), build_bitset(10)), std::make_tuple(build_bitset(1023), 5u));
}

TEST(BitsetOpsTest, test_minimize_mantissa)
{
    EXPECT_EQ(minimize_mantissa(bitset(std::string("011000")), 3), std::make_tuple(bitset(std::string("011000")), 3));

    EXPECT_EQ(minimize_mantissa(bitset(std::string("0100")), -3), std::make_tuple(bitset(std::string("0001")), -1));

    EXPECT_EQ(minimize_mantissa(bitset(std::string("110000")), -3), std::make_tuple(bitset(std::string("000110")), 0));
}

TEST(BitsetOpsTest, test_remove_fraction)
{
    EXPECT_EQ(remove_fraction(build_bitset(2), -4), std::make_tuple(build_bitset((1*5*5*5*5) * 2), 0, -4));

    EXPECT_EQ(remove_fraction(build_bitset(3), 0), std::make_tuple(build_bitset(3), 0, 0));

    EXPECT_EQ(remove_fraction(build_bitset(4), 6), std::make_tuple(build_bitset(4), 6, 0));
}

TEST(BitsetOpsTest, test_reduce_binary_exponent)
{
    EXPECT_EQ(reduce_binary_exponent(bitset(std::string("1100")), 4), bitset(std::string("11000000")));
    EXPECT_EQ(reduce_binary_exponent(bitset(std::string("1100")), 1), bitset(std::string("11000")));
    EXPECT_EQ(reduce_binary_exponent(bitset(std::string("110")), 0), bitset(std::string("110")));
}
