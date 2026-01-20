#!/bin/bash
mkdir -vp build/codegen
cmake -DCMAKE_BUILD_TYPE=Debug -B build 
cmake --build build --parallel