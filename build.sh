#!/bin/bash

mkdir -v build
cmake -DCMAKE_BUILD_TYPE=Debug -B build 
make -C build