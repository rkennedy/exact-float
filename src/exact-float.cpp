#undef _GLIBCXX_DEBUG
#include "config.h"
#include <cstdint>
#include <iomanip>
#include <ios>
#include <iostream>
#include <limits>
#include <locale>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <boost/multiprecision/cpp_int.hpp>
#include "exact-float.h"

namespace mp = boost::multiprecision;

std::ostream& operator<<(std::ostream& os, float_type const type)
{
    switch (type)
    {
        case unknown:
            return os << "unknown";
        case normal:
            return os << "normal";
        case zero:
            return os << "zero";
        case denormal:
            return os << "denormal";
        case indefinite:
            return os << "indefinite";
        case infinity:
            return os << "infinity";
        case quiet_nan:
            return os << "quiet_nan";
        case signaling_nan:
            return os << "signaling_nan";
        default:
            assert(false);
    }
}

namespace {

struct tail_repeater
{
    tail_repeater(std::string const& source):
        m_source(source),
        m_iterator(m_source.begin())
    { }
    char operator()() {
        if (m_iterator == m_source.end())
            return m_source.at(m_source.size() - 1);
        return *m_iterator++;
    }
private:
    std::string m_source;
    std::string::const_iterator m_iterator;
};

void insert_thousands(std::ostream& os, std::string const& pattern, char const separator, std::string subject)
{
    if (!pattern.empty()) {
        size_t const subject_size{subject.size()};
        tail_repeater next_count{pattern};
        char x = next_count();
        for (unsigned total{0};
             x > 0 && x != CHAR_MAX && total + x < subject_size;
             x = next_count())
        {
            total += x;
            subject.insert(subject_size - total, 1, separator);
        }
    }
    os << subject;
}

// Print Man * 10^DecExp
void
build_result(std::ostream& os, int DecExp, mp::cpp_int Man, bool negative)
{
    mp::cpp_int const Factor = DecExp < 0
        ? mp::pow(mp::cpp_int(10), -DecExp)
        : mp::cpp_int(1);
    mp::cpp_int Remainder;
    mp::divide_qr(Man, Factor, Man, Remainder);

    std::ostringstream result;
    std::numpunct<char> const& punct = std::use_facet<std::numpunct<char>>(os.getloc());
    insert_thousands(result, punct.grouping(), punct.thousands_sep(), boost::lexical_cast<std::string>(Man));
    auto const flags = os.flags();
    if (!Remainder.is_zero() || flags & os.showpoint) {
        result << punct.decimal_point();
        result << std::setw(-DecExp) << std::setfill('0') << Remainder;
    }

    size_t const target_width = os.width(0);
    bool const include_sign{negative || flags & os.showpos};
    size_t const result_width = result.str().size() + include_sign;
    size_t const padding_width = result_width >= target_width ? 0 : target_width - result_width;
    if ((flags & os.adjustfield) == 0 || flags & os.right)
        os << std::string(padding_width, os.fill());
    char const sign = negative ? '-' : '+';
    if (include_sign)
        os << sign;
    if (flags & os.internal)
        os << std::string(padding_width, os.fill());
    os << result.str();
    if (flags & os.left)
        os << std::string(padding_width, os.fill());
}

/**
 * Reduce mantissa to minimum number of bits
 * That is, while mantissa is even, divide by 2 and increment binary
 * exponent. Stop if the exponent isn't negative. */
std::tuple<mp::cpp_int, int /*BinExp*/>
minimize_mantissa(mp::cpp_int const& Man, int const BinExp)
{
    auto const idx = Man.is_zero() ? 0 : mp::lsb(Man);
    int const adjustment = std::min<int>(idx, -BinExp);
    if (adjustment <= 0)
        return std::make_tuple(Man, BinExp);
    return std::make_tuple(Man >> adjustment, BinExp + adjustment);
}

/**
 * Repeatedly multiply by 10 until there is no more fraction. Decrement
 * the DecExp at the same time. Note that a multiply by 10 is the same
 * as multiply by 5 and increment of the BinExp exponent. Also note
 * that a multiply by 5 adds two or three bits to the number of
 * mantissa bits. */
std::tuple<mp::cpp_int, int /*BinExp*/, int/*DecExp*/>
remove_fraction(mp::cpp_int Man, int const BinExp)
{
    if (BinExp >= 0)
        return std::make_tuple(Man, BinExp, 0);

    return std::make_tuple(Man * mp::pow(mp::cpp_int(5), -BinExp), 0, BinExp);
}

// Finish reducing BinExp to 0 by shifting mantissa up
mp::cpp_int
reduce_binary_exponent(mp::cpp_int Man, int BinExp)
{
    assert(BinExp >= 0);
    return Man << BinExp;
}

} // namespace

std::map<std::type_index, float_traits> const float_trait_map {
#ifdef BOOST_FLOAT80_C
    { typeid(boost::float80_t), {
        80, std::numeric_limits<boost::float80_t>::digits, false, std::numeric_limits<boost::float80_t>::max_exponent, "an", "Extended"
    }},
#endif
#ifdef BOOST_FLOAT64_C
    { typeid(boost::float64_t), {
        64, std::numeric_limits<boost::float64_t>::digits, true, std::numeric_limits<boost::float64_t>::max_exponent, "a", "Double"
    }},
#endif
#ifdef BOOST_FLOAT32_C
    { typeid(boost::float32_t), {
        32, std::numeric_limits<boost::float32_t>::digits, true, std::numeric_limits<boost::float32_t>::max_exponent, "a", "Single"
    }},
#endif
};

float_type
get_float_type(mp::cpp_int exponent, mp::cpp_int mantissa, std::type_index type)
{
    float_traits const& traits = float_trait_map.at(type);
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

bool FloatInfo::operator==(FloatInfo const& other) const
{
    return negative == other.negative
        && exponent == other.exponent
        && mantissa == other.mantissa
        && number_type == other.number_type;
}

// Value = Mantissa * 2^BinExp * 10^DecExp
void
FloatingBinPointToDecStr(std::ostream& os, mp::cpp_int Value, int BinExp, bool negative)
{
    mp::cpp_int Man = Value;

    std::tie(Man, BinExp) = minimize_mantissa(Man, BinExp);

    int DecExp;
    std::tie(Man, BinExp, DecExp) = remove_fraction(Man, BinExp);
    assert(DecExp <= 0);

    Man = reduce_binary_exponent(Man, BinExp);

    build_result(os, DecExp, Man, negative);
}

std::ostream&
operator<<(std::ostream& os, FloatInfo const& info)
{
    switch (info.number_type) {
        case normal: {
            unsigned const mantissa_offset = info.traits.mantissa_bits() - 1 + info.traits.implied_one;
            mp::cpp_int const full_mantissa = (mp::cpp_int(1) << mantissa_offset) | info.mantissa;
            mp::cpp_int const adjusted_exponent = info.exponent - info.traits.exponent_bias() - mantissa_offset;
            FloatingBinPointToDecStr(os, full_mantissa, adjusted_exponent.convert_to<int>(), info.negative);
            return os;
        }
        case zero:
            FloatingBinPointToDecStr(os, 0, 1, info.negative);
            return os;
        case denormal:
            // TODO!
            FloatingBinPointToDecStr(os, info.mantissa, -info.traits.exponent_bias() - (info.traits.mantissa_bits() - 2), info.negative);
            return os;
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
