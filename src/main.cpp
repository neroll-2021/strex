#include <exception>
#include <string>
#include <print>
#include <iostream>

#include <strex/compile_option.hpp>
#include <strex/strex.hpp>

#include <argparse/argparse.hpp>

int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::string regex_string;
        while (std::getline(std::cin, regex_string)) {
            strex::compiled_regex regex(regex_string);
            std::println("{}", strex::from_regex(regex));
        }
    }

    argparse::ArgumentParser program("strex", "strex 0.1.0");

    program.add_description("Generate strings that match the given  regular expression.");

    program.add_argument("-r", "--regex")
        .help("regular expression that used to generate string")
        .metavar("<str>")
        .store_into(strex::compile_option::base_regex)
        .required();

    program.add_argument("-n", "--number")
        .help("number of string to be generated")
        .default_value(1)
        .nargs(1)
        .scan<'u', unsigned int>()
        .store_into(strex::compile_option::generate_count)
        .metavar("<integer>");

    try {
        program.parse_args(argc, argv);

        strex::compiled_regex regex(strex::compile_option::base_regex);
        while (strex::compile_option::generate_count--) {
            std::println("{}", strex::from_regex(regex));
        }
    }
    catch (std::exception &e) {
        std::println("{}", e.what());
        std::print("{}", program.help().str());
        return 1;
    }
}
