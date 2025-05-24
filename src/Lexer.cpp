#include <cassert>
#include <cctype>
#include <charconv>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

#include <strex/Exception.hpp>
#include <strex/Lexer.hpp>
#include <strex/TextRange.hpp>
#include <strex/Token.hpp>

strex::Lexer::Lexer(std::string regex) : regex_(std::move(regex)) {}

void strex::Lexer::scan_groups() {
    std::vector<Token> tokens;
    tokens_ = &tokens;
    auto token = next_token();
    while (!token.is(TokenType::End)) {
        tokens.push_back(token);
        token = next_token();
    }
    tokens.push_back(token);

    current_position_ = 0;
    token_begin_position_ = 0;
    in_charset_ = false;
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
        case '{':
            return left_brace();
        case '*':
            return asterisk();
        case '+':
            return plus();
        case '|':
            return vertical_bar();
        case '-':
            return hyphen();
        case '?':
            return question();
        case '.':
            return dot();
        case '^':
            return make_token(TokenType::Caret);
        case '$':
            return make_token(TokenType::Dollar);
        default:
            return character(ch);
    }
}

auto strex::Lexer::backslash() -> Token {
    if (is_end())
        throw LexicalError("pattern may not end with a trailing backslash");

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
        case 'x':
            return hex_number(ch, 2);
        case 'u':
            return hex_number(ch, 4);
        default:
            // other escape characters are treated as normal characters, e.g., \(, \$
            return make_character(ch);
    }
}

auto strex::Lexer::left_bracket() -> Token {
    if (in_charset_)
        return make_character('[');
    in_charset_ = true;
    return make_token(TokenType::Left_Bracket);
}

auto strex::Lexer::right_bracket() -> Token {
    if (in_charset_) {
        in_charset_ = false;
        return make_token(TokenType::Right_Bracket);
    }
    return make_character(']');
}

auto strex::Lexer::left_paren() -> Token {
    if (in_charset_)
        return make_character('(');
    if (!has_preprocessed_)
        group_count_++;
    return make_token(TokenType::Left_Paren);
}

auto strex::Lexer::right_paren() -> Token {
    if (in_charset_)
        return make_character(')');
    return make_token(TokenType::Right_Paren);
}

auto strex::Lexer::left_brace() -> Token {
    if (in_charset_)
        return make_character('{');
    Token token = repeat();
    if (!token.is(TokenType::Character) && !is_end() && peek() == '?')
        advance();
    return token;
}

auto strex::Lexer::asterisk() -> Token {
    if (in_charset_)
        return make_character('*');
    if (!is_end() && peek() == '?') { // *? lazy match
        advance();
        return make_token(TokenType::Star);
    }
    return make_token(TokenType::Star);
}

auto strex::Lexer::plus() -> Token {
    if (in_charset_)
        return make_character('+');
    if (!is_end() && peek() == '?') { // +? lazy match
        advance();
        return make_token(TokenType::Plus);
    }
    return make_token(TokenType::Plus);
}

auto strex::Lexer::vertical_bar() -> Token {
    if (in_charset_)
        return make_character('|');
    return make_token(TokenType::Alternation);
}

auto strex::Lexer::hyphen() -> Token {
    if (in_charset_ && is_first_in_charset())
        return make_character('-');
    if (in_charset_ && !is_end() && peek() == ']')
        return make_character('-');
    if (in_charset_)
        return make_token(TokenType::Hyphen);
    return make_character('-');
}

auto strex::Lexer::question() -> Token {
    if (!tokens_->empty() && prev_token().is(TokenType::Left_Paren))
        return extension();
    if (in_charset_)
        return make_character('?');
    if (!is_end() && peek() == '?')
        advance();
    return make_token(TokenType::Question);
}

auto strex::Lexer::dot() -> Token {
    if (in_charset_)
        return make_character('.');
    return make_char_class('.');
}

