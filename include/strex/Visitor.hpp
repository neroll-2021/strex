#ifndef NEROLL_STREX_VISITOR_HPP
#define NEROLL_STREX_VISITOR_HPP

namespace strex {

class ASTNode;
class TextNode;
class CharsetNode;
class SequenceNode;
class RepeatNode;
class GroupNode;
class AlternationNode;
class BackrefNode;

class ASTVisitor {
 public:
    virtual ~ASTVisitor() {}

    virtual void visit(const TextNode *node) = 0;
    virtual void visit(const CharsetNode *node) = 0;
    virtual void visit(const SequenceNode *node) = 0;
    virtual void visit(const RepeatNode *node) = 0;
    virtual void visit(const GroupNode *node) = 0;
    virtual void visit(const AlternationNode *node) = 0;
    virtual void visit(const BackrefNode *node) = 0;
};

} // namespace strex

#endif