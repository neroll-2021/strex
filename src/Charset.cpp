#include <algorithm>
#include <set>
#include <tuple>

#include <strex/Charset.hpp>

#define DIGIT_CHARACTERS "0123456789"
#define UPPER_CHARACTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWER_CHARACTERS "abcdefghijklmnopqrstuvwxyz"
#define SPACE_CHARACTERS " \t\r\n"

auto strex::Charset::get(std::string alphabet, bool is_inclusive) -> const Charset & {
    static std::set<Charset> charsets;
    std::ranges::sort(alphabet);
    auto [iter, _] = charsets.insert({std::move(alphabet), is_inclusive});
    return *iter;
}

auto strex::Charset::digits() -> const Charset & {
    return get(DIGIT_CHARACTERS, true);
}

auto strex::Charset::non_digit() -> const Charset & {
    return get(DIGIT_CHARACTERS, false);
}

auto strex::Charset::word() -> const Charset & {
    return get(DIGIT_CHARACTERS UPPER_CHARACTERS LOWER_CHARACTERS, true);
}

auto strex::Charset::non_word() -> const Charset & {
    return get(DIGIT_CHARACTERS UPPER_CHARACTERS LOWER_CHARACTERS, false);
}

auto strex::Charset::space() -> const Charset & {
    return get(SPACE_CHARACTERS, true);
}

auto strex::Charset::non_space() -> const Charset & {
    return get(SPACE_CHARACTERS, false);
}

bool strex::Charset::operator<(const Charset &other) const {
    return std::tie(alphabet_, is_inclusive_) < std::tie(other.alphabet_, other.is_inclusive_);
}

strex::Charset::Charset(std::string alphabet, bool is_inclusive)
    : alphabet_(std::move(alphabet)), is_inclusive_(is_inclusive) {}