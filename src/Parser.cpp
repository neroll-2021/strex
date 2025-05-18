#include <cassert>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include <strex/AST.hpp>
#include <strex/Charset.hpp>
#include <strex/Exception.hpp>
#include <strex/Parser.hpp>
#include <strex/TextRange.hpp>
#include <strex/Token.hpp>

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
    auto alter = concat();
    while (match(TokenType::Alternation)) {
        range = range_union(range, peek().range());
        auto rhs = concat();
        alter = std::make_unique<AlternationNode>(std::move(alter), std::move(rhs), range);
    }
    return alter;
}

auto strex::Parser::concat() -> std::unique_ptr<ASTNode> {
    std::vector<std::unique_ptr<ASTNode>> elements;
    TextRange start_range = peek().range();
    TextRange end_range = start_range;
    while (is_atom(peek().type())) {
        elements.push_back(alternative());
        end_range = previous().range();
    }
    // No need for `SequenceNode` when there is only one element.
    if (elements.size() == 1)
        return std::move(elements[0]);
    return std::make_unique<SequenceNode>(std::move(elements), range_union(start_range, end_range));
}

auto strex::Parser::alternative() -> std::unique_ptr<ASTNode> {
    TextRange range = peek().range();

    if (match(TokenType::Character))
        return std::make_unique<TextNode>(previous().character(), range);

    if (match(TokenType::Char_Class))
        return std::make_unique<CharsetNode>(Charset::from_char_class(previous().character()),
                                             range);

    if (match(TokenType::Left_Paren)) {
        // TODO record index of group
        auto subexpression = disjunction();

        consume(TokenType::Right_Paren, "expect ')' to complete group");

        TextRange end_range = previous().range();
        return std::make_unique<GroupNode>(std::move(subexpression), range_union(range, end_range));
    }

    // zero-length character
    return std::make_unique<TextNode>("", range);
}

bool strex::Parser::is_atom(TokenType type) const {
    return type == TokenType::Character || type == TokenType::Char_Class || type == TokenType::Left_Paren;
}

auto strex::Parser::consume(TokenType expect, std::string_view message) -> const Token & {
    if (!match(expect))
        throw ParseError("{}", message);
    return previous();
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