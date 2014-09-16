#include "config.h"
#include <array>
#include <boost/cstdfloat.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/bitwise.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/insert.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/mpl/map.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/vector.hpp>

namespace detail
{

template <unsigned char B>
using uchar = boost::mpl::integral_c<unsigned char, B>;

struct invalid_char {};

template <char C>
using char_to_nibble =
    typename boost::mpl::eval_if_c<C == '0', uchar<0>,
    typename boost::mpl::eval_if_c<C == '1', uchar<1>,
    typename boost::mpl::eval_if_c<C == '2', uchar<2>,
    typename boost::mpl::eval_if_c<C == '3', uchar<3>,
    typename boost::mpl::eval_if_c<C == '4', uchar<4>,
    typename boost::mpl::eval_if_c<C == '5', uchar<5>,
    typename boost::mpl::eval_if_c<C == '6', uchar<6>,
    typename boost::mpl::eval_if_c<C == '7', uchar<7>,
    typename boost::mpl::eval_if_c<C == '8', uchar<8>,
    typename boost::mpl::eval_if_c<C == '9', uchar<9>,
    typename boost::mpl::eval_if_c<C == 'a' || C == 'A', uchar<10>,
    typename boost::mpl::eval_if_c<C == 'b' || C == 'B', uchar<11>,
    typename boost::mpl::eval_if_c<C == 'c' || C == 'C', uchar<12>,
    typename boost::mpl::eval_if_c<C == 'd' || C == 'D', uchar<13>,
    typename boost::mpl::eval_if_c<C == 'e' || C == 'E', uchar<14>,
    typename boost::mpl::eval_if_c<C == 'f' || C == 'F', uchar<15>,
    invalid_char>>>>>>>>>>>>>>>>::type;

template <char... Chars>
struct build_float;

template <char C1, char C2, char... Chars>
struct build_float<C1, C2, Chars...>
{
    using type = typename boost::mpl::push_back<
        typename build_float<Chars...>::type,
        typename boost::mpl::bitor_<
            typename boost::mpl::shift_left<
                char_to_nibble<C1>,
                boost::mpl::int_<4>
            >::type,
            char_to_nibble<C2>
        >::type
    >::type;
};

template <>
struct build_float<>
{
    using type = boost::mpl::vector<>;
};

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

#ifdef BOOST_FLOAT80_C
using m2 = boost::mpl::insert<boost::mpl::map<>, boost::mpl::pair<boost::mpl::long_<80>, boost::float80_t>>::type;
#else
using m2 = boost::mpl::map<>;
#endif
#ifdef BOOST_FLOAT64_C
using m1 = boost::mpl::insert<m2, boost::mpl::pair<boost::mpl::long_<64>, boost::float64_t>>::type;
#else
using m1 = m2;
#endif
#ifdef BOOST_FLOAT32_C
using m = boost::mpl::insert<m1, boost::mpl::pair<boost::mpl::long_<32>, boost::float32_t>>::type;
#else
using m = m1;
#endif

template <size_t Bits>
union hex_float
{
    using array_type = std::array<unsigned char, Bits / CHAR_BIT>;

    typename boost::mpl::at_c<m, Bits>::type f;
    array_type b;

    constexpr hex_float(array_type const& b):
        b(b)
    { }
};

template <char Zero, char X, char... Chars>
struct to_float
{
private:
    static_assert(Zero == '0', "Hex float literal must start with \"0x\"");
    static_assert(X == 'x' || X == 'X', "Hex float literal must start with \"0x\"");
    //static_assert(sizeof...(Chars) % 2 == 0, "Hex float literal must have even number of characters");
    using type = typename build_float<Chars...>::type;
    static constexpr size_t length = boost::mpl::size<type>::value;
    static_assert(length == sizeof...(Chars) / 2, "Length mismatch");
    using array_type = typename hex_float<length * CHAR_BIT>::array_type;

    template <size_t... Indices>
    static constexpr array_type make(indices<Indices...> const&) {
        static_assert(length == sizeof...(Indices), "Length mismatch");
        return array_type{{
            boost::mpl::at_c<type, Indices>::type::value...
        }};
    }
public:
    static constexpr std::array<unsigned char, length> as_array() {
        return make(typename build_indices<length>::type{});
    }
};

} // namespace detail

// sizeof...(Chars) - 2 is the number of characters in the hexadecimal portion
// of the literal. The extra two are the "0x" prefix.
template <char... Chars> constexpr
typename boost::enable_if<
    boost::mpl::has_key<
        detail::m,
        boost::mpl::long_<(sizeof...(Chars) - 2) * CHAR_BIT / 2>
    >,
    typename boost::mpl::at<
        detail::m,
        boost::mpl::long_<(sizeof...(Chars) - 2) * CHAR_BIT / 2>
    >::type
>::type operator"" _float()
{
    return detail::hex_float<(sizeof...(Chars) - 2) / 2 * CHAR_BIT>{
        detail::to_float<Chars...>::as_array()
    }.f;
}
