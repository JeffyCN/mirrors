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
 *   @file types.h
 *
 *  This file defines some basic type names like the int types.
 *
 *****************************************************************************/
#ifndef TYPES_H_
#define TYPES_H_

#include "linux_compat.h"
#include <limits.h>

#ifndef __cplusplus
/* Only C99 compilers know stdbool */
#if (__STDC_VERSION__ >= 199901L) || defined(CONFIG_HAVE_STDBOOL)
#include <stdbool.h>
#else
#define bool    unsigned int
#define true    (1)
#define false   (0)
#endif
#endif


#if defined(__GNUC__)
#if !defined(INLINE)
#define INLINE static inline
#endif
#endif

#include <stddef.h>

#if defined(__cplusplus) || ((__STDC_VERSION__ >= 199901L))
#include <stdint.h>
#else

/* We only check for __int8_t_defined */
/* as this is all that gcc defines. */
#if !defined(__int8_t_defined)

/* In the rare cases that a system does not define
 * __int8_t_defined (Android p.e.) we check for the
 * include guard of the stdint header */
#if !defined(_STDINT_H)

typedef unsigned char        uint8_t;
typedef signed   char        int8_t;
typedef unsigned short       uint16_t;
typedef          short       int16_t;
typedef unsigned int         uint32_t;
typedef          int         int32_t;

#if !defined(_MSC_VER)
typedef signed   long long  int64_t;
typedef unsigned long long  uint64_t;
#else
typedef signed   __int64    int64_t;
typedef unsigned __int64    uint64_t;
#endif /* #if !defined(_MSC_VER) */

typedef unsigned int   uint_least32_t;
typedef          int   int_least32_t;
typedef unsigned int   uint_least8_t;
typedef unsigned int   uint;
typedef unsigned char  uchar;

#endif /* #if !defined(_STDINT_H) */
#endif /* #if !defined(__int8_t_defined) */
#endif /* #if defined(__cplusplus) || ((__STDC_VERSION__ >= 199901L)) */

#ifndef NULL
#define NULL ((void*)0)
#endif

/* make lint happy: */
typedef char CHAR;
typedef char char_t;    /* like suggested in  Misra 6.3 (P. 29) */

typedef enum {
  BOOL_FALSE = 0,
  BOOL_TRUE = (!BOOL_FALSE)
} bool_t;


#define UNUSED_PARAM(unref_param)  ((void) (unref_param));
#define CAST_POINTER_TO_UINT32( pointer ) ((uint32_t) (pointer))
#define CAST_POINTER_TO_INT32( pointer ) ((int32_t) (pointer))
#define CAST_UINT32_TO_POINTER( pointerType, value ) ((pointerType)(value))
#define CAST_INT32_TO_POINTER( value ) ((int32_t*)(value))
#define N_ELEMENTS(s)      (sizeof(s) / sizeof ((s) [0]))
#define ABS(a)         ((a) > 0 ? (a) : -(a))

#endif /*TYPES_H_*/
