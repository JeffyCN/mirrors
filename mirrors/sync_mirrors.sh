#!/bin/bash -x

function sync_mirror() {
	cd mirrors/$1
	git fetch origin &>/dev/null
	git fetch rk &>/dev/null
	git reset --hard m/master
	cd -
	cd misc
	git fetch ../mirrors/$1
	git checkout FETCH_HEAD -B $1
	git push origin $1:$1&
	cd -
}

for g in $(ls mirrors/);do
	sync_mirror $g
done
wait
