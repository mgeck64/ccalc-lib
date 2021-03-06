#ifndef CALC_LEXER_HPP
#define CALC_LEXER_HPP

#include <array>
#include <string_view>
#include <cassert>
#include "basics.hpp"
#include "const_string_itr.hpp"

struct lexer_token {
// token scanned by calc_lexer (below)
    enum token_ids {
        unspecified, end, number, identifier, add, sub, mul, div, mod, pow, fac,
        dfac, mfac, lparen, rparen, shiftl, shiftr, band, bor, bxor, bnot, eq,
        help, del, option};
    static constexpr auto token_txt = std::array {
        // text suitable for parser error message.
        // elements correspond with token_ids enums so enum can be used as index
        "unspecified", "end", "number", "identifier", "\"+\"", "\"-\"", "\"*\"", "\"/\"", "\"%\"", "\"^\"","\"!\"",
        "\"!!\"", "multifactorial", "\"(\"", "\")\"", "\"<<\"", "\">>\"", "\"&\"", "\"|\"", "\"^|\"", "\"~\"", "\"=\"",
        "help", "delete", "option"};

    token_ids id = unspecified;
    std::string_view view = {}; // view of scanned token in input string, or default empty view
    using offset_type = std::string_view::size_type;
    offset_type view_offset = 0; // offset of scanned token from start of input string

    lexer_token(token_ids id_, std::string_view view_, offset_type view_offset_)
        : id{id_}, view{view_}, view_offset(view_offset_) {}

    lexer_token() = default;
};

class calc_lexer {
public:
    calc_lexer(std::string_view input, calc_val::radices default_number_radix);
    // input: the view must be valid for the lifetime of the class instance.
    // implementation note: string_view was chosen over const char* (c-style
    // string) as input's type to explore a modern c++ approach for a common
    // read-only string reference (for both c-style and std::string string).
    // this means that input is technically not required to be a c-style string
    // and thus this class's implementation assumes that input is not a c-style
    // string. this assumption both complicated the implementation and made it a
    // little less efficient (but still performant) than if input were assumed
    // to be a c-style string

    auto get_token() -> lexer_token;

    void default_number_radix(calc_val::radices default_number_radix)
    {default_number_radix_ = default_number_radix;}

private:
    const_string_itr in_itr{};
    const_string_itr in_begin = in_itr;
    calc_val::radices default_number_radix_ = calc_val::base10;
    void scan_as_number();
};

inline calc_lexer::calc_lexer(std::string_view input, calc_val::radices default_number_radix)
    : in_itr{input}, in_begin{in_itr}, default_number_radix_{default_number_radix}
{}

#endif // CALC_LEXER_HPP
