#!/bin/bash

INPUT=$1
OUTPUT=$2

HEADER="#include \"$(basename $INPUT)\""

echo $HEADER

echo $HEADER > $OUTPUT
