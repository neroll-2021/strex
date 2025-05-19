#include <string>
#include <string_view>

#include <strex/AST.hpp>
#include <strex/Exception.hpp>
#include <strex/Lexer.hpp>
#include <strex/Parser.hpp>
#include <strex/Token.hpp>

#include "helper/ASTFormatter.hpp"

#include <doctest/doctest.h>

using namespace strex;

void check(std::string regex, std::string_view expect_ast) {
    Lexer lexer(std::move(regex));
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    test::ASTFormatter formatter(ast.get());
    CHECK(formatter.format() == expect_ast);
}

TEST_CASE("alternative") {
    check("a|b|c", R"((alter (alter (text "a") | (text "b")) | (text "c")))");
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