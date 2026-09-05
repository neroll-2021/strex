#include <utility>
#include <vector>

#include <strex/Exception.hpp>
#include <strex/Format.hpp>
#include <strex/Lexer.hpp>
#include <strex/TextRange.hpp>
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
    std::vector<strex::TokenType> expect_types = {
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

TEST_CASE("escape character at the beginning") {
    Lexer lexer(R"(\x)");
    std::vector<std::pair<strex::TokenType, char>> expect_types = {
        {TokenType::Character, 'x'},
        {TokenType::End, '\0'},
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

TEST_CASE("escape character in charset") {
    Lexer lexer(R"([ \ \t\n])");
    std::vector<std::pair<strex::TokenType, char>> expect_types = {
        {TokenType::Left_Bracket, '['},  // [
        {TokenType::Character, ' '},     // ' '
        {TokenType::Character, ' '},     // \' '
        {TokenType::Character, '\t'},    // \t
        {TokenType::Character, '\n'},    // \n
        {TokenType::Right_Bracket, ']'}, // ]
        {TokenType::End, '\0'},
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

TEST_CASE("control characters") {
    Lexer lexer(R"(\cA\cz\cw)");
    std::vector<std::pair<strex::TokenType, char>> expect_types = {
        {TokenType::Character, 1},  // \cA
        {TokenType::Character, 26}, // \cz
        {TokenType::Character, 23}, // \cw
        {TokenType::End, '\0'},
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

TEST_CASE("\\c followed by non-letter character") {
    Lexer lexer(R"(\c#)");
    std::vector<std::pair<strex::TokenType, char>> expect_types = {
        {TokenType::Character, '\\'},
        {TokenType::Character, 'c'},
        {TokenType::Character, '#'},
        {TokenType::End, '\0'},
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

TEST_CASE("truncated control character at the end") {
    Lexer lexer("\\c");
    std::vector<std::pair<strex::TokenType, char>> expect_types = {
        {TokenType::Character, '\\'},
        {TokenType::Character, 'c'},
        {TokenType::End, '\0'},
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

TEST_CASE("control character in charset") {
    Lexer lexer("[\\c2]");
    std::vector<std::pair<strex::TokenType, char>> expect_types = {
        {TokenType::Left_Bracket, '['},
        {TokenType::Character, 18},
        {TokenType::Right_Bracket, ']'},
        {TokenType::End, '\0'},
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

TEST_CASE("continuous hyphen in charset") {
    Lexer lexer("[---]");
    std::vector<std::pair<strex::TokenType, char>> expect_types = {
        {TokenType::Left_Bracket, '['},  // [
        {TokenType::Character, '-'},     // -
        {TokenType::Hyphen, '-'},        // -
        {TokenType::Character, '-'},     // -
        {TokenType::Right_Bracket, ']'}, // ]
        {TokenType::End, '\0'},
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

TEST_CASE("continuous hyphen in exclusive charset") {
    Lexer lexer("[^---]");
    std::vector<std::pair<strex::TokenType, char>> expect_types = {
        {TokenType::Left_Bracket, '['},  // [
        {TokenType::Caret, '^'},         // ^
        {TokenType::Character, '-'},     // -
        {TokenType::Hyphen, '-'},        // -
        {TokenType::Character, '-'},     // -
        {TokenType::Right_Bracket, ']'}, // ]
        {TokenType::End, '\0'},
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

TEST_CASE("continuous hyphen in exclusive charset 2") {
    Lexer lexer("[^--^]");
    std::vector<std::pair<strex::TokenType, char>> expect_types = {
        {TokenType::Left_Bracket, '['},  // [
        {TokenType::Caret, '^'},         // ^
        {TokenType::Character, '-'},     // -
        {TokenType::Hyphen, '-'},        // -
        {TokenType::Character, '^'},     // ^
        {TokenType::Right_Bracket, ']'}, // ]
        {TokenType::End, '\0'},
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

TEST_CASE("word boundary in charset") {
    Lexer lexer(R"([\b\B])");
    std::vector<std::pair<strex::TokenType, char>> expect_types = {
        {TokenType::Left_Bracket, '['},  // [
        {TokenType::Character, '\b'},    // \b
        {TokenType::Character, 'B'},     // \B
        {TokenType::Right_Bracket, ']'}, // ]
        {TokenType::End, '\0'},
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

TEST_CASE("backreference in charset") {
    Lexer lexer(R"((a)(b)[\1\2\3\8])");
    std::vector<std::pair<strex::TokenType, char>> expect_types = {
        {TokenType::Left_Paren, '('},    // (
        {TokenType::Character, 'a'},     // a
        {TokenType::Right_Paren, ')'},   // )
        {TokenType::Left_Paren, '('},    // (
        {TokenType::Character, 'b'},     // b
        {TokenType::Right_Paren, ')'},   // )
        {TokenType::Left_Bracket, '['},  // [
        {TokenType::Character, '\1'},    // \1
        {TokenType::Character, '\2'},    // \2
        {TokenType::Character, '\3'},    // \3
        {TokenType::Character, '8'},     // \8
        {TokenType::Right_Bracket, ']'}, // ]
        {TokenType::End, '\0'},
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

TEST_CASE("invalid escape character") {
    Lexer lexer(R"(\d\n\m\y)");
    std::vector<strex::TokenType> expect_types = {
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
    std::vector<strex::TokenType> expect_types = {
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
    std::vector<strex::TokenType> expect_types = {
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
    std::vector<strex::TokenType> expect_types = {
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
    std::vector<std::pair<strex::TokenType, char>> expect_types = {
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

TEST_CASE("backreference2") {
    Lexer lexer(R"(\0\1\2\3\4(())())");
    std::vector<std::pair<strex::TokenType, char>> expect_types = {
        {TokenType::Character, '\0'},     // \0
        {TokenType::Backreference, '\0'}, // \1
        {TokenType::Backreference, '\0'}, // \2
        {TokenType::Backreference, '\0'}, // \3
        {TokenType::Character, '\4'},     // \4
        {TokenType::Left_Paren, '\0'},    // (
        {TokenType::Left_Paren, '\0'},    // (
        {TokenType::Right_Paren, '\0'},   // )
        {TokenType::Right_Paren, '\0'},   // )
        {TokenType::Left_Paren, '\0'},    // (
        {TokenType::Right_Paren, '\0'},   // )
        {TokenType::End, '\0'},
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
    std::vector<std::pair<strex::TokenType, std::pair<int, int>>> expect_types = {
        {TokenType::Character, {}},      // a
        {TokenType::Repeat, {100, 200}}, // {100,200}
        {TokenType::Character, {}},      // b
        {TokenType::Repeat, {1, -1}},    // {1,}
        {TokenType::Character, {}},      // c
        {TokenType::Character, {}},      // {
        {TokenType::Character, {}},      // ,
        {TokenType::Character, {}},      // 1
        {TokenType::Character, {}},      // 0
        {TokenType::Character, {}},      // }
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

TEST_CASE("single left brace") {
    Lexer lexer("{");
    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == 2);
    CHECK(tokens[0].is(TokenType::Character));
}

TEST_CASE("not repetition") {
    Lexer lexer("{1,k}");
    auto tokens = lexer.tokenize();
    std::vector<strex::TokenType> expect_types = {
        TokenType::Character, // {
        TokenType::Character, // 1
        TokenType::Character, // ,
        TokenType::Character, // k
        TokenType::Character, // }
        TokenType::End,
    };
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i]);
    }
}

TEST_CASE("invalid repeat range") {
    Lexer lexer(R"(a{2,1})");
    CHECK_THROWS_WITH_AS(lexer.tokenize(),
                         "invalid repeat quantifier: lower bound 2 is greater than upper bound 1",
                         LexicalError);
}

TEST_CASE("brace in charset") {
    Lexer lexer(R"([{}])");
    std::vector<strex::TokenType> expect_types = {
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
    std::vector<strex::TokenType> expect_types = {
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
    CHECK_THROWS_WITH_AS(lexer.tokenize(), "pattern may not end with a trailing backslash",
                         LexicalError);
}

TEST_CASE("hyphen") {
    Lexer lexer(R"(-[-a-z-]-)");
    std::vector<strex::TokenType> expect_types = {
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

TEST_CASE("^, $ in charset") {
    Lexer lexer(R"([-^$])");
    std::vector<strex::TokenType> expect_types = {
        TokenType::Left_Bracket,  // [
        TokenType::Character,     // -
        TokenType::Character,     // ^
        TokenType::Character,     // $
        TokenType::Right_Bracket, // ]
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
    std::vector<strex::TokenType> expect_types = {
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

TEST_CASE("non-capturing group") {
    Lexer lexer("(?:a)");
    std::vector<strex::TokenType> expect_types = {
        TokenType::Left_Paren,          // (
        TokenType::Non_Capturing_Group, // ?:
        TokenType::Character,           // a
        TokenType::Right_Paren,         // )
        TokenType::End,
    };

    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i]);
    }
}

TEST_CASE("truncated non-capturing group") {
    Lexer lexer("(?:");
    std::vector<strex::TokenType> expect_types = {
        TokenType::Left_Paren,
        TokenType::Non_Capturing_Group,
        TokenType::End,
    };

    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i]);
    }
}

TEST_CASE("truncated non-capturing group 2") {
    Lexer lexer("(?");

    // NOLINTNEXTLINE(bugprone-string-literal-with-embedded-nul)
    CHECK_THROWS_WITH_AS(lexer.tokenize(), "unknown extension '?\0'", LexicalError);
}

TEST_CASE("named capturing group") {
    Lexer lexer("(?<this_is_name1>a)");
    std::vector<std::pair<strex::TokenType, char>> expect_types = {
        {TokenType::Left_Paren, '\0'},            // (
        {TokenType::Named_Capturing_Group, '\0'}, // ?<this_is_name1>
        {TokenType::Character, 'a'},              // a
        {TokenType::Right_Paren, '\0'},           // )
        {TokenType::End, '\0'},
    };

    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i].first);
        if (tokens[i].is(TokenType::Character))
            CHECK(tokens[i].character() == expect_types[i].second);
    }
    CHECK(tokens[1].group_name() == "this_is_name1");
}

TEST_CASE("named capturing group missing closing delimiter") {
    Lexer lexer("(?<this_is_name)");

    CHECK_THROWS_WITH_AS(lexer.tokenize(), "group name is missing its closing delimiter",
                         LexicalError);
}

TEST_CASE("named capturing group missing name") {
    Lexer lexer("(?<>)");

    CHECK_THROWS_WITH_AS(lexer.tokenize(), "unknown extension '?<>'", LexicalError);
}

TEST_CASE("named backreference") {
    Lexer lexer("(?<name>a)\\k<name>b");

    std::vector<std::pair<strex::TokenType, char>> expect_types = {
        {TokenType::Left_Paren, '\0'},            // (
        {TokenType::Named_Capturing_Group, '\0'}, // ?<name>
        {TokenType::Character, 'a'},              // a
        {TokenType::Right_Paren, '\0'},           // )
        {TokenType::Backreference, '\0'},         // \k<name>
        {TokenType::Character, 'b'},              // b
        {TokenType::End, '\0'},
    };

    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i].first);
        if (tokens[i].is(TokenType::Character))
            CHECK(tokens[i].character() == expect_types[i].second);
    }
    CHECK(tokens[1].group_name() == "name");
    CHECK(tokens[4].group_name() == "name");
}

TEST_CASE("named backreference missing name") {
    Lexer lexer("\\k");

    CHECK_THROWS_WITH_AS(lexer.tokenize(), "reference is missing a group name", LexicalError);
}

TEST_CASE("named backreference missing name 2") {
    Lexer lexer("\\k<");

    CHECK_THROWS_WITH_AS(lexer.tokenize(), "reference is missing a group name", LexicalError);
}

TEST_CASE("named backreference missing name 3") {
    Lexer lexer("\\k<>");

    CHECK_THROWS_WITH_AS(lexer.tokenize(), "reference is missing a group name", LexicalError);
}

TEST_CASE("named backreference missing closing delimiter") {
    Lexer lexer("\\k<name");

    CHECK_THROWS_WITH_AS(lexer.tokenize(), "backreference is missing its closing delimiter",
                         LexicalError);
}

TEST_CASE("named backreference in charset") {
    Lexer lexer("[\\k]");

    CHECK_THROWS_WITH_AS(lexer.tokenize(), "cannot use the \\k escape as a literal", LexicalError);
}

// TODO
// TEST_CASE("extension") {
//     Lexer lexer(R"((?=)(?!)(?<=)(?<!)(?:))");
//     std::vector<strex::TokenType> expect_types = {
//         TokenType::Left_Paren,          // (
//         TokenType::Positive_Lookahead,  // ?=
//         TokenType::Right_Paren,         // )
//         TokenType::Left_Paren,          // (
//         TokenType::Negative_Lookahead,  // ?!
//         TokenType::Right_Paren,         // )
//         TokenType::Left_Paren,          // (
//         TokenType::Positive_Lookbehind, // ?<=
//         TokenType::Right_Paren,         // )
//         TokenType::Left_Paren,          // (
//         TokenType::Negative_Lookbehind, // ?<!
//         TokenType::Right_Paren,         // )
//         TokenType::Left_Paren,          // (
//         TokenType::Non_Capturing_Group, // ?:
//         TokenType::Right_Paren,         // )
//         TokenType::End,                 // EOF
//     };
//     auto tokens = lexer.tokenize();
//     REQUIRE(tokens.size() == expect_types.size());
//     for (std::size_t i = 0; i < tokens.size(); i++) {
//         CHECK(tokens[i].type() == expect_types[i]);
//     }
// }

TEST_CASE("invalid extension") {
    Lexer lexer(R"((?a))");
    CHECK_THROWS_WITH_AS(lexer.tokenize(), "unknown extension '?a'", LexicalError);
}

TEST_CASE("hex number2") {
    Lexer lexer(R"(\x00\xff\x1f\x1g\xg1)");
    std::vector<std::pair<strex::TokenType, char>> expect_types = {
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
    std::vector<std::pair<strex::TokenType, char>> expect_types = {
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
    CHECK_THROWS_WITH_AS(lexer.tokenize(), "unsupported hex value ffff", LexicalError);
}

TEST_CASE("dot") {
    Lexer lexer(R"(\.[\..].)");
    std::vector<std::pair<strex::TokenType, char>> expect_types = {
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

TEST_CASE("non-ascii") {
    Lexer lexer(R"(你好)");
    std::vector<strex::TokenType> expect_types = {
        TokenType::Character, TokenType::Character, TokenType::Character, TokenType::Character,
        TokenType::Character, TokenType::Character, TokenType::End,
    };
    auto tokens = lexer.tokenize();
    REQUIRE(tokens.size() == expect_types.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        CHECK(tokens[i].type() == expect_types[i]);
    }
}

TEST_CASE("non-ascii in charset") {
    Lexer lexer("[你好]");
    CHECK_THROWS_WITH_AS(lexer.tokenize(), "non-ascii character in charset is not supported",
                         LexicalError);
}