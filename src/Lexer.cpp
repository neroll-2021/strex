#include <cassert>
#include <string>
#include <vector>

#include <strex/Exception.hpp>
#include <strex/Lexer.hpp>
#include <strex/Token.hpp>

strex::Lexer::Lexer(std::string regex) : regex_(std::move(regex)) {}

auto strex::Lexer::tokenize() -> std::vector<Token> {
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
        default:
            assert(false && "other character not support yet");
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
        default:
            // invalid escape character
            throw LexicalError("invalid escape character \\{}", ch);
    }
}

auto strex::Lexer::left_bracket() -> Token {
    if (in_charset_)
        return make_character('[');
    in_charset_ = true;
    return Token::create(TokenType::Left_Bracket, make_token_range());
}

auto strex::Lexer::right_bracket() -> Token {
    // a ']' at the beginning of charset is treated as a character
    if (in_charset_ && is_first_in_charset())
        return make_character(']');
    // ']' is not the first character in charset, close the charset
    if (in_charset_) {
        in_charset_ = false;
        return Token::create(TokenType::Right_Bracket, make_token_range());
    }
    return make_character(']');
}

auto strex::Lexer::word_boundary(char ch) -> Token {
    assert(ch == 'b' || ch == 'B');

    if (ch == 'b') {
        if (in_charset_)
            return make_character('\b');
    }
    return Token::create(TokenType::Word_Boundary, make_token_range());
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