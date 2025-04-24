#include <cassert>
#include <string>
#include <vector>

#include <strex/Lexer.hpp>
#include <strex/Token.hpp>

strex::Lexer::Lexer(std::string regex) : regex_(std::move(regex)) {}

auto strex::Lexer::tokenize() -> std::vector<Token> {
    std::vector<Token> tokens;
    auto token = next_token();
    while (!token.is(TokenType::End)) {
        tokens.push_back(token);
        token = next_token();
    }
    tokens.push_back(token);
    return tokens;
}

auto strex::Lexer::next_token() -> Token {
    token_begin_position_ = current_position_;

    if (is_end())
        return Token::create(TokenType::End, make_token_range());

    bool in_charset = false;
    char ch = advance();
    if (ch == '\\')
        return backslash(in_charset);
    assert(false && "other character not support yet");
}

auto strex::Lexer::backslash(bool in_charset) -> Token {
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
            return word_boundary(ch, in_charset);
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
        default:
            // invalid escaped character will be treated as normal character
            return make_character(ch);
    }
}

auto strex::Lexer::word_boundary(char ch, bool in_charset) -> Token {
    assert(ch == 'b' || ch == 'B');

    if (ch == 'b') {
        if (in_charset)
            return make_character('\b');
    }
    return Token::create(TokenType::Word_Boundary, make_token_range());
}

auto strex::Lexer::make_char_class(char ch) const -> Token {
    return Token::create_char_class(ch, make_token_range());
}

auto strex::Lexer::make_character(char ch) const -> Token {
    return Token::create_character(ch, make_token_range());
}

auto strex::Lexer::make_token_range() const -> TextRange {
    return {token_begin_position_, current_position_};
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