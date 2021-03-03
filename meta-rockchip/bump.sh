#!/bin/sh -x

TAG=$(cd /ssd/misc/mpp-dev; git tag|tail -1)
sed -i "s/mpp-dev-[0-9_]*/$TAG/" recipes-multimedia/rockchip-mpp/rockchip-mpp.bb

TAG=$(cd /ssd/misc/kernel-4.19; git tag|tail -1)
sed -i "s/kernel-4.19-[0-9_]*/$TAG/" recipes-kernel/linux/linux-rockchip_4.19.bb
sed -i "s/kernel-4.19-[0-9_]*/$TAG/" recipes-kernel/linux-libc-headers/linux-libc-headers_4.19-custom.bb

cd ..
. ./oe-init-build-env

sed -i 's/^\(FREEZE_REV\).*/\1 = "1"/' conf/include/common.conf
bitbake -p -r conf/include/autorev.conf

cd conf
git checkout include/common.conf

cd ../../meta-rockchip
git commit -a -s -m "Bump BSP package revisions at $(date +%Y_%m_%d)"
