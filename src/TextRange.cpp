#include <algorithm>
#include <cassert>

#include <strex/TextRange.hpp>

auto strex::range_union(const TextRange &x, const TextRange &y) -> TextRange {
    auto start = std::min(x.start, y.start);
    auto end = std::max(x.end, y.end);
    return {start, end};
}