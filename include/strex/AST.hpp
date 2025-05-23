#ifndef NEROLL_STREX_AST_HPP
#define NEROLL_STREX_AST_HPP

#include <memory>
#include <string>
#include <vector>

#include <strex/Charset.hpp>
#include <strex/TextRange.hpp>
#include <strex/Token.hpp>
#include <strex/Visitor.hpp>

namespace strex {

class ASTNode {
 public:
    virtual ~ASTNode() {}

    virtual void accept(ASTVisitor *visitor) const = 0;
};

/// Represents a plain character.
class TextNode : public ASTNode {
 public:
    TextNode(char text, const TextRange &range);

    TextNode(std::string text, const TextRange &range);

    void accept(ASTVisitor *visitor) const override { return visitor->visit(this); }

    std::string text() const { return text_; }

    const TextRange &text_range() const { return range_; }

 private:
    TextRange range_;
    std::string text_;
};

class CharsetNode : public ASTNode {
 public:
    explicit CharsetNode(const Charset &charset, const TextRange &range);

    void accept(ASTVisitor *visitor) const override { return visitor->visit(this); }

    const Charset *charset() const { return charset_; }

 private:
    const Charset *charset_;
    TextRange range_;
};

class SequenceNode : public ASTNode {
 public:
    SequenceNode(std::vector<std::unique_ptr<ASTNode>> nodes, const TextRange &range);

    void accept(ASTVisitor *visitor) const override { return visitor->visit(this); }

    const std::vector<std::unique_ptr<ASTNode>> &sequence() const { return nodes_; }

    const TextRange &text_range() const { return range_; }

 private:
    TextRange range_;
    std::vector<std::unique_ptr<ASTNode>> nodes_;
};

class RepeatNode : public ASTNode {
 public:
    RepeatNode(std::unique_ptr<ASTNode> node, int lower, int upper, const TextRange &range);

    void accept(ASTVisitor *visitor) const override { return visitor->visit(this); }

    const ASTNode *content() const { return node_.get(); }

    const TextRange &text_range() const { return range_; }

    int repeat_lower() const { return lower_; }

    int repeat_upper() const { return upper_; }

 private:
    std::unique_ptr<ASTNode> node_;
    TextRange range_;
    int lower_;
    int upper_;
};

class GroupNode : public ASTNode {
 public:
    GroupNode(std::unique_ptr<ASTNode> node, int index, const TextRange &range);

    void accept(ASTVisitor *visitor) const override { return visitor->visit(this); }

    const ASTNode *content() const { return node_.get(); }

    const TextRange &text_range() const { return range_; }

    int index() const { return index_; }

 private:
    constexpr static int max_group_number = 255;

    std::unique_ptr<ASTNode> node_;
    TextRange range_;
    int index_;
};

class AlternationNode : public ASTNode {
 public:
    AlternationNode(std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right,
                    const TextRange &range);

    void accept(ASTVisitor *visitor) const override { return visitor->visit(this); }

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
    BackrefNode(const GroupNode *group, const TextRange &range);

    void accept(ASTVisitor *visitor) const override { return visitor->visit(this); }

    const TextRange &text_range() const { return range_; }

    const GroupNode *group() const { return group_; }

 private:
    const GroupNode *group_;
    TextRange range_;
};

} // namespace strex

#endif