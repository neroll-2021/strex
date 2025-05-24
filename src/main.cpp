#include <print>

#include <strex/strex.hpp>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::println("usage: ./strex <regex>");
        return 1;
    }

    strex::compiled_regex regex(argv[1]);
    std::println("{}", strex::from_regex(regex));
}
