#!/bin/sh -x

FORCE=${1}
BRANCHES="master warrior zeus dunfell"

for b in ${BRANCHES};do
	git push origin ${FORCE}${b}:${b}&
	git push origin ${FORCE}${b}-next:${b}-next&
done
wait
