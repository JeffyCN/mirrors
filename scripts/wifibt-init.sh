#!/bin/sh

wifi_ready()
{
	grep -wqE "wlan0|p2p0" /proc/net/dev
}

start_bt_brcm()
{
	killall -q -9 brcm_patchram_plus1

	echo 0 > /sys/class/rfkill/rfkill0/state
	echo 0 > /proc/bluetooth/sleep/btwrite
	sleep .5
	echo 1 > /sys/class/rfkill/rfkill0/state
	echo 1 > /proc/bluetooth/sleep/btwrite
	sleep .5

	brcm_patchram_plus1 --bd_addr_rand --enable_hci --no2bytes \
		--use_baudrate_for_download --tosleep 200000 \
		--baudrate 1500000 \
		--patchram ${WIFIBT_FIRMWARE_DIR:-/lib/firmware}/ $WIFIBT_TTY
}

start_bt_rtk_uart()
{
	killall -q -9 rtk_hciattach

	echo 0 > /sys/class/rfkill/rfkill0/state
	echo 0 > /proc/bluetooth/sleep/btwrite
	sleep .5
	echo 1 > /sys/class/rfkill/rfkill0/state
	echo 1 > /proc/bluetooth/sleep/btwrite
	sleep .5

	insmod hci_uart.ko
	sleep .5

	rtk_hciattach -n -s 115200 $WIFIBT_TTY rtk_h5
}

start_bt_rtk_usb()
{
	insmod rtk_btusb.ko
}

start_wifi()
{
	! wifi_ready || return

	cd "${WIFIBT_MODULE_DIR:-/lib/modules}"

	if [ "$WIFIBT_VENDOR" = Broadcom -a -f dhd_static_buf.ko ]; then
		insmod dhd_static_buf.ko
	fi

	echo "Installing WiFi/BT module: $WIFIBT_MODULE"
	insmod "$WIFIBT_MODULE"

	for i in `seq 100`; do
		if wifi_ready; then
			if grep -wqE "wlan0" /proc/net/dev; then
				ifup wlan0&
			fi
			return
		fi
		sleep .2
	done
	echo "Failed to init Wi-Fi for $WIFIBT_CHIP!"
}


start_bt()
{
	cd "${WIFIBT_MODULE_DIR:-/lib/modules}"

	case "$WIFIBT_VENDOR" in
		Broadcom) start_bt_brcm ;;
		Realtek)
			case "$WIFIBT_BUS" in
				usb) start_bt_rtk_usb ;;
				*) start_bt_rtk_uart ;;
			esac
			;;
		*)
			echo "Unknow Wi-Fi/BT chip, fallback to Broadcom..."
			start_bt_brcm
			;;
	esac
}

WIFIBT_CHIP=$(wifibt-util.sh chip)
if [ -z "$WIFIBT_CHIP" ]; then
	echo "Failed to detect Wi-Fi/BT chip!"
	exit 0
fi

WIFIBT_VENDOR="$(wifibt-util.sh vendor)"
WIFIBT_BUS="$(wifibt-util.sh bus)"
WIFIBT_MODULE="$(wifibt-util.sh module)"
WIFIBT_TTY=$(wifibt-util.sh tty)

echo "Handling ${1:-start} for Wi-Fi/BT chip: $(wifibt-util.sh info)"

case "${1:-start}" in
	start|restart)
		echo "Starting Wifi/BT..."
		start_wifi&
		start_bt&
		;;
	start_wifi)
		start_wifi&
		;;
	start_bt)
		start_bt&
		;;
	stop)
		echo "Stopping Wi-Fi/BT..."
		killall -q -9 brcm_patchram_plus1 rtk_hciattach || true
		;;
	*)
		echo "Usage: [start|stop|start_wifi|start_bt|restart]" >&2
		exit 3
		;;
esac

:
