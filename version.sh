#!/bin/bash
rm -f version.h

COMMIT_ID=$(cd $(dirname $0) && git log -1 | awk 'NR==1{print}')
SUB_COMMIT_ID=$(echo "$COMMIT_ID" | cut -c8-14)
CUR_TIME=$(date "+%G-%m-%d %H:%M:%S")
RK_GRAPHICS_VERSION=$SUB_COMMIT_ID+"$CUR_TIME"

#Only when compiling with CMAKE in linux, will the current directory generate version.h.
if [ $TARGET_PRODUCT ];then
	cat $(dirname $0)/version.h.template | sed "s/\$FULL_VERSION/$RK_GRAPHICS_VERSION/g"
elif [ "$1" = "meson" ];then
	echo "$RK_GRAPHICS_VERSION"
else
	cat $(dirname $0)/version.h.template | sed "s/\$FULL_VERSION/$RK_GRAPHICS_VERSION/g" > $(dirname $0)/version.h
	echo "Generated version.h"
fi
