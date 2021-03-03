#!/bin/bash -e

BRANCHES="master warrior zeus dunfell gatesgarth"

function pick_cl {
	BRANCH=${1}

	git checkout ${BRANCH}
	git cherry-pick $2 || git reset
	git checkout ${BRANCH}-next
	git cherry-pick $2 || git reset
}

for b in ${BRANCHES};do
	echo Picking branch: ${b}
	pick_cl ${b} $@
done
