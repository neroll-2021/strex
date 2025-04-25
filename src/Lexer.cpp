#include <cassert>
#include <string>
#include <type_traits>
#include <vector>

#include <strex/Exception.hpp>
#include <strex/Lexer.hpp>
#include <strex/Token.hpp>

strex::Lexer::Lexer(std::string regex) : regex_(std::move(regex)) {}

void strex::Lexer::scan_groups() {
    while (!next_token().is(TokenType::End))
        continue;
    current_position_ = 0;
    token_begin_position_ = 0;
    in_charset_ = false;
    group_depth_ = 0;
    tokens_ = nullptr;
    has_preprocessed_ = true;
}

auto strex::Lexer::tokenize() -> std::vector<Token> {
    scan_groups();
    std::vector<Token> tokens;
    tokens_ = &tokens;
    auto token = next_token();
    while (!token.is(TokenType::End)) {
        tokens.push_back(token);
        token = next_token();
    }
    tokens.push_back(token);
    tokens_ = nullptr;
    assert(group_depth_ == 0);
    return tokens;
}

auto strex::Lexer::next_token() -> Token {
    token_begin_position_ = current_position_;

    if (is_end())
        return Token::create(TokenType::End, make_token_range());

    char ch = advance();
    switch (ch) {
        case '\\':
            return backslash();
        case '[':
            return left_bracket();
        case ']':
            return right_bracket();
        case '(':
            return left_paren();
        case ')':
            return right_paren();
        default:
            return make_character(ch);
    }
}

auto strex::Lexer::backslash() -> Token {
    char ch = advance();
    switch (ch) {
        case 'd':
        case 'D':
        case 's':
        case 'S':
        case 'w':
        case 'W':
            return make_char_class(ch);
        case 'b':
        case 'B':
            return word_boundary(ch);
        case 'f':
            return make_character('\f');
        case 'n':
            return make_character('\n');
        case 'r':
            return make_character('\r');
        case 't':
            return make_character('\t');
        case 'v':
            return make_character('\v');
        case '\\':
            return make_character('\\');
        case '\'':
            return make_character('\'');
        case '\"':
            return make_character('\"');
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return number_after_backslash(ch);
        default:
            // other escape character is treated as normal character, e.g., \(, \$
            return make_character(ch);
    }
}

auto strex::Lexer::left_bracket() -> Token {
    if (in_charset_)
        return make_character('[');
    in_charset_ = true;
    return Token::create(TokenType::Left_Bracket, make_token_range());
}

auto strex::Lexer::right_bracket() -> Token {
    if (in_charset_) {
        in_charset_ = false;
        return Token::create(TokenType::Right_Bracket, make_token_range());
    }
    return make_character(']');
}

auto strex::Lexer::left_paren() -> Token {
    if (in_charset_)
        return make_character('(');
    group_depth_++;
    if (!has_preprocessed_)
        group_count_++;
    return make_token(TokenType::Left_Paren);
}

auto strex::Lexer::right_paren() -> Token {
    if (in_charset_)
        return make_character(')');
    if (group_depth_ <= 0)
        throw LexicalError("unmatched parenthesis ')'");
    group_depth_--;
    return make_token(TokenType::Right_Paren);
}

auto strex::Lexer::word_boundary(char ch) -> Token {
    assert(ch == 'b' || ch == 'B');

    if (ch == 'b') {
        if (in_charset_)
            return make_character('\b');
    }
    return Token::create(TokenType::Word_Boundary, make_token_range());
}

// Checks if a character is a digit.
static bool is_digit(char ch);
// Checks if a character is a octal digit.
static bool is_octal(char ch);

// Calculates the value of octal character sequence.
template <typename T, typename... Args>
    requires std::is_same_v<T, char>
constexpr int octal_value(T ch, Args... args) {
    static_assert(sizeof...(args) < 5, "too many argument");
    assert(ch >= '0' && ch <= '7');
    return (ch - '0') * (1 << (3 * sizeof...(args))) + octal_value(args...);
}

