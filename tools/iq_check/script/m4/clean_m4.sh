#!/bin/sh -e

IN_DIR=${1:-.}
OUT_DIR=${2:-generated}

rm -rf $OUT_DIR
mkdir -p $OUT_DIR

HEADERS=$(cd $IN_DIR; find . -name "*.h.in")

for f in $HEADERS; do
	IN=$IN_DIR/$f
	OUT=$OUT_DIR/${f%.*}

	echo clean $IN to $OUT...

	mkdir -p $(dirname $OUT)
	grep -v "\<M4_" $IN > $OUT
done
