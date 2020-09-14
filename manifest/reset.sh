#!/bin/bash -e

BRANCHES="master warrior zeus dunfell"

function reset {
	BRANCH=${1}

	git checkout ${BRANCH}
	git reset --hard origin/${BRANCH}
	git checkout ${BRANCH}-next
	git reset --hard origin/${BRANCH}-next
}

for b in ${BRANCHES};do
	echo Resetting branch: ${b}
	reset ${b}
done
