#!/bin/bash

find . -name "*.c" -o -name "*.h" -o -name "*.cpp" | xargs clang-format -style=LLVM -i
