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

    bool can_match_empty(const ASTNode *node);

    /// Computes whether a subtree has a generation path that produces the
    /// empty string. `visit(RepeatNode)` only needs the empty-iteration
    /// guard (and its capture snapshot) when this is possible; the analysis
    /// errs on the side of "possible", which only costs performance, never
    /// correctness.
    class EmptyMatchAnalyzer : public ASTVisitor {
     public:
        void visit(const TextNode *node) override { possible = node->text().empty(); }

        void visit(const CharsetNode *) override { possible = false; }

        void visit(const BackrefNode *) override { possible = true; }

        void visit(const SequenceNode *node) override {
            possible = true;
            for (const auto &element : node->sequence()) {
                if (!can_match_empty(element.get())) {
                    possible = false;
                    break;
                }
            }
        }

        void visit(const RepeatNode *node) override {
            possible = node->repeat_lower() == 0 || can_match_empty(node->content());
        }

        void visit(const GroupNode *node) override { possible = can_match_empty(node->content()); }

        void visit(const AlternationNode *node) override {
            possible = false;
            for (const auto &element : node->elements())
                possible = possible || can_match_empty(element.get());
        }

        bool possible = false;
    };

    bool can_match_empty(const ASTNode *node) {
        EmptyMatchAnalyzer analyzer;
        node->accept(&analyzer);
        return analyzer.possible;
    }

} // namespace
} // namespace strex

void strex::Generator::visit(const RepeatNode *node) {
    int lower = node->repeat_lower();
    int upper = node->repeat_upper();

    std::uniform_int_distribution<int> random(lower, upper);

    GroupCollector collector;
    node->content()->accept(&collector);

    // The empty-iteration guard can only trigger when the subtree has an
    // empty generation path; otherwise skip the per-iteration snapshot.
    const bool may_match_empty = can_match_empty(node->content());

    int repeat_count = random(engine_);
    int accepted = 0;
    while (repeat_count--) {
        std::size_t size_before = 0;
        std::unordered_map<const GroupNode *, std::string> snapshot;
        if (may_match_empty) {
            // Snapshot the subtree captures before resetting them, i.e. the
            // state after the last accepted iteration.
            for (const GroupNode *group : collector.groups) {
                if (auto it = group_generated_.find(group); it != group_generated_.end())
                    snapshot.emplace(group, it->second);
            }
            size_before = generated_string_.size();
        }
        for (const GroupNode *group : collector.groups)
            group_generated_.erase(group);

        generate(node->content());

        // ECMA-262 RepeatMatcher: once the lower bound is satisfied, an
        // iteration that matches empty is rejected - the repetition stops
        // before it and the captures of the last accepted iteration are
        // kept. While the lower bound is not yet satisfied, empty
        // iterations are accepted and count towards it.
        if (may_match_empty && generated_string_.size() == size_before && accepted >= lower) {
            for (const GroupNode *group : collector.groups)
                group_generated_.erase(group);
            group_generated_.merge(snapshot);
            break;
        }
        accepted++;
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
