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

#ifndef __MTTY_H
#define __MTTY_H
#include <wcn_bus.h>

enum mtty_state
{
    MTTY_STATE_CLOSE,
    MTTY_STATE_OPEN
};

struct mtty_init_data
{
    char *name;
};

struct mtty_device
{
    struct mtty_init_data *pdata;
    struct tty_port *port;
    struct tty_struct *tty;
    struct tty_driver *driver;

    /* mtty state */
    atomic_t state;
};

#define MTTY_DEV_MAX_NR     1

extern int marlin_get_wcn_module_vendor(void);
extern void mdbg_assert_interface(char *str);
#endif
