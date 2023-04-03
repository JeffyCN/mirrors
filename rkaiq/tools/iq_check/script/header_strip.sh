#!/bin/sh -e

INPUT=$1
OUTPUT=$2

# Drop all global headers
cat $INPUT |tac |sed '/^#.*"\/usr\//,$d' |sed '/__fsid_t/d' |tac |grep -v "^#" | sed -e 's/_Bool/_Bool\n/'  > $OUTPUT
