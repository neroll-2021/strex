# Strex
A random string generator that generates strings from regular expression.

This project is inspired by [daidodo/regxstring](https://github.com/daidodo/regxstring) and [elarsonSU/egret](https://github.com/elarsonSU/egret).

## Build
### XMake
This project uses [XMake](https://xmake.io/) as the build system. XMake is not only a simple and easy-to-use build system, but also a powerful package manager. Strex dependents on [argparse](https://github.com/p-ranav/argparse) and [doctest](https://github.com/p-ranav/argparse), using XMake makes it easy to manage dependencies.

To build Srex, first enter the root directory of project, and then execute `xmake`in terminal. If you have not previously installed argparse and doctest using xmake, xmake will prompt you to install these dependencies. In this case, simply enter `y`.

Wait for a while, the project will be built.

## Run
### XMake
After building the project, run it using `xmake run strex`. You can pass command-line arguments to the program, for example, you can enter `xmake run strex --help` to display help information. Anything after `xmake run strex` will be treated as command-line arguments.

Enter `xmake run strex - r "<regex>"` to generate a random string based on a regular expression. Note that if your regular expression contains double quotes, you may need to add a backslash before them to escape them.

To generate more than one string, you can use '-n' to specify the number of strings you want to generate. For example, enter `xmake run strex -r "<regex> -n 10"` to generate 10 strings that match the regular expression.

## Install
### XMake
#### Linux
After building the project, enter `xmake install` to install Strex. By default, the installation directory is `/usr/local`. You can use `-o` to specify the installation location, for example, `xmake install -o path/to/an/empty/directory`. If you have a permission error, add `--admin` option and try again.

By the way, `/usr/local/lib` is not in the default search path in most Linux distributions. If you want to use Strex as a library in your program, you may need to manually specify the link options, or you can add `/usr/local/lib` to the default search path.

### Windows
After building the project, enter `xmake install -o "path\to\an\empty\directory"` to install Strex to the specified directory. If you have a permission error, add `--admin` option and try again.

## Examples
Generate IPv4 address.

```c++
#include <print>
#include <strex/strex.hpp>

int main() {
    const char *regex = R"(((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.){3}((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)))";
    std::println("{}", strex::from_regex(regex));
}
```

`std::string strex::from_regex(std::string_view regex)` will parse the regular expression **every time** it is called. If you want to generate multiple strings based on the same regular expression, you should use `strex::compiled_regex` to reduce unnecessary parsing.

```c++
#include <print>
#include <strex/strex.hpp>

int main() {
    const char *regex = R"(((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.){3}((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)))";
    strex::compiled_regex cr(regex);
    for (int i = 0; i < 10; i++)
        std::println("{}", strex::from_regex(cr));
}
```