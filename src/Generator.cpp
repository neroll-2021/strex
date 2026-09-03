#include <algorithm>
#include <cassert>
#include <cstddef>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include <strex/AST.hpp>
#include <strex/Charset.hpp>
#include <strex/Exception.hpp>
#include <strex/Generator.hpp>

strex::Generator::Generator(const ASTNode *ast, std::size_t seed)
    : ast_(ast), engine_(static_cast<std::mt19937::result_type>(seed)) {
    assert(ast != nullptr);
}

std::string strex::Generator::generate() {
    generated_string_.clear();
    group_generated_.clear();
    generate(ast_);
    return generated_string_;
}

void strex::Generator::generate(const ASTNode *node) {
    node->accept(this);
}

void strex::Generator::visit(const TextNode *node) {
    generated_string_.append(node->text());
}

static std::string exclude(std::string_view except);

void strex::Generator::visit(const CharsetNode *node) {
    const Charset *charset = node->charset();
    std::string characters{charset->alphabet()};

    assert(!characters.empty());

    if (!charset->is_inclusive())
        characters = exclude(characters);

    if (characters.size() == 1) {
        generated_string_.push_back(characters[0]);
        return;
    }

    std::uniform_int_distribution<std::size_t> random(0, characters.size() - 1);

    generated_string_.push_back(characters[random(engine_)]);
}

void strex::Generator::visit(const SequenceNode *node) {
    for (const auto &element : node->sequence()) {
        generate(element.get());
    }
}

namespace strex {
namespace {

    /// Collects every group node in a subtree, used by `visit(RepeatNode)` to reset
    /// captures of the repeated subtree at the start of each iteration
    /// (ECMA-262 RepeatMatcher).
    class GroupCollector : public ASTVisitor {
     public:
        void visit(const TextNode *) override {}

        void visit(const CharsetNode *) override {}

        void visit(const BackrefNode *) override {}

        void visit(const SequenceNode *node) override {
            for (const auto &element : node->sequence())
                element->accept(this);
        }

        void visit(const RepeatNode *node) override { node->content()->accept(this); }

        void visit(const GroupNode *node) override {
            groups.push_back(node);
            node->content()->accept(this);
        }

        void visit(const AlternationNode *node) override {
            for (const auto &element : node->elements())
                element->accept(this);
        }

        std::vector<const GroupNode *> groups;
    };

} // namespace
} // namespace strex

void strex::Generator::visit(const RepeatNode *node) {
    int lower = node->repeat_lower();
    int upper = node->repeat_upper();

    std::uniform_int_distribution<int> random(lower, upper);

    GroupCollector collector;
    node->content()->accept(&collector);

    int repeat_count = random(engine_);
    while (repeat_count--) {
        for (const GroupNode *group : collector.groups)
            group_generated_.erase(group);
        generate(node->content());
    }
}

void strex::Generator::visit(const GroupNode *node) {
    // assert(!group_generated_.contains(node));

    std::string temp = std::move(generated_string_);
    generate(node->content());
    group_generated_[node] = generated_string_;
    generated_string_ = std::move(temp);
    generated_string_.append(group_generated_[node]);
}

void strex::Generator::visit(const AlternationNode *node) {
    const auto &elements = node->elements();
    if (elements.empty())
        return;
    if (elements.size() == 1) {
        generate(elements[0].get());
        return;
    }
    std::uniform_int_distribution<std::size_t> random(0, elements.size() - 1);

    generate(elements[random(engine_)].get());
}

void strex::Generator::visit(const BackrefNode *node) {
    // for regex like `(abc)|\1`
    if (!group_generated_.contains(node->group()))
        return;

    assert(group_generated_.contains(node->group()));

    generated_string_.append(group_generated_[node->group()]);
}

std::string exclude(std::string_view except) {
    assert(std::ranges::is_sorted(except));

    static const auto ascii_characters = [] {
        std::string s;
        s.resize_and_overwrite(128, [](char *s, std::size_t n) {
            for (std::size_t i = 0; i < n; i++)
                s[i] = static_cast<char>(i);
            return n;
        });
        return s;
    }();

    std::string characters;

    std::ranges::set_difference(ascii_characters, except, std::back_inserter(characters));
    return characters;
}
