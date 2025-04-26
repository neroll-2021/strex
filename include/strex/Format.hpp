/// @file

#ifndef NEROLL_STREX_FORMAT_HPP
#define NEROLL_STREX_FORMAT_HPP

#include <format>
#include <string>

#include <strex/Enum.hpp>
#include <strex/Token.hpp>

/// Formatter for Token, used for debug.
template <>
struct std::formatter<strex::Token> {
    template <typename Context>
    constexpr auto parse(Context &context) {
        auto iter = context.begin();
        if (iter != context.end() && *iter != '}')
            throw std::format_error("invalid format args for Token");
        return iter;
    }

    template <typename Context>
    auto format(const strex::Token &token, Context &context) const {
        std::string fmt = std::format("<{}", strex::enum_name(token.type()));
        if (token.is(strex::TokenType::Character)) {
            char ch = token.character();
            fmt += std::format(": '{}' ({})", ch, static_cast<int>(ch));
        }
        if (token.is(strex::TokenType::Repeat)) {
            int lower = token.repeat_lower();
            int upper = token.repeat_upper();
            std::string upper_str = upper == -1 ? "inf" : std::to_string(upper);
            fmt += std::format(": [{}-{}{}", lower, upper_str, (upper == -1) ? ')' : ']');
        }
        if (token.is(strex::TokenType::Backreference)) {
            fmt += std::format(": {}", token.group_number());
        }
        if (token.is(strex::TokenType::Char_Class)) {
            fmt += std::format(": \\{}", token.character());
        }
        fmt += '>';
        return std::format_to(context.out(), "{}", fmt);
    }
};

#endif