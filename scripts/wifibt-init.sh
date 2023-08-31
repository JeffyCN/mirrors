#!/bin/sh

wifi_ready()
{
	ifconfig -a | grep -wqE "^(wlan0|p2p0)"
}

start_bt_brcm()
{
	killall -q brcm_patchram_plus1

	echo 0 > /sys/class/rfkill/rfkill0/state
	echo 0 > /proc/bluetooth/sleep/btwrite
	sleep .5
	echo 1 > /sys/class/rfkill/rfkill0/state
	echo 1 > /proc/bluetooth/sleep/btwrite
	sleep .5

	BT_TTY=$(wifibt-util.sh tty)
	brcm_patchram_plus1 --bd_addr_rand --enable_hci --no2bytes \
		--use_baudrate_for_download --tosleep 200000 \
		--baudrate 1500000 \
		--patchram ${WIFIBT_FIRMWARE_DIR:-/lib/firmware}/ $BT_TTY
}

start_bt_rtk_uart()
{
	killall -q rtk_hciattach

	echo 0 > /sys/class/rfkill/rfkill0/state
	echo 0 > /proc/bluetooth/sleep/btwrite
	sleep .5
	echo 1 > /sys/class/rfkill/rfkill0/state
	echo 1 > /proc/bluetooth/sleep/btwrite
	sleep .5

	insmod hci_uart.ko
	sleep .5

	BT_TTY=$(wifibt-util.sh tty)
	rtk_hciattach -n -s 115200 $BT_TTY rtk_h5
}

start_bt_rtk_usb()
{
	insmod rtk_btusb.ko
}

start_wifibt()
{
	cd "${WIFIBT_MODULE_DIR:-/lib/modules}"

	# Start Wifi
	if ! wifi_ready; then
		if [ "$WIFIBT_VENDOR" = Broadcom -a -f dhd_static_buf.ko ];then
			insmod dhd_static_buf.ko
		fi

		insmod "$WIFIBT_MODULE"

		for i in `seq 100`;do
			! wifi_ready || break
			sleep .2
		done
	fi

	# Start BT
	case "$WIFIBT_VENDOR" in
		Broadcom) start_bt_brcm ;;
		Realtek)
			case "$WIFIBT_CHIP" in
				*E*) start_bt_rtk_usb ;;
				*) start_bt_rtk_uart ;;
			esac
			;;
	esac
}

WIFIBT_CHIP=$(wifibt-util.sh chip)
[ "$WIFIBT_CHIP" ] || exit 0

WIFIBT_VENDOR="$(wifibt-util.sh vendor)"
WIFIBT_MODULE="$(wifibt-util.sh module)"

case "${1:-start}" in
        start|restart)
		echo "Starting Wifi/BT..."
		start_wifibt&
		;;
	stop)
		echo "Stopping Wifi/BT..."
		killall -q brcm_patchram_plus1 rtk_hciattach || true
                ;;
        *)
                echo "Usage: [start|stop|restart]" >&2
                exit 3
                ;;
esac

:
