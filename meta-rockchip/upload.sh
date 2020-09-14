#!/bin/sh -x

FORCE=${1}

git push git ${FORCE}master:master&
git push rk ${FORCE}master:yocto-next&
git push gitrk ${FORCE}master:yocto-next&
wait
