#include <memory>
#include <string>
#include <vector>

#include <strex/AST.hpp>
#include <strex/Generator.hpp>
#include <strex/Lexer.hpp>
#include <strex/Parser.hpp>
#include <strex/Token.hpp>
#include <strex/strex.hpp>

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#if !defined(NDEBUG) && !defined(__OPTIMIZE__)
    #warning "benchmarks in debug build?"
#endif

int main() {
    const char *phone = R"(1(3[0-9]|4[57]|5[0-35-9]|7[0678]|8[0-9])\d{8})";

    std::vector<strex::Token> tokens;
    ankerl::nanobench::Bench().run("tokenize phone number", [&] {
        strex::Lexer lexer(phone);
        tokens = lexer.tokenize();
        ankerl::nanobench::doNotOptimizeAway(tokens);
    });

    strex::Lexer lexer(phone);
    tokens = lexer.tokenize();
    std::unique_ptr<strex::ASTNode> ast;
    ankerl::nanobench::Bench().minEpochIterations(18474).run("parse phone number ast", [&] {
        strex::Parser parser(tokens);
        ast = parser.parse();
        ankerl::nanobench::doNotOptimizeAway(ast);
    });

    strex::Generator generator(ast.get());
    std::string str1;
    ankerl::nanobench::Bench().minEpochIterations(42253).run("generate phone number", [&] {
        str1 = generator.generate();
        ankerl::nanobench::doNotOptimizeAway(str1);
    });

    ankerl::nanobench::Bench bench;
    bench.relative(true);

    std::string str2;
    bench.run("generate phone number with from_regex", [&] {
        str2 = strex::from_regex(phone);
        ankerl::nanobench::doNotOptimizeAway(str2);
    });

    strex::ParsedRegex parsed(phone);
    std::string str3;
    bench.run("generate phone number from ParsedRegex", [&] {
        str3 = strex::from_regex(parsed);
        ankerl::nanobench::doNotOptimizeAway(str3);
    });
}