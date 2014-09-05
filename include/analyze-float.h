#ifndef ANALYZE_FLOAT_H
#define ANALYZE_FLOAT_H
#undef _GLIBCXX_DEBUG
#include <cstdint>
#include <limits>
#include <string>
#include <map>
#include <bitset>
#include <typeindex>
#include <boost/format.hpp>
#include <boost/cstdfloat.hpp>
#include <boost/multiprecision/cpp_int.hpp>

namespace mp = boost::multiprecision;

enum float_type { unknown, normal, zero, denormal, indefinite, infinity, quiet_nan, signaling_nan };

std::ostream& operator<<(std::ostream&, float_type);

struct float_traits
{
    unsigned bits;
    unsigned digits;
    bool implied_one;
    unsigned max_exponent;
    char const* article;
    char const* name;

    unsigned mantissa_bits() const {
        return digits - implied_one;
    }

    unsigned exponent_bits() const {
        return bits - 1 - mantissa_bits();
    }

    unsigned exponent_bias() const {
        return max_exponent - 1;
    }

    mp::cpp_int get_exponent(mp::cpp_int const& rec) const {
        auto const exponent_mask = (mp::cpp_int(1) << exponent_bits()) - 1;
        return (rec >> mantissa_bits()) & exponent_mask;
    }
    mp::cpp_int get_mantissa(mp::cpp_int const& rec) const {
        auto const mantissa_mask = (mp::cpp_int(1) << mantissa_bits()) - 1;
        return rec & mantissa_mask;
    }
};

extern std::map<std::type_index, float_traits> const float_trait_map;

template <typename Float>
mp::cpp_int
to_float_rec(Float const value)
{
    float_traits const& traits = float_trait_map.at(typeid(Float));
    mp::cpp_int result;
    auto const v = reinterpret_cast<unsigned char const*>(&value);
    // TODO: Handle endian variation
    for (auto i = 0u; i < traits.bits; ++i)
        if (v[i / CHAR_BIT] & (1u << (i % CHAR_BIT)))
            mp::bit_set(result, i);
    return result;
}

float_type
get_float_type(mp::cpp_int exponent, mp::cpp_int mantissa, std::type_index type);

struct FloatInfo
{
private:
    float_traits const& traits;
    mp::cpp_int rec;
public:
    bool negative;
    mp::cpp_int exponent;
    mp::cpp_int mantissa;
    float_type number_type;

    template <typename Float>
    explicit FloatInfo(Float const value):
        traits(float_trait_map.at(typeid(Float))),
        rec(to_float_rec(value)),
        negative(value < 0),
        exponent(traits.get_exponent(rec)),
        mantissa(traits.get_mantissa(rec)),
        number_type(get_float_type(exponent, mantissa, typeid(Float)))
    { }

    FloatInfo(bool negative, mp::cpp_int exponent, mp::cpp_int mantissa, float_type number_type, std::type_index type):
        traits(float_trait_map.at(type)),
        rec(), negative(negative), exponent(exponent), mantissa(mantissa), number_type(number_type)
    { }

    bool operator==(FloatInfo const& other) const;

    friend std::ostream& operator<<(std::ostream& os, FloatInfo const& info);
};

template <typename Float>
FloatInfo exact(Float f)
{
    return FloatInfo(f);
}

std::string
FloatingBinPointToDecStr(mp::cpp_int Value, int ValBinExp, bool negative, char decimal_point = '.', char thousands_sep = ' ');

#endif
