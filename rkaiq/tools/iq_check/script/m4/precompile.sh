#!/bin/sh -e

INPUT=$1
OUTPUT=$2

# Drop all global headers
cat $INPUT |tac |sed '/^#.*"\/usr\//,$d' |tac |grep -v "^#" > $OUTPUT
