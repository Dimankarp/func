#!/usr/bin/env bash

set -euo pipefail

usage() {
  cat >&2 <<'EOF'
Usage: ./compile.sh [-o OUTPUT] [COMPILER_FLAGS...] <file1.fc> [file2.fc ...]

OPTIONS:
  -o NAME_OR_PATH   Output filename
  -h, --help        Show this help

COMPILER_FLAGS:
  -p, --print-ast
  --trace-parsing
  --trace-scanning
  -d, --debug
  -a, --alloc
  --arch <sim|x64>
EOF
}

out="out"
arch="x64"
inputs=()
compiler_args=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    -o)
      shift
      [[ $# -gt 0 ]] || { echo "error: -o requires an argument" >&2; usage; exit 2; }
      out="$1"
      shift
      ;;

    -p|--print-ast|--trace-parsing|--trace-scanning|-d|--debug|-a|--alloc)
      compiler_args+=("$1")
      shift
      ;;

    --arch)
      shift
      [[ $# -gt 0 ]] || { echo "error: --arch requires an argument" >&2; usage; exit 2; }
      arch="$1"
      compiler_args+=("--arch" "$1")
      shift
      ;;
    --arch=*)
      compiler_args+=("$1")
      shift
      ;;

    -h|--help)
      usage
      exit 0
      ;;

    -*)
      echo "error: unknown option: $1" >&2
      usage
      exit 2
      ;;

    *)
      inputs+=("$1")
      shift
      ;;
  esac
done

[[ ${#inputs[@]} -gt 0 ]] || { echo "error: no input files provided" >&2; usage; exit 2; }

mkdir -p ./build

if [[ "$arch" == "x64" ]]; then
nasm -f elf64 ./stdlib/stdfunc.asm -o ./build/stdfunc.o
fi

ll_files=()
for src in "${inputs[@]}"; do
  [[ -f "$src" ]] || { echo "error: input not found: $src" >&2; exit 2; }

  base="$(basename "$src")"
  base="${base%.*}"
  ll="./build/${base}.ll"

  ./build/compiler "${compiler_args[@]}" -o "$ll" "$src"
  ll_files+=("$ll")
done
 
if [[ "$out" == */* ]]; then
  clang_out="$out"
else
  clang_out="./build/$out"
fi

if [[ "$arch" == "x64" ]]; then
clang "${ll_files[@]}" ./build/stdfunc.o -o "$clang_out"
fi