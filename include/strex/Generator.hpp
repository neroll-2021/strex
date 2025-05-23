#ifndef NEROLL_STREX_GENERATOR_HPP
#define NEROLL_STREX_GENERATOR_HPP

#include <random>
#include <string>
#include <unordered_map>

#include <strex/Visitor.hpp>

namespace strex {

class Generator : public ASTVisitor {
 public:
    explicit Generator(const ASTNode *ast);

    std::string generate();

 private:
    void generate(const ASTNode *node);

    void visit(const TextNode *node) override;

    void visit(const CharsetNode *node) override;

    void visit(const SequenceNode *node) override;

    void visit(const RepeatNode *node) override;

    void visit(const GroupNode *node) override;

    void visit(const AlternationNode *node) override;

    void visit(const BackrefNode *node) override;

    const ASTNode *ast_;
    std::string generated_string_;
    std::default_random_engine engine_{std::random_device{}()};
    std::unordered_map<const GroupNode *, std::string> group_generated_;
};

} // namespace strex

#endif