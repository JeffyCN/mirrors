#!/bin/bash

usage() {
	echo "Usage: ./envsetup.sh <VER>"
	echo "  VER : mandatory, support '20' or '21' now"
}

echo "Set ISP HW VERSION to: ${1}"

set_isp_hw() {
	unset ISP_HW_VERSION
	export ISP_HW_VERSION=-DISP_HW_V${1}
	env | grep -q ISP_HW_VERSION && echo "Success !" || echo "Failed !"
}

case $1 in
	20)
		set_isp_hw $1
		;;
	21)
		set_isp_hw $1
		;;
	30)
		set_isp_hw $1
		;;
	*)
		echo "Wrong ISP HW VERSION !"
		usage
		;;
esac
