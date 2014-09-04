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

std::map<std::type_index, float_traits> const float_trait_map {
    { typeid(boost::float80_t), {
        80, std::numeric_limits<boost::float80_t>::digits, false, std::numeric_limits<boost::float80_t>::max_exponent, "an", "Extended"
    }},
    { typeid(boost::float64_t), {
        64, std::numeric_limits<boost::float64_t>::digits, true, std::numeric_limits<boost::float64_t>::max_exponent, "a", "Double"
    }},
    { typeid(boost::float32_t), {
        32, std::numeric_limits<boost::float32_t>::digits, true, std::numeric_limits<boost::float32_t>::max_exponent, "a", "Single"
    }},
    { typeid(void), {
        0, 0, false, 0, nullptr, nullptr
    }},
};

template <typename Float>
mp::cpp_int
to_float_rec(Float const value)
{
    mp::cpp_int result;
    auto const v = reinterpret_cast<unsigned char const*>(&value);
    // TODO: Handle endian variation
    for (auto i = 0u; i < float_trait_map.at(typeid(Float)).bits; ++i)
        if (v[i / CHAR_BIT] & (1u << (i % CHAR_BIT)))
            mp::bit_set(result, i);
    return result;
}

template <typename Float>
float_type
get_float_type(mp::cpp_int exponent, mp::cpp_int mantissa)
{
    float_traits const& traits = float_trait_map.at(typeid(Float));
    auto const exponent_mask = (mp::cpp_int(1) << traits.exponent_bits()) - 1;
    if (exponent == exponent_mask) {
        if (mantissa.is_zero())
            return infinity;
        bool const top_bit = mp::bit_test(mantissa, traits.mantissa_bits() - 1);
        if (traits.implied_one)
            return top_bit ? quiet_nan : signaling_nan;
        // From here down, we know it's a float80_t
        if (!top_bit)
            return signaling_nan;
        if (mp::bit_test(mantissa, traits.mantissa_bits() - 2))
            return mp::lsb(mantissa) < traits.mantissa_bits() - 2 ? quiet_nan : indefinite;
        return mp::lsb(mantissa) < traits.mantissa_bits() - 1 ? signaling_nan : infinity;
    } else if (exponent.is_zero())
        return mantissa.is_zero() ? zero : denormal;
    else
        return (!traits.implied_one && !mp::bit_test(mantissa, traits.mantissa_bits() - 1)) ? denormal : normal;
}

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
    FloatInfo(Float const value):
        traits(float_trait_map.at(typeid(Float))),
        rec(to_float_rec(value)),
        negative(value < 0),
        exponent(traits.get_exponent(rec)),
        mantissa(traits.get_mantissa(rec)),
        number_type(get_float_type<Float>(exponent, mantissa))
    { }

    FloatInfo(bool negative, mp::cpp_int exponent, mp::cpp_int mantissa, float_type number_type, std::type_index type):
        traits(float_trait_map.at(type)),
        rec(), negative(negative), exponent(exponent), mantissa(mantissa), number_type(number_type)
    { }

    bool operator==(FloatInfo const& other) const {
        return negative == other.negative
            && exponent == other.exponent
            && mantissa == other.mantissa
            && number_type == other.number_type;
    }

    friend std::ostream& operator<<(std::ostream& os, FloatInfo const& info);
};

template <typename Float>
FloatInfo exact(Float f)
{
    return FloatInfo(f);
}

std::string
FloatingBinPointToDecStr(mp::cpp_int Value, int ValBinExp, bool negative, char decimal_point = '.', char thousands_sep = ' ');

inline std::ostream&
operator<<(std::ostream& os, FloatInfo const& info)
{
    switch (info.number_type) {
        case normal: {
            unsigned const mantissa_offset = info.traits.mantissa_bits() - 1 + info.traits.implied_one;
            mp::cpp_int const full_mantissa = (mp::cpp_int(1) << mantissa_offset) | info.mantissa;
            mp::cpp_int const adjusted_exponent = info.exponent - info.traits.exponent_bias() - mantissa_offset;
            return os << FloatingBinPointToDecStr(full_mantissa, adjusted_exponent.convert_to<int>(), info.negative, '.', ' ');
        }
        case zero:
            return os << (info.negative ? "- 0" : "+ 0");
        case denormal:
            // TODO!
            return os << FloatingBinPointToDecStr(info.mantissa, -info.traits.exponent_bias() - (info.traits.mantissa_bits() - 2), info.negative, '.', ' ');
        case indefinite:
            return os << "Indefinite";
        case infinity:
            return os << (info.negative ? "- Infinity" : "+ Infinity");
        case quiet_nan:
            return os << boost::format("QNaN(%d)") % info.mantissa;
        case signaling_nan:
            return os << boost::format("SNaN(%d)") % info.mantissa;
        default:
            return os << "unknown-number-type";
    }
}
#endif
