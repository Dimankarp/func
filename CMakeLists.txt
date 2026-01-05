cmake_minimum_required(VERSION 3.27)


project(Compiler CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)


set(LEXER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/codegen/lexer.l")
set(PARSER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/codegen/parser.y")


set(LEXER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(PARSER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")


flex_target(LEXER ${LEXER_SOURCE} ${LEXER_OUTPUT_DIR}/lexer.cpp
           DEFINES_FILE ${LEXER_OUTPUT_DIR}/lexer.hpp)
bison_target(PARSER ${PARSER_SOURCE} ${PARSER_OUTPUT_DIR}/parser.tab.cpp
            DEFINES_FILE ${PARSER_OUTPUT_DIR}/parser.tab.hpp
            COMPILE_FLAGS "-v --language=c++ -Wconflicts-sr -Wcounterexamples")

add_flex_bison_dependency(LEXER PARSER)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Create executable
add_executable(compiler
    ${BISON_PARSER_OUTPUTS}
    ${FLEX_LEXER_OUTPUTS}
    ${SOURCES}
)

set_target_properties(compiler PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)