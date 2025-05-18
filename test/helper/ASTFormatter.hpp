#ifndef NEROLL_STREX_AST_FORMATTER_HPP
#define NEROLL_STREX_AST_FORMATTER_HPP

#include <string>

#include <strex/Visitor.hpp>

namespace strex::test {

class ASTFormatter : public ASTVisitor {
 public:
    explicit ASTFormatter(const ASTNode *ast);

    std::string format();

 private:
    void format(const ASTNode *node);

    void visit(const TextNode *node) override;

    void visit(const CharsetNode *node) override;

    void visit(const SequenceNode *node) override;

    void visit(const RepeatNode *node) override;

    void visit(const GroupNode * node) override;

    void visit(const AlternationNode *node) override;

    void visit(const BackrefNode *node) override;

    const ASTNode *ast_;
    std::string formatted_;
};

}

#endif