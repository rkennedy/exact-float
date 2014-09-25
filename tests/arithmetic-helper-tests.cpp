#include "config.h"
#include <iostream>
#include <tuple>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/utility/binary.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include "analyze-float.h"
#include "src/analyze-float.cpp"

using ::testing::Eq;

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

TEST(Thousands, basic_separator)
{
    EXPECT_THAT(insert_thousands("\3", ',', "1234567890"), std::string("1,234,567,890"));
}
