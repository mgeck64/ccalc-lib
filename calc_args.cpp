#include "calc_args.hpp"
#include "const_string_itr.hpp"
#include <cctype>
#include <charconv>

static bool single_flag_option(const_string_itr arg_itr, calc_args& args);
static bool double_flag_option(const_string_itr arg_itr, calc_args& args);

void interpret_arg(std::string_view arg_str, char option_code, calc_args& args) {
    auto arg_itr = const_string_itr(arg_str);
    if (arg_itr && *arg_itr == option_code) {
        if (single_flag_option(++arg_itr, args))
            return;
        if (arg_itr && *arg_itr == option_code)
            if (double_flag_option(++arg_itr, args))
                return;
    }
    args.other_args = true;
}

static bool single_flag_option(const_string_itr arg_itr, calc_args& args) {
    if (arg_itr.length() > 1 && *arg_itr == 'p' && arg_itr[1] == 'r') {
        decltype(args.precision) precision = 0;
        std::from_chars_result r = std::from_chars(arg_itr.begin() + 2, arg_itr.end(), precision);
        if (r.ec == std::errc() && r.ptr == arg_itr.end())
            args.precision = precision;
        ++args.n_precision_options;
        return true; // true means handled, not necessarily successfully
    }

    auto arg_view = arg_itr.view();
    if (arg_view == "h") {
        ++args.n_help_options;
        return true;
    }
    if (arg_view == "w8") {
        args.int_word_size = calc_val::int_bits_8;
        ++args.n_int_word_size_options;
        return true;
    }
    if (arg_view == "w16") {
        args.int_word_size = calc_val::int_bits_16;
        ++args.n_int_word_size_options;
        return true;
    }
    if (arg_view == "w32") {
        args.int_word_size = calc_val::int_bits_32;
        ++args.n_int_word_size_options;
        return true;
    }
    if (arg_view == "w64") {
        args.int_word_size = calc_val::int_bits_64;
        ++args.n_int_word_size_options;
        return true;
    }
    if (arg_view == "w128") {
        args.int_word_size = calc_val::int_bits_128;
        ++args.n_int_word_size_options;
        return true;
    }

    if (arg_view == "pn") {
        args.output_fp_normalized = true;
        ++args.n_output_fp_normalized_options;
        return true;
    }
    if (arg_view == "pu") {
        args.output_fp_normalized = false;
        ++args.n_output_fp_normalized_options;
        return true;
    }

    // if arg is ( '0' | 'm' ) <prefix code 1> [ <prefix code 2> ] <end>
    //     update <input defaults>
    // if arg is ( 'o' | 'm' ) <prefix code 1> <end>
    //     update <output base>
    // note: update both <input defaults> and <output base> for code 'm'

    auto option_code = arg_itr ? std::tolower(*arg_itr++) : '\0'; // might be '0' | 'o' | 'm'
    auto prefix_code_1 = arg_itr ? std::tolower(*arg_itr++) : '\0'; // might be <prefix code_1>
    auto updated = false;

    auto default_number_radix = calc_val::base10;
    auto output_radix = calc_val::base10;
    switch (prefix_code_1) {
        case base2_prefix_code:
            default_number_radix = calc_val::base2;
            output_radix = calc_val::base2;
            break;
        case base8_prefix_code:
            default_number_radix = calc_val::base8;
            output_radix = calc_val::base8;
            break;
        case base10_prefix_code:
            default_number_radix = calc_val::base10;
            output_radix = calc_val::base10;
            break;
        case base16_prefix_code:
            default_number_radix = calc_val::base16;
            output_radix = calc_val::base16;
            break;
        default:
            option_code = '\0';
    }

    if (option_code == '0' || option_code == 'm') {
        auto default_number_type_code = [&] {
            if (arg_itr) {
                auto prefix_code_2 = std::tolower(*arg_itr);
                switch (prefix_code_1) {
                    case base2_prefix_code:
                    case base8_prefix_code:
                    case base10_prefix_code:
                    case base16_prefix_code:
                        if (prefix_code_2 == complex_prefix_code) {
                            ++arg_itr;
                            return calc_val::complex_code;
                        }
                        if (prefix_code_2 == signed_prefix_code) {
                            ++arg_itr;
                            return calc_val::int_code;
                        }
                        if (prefix_code_2 == unsigned_prefix_code) {
                            ++arg_itr;
                            return calc_val::uint_code;
                        }
                }
            }
            return calc_val::int_code;
        }();

        if (arg_itr.at_end()) {
            args.default_number_radix = default_number_radix;
            args.default_number_type_code = default_number_type_code;
            ++args.n_default_options;
            updated = true;
        }
    }

    if ((option_code == 'o' || option_code == 'm') && arg_itr.at_end()) {
        args.output_radix = output_radix;
        ++args.n_output_options;
        updated = true;
    }

    return updated;
}

static bool double_flag_option(const_string_itr arg_itr, calc_args& args) {
    if (arg_itr.view() == "help") {
        ++args.n_help_options;
        return true;
    }
    return false;
}
