#include <memory>
#include <string>
#include <vector>

#include <strex/AST.hpp>
#include <strex/Exception.hpp>
#include <strex/TextRange.hpp>
#include <strex/Token.hpp>
#include <strex/Utils.hpp>

strex::TextNode::TextNode(std::string text, const TextRange &range)
    : range_(range), text_(std::move(text)) {}

strex::SequenceNode::SequenceNode(std::vector<std::unique_ptr<ASTNode>> nodes,
                                  const TextRange &range)
    : range_(range), nodes_(std::move(nodes)) {}

strex::RepeatNode::RepeatNode(std::unique_ptr<ASTNode> node, int lower, int upper,
                              const TextRange &range)
    : node_(std::move(node)), range_(range), lower_(lower), upper_(upper) {}

strex::GroupNode::GroupNode(std::unique_ptr<ASTNode> node, const TextRange &range)
    : node_(std::move(node)), range_(range), index_(create_index()) {}

int strex::GroupNode::create_index() {
    static int next_int = 0;
    int group_number = ++next_int;
    if (group_number > max_group_number)
        throw ParseError("group number reaches limit {}", max_group_number);
    return group_number;
}