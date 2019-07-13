#!/bin/bash

usage()
{
	echo "$0 [ -b/r ]"
	echo "	b	path to boot.img"
	echo "	r	path to recovry.img"
	echo "vbmeta.img generate in out/vbmeta.img"
}

signed_image()
{
	IMAGE=$1
	echo "sign ${IMAGE}"
	SIZE=`ls out/${IMAGE}.img -l | awk '{printf $5}'`
	# At least 64K greater than origin file
	SIZE=$[(SIZE / 4024 + 64) * 4096]
	python avbtool add_hash_footer --image out/${IMAGE}.img --partition_size ${SIZE} --partition_name ${IMAGE} --key testkey_psk.pem --algorithm SHA512_RSA4096
	echo "Done"
}

parameter=`getopt -o b:r:h -n "$0" -- "$@"`
if [ $? != 0 ]; then
	echo "Terminating ......" >&2
	exit 1
fi

eval set -- "$parameter"

while true
do
	case "$1" in
		-b)		BOOT="$2"; shift 2;;
		-r)		RECOVERY="$2"; shift 2;;
		-h)		usage $@; shift; exit 0;;
		--)		shift; break;;
		*)		echo "Internal error!"; exit 1;;
	esac
done

if [ -d out ]; then
	rm out/* -rf
else
	mkdir out
fi

if [ ! -z ${BOOT} ] && [ -e ${BOOT} ]; then
	# Do not modify origin file, copy it here
	cp ${BOOT} out/boot.img
	signed_image boot
	VBMETA_CMD="--include_descriptors_from_image out/boot.img"
else
	echo "No boot found"
fi

if [ ! -z ${RECOVERY} ] && [ -e ${RECOVERY} ]; then
	# Do not modify origin file, copy it here
	cp ${RECOVERY} out/recovery.img
	signed_image recovery
	VBMETA_CMD="${VBMETA_CMD} --include_descriptors_from_image out/recovery.img"
else
	echo "No recovery found"
fi

#echo "VBMETA_CMD=${VBMETA_CMD}"
echo "Generate vbmeta.img"
python avbtool make_vbmeta_image --public_key_metadata metadata.bin ${VBMETA_CMD} --algorithm SHA256_RSA4096 --rollback_index 0 --key testkey_psk.pem  --output out/vbmeta.img
echo "Done"
