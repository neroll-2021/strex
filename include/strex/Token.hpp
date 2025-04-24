/// @file

#ifndef NEROLL_STREX_TOKEN_HPP
#define NEROLL_STREX_TOKEN_HPP

#include <string>

namespace strex {

/// Types of tokens.
enum class TokenType {

    Character,  ///< any "non-special" character
    Char_Class, ///< `\w`, `\d`, etc.

    Star,     ///< `*`
    Plus,     ///< `+`
    Question, ///< `?`
    Repeat,   ///< `{n}`, `{n,}`, `{,m}` or `{n,m}`

    Alternation, ///< `|`

    Word_Boundary, ///< `\b`

    Non_Capturing_Group, ///< `(?:...)`
    Positive_Lookahead,  ///< `(?=...)`
    Negative_Lookahead,  ///< `(?!...)`
    Positive_Lookbehind, ///< `(?<=...)`
    Negative_Lookbehind, ///< `(?<!...)`
    Ignored_Extension,   ///< extensions that are ignored

    Backreference, ///< backreferences

    Left_Paren,    ///< `(`
    Right_Paren,   ///< `)`
    Left_Bracket,  ///< `[`
    Right_Bracket, ///< `]`

    Caret,  ///< `^`
    Dollar, ///< `$`
    Hyphen, ///< `-` in character set

    Error, ///< Error token
    End,   ///< End of input
};

/// A [start, end) range in regular expression, counted by byte offset, 0-based.
struct TextRange {

    TextRange() = default;

    TextRange(std::size_t start, std::size_t end) : start(start), end(end) {}

    std::size_t start;
    std::size_t end;
};

/// Basic element to be processed extracted from raw regular expression.
class Token {
 public:
    /// Creates a token with type @link TokenType::Character Character @endlink.
    static Token create_character(char ch, const TextRange &range);

    /// Creates a token with type `Char_Class`.
    static Token create_char_class(char ch, const TextRange &range);

    /// Creates a token with type `Repeat`.
    static Token create_repeat(int repeat_lower, int repeat_upper, const TextRange &range);

    /// Creates a token with type `Backreference`.
    static Token create_backreference(int group_number, const TextRange &range);

    /// Creates a token with specific type.
    static Token create(TokenType type, const TextRange &range);

    /// Checks if token type is the same with given type
    bool is(TokenType type) const { return type_ == type; }

    /// Returns the text range of token.
    const TextRange &range() const { return range_; }

    /// Returns the type of token.
    TokenType type() const { return type_; }

    /// Returns the minimum times of repetition.
    /// Can only be called when token type is `Repeat`.
    int repeat_lower() const;
    /// Returns the maximum times of repetition.
    /// Can only be called when token type is `Repeat`.
    int repeat_upper() const;

    /// Returns group number of backreference.
    /// Can only be called when token type is `Backreference`.
    int group_number() const;

    /// Returns character represented by token.
    /// Can only be called when token type is `Character` or `Char_Class`.
    char character() const;

 private:
    /// Constructs Token with specific character and type.
    Token(char ch, TokenType type, const TextRange &range);

    /// Constructs Token with type `Repeat`.
    Token(int repeat_lower, int repeat_upper, const TextRange &range);

    /// Constructs Token with type `Backreference`.
    Token(int group_number, const TextRange &range);

    /// Constructs Token with specific type.
    Token(TokenType type, const TextRange &range);

    TokenType type_{TokenType::Error};
    std::string group_name_; // for named group
    TextRange range_;        // text range in regular expression
    int repeat_lower_{-1};   // for Repeat, minimum times of repetition
    int repeat_upper_{-1};   // for Repeat, maximum times of repetition
    int group_number_{-1};   // for Backreference
    char character_{'\0'};   // for Character and Char_Class
};

} // namespace strex

#endif