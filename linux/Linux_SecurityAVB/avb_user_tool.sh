#!/bin/bash

set -e

KEYS=avb_keys
PRODUCT_ID=0123456789ABCDE
SCRIPTS=scripts
OUT=out

usage()
{
	echo "$0 [ -n/f/s/d/l/u/h or --su_pswd]"
	echo "	n	< Product id > #16 bytes"
	echo "		Generate new AVB keys"
	echo "	f	< /path/to/secureboot/private/key >"
	echo "		Config efuse device"
	echo "		Must generated keys [-n] firstly"
	echo "	s	Sign file"
	echo "		[ -b < /path/to/boot.img > ]: Sign boot.img"
	echo "		[ -r < /path/to/recovery.img > ]: Sign recovery.img"
	echo "	d	Download permanent_attributes.bin to OTP or RPMB"
	echo "	l	Lock device"
	echo "	u	Unlock device"
	echo "	h	Show this context"
	echo "	--su_pswd	Set super user password for fastboot"
}

Generate_keys()
{
	# generate config file
	touch $KEYS/temp.bin
	echo -n $PRODUCT_ID > $KEYS/product_id.bin
	# generate test keys
	openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:4096 -outform PEM -out $KEYS/testkey_prk.pem
	openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:4096 -outform PEM -out $KEYS/testkey_psk.pem
	openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:4096 -outform PEM -out $KEYS/testkey_pik.pem
	openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:4096 -outform PEM -out $KEYS/testkey_puk.pem

	# generate certificate.bin and metadata
	python $SCRIPTS/avbtool make_atx_certificate --output=avb_keys/pik_certificate.bin --subject=avb_keys/temp.bin --subject_key=avb_keys/testkey_pik.pem --subject_is_intermediate_authority --subject_key_version 42 --authority_key=avb_keys/testkey_prk.pem
	python $SCRIPTS/avbtool make_atx_certificate --output=avb_keys/psk_certificate.bin --subject=avb_keys/product_id.bin --subject_key=avb_keys/testkey_psk.pem --subject_key_version 42 --authority_key=avb_keys/testkey_pik.pem
	python $SCRIPTS/avbtool make_atx_certificate --output=avb_keys/puk_certificate.bin --subject=avb_keys/product_id.bin --subject_key=avb_keys/testkey_puk.pem --usage=com.google.android.things.vboot.unlock --subject_key_version 42 --authority_key=avb_keys/testkey_pik.pem
	python $SCRIPTS/avbtool make_atx_metadata --output=avb_keys/metadata.bin --intermediate_key_certificate=avb_keys/pik_certificate.bin --product_key_certificate=avb_keys/psk_certificate.bin

	# Generate permanent_attributes.bin
	python $SCRIPTS/avbtool make_atx_permanent_attributes --output=avb_keys/permanent_attributes.bin --product_id=avb_keys/product_id.bin --root_authority_key=avb_keys/testkey_prk.pem
	echo "Generate AVB Keys Done!!!"
}

signed_image()
{
	IMAGE=$1
	echo "Sign ${IMAGE}"
	SIZE=`ls $OUT/${IMAGE}.img -l | awk '{printf $5}'`
	echo "image size is ${SIZE}"
	# At least 68K greater than origin file
	# Source code (scripts/avbtool)
	# reserve some memory for (footer + vbmeta struct)
	# - MAX_VBMETA_SIZE = 64 * 1024
	# - MAX_FOOTER_SIZE = 4096
	SIZE=$[(SIZE / 4096 + 18) * 4096]
	echo "set size to ${SIZE}"
	python $SCRIPTS/avbtool add_hash_footer --image $OUT/${IMAGE}.img --partition_size ${SIZE} --partition_name ${IMAGE} --key avb_keys/testkey_psk.pem --algorithm SHA512_RSA4096
	echo "Sign $IMAGE Done"
}

