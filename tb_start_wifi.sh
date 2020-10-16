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
getdhcp_cnt=0
connect_cnt=0
wpa_cnt=0

trap "echo 'ifconfig wlan0 down'; ifconfig wlan0 down; exit" SIGTERM

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
			echo 255 > /sys/class/leds/blue/brightness

			if [ "$FORCE_CONFIG_WIFI" == "true" ];then
				echo "reconnect, restart mediaserver..."
				killall -9 mediaserver
				sleep 1
				mediaserver -a -d -c /usr/share/mediaserver/tb_rtsp-link.conf > /dev/kmsg 2>&1 &
			fi

			break
		fi

		sleep 0.01

		let getdhcp_cnt++
		if [ $getdhcp_cnt -gt 3000 ]; then
			echo "$NEW_SSID getdhcp failed!!!"
			exit
		fi
	done
}

function check_wlan0() {
	while true
	do
		lsmod | grep bcmdhd
		if [ $? -ne 0 ]; then
			echo "wait bcmdhd.ko ..."
			sleep .2
		else
			echo "ismod bcmdhd.ko okay"
			break
		fi

		let insmod_cnt++
		if [ $insmod_cnt -gt 10 ]; then
			echo "insmod failed !!!"
			exit
		fi
	done

	while true
	do
		ifconfig -a wlan0 | grep wlan0
		if [ $? -ne 0 ]; then
			echo "wait wlan0 ..."
			sleep .2
		else
			echo "wlan0 exist"
			break
		fi

		let check_cnt++
		if [ $check_cnt -gt 10 ]; then
			echo "check wlan0 failed!!!"
			exit
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
			sleep 0.2
		else
			echo "wlan0 up succeed"
			break
		fi

		let up_cnt++
		if [ $up_cnt -gt 10 ]; then
			echo "wlan0 up failed!!!"
			exit
		fi
	done
}

function connect_wifi() {

SSID=`dhd_priv isam_status | awk -F, '{print $5}' | sed -n '4p' | cut -d '"' -f 2`

if [ "$SSID" ==  "" ] || [ "$FORCE_CONFIG_WIFI" == "true" ];then

	if [ "$SSID" ==  "" ] && [ "$FORCE_CONFIG_WIFI" != "true" ];then
		echo "SSID is empty, and FORCE_CONFIG_WIFI not true"
		exit
	fi

	if [ "$SSID" == "$WIFISSID" ];then
		echo "$WIFISSID already connected"
		return
	fi

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

	ifconfig wlan0 down
	sleep 0.5
	ifconfig wlan0 up
	sleep 0.5

	while true
	do
		wpa_supplicant -B -i wlan0 -c /tmp/wpa_supplicant.conf
		WPAPID=`ps | grep wpa_supplicant | grep -v grep | awk '{print $1}'`
		if [ "$WPAPID" == "" ];then
			sleep 0.5
		else
			echo "wpa_supplicant running"
			break
		fi

		let wpa_cnt++
		if [ $wpa_cnt -gt 10 ]; then
			echo "wpa_supplicant up failed!!!"
			exit
		fi
	done

	while true
	do
		NEW_SSID=`dhd_priv isam_status | awk -F, '{print $5}' | sed -n '4p' | cut -d '"' -f 2`
		if [ "$NEW_SSID" ==  "" ];then
			echo "$WIFISSID connecting..."
			sleep 1
		elif [ "$NEW_SSID" == "$SSID" ];then
			echo "waiting $SSID disconnect ..."
			sleep 1
		else [ "$NEW_SSID" == "WIFISSID" ]
			echo "$NEW_SSID connected to getdhcp"
			break
		fi

		let connect_cnt++
		if [ $connect_cnt -gt 60 ]; then
			echo "connect $NEW_SSID failed!!!"
			exit
		fi
	done
fi
}

check_wlan0
wlan_up
tcpka_del
connect_wifi
getdhcp
