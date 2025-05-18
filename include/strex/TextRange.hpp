#ifndef NEROLL_STREX_TEXT_RANGE_HPP
#define NEROLL_STREX_TEXT_RANGE_HPP

#include <cassert>
#include <cstddef>

namespace strex {

/// A [start, end) range in regular expression, counted by byte offset, 0-based.
struct TextRange {

    TextRange() = default;

    TextRange(std::size_t start, std::size_t end) : start(start), end(end) {}

    std::size_t start;
    std::size_t end;
};

TextRange union_range(const TextRange &x, const TextRange &y);

} // namespace strex

#endif