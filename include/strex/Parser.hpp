#ifndef NEROLL_STREX_PARSER_HPP
#define NEROLL_STREX_PARSER_HPP

#include <memory>
#include <span>
#include <string_view>

#include <strex/AST.hpp>
#include <strex/Token.hpp>

namespace strex {

/// Build AST from tokens.
/// Use Modified ECMAScript regular expression grammar.
/// @see https://en.cppreference.com/w/cpp/regex/ecmascript
class Parser {
 public:
    explicit Parser(std::span<const Token> tokens);

    /// Build an AST.
    std::unique_ptr<ASTNode> parse();

 private:
    /// Returns a TextNode`, `CharsetNode`, `GroupNode`, `RepeatNode` or `AlternationNode`.
    std::unique_ptr<ASTNode> alternative();

    /// Returns a `SequenceNode` if there is a sequence with more than one element.
    /// Returns a `TextNode`, `CharsetNode`, `GroupNode` or `RepeatNode` if there is only one element.
    std::unique_ptr<ASTNode> sequence();

    /// Return a `TextNode`, `CharsetNode`, `GroupNode` or `RepeatNode`.
    std::unique_ptr<ASTNode> term();

    /// Return a `TextNode`, `CharsetNode` or `GroupNode`.
    std::unique_ptr<ASTNode> atom();

    /// Returns a `RepeatNode`.
    std::unique_ptr<ASTNode> quantifier(std::unique_ptr<ASTNode> content);

    /// Returns a `GroupNode`.
    std::unique_ptr<ASTNode> group();

    /// Returns a `CharsetNode`.
    std::unique_ptr<ASTNode> charset();

    /// Returns all characters in character set.
    std::string charset_item_list();

    /// Checks if meets character range.
    bool is_char_range();

    /// Returns all characters in a character range.
    std::string char_range();

    bool is_atom(TokenType type) const;

    bool is_quantifier(TokenType type) const;

    const Token &consume(TokenType expect, std::string_view message);

    /// Moves forward for one token if current token has expected type.
    /// Do nothing if the type of current token is not expected.
    bool match(TokenType expect);

    /// Checks if current token has the expected type.
    bool check(TokenType expect) const;

    /// Returns the token in current position, not move forward.
    const Token &peek() const;

    /// Returns the token that in previous position.
    const Token &previous() const;

    /// Returns the token in current position and move forward for one token.
    const Token &advance();

    /// Checks if it has been processed to the end of tokens.
    /// @return Whether Parser reaches the end or not.
    bool is_end() const;

    // TODO Maybe can be given in command line arguments.
    constexpr static int default_max_repeat_count = 3;

    std::span<const Token> tokens_;
    std::size_t current_position_{0};
};

} // namespace strex

#endif