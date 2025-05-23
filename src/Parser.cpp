#include <algorithm>
#include <cassert>
#include <iterator>
#include <memory>
#include <span>
#include <string>
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
    while (is_atom(peek().type())) {
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
    if (is_quantifier(peek().type()))
        return quantifier(std::move(content));
    return content;
}

auto strex::Parser::atom() -> std::unique_ptr<ASTNode> {
    TextRange range = peek().range();

    if (match(TokenType::Character))
        return std::make_unique<TextNode>(previous().character(), range);

    if (match(TokenType::Char_Class)) {
        if (previous().character() == '.')
            return std::make_unique<CharsetNode>(*Charset::any(), range);
        return std::make_unique<CharsetNode>(*Charset::from_char_class(previous().character()),
                                             range);
    }

    if (match(TokenType::Left_Paren))
        return group();

    if (match(TokenType::Left_Bracket))
        return charset();

    if (match(TokenType::Backreference))
        return backreference();

    // zero-length character
    return std::make_unique<TextNode>("", range);
}

auto strex::Parser::quantifier(std::unique_ptr<ASTNode> content) -> std::unique_ptr<ASTNode> {
    assert(is_quantifier(peek().type()));
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
        int lower = quantifier.repeat_lower();
        // If the upper bound is open-ended (-1), we use the lower bound plus default_max_repeat_count
        // to provide a practical limit for repetition, preventing potential infinite loops.
        int max_repeat_count = (quantifier.repeat_upper() == -1 ? lower + default_max_repeat_count
                                                                : quantifier.repeat_upper());
        return std::make_unique<RepeatNode>(std::move(content), quantifier.repeat_lower(),
                                            max_repeat_count, quantifier.range());
    }

    // This code path should never be hit because all quantifier cases are handled above.
    std::unreachable();
}

auto strex::Parser::group() -> std::unique_ptr<ASTNode> {
    // TODO record index of group
    TextRange start_range = previous().range();
    auto subexpression = alternative();
    consume(TokenType::Right_Paren, "expect ')' to complete group");
    TextRange end_range = previous().range();

    auto group =
        std::make_unique<GroupNode>(std::move(subexpression), static_cast<int>(groups_.size()),
                                    range_union(start_range, end_range));
    if (group->index() > max_group_number)
        throw ParseError("group number reaches limit {}", max_group_number);

    groups_.push_back(group.get());

    return group;
}

/// Returns ASCII characters that are not in parameter `except`.
static std::string exclude(std::string except);

auto strex::Parser::charset() -> std::unique_ptr<ASTNode> {
    TextRange start_range = previous().range();
    bool is_inclusive = !match(TokenType::Caret);
    std::string characters = charset_item_list();

    if (!is_inclusive)
        characters = exclude(characters);
    is_inclusive = true;

    const Token &end_token = consume(TokenType::Right_Bracket, "expect ']' to close character set");
    TextRange range = range_union(start_range, end_token.range());
    const Charset *cs = Charset::get(characters);
    return std::make_unique<CharsetNode>(*cs, range);
}

auto strex::Parser::backreference() -> std::unique_ptr<ASTNode> {
    int group_number = previous().group_number();
    assert(group_number != 0);
    // if backreference is before the associated group, matches zero-length text
    if (group_number >= static_cast<int>(groups_.size())) {
        return std::make_unique<TextNode>("", previous().range());
    } else {
        const GroupNode *group = groups_[group_number];
        assert(group != nullptr);
        return std::make_unique<BackrefNode>(group, previous().range());
    }
}

std::string strex::Parser::charset_item_list() {
    std::string characters;
    while (!is_end() && !check(TokenType::Right_Bracket)) {
        assert(peek().is_one_of(TokenType::Character, TokenType::Char_Class));
        if (is_char_range())
            characters.append(char_range());
        else if (check(TokenType::Character)) {
            characters.push_back(advance().character());
        } else if (check(TokenType::Char_Class)) {
            auto cs = Charset::from_char_class(advance().character());
            if (cs->is_inclusive())
                characters.append(cs->alphabet());
            else
                characters.append(exclude(std::string{cs->alphabet()}));
        }
    }
    return characters;
}

bool strex::Parser::is_char_range() {
    auto position = current_position_;
    if (!check(TokenType::Character)) {
        current_position_ = position;
        return false;
    }
    advance();
    if (!check(TokenType::Hyphen)) {
        current_position_ = position;
        return false;
    }
    advance();
    if (!check(TokenType::Character)) {
        current_position_ = position;
        return false;
    }
    current_position_ = position;
    return true;
}

std::string strex::Parser::char_range() {
    char start = advance().character();
    advance();
    char end = advance().character();
    if (start > end)
        throw ParseError("invalid character range: {}-{} ({:02x}-{:02x})", start, end, start, end);
    std::string characters;
    characters.resize_and_overwrite(end - start + 1, [=](char *s, std::size_t size) {
        for (std::size_t i = 0; i < size; i++)
            s[i] = static_cast<char>(start + i);
        return size;
    });
    return characters;
}

bool strex::Parser::is_atom(TokenType type) const {
    return type == TokenType::Character || type == TokenType::Char_Class ||
           type == TokenType::Left_Paren || type == TokenType::Left_Bracket ||
           type == TokenType::Backreference;
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

bool strex::Parser::check(TokenType expect) const {
    if (is_end())
        return false;
    return peek().is(expect);
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

std::string exclude(std::string except) {
    static const auto ascii_characters = [] {
        std::string s;
        s.resize_and_overwrite(128, [](char *s, std::size_t n) {
            for (std::size_t i = 0; i < n; i++)
                s[i] = static_cast<char>(i);
            return n;
        });
        return s;
    }();

    // Parameters of `set_difference` must be sorted.
    std::ranges::sort(except);
    auto [last, end] = std::ranges::unique(except);
    except.erase(last, end);

    std::string result;
    std::ranges::set_difference(ascii_characters, except, std::back_inserter(result));
    return result;
}