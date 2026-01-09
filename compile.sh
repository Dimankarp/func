#! /bin/bash

./build/compiler $1 > test.ll
clang test.ll ~/stdfunc/stdfunc.o

