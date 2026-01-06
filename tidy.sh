#!/usr/bin/env bash
find src -path 'src/codegen' -prune -o \( -name '*.cpp' -o -name '*.hpp' \) -type f -print0 | xargs -0 -n1 -P"$(nproc)" clang-tidy -p build