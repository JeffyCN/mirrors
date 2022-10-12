/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   mdrv_gpio.h
/// @brief  GPIO Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_GPIO_H_
#define _DRV_GPIO_H_

#include <asm/types.h>
#include "mdrv_types.h"
#include "mdrv_gpio_io.h"
#include <linux/interrupt.h>
//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Init(void);
void MDrv_GPIO_WriteRegBit(U32 u32Reg, U8 u8Enable, U8 u8BitMsk);
void MDrv_GPIO_Pad_Set(U32 u32IndexGPIO);
void MDrv_GPIO_Pad_Oen(U32 u32IndexGPIO);
void MDrv_GPIO_Pad_Odn(U32 u32IndexGPIO);
U8 MDrv_GPIO_Pad_Read(U32 u32IndexGPIO);
U8 MDrv_GPIO_Pad_InOut(U32 u32IndexGPIO);
void MDrv_GPIO_Pull_High(U32 u32IndexGPIO);
void MDrv_GPIO_Pull_Low(U32 u32IndexGPIO);
void MDrv_GPIO_Set_High(U32 u32IndexGPIO);
void MDrv_GPIO_Set_Low(U32 u32IndexGPIO);
void MDrv_GPIO_Set_Input(U32 u32IndexGPIO);

#ifdef CONFIG_EXT_INTERRUPT_SUPPORT
/*Mstar_Gpio_Irq_Attach API export to Utopia for debug*/
void Mstar_Gpio_Irq_Attach(int gpio_num, unsigned long irqflags,void* ptr);
int request_gpio_irq(int gpio_num, irq_handler_t handler, unsigned long irqflags, void *dev_id);
int free_gpio_irq(int gpio_num, void *dev_id);
#endif

#endif // _DRV_GPIO_H_

