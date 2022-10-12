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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/vt_kern.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/vmalloc.h>
#include <linux/atomic.h>
#include <linux/of_device.h>
#include <linux/compat.h>
#include <linux/tty_flip.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

#include "include/debug.h"

static unsigned int log_level = LOG_LEVEL_NONE;

unsigned int check_log_level(void)
{
    return log_level;
}

unsigned int set_log_level(int level)
{
    log_level = level;
    return 0;
}

void hex_dump(unsigned char *bin, size_t binsz)
{
    char *str, hex_str[] = "0123456789ABCDEF";
    size_t i;

    str = (char *)vmalloc(binsz * 3);

    if (!str)
    {
        return;
    }

    for (i = 0; i < binsz; i++)
    {
        str[(i * 3) + 0] = hex_str[(bin[i] >> 4) & 0x0F];
        str[(i * 3) + 1] = hex_str[(bin[i]     ) & 0x0F];
        str[(i * 3) + 2] = ' ';
    }

    str[(binsz * 3) - 1] = 0x00;
    pr_info("%s\n", str);
    vfree(str);
}

void hex_dump_block(unsigned char *bin, size_t binsz)
{
#define HEX_DUMP_BLOCK_SIZE 20
    int loop = binsz / HEX_DUMP_BLOCK_SIZE;
    int tail = binsz % HEX_DUMP_BLOCK_SIZE;
    int i;

    if (!loop)
    {
        hex_dump(bin, binsz);
        return;
    }

    for (i = 0; i < loop; i++)
    {
        hex_dump(bin + i * HEX_DUMP_BLOCK_SIZE, HEX_DUMP_BLOCK_SIZE);
    }

    if (tail)
    {
        hex_dump(bin + i * HEX_DUMP_BLOCK_SIZE, tail);
    }
}

