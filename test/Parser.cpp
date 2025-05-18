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

TEST_CASE("incomplete group") {
    Lexer lexer("((a)");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    CHECK_THROWS_AS_MESSAGE(parser.parse(), ParseError, "expect ')' to complete group");
}