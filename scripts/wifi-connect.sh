#!/bin/sh
SSID=$1
PASSWORD=$2
CONF=/tmp/wpa_supplicant.conf

cp /etc/wpa_supplicant.conf $CONF
echo "connect to WiFi ssid: $SSID, Passwd: $PASSWORD"
sed -i "s/SSID/$SSID/g" $CONF
sed -i "s/PASSWORD/$PASSWORD/g" $CONF
killall wpa_supplicant
sleep 1
wpa_supplicant -B -i wlan0 -c $CONF
