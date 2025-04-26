#include <utility>
#include <vector>

#include <strex/Exception.hpp>
#include <strex/Format.hpp>
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
    std::vector<TokenType> expect_types = {
        TokenType::Char_Class, // \d
        TokenType::Character,  // \n
        TokenType::Character,  // \m
        TokenType::Character,  // \y
        TokenType::End,
    };
    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i]);
    }
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
        TokenType::Right_Bracket, // ]
        TokenType::Char_Class,    // \d
        TokenType::Character,     // `\\`
        TokenType::Left_Bracket,  // [
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

TEST_CASE("parentheses in charset") {
    Lexer lexer(R"([()]())");
    std::vector<TokenType> expect_types = {
        TokenType::Left_Bracket,  // [
        TokenType::Character,     // (
        TokenType::Character,     // )
        TokenType::Right_Bracket, // ]
        TokenType::Left_Paren,    // (
        TokenType::Right_Paren,   // )
        TokenType::End,           // EOF
    };
    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i]);
    }
}

TEST_CASE("backreference") {
    Lexer lexer(R"(\1\2()\0\1\2\7\8\9\10\17\18\19\20\100\200\300\377\400\000)");
    std::vector<std::pair<TokenType, char>> expect_types = {
        {TokenType::Backreference, '\1'}, // \1
        {TokenType::Character, '\2'},     // \2
        {TokenType::Left_Paren, '\0'},    // (
        {TokenType::Right_Paren, '\0'},   // )
        {TokenType::Character, '\0'},     // \0
        {TokenType::Backreference, '\0'}, // \1
        {TokenType::Character, '\2'},     // \2
        {TokenType::Character, '\7'},     // \7
        {TokenType::Character, '8'},      // \8
        {TokenType::Character, '9'},      // \9
        {TokenType::Character, '\10'},    // \10
        {TokenType::Character, '\17'},    // \17
        {TokenType::Character, '\1'},     // \1
        {TokenType::Character, '8'},      // 8
        {TokenType::Character, '\1'},     // \1
        {TokenType::Character, '9'},      // 9
        {TokenType::Character, '\20'},    // \20
        {TokenType::Character, '\100'},   // \100
        {TokenType::Character, '\200'},   // \200
        {TokenType::Character, '\300'},   // \300
        {TokenType::Character, '\377'},   // \377
        {TokenType::Character, '\40'},    // \40
        {TokenType::Character, '0'},      // 0,
        {TokenType::Character, '\0'},     // \000
        {TokenType::End, '\0'},           // EOF
    };
    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i].first);
        if (tokens[i].is(TokenType::Character)) {
            CHECK(tokens[i].character() == expect_types[i].second);
        }
    }
}

TEST_CASE("repeat") {
    Lexer lexer(R"(a{100,200}b{1,}c{,10}d{5}e{1, 2}f{,}g{})");
    std::vector<std::pair<TokenType, std::pair<int, int>>> expect_types = {
        {TokenType::Character, {}},      // a
        {TokenType::Repeat, {100, 200}}, // {100,200}
        {TokenType::Character, {}},      // b
        {TokenType::Repeat, {1, -1}},    // {1,}
        {TokenType::Character, {}},      // c
        {TokenType::Repeat, {0, 10}},    // {,10}
        {TokenType::Character, {}},      // d
        {TokenType::Repeat, {5, 5}},     // {5}
        {TokenType::Character, {}},      // e
        {TokenType::Character, {}},      // {
        {TokenType::Character, {}},      // 1
        {TokenType::Character, {}},      // ,
        {TokenType::Character, {}},      // ' '
        {TokenType::Character, {}},      // 2
        {TokenType::Character, {}},      // }
        {TokenType::Character, {}},      // f
        {TokenType::Character, {}},      // {
        {TokenType::Character, {}},      // ,
        {TokenType::Character, {}},      // }
        {TokenType::Character, {}},      // g
        {TokenType::Character, {}},      // {
        {TokenType::Character, {}},      // }
        {TokenType::End, {}},            // EOF
    };
    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i].first);
        if (tokens[i].is(TokenType::Repeat)) {
            CHECK(tokens[i].repeat_lower() == expect_types[i].second.first);
            CHECK(tokens[i].repeat_upper() == expect_types[i].second.second);
        }
    }
}

TEST_CASE("invalid repeat range") {
    Lexer lexer(R"(a{2,1})");
    CHECK_THROWS_AS_MESSAGE(
        lexer.tokenize(), LexicalError,
        "invalid repeat quantifier: lower bound 2 is greater than upper bound 1");
}

TEST_CASE("brace in charset") {
    Lexer lexer(R"([{}])");
    std::vector<TokenType> expect_types = {
        TokenType::Left_Bracket,  // [
        TokenType::Character,     // {
        TokenType::Character,     // }
        TokenType::Right_Bracket, // ]
        TokenType::End,           // EOF
    };
    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i]);
    }
}

