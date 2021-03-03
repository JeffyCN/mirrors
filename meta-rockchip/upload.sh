#!/bin/sh -x

FORCE=${1}
COMMIT=master

git push git ${FORCE}$COMMIT:master&
git push rk ${FORCE}$COMMIT:yocto-next&
git push gitrk ${FORCE}$COMMIT:yocto-next&
wait
