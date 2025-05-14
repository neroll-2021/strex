#ifndef NEROLL_STREX_EXCEPTION_HPP
#define NEROLL_STREX_EXCEPTION_HPP

#include <format>
#include <stdexcept>
#include <utility>

namespace strex {

class LexicalError : public std::runtime_error {
 public:
    template <typename... Args>
    explicit LexicalError(std::format_string<Args...> fmt, Args &&...args)
        : std::runtime_error(std::format(fmt, std::forward<Args>(args)...)) {}
};

class ParseError : public std::runtime_error {
 public:
    template <typename... Args>
    explicit ParseError(std::format_string<Args...> fmt, Args &&...args)
        : std::runtime_error(std::format(fmt, std::forward<Args>(args)...)) {}
};

} // namespace strex

#endif