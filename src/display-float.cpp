#undef _GLIBCXX_DEBUG
#include "config.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <boost/cstdfloat.hpp>
#include <boost/lexical_cast.hpp>

template <typename Float>
std::string as_hex(Float const& f)
{
    std::ostringstream result;
    auto float_size = sizeof(Float) == 16 ? 10u : sizeof(Float);
#ifdef BOOST_LITTLE_ENDIAN
    unsigned char const* x = reinterpret_cast<unsigned char const*>(&f) + float_size - 1;
    for (auto i = 0; i < float_size; ++i, --x)
#else
    unsigned char const* x = reinterpret_cast<unsigned char const*>(&f);
    for (auto i = 0; i < float_size; ++i, ++x)
#endif
    {
	result << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(*x);
    }
    return result.str();
}

int main()
{
    std::string value;
    while (std::getline(std::cin, value)) {
#ifdef BOOST_FLOAT80_C
        auto const f80 = boost::lexical_cast<boost::float80_t>(value);
	std::cout << value << " = " << as_hex(f80) << "\n";
#endif
#ifdef BOOST_FLOAT64_C
        auto const f64 = boost::lexical_cast<boost::float64_t>(value);
	std::cout << value << " = " << as_hex(f64) << "\n";
#endif
#ifdef BOOST_FLOAT32_C
	auto const f32 = boost::lexical_cast<boost::float32_t>(value);
	std::cout << value << " = 0x" << as_hex(f32)<< "\n";
#endif
    }
    return 0;
}
