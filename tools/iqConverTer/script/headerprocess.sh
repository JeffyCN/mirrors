#!/bin/bash

INPUT=$1
OUTPUT=$2

HEADER="#include \"$(basename $(realpath --relative-to=. $INPUT))\""

echo $HEADER

echo $HEADER > $OUTPUT
