#!/bin/sh

cd ..
. ./oe-init-build-env

sed -i 's/^\(FREEZE_REV\).*/\1 = "1"/' conf/include/common.conf
bitbake -p -r conf/include/autorev.conf -r conf/mirror.conf

cd conf
git checkout include/common.conf

cd ../../meta-rockchip
git commit -a -s -m "Bump BSP package revisions at $(date +%Y_%m_%d)"
