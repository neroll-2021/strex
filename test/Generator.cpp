#include <algorithm>
#include <cctype>
#include <regex>
#include <string>
#include <string_view>

#include <strex/Exception.hpp>
#include <strex/Generator.hpp>
#include <strex/Lexer.hpp>
#include <strex/Parser.hpp>

#include "helper/ASTFormatter.hpp"

#include <doctest/doctest.h>

using namespace strex;

constexpr static int default_test_count = 50;

void check(std::string_view regex, int test_count = default_test_count) {
    Lexer lexer(std::string{regex});
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    auto ast = parser.parse();

    test::ASTFormatter formatter(ast.get());
    std::string formatted_ast = formatter.format();

    Generator generator(ast.get());

    for (int i = 0; i < test_count; i++) {
        auto str = generator.generate();

        std::regex r(std::string{regex});

        INFO("generated string: \"", str, "\"");
        INFO("AST: ", formatted_ast);
        REQUIRE(std::ranges::all_of(str, ::isprint));
        CHECK(std::regex_match(str, r));
    }
}

TEST_CASE("generate text") {
    check("a");
    check("0");
    check("_");
    check(";");
    check("!");
    check("\\\\");
}

TEST_CASE("generate charset include") {
    check("[abcde]");

    check("[a-z]");
    check("[0-9]");
    check("[A-Za-z0-9_]");
    check("[-a-z-]");

    check("[|.'?+*]");

    check("[\\d]");
    check("[\\D]");

    check("[\\s]");
    check("[\\S]");

    check("[\\w]");
    check("[\\W]");
}

TEST_CASE("generate charset exclude") {
    check("[^abcd]");

    check("[^a-z]");
    check("[^0-9]");
    check("[^0-9a-zA-Z_]");
    check("[-a-z-]");

    check("[^|.'?+*]");

    check("[^\\d]");
    check("[^\\D]");
    check("[^\\s]");
    check("[^\\S]");
    check("[^\\w]");
    check("[^\\W]");
}

TEST_CASE("generate char class") {
    check("\\d");
    check("\\D");
    check("\\s");
    check("\\S");
    check("\\w");
    check("\\W");
    check(".");
}

TEST_CASE("generate repeat") {
    check("a?");
    check("a+");
    check("a*");
    check("a{4,8}");
    check("a{4,}");

    check("[abcde]?");
    check("[abcde]+");
    check("[abcde]*");
    check("[abcde]{4,8}");
    check("[abcde]{4,}");
}

TEST_CASE("backreference") {
    check("(a)\\1");
    check("(ab[cd]*)ef\\1");
    check(R"((a?)\1(b+)\2(ccd)\3)");
}

TEST_CASE("generate sequence") {
    check("hello world!");
    check("[^ab]cd(ef)\\1g+h?i*jk(\\1)");
}

TEST_CASE("alternation") {
    check("a|b|c|d");
    check("[ab]|[cd]|[^ef]");
    check("(ab)|(cd)|(ef)");
    check(R"(\d|\w|\s)");
    check("a*|b+|c?|d|e");
}

TEST_CASE("empty alternation") {
    check("(a|)");
}

TEST_CASE("group and backreference in the same alternation") {
    // In this case, Strex cannot guarantee that
    // the generated string will always match the regular expression.
    Lexer lexer("(aa)|\\1");
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    auto ast = parser.parse();

    Generator generator(ast.get());

    try {
        std::string s = generator.generate();
        bool result = (s == "aa" || s == "");
        CHECK(result);
    }
    catch (GenerateError &e) {
        CHECK_EQ(e.what(),
                 std::string{"failed to generate string: backreference to optional group"});
    }
}

TEST_CASE("phone number") {
    check(R"(1(3[0-9]|4[57]|5[0-35-9]|7[0678]|8[0-9])\d{8})");
    check(R"((13[0-9]|14[01456879]|15[0-35-9]|16[2567]|17[0-8]|18[0-9]|19[0-35-9])\d{8})");
}

TEST_CASE("ipv4 address") {
    check(
        R"(((2((5[0-5])|([0-4]\d)))|([0-1]?\d{1,2}))(\.((2((5[0-5])|([0-4]\d)))|([0-1]?\d{1,2}))){3})");
    // check(R"(((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4})");
}

TEST_CASE("email") {
    check(
        R"(([-!#-'*+/-9=?A-Z^-~]+(\.[-!#-'*+/-9=?A-Z^-~]+)*|"(!#-[^-~ \t]|(\\[\t -~]))+")@([0-9A-Za-z]([0-9A-Za-z-]{0,61}[0-9A-Za-z])?(\.[0-9A-Za-z]([0-9A-Za-z-]{0,61}[0-9A-Za-z])?)*|\[((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])(\.(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])){3}|IPv6:((((0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}):){6}|::((0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}):){5}|[0-9A-Fa-f]{0,4}::((0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}):){4}|(((0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}):)?(0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}))?::((0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}):){3}|(((0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}):){0,2}(0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}))?::((0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}):){2}|(((0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}):){0,3}(0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}))?::(0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}):|(((0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}):){0,4}(0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}))?::)((0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}):(0|[1-9A-Fa-f][0-9A-Fa-f]{0,3})|(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])(\.(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])){3})|(((0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}):){0,5}(0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}))?::(0|[1-9A-Fa-f][0-9A-Fa-f]{0,3})|(((0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}):){0,6}(0|[1-9A-Fa-f][0-9A-Fa-f]{0,3}))?::))]))");

    check(
        R"(([\w\!\#$\%\&\'\*\+\-\/\=\?\^\`{\|\}\~]+\.)*[\w\!\#$\%\&\'\*\+\-\/\=\?\^\`{\|\}\~]+@((((([a-z0-9]{1}[a-z0-9\-]{0,62}[a-z0-9]{1})|[a-z])\.)+[a-z]{2,6})|(\d{1,3}\.){3}\d{1,3}(\:\d{1,5})?))");
}