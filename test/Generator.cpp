#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <vector>

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

TEST_CASE("generate high byte range") {
    // libstdc++ std::regex rejects character ranges with endpoints above 0x7F
    // ("Invalid range in bracket expression"), so the output set is verified
    // manually instead of via check().
    Lexer lexer("[a-\\xff]");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    Generator generator(ast.get());

    std::set<unsigned char> seen;
    for (int i = 0; i < 5000; i++) {
        std::string s = generator.generate();
        REQUIRE(s.size() == 1);
        auto ch = static_cast<unsigned char>(s[0]);
        CHECK(ch >= 0x61);
        CHECK(ch <= 0xff);
        seen.insert(ch);
    }
    CHECK(seen.size() == 159);
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
    // According to ECMAScript, a backreference to a group that has not
    // participated in the match matches the empty string.
    Lexer lexer("(aa)|\\1");
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    auto ast = parser.parse();

    Generator generator(ast.get());

    std::string s = generator.generate();
    bool result = (s == "aa" || s == "");
    CHECK(result);
}

TEST_CASE("nested group and backreference") {
    check("(a(b))\\1");
    check("(a(b))\\2");
}

TEST_CASE("backreference after quantified group") {
    // the capture of the last iteration is kept after the quantifier
    check("(a)+\\1");
}

TEST_CASE("backreference in repeated group") {
    // ECMA-262 RepeatMatcher resets the captures of the repeated subtree at
    // the start of each iteration, so the backreference inside the quantifier
    // only sees the current iteration's capture. libstdc++ std::regex rejects
    // self-referencing groups ("Back-reference referred to an opened
    // sub-expression"), so fixed expectations are used instead of check().
    {
        Lexer lexer("(3\\1){6}");
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parse();

        Generator generator(ast.get());
        CHECK(generator.generate() == "333333");
    }
    {
        // libstdc++ std::regex does not support named capturing group
        Lexer lexer("(?<a>x\\k<a>){3}");
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parse();

        Generator generator(ast.get());
        CHECK(generator.generate() == "xxx");
    }
}

TEST_CASE("backreference after quantified alternation") {
    // per ECMA-262, a capture that did not participate in the last iteration
    // is undefined, so the backreference matches empty unless the last
    // iteration took the "(1)" branch. Valid outputs are strings over
    // {'1', '2'} ending with '2' or with "11". libstdc++ std::regex accepts
    // invalid strings like "121" here (it deviates from the spec), so the
    // valid set is checked manually.
    Lexer lexer("(?:(1)|2)+\\1");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    Generator generator(ast.get());

    for (int i = 0; i < 200; i++) {
        std::string s = generator.generate();
        INFO("generated string: \"", s, "\"");
        CHECK(!s.empty());
        CHECK(s.find_first_not_of("12") == std::string::npos);
        bool valid_end = s.ends_with('2') || s.ends_with("11");
        CHECK(valid_end);
    }
}

TEST_CASE("named capturing group and backreferences") {
    // libstdc++ std::regex does not support named capturing group

    Lexer lexer("(?<name>a)\\k<name>");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    Generator generator(ast.get());
    std::string s = generator.generate();
    CHECK(s == "aa");
}

TEST_CASE("named backreference before named capturing group") {
    // libstdc++ std::regex does not support named capturing group

    Lexer lexer("\\k<name>(?<name>a)");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    Generator generator(ast.get());
    std::string s = generator.generate();
    CHECK(s == "a");
}

TEST_CASE("named backreference in named capturing group") {
    // libstdc++ std::regex does not support named capturing group

    Lexer lexer("(?<name>\\k<name>)");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    Generator generator(ast.get());
    std::string s = generator.generate();
    CHECK(s == "");
}

TEST_CASE("generate with same seed") {
    const char *phone = R"(1(3[0-9]|4[57]|5[0-35-9]|7[0678]|8[0-9])\d{8})";
    Lexer lexer(phone);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    Generator generator0(ast.get(), 1);
    Generator generator1(ast.get(), 1);

    std::vector<std::string> strs0;
    std::vector<std::string> strs1;
    strs0.reserve(default_test_count);
    strs1.reserve(default_test_count);
    for (int i = 0; i < default_test_count; i++) {
        strs0.emplace_back(generator0.generate());
        strs1.emplace_back(generator1.generate());
    }
    REQUIRE(strs0.size() == default_test_count);
    REQUIRE(strs0.size() == strs1.size());
    for (int i = 0; i < default_test_count; i++) {
        CHECK(strs0[i] == strs1[i]);
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
    check(R"(((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.){3}((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)))");
}

TEST_CASE("email") {
    check(
        R"(([\w\!\#$\%\&\'\*\+\-\/\=\?\^\`{\|\}\~]+\.)*[\w\!\#$\%\&\'\*\+\-\/\=\?\^\`{\|\}\~]+@((((([a-z0-9]{1}[a-z0-9\-]{0,62}[a-z0-9]{1})|[a-z])\.)+[a-z]{2,6})|(\d{1,3}\.){3}\d{1,3}(\:\d{1,5})?))");
}