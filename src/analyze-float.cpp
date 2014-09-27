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
#include <boost/range/algorithm/merge.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/transformed.hpp>
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

namespace {
template <size_t... Indices>
struct indices
{
    using type = indices<Indices..., sizeof...(Indices)>;
};

template <size_t N>
struct build_indices
{
    using type = typename build_indices<N - 1>::type::type;
};

template <>
struct build_indices<0>
{
    using type = indices<>;
};

}

template <typename... T>
struct zip_iterator
{
    using value_type = std::tuple<typename T::value_type...>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = std::tuple<typename T::reference...>;
    using iterator_category = std::forward_iterator_tag;

    zip_iterator(T const&... iterators):
        m_iterators(iterators...)
    { }

    value_type operator*() const {
        return dereference(index_type{});
    }

    zip_iterator& operator++() {
        increment(index_type{});
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
    using index_type = typename build_indices<sizeof...(T)>::type;

    template <size_t... Indices>
    value_type dereference(indices<Indices...> const&) const {
        return std::make_tuple(*std::get<Indices>(m_iterators)...);
    }

    template <size_t... Indices>
    void increment(indices<Indices...> const&) {
        std::make_tuple(++std::get<Indices>(m_iterators)...);
    }

private:
    std::tuple<T...> m_iterators;
};

template <typename... T>
zip_iterator<T...> make_zip_iterator(T&&... args) {
    return zip_iterator<T...>(std::forward<T>(args)...);
}

template <typename... Ranges>
auto combine(Ranges&&... args) -> decltype(
    boost::make_iterator_range(
        make_zip_iterator(std::begin(args)...),
        make_zip_iterator(std::end(args)...))) {
    return boost::make_iterator_range(
        make_zip_iterator(std::begin(args)...),
        make_zip_iterator(std::end(args)...));
}

template <size_t I>
struct getter
{
    template <typename T>
    auto operator()(T&& arg) const -> decltype(std::get<I>(std::forward<T>(arg))) {
        return std::get<I>(std::forward<T>(arg));
    }
};

std::string insert_thousands(std::string const& pattern, char const separator, std::string const& subject)
{
    std::vector<std::tuple<unsigned, char>> separators;
    tail_repeater next_count{pattern};
    for (unsigned total{0}, x{static_cast<unsigned>(next_count())};
         total + x < subject.size();
         x = static_cast<unsigned>(next_count()))
    {
        total += x;
        separators.emplace_back(total, separator);
    }
    auto const chars = ::combine(
        boost::counting_range(0ul, subject.size()),
        subject | boost::adaptors::reversed);
    std::deque<std::tuple<unsigned, char>> pre_result;
    boost::merge(chars, separators,
                 std::front_inserter(pre_result));

    std::string result;
    boost::copy(pre_result | boost::adaptors::transformed(getter<1>()),
                std::back_inserter(result));
    return result;
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
    result << insert_thousands("\3", ',', boost::lexical_cast<std::string>(Man));
    if (!Remainder.is_zero() || os.flags() & os.showpoint) {
        result << std::use_facet<std::numpunct<char>>(os.getloc()).decimal_point();
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
