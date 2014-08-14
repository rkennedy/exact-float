#include "config.h"
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include "analyze-float.h"

template <typename T>
std::string
ExactFloatToStrEx(T const value, char decimal_point = '.', char thousands_sep = ' ')
{
    FloatInfo<T> const info(value);
    switch (info.number_type) {
        case normal: {
            std::uint64_t const full_mantissa = (static_cast<std::uint64_t>(1) << (float_traits<T>::mantissa_bits - 1 + float_traits<T>::implied_one)) | info.mantissa;
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

template <typename T> bool
print_number(std::string const& arg)
{
    try {
        T const ld = boost::lexical_cast<T>(arg);
        std::cout << arg << " = " << ExactFloatToStrEx(ld) << std::endl;
    } catch (boost::bad_lexical_cast const& e) {
        std::cout << boost::format("%s doesn't look like %s %s.") % arg % float_traits<T>::article % float_traits<T>::name <<std::endl;
        return true;
    }
    return false;
}

int
main(int argc, char const* argv[])
{
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "display program help")
        ("version", "display program version")
        ("number", po::value<std::vector<std::string>>()->composing(), "floating-point value to display")
        ;
    po::positional_options_description pd;
    pd.add("number", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(pd).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return EXIT_SUCCESS;
    }
    if (vm.count("version")) {
        std::cout << PACKAGE_STRING << std::endl;
        return EXIT_SUCCESS;
    }
    std::vector<std::string> const& args(vm["number"].as<std::vector<std::string>>());

    if (args.empty())
        return EXIT_FAILURE;
    bool error = false;
    for (auto arg: args) {
        error |= print_number<long double>(arg);
        error |= print_number<double>(arg);
        error |= print_number<float>(arg);
    }
    return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
