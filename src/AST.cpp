#include <memory>
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
    assert(lower_ != -1);
    // maximum value of upper_ is indicated by `Parser`
    assert(upper_ != -1);
}

strex::GroupNode::GroupNode(std::unique_ptr<ASTNode> node, const TextRange &range)
    : node_(std::move(node)), range_(range), index_(create_index()) {}

int strex::GroupNode::create_index() {
    static int next_int = 0;
    int group_number = ++next_int;
    if (group_number > max_group_number)
        throw ParseError("group number reaches limit {}", max_group_number);
    return group_number;
}

strex::AlternationNode::AlternationNode(std::unique_ptr<ASTNode> left,
                                        std::unique_ptr<ASTNode> right, const TextRange &range)
    : range_(range), left_(std::move(left)), right_(std::move(right)) {}