#include <strex/AST.hpp>
#include <strex/Lexer.hpp>
#include <strex/Parser.hpp>
#include <strex/Token.hpp>

#include "helper/ASTFormatter.hpp"

#include <doctest/doctest.h>

using namespace strex;

TEST_CASE("alternative") {
    Lexer lexer("a|b|c");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    test::ASTFormatter formatter(ast.get());
    CHECK(formatter.format() == R"((alter (alter (text "a") | (text "b")) | (text "c")))");
}