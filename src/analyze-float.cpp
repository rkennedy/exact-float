#undef _GLIBCXX_DEBUG
#include "config.h"
#include <cstdint>
#include <ios>
#include <iomanip>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include <deque>
#include <iterator>
#include <locale>
#include <sstream>
#include <type_traits>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/range/counting_range.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/adaptor/map.hpp>
#include "analyze-float.h"

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

template <typename I1, typename I2>
struct zip_iterator
{
    using value_type = std::pair<typename I1::value_type, typename I2::value_type>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = std::pair<typename I1::value_type, typename I2::value_type>;
    using iterator_category = std::forward_iterator_tag;

    zip_iterator(I1 const& iterator1, I2 const& iterator2):
        m_iterators(iterator1, iterator2)
    { }

    value_type operator*() const {
        return value_type(*m_iterators.first, *m_iterators.second);
    }

    zip_iterator& operator++() {
        ++m_iterators.first;
        ++m_iterators.second;
        return *this;
    }

    zip_iterator operator++(int) {
        zip_iterator other(*this);
        ++*this;
        return other;
    }

    bool operator==(zip_iterator const& other) const {
        return other.m_iterators == m_iterators;
    }

    bool operator!=(zip_iterator const& other) const {
        return other.m_iterators != m_iterators;
    }

private:
    std::pair<I1, I2> m_iterators;
};

template <typename I1, typename I2>
zip_iterator<I1, I2> make_zip_iterator(I1&& i1, I2&& i2) {
    return zip_iterator<I1, I2>(std::forward<I1>(i1), std::forward<I2>(i2));
}

template <typename Range1, typename Range2>
auto combine(Range1&& arg1, Range2&& arg2) -> decltype(
    boost::make_iterator_range(
        make_zip_iterator(std::begin(arg1), std::begin(arg2)),
        make_zip_iterator(std::end(arg1), std::end(arg2)))) {
    return boost::make_iterator_range(
        make_zip_iterator(std::begin(arg1), std::begin(arg2)),
        make_zip_iterator(std::end(arg1), std::end(arg2)));
}

void insert_thousands(std::ostream& os, std::string const& pattern, char const separator, std::string const& subject)
{
    std::multimap<unsigned, char> result;
    if (!pattern.empty()) {
        tail_repeater next_count{pattern};
        char x = next_count();
        for (unsigned total{0};
             x > 0 && x != CHAR_MAX && total + x < subject.size();
             x = next_count())
        {
            total += x;
            result.emplace(subject.size() - total, separator);
        }
    }
    auto const chars = ::combine(
        boost::counting_range(0ul, subject.size()),
        subject);
    result.insert(std::begin(chars), std::end(chars));
    boost::copy(result | boost::adaptors::map_values,
                std::ostream_iterator<char>(os));
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
    char const sign = negative ? '-' : '+';
    if (negative || os.flags() & os.showpos)
        result << sign;
    std::numpunct<char> const& punct = std::use_facet<std::numpunct<char>>(os.getloc());
    insert_thousands(result, punct.grouping(), punct.thousands_sep(), boost::lexical_cast<std::string>(Man));
    if (!Remainder.is_zero() || os.flags() & os.showpoint) {
        result << punct.decimal_point();
        result << std::setw(-DecExp) << std::setfill('0') << Remainder;
    }
    os << result.str();
}

/**
 * Reduce mantissa to minimum number of bits
 * That is, while mantissa is even, divide by 2 and increment binary
 * exponent. Stop if the exponent isn't negative. */
std::tuple<mp::cpp_int, int /*BinExp*/>
minimize_mantissa(mp::cpp_int const& Man, int const BinExp)
{
    assert(Man != 0);
    auto const idx = mp::lsb(Man);
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
    { typeid(boost::float80_t), {
        80, std::numeric_limits<boost::float80_t>::digits, false, std::numeric_limits<boost::float80_t>::max_exponent, "an", "Extended"
    }},
    { typeid(boost::float64_t), {
        64, std::numeric_limits<boost::float64_t>::digits, true, std::numeric_limits<boost::float64_t>::max_exponent, "a", "Double"
    }},
    { typeid(boost::float32_t), {
        32, std::numeric_limits<boost::float32_t>::digits, true, std::numeric_limits<boost::float32_t>::max_exponent, "a", "Single"
    }},
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
    assert(Man != 0);

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
            return os << (info.negative ? "- 0" : "+ 0");
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
