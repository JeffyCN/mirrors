#!/bin/sh -e

BT_PINCTRL_DIR="/sys/firmware/devicetree/base/pinctrl/wireless-bluetooth"
CHIPS_FILE="/var/run/wifibt-chips.txt"
CHIP_FILE="/var/run/wifibt-info.txt"

bt_tty()
{
	[ -d "$BT_PINCTRL_DIR" ] || return
	UART_DIR=$(find "$BT_PINCTRL_DIR/" -name "uart[0-9]*" | head -n 1)
	echo "$UART_DIR" | sed 's~.*uart\([0-9]\+\).*~/dev/ttyS\1~'
}

wifibt_info()
{
	if [ -r "$CHIP_FILE" ]; then
		cat "$CHIP_FILE"
		return
	fi

	VIDS="$(grep -v "^#" "$CHIPS_FILE" | cut -f3 | cut -d":" -f1 | \
		sort | uniq | xargs | tr ' ' '|')"
	VENDORS="$(find /sys/devices/platform/ -type f -name vendor | \
		xargs grep -El "$VIDS" || true)"
	for VENDOR in $VENDORS; do
		UEVENT="$(echo $VENDOR | sed 's~\(/id\)\?/vendor~/uevent~')"
		VID="$(cat "$VENDOR" | sed 's/^0x//')"

		IDS="$(grep -v "^#" "$CHIPS_FILE" | grep "\b$VID:" | \
			cut -f3 | sort | uniq | tr ':' '.')"
		for ID in $IDS; do
			grep -iqE "$ID" "$UEVENT" || continue
			grep -iw -m 1 "$ID" "$CHIPS_FILE" | tee "$CHIP_FILE"
			return
		done
	done
}

wifibt_vendor()
{
	wifibt_info | cut -f 1
}

wifibt_chip()
{
	wifibt_info | cut -f 2
}

wifibt_module()
{
	wifibt_info | cut -f 4
}

if [ ! -r "$CHIPS_FILE" ]; then
	echo "# Vendor Name VID:PID Module\n" > "$CHIPS_FILE"

	# Prefer /etc/ version
	cat "/etc/wifibt-chips.txt" >>"$CHIPS_FILE" 2>/dev/null || true

	# Append built-in chip list
	cat << EOF >> "$CHIPS_FILE"
Realtek	RTL8188EU	0bda:8179	RTL8188EU.ko
Realtek	RTL8188EU	0bda:0179	RTL8188EU.ko
Realtek	RTL8723BU	0bda:b720	RTL8723BU.ko
Realtek	RTL8723BS	024c:b723	RTL8723BS.ko
Realtek	RTL8822BS	024c:b822	RTL8822BS.ko
Realtek	RTL8723CS	024c:b703	RTL8723CS.ko
Realtek	RTL8723DS	024c:d723	RTL8723DS.ko
Realtek	RTL8188FU	0bda:f179	RTL8188FU.ko
Realtek	RTL8822BU	0bda:b82c	RTL8822BU.ko
Realtek	RTL8189ES	024c:8179	RTL8189ES.ko
Realtek	RTL8189FS	024c:f179	RTL8189FS.ko
Realtek	RTL8192DU	0bda:8194	RTL8192DU.ko
Realtek	RTL8812AU	0bda:8812	RTL8812AU.ko
Realtek	RTL8822BE	10ec:b822	RTL8822BE.ko
Realtek	RTL8822CE	10ec:c822	RTL8822CE.ko
Broadcom	AP6354	02d0:4354	bcmdhd.ko
Broadcom	AP6330	02d0:4330	bcmdhd.ko
Broadcom	AP6356S	02d0:4356	bcmdhd.ko
Broadcom	AP6398S	02d0:4359	bcmdhd.ko
Broadcom	AP6335	02d0:4335	bcmdhd.ko
Broadcom	AP6255	02d0:a9bf	bcmdhd.ko
Broadcom	AP6256	02d0:a9bf	bcmdhd.ko
Broadcom	AP6212A	02d0:a9a6	bcmdhd.ko
Broadcom	AP6275S	02d0:aae8	bcmdhd.ko
Broadcom	AP6275P	14e4:449d	bcmdhd_pcie.ko
EOF
fi

if [ -e "$CHIP_FILE" ]; then
	# Invalid out-dated cache file
	if [ "$CHIP_FILE" -ot "$CHIPS_FILE" ] ||
		[ "$CHIP_FILE" -ot "$(realpath "$0")" ]; then
		rm -f "$CHIP_FILE"
	fi
fi

for cmd in "$(basename "$0")" "$1"; do
	# Invalid cache file when forcing reload
	case "$1" in
		-f | --force-reload) rm -f "$CHIP_FILE" ;;
	esac

	case "$cmd" in
		tty | bt-tty) bt_tty; exit ;;
		info | wifibt-info) wifibt_info; exit ;;
		vendor | wifibt-vendor) wifibt_vendor; exit ;;
		chip | wifibt-chip) wifibt_chip; exit ;;
		module | wifibt-module) wifibt_module; exit ;;
	esac

	[ -z "$1" ] || shift
done

echo "$(basename "$0") <tty|info|vendor|chip|module> [-f|--force-reload]"
