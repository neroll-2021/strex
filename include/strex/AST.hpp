#ifndef NEROLL_STREX_AST_HPP
#define NEROLL_STREX_AST_HPP

#include <memory>
#include <string>
#include <vector>

#include <strex/Charset.hpp>
#include <strex/TextRange.hpp>
#include <strex/Token.hpp>

namespace strex {

class ASTNode {
 public:
    virtual ~ASTNode() {}
};

/// Represents a plain character.
class TextNode : public ASTNode {
 public:
    TextNode(std::string text, const TextRange &range);

    std::string text() const { return text_; }

    const TextRange &text_range() const { return range_; }

 private:
    TextRange range_;
    std::string text_;
};

class CharsetNode : public ASTNode {
 public:
    explicit CharsetNode(const Charset &charset);

    const Charset *charset() const { return charset_; }

 private:
    const Charset *charset_;
};

class SequenceNode : public ASTNode {
 public:
    SequenceNode(std::vector<std::unique_ptr<ASTNode>> nodes, const TextRange &range);

    const TextRange &text_range() const { return range_; }

 private:
    TextRange range_;
    std::vector<std::unique_ptr<ASTNode>> nodes_;
};

class RepeatNode : public ASTNode {
 public:
    RepeatNode(std::unique_ptr<ASTNode> node, int lower, int upper, const TextRange &range);

    const TextRange &text_range() const { return range_; }

    int repeat_lower() const { return lower_; }

    int repeat_upper() const { return upper_; }

 private:
    // TODO Maybe can be given in command line arguments.
    constexpr static int max_repeat_count = 3;

    std::unique_ptr<ASTNode> node_;
    TextRange range_;
    int lower_;
    int upper_;
};

class GroupNode : public ASTNode {
 public:
    explicit GroupNode(std::unique_ptr<ASTNode> node, const TextRange &range);

    const TextRange &text_range() const { return range_; }

    int index() const { return index_; }

 private:
    /// Gets the index of the group.
    int create_index();

    constexpr static int max_group_number = 255;

    std::unique_ptr<ASTNode> node_;
    TextRange range_;
    int index_;
};

class AlternationNode : public ASTNode {
 public:
    AlternationNode(std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right,
                    const TextRange &range);

    const TextRange &text_range() const { return range_; }

    const ASTNode *left() const { return left_.get(); }

    const ASTNode *right() const { return right_.get(); }

 private:
    TextRange range_;
    std::unique_ptr<ASTNode> left_;
    std::unique_ptr<ASTNode> right_;
};

class BackrefNode : public ASTNode {
 public:
    BackrefNode(const ASTNode *group, const TextRange &range);

    const TextRange &text_range() const { return range_; }

    const ASTNode *group() const { return group_; }

 private:
    TextRange range_;
    const ASTNode *group_;
};

} // namespace strex

#endif