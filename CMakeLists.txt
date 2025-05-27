cmake_minimum_required(VERSION 4.0.0)

project(strex VERSION 0.1.0
              DESCRIPTION "A random string generator that generates strings from regular expression."
              LANGUAGES CXX)

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
target_include_directories(static_library PUBLIC include PUBLIC "${argparse_SOURCE_DIR}/include")
set_target_properties(static_library PROPERTIES OUTPUT_NAME strex_static)

# shared library
add_library(shared_library SHARED ${LIBRARY_SOURCE_FILES})
target_include_directories(shared_library PUBLIC include PUBLIC "${argparse_SOURCE_DIR}/include")
set_target_properties(shared_library PROPERTIES OUTPUT_NAME strex_shared)

# executable
add_executable(strex src/main.cpp src/compile_option.cpp)
target_link_libraries(strex PRIVATE static_library)

# tests
option(ENABLE_TESTS "Enable tests" ON)
if(ENABLE_TESTS)
    message(STATUS "enable tests")

    include(CTest)
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG        v2.4.12
    )
    FetchContent_MakeAvailable(doctest)

    add_subdirectory(test/)
endif()

install(TARGETS strex DESTINATION bin)
install(TARGETS static_library LIBRARY DESTINATION lib)
install(TARGETS shared_library LIBRARY DESTINATION lib
                               RUNTIME DESTINATION bin)
install(DIRECTORY include/strex DESTINATION include)
install(DIRECTORY "${argparse_SOURCE_DIR}/include/argparse" DESTINATION include)