TEST_CASE("characters") {
    Lexer lexer(R"(*^|$+]})");
    std::vector<TokenType> expect_types = {
        TokenType::Star,        // *
        TokenType::Caret,       // ^
        TokenType::Alternation, // |
        TokenType::Dollar,      // $
        TokenType::Plus,        // +
        TokenType::Character,   // ]
        TokenType::Character,   // }
        TokenType::End,         // EOF
    };
    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i]);
    }
}

TEST_CASE("trailing backslash") {
    Lexer lexer(R"(\)");
    CHECK_THROWS_AS_MESSAGE(lexer.tokenize(), LexicalError,
                            "pattern may not end with a trailing backslash");
}

TEST_CASE("hyphen") {
    Lexer lexer(R"(-[-a-z-]-)");
    std::vector<TokenType> expect_types = {
        TokenType::Character,     // -
        TokenType::Left_Bracket,  // [
        TokenType::Character,     // -
        TokenType::Character,     // a
        TokenType::Hyphen,        // -
        TokenType::Character,     // b
        TokenType::Character,     // -
        TokenType::Right_Bracket, // ]
        TokenType::Character,     // -
        TokenType::End,           // EOF
    };
    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i]);
    }
}

TEST_CASE("lazy") {
    Lexer lexer(R"(+???*?)");
    std::vector<TokenType> expect_types = {
        TokenType::Plus,     // +?
        TokenType::Question, // ??
        TokenType::Star,     // *?
        TokenType::End,      // EOF
    };
    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i]);
    }
}

TEST_CASE("extension") {
    Lexer lexer(R"((?=)(?!)(?<=)(?<!)(?:))");
    std::vector<TokenType> expect_types = {
        TokenType::Left_Paren,          // (
        TokenType::Positive_Lookahead,  // ?=
        TokenType::Right_Paren,         // )
        TokenType::Left_Paren,          // (
        TokenType::Negative_Lookahead,  // ?!
        TokenType::Right_Paren,         // )
        TokenType::Left_Paren,          // (
        TokenType::Positive_Lookbehind, // ?<=
        TokenType::Right_Paren,         // )
        TokenType::Left_Paren,          // (
        TokenType::Negative_Lookbehind, // ?<!
        TokenType::Right_Paren,         // )
        TokenType::Left_Paren,          // (
        TokenType::Non_Capturing_Group, // ?:
        TokenType::Right_Paren,         // )
        TokenType::End,                 // EOF
    };
    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i]);
    }
}

TEST_CASE("invalid extension") {
    Lexer lexer(R"((?a))");
    CHECK_THROWS_AS_MESSAGE(lexer.tokenize(), LexicalError, "unknown extension '?a'");
}

TEST_CASE("hex number2") {
    Lexer lexer(R"(\x00\xff\x1f\x1g\xg1)");
    std::vector<std::pair<TokenType, char>> expect_types = {
        {TokenType::Character, '\0'},   // \x00
        {TokenType::Character, '\xff'}, // \xff
        {TokenType::Character, '\x1f'}, // \x1f
        {TokenType::Character, 'x'},    // \x
        {TokenType::Character, '1'},    // 1
        {TokenType::Character, 'g'},    // g
        {TokenType::Character, 'x'},    // \x
        {TokenType::Character, 'g'},    // g
        {TokenType::Character, '1'},    // 1
        {TokenType::End, '\0'},         // EOF
    };
    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i].first);
        if (tokens[i].is(TokenType::Character))
            CHECK(tokens[i].character() == expect_types[i].second);
    }
}

TEST_CASE("hex number4") {
    Lexer lexer(R"(\u0000\u00ff)");
    std::vector<std::pair<TokenType, char>> expect_types = {
        {TokenType::Character, '\0'},   // \u0000
        {TokenType::Character, '\xff'}, // \u00ff
        {TokenType::End, '\0'},         // EOF
    };
    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i].first);
        if (tokens[i].is(TokenType::Character))
            CHECK(tokens[i].character() == expect_types[i].second);
    }
}

TEST_CASE("unsupported hex value") {
    Lexer lexer(R"(\uffff)");
    CHECK_THROWS_AS_MESSAGE(lexer.tokenize(), LexicalError, "unsupported hex value 0xffff");
}

TEST_CASE("dot") {
    Lexer lexer(R"(\.[\..].)");
    std::vector<std::pair<TokenType, char>> expect_types = {
        {TokenType::Character, '.'},     // \.
        {TokenType::Left_Bracket, '['},  // [
        {TokenType::Character, '.'},     // \.
        {TokenType::Character, '.'},     // .
        {TokenType::Right_Bracket, ']'}, // ]
        {TokenType::Char_Class, '.'},    // .
        {TokenType::End, '\0'},          // EOF
    };
    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i].first);
        if (tokens[i].is(TokenType::Character) || tokens[i].is(TokenType::Char_Class))
            CHECK(tokens[i].character() == expect_types[i].second);
    }
}