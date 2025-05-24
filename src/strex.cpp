#include <print>
#include <string>
#include <string_view>

#include <strex/AST.hpp>
#include <strex/Exception.hpp>
#include <strex/Generator.hpp>
#include <strex/Lexer.hpp>
#include <strex/Parser.hpp>
#include <strex/strex.hpp>

strex::compiled_regex::compiled_regex(std::string_view regex) {
    Lexer lexer(std::string{regex});
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    ast_ = parser.parse();
}

auto strex::compiled_regex::ast() const -> const ASTNode * {
    assert(ast_ != nullptr);
    return ast_.get();
}

strex::compiled_regex::~compiled_regex() {}

std::string strex::from_regex(std::string_view regex) {
    Lexer lexer(std::string{regex});
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    Generator generator(ast.get());
    return generator.generate();
}

std::string strex::from_regex(const compiled_regex &regex) {
    Generator generator(regex.ast());
    return generator.generate();
}