Sign_file()
{
	while [ $# -gt 1 ]
	do
		FILE=$2
		case $1 in
			-b)
				cp $2 $OUT/boot.img
				signed_image boot
				VBMETA_CMD="${VBMETA_CMD} --include_descriptors_from_image $OUT/boot.img"
				;;
			-r)
				cp $2 $OUT/recovery.img
				signed_image recovery
				VBMETA_CMD="${VBMETA_CMD} --include_descriptors_from_image $OUT/recovery.img"
				;;
			*)
				echo "unknown file type"
				exit -1
				;;
		esac
		shift 2
	done

	echo "Generate vbmeta.img"
	python $SCRIPTS/avbtool make_vbmeta_image --public_key_metadata $KEYS/metadata.bin ${VBMETA_CMD} --algorithm SHA256_RSA4096 --rollback_index 0 --key $KEYS/testkey_psk.pem  --output $OUT/vbmeta.img
	echo "Genrate vbmeta.img Done"
}

Expect_cmd_fastboot()
{
		test -z ${SU_PSWD} && exit -1

/usr/bin/expect << EOF
		set timeout 2
		spawn sudo ./${SCRIPTS}/fastboot $1
		expect {
			"* password for *" {send "${SU_PSWD}\r"; exp_continue;}
			"OKAY *" {send "fastboot succeed\r"}
			"rebooting...*" {send "fastboot succeed\r"}
			default {send_error "expect_timeout 2\n"; exit 1}
		}
		expect eof
EOF
}

Make_unlock()
{
	python $SCRIPTS/avb-challenge-verify.py raw_unlock_challenge.bin $KEYS/product_id.bin # Generate unlock_challenge.bin
	python $SCRIPTS/avbtool make_atx_unlock_credential --output=unlock_credential.bin --intermediate_key_certificate=$KEYS/pik_certificate.bin --unlock_key_certificate=$KEYS/puk_certificate.bin --challenge=unlock_challenge.bin --unlock_key=$KEYS/testkey_puk.pem
}

load_su_pswd()
{
	if [ ! -e $SCRIPTS/.su_pswd ]; then
		echo "Please set super user password with --su_pswd first"
		exit
	fi

	SU_PSWD=$(cat $SCRIPTS/.su_pswd)
}

case $1 in
	-n)
		if [ ${#2} != 16 ]; then
			echo "please input 16 bytes product_id behind -n !"
			exit
		fi
		PRODUCT_ID=$2
		test -d $KEYS && rm $KEYS -rf
		mkdir $KEYS
		Generate_keys
		;;
	-f)
		if [ $# -lt 2 ]; then
			usage
			exit -1
		fi

		openssl dgst -sha256 -out $KEYS/permanent_attributes_cer.bin -sign $2 $KEYS/permanent_attributes.bin

		test -e .setting || touch .setting
		sed -i "/type=/d" .setting
		echo "type=efuse" >> .setting
		;;
	-s)
		if [ $# -lt 3 ]; then
			usage
			exit
		fi

		shift 1
		test -d $OUT || mkdir $OUT
		Sign_file $@
		;;
	--su_pswd)
		if [ $# -lt 2 ]; then
			usage
		fi
		echo -n "$2" > $SCRIPTS/.su_pswd
		;;
	-d)
		load_su_pswd
		test -e .setting && source .setting || echo "no .setting"
		Expect_cmd_fastboot "stage ${KEYS}/permanent_attributes.bin"
		Expect_cmd_fastboot "oem fuse at-perm-attr"
		if [ "$type" = "efuse" ]; then
			Expect_cmd_fastboot "stage ${KEYS}/permanent_attributes_cer.bin"
			Expect_cmd_fastboot "oem fuse at-rsa-perm-attr"
		fi
		;;
	-l)
		load_su_pswd
		Expect_cmd_fastboot "oem at-lock-vboot"
		Expect_cmd_fastboot "reboot"
		;;
	-u)
		load_su_pswd
		Expect_cmd_fastboot "oem at-get-vboot-unlock-challenge"
		Expect_cmd_fastboot "get_staged raw_unlock_challenge.bin"
		Make_unlock
		Expect_cmd_fastboot "stage unlock_credential.bin"
		Expect_cmd_fastboot "oem at-unlock-vboot"
		rm raw_unlock_challenge.bin -f
		rm unlock_challenge.bin -f
		rm unlock_credential.bin -f
		Expect_cmd_fastboot "reboot"
		;;
	*)
		usage
		;;
esac
