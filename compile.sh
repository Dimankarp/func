#! /bin/bash

nasm -f elf64 ./stdlib/stdfunc.asm -o ./build/stdfunc.o
./build/compiler $1 > ./build/out.ll
clang ./build/out.ll ./build/stdfunc.o -o ./build/out
