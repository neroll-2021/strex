#include <print>
#include <string>
#include <string_view>

#include <strex/AST.hpp>
#include <strex/Exception.hpp>
#include <strex/Generator.hpp>
#include <strex/Lexer.hpp>
#include <strex/Parser.hpp>
#include <strex/strex.hpp>

strex::ParsedRegex::ParsedRegex(std::string_view regex) {
    Lexer lexer(std::string{regex});
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    ast_ = parser.parse();
}

auto strex::ParsedRegex::ast() const -> const ASTNode * {
    assert(ast_ != nullptr);
    return ast_.get();
}

strex::ParsedRegex::~ParsedRegex() {}

std::string strex::from_regex(std::string_view regex) {
    Lexer lexer(std::string{regex});
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    Generator generator(ast.get());
    return generator.generate();
}

std::string strex::from_regex(const ParsedRegex &regex) {
    Generator generator(regex.ast());
    return generator.generate();
}