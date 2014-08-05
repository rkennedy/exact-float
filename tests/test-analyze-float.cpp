#include "config.h"
#include <tuple>
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>
#include <boost/dynamic_bitset.hpp>
//#include <boost/tuple/tuple_comparison.hpp>
//#include <boost/tuple/tuple_io.hpp>
#include "tuple_io.h"
#include "analyze-float.h"
#include "src/analyze-float.cpp"

BOOST_AUTO_TEST_CASE(extended_zero)
{
    FloatInfo<long double> const info(0.0);
    BOOST_CHECK(!info.negative);
    BOOST_CHECK_EQUAL(info.exponent, 0);
    BOOST_CHECK_EQUAL(info.mantissa, 0u);
    BOOST_CHECK_EQUAL(info.number_type, zero);
}

BOOST_AUTO_TEST_CASE(double_zero)
{
    FloatInfo<double> const info(0.0);
    BOOST_CHECK(!info.negative);
    BOOST_CHECK_EQUAL(info.exponent, 0);
    BOOST_CHECK_EQUAL(info.mantissa, 0u);
    BOOST_CHECK_EQUAL(info.number_type, zero);
}

BOOST_AUTO_TEST_CASE(single_zero)
{
    FloatInfo<float> const info(0.0);
    BOOST_CHECK(!info.negative);
    BOOST_CHECK_EQUAL(info.exponent, 0);
    BOOST_CHECK_EQUAL(info.mantissa, 0u);
    BOOST_CHECK_EQUAL(info.number_type, zero);
}

BOOST_AUTO_TEST_CASE(extended_one)
{
    FloatInfo<long double> const info(1.0);
    BOOST_CHECK(!info.negative);
    BOOST_CHECK_EQUAL(info.exponent, 16383);
    BOOST_CHECK_EQUAL(info.mantissa, 0x8000000000000000ull);
    BOOST_CHECK_EQUAL(info.number_type, normal);
}

BOOST_AUTO_TEST_CASE(double_one)
{
    FloatInfo<double> const info(1.0);
    BOOST_CHECK(!info.negative);
    BOOST_CHECK_EQUAL(info.exponent, 1023);
    BOOST_CHECK_EQUAL(info.mantissa, 0u);
    BOOST_CHECK_EQUAL(info.number_type, normal);
}

BOOST_AUTO_TEST_CASE(single_one)
{
    FloatInfo<float> const info(1.0);
    BOOST_CHECK(!info.negative);
    BOOST_CHECK_EQUAL(info.exponent, 127);
    BOOST_CHECK_EQUAL(info.mantissa, 0u);
    BOOST_CHECK_EQUAL(info.number_type, normal);
}

BOOST_AUTO_TEST_CASE(negative_extended_one)
{
    FloatInfo<long double> const info(-1);
    BOOST_CHECK(info.negative);
    BOOST_CHECK_EQUAL(info.exponent, 16383);
    BOOST_CHECK_EQUAL(info.mantissa, 0x8000000000000000ull);
    BOOST_CHECK_EQUAL(info.number_type, normal);
}

BOOST_AUTO_TEST_CASE(negative_two_double)
{
    FloatInfo<double> const info(-2.0);
    BOOST_CHECK(info.negative);
    BOOST_CHECK_EQUAL(info.exponent, 1024);
    BOOST_CHECK_EQUAL(info.mantissa, 0u);
    BOOST_CHECK_EQUAL(info.number_type, normal);
}

BOOST_AUTO_TEST_CASE(test_find_last)
{
    bitset empty_bitset;
    BOOST_CHECK_EQUAL(find_last(empty_bitset), bitset::size_type(-1));

    bitset one_bit_set(std::string("100"));
    BOOST_CHECK_EQUAL(find_last(one_bit_set), 2u);

    bitset many_bits_set(std::string("1111111111111111111111111111111111"));
    BOOST_CHECK_EQUAL(find_last(many_bits_set), 33u);
}

BOOST_AUTO_TEST_CASE(test_build_bitset)
{
    BOOST_CHECK_EQUAL(bitset(), build_bitset(0, 0));

    BOOST_CHECK_EQUAL(bitset(std::string("1010101010101010101010101010101010101010101010101010101010101010")), build_bitset(0xAAAAAAAAAAAAAAAAull, 64));
}

BOOST_AUTO_TEST_CASE(test_find_last_int)
{
    BOOST_CHECK_EQUAL(size_t(-1), find_last(0));
    BOOST_CHECK_EQUAL(size_t(0), find_last(1));
    BOOST_CHECK_EQUAL(size_t(4), find_last(20));
}

BOOST_AUTO_TEST_CASE(test_build_bitset_int)
{
    BOOST_CHECK_EQUAL(bitset(), build_bitset(0));
    BOOST_CHECK_EQUAL(bitset(std::string("1")), build_bitset(1));
    BOOST_CHECK_EQUAL(bitset(std::string("10100")), build_bitset(20));
}

BOOST_AUTO_TEST_CASE(test_one_bit_add)
{
    BOOST_CHECK_EQUAL(one_bit_add(false, false, false), std::make_tuple(false, false));
    BOOST_CHECK_EQUAL(one_bit_add(false, false, true), std::make_tuple(true, false));
    BOOST_CHECK_EQUAL(one_bit_add(false, true, false), std::make_tuple(true, false));
    BOOST_CHECK_EQUAL(one_bit_add(false, true, true), std::make_tuple(false, true));
    BOOST_CHECK_EQUAL(one_bit_add(true, false, false), std::make_tuple(true, false));
    BOOST_CHECK_EQUAL(one_bit_add(true, false, true), std::make_tuple(false, true));
    BOOST_CHECK_EQUAL(one_bit_add(true, true, false), std::make_tuple(false, true));
    BOOST_CHECK_EQUAL(one_bit_add(true, true, true), std::make_tuple(true, true));
}

