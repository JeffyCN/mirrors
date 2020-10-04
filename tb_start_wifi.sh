#!/bin/sh

SSID=
WIFISSID=$1
WIFIPWD=$2
IPADDR=
WPAPID=""
NETMASK=
GW=
DNS=
up_cnt=0
insmod_cnt=0
check_cnt=0
FORCE_CONFIG_WIFI=$3
NEW_SSID=

if [ -z $WIFISSID ]; then
	echo -e "\033[33m WIFISSID is invalid, assume to Rockchip-guest \033[0m"
	WIFISSID=Rockchip-guest
fi

if [ -z $WIFIPWD ]; then
	echo -e "\033[33m WIFIPWD is invalid, assume to RKguest2.4 \033[0m"
	WIFIPWD=RKguest2.4
fi

function getdhcp() {
	while true
	do
		IPADDR=`dhd_priv wl dhcpc_dump | awk '{print $5}' | sed -n '3p'`
		if [ "$IPADDR" != "0.0.0.0" ];then
			NETMASK=`dhd_priv wl dhcpc_dump | awk '{print $7}' | sed -n '3p'`
			GW=`dhd_priv wl dhcpc_dump | awk '{print $9}' | sed -n '3p'`
			DNS=`dhd_priv wl dhcpc_dump | awk '{print $11}' | sed -n '3p'`

			ifconfig wlan0 $IPADDR netmask $NETMASK
			route add default gw $GW
			echo "nameserver $DNS" > /etc/resolv.conf

			echo $IPADDR $NETMASK $GW $DNS
			break
		fi
	done
}

function check_wlan0() {
	while true
	do
		ifconfig -a wlan0 | grep wlan0
		if [ $? -ne 0 ]; then
			echo "wait wlan0 ..."
			sleep .1
		else
			echo "wlan0 exist"
			break
		fi
		let check_cnt++
		if [ $check_cnt -gt 2 ]; then
			break
		fi
	done
}

function tcpka_del() {
	while true
	do
		IPID=`dhd_priv wl tcpka_conn_sess_info 1 | grep ipid`
		if [ "$IPID" != "" ]; then
			sleep 0.05
			dhd_priv wl tcpka_conn_enable 1 0 0 0 0
			sleep 0.05
			dhd_priv wl tcpka_conn_del 1
			sleep 0.05
		else
			break
		fi
	done
}

function wlan_up() {
	while true
	do
		ifconfig wlan0 up
		if [ $? -ne 0 ]; then
			echo "wlan0 up failed"
			sleep 0.1
		else
			echo "wlan0 up succeed"
			break
		fi

		let up_cnt++
		if [ $up_cnt -gt 2 ]; then
			break
		fi
	done
}

check_wlan0
wlan_up
tcpka_del

SSID=`dhd_priv isam_status | awk -F, '{print $5}' | sed -n '4p' | cut -d '"' -f 2`

if [ "$SSID" ==  "" ] || [ "$FORCE_CONFIG_WIFI" == "true" ];then
	echo "WIFI INFO: $SSID, CONFIG: $FORCE_CONFIG_WIFI"
	cp /etc/wpa_supplicant.conf /tmp/
	echo "connect to WiFi ssid: $WIFISSID, Passwd: $WIFIPWD"
	sed -i "s/SSID/$WIFISSID/g" /tmp/wpa_supplicant.conf
	sed -i "s/PASSWORD/$WIFIPWD/g" /tmp/wpa_supplicant.conf

	WPAPID=`ps | grep wpa_supplicant | grep -v grep | awk '{print $1}'`
	if [ "$WPAPID" != "" ];then
		killall wpa_supplicant
		sleep 0.5
	fi

	while true
	do
		wpa_supplicant -B -i wlan0 -c /tmp/wpa_supplicant.conf
		WPAPID=`ps | grep wpa_supplicant | grep -v grep | awk '{print $1}'`
		if [ "$WPAPID" == "" ];then
			sleep 0.5
		else
			break
		fi
	done

	if [ "$SSID" !=  "" ] && [ "$FORCE_CONFIG_WIFI" == "true" ];then
		echo "waiting $SSID disconnect ..."
		sleep 2
	fi

	while true
	do
		NEW_SSID=`dhd_priv isam_status | awk -F, '{print $5}' | sed -n '4p' | cut -d '"' -f 2`
		if [ "$NEW_SSID" !=  "" ];then
			echo "$NEW_SSID connecting..."
			break
		fi
	done

	getdhcp
else
	getdhcp
fi
