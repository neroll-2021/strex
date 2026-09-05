cmake_minimum_required(VERSION 3.28)

project(strex VERSION 0.2.1
              DESCRIPTION "A random string generator that generates strings from regular expression."
              LANGUAGES CXX)

option(STREX_ENABLE_TESTS "Enable tests" ON)
option(STREX_ENABLE_BENCHMARKS "Enable benchmarks" OFF)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "set build type to 'Release' by default")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
else()
    message(STATUS "build type: ${CMAKE_BUILD_TYPE}")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG        v3.2
)
FetchContent_MakeAvailable(argparse)

add_library(strict_check INTERFACE)
if(LINUX)
    target_compile_options(strict_check INTERFACE -Wall -Wextra -Werror)
endif()

set(LIBRARY_SOURCE_FILES src/AST.cpp
                         src/Charset.cpp
                         src/Generator.cpp
                         src/Lexer.cpp
                         src/Parser.cpp
                         src/strex.cpp
                         src/TextRange.cpp
                         src/Token.cpp)

# static library
add_library(static_library STATIC ${LIBRARY_SOURCE_FILES})
target_link_libraries(static_library PUBLIC argparse::argparse strict_check)
target_include_directories(static_library PUBLIC include)
set_target_properties(static_library PROPERTIES OUTPUT_NAME strex_static)

# shared library
add_library(shared_library SHARED ${LIBRARY_SOURCE_FILES})
target_link_libraries(shared_library PUBLIC argparse::argparse strict_check)
target_include_directories(shared_library PUBLIC include)
set_target_properties(shared_library PROPERTIES OUTPUT_NAME strex_shared)

# executable
add_executable(strex src/main.cpp src/compile_option.cpp)
target_link_libraries(strex PRIVATE static_library)

# tests
if(STREX_ENABLE_TESTS)
    message(STATUS "enable tests")

    include(CTest)
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG        v2.4.12
    )
    # doctest's install rules are not guarded for subproject usage,
    # disable them explicitly to avoid installing doctest with strex
    set(DOCTEST_NO_INSTALL ON)
    FetchContent_MakeAvailable(doctest)

    add_subdirectory(test/)
endif()

if(STREX_ENABLE_BENCHMARKS)
    message(STATUS "enable benchmarks")

    FetchContent_Declare(
        nanobench
        GIT_REPOSITORY https://github.com/martinus/nanobench.git
        GIT_TAG        v4.3.11
    )
    FetchContent_MakeAvailable(nanobench)
    add_executable(bench benchmark/main.cpp)
    target_link_libraries(bench PRIVATE static_library nanobench)
endif()

install(TARGETS strex DESTINATION bin)
install(TARGETS static_library LIBRARY DESTINATION lib)
install(TARGETS shared_library LIBRARY DESTINATION lib
                               RUNTIME DESTINATION bin)
install(FILES include/strex/strex.hpp DESTINATION include/strex)
