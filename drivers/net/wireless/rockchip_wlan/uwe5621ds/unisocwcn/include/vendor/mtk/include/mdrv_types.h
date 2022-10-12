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


#ifndef __MDRV_TYPES_H__
#define __MDRV_TYPES_H__

//#include <linux/types.h>

/// data type unsigned char, data length 1 byte
typedef unsigned char                          U8;                                 // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short                         U16;                                // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned int                           U32;                                // 4 bytes
/// data type unsigned int, data length 8 byte
typedef unsigned long long                     U64;                               // 8 bytes

/// data type signed char, data length 1 byte
typedef signed char                         S8;                                 // 1 byte
/// data type signed short, data length 2 byte
typedef signed short                        S16;                                // 2 bytes
/// data type signed int, data length 4 byte
typedef signed int                          S32;                                // 4 bytes
/// data type unsigned int, data length 8 byte
typedef signed long long                    S64;                               // 8 bytes

/// definitaion for MStar BOOL
typedef unsigned short            B16;
/// definition for MS_BOOL, this will be removed later
typedef unsigned int	                    BOOL;

#define TQM_MODE

#if !defined(TRUE) && !defined(FALSE)
/// definition for TRUE
#define TRUE                        1
/// definition for FALSE
#define FALSE                       0
#endif

#if !defined(ENABLE) && !defined(DISABLE)
/// definition for ENABLE
#define ENABLE                      1
/// definition for DISABLE
#define DISABLE                     0
#endif

#if !defined(ON) && !defined(OFF)
/// definition for ON
#define ON                          1
/// definition for OFF
#define OFF                         0
#endif

/// @define TEE Info
#define SECURITY_TEEINFO_OSTYPE_NOTEE                 0
#define SECURITY_TEEINFO_OSTYPE_NUTTX                 1
#define SECURITY_TEEINFO_OSTYPE_OPTEE                 2
#define SECURITY_TEEINFO_OSTYPE_SECARM                 3
#define SECURITY_TEEINFO_OSTYPE_NO_TEE_NS             4
extern int TEEINFO_TYPTE;


/// @name BIT#
/// definition of one bit mask
/// @{
#if !defined(BIT0) && !defined(BIT1)
#define BIT0	                    0x00000001
#define BIT1	                    0x00000002
#define BIT2	                    0x00000004
#define BIT3	                    0x00000008
#define BIT4	                    0x00000010
#define BIT5	                    0x00000020
#define BIT6	                    0x00000040
#define BIT7	                    0x00000080
#define BIT8	                    0x00000100
#define BIT9	                    0x00000200
#define BIT10	                    0x00000400
#define BIT11	                    0x00000800
#define BIT12	                    0x00001000
#define BIT13	                    0x00002000
#define BIT14	                    0x00004000
#define BIT15  	                    0x00008000
#define BIT16                       0x00010000
#define BIT17                       0x00020000
#define BIT18                       0x00040000
#define BIT19                       0x00080000
#define BIT20                       0x00100000
#define BIT21                       0x00200000
#define BIT22                       0x00400000
#define BIT23                       0x00800000
#define BIT24                       0x01000000
#define BIT25                       0x02000000
#define BIT26                       0x04000000
#define BIT27                       0x08000000
#define BIT28                       0x10000000
#define BIT29                       0x20000000
#define BIT30                       0x40000000
#define BIT31                       0x80000000
#endif
/// @}

#if defined(CONFIG_MSTAR_MSYSTEM) || defined(CONFIG_MSTAR_MSYSTEM_MODULE)
  #define MSYSTEM_STATIC
#else//#if defined(CONFIG_MSTAR_MSYSTEM) || defined(CONFIG_MSTAR_MSYSTEM_MODULE)
  #define MSYSTEM_STATIC static
#endif//#if defined(CONFIG_MSTAR_MSYSTEM) || defined(CONFIG_MSTAR_MSYSTEM_MODULE)

struct arm_smccc_res_mstar {
	unsigned long a0;
	unsigned long a1;
	unsigned long a2;
	unsigned long a3;
};

typedef enum _GPIO_Edge
{
    E_GPIO_RISING_EDGE,
    E_GPIO_FALLING_EDGE
} GPIO_Edge;
#endif // #ifndef __MDRV_TYPES_H__
