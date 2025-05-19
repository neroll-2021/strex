#include <cassert>
#include <memory>
#include <span>
#include <string_view>
#include <utility>
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
    auto ast = alternative();

    if (!match(TokenType::End)) {
        if (peek().is_one_of(TokenType::Star, TokenType::Plus, TokenType::Question,
                             TokenType::Repeat))
            throw ParseError("the preceding token is not quantifiable");
        throw ParseError("invalid regex");
    }
    return ast;
}

auto strex::Parser::alternative() -> std::unique_ptr<ASTNode> {
    TextRange range = peek().range();
    auto alter = sequence();
    while (match(TokenType::Alternation)) {
        range = range_union(range, peek().range());
        auto rhs = sequence();
        alter = std::make_unique<AlternationNode>(std::move(alter), std::move(rhs), range);
    }
    return alter;
}

auto strex::Parser::sequence() -> std::unique_ptr<ASTNode> {
    std::vector<std::unique_ptr<ASTNode>> elements;
    TextRange start_range = peek().range();
    TextRange end_range = start_range;
    while (peek().is_one_of(TokenType::Character, TokenType::Char_Class, TokenType::Left_Paren)) {
        elements.push_back(term());
        end_range = previous().range();
    }
    // No need for `SequenceNode` when there is only one element.
    if (elements.size() == 1)
        return std::move(elements[0]);
    return std::make_unique<SequenceNode>(std::move(elements), range_union(start_range, end_range));
}

auto strex::Parser::term() -> std::unique_ptr<ASTNode> {
    auto content = atom();
    if (is_quantifier(peek().type())) {
        if (match(TokenType::Star))
            return std::make_unique<RepeatNode>(std::move(content), 0, default_max_repeat_count,
                                                previous().range());
        if (match(TokenType::Plus))
            return std::make_unique<RepeatNode>(std::move(content), 1, default_max_repeat_count,
                                                previous().range());
        if (match(TokenType::Question))
            return std::make_unique<RepeatNode>(std::move(content), 0, 1, previous().range());
        if (match(TokenType::Repeat)) {
            const Token &quantifier = previous();
            int max_repeat_count = (quantifier.repeat_upper() == -1 ? default_max_repeat_count
                                                                    : quantifier.repeat_upper());
            return std::make_unique<RepeatNode>(std::move(content), quantifier.repeat_lower(),
                                                max_repeat_count, quantifier.range());
        }
        // This code path should never be hit because all quantifier cases are handled above.
        std::unreachable();
    }
    return content;
}

auto strex::Parser::atom() -> std::unique_ptr<ASTNode> {
    TextRange range = peek().range();

    if (match(TokenType::Character))
        return std::make_unique<TextNode>(previous().character(), range);

    if (match(TokenType::Char_Class))
        return std::make_unique<CharsetNode>(Charset::from_char_class(previous().character()),
                                             range);

    if (match(TokenType::Left_Paren)) {
        // TODO record index of group
        auto subexpression = alternative();

        consume(TokenType::Right_Paren, "expect ')' to complete group");

        TextRange end_range = previous().range();
        return std::make_unique<GroupNode>(std::move(subexpression), range_union(range, end_range));
    }

    // zero-length character
    return std::make_unique<TextNode>("", range);
}

bool strex::Parser::is_atom(TokenType type) const {
    return type == TokenType::Character || type == TokenType::Char_Class ||
           type == TokenType::Left_Paren;
}

bool strex::Parser::is_quantifier(TokenType type) const {
    return type == TokenType::Star || type == TokenType::Plus || type == TokenType::Question ||
           type == TokenType::Repeat;
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