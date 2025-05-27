#ifndef NEROLL_STREX_STREX_HPP
#define NEROLL_STREX_STREX_HPP

#include <memory>
#include <string>
#include <string_view>

namespace strex {

class ASTNode;

/// Compiled regular expression.
/// This is used to avoid multiple parsing of the same regular expression.
class ParsedRegex { // NOLINT
    friend std::string from_regex(const ParsedRegex &regex);

 public:
    explicit ParsedRegex(std::string_view regex);
    ~ParsedRegex();

    ParsedRegex(const ParsedRegex &other) = delete;
    ParsedRegex &operator=(const ParsedRegex &other) = delete;

    ParsedRegex(ParsedRegex &&other) = default;
    ParsedRegex &operator=(ParsedRegex &&other) = default;

 private:
    const ASTNode *ast() const;

    std::unique_ptr<ASTNode> ast_;
};

std::string from_regex(std::string_view regex);

std::string from_regex(const ParsedRegex &regex);

} // namespace strex

#endif