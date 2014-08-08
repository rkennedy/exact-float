#include "config.h"
#include <tuple>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/dynamic_bitset.hpp>
#include "analyze-float.h"
#include "src/analyze-float.cpp"

using ::testing::Eq;

int main(int argc, char* argv[])
{
    ::testing::InitGoogleMock(&argc, argv);
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
    EXPECT_THAT(info.exponent, Eq(0));
    EXPECT_THAT(info.mantissa, Eq(0u));
    EXPECT_THAT(info.number_type, Eq(zero));
}

TEST(FloatInfoTest, extended_one)
{
    FloatInfo<long double> const info(1.0);
    EXPECT_FALSE(info.negative);
    EXPECT_THAT(info.exponent, Eq(16383));
    EXPECT_THAT(info.mantissa, Eq(0x8000000000000000ull));
    EXPECT_THAT(info.number_type, Eq(normal));
}

TEST(FloatInfoTest, double_one)
{
    FloatInfo<double> const info(1.0);
    EXPECT_FALSE(info.negative);
    EXPECT_THAT(info.exponent, Eq(1023));
    EXPECT_THAT(info.mantissa, Eq(0u));
    EXPECT_THAT(info.number_type, Eq(normal));
}

TEST(FloatInfoTest, single_one)
{
    FloatInfo<float> const info(1.0);
    EXPECT_FALSE(info.negative);
    EXPECT_THAT(info.exponent, Eq(127));
    EXPECT_THAT(info.mantissa, Eq(0u));
    EXPECT_THAT(info.number_type, Eq(normal));
}

TEST(FloatInfoTest, negative_extended_one)
{
    FloatInfo<long double> const info(-1);
    EXPECT_TRUE(info.negative);
    EXPECT_THAT(info.exponent, Eq(16383));
    EXPECT_THAT(info.mantissa, Eq(0x8000000000000000ull));
    EXPECT_THAT(info.number_type, Eq(normal));
}

TEST(FloatInfoTest, negative_two_double)
{
    FloatInfo<double> const info(-2.0);
    EXPECT_TRUE(info.negative);
    EXPECT_THAT(info.exponent, Eq(1024));
    EXPECT_THAT(info.mantissa, Eq(0u));
    EXPECT_THAT(info.number_type, Eq(normal));
}

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
