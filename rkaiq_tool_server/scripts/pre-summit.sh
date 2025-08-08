#!/bin/bash

git diff -U0 --no-color HEAD\^ | ./scripts/clang-format-diff.py -p1 -i
