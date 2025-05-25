#include <format>
#include <ranges>
#include <string>

#include "ASTFormatter.hpp"
#include <strex/AST.hpp>

strex::test::ASTFormatter::ASTFormatter(const ASTNode *ast) : ast_(ast) {}

std::string strex::test::ASTFormatter::format() {
    formatted_.clear();
    format(ast_);
    return formatted_;
}

void strex::test::ASTFormatter::format(const ASTNode *node) {
    node->accept(this);
}

void strex::test::ASTFormatter::visit(const TextNode *node) {
    formatted_.append(std::format("(text \"{}\")", node->text()));
}

void strex::test::ASTFormatter::visit(const CharsetNode *node) {
    auto charset = node->charset();
    formatted_.append(std::format(
        "(charset {} {})", charset->is_inclusive() ? "include" : "exclude", charset->alphabet()));
}

void strex::test::ASTFormatter::visit(const SequenceNode *node) {
    formatted_.append("(sequence ");
    for (const auto &[index, element] : node->sequence() | std::views::enumerate) {
        if (index != 0) {
            formatted_.append(", ");
        }
        format(element.get());
    }
    formatted_.append(")");
}

void strex::test::ASTFormatter::visit(const RepeatNode *node) {
    formatted_.append("(repeat ");
    format(node->content());
    formatted_.append(std::format(" [{}, {}])", node->repeat_lower(), node->repeat_upper()));
}

void strex::test::ASTFormatter::visit(const GroupNode *node) {
    formatted_.append("(group ");
    format(node->content());
    formatted_.push_back(')');
}

void strex::test::ASTFormatter::visit(const AlternationNode *node) {
    formatted_.append("(alter ");
    for (const auto &[index, element] : node->elements() | std::views::enumerate) {
        if (index != 0)
            formatted_.append(" | ");
        format(element.get());
    }
    formatted_.push_back(')');
}

void strex::test::ASTFormatter::visit(const BackrefNode *node) {
    formatted_.append(std::format("(backref {})", node->group()->index()));
}