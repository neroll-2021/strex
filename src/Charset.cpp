#include <algorithm>
#include <cassert>
#include <numeric>
#include <set>
#include <tuple>

#include <strex/Charset.hpp>

#define DIGIT_CHARACTERS "0123456789"
#define UPPER_CHARACTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWER_CHARACTERS "abcdefghijklmnopqrstuvwxyz"
#define SPACE_CHARACTERS " \t\r\n"
#define UNDER_SCROLL "_"

auto strex::Charset::get(std::string alphabet, bool is_inclusive) -> const Charset & {
    static std::set<Charset, std::less<>> charsets;
    std::ranges::sort(alphabet);
    auto [iter, _] = charsets.insert({std::move(alphabet), is_inclusive});
    return *iter;
}

auto strex::Charset::from_char_class(char char_class) -> const Charset & {
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

auto strex::Charset::digits() -> const Charset & {
    return get(DIGIT_CHARACTERS, true);
}

auto strex::Charset::non_digit() -> const Charset & {
    return get(DIGIT_CHARACTERS, false);
}

auto strex::Charset::word() -> const Charset & {
    return get(DIGIT_CHARACTERS UPPER_CHARACTERS LOWER_CHARACTERS UNDER_SCROLL, true);
}

auto strex::Charset::non_word() -> const Charset & {
    return get(DIGIT_CHARACTERS UPPER_CHARACTERS LOWER_CHARACTERS UNDER_SCROLL, false);
}

auto strex::Charset::space() -> const Charset & {
    return get(SPACE_CHARACTERS, true);
}

auto strex::Charset::non_space() -> const Charset & {
    return get(SPACE_CHARACTERS, false);
}

auto strex::Charset::any() -> const Charset & {
    std::string all_characters(127, '\0');
    std::ranges::iota(all_characters, '\0');
    return get(std::move(all_characters), true);
}

bool strex::Charset::operator<(const Charset &other) const {
    return std::tie(alphabet_, is_inclusive_) < std::tie(other.alphabet_, other.is_inclusive_);
}

strex::Charset::Charset(std::string alphabet, bool is_inclusive)
    : alphabet_(std::move(alphabet)), is_inclusive_(is_inclusive) {}