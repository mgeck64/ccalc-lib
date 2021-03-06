#ifndef COMPLEX_TYPE_HPP
#define COMPLEX_TYPE_HPP

#include "basics.hpp"
#include <boost/multiprecision/cpp_complex.hpp>
#include <boost/version.hpp>

namespace calc_val {

using complex_type = boost::multiprecision::cpp_complex_100;
static_assert(std::is_same_v<float_type, complex_type::value_type>);

static const complex_type c_pi = complex_type(pi, 0);
static const complex_type c_e = complex_type(e, 0);
static const complex_type i = complex_type(0, 1);



template <typename T>
constexpr auto is_complex_type() -> bool
{return std::is_same_v<std::decay_t<T>, complex_type>;}



// custom pow function provides more accurate results if the exponent argument
// is a whole real number (no fractional or imaginary part), or if the base
// argument is the e constant

auto pow(const complex_type& z, const complex_type& e) -> complex_type;

template <typename T>
auto pow(const complex_type& z, T e) -> std::enable_if_t<
    std::is_integral_v<T> && std::is_unsigned_v<T>, complex_type>;

template <typename T>
auto pow(const complex_type& z, T e) -> std::enable_if_t<
    std::is_integral_v<T> && std::is_signed_v<T>, complex_type>;



// need wrappers for the following functions so they match calc_parser::unary_fn:

auto arg_wrapper(const complex_type& z) -> complex_type;
auto norm_wrapper(const complex_type& z) -> complex_type;

} // namespace calc_val



// definitions------------------------------------------------------------------



namespace calc_val {

namespace helper { // implementation helpers; not meant for public use
    auto pow_uint_e(const complex_type& z, max_uint_type e) -> complex_type;
    auto pow_int_e(const complex_type& z, max_int_type e) -> complex_type;
}

template <typename T>
inline auto pow(const complex_type& z, T e) -> std::enable_if_t<
    std::is_integral_v<T> && std::is_unsigned_v<T>, complex_type>
{return helper::pow_uint_e(z, e);}

template <typename T>
inline auto pow(const complex_type& z, T e) -> std::enable_if_t<
    std::is_integral_v<T> && std::is_signed_v<T>, complex_type>
{return helper::pow_int_e(z, e);}

} // namespace calc_val

#endif // COMPLEX_TYPE_HPP
