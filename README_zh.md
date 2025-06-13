# Strex
[English](README.md)

一个基于正则表达式的随机字符串生成器。

本项目受 [daidodo/regxstring](https://github.com/daidodo/regxstring) 和 [elarsonSU/egret](https://github.com/elarsonSU/egret) 启发。

## 构建
### XMake
本项目使用 [XMake](https://xmake.io/) 作为构建系统。XMake 不仅是一个简单易用的构建系统，同时也是强大的包管理器。Strex 依赖 [argparse](https://github.com/p-ranav/argparse) 和 [doctest](https://github.com/p-ranav/argparse)，使用 XMake 可以使依赖管理更方便。

要构建 Strex，首先进入项目根目录，在命令行中输入 `xmake`。如果你此前没有使用 xmake 安装 argparse 和 doctest，xmake 将提示你安装这些依赖，在这种情况下，输入 `y` 安装依赖。

稍等片刻，项目就会构建完毕。

### CMake
在项目的根目录下输入以下命令：

```shell
mkdir build
cd build
cmake ..
```

如果 CMake 选择 *Makefile* 作为生成器，那么在命令行中输入 `make` 来构建项目。

## 运行
### XMake
构建完项目后，输入 `xmake run strex` 运行程序。你可以给程序传递命令行参数，例如，你可以输入 `xmake run strex --help` 展示帮助信息。在 `xmake run strex` 之后的内容都会作为命令行参数传递给程序。

输入 `xmake run strex -r "<regex>"` 生成基于正则表达式的随机字符串。注意，如果你的正则表达式中含有双引号，请在前面加上反斜杠转义。

要生成多个字符串，你可以使用 '-n' 选项来指定你要生成的数量。例如，输入 `xmake run strex -r "<regex> -n 10"` 会生成 10 个匹配正则表达式的字符串。

### CMake
构建完项目后，在刚刚创建的 `build` 目录中输入 `./strex` 运行程序。

要了解 Strex 的更多用法，输入 `./strex --help` 打印帮助信息。

## 安装
### XMake
#### Linux
构建完项目后，输入 `xmake install` 安装 Strex。默认情况下，安装目录是 `/usr/local`，可以使用 `-o` 选项指定安装位置，例如 `xmake install -o path/to/an/empty/directory`。如果出现访问权限错误，试着加上 `--admin` 选项重试。

另外，`/usr/local/lib` 在大多数 Linux 发行版中不在默认的搜索路径中。如果你想在自己的代码中将 Strex 作为库使用，你可能需要手动指定链接选项，或者你可以将 `/usr/local/lib` 加入到默认搜索路径中。

#### Windows
构建完项目后，输入 `xmake install -o "path\to\an\empty\directory"` 将 Strex 安装到指定目录中。如果出现访问权限错误，试着加上 `--admin` 选项重试。

### CMake
在 `build` 目录中，输入 `cmake .. -DCMAKE_INSTALL_PREFIX=path/to/an/empty/directory` 配置安装目录，然后执行 `make install` 安装 Strex。

## 示例
生成随机 IPv4 地址。

```c++
#include <print>
#include <strex/strex.hpp>

int main() {
    const char *regex = R"(((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.){3}((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)))";
    std::println("{}", strex::from_regex(regex));
}
```

`std::string strex::from_regex(std::string_view regex)` 会在**每次**调用时都解析正则表达式。如果你想基于同一个正则表达式生成多个字符串，应该使用 `strex::ParsedRegex` 以避免不必要的重复解析。

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