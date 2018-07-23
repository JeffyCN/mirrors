/******************************************************************************
 *
 * Copyright 2016, Fuzhou Rockchip Electronics Co.Ltd . All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 * 
 *
 *****************************************************************************/
/**
 *   @file builtins.h
 *
 *  This file defines some macros for standard library functions. Usually we
 *  dont link against glibc, so we use the builtins. Standard library function
 *  calls are only permitted in debug mode.
 *
 *****************************************************************************/
#ifndef BUILTINS_H_
#define BUILTINS_H_

#include "types.h"

#if defined(__GNUC__)
#include <stddef.h>

void* __builtin_memset(void* s, int32_t c, size_t n);
#define MEMSET( TARGET, C, LEN) __builtin_memset(TARGET, C, LEN)

void* __builtin_memcpy(void* s1, const void* s2, size_t n);
#define MEMCPY( DST, SRC, LEN)  __builtin_memcpy(DST,SRC,LEN)
#else
#include <string.h>
#define MEMSET( TARGET, C, LEN) memset(TARGET,C,LEN)
#define MEMCPY( DST, SRC, LEN)  memcpy(DST,SRC,LEN)
#endif

#define WIPEOBJ( TARGET ) MEMSET( &TARGET, 0, sizeof( TARGET ) )

#endif /*BUILTINS_H_*/
