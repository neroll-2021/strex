# Strex
[简体中文](README_zh.md)

A random string generator that generates strings from regular expression.

This project is inspired by [daidodo/regxstring](https://github.com/daidodo/regxstring) and [elarsonSU/egret](https://github.com/elarsonSU/egret).

## Build
### XMake
This project uses [XMake](https://xmake.io/) as the build system. XMake is not only a simple and easy-to-use build system, but also a powerful package manager. Strex dependents on [argparse](https://github.com/p-ranav/argparse) and [doctest](https://github.com/p-ranav/argparse), using XMake makes it easy to manage dependencies.

To build Strex, first enter the root directory of project, and then execute `xmake`in terminal. If you have not previously installed argparse and doctest using xmake, xmake will prompt you to install these dependencies. In this case, simply enter `y` to continue.

Wait for a while, the project will be built.

### CMake
At the root directory of the project, enter these commands in terminal.

```shell
mkdir build
cd build
cmake ..
```

If CMake choose *Makefile* as generator, then enter `make` in terminal to build the project.

## Run
### XMake
After building the project, run it using `xmake run strex`. You can pass command-line arguments to the program, for example, you can enter `xmake run strex --help` to display help information. Anything after `xmake run strex` will be treated as command-line arguments.

Enter `xmake run strex -r "<regex>"` to generate a random string based on a regular expression. Note that if your regular expression contains double quotes, you may need to add a backslash before them to escape them.

To generate more than one string, you can use '-n' to specify the number of strings you want to generate. For example, enter `xmake run strex -r "<regex> -n 10"` to generate 10 strings that match the regular expression.

### CMake
After building the project, enter `./strex` in `build` directory that you have created, then the program should be running.

To know usage of Strex, enter `./strex --help` to print help information.

## Install
### XMake
#### Linux
After building the project, enter `xmake install` to install Strex. By default, the installation directory is `/usr/local`. You can use `-o` to specify the installation location, for example, `xmake install -o path/to/an/empty/directory`. If you have a permission error, add `--admin` option and try again.

By the way, `/usr/local/lib` is not in the default search path in most Linux distributions. If you want to use Strex as a library in your program, you may need to manually specify the link options, or you can add `/usr/local/lib` to the default search path.

#### Windows
After building the project, enter `xmake install -o "path\to\an\empty\directory"` to install Strex to the specified directory. If you have a permission error, add `--admin` option and try again.

### CMake
At the `build` directory, enter `cmake .. -DCMAKE_INSTALL_PREFIX=path/to/an/empty/directory` to config the installation directory. Then execute `make install` to install Strex.

If `CMAKE_INSTALL_PREFIX` is not specified, the default installation path is `/usr/local` on Linux, `C:\Program Files (x86)\strex` on Windows.

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

`std::string strex::from_regex(std::string_view regex)` will parse the regular expression **every time** it is called. If you want to generate multiple strings based on the same regular expression, you should use `strex::ParsedRegex` to reduce unnecessary parsing.

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