#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <functional>
#include <random>
#include <string>
#include <string_view>

#include <strex/AST.hpp>
#include <strex/Charset.hpp>
#include <strex/Exception.hpp>
#include <strex/Generator.hpp>

strex::Generator::Generator(const ASTNode *ast) : ast_(ast) {
    assert(ast != nullptr);
}

std::string strex::Generator::generate() {
    generated_string_.clear();
    group_generated_.clear();
    generate(ast_);
    assert(std::ranges::all_of(generated_string_, ::isprint));
    return generated_string_;
}

void strex::Generator::generate(const ASTNode *node) {
    node->accept(this);
}

void strex::Generator::visit(const TextNode *node) {
    generated_string_.append(node->text());
}

static std::string exclude(std::string_view except);

static std::string remove_unprintable(std::string &characters);

void strex::Generator::visit(const CharsetNode *node) {
    const Charset *charset = node->charset();
    std::string characters{charset->alphabet()};
    if (!charset->is_inclusive())
        characters = exclude(characters);

    remove_unprintable(characters);

    if (characters.empty())
        return;

    std::uniform_int_distribution<std::size_t> random(0, characters.size() - 1);

    generated_string_.push_back(characters[random(engine_)]);
}

void strex::Generator::visit(const SequenceNode *node) {
    for (const auto &element : node->sequence()) {
        generate(element.get());
    }
}

void strex::Generator::visit(const RepeatNode *node) {
    int lower = node->repeat_lower();
    int upper = node->repeat_upper();

    std::uniform_int_distribution<int> random(lower, upper);

    int repeat_count = random(engine_);
    while (repeat_count--)
        generate(node->content());
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

std::string remove_unprintable(std::string &characters) {
    assert(std::ranges::is_sorted(characters));
    auto [last, end] = std::ranges::remove_if(characters, std::not_fn(::isprint));
    characters.erase(last, end);
    return characters;
}