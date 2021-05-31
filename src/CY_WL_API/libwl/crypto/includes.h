/*
 * wpa_supplicant/hostapd - Default include files
 * Copyright (c) 2005-2006, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 *
 * This header file is included into all C files so that commonly used header
 * files can be selected with OS specific ifdef blocks in one place instead of
 * having to have OS/C library specific selection in many files.
 */

#ifndef INCLUDES_H
#define INCLUDES_H

/* Include possible build time configuration before including anything else */
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>

#ifndef os_strlen
#define os_strlen(s) strlen(s)
#endif

#ifndef os_memcpy
#define os_memcpy(d, s, n) memcpy((d), (s), (n))
#endif
#ifndef os_memset
#define os_memset(s, c, n) memset(s, c, n)
#endif

#define TEST_FAIL() 0

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

#define CONFIG_CRYPTO_INTERNAL

#endif /* INCLUDES_H */
