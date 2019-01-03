#!/bin/sh
WIFISSID=$1
WIFIPWD=$2
CONF=/userdata/cfg/wpa_supplicant.conf
if [ ! -d /userdata/cfg ];then
	mkdir /userdata/cfg
fi

cp /etc/wpa_supplicant.conf /userdata/cfg/
echo "connect to WiFi ssid: $WIFISSID, Passwd: $WIFIPWD"
sed -i "s/SSID/$WIFISSID/g" $CONF
sed -i "s/PWD/$WIFIPWD/g" $CONF
wpa_supplicant -B -i wlan0 -c $CONF
