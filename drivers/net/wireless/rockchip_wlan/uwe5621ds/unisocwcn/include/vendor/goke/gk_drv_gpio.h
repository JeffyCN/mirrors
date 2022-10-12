/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Hunan Goke,Chengdu Goke,Shandong Goke. 2021. All rights reserved.
 */

#ifndef __DRV_GPIO_EXT_H__
#define __DRV_GPIO_EXT_H__

#include "gk_type.h"

#define GK_ERR_GPIO_NOT_INIT        (int)(0x80470003)
#define GK_ERR_GPIO_INVALID_PARA    (int)(0x80470004)
#define GK_ERR_GPIO_NULL_PTR        (int)(0x80470005)
#define GK_ERR_GPIO_INVALID_OPT     (int)(0x80470006)

/**GPIO output type*/
/** CNcomment:GPIO */
enum gpio_outputtype {
    GK_API_GPIO_OUTPUTTYPE_CMOS,
    GK_API_GPIO_OUTPUTTYPE_OD,
    GK_API_GPIO_OUTPUTTYPE_BUTT,
};


int drv_gpio_init(void);

/*param[in] u32GpioNo :gpio No.,for example the No. of GPIO5_1 is 41(5*8+1);
param[in] u32DirBit: 0 - output, 1 - input
 */
int drv_gpio_set_direction_bit(unsigned int gpio_num, unsigned int dir_bit);
int drv_gpio_get_direction_bit(unsigned int gpio_num, unsigned int* dir_bit );
int drv_gpio_read_bit(unsigned int gpio_num, unsigned int* bit_value);
int drv_gpio_write_bit(unsigned int gpio_num, unsigned int bit_value);
int drv_gpio_set_output_type(unsigned int gpio_num, enum gpio_outputtype  output_type);
int drv_gpio_get_output_type(unsigned int gpio_num, enum gpio_outputtype  *output_type);

#endif