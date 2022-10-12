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

#ifndef __MSDIO_H
#define __MSDIO_H
#include <wcn_bus.h>
#include <marlin_platform.h>

#include "interface.h"

#define BT_TX_CHANNEL    3
#define BT_RX_CHANNEL     17
#define BT_TX_INOUT    1
#define BT_RX_INOUT     0
#define BT_TX_POOL_SIZE   64  // the max buffer is 64
#define BT_RX_POOL_SIZE   1
#define BT_SDIO_HEAD_LEN   4

struct bt_data_interface_t *get_marlin_sdio_interface(void);
int set_need_indicate_cp2_woble(int set);

#endif
