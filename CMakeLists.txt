cmake_minimum_required(VERSION 3.27)


project(Compiler CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find dependencies

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Flex & Bison

set(LEXER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/codegen/lexer.l")
set(PARSER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/codegen/parser.y")


set(LEXER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/codegen")
set(PARSER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/codegen")


flex_target(LEXER ${LEXER_SOURCE} ${LEXER_OUTPUT_DIR}/lexer.cpp
           DEFINES_FILE ${LEXER_OUTPUT_DIR}/lexer.hpp)
bison_target(PARSER ${PARSER_SOURCE} ${PARSER_OUTPUT_DIR}/parser.tab.cpp
            DEFINES_FILE ${PARSER_OUTPUT_DIR}/parser.tab.hpp
            COMPILE_FLAGS "-v --language=c++ -Wconflicts-sr -Wcounterexamples")

add_flex_bison_dependency(LEXER PARSER)


# LLVM Defines

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/depends
    ${CMAKE_CURRENT_BINARY_DIR}
    ${LLVM_INCLUDE_DIRS}
)

# Create executable

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE DEPENDENCIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/depends/*.cpp")

add_executable(compiler
    ${BISON_PARSER_OUTPUTS}
    ${FLEX_LEXER_OUTPUTS}
    ${SOURCES}
    ${DEPENDENCIES}
)

set_target_properties(compiler PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_compile_options(compiler PRIVATE -Wall -Wextra -Wpedantic -Wno-sign-compare)

# Magically link to LLVM
llvm_config(compiler USE_SHARED support core irreader)


