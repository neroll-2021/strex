# Strex

[English](README.md)

Strex 根据给定的正则表达式生成匹配的随机字符串。其语法和匹配规则遵循 ECMAScript 标准。Strex 既可以作为命令行工具使用，也可以作为 C++23 库使用。

本项目受 [daidodo/regxstring](https://github.com/daidodo/regxstring) 和 [elarsonSU/egret](https://github.com/elarsonSU/egret) 启发。

## 环境要求

- 支持 C++23 的编译器（GCC 14+、Clang 18+ 或 MSVC 17.8+）
- [XMake](https://xmake.io/) 2.9.8+ 或 [CMake](https://cmake.org/) 3.28+
- Linux 或 Windows
- 首次构建需要联网，因为需要下载依赖

## 构建

### XMake

```sh
xmake
```

### CMake

```sh
cmake -S . -B build
cmake --build build
```

## 安装

### XMake

```sh
xmake install -o path/to/install/dir
```

### CMake

```sh
cmake --install build --prefix path/to/install/dir
```

两条命令都会安装 `strex` 可执行文件、静态库和动态库，以及公开的头文件。

## 使用

### 命令行

从正则表达式生成字符串：

```sh
xmake run strex -r "<regex>"       # XMake 构建
./build/strex -r "<regex>"         # CMake 构建
```

使用 `-n` 一次生成多个字符串。下面的命令生成 10 个字符串：

```sh
xmake run strex -r "<regex>" -n 10
```

示例——生成 IPv4 地址：

```sh
$ ./build/strex -r "((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.){3}((25[0-5]|(2[0-4]|1\d|[1-9]|)\d))" -n 5
2.230.109.255
23.254.57.0
176.40.252.42
235.43.9.252
2.218.3.239
```

运行 `strex --help` 查看所有选项。

不带参数运行 `strex` 时，它会逐行读取标准输入中的正则表达式，并为每行打印一个生成的字符串。非法的模式会打印错误信息，程序继续读取：

```sh
$ strex
a|b|c
a
\d{3}-\d{4}
451-7826
```

### 库

生成随机 IPv4 地址：

```c++
#include <print>
#include <strex/strex.hpp>

int main() {
    const char *regex = R"(((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.){3}((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)))";
    std::println("{}", strex::from_regex(regex));
}
```

`strex::from_regex(std::string_view regex)` 每次调用都会**重新解析**正则表达式。要基于同一个正则表达式生成多个字符串，先用 `strex::ParsedRegex` 解析一次：

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

传入种子可以得到固定的结果：相同的种子和正则表达式总是生成相同的字符串：

```c++
#include <print>
#include <strex/strex.hpp>

int main() {
    const char *regex = R"(((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.){3}((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)))";
    strex::ParsedRegex parsed(regex);
    for (int i = 0; i < 10; i++)
        // 所有字符串都相同。
        std::println("{}", strex::from_regex(parsed, 0));
}
```

非法的正则表达式会使 `strex::ParsedRegex` 和 `from_regex` 函数抛出异常：词法错误抛出 `strex::LexicalError`，语法错误抛出 `strex::ParseError`，不支持的语法抛出 `strex::SyntaxNotSupport`。它们都继承自 `std::runtime_error`。

## 支持的语法

| 语法 | 说明 |
| ------ | ------ |
| `abc` | 字面字符 |
| `.` | 任意字符 |
| `[abc]`、`[^abc]`、`[a-z]` | 字符集、取反字符集和范围 |
| `\d` `\D` `\s` `\S` `\w` `\W` | 预定义字符类 |
| `\f` `\n` `\r` `\t` `\v` `\\` `\'` `\"` `\cX` | 转义字符 |
| `\xHH`、`\uHHHH` | 十六进制转义字符（值最大到 0xFF） |
| `*` `+` `?` `{n}` `{n,}` `{n,m}` | 量词 |
| `a\|b` | 分支选择 |
| `(...)`、`(?:...)`、`(?<name>...)` | 捕获组、非捕获组和命名组 |
| `\1` ... `\255`、`\k<name>` | 反向引用 |

每次重复都独立进行，所以 `(a|b){3}` 可能生成 `bab` 这样的混合结果。反向引用所指向的组没有被选中时，反向引用产生空串，例如 `(?:(a)|b)\1` 会生成 `aa` 或 `b`。

不支持：

- 锚点 `^` 和 `$`
- 词边界 `\b` 和 `\B`
- 先行断言和后行断言：`(?=...)`、`(?!...)`、`(?<=...)`、`(?<!...)`
- Unicode

## 限制

- 无上限的量词会被设置 3 次的重复上限：`x*` 重复 0 到 3 次，`x+` 重复 1 到 3 次，`x{n,}` 重复 n 到 n+3 次。实际次数在相应范围内随机选取。
- 一个正则表达式最多包含 255 个分组。
- 匹配不到任何字符串的正则表达式（如 `[]`）会被报错拒绝。
- 精确重复没有上限。`x{1000000000}` 会被接受，程序会尝试构造完整的字符串，巨大的重复次数可能耗尽内存。
- 字符集和 `.` 仅限于 ASCII。`.` 和取反字符集（如 `[^a]`）可能产生控制字符，例如 `\x01`。

## 已知问题

- 并行解析正则表达式不安全：库内部的字符集缓存没有做同步。并行生成字符串是安全的。
- 深层嵌套的重复（如 `((((a{2}){2})...))`）会成倍累积出巨大的总量。此时程序可能不会以干净的内存耗尽错误退出，而是耗尽所有可用内存，拖慢整台机器。

## 开发

运行测试：

```sh
xmake test                # XMake
ctest --test-dir build    # CMake
```

构建并运行基准测试：

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

构建时开启 address、leak 和 undefined-behavior sanitizer（仅限 Linux 的 debug 模式）：

```sh
xmake f --dev=y -m debug && xmake
```

## 工作原理

Strex 分三步处理正则表达式：lexer 把模式拆分成 token，parser 把 token 转换成 AST，generator 遍历 AST 并随机选取字符。

## 项目结构

```
benchmark/      基准测试（nanobench）
include/strex/  头文件；只有 strex.hpp 是公开的
src/            库源码和命令行工具
test/           单元测试（doctest）
```

## 许可证

Strex 基于 [GPL-3.0](LICENSE) 许可证发布。