auto strex::Lexer::character(char ch) -> Token {
    if (in_charset_ && !isascii(ch))
        throw LexicalError("non-ascii character in charset is not supported");
    return make_character(ch);
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

// Checks if a character is a valid hex character.
static bool is_hex(char ch);

constexpr int hex_value(char ch);

auto strex::Lexer::hex_number(char prev, int digit) -> Token {
    assert(prev == 'x' || prev == 'u');
    assert(digit == 2 || digit == 4);

    if (!is_hex(peek()))
        return make_character(prev);

    std::size_t position = current_position_;

    int value = 0;
    for (int i = 0; i < digit; i++) {
        char ch = advance();
        if (!is_hex(ch)) {
            current_position_ = position;
            return make_character(prev);
        }
        value = value * 16 + hex_value(ch);
    }

    if (value > 255)
        throw LexicalError("unsupported hex value {:x}", value);
    return make_character(static_cast<char>(value));
}

// convert string to int
static int to_repeat_count(std::string_view str);

auto strex::Lexer::repeat() -> Token {
    std::size_t position = current_position_;

    std::string count_str;
    while (is_digit(peek())) {
        count_str.push_back(advance());
    }

    char ch = advance();
    int repeat_lower = -1;
    int repeat_upper = -1;
    if (ch == ',') {
        if (!count_str.empty()) {
            repeat_lower = to_repeat_count(count_str);
        }
    } else if (ch == '}') {
        if (count_str.empty()) {
            current_position_ = position;
            return make_character('{');
        }
        repeat_lower = to_repeat_count(count_str);
        repeat_upper = repeat_lower;
        return make_repeat(repeat_lower, repeat_upper);
    } else {
        current_position_ = position;
        return make_character('{');
    }

    count_str.clear();
    while (is_digit(peek())) {
        count_str.push_back(advance());
    }
    ch = advance();
    if (ch == '}') {
        if (count_str.empty())
            repeat_upper = -1;
        else
            repeat_upper = to_repeat_count(count_str);

        if (repeat_lower == -1 && repeat_upper == -1) {
            current_position_ = position;
            return make_character('{');
        }

        if (repeat_lower == -1)
            repeat_lower = 0;
        if (repeat_upper == -1)
            return make_repeat(repeat_lower, repeat_upper);

        if (repeat_lower > repeat_upper)
            throw LexicalError(
                "invalid repeat quantifier: lower bound {} is greater than upper bound {}",
                repeat_lower, repeat_upper);
        return make_repeat(repeat_lower, repeat_upper);
    } else {
        current_position_ = position;
        return make_character('{');
    }
}

auto strex::Lexer::extension() -> Token {
    char ext = advance();
    // TODO support lookahead, lookbehind and non-capture group
    switch (ext) {
        case ':':
            throw SyntaxNotSupport("non-capture group is not supported");
        case '=':
            throw SyntaxNotSupport("positive lookahead is not supported");
        case '!':
            throw SyntaxNotSupport("negative lookahead is not supported");
        case '<':
            ext = peek();
            if (ext == '=') {
                advance();
                throw SyntaxNotSupport("positive lookbehind is not supported");
            }
            if (ext == '!') {
                advance();
                throw SyntaxNotSupport("negative lookbehind is not supported");
            }
            // Named capture group is not support yet.
            // if (is_alpha(ext))
            //     return named_capture_group();
            throw LexicalError("unknown extension '?<{}'", ext);
        default:
            throw LexicalError("unknown extension '?{}'", ext);
    }
}

auto strex::Lexer::named_capture_group() -> Token {
    // TODO
    return make_token(TokenType::Error);
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

auto strex::Lexer::make_repeat(int repeat_lower, int repeat_upper) const -> Token {
    return Token::create_repeat(repeat_lower, repeat_upper, make_token_range());
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

int to_repeat_count(std::string_view str) {
    int result = -1;
    auto [_, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    if (ec == std::errc::result_out_of_range)
        throw strex::LexicalError("repeat count too large: {}", str);
    assert(ec != std::errc::invalid_argument);
    return result;
}

bool is_hex(char ch) {
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

constexpr int hex_value(char ch) {
    assert(is_hex(ch));
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'a' && ch <= 'f')
        return 10 + ch - 'a';
    if (ch >= 'A' && ch <= 'F')
        return 10 + ch - 'A';
    std::unreachable();
}