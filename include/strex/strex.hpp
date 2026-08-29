#ifndef NEROLL_STREX_STREX_HPP
#define NEROLL_STREX_STREX_HPP

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

namespace strex {

class ASTNode;

/// Compiled regular expression.
/// This is used to avoid multiple parsing of the same regular expression.
class ParsedRegex { // NOLINT
    friend std::string from_regex(const ParsedRegex &regex, std::size_t seed);

 public:
    explicit ParsedRegex(std::string_view regex);
    ~ParsedRegex();

    ParsedRegex(const ParsedRegex &other) = delete;
    ParsedRegex &operator=(const ParsedRegex &other) = delete;

    ParsedRegex(ParsedRegex &&other) noexcept;
    ParsedRegex &operator=(ParsedRegex &&other) noexcept;

 private:
    const ASTNode *ast() const;

    std::unique_ptr<ASTNode> ast_;
};

/// Generates a random string that matches the given regular expression.
std::string from_regex(std::string_view regex);

/// Generates a random string that matches the given regular expression.
/// The same seed and regex always generate the same string.
std::string from_regex(std::string_view regex, std::size_t seed);

/// Generates a random string with an already parsed regular expression.
/// This is used to avoid multiple parsing of the same regular expression.
std::string from_regex(const ParsedRegex &regex);

/// Generates a random string with an already parsed regular expression.
/// This is used to avoid multiple parsing of the same regular expression.
/// The same seed and regex always generate the same string.
std::string from_regex(const ParsedRegex &regex, std::size_t seed);

} // namespace strex

#endif
