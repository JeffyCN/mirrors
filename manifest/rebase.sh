#!/bin/bash -ex

BRANCHES="master warrior zeus dunfell gatesgarth"

HEAD=$(cd ../../meta-rockchip && git log --pretty=%H -1)

function rebase {
	BRANCH=${1}

	git checkout ${BRANCH}
	git reset --hard origin/${BRANCH}
	sed -i "/meta-rockchip/s/\(revision=\"\)[^\"]*/\1${HEAD}/" yocto.xml
	git commit --amend -a --no-edit
}

for b in ${BRANCHES};do
	echo Rebasing branch: ${b} on ${HEAD}
	rebase ${b}
done
