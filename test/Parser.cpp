#include <algorithm>
#include <format>
#include <iterator>
#include <string>

#include <strex/AST.hpp>
#include <strex/Exception.hpp>
#include <strex/Lexer.hpp>
#include <strex/Parser.hpp>
#include <strex/Token.hpp>

#include "helper/ASTFormatter.hpp"

#include <doctest/doctest.h>

#define DIGIT_CHARACTERS "0123456789"
#define UPPER_CHARACTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWER_CHARACTERS "abcdefghijklmnopqrstuvwxyz"
#define SPACE_CHARACTERS " \t\r\n"
#define UNDER_SCROLL     "_"
#define WORD_CHARACTERS  DIGIT_CHARACTERS LOWER_CHARACTERS UPPER_CHARACTERS UNDER_SCROLL

using namespace strex;

template <typename... Args>
void check(std::string regex, std::format_string<Args...> fmt, Args &&...args) {
    Lexer lexer(std::move(regex));
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    test::ASTFormatter formatter(ast.get());
    std::string expect_ast = std::format(fmt, std::forward<Args>(args)...);
    std::string actual_ast = formatter.format();

    CHECK_EQ(actual_ast, expect_ast);
}

std::string characters(std::string chars) {
    std::ranges::sort(chars);
    auto [last, end] = std::ranges::unique(chars);
    chars.erase(last, end);
    return chars;
}

std::string_view ascii_characters() {
    static const auto characters = [] {
        std::string s;
        s.resize_and_overwrite(128, [](char *s, std::size_t n) {
            for (int i = 0; i < 128; i++)
                s[i] = static_cast<char>(i);
            return n;
        });
        return s;
    }();
    return characters;
}

std::string exclude(std::string except) {
    static const std::string all_characters{ascii_characters()};

    std::string characters;

    // Parameters of `set_difference` must be sorted.
    std::ranges::sort(except);
    auto [last, end] = std::ranges::unique(except);
    except.erase(last, end);

    std::ranges::set_difference(all_characters, except, std::back_inserter(characters));
    return characters;
}

TEST_CASE("alternative") {
    check("a|b|c", R"((alter (alter (text "a") | (text "b")) | (text "c")))");
}

TEST_CASE("alternation with char class") {
    check(R"(\d|\s|\w)",
          "(alter (alter (charset include {}) | (charset include {})) | (charset include {}))",
          characters(DIGIT_CHARACTERS), characters(SPACE_CHARACTERS), characters(WORD_CHARACTERS));
}

TEST_CASE("alternation with charset") {
    check("[ab]|[cd]|[^ef]",
          "(alter (alter (charset include ab) | (charset include cd)) | (charset include {}))",
          exclude("ef"));
}

TEST_CASE("alternation with group") {
    check("(a)|(b)|(c)",
          R"((alter (alter (group (text "a")) | (group (text "b"))) | (group (text "c"))))");
}

TEST_CASE("alternation in sequence") {
    check(
        "ab|cd|ef",
        R"((alter (alter (sequence (text "a"), (text "b")) | (sequence (text "c"), (text "d"))) | (sequence (text "e"), (text "f"))))");
}

TEST_CASE("group with text") {
    check("(a0_)", R"((group (sequence (text "a"), (text "0"), (text "_"))))");
}

TEST_CASE("group with quantifier") {
    check("(a{1,2}b*)",
          R"((group (sequence (repeat (text "a") [1, 2]), (repeat (text "b") [0, 3]))))");
}

TEST_CASE("incomplete group") {
    Lexer lexer("((a)");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    CHECK_THROWS_AS_MESSAGE(parser.parse(), ParseError, "expect ')' to complete group");
}

TEST_CASE("quantifier") {
    check(
        "a*b+c?",
        R"((sequence (repeat (text "a") [0, 3]), (repeat (text "b") [1, 3]), (repeat (text "c") [0, 1])))");
}

TEST_CASE("quantifier2") {
    check("a{1,5}", R"((repeat (text "a") [1, 5]))");
}

TEST_CASE("invalid quantifier") {
    Lexer lexer("a{1,2}{3,4}");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    CHECK_THROWS_AS_MESSAGE(parser.parse(), ParseError, "the preceding token is not quantifiable");
}

TEST_CASE("char class \\d") {
    check("\\d", "(charset include {})", characters(DIGIT_CHARACTERS));
}

TEST_CASE("char class \\D") {
    check("\\D", "(charset exclude {})", characters(DIGIT_CHARACTERS));
}

TEST_CASE("char class \\s") {
    check("\\s", "(charset include {})", characters(SPACE_CHARACTERS));
}

TEST_CASE("char class \\S") {
    check("\\S", "(charset exclude {})", characters(SPACE_CHARACTERS));
}

TEST_CASE("char class \\w") {
    check("\\w", "(charset include {})", characters(WORD_CHARACTERS));
}

TEST_CASE("char class \\W") {
    check("\\W", "(charset exclude {})", characters(WORD_CHARACTERS));
}

TEST_CASE("char class .") {
    check(".", "(charset include {})", ascii_characters());
}

TEST_CASE("charset include plain character") {
    // Characters in charset are sorted.
    check("[a0_]", R"((charset include 0_a))");
}

TEST_CASE("charset exclude plain character") {
    check("[^a0_]", "(charset include {})", exclude("0_a"));
}

TEST_CASE("charset include \\d") {
    check(R"([\d])", R"((charset include 0123456789))");
}

TEST_CASE("charset exclude \\d") {
    check("[^\\d]", "(charset include {})", exclude(DIGIT_CHARACTERS));
}

TEST_CASE("charset include \\D") {
    check(R"([\D])", "(charset include {})", exclude("0123456789"));
}

TEST_CASE("charset exclude \\D") {
    check("[^\\D]", "(charset include {})", DIGIT_CHARACTERS);
}

TEST_CASE("charset include \\s") {
    check(R"([\s])", "(charset include {})", characters(" \r\t\n"));
}

TEST_CASE("charset include \\s") {
    check("[^\\s]", "(charset include {})", exclude(SPACE_CHARACTERS));
}

TEST_CASE("charset include \\S") {
    check("[\\S]", "(charset include {})", exclude(" \r\t\n"));
}

TEST_CASE("charset exclude \\S") {
    check("[^\\S]", "(charset include {})", characters(SPACE_CHARACTERS));
}

TEST_CASE("charset include \\w") {
    check("[\\w]", "(charset include {})", characters(WORD_CHARACTERS));
}

TEST_CASE("charset exclude \\w") {
    check("[^\\w]", "(charset include {})", exclude(WORD_CHARACTERS));
}

TEST_CASE("charset include \\W") {
    check("[\\W]", "(charset include {})", exclude(WORD_CHARACTERS));
}

TEST_CASE("charset exclude \\W") {
    check("[^\\W]", "(charset include {})", characters(WORD_CHARACTERS));
}

TEST_CASE("all characters") {
    check("[\\d\\D]", "(charset include {})", ascii_characters());
}

TEST_CASE("backreference") {
    check("(a)\\1", R"((sequence (group (text "a")), (backref 1)))");
}

TEST_CASE("backreference2") {
    // Backreference before associated group matches zero-length character.
    check("\\1\\2(a)", R"((sequence (text ""), (text "{}"), (group (text "a"))))", '\2');
}