BOOST_AUTO_TEST_CASE(test_ont_bit_subtract)
{
    BOOST_CHECK_EQUAL(one_bit_subtract(false, false, false), std::make_tuple(false, false));
    BOOST_CHECK_EQUAL(one_bit_subtract(false, false, true), std::make_tuple(true, true));
    BOOST_CHECK_EQUAL(one_bit_subtract(false, true, false), std::make_tuple(true, true));
    BOOST_CHECK_EQUAL(one_bit_subtract(false, true, true), std::make_tuple(false, true));
    BOOST_CHECK_EQUAL(one_bit_subtract(true, false, false), std::make_tuple(true, false));
    BOOST_CHECK_EQUAL(one_bit_subtract(true, false, true), std::make_tuple(false, false));
    BOOST_CHECK_EQUAL(one_bit_subtract(true, true, false), std::make_tuple(false, false));
    BOOST_CHECK_EQUAL(one_bit_subtract(true, true, true), std::make_tuple(true, true));
}

BOOST_AUTO_TEST_CASE(test_multiply)
{
    BOOST_CHECK_EQUAL(Multiply(build_bitset(10), build_bitset(50)), build_bitset(500));
    BOOST_CHECK_EQUAL(Multiply(build_bitset(50), build_bitset(2)), build_bitset(100));
    BOOST_CHECK_EQUAL(Multiply(build_bitset(0), build_bitset(1)), bitset());
    BOOST_CHECK_EQUAL(Multiply(build_bitset(0xffffffff), build_bitset(0xffffffff)), build_bitset(18446744065119617025u));
    BOOST_CHECK_EQUAL(Multiply(bitset(), bitset()), bitset());
}

BOOST_AUTO_TEST_CASE(test_subtract)
{
    BOOST_CHECK_EQUAL(Subtract(bitset(), bitset()), bitset());
    BOOST_CHECK_EQUAL(Subtract(build_bitset(500), build_bitset(3)), build_bitset(497));
    BOOST_CHECK_EQUAL(Subtract(build_bitset(15), build_bitset(15)), bitset());
}

BOOST_AUTO_TEST_CASE(test_bitset_ge)
{
    BOOST_CHECK_PREDICATE(std::greater_equal<bitset>(), (build_bitset(2))(build_bitset(1)));
    BOOST_CHECK_PREDICATE(std::greater_equal<bitset>(), (build_bitset(10))(build_bitset(9)));
    BOOST_CHECK_PREDICATE(std::greater_equal<bitset>(), (build_bitset(20))(build_bitset(2)));
    BOOST_CHECK_PREDICATE(std::greater_equal<bitset>(), (bitset(std::string("00100")))(bitset(std::string("100"))));
    BOOST_CHECK_PREDICATE(std::not2(std::greater_equal<bitset>()), (bitset())(build_bitset(2)));
}

BOOST_AUTO_TEST_CASE(test_divide)
{
    BOOST_CHECK_EQUAL(DivideAndRemainder(build_bitset(42), build_bitset(7)), std::make_tuple(build_bitset(6), 0u));

    BOOST_CHECK_EQUAL(DivideAndRemainder(bitset(), build_bitset(45)), std::make_tuple(bitset(), 0u));

    BOOST_CHECK_EQUAL(DivideAndRemainder(build_bitset(10235), build_bitset(10)), std::make_tuple(build_bitset(1023), 5u));
}

BOOST_AUTO_TEST_CASE(test_minimize_mantissa)
{
    BOOST_CHECK_EQUAL(minimize_mantissa(bitset(std::string("011000")), 3), std::make_tuple(bitset(std::string("011000")), 3));

    BOOST_CHECK_EQUAL(minimize_mantissa(bitset(std::string("0100")), -3), std::make_tuple(bitset(std::string("0001")), -1));

    BOOST_CHECK_EQUAL(minimize_mantissa(bitset(std::string("110000")), -3), std::make_tuple(bitset(std::string("000110")), 0));
}

BOOST_AUTO_TEST_CASE(test_remove_fraction)
{
    BOOST_CHECK_EQUAL(remove_fraction(build_bitset(2), -4), std::make_tuple(build_bitset((1*5*5*5*5) * 2), 0, -4));

    BOOST_CHECK_EQUAL(remove_fraction(build_bitset(3), 0), std::make_tuple(build_bitset(3), 0, 0));

    BOOST_CHECK_EQUAL(remove_fraction(build_bitset(4), 6), std::make_tuple(build_bitset(4), 6, 0));
}

BOOST_AUTO_TEST_CASE(test_reduce_binary_exponent)
{
    BOOST_CHECK_EQUAL(reduce_binary_exponent(bitset(std::string("1100")), 4), bitset(std::string("11000000")));
    BOOST_CHECK_EQUAL(reduce_binary_exponent(bitset(std::string("1100")), 1), bitset(std::string("11000")));
    BOOST_CHECK_EQUAL(reduce_binary_exponent(bitset(std::string("110")), 0), bitset(std::string("110")));
}
