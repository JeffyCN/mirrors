#!/bin/bash -x

function sync_mirror() {
	cd mirrors2/$1
	git fetch origin &>/dev/null
	git fetch rk &>/dev/null
	git reset --hard m/master
	cd -
	cd misc2
	git fetch ../mirrors2/$1
	git push origin +FETCH_HEAD:$1
	cd -
}

for g in $(ls mirrors2/);do
	sync_mirror $g
done
wait
