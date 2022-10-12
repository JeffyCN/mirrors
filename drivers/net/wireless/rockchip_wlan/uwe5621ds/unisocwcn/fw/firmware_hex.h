/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _FIRMWARE_HEX_H
#define _FIRMWARE_HEX_H

#if defined(CONFIG_WCN_SDIO)
#define firmware_hexcode "wcnmodem.bin.hex"
#elif defined(CONFIG_WCN_USB)
#define firmware_hexcode "wcnmodem_usb.bin.hex"
#endif

static unsigned char firmware_hex_buf[] = {
#include firmware_hexcode
};

#define FIRMWARE_HEX_SIZE sizeof(firmware_hex_buf)

#endif /* _FIRMWARE_HEX_H */
