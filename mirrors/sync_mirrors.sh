#!/bin/bash -x

function sync_mirror() {
	cd mirrors/$1
	git fetch origin &>/dev/null
	git fetch rk &>/dev/null
	git reset --hard m/master || git reset --hard origin/master
	cd -
	cd misc
	mkdir -p $1
	cd $1
	git init
	git remote add origin git@github.com:JeffyCN/mirrors.git
	git fetch origin refs/heads/$1
	git fetch ../../mirrors/$1
	git checkout FETCH_HEAD -B $1
	git push origin $1:$1
	cd ../../
}

for g in $(ls mirrors/);do
	sync_mirror $g&
done
wait

cd misc/mpp-dev
if ! git push origin mpp-dev:mpp-dev; then
	git push origin +mpp-dev:mpp-dev
	git tag mpp-dev-$(date +%Y_%m_%d)
	git push origin mpp-dev-$(date +%Y_%m_%d)
	git tag
fi

cd ../kernel-4.19
if ! git push origin +kernel-4.19:kernel-4.19; then
	git push origin +kernel-4.19:kernel-4.19
	git tag kernel-4.19-$(date +%Y_%m_%d)
	git push origin kernel-4.19-$(date +%Y_%m_%d)
	git tag
fi
