#!/usr/bin/env bash
set -euo pipefail

find . \
  \( -path './build' -o -path './build/*' \
     -o -path './depends' -o -path './depends/*' \
     -o -path './src/codegen' -o -path './src/codegen/*' \) -prune -o \
  \( -name '*.cpp' -o -name '*.hpp' \) -type f -print0 \
| xargs -0 -n1 -P"$(nproc)" clang-tidy -p "./build"