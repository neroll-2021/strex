#include <cassert>

#include <strex/Token.hpp>

auto strex::Token::create_character(char ch, const TextRange &range) -> Token {
    return {ch, TokenType::Character, range};
}

auto strex::Token::create_char_class(char ch, const TextRange &range) -> Token {
    return {ch, TokenType::Char_Class, range};
}

auto strex::Token::create_repeat(int repeat_lower, int repeat_upper, const TextRange &range)
    -> Token {
    return {repeat_lower, repeat_upper, range};
}

auto strex::Token::create_backreference(int group_number, const TextRange &range) -> Token {
    return {group_number, range};
}

auto strex::Token::create(TokenType type, const TextRange &range) -> Token {
    assert(type != TokenType::Character && "use Token::create_character instead");
    assert(type != TokenType::Char_Class && "use Token::create_char_class instead");
    assert(type != TokenType::Repeat && "use Token::create_repeat instead");
    assert(type != TokenType::Backreference && "use Token::create_backreference instead");

    return {type, range};
}

int strex::Token::repeat_lower() const {
    assert(is(TokenType::Repeat));
    return repeat_lower_;
}

int strex::Token::repeat_upper() const {
    assert(is(TokenType::Repeat));
    return repeat_upper_;
}

int strex::Token::group_number() const {
    assert(is(TokenType::Backreference));
    return group_number_;
}

char strex::Token::character() const {
    assert(is(TokenType::Character) || is(TokenType::Char_Class));
    return character_;
}

strex::Token::Token(char ch, TokenType type, const TextRange &range)
    : type_(type), range_(range), character_(ch) {}

strex::Token::Token(int repeat_lower, int repeat_upper, const TextRange &range)
    : type_(TokenType::Repeat),
      range_(range),
      repeat_lower_(repeat_lower),
      repeat_upper_(repeat_upper) {}

strex::Token::Token(int group_number, const TextRange &range)
    : type_(TokenType::Backreference), range_(range), group_number_(group_number) {}

strex::Token::Token(TokenType type, const TextRange &range) : type_(type), range_(range) {}