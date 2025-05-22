#ifndef NEROLL_STREX_CHARSET_HPP
#define NEROLL_STREX_CHARSET_HPP

#include <string>

namespace strex {

class Charset {
 public:
    // TODO Maybe can use enum instead of bool parameter.
    static const Charset *get(std::string alphabet, bool is_inclusive = true);

    static const Charset *from_char_class(char char_class);

    /// \d, [0-9]
    static const Charset *digits();

    /// \D, [^0-9]
    static const Charset *non_digit();

    /// \w, [a-zA-Z0-9_]
    static const Charset *word();

    /// \W, [^a-zA-Z0-9_]
    static const Charset *non_word();

    /// \s, [ \t\r\n]
    static const Charset *space();

    /// \S, [^ \t\r\n]
    static const Charset *non_space();

    static const Charset *any();

    bool is_inclusive() const { return is_inclusive_; }

    std::string alphabet() const;

    bool operator<(const Charset &other) const;

 private:
    Charset(std::string alphabet, bool is_inclusive);

    std::string alphabet_; ///< characters in charset
    bool is_inclusive_;    ///< if the charset is inclusive
};

} // namespace strex

#endif