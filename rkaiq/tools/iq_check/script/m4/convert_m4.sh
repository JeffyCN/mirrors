#!/bin/sh -e

DIR=${0%/*}
PATH=$PATH:$DIR
IN_DIR=${1:-.}
OUT_DIR=${2:-generated}

rm -rf $OUT_DIR
mkdir -p $OUT_DIR

HEADERS=$(cd $IN_DIR; find . -name "*.h.in")

for f in $HEADERS; do
	IN=$IN_DIR/$f
	OUT=$OUT_DIR/${f%.*}

	echo converting $IN to $OUT...

	mkdir -p $(dirname $OUT)
	cat $DIR/desc.m4 $IN |m4 -E - > $OUT
done
