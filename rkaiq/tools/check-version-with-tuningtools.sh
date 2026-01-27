#!/bin/sh
# checking if AIQ version matchs up with TuningTool 

# get AIQ version string
AIQVERSTR=$(grep -E "RK_AIQ_VERSION_REAL_V \"v" $1/RkAiqVersion.h)

ret=$?
if [ $ret != 0 ] ; then
	echo "error! no AIQVER"
	exit 1
fi

# parse the middule version num
RKAIQ_VER=$(echo $AIQVERSTR | cut -f 3 -d ' ')
# delete ""
RKAIQ_VER=$(echo ${RKAIQ_VER%\"*})
RKAIQ_VER=$(echo ${RKAIQ_VER#*\"})
echo "AIQ VERSION :" $RKAIQ_VER

# get middle and last version
AIQ_MIDDLE_LAST_VER=$(echo $RKAIQ_VER | cut -f 2,3 -d '.' | cut -c -3)
echo "AIQ VERSION MIDDLE_LAST NUM:" $AIQ_MIDDLE_LAST_VER

# get tuner version string
TUNER_VERSION_FILE=$1/rkisp2x_tuner/rkisp2x_tuner_version.txt
if [ ! -f  $TUNER_VERSION_FILE ]; then
	TUNER_VERSION_FILE=$1/../rkisp2x_tuner/rkisp2x_tuner_version.txt
fi

TUNINGTOOL_VER_STRING=$(grep -E "Current Version" $TUNER_VERSION_FILE)
ret=$?
if [ $ret != 0 ] ; then
	echo "error! no tuner version"
	exit 1
fi

echo "TUNER VERSION :" $TUNINGTOOL_VER_STRING
TUNER_MIDDLE_LAST_VER=$(echo $TUNINGTOOL_VER_STRING | cut -f 2,3 -d '.' | cut -c -3)
echo "TUNER VERSION MIDDLE_LAST NUM: ${TUNER_MIDDLE_LAST_VER}"

# check if version is matched
if [ "$TUNER_MIDDLE_LAST_VER" != "$AIQ_MIDDLE_LAST_VER" ]; then
	echo "!!! WARNING !!!"
	echo "---- Aiq version $RKAIQ_VER not matched with Tuning tool version $TUNINGTOOL_VER_STRING ----"
	echo "!!! WARNING END !!!"
else
	echo "******** Aiq version $RKAIQ_VER matched with Tuner $TUNINGTOOL_VER_STRING *****"
fi
