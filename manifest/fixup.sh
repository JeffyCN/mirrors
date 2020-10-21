#!/bin/bash

VERSIONS="
release-1.0.0_20191016
release-1.1.0_20191030
release-1.2.0_20191227
release-1.3.0_20200915"

BRANCHES="default thud warrior zeus dunfell"

function fixup {
	BRANCH=${1}
	VERSION=${2}
	SUBFIX=_${BRANCH}

	[ "${BRANCH}" = "default" ] && SUBFIX=""
	RELEASE=${VERSION}${SUBFIX}

	git checkout origin/${RELEASE} -B ${RELEASE} &>/dev/null|| return

	echo Fixing branch: ${RELEASE}

	cd ../../meta-rockchip
	COMMIT=$(git log -1 ${VERSION} |head -1|cut -f 2 -d' ')
	cd -

	XML=yocto.xml
	[ -f ${XML} ] || XML=default.xml
	sed -i "s#\(^.*meta-rockchip.*revision=\"\).*\(\".*\)#\1${COMMIT}\2#" ${XML}
	git add ${XML}
	git commit -s -m "meta-rockchip: Use mirrors for Rockchip BSP repositories"
	git push origin ${RELEASE}:${RELEASE}&
}

for v in ${VERSIONS};do
	for b in ${BRANCHES};do
		fixup ${b} ${v}
	done
done
wait
