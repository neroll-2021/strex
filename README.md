# Strex

[简体中文](README_zh.md)

Strex generates random strings that match a given regular expression. The syntax and the matching rules follow the ECMAScript standard closely. It works both as a command-line tool and as a C++23 library.

This project is inspired by [daidodo/regxstring](https://github.com/daidodo/regxstring) and [elarsonSU/egret](https://github.com/elarsonSU/egret).

## Requirements

- A compiler with C++23 support (GCC 14+, Clang 18+, or MSVC 17.8+)
- [XMake](https://xmake.io/) 2.9.8+ or [CMake](https://cmake.org/) 3.28+
- Linux or Windows
- Network access on the first build, because dependencies are downloaded automatically

## Build

### XMake

```sh
xmake
```

### CMake

```sh
cmake -S . -B build
cmake --build build
```

## Install

### XMake

```sh
xmake install -o path/to/install/dir
```

### CMake

```sh
cmake --install build --prefix path/to/install/dir
```

Both commands install the `strex` executable, the static and shared libraries, and the public header.

## Usage

### Command line

Generate a string from a regular expression:

```sh
xmake run strex -r "<regex>"       # built with XMake
./build/strex -r "<regex>"         # built with CMake
```

Use `-n` to generate several strings at once. This command generates 10 strings:

```sh
xmake run strex -r "<regex>" -n 10
```

Example — generate IPv4 addresses:

```sh
$ ./build/strex -r "((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.){3}((25[0-5]|(2[0-4]|1\d|[1-9]|)\d))" -n 5
2.230.109.255
23.254.57.0
176.40.252.42
235.43.9.252
2.218.3.239
```

Run `strex --help` to see all options.

When `strex` is started without arguments, it reads regular expressions from standard input, one per line, and prints a generated string for each line. Invalid patterns print an error message, and the program keeps reading:

```sh
$ strex
a|b|c
a
\d{3}-\d{4}
451-7826
```

### Library

Generate a random IPv4 address:

```c++
#include <print>
#include <strex/strex.hpp>

int main() {
    const char *regex = R"(((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.){3}((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)))";
    std::println("{}", strex::from_regex(regex));
}
```

`strex::from_regex(std::string_view regex)` parses the regular expression **every time** it is called. To generate many strings from the same expression, parse it once with `strex::ParsedRegex`:

```c++
#include <print>
#include <strex/strex.hpp>

int main() {
    const char *regex = R"(((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.){3}((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)))";
    strex::ParsedRegex parsed(regex);
    for (int i = 0; i < 10; i++)
        std::println("{}", strex::from_regex(parsed));
}
```

Pass a seed to get a fixed result. The same seed and regex always produce the same string:

```c++
#include <print>
#include <strex/strex.hpp>

int main() {
    const char *regex = R"(((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.){3}((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)))";
    strex::ParsedRegex parsed(regex);
    for (int i = 0; i < 10; i++)
        // All strings will be the same.
        std::println("{}", strex::from_regex(parsed, 0));
}
```

An invalid pattern makes `strex::ParsedRegex` and the `from_regex` functions throw an exception: `strex::LexicalError` for lexical errors, `strex::ParseError` for grammar errors, and `strex::SyntaxNotSupport` for unsupported syntax. All of them derive from `std::runtime_error`.

## Supported syntax

| Syntax | Description |
| ------ | ----------- |
| `abc` | literal characters |
| `.` | any character |
| `[abc]`, `[^abc]`, `[a-z]` | character classes, negated classes, and ranges |
| `\d` `\D` `\s` `\S` `\w` `\W` | predefined character classes |
| `\f` `\n` `\r` `\t` `\v` `\\` `\'` `\"` `\cX` | escaped characters |
| `\xHH`, `\uHHHH` | escaped characters in hex form (values up to 0xFF) |
| `*` `+` `?` `{n}` `{n,}` `{n,m}` | quantifiers |
| `a\|b` | alternation |
| `(...)`, `(?:...)`, `(?<name>...)` | capturing, non-capturing, and named groups |
| `\1` ... `\255`, `\k<name>` | backreferences |

Each repetition runs on its own, so `(a|b){3}` can produce mixed output such as `bab`. A backreference produces nothing when its group did not run, for example `(?:(a)|b)\1` generates `aa` or `b`.

Not supported:

- anchors `^` and `$`
- word boundaries `\b` and `\B`
- lookahead and lookbehind: `(?=...)`, `(?!...)`, `(?<=...)`, `(?<!...)`
- Unicode

## Limitations

- Open-ended quantifiers have an upper bound of 3: `x*` repeats 0 to 3 times, `x+` repeats 1 to 3 times, and `x{n,}` repeats n to n+3 times. The actual count is picked at random from that range.
- A pattern can contain at most 255 groups.
- A pattern that matches no string, such as `[]`, is rejected with an error.
- Exact repeats have no upper bound. `x{1000000000}` is accepted and the program tries to build the full string, so huge counts can run out of memory.
- Character classes and `.` are limited to ASCII. `.` and negated classes such as `[^a]` can produce control characters, for example `\x01`.

## Known issues

- Parsing patterns in parallel is not safe: the character set cache inside the library is not synchronized. Generating strings in parallel is safe.
- Deeply nested repeats, such as `((((a{2}){2})...))`, multiply into huge totals. Instead of failing with a clean out-of-memory error, the program can consume all available memory and slow down the whole machine.

## Development

Run the tests:

```sh
xmake test                # XMake
ctest --test-dir build    # CMake
```

Build and run the benchmarks:

**XMake**

```sh
xmake f -m release --enable_benchmarks=y
xmake
xmake run bench
```

**CMake**

```sh
cmake -S . -B build -DSTREX_ENABLE_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/bench
```

Build with address, leak, and undefined-behavior sanitizers (Linux, debug mode only):

```sh
xmake f --dev=y -m debug && xmake
```

## How it works

Strex processes a regular expression in three steps: the lexer splits the pattern into tokens, the parser turns the tokens into AST, and the generator walks the AST and picks characters at random.

## Project structure

```
benchmark/      benchmarks (nanobench)
include/strex/  headers; strex.hpp is the only public one
src/            library sources and the command-line tool
test/           unit tests (doctest)
```

## License

Strex is licensed under the [GPL-3.0](LICENSE).
