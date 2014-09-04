#include "config.h"
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include "analyze-float.h"

struct print_number
{
private:
    std::string const& m_arg;
    bool& m_error;
public:
    print_number(std::string const& arg, bool& error):
        m_arg(arg), m_error(error)
    { }

    void operator()(int) { }

    template <typename T>
    void operator()(T)
    {
        try {
            T const ld = boost::lexical_cast<T>(m_arg);
            std::cout << m_arg << " = " << exact(ld) << std::endl;
        } catch (boost::bad_lexical_cast const& e) {
            float_traits const& traits = float_trait_map.at(typeid(T));
            std::cout << boost::format("%s doesn't look like %s %s.") % m_arg % traits.article % traits.name << std::endl;
            m_error |= true;
        }
    }
};

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
        boost::mpl::for_each<boost::mpl::vector<
#ifdef BOOST_FLOAT80_C
            boost::float80_t,
#endif
#ifdef BOOST_FLOAT64_C
            boost::float64_t,
#endif
#ifdef BOOST_FLOAT32_C
            boost::float32_t,
#endif
            int
        >::type>(print_number(arg, error));
    }
    return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
