function(add_test_case test_name file_name)
    add_executable("${test_name}" "${file_name}" "helper/ASTFormatter.cpp")
    target_include_directories("${test_name}" PRIVATE  helper
                                              PRIVATE "${doctest_SOURCE_DIR}")
    target_link_libraries("${test_name}" PRIVATE static_library)
    target_compile_definitions("${test_name}" PRIVATE DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)
    add_test(NAME "${test_name}" COMMAND "${test_name}")
endfunction()

add_test_case(test_lexer Lexer.cpp)
add_test_case(test_parser Parser.cpp)
add_test_case(test_generator Generator.cpp)