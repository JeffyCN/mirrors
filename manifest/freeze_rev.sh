#!/bin/bash -e

TEMP_XML=freeze.xml
BRANCHES="master warrior zeus dunfell"

function freeze {
	BRANCH=${1}

	git checkout ${BRANCH}
	git checkout ${BRANCH}-next yocto.xml
	git commit -a --allow-empty -s -m "test"

	repo sync --no-manifest-update -j 32

	# Freeze revisions
	repo manifest -r --suppress-upstream-revision -o $TEMP_XML

	# Change yocto-conf to extend-project and remove local.conf linkfile.
	sed -i '/yocto-conf/{s/<\(.*\)>/<extend-\1\/>/;n;N;d}' $TEMP_XML

	mv $TEMP_XML yocto.xml
	git reset HEAD^
	git commit -s -a --no-edit -m \
		"Bump revisions for ${BRANCH} at $(date +%Y_%m_%d)" || :
}

for b in ${BRANCHES};do
	echo Freezing branch: ${b}
	freeze ${b}
done
