/// @file

#ifndef NEROLL_STREX_LEXER_HPP
#define NEROLL_STREX_LEXER_HPP

#include <cstddef>
#include <string>
#include <vector>

#include <strex/Token.hpp>

namespace strex {

/// Split the raw regular expression into tokens.
/// Use ECMAScript flavor.
class Lexer {
 public:
    explicit Lexer(std::string regex);

    /// Tokenizes the regular expression and returns all tokens in it.
    std::vector<Token> tokenize();

 private:
    /// Check group count in regular expression.
    /// Throws exception if there are unmatched parentheses.
    void scan_groups();

    /// Returns the next token.
    Token next_token();

    /// Processes the character after a backslash.
    /// Maybe an escaped character, a character class, a word boundary, or a backreference.
    Token backslash();

    /// Process the left bracket.
    /// Returns a token with type `Left_Bracket` or `Character`
    Token left_bracket();

    /// Process the right bracket.
    /// Returns a token with type `Right_Bracket` or `Character`
    Token right_bracket();

    /// Proccess the left parenthesis.
    /// Returns a token with type `Left_Paren` or `Character`.
    Token left_paren();

    /// Processes the right parenthesis.
    /// Returns a token with type `Right_Paren` or `Character`.
    Token right_paren();

    /// Process the left brace.
    /// Returns a token with type `Character` or `Repeat`.
    Token left_brace();

    /// Processes a word boundary (`\b`, `\B`).
    /// If `\b` is in a charset, it will be treated as a character with ASCII 8,
    Token word_boundary(char ch);

    /// Process the number after backslash.
    /// Returns a token with type `Character` or `Backreference`.
    Token number_after_backslash(char ch);

    /// Returns a token with type `Character` or `Repeat`.
    Token repeat();

    /// Checks if current token is the first element in charset.
    bool is_first_in_charset() const;

    /// Processes a character class, e.g., `\d`, `\w`, `\s`.
    Token make_char_class(char ch) const;

    /// Returns a token with type `Character` and given character.
    Token make_character(char ch) const;

    /// Returns a token with type `Backreference`.
    Token make_backreference(int group_number) const;

    /// Returns a token with type `Repeat`.
    Token make_repeat(int repeat_lower, int repeat_upper) const;

    /// Returns a token with given type.
    Token make_token(TokenType type) const;

    /// Returns text range of the token being processed.
    TextRange make_token_range() const;

    /// Returns the previous token that has been processed.
    const Token &prev_token() const;

    /// Returns the character in current position, not move forward.
    char peek() const;

    /// Returns the character in current position and move forward for one character.
    char advance();

    /// Checks if it has been processed to the end of the regular expression
    /// @return Whether reaches the end or not
    bool is_end() const;

    std::string regex_;                         ///< regular expression to be tokenized
    std::size_t current_position_{0};           ///< current processing position
    std::size_t token_begin_position_{0};       ///< begin position of the token being processed
    const std::vector<Token> *tokens_{nullptr}; ///< tokens that has been processed
    int group_depth_{0};                        ///< nest depth of groups
    int group_count_{0};                        ///< the count of group that has been processed
    bool in_charset_{false};                    ///< if current token is in a charset
    bool has_preprocessed_{false};              ///< if the regular expression has been preprocessed
};

} // namespace strex

#endif