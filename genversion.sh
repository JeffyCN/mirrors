#!/bin/bash

CURRENTDIR=`dirname $0`

cd $CURRENTDIR

rm -f $CURRENTDIR/version.h

commit_ts=`git log -1 --format="%ct"`
commit_time=`date -d@$commit_ts +"%Y-%m-%d %H:%M:%S"`
current_time=`date +"%Y-%m-%d %H:%M:%S"`
git_version=`git log -1 --format="%h"`
current_product=$TARGET_PRODUCT

VERSION_TARGET="$(cat version.h.template | sed  -e 's/\$GIT_BUILD_VERSION/'"$git_version build: $current_time"'/g' -e 's/\$PRODUCT_BASE/'"$current_product"'/g' version.h.template)"

#Only when compiling with CMAKE in linux, will the current directory generate version.h.
if [ $TARGET_PRODUCT ];then
	if [ "$1" = "Android.mk" ];then
		echo RGA is compiling with Android.mk
		echo "${VERSION_TARGET}" > version.h
	elif [ "$1" = "Android.bp" ];then
		echo RGA is compiling with Android.bp
		echo "${VERSION_TARGET}" > version.h
		#echo "${VERSION_TARGET}"
	else
		echo $TARGET_PRODUCT requires subcommands!
		exit 1
	fi
elif [ "$1" = "meson" ];then
	echo RGA is compiling with meson
        echo "${VERSION_TARGET}"
elif [ "$1" = "cmake" ];then
	echo RGA is compiling with cmake
	echo "${VERSION_TARGET}" > version.h
else
        echo Requires subcommands!
        exit 2
fi

echo "Generated version.h"

