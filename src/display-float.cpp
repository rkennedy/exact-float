#include "config.h"
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include "analyze-float.h"

template <typename T> bool
print_number(std::string const& arg)
{
    try {
        T const ld = boost::lexical_cast<T>(arg);
        std::cout << arg << " = " << exact(ld) << std::endl;
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
#ifdef BOOST_FLOAT80_C
        error |= print_number<boost::float80_t>(arg);
#endif
#ifdef BOOST_FLOAT64_C
        error |= print_number<boost::float64_t>(arg);
#endif
#ifdef BOOST_FLOAT32_C
        error |= print_number<boost::float32_t>(arg);
#endif
    }
    return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
