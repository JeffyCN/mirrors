#!/bin/sh

# Format C/C++ source
# sudo apt install clang-format
find . -regex '.*\.\(cpp\|hpp\|cu\|c\|h\)' -exec clang-format -i {} \;

# Format Python source
# pip3 install yapf
find . -iname '*.py' -exec python3 -m yapf --in-place --recursive --style="{indent_width: 4}" {} \;

# Format Android.bp
# bpfmt is provided in AOSP
bpfmt -w .
