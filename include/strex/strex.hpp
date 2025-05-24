#ifndef NEROLL_STREX_STREX_HPP
#define NEROLL_STREX_STREX_HPP

#include <memory>
#include <string>
#include <string_view>

namespace strex {

class ASTNode;

/// Compiled regular expression.
/// This is used to avoid multiple parsing of the same regular expression.
class compiled_regex { // NOLINT
    friend std::string from_regex(const compiled_regex &regex);

 public:
    explicit compiled_regex(std::string_view regex);
    ~compiled_regex();

    compiled_regex(const compiled_regex &other) = delete;
    compiled_regex &operator=(const compiled_regex &other) = delete;

    compiled_regex(compiled_regex &&other) = default;
    compiled_regex &operator=(compiled_regex &&other) = default;

 private:
    const ASTNode *ast() const;

    std::unique_ptr<ASTNode> ast_;
};

std::string from_regex(std::string_view regex);

std::string from_regex(const compiled_regex &regex);

} // namespace strex

#endif