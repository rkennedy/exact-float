#include <cstddef>
#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include "analyze-float.h"

template <typename T>
std::string
ExactFloatToStrEx(T const value, char decimal_point = '.', char thousands_sep = ' ')
{
	FloatInfo<T> const info(value);
	switch (info.number_type) {
		case normal:
		{
			uint64_t const full_mantissa = (static_cast<uint64_t>(1) << (float_traits<T>::mantissa_bits - 1 + float_traits<T>::implied_one)) | info.mantissa;
			return FloatingBinPointToDecStr(full_mantissa, info.exponent - float_traits<T>::exponent_bias - (float_traits<T>::mantissa_bits - 1 + float_traits<T>::implied_one), info.negative, decimal_point, thousands_sep);
		}
		case zero:
			return info.negative ? "- 0" : "+ 0";
		case denormal:
			// TODO!
			return FloatingBinPointToDecStr(info.mantissa, -float_traits<T>::exponent_bias - (float_traits<T>::mantissa_bits - 2), info.negative, decimal_point, thousands_sep);
		case indefinite:
			return "Indefinite";
		case infinity:
			return info.negative ? "- Infinity" : "+ Infinity";
		case quiet_nan:
			return (boost::format("QNaN(%d)") % info.mantissa).str();
		case signaling_nan:
			return (boost::format("SNaN(%d)") % info.mantissa).str();
		default:
			return "unknown-number-type";
	}
}

int
main(int argc, char const* argv[])
{
	if (argc < 2)
		return EXIT_FAILURE;
	std::string const arg = argv[1];
	bool error = false;
	try {
		long double const ld = boost::lexical_cast<long double>(arg);
		std::cout << arg << " = " << ExactFloatToStrEx(ld) << std::endl;
	} catch (boost::bad_lexical_cast const& e) {
		std::cout << arg << " doesn't look like an Extended." << std::endl;
		error = true;
	}
	try {
		double const d = boost::lexical_cast<double>(arg);
		std::cout << arg << " = " << ExactFloatToStrEx(d) << std::endl;
	} catch (boost::bad_lexical_cast const& e) {
		std::cout << arg << " doesn't look like a Double." << std::endl;
		error = true;
	}
	try {
		float const f = boost::lexical_cast<float>(arg);
		std::cout << arg << " = " << ExactFloatToStrEx(f) << std::endl;
	} catch (boost::bad_lexical_cast const& e) {
		std::cout << arg << " doesn't look like a Single." << std::endl;
		error = true;
	}
	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
