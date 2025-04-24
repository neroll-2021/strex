/// @file

#ifndef NEROLL_STREX_LEXER_HPP
#define NEROLL_STREX_LEXER_HPP

#include <cstddef>
#include <string>
#include <vector>

#include <strex/Token.hpp>

namespace strex {

/// Split the raw regular expression into tokens.
class Lexer {
 public:
    explicit Lexer(std::string regex);

    /// Tokenizes the regular expression and returns all tokens in it.
    std::vector<Token> tokenize();

 private:
    /// Returns the next token.
    Token next_token();

    /// Processes the character after a backslash.
    /// Maybe an escaped character, a character class, a word boundary, or a backreference.
    Token backslash(bool in_charset);

    /// Processes a word boundary (`\b`, `\B`).
    /// If `\b` is in a charset, it will be treated as a character with ASCII 8,
    Token word_boundary(char ch, bool in_charset);

    /// Processes a character class, e.g., `\d`, `\w`, `\s`.
    Token make_char_class(char ch) const;

    /// Returns a token with type `Character` and given character.
    Token make_character(char ch) const;

    /// Returns text range of the token being processed.
    TextRange make_token_range() const;

    /// Returns the character in current position, not move forward.
    /// Returns '\0' if reaches the end.
    char peek() const;

    /// Returns the character in current position and move forward for one character.
    /// Returns '\0' if reaches the end, and won't move forward.
    char advance();

    /// Checks if it has been processed to the end of the regular expression
    /// @return Whether reaches the end or not
    bool is_end() const;

    std::string regex_;                   ///< regular expression to be tokenized
    std::size_t current_position_{0};     ///< current processing position
    std::size_t token_begin_position_{0}; ///< begin position of the token being processed
};

} // namespace strex

#endif