/*
 * Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __MHCI_H
#define __MHCI_H

#define HCI_CMD         0x01
#define HCI_ACL         0x02
#define HCI_SCO         0x03
#define HCI_EVT         0x04

#define PACKET_TYPE     0
#define EVT_HEADER_TYPE     0
#define EVT_HEADER_EVENT    1
#define EVT_HEADER_SIZE     2
#define EVT_VENDOR_CODE_LSB 3
#define EVT_VENDOR_CODE_MSB 4
#define EVT_LE_META_SUBEVT  3
#define EVT_ADV_LENGTH     13

#define BT_HCI_EVT_CMD_COMPLETE                 0x0e
#define BT_HCI_EVT_CMD_STATUS                   0x0f

#define ACL_HEADER_SIZE_LB  3
#define ACL_HEADER_SIZE_HB  4
#define EVT_HEADER_STATUS   4

#define HCI_CMD_MAX_LEN 258
#define BD_ADDR_LEN     6

#define UINT8_TO_STREAM(p, u8)   {*(p)++ = (uint8_t)(u8);}
#define UINT16_TO_STREAM(p, u16) {*(p)++ = (uint8_t)(u16); *(p)++ = (uint8_t)((u16) >> 8);}
#define ARRAY_TO_STREAM(p, a, len) {register int ijk; for (ijk = 0; ijk < len;        ijk++) *(p)++ = (uint8_t) a[ijk];}
#define STREAM_TO_UINT8(u8, p)   {u8 = (uint8_t)(*(p)); (p) += 1;}
#define STREAM_TO_UINT16(u16, p) {u16 = ((uint16_t)(*(p)) + (((uint16_t)(*((p) + 1))) << 8)); (p) += 2;}
#define BDADDR_TO_STREAM(p, a)   {register int ijk; for (ijk = 0; ijk < BD_ADDR_LEN;  ijk++) *(p)++ = (uint8_t) a[BD_ADDR_LEN - 1 - ijk];}
#define STREAM_TO_ARRAY(a, p, len) {register int ijk; for (ijk = 0; ijk < len; ijk++) ((uint8_t *) a)[ijk] = *p++;}

#define BT_HCI_OP_RESET             0x0c03
#define BT_HCI_OP_ENABLE            0xfca1
#define BT_HCI_OP_WOBLE             0xfd08

#define UNISOC_WOBLE_UUID           0xfd01

#define BT_HCI_OP_SET_SLEEPMODE     0xfd09
#define BT_HCI_OP_ADD_WAKEUPLIST    0xfd0a
#define BT_HCI_OP_CLEANUP_WAKEUPLIST    0xfd0b
#define BT_HCI_OP_SET_STARTSLEEP    0xfd0d

#define BT_HCI_OP_LE_SET_EX_SCAN_PARAMETERS     0x2041
#define BT_HCI_OP_LE_SET_EX_SCAN_ENABLE         0x2042


#define WOBLE_DEVICES_SIZE 10

struct HC_BT_HDR
{
    unsigned short event;
    unsigned short len;
    unsigned short offset;
    unsigned short layer_specific;
    unsigned char data[];
};

struct hci_cmd_t
{
    unsigned short opcode;
    struct semaphore wait;
    struct HC_BT_HDR response;
};


typedef enum {
    WOBLE_MOD_DISABLE = 0,
    WOBLE_MOD_ENABLE,
    WOBLE_MOD_UNDEFINE = 0xff,
} WOBLE_MOD;

typedef enum {
    WOBLE_SLEEP_MOD_COULD_KNOW = 0,
    WOBLE_SLEEP_MOD_COULD_NOT_KNOW
} WOBLE_SLEEP_MOD;

typedef enum {
    WOBLE_SLEEP_MOD_NOT_NEED_NOTITY = 0,
    WOBLE_SLEEP_MOD_NEED_NOTITY,
} WOBLE_NOFITY_MOD;

typedef enum {
    WOBLE_IS_NOT_SHUTDOWN = 0,
    WOBLE_IS_SHUTDOWN,
} WOBLE_SHUTDOWN_MOD;

typedef enum {
    WOBLE_IS_NOT_RESUME = 0,
    WOBLE_IS_RESUME,
} WOBLE_RESUME_MOD;

typedef struct {
    unsigned char woble_mod;
    unsigned char sleep_mod;
    unsigned short timeout;
    unsigned char notify;
} woble_config_t;

typedef enum {
    ADV_PUBLIC_ADDRESS = 0,
    ADV_RANDOM_ADDRESS,
    ADV_ADDRESS_FROM_WHITTLIST = 0XFF,
} ADV_ADDRESS_TYPE;

#define ANY_ADV                     ( 1 )
#define ANY_DATA_ON_LINK            ( 1 << 1 )
#define ADV_IN_FILTER               ( 1 << 2 )
#define DATA_IN_FILTER_ON_LINK      ( 1 << 3 )
#define SUB_ADV_DATA_FILTER         ( 1 << 4 )

typedef enum {
    KEEP_THE_LINK = 0,
    DISCONNECT_WITH_DEVICE,
} ADV_STATE;

typedef struct mtty_bt_wake_t {
    unsigned char addr[6];
    char *addr_str;
    unsigned char dev_tp;
    char *dev_tp_str;
    unsigned char addr_tp;
    char *addr_tp_str;
} mtty_bt_wake_t;

int hci_init(void);
int hci_destory(void);
void hci_cleanup(void);
void hci_woble_enable(void);
int rx_data_recv(const unsigned char *buf, int count, int (*upper_cb)(const unsigned char *buf, int count));
int update_woble_devices(unsigned char type, unsigned short handler, unsigned char *bd_addr);
int del_woble_devices(unsigned char type, unsigned char *bd_addr);
int clear_woble_devices(void);
int set_random_address(unsigned char *bd_addr);
void dump_woble_devices(void);
int hci_cmd_send_sync(unsigned short opcode, struct HC_BT_HDR *py, struct HC_BT_HDR *rsp);

void hci_set_ap_sleep_mode(int is_shutdown, int is_resume);
void hci_cleanup_wakeup_list(void);
void hci_add_device_to_wakeup_list(void);
void hci_set_ap_start_sleep(void);
void hci_set_scan_parameters(void);
void hci_set_scan_enable(int enable);

typedef enum
{
    WOBLE_KERNEL_OP_CLEAR,
    WOBLE_KERNEL_OP_SET_OWN,
    WOBLE_KERNEL_OP_ADD_RMT,
    WOBLE_KERNEL_OP_DEL_RMT,
    WOBLE_KERNEL_OP_ENABLE_BLE_INDICATE,
    WOBLE_KERNEL_OP_DISABLE_BLE_INDICATE,
} WOBLE_KERNEL_OP;


#endif
