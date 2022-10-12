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

#ifndef __MDEBUG_H
#define __MDEBUG_H
#include <linux/types.h>


enum debug_log_level
{
    LOG_LEVEL_NONE,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_VER = 5,
};

unsigned int check_log_level(void);
unsigned int set_log_level(int level);
void hex_dump(unsigned char *bin, size_t binsz);
void hex_dump_block(unsigned char *bin, size_t binsz);


#define BT_DEBUG(fmt, ...)                        \
    do {                                        \
        if (check_log_level() >= LOG_LEVEL_DEBUG) \
            pr_err(fmt, ##__VA_ARGS__);         \
    } while (0)


#define BT_VER(fmt, ...)                        \
    do {                                        \
        if (check_log_level() >= LOG_LEVEL_VER) \
            pr_err(fmt, ##__VA_ARGS__);         \
    } while (0)

#define BT_VERDUMP(bin, binsz)                      \
    do {                                        \
        if (check_log_level() >= LOG_LEVEL_VER) \
            hex_dump_block(bin, binsz);         \
    } while (0)


#endif
