#include <cstddef>
#include <random>
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

strex::ParsedRegex::~ParsedRegex() = default;

strex::ParsedRegex::ParsedRegex(ParsedRegex &&other) noexcept = default;

auto strex::ParsedRegex::operator=(ParsedRegex &&other) noexcept -> ParsedRegex & = default;

namespace {

std::size_t next_seed() {
    thread_local std::size_t counter = std::random_device{}();
    std::size_t z = ++counter + 0x9e3779b97f4a7c15ULL;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

} // namespace

std::string strex::from_regex(std::string_view regex) {
    return from_regex(regex, next_seed());
}

std::string strex::from_regex(std::string_view regex, std::size_t seed) {
    Lexer lexer(std::string{regex});
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    Generator generator(ast.get(), seed);
    return generator.generate();
}

std::string strex::from_regex(const ParsedRegex &regex) {
    return from_regex(regex, next_seed());
}

std::string strex::from_regex(const ParsedRegex &regex, std::size_t seed) {
    Generator generator(regex.ast(), seed);
    return generator.generate();
}
