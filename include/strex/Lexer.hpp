/// @file

#ifndef NEROLL_STREX_LEXER_HPP
#define NEROLL_STREX_LEXER_HPP

#include <cstddef>
#include <string>
#include <vector>

#include <strex/TextRange.hpp>
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

    /// Processes the backslash.
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

    /// Processes the left brace.
    /// Returns a token with type `Character` or `Repeat`.
    Token left_brace();

    /// Processes the asterisk character '*'.
    /// Returns a token with type `Character` or `Star`.
    Token asterisk();

    /// Processes the plus character '+'.
    /// Returns a token with `Character` or `Plus`.
    Token plus();

    /// Processes the vertical bar character '|'.
    /// Returns a token with type `Character` or `Alternation`.
    Token vertical_bar();

    /// Processes the hyphen character '-'.
    /// Returns a token with type `Character` or `Hyphen`.
    Token hyphen();

    /// Processes the question character '?'.
    /// Returns a token with type `Character` or `Question`.
    Token question();

    /// Processes the dot character '.'.
    /// Returns a token with type `Character` or `Char_Class`.
    Token dot();

    /// Processes the caret character `^`.
    /// Returns a token with type `Caret` or `Character`.
    Token caret();

    /// Processes the dollar character `$`.
    /// Returns a token with type `Dollar` or `Character`.
    Token dollar();

    /// Processes the character.
    /// Checks if the character is valid, returns a token with type `Character`.
    Token character(char ch);

    /// Processes a word boundary (`\b`, `\B`).
    /// If `\b` is in a charset, it will be treated as a character with ASCII 8,
    Token word_boundary(char ch);

    /// Processes the number after backslash.
    /// Returns a token with type `Character` or `Backreference`.
    Token number_after_backslash(char ch);

    /// Processes the hex number after `\x` or `\u`.
    /// Returns a token with type `Character`.
    Token hex_number(char prev, int digit);

    /// Returns a token with type `Character` or `Repeat`.
    Token repeat();

    /// Processes extension such as `?:`, `?=`, `?!`.
    Token extension();

    /// Returns a token with type `Named_Capture_Group`.
    Token named_capture_group();

    /// Returns if there is a repetition to process.
    bool is_repeat();

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
    int group_count_{0};                        ///< the count of group that has been processed
    bool in_charset_{false};                    ///< if current token is in a charset
    bool has_preprocessed_{false};              ///< if the regular expression has been preprocessed
};

} // namespace strex

#endif