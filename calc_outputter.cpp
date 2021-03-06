#include "calc_outputter.hpp"
#include <boost/io/ios_state.hpp>
#include <limits>
#include <cassert>
#include <boost/container/static_vector.hpp>

std::ostream& operator<<(std::ostream& out, const calc_outputter& outputter) {
    boost::io::ios_all_saver guard(out);
    out.unsetf(std::ios::basefield | std::ios::adjustfield | std::ios::floatfield); // put in known default state
    (outputter.*outputter.output_fn)(out);
    return out;
}

auto calc_outputter::output_fn_for(calc_val::radices radix) -> output_fn_type {
    switch (radix) {
        case calc_val::base2:
        case calc_val::base8:
        case calc_val::base16:
            return &calc_outputter::output_radix_pow2;
        default:
            assert(false); // missed one; fallthru to base10 case
        case calc_val::base10:
            return &calc_outputter::output_dec;
    }
}

auto calc_outputter::output_dec(std::ostream& out) const -> std::ostream& {
    if (out_options.precision == 0
        || out_options.precision > std::numeric_limits<calc_val::float_type>::digits10
        )
        out.precision(std::numeric_limits<calc_val::float_type>::digits10);
    else
        out.precision(out_options.precision);

    return std::visit([&](const auto& val) -> std::ostream& {
        using VT = std::decay_t<decltype(val)>;
        if constexpr (std::is_integral_v<VT>)
            // note: inserter for __int128 not available, thus this:
            output_dec_uint(out, [&]() -> std::make_unsigned_t<VT> {
                if (val < 0) {
                    out << '-';
                    return -val;
                }
                return val;
            }());
        else {
            static_assert(std::is_same_v<calc_val::complex_type, VT>);
            if (val.real() != 0 || val.imag() == 0)
                out << val.real();
            if (val.imag() != 0) {
                if (val.real() != 0 && !signbit(val.imag()))
                    out << '+';
                if (val.imag() == -1)
                    out << '-';
                else if (val.imag() != 1)
                    out << val.imag();
                out << 'i';
            }
        }
        return out;
    }, val);
}

auto calc_outputter::output_dec_uint(std::ostream& out, calc_val::max_uint_type val) const -> std::ostream& {
    boost::container::static_vector<char,
        std::numeric_limits<std::decay_t<decltype(val)>>::digits10 + 1> reversed;
    do {
        assert(reversed.size() < reversed.capacity());
        reversed.emplace_back(digits.at(val % 10));
        val /= 10;
    } while (val);
    for (auto itr = reversed.rbegin(); itr != reversed.rend(); ++itr)
        out << *itr;
    return out;
}

auto calc_outputter::output_radix_pow2(std::ostream& out) const -> std::ostream& {
    return std::visit([&](const auto& val) -> std::ostream& {
        using VT = std::decay_t<decltype(val)>;
        if constexpr (std::is_integral_v<VT> && std::is_signed_v<VT>) {
            output_radix_pow2_uint(out, [&]() -> std::make_unsigned_t<VT> {
                if (val < 0) {
                    out << '-';
                    return -val;
                }
                return val;
            }());
        } else if constexpr (std::is_integral_v<VT>)
            output_radix_pow2_uint(out, val);
        else {
            static_assert(std::is_same_v<calc_val::complex_type, VT>);
            if (val.real() != 0 || val.imag() == 0)
                output_radix_pow2_float(out, val.real());
            if (val.imag() != 0) {
                if (val.real() != 0 && !signbit(val.imag()))
                    out << '+';
                if (val.imag() == -1)
                    out << '-';
                else if (val.imag() != 1)
                    output_radix_pow2_float(out, val.imag());
                out << 'i';
            }
        }
        return out;
    }, val);
}

auto calc_outputter::output_radix_pow2_uint(std::ostream& out, calc_val::max_uint_type val) const -> std::ostream& {
    unsigned delimit_at;
    decltype(val) digit_mask;
    size_t digit_n_bits;
    if (out_options.output_radix == calc_val::base2) {
        delimit_at = 4;
        digit_mask = 1;
        digit_n_bits = 1;
    } else if (out_options.output_radix == calc_val::base8) {
        delimit_at = 3;
        digit_mask = 7;
        digit_n_bits = 3;
    } else { // assume base 16; output in other bases is unsupported
        assert(out_options.output_radix == calc_val::base16);
        delimit_at = 4;
        digit_mask = 15;
        digit_n_bits = 4;
    }

    std::vector<char> reversed;
    reversed.reserve(std::numeric_limits<std::decay_t<decltype(val)>>::digits / digit_n_bits + 1);

    do {
        reversed.emplace_back(digits.at(val & digit_mask));
        val >>= digit_n_bits;
    } while (val);

    unsigned digit_count = reversed.size();
    for (auto itr = reversed.rbegin(); itr != reversed.rend(); ++itr) {
        if (itr != reversed.rbegin() && !(--digit_count % delimit_at))
            out << ' ';
        out << *itr;
    }

    return out;
}

auto calc_outputter::output_radix_pow2_float(std::ostream& out, const calc_val::float_type& val) const -> std::ostream& {
    if (signbit(val))
        out << '-';

    if (isinf(val))
        return out << "inf";
    if (isnan(val))
        return out << "nan";
    if (iszero(val)) // can't be handled in general routine
        return out << "0";

    unsigned digit_mask;
    unsigned digit_n_bits;
    if (out_options.output_radix == calc_val::base2) {
        digit_mask = 1;
        digit_n_bits = 1;
    } else if (out_options.output_radix == calc_val::base8) {
        digit_mask = 7;
        digit_n_bits = 3;
    } else { // assume base 16; output in other bases is unsupported
        assert(out_options.output_radix == calc_val::base16);
        digit_mask = 15;
        digit_n_bits = 4;
    }

    using significand_type = boost::multiprecision::number<calc_val::float_type::backend_type::rep_type>;
    assert(std::numeric_limits<significand_type>::digits >= digit_n_bits);
    auto significand = significand_type();
    assert(significand == 0);
    auto exponent = val.backend().exponent();

    static_assert(!std::numeric_limits<decltype(out_options.precision)>::is_signed);
    auto precision = out_options.precision;
    if (precision == 0) {
        precision = std::numeric_limits<significand_type>::digits / digit_n_bits;
        if (std::numeric_limits<significand_type>::digits % digit_n_bits)
            ++precision;
    }
    if (precision * digit_n_bits >= std::numeric_limits<significand_type>::digits) {
        significand = val.backend().bits();
    } else { // handle rounding to precision
        auto f = calc_val::float_type();
        f.backend().bits() = val.backend().bits();
        f.backend().exponent() = 0;

        if (out_options.output_fp_normalized)
            f.backend().exponent() -= digit_n_bits;
        else
            f.backend().exponent() = f.backend().exponent() - (digit_n_bits - exponent % digit_n_bits);

        f.backend().exponent() += precision * digit_n_bits;
        auto e0 = f.backend().exponent();
        f += calc_val::float_type(out_options.output_radix / 2) / calc_val::float_type(unsigned(out_options.output_radix));
        f = trunc(f);
        significand = f.backend().bits();
        assert(!iszero(f) && !isinf(f) && !isnan(f));
        exponent += f.backend().exponent() - e0;
    }

    static_assert(!std::numeric_limits<significand_type>::is_signed);
    std::vector<char> reversed;
    reversed.reserve(std::numeric_limits<significand_type>::digits / digit_n_bits * 2); // should provide plenty of working space
    auto n_bits = std::numeric_limits<significand_type>::digits - 1; // exclude leading bit, which is handled specially

    if (!out_options.output_fp_normalized) {
        unsigned adjustment = exponent % digit_n_bits;
        n_bits -= adjustment;
        exponent -= adjustment;
    }
    if (auto shift = n_bits % digit_n_bits) { // partial digit
        auto shift2 = digit_n_bits - shift;
        auto digit = unsigned(significand & (digit_mask >> shift2)) << shift2;
        if (digit)
            reversed.emplace_back(digits.at(digit));
        significand >>= shift;
    }
    for (auto n = n_bits / digit_n_bits; n; --n) {
        auto digit = unsigned(significand & digit_mask);
        if (digit || !reversed.empty())
            reversed.emplace_back(digits.at(digit));
        significand >>= digit_n_bits;
    }

    auto make_signed = [](const auto& x) {return std::make_signed_t<std::decay_t<decltype(x)>>(x);};
    if (!out_options.output_fp_normalized
            && exponent >= -4 * make_signed(digit_n_bits) // -4 seems to match what gcc and boost does; don't know why this particular number
            && exponent < make_signed(precision) * make_signed(digit_n_bits)) {
        // output normally (not in scientific notation)
        auto exponent_ = exponent;
        if (exponent_ < 0) {
            out << "0.";
            while ((exponent_ += digit_n_bits) < 0)
                out << '0';
        }
        out << digits.at(unsigned(significand));
        for (auto itr = reversed.rbegin(); itr != reversed.rend(); ++itr) {
            if (exponent_ == 0 && exponent >= 0)
                out << '.';
            out << *itr;
            exponent_ -= digit_n_bits;
        }
        for (; exponent_ > 0; exponent_ -= digit_n_bits)
            out << '0';
    } else { // output in scientific notation
        out << digits.at(unsigned(significand));
        if (!reversed.empty()) {
            out << '.';
            for (auto itr = reversed.rbegin(); itr != reversed.rend(); ++itr)
                out << *itr;
        }
        out << 'p' << std::dec;
        if (exponent >= 0)
            out << '+';
        out << exponent;
    }

    return out;
}
