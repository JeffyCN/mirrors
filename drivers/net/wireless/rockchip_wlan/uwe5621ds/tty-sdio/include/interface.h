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

#ifndef __MINTERFACE_H
#define __MINTERFACE_H
#include <linux/kernel.h>

struct bt_data_interface_cb_t
{
    size_t size;
    int (*recv)(const unsigned char *buf, int count);
};


struct bt_data_interface_t
{
    size_t size;
    int (*init)(struct bt_data_interface_cb_t *cb);
    void (*cleanup)(void);
    int (*write)(const unsigned char *buf, int count);
};


#endif
