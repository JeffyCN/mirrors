/*
 * Copyright 2012 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! \file ia_abstraction.h
   \brief Constants, definitions and macros used IA modules.
*/
#ifndef _IA_ABSTRACTION_H_
#define _IA_ABSTRACTION_H_

#include <stdlib.h>  /* defines malloc and free */
#include <string.h>  /* defines memcpy and memset */
#include <stddef.h>  /* defines NULL */
#include <stdint.h>  /* defines fixed width integers */
#include <math.h>

/*!
 * \brief extra Q number format typedefs.
*/
typedef int16_t sq7_8_t;
typedef uint16_t uq8_8_t;
typedef int32_t sq15_16_t;
typedef uint32_t uq16_16_t;

#define FLOAT_TO_Q16_16(n) (uint32_t)(((float)(n))*65536.0f)
#define INT_TO_Q16_16(n)   ((n)<<16)
#define Q16_16_TO_FLOAT(n) (((float)(n))*0.0000152587890625f)
#define Q16_16_TO_INT(n)   ((n)>>16)

#define FLOAT_TO_Q1_15(n)  (uint16_t)(((float)(n))*32768.0f)
#define Q1_15_TO_FLOAT(n)  (((float)(n))*0.000030518f)

#define FLOAT_TO_Q8_8(n)   (uint16_t)(((float)(n))*256.0f)
#define INT_TO_Q8_8(n)     ((n)<<8)
#define Q8_8_TO_FLOAT(n)   (((float)(n))*0.00390625f)
#define Q8_8_TO_INT(n)     ((n)>>8)

#define FLOAT_TO_QX_10(n)  ((n)*1024.0f)
#define FLOAT_TO_QX_12(n)  ((float)(n)*4096.0f)
#define INT_TO_QX_10(n)    ((n)<<10)
#define QX_10_TO_FLOAT(n)  (((float)(n))*0.0009765625f)
#define QX_12_TO_FLOAT(n)  (((float)(n))*0.0000038294f)
#define QX_10_TO_INT(n)    ((n)>>10)

#define Q16_12_TO_FLOAT(n) (((float)(n))*0.000244141f)

/*!
 * \brief Calculates aligned value.
 * Works only with unsigned values.
 * \param a Number to align.
 * \param b Alignment.
 * \return  Aligned number.
 */
#define IA_ALIGN(a,b)            (((unsigned)(a)+(unsigned)(b-1)) & ~(unsigned)(b-1))

#define IA_ALLOC(x)              malloc(x)
#define IA_CALLOC(x)             calloc(1, x)
#define IA_REALLOC(x, y)         realloc(x, y)
#define IA_FREEZ(x)              { free(x); x = NULL;}
#define IA_MEMCOPY(x, y, z)      memcpy(x, y, z)
#define IA_MEMMOVE(x,y,z)        memmove(x, y, z)
#define IA_MEMSET(x, y, z)       memset(x, y, z)
#define IA_MEMCOMPARE(x,y,z)     memcmp(x, y, z)
#define IA_ABS(a)                abs((int)(a))
#define IA_FABS(a)               fabs((double)(a))
#define IA_MIN(a, b)             ((a) < (b) ? (a) : (b))
#define IA_MAX(a, b)             ((a) > (b) ? (a) : (b))
#define IA_POW(a, b)             (float)pow((float)(a), (float)(b))
#define IA_EXP(a)                exp((double)(a))
#define IA_SQRT(a)               sqrt((double)(a))
#define IA_ROUND(a)              (((double)(a) > 0.0f) ? floor((double)(a) + 0.5f) : ceil((double)(a) - 0.5f))
#define IA_CEIL(a)               ceil((double)(a))
#define IA_FLOOR(a)              floor((double)(a))
#define IA_SIN(a)                sin((double)(a))
#define IA_COS(a)                cos((double)(a))
#define IA_ATAN(a)               atan ((double)(a))
#define IA_LN(a)                 log((double)(a))
#define IA_UNUSED(x)             (void)x
#define IA_LOG2(x)               (log((double)(x)) / log((double)2))

#ifdef _WIN32
#include <windows.h>

#if defined(_MSC_VER)
#define snprintf _snprintf
#if !defined(__BOOL_DEFINED)
typedef unsigned char bool;
#define true 1
#define false 0
#endif
#include <float.h>
#define IA_ISNAN(val) _isnan((double)(val))
#else
#include <stdbool.h> /* defines bool */
#define IA_ISNAN(val) isnan((double)(val))
#endif
#else
#define IA_ISNAN(val) isnan((double)(val))
#endif

#ifndef __cplusplus
#if (defined(_WIN32) || defined(WIN32)) && !defined(__GNUC__)
#define inline __inline
#elif defined(__GNUC__)
#define inline  __inline__
#else
#define inline                    /* default is to define inline to empty */
#endif
#endif

#endif /* _IA_ABSTRACTION_H_ */