template <>
constexpr int octal_value(char ch) {
    assert(ch >= '0' && ch <= '7');
    return ch - '0';
}

// Calculates the pow of 10.
constexpr int pow_of_10(int n);

// Calculates the value of decimal character sequence.
template <typename T, typename... Args>
    requires std::is_same_v<T, char>
constexpr int decimal_value(T ch, Args... args) {
    static_assert(sizeof...(args) < 5, "too many argument");
    assert(ch >= '0' && ch <= '9');
    return (ch - '0') * pow_of_10(sizeof...(args)) + decimal_value(args...);
}

template <>
constexpr int decimal_value(char ch) {
    assert(ch >= '0' && ch <= '9');
    return ch - '0';
}

auto strex::Lexer::number_after_backslash(char ch) -> Token {
    char first_digit = ch;

    // if only has one digit
    if (is_end() || !is_digit(peek())) {
        if (first_digit == '0')
            return make_character('\0');
        if (first_digit - '0' > group_count_) {
            if (is_octal(first_digit))
                return make_character(static_cast<char>(first_digit - '0'));
            return make_character(first_digit);
        }
        return make_backreference(first_digit - '0');
    }

    // if has the second digit

    char second_digit = peek();

    // if is a backreference
    int group_number = decimal_value(first_digit, second_digit);
    if (group_number != 0 && group_number <= group_count_) {
        advance();
        if (is_end() || !is_digit(peek()))
            return make_backreference(group_number);
        char third_digit = peek();
        if ((group_number * 10) + (third_digit - '0') <= group_count_) {
            advance();
            return make_backreference(group_number * 10 + third_digit - '0');
        }
    }

    // not a backreference

    if (!is_octal(first_digit))
        return make_character(first_digit);

    // the first digit is octal

    if (!is_octal(second_digit))
        return make_character(static_cast<char>(first_digit - '0'));

    // the second digit is octal

    advance();
    if (is_end() || !is_digit(peek()))
        return make_character(static_cast<char>(octal_value(first_digit, second_digit)));

    // if has the thrid digit

    char third_digit = peek();
    if (!is_octal(third_digit))
        return make_character(static_cast<char>(octal_value(first_digit, second_digit)));

    // all three digits are octal

    if (octal_value(first_digit, second_digit, third_digit) > 255)
        return make_character(static_cast<char>(octal_value(first_digit, second_digit)));

    advance();
    return make_character(static_cast<char>(octal_value(first_digit, second_digit, third_digit)));
}

bool strex::Lexer::is_first_in_charset() const {
    assert(tokens_ != nullptr);
    return !tokens_->empty() && prev_token().is(TokenType::Left_Bracket);
}

auto strex::Lexer::make_char_class(char ch) const -> Token {
    return Token::create_char_class(ch, make_token_range());
}

auto strex::Lexer::make_character(char ch) const -> Token {
    return Token::create_character(ch, make_token_range());
}

auto strex::Lexer::make_backreference(int group_number) const -> Token {
    return Token::create_backreference(group_number, make_token_range());
}

auto strex::Lexer::make_token(TokenType type) const -> Token {
    return Token::create(type, make_token_range());
}

auto strex::Lexer::make_token_range() const -> TextRange {
    return {token_begin_position_, current_position_};
}

auto strex::Lexer::prev_token() const -> const Token & {
    assert(tokens_ != nullptr);
    assert(!tokens_->empty());
    return tokens_->back();
}

char strex::Lexer::peek() const {
    assert(!is_end());
    return regex_[current_position_];
}

char strex::Lexer::advance() {
    assert(!is_end());
    return regex_[current_position_++];
}

bool strex::Lexer::is_end() const {
    return current_position_ >= regex_.size();
}

bool is_digit(char ch) {
    return ch >= '0' && ch <= '9';
}

bool is_octal(char ch) {
    return ch >= '0' && ch <= '7';
}

int decimal_value(char first, char second) {
    return (first - '0') * 10 + second - '0';
}

constexpr int pow_of_10(int n) {
    return n == 0 ? 1 : 10 * pow_of_10(n - 1);
}