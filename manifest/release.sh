#!/bin/bash -e

VERSION=${1:-1.4.0}
BRANCHES="warrior zeus dunfell default gatesgarth"
DEFAULT="dunfell"

function release {
	BRANCH=${1}
	SUBFIX=-${BRANCH}

	if [ "$BRANCH" = "default" ]; then
		BRANCH=${DEFAULT}
		SUBFIX=""
	fi
	RELEASE=release-${VERSION}_$(date +%Y%m%d)${SUBFIX}
	git checkout ${BRANCH}
	git checkout -B ${RELEASE}
	git push origin ${RELEASE}:${RELEASE}&
}

for b in ${BRANCHES};do
	echo Releasing branch: ${b}
	release ${b}
done
wait
