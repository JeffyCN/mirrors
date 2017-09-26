#!/bin/bash
rm -f version.h

COMMIT_ID=$(cd $(dirname $0) && git log -1 | awk 'NR==1{print}')
SUB_COMMIT_ID=$(echo "$COMMIT_ID" | cut -c8-14)
CUR_TIME=$(date "+%G-%m-%d %H:%M:%S")
RK_GRAPHICS_VERSION=$SUB_COMMIT_ID+"$CUR_TIME"

cd $(dirname $0) && cat version.h.template | sed "s/\$FULL_VERSION/$RK_GRAPHICS_VERSION/g" > version.h
echo "Generated version.h"
