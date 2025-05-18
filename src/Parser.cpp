#include <memory>
#include <span>
#include <cassert>

#include <strex/AST.hpp>
#include <strex/Charset.hpp>
#include <strex/TextRange.hpp>
#include <strex/Token.hpp>
#include <strex/Parser.hpp>

// Pattern ::
//     Disjunction
//
// Disjunction ::
//     Alternative
//     Alternative | Disjunction
//
// Alternative ::
//     [empty]
//     Alternative Term
//
// Alternative ::
//     Term*
//
// Term ::
//     Assertion
//     Atom
//     Atom Quantifier
//
// Quantifier ::
//     QuantifierPrefix
//     QuantifierPrefix ?
//
// QuantifierPrefix ::
//     *
//     +
//     ?
//     {DecimalDigits}
//     {DecimalDigits,}
//     {DecimalDigits,DecimalDigits}
//
// Assertion ::
//     ^
//     $
//     \b
//     \B
//     (?=Disjunction)
//     (?!Disjunction)
//
// Atom ::
//     PatternCharacter
//     .
//     \AtomEscape
//     CharacterClass
//     (Disjunction)
//     (?:Disjunction)
//
// AtomEscape ::
//     DecimalEscape
//     CharacterEscape
//     CharacterClassEscape

strex::Parser::Parser(std::span<const Token> tokens) : tokens_(tokens) {}

auto strex::Parser::parse() -> std::unique_ptr<ASTNode> {
    return disjunction();
}

auto strex::Parser::disjunction() -> std::unique_ptr<ASTNode> {
    TextRange range = peek().range();
    auto alter = alternative();
    while (match(TokenType::Alternation)) {
        range = union_range(range, peek().range());
        auto rhs = alternative();
        alter = std::make_unique<AlternationNode>(std::move(alter), std::move(rhs), range);
    }
    return alter;
}

auto strex::Parser::alternative() -> std::unique_ptr<ASTNode> {
    TextRange range = peek().range();

    if (match(TokenType::Character))
        return std::make_unique<TextNode>(previous().character(), range);

    if (match(TokenType::Char_Class))
        return std::make_unique<CharsetNode>(Charset::from_char_class(previous().character()));

    // zero-length character
    return std::make_unique<TextNode>("", range);
}

bool strex::Parser::match(TokenType expect) {
    if (peek().is(expect)) {
        advance();
        return true;
    }
    return false;
}

auto strex::Parser::peek() const -> const Token & {
    assert(!is_end());
    return tokens_[current_position_];
}

auto strex::Parser::previous() const -> const Token & {
    assert(current_position_ != 0);
    return tokens_[current_position_ - 1];
}

auto strex::Parser::advance() -> const Token & {
    assert(!is_end());
    return tokens_[current_position_++];
}

bool strex::Parser::is_end() const {
    return current_position_ >= tokens_.size();
}