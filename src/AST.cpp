#include <memory>
#include <print>
#include <string>
#include <vector>

#include <strex/AST.hpp>
#include <strex/Charset.hpp>
#include <strex/Exception.hpp>
#include <strex/TextRange.hpp>
#include <strex/Token.hpp>
#include <strex/Utils.hpp>

strex::TextNode::TextNode(char text, const TextRange &range) : range_(range), text_(1, text) {}

strex::TextNode::TextNode(std::string text, const TextRange &range)
    : range_(range), text_(std::move(text)) {}

strex::CharsetNode::CharsetNode(const Charset &charset, const TextRange &range)
    : charset_(&charset), range_(range) {}

strex::SequenceNode::SequenceNode(std::vector<std::unique_ptr<ASTNode>> nodes,
                                  const TextRange &range)
    : range_(range), nodes_(std::move(nodes)) {}

strex::RepeatNode::RepeatNode(std::unique_ptr<ASTNode> node, int lower, int upper,
                              const TextRange &range)
    : node_(std::move(node)), range_(range), lower_(lower), upper_(upper) {
    // minimum value of lower_ is 0
    assert(lower_ >= 0);
    // maximum value of upper_ is indicated by `Parser`
    assert(upper_ >= 0);

    if (lower_ > upper_)
        std::println("lower: {}, upper: {}", lower_, upper_);
    assert(lower_ <= upper_);
}

strex::GroupNode::GroupNode(std::unique_ptr<ASTNode> node, int index, const TextRange &range)
    : node_(std::move(node)), range_(range), index_(index) {}

strex::AlternationNode::AlternationNode(std::unique_ptr<ASTNode> left,
                                        std::unique_ptr<ASTNode> right, const TextRange &range)
    : range_(range), left_(std::move(left)), right_(std::move(right)) {}

strex::BackrefNode::BackrefNode(const GroupNode *group, const TextRange &range)
    : group_(group), range_(range) {
    assert(group != nullptr);
}