#include <algorithm>
#include <cassert>
#include <set>
#include <string_view>
#include <tuple>

#include <strex/Charset.hpp>

#define DIGIT_CHARACTERS "0123456789"
#define UPPER_CHARACTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWER_CHARACTERS "abcdefghijklmnopqrstuvwxyz"
#define SPACE_CHARACTERS " \t\r\n"
#define UNDER_SCROLL     "_"
#define WORD_CHARACTERS  DIGIT_CHARACTERS UPPER_CHARACTERS LOWER_CHARACTERS UNDER_SCROLL

auto strex::Charset::get(std::string alphabet, bool is_inclusive) -> const Charset * {
    static std::set<Charset> charsets;
    std::ranges::sort(alphabet);
    auto [first, last] = std::ranges::unique(alphabet);
    alphabet.erase(first, last);
    auto [iter, _] = charsets.insert({std::move(alphabet), is_inclusive});
    return &(*iter);
}

auto strex::Charset::from_char_class(char char_class) -> const Charset * {
    switch (char_class) {
        case 'd':
            return digits();
        case 'D':
            return non_digit();
        case 's':
            return space();
        case 'S':
            return non_space();
        case 'w':
            return word();
        case 'W':
            return non_word();
        case '.':
            return any();
        default:
            assert(false && "[Charset::from_char_class] invalid char class character");
    }
}

auto strex::Charset::digits() -> const Charset * {
    return get(DIGIT_CHARACTERS, true);
}

auto strex::Charset::non_digit() -> const Charset * {
    return get(DIGIT_CHARACTERS, false);
}

auto strex::Charset::word() -> const Charset * {
    return get(WORD_CHARACTERS, true);
}

auto strex::Charset::non_word() -> const Charset * {
    return get(WORD_CHARACTERS, false);
}

auto strex::Charset::space() -> const Charset * {
    return get(SPACE_CHARACTERS, true);
}

auto strex::Charset::non_space() -> const Charset * {
    return get(SPACE_CHARACTERS, false);
}

auto strex::Charset::any() -> const Charset * {
    auto all_characters = [] {
        std::string s;
        s.resize_and_overwrite(128, [](char *s, std::size_t) {
            int i;
            char ch = '\0';
            for (i = 0; i < 127; i++, ch++) {
                s[i] = ch;
                // Skip newline characters as they cannot be matched by `.`.
                if (ch == '\n')
                    i--;
            }
            return i;
        });
        return s;
    }();
    return get(std::move(all_characters), true);
}

std::string_view strex::Charset::alphabet() const {
    assert(std::ranges::is_sorted(alphabet_));
    return alphabet_;
}

bool strex::Charset::operator<(const Charset &other) const {
    return std::tie(alphabet_, is_inclusive_) < std::tie(other.alphabet_, other.is_inclusive_);
}

strex::Charset::Charset(std::string alphabet, bool is_inclusive)
    : alphabet_(std::move(alphabet)), is_inclusive_(is_inclusive) {}