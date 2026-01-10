#! /bin/bash

nasm -f elf64 ./stdlib/stdfunc.asm -o ./build/stdfunc.o
./build/compiler $1 > ./build/code.ll
clang ./build/code.ll ./build/stdfunc.o -o ./build/out
