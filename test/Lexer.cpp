#include <vector>

#include <strex/Exception.hpp>
#include <strex/Lexer.hpp>
#include <strex/Token.hpp>

#include <doctest/doctest.h>

using namespace strex;

TEST_CASE("token range") {
    Lexer lexer(R"(\b\w\d\\\'\")");
    auto tokens = lexer.tokenize();
    std::vector<TextRange> expect_ranges = {{0, 2},  {2, 4},   {4, 6},  {6, 8},
                                            {8, 10}, {10, 12}, {12, 12}};
    CHECK(tokens.size() == expect_ranges.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].range().start == expect_ranges[i].start);
        CHECK(tokens[i].range().end == expect_ranges[i].end);
    }
}

TEST_CASE("backslash token type") {
    Lexer lexer(R"(\d\D\s\S\w\W\b\B\f\n\r\t\v\\\'\")");
    std::vector<TokenType> expect_types = {
        TokenType::Char_Class,    // \d
        TokenType::Char_Class,    // \D
        TokenType::Char_Class,    // \s
        TokenType::Char_Class,    // \S
        TokenType::Char_Class,    // \w
        TokenType::Char_Class,    // \W
        TokenType::Word_Boundary, // \b
        TokenType::Word_Boundary, // \B
        TokenType::Character,     // \f
        TokenType::Character,     // \n
        TokenType::Character,     // \r
        TokenType::Character,     // \t
        TokenType::Character,     // \v
        TokenType::Character,     // `\`
        TokenType::Character,     // '
        TokenType::Character,     // "
        TokenType::End,           // EOF
    };
    auto tokens = lexer.tokenize();
    CHECK(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].is(expect_types[i]));
    }
}

TEST_CASE("backslash token character") {
    Lexer lexer(R"(\d\D\s\S\w\W\f\n\r\t\v\\\'\")");
    std::vector<char> expect_characters = {'d',  'D',  's',  'S',  'w',  'W',  '\f',
                                           '\n', '\r', '\t', '\v', '\\', '\'', '\"'};
    auto tokens = lexer.tokenize();
    CHECK(tokens.size() == expect_characters.size() + 1);
    for (std::size_t i = 0; i < expect_characters.size(); i++) {
        CHECK(tokens[i].character() == expect_characters[i]);
    }
}

TEST_CASE("invalid escape character") {
    Lexer lexer(R"(\d\n\m\y)");
    CHECK_THROWS_AS_MESSAGE(lexer.tokenize(), LexicalError, "invalid escape character \\m");
}

TEST_CASE("charset") {
    Lexer lexer(R"(\d[\d\\[])");
    std::vector<TokenType> expect_types = {
        TokenType::Char_Class,    // \d
        TokenType::Left_Bracket,  // [
        TokenType::Char_Class,    // \d
        TokenType::Character,     // `\\`
        TokenType::Character,     // [
        TokenType::Right_Bracket, // ]
        TokenType::End,           // EOF
    };
    auto tokens = lexer.tokenize();
    CHECK(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < expect_types.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i]);
    }
}

TEST_CASE("charset with ']' at the beginning") {
    Lexer lexer(R"(\d[]\d\\[]\d)");
    std::vector<TokenType> expect_types = {
        TokenType::Char_Class,    // \d
        TokenType::Left_Bracket,  // [
        TokenType::Character,     // ]
        TokenType::Char_Class,    // \d
        TokenType::Character,     // `\\`
        TokenType::Character,     // [
        TokenType::Right_Bracket, // ]
        TokenType::Char_Class,    // \d
        TokenType::End,           // EOF
    };
    auto tokens = lexer.tokenize();
    CHECK(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < expect_types.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i]);
    }
}