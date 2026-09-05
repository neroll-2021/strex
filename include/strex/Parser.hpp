#ifndef NEROLL_STREX_PARSER_HPP
#define NEROLL_STREX_PARSER_HPP

#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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
    /// Collects all group names.
    /// This makes backreferences to groups appearing later can be recognized.
    void collect_group_names();

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

    /// Returns a `BackrefNode`.
    std::unique_ptr<ASTNode> backreference();

    /// Returns all characters in character set.
    std::string charset_item_list();

    /// Checks if meets a character range.
    /// A range has the form `ClassAtom - ClassAtom`, where a `ClassAtom` is a
    /// single character, a character class escape, or a literal '-'
    /// (`Hyphen` token) serving as either endpoint.
    bool is_char_range();

    /// Returns all characters in a character range.
    /// If either endpoint is a character class escape, the range degenerates
    /// into a union of both endpoints and a literal '-' instead.
    std::string char_range();

    /// Returns the characters of a class atom: the character itself for a
    /// `Character` token, the alphabet (or its complement) for a `Char_Class`.
    std::string class_atom_text(const Token &token);

    /// Checks if the token can be a single-character range endpoint.
    bool is_class_atom(const Token &token) const;

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

    /// Returns the token `offset` positions ahead, not move forward.
    /// Returns the `End` token if that position is past the end.
    const Token &peek(std::size_t offset) const;

    /// Returns the token that in previous position.
    const Token &previous() const;

    /// Returns the token in current position and move forward for one token.
    const Token &advance();

    /// Checks if it has been processed to the end of tokens.
    /// @return Whether Parser reaches the end or not.
    bool is_end() const;

    // TODO Maybe can be given in command line arguments.
    constexpr static int default_max_repeat_count = 3;

    /// Max count of groups.
    // TODO Maybe can be given in command line arguments.
    constexpr static int max_group_number = 255;

    std::span<const Token> tokens_;                      ///< tokens to be processed
    std::size_t current_position_{0};                    ///< a
    std::size_t group_count_{0};                         ///< group count that has been processed,
                                                         ///< including non-capturing group
    std::vector<GroupNode *> capturing_groups_{nullptr}; ///< groups that has been processed
    std::unordered_map<std::string_view, const GroupNode *>
        named_groups_; ///< named capturing groups that have been processed
    std::unordered_set<std::string_view> all_group_names_; ///< all group names in regex
};

} // namespace strex

#endif