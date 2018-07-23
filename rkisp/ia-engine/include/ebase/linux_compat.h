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
 * @file linux_compat.h
 *
 * @brief
 *   Linux compatibility layer.
 *
 *****************************************************************************/
#ifndef LINUX_COMPAT_H_
#define LINUX_COMPAT_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***** macro definitions *****************************************************/

/* comment this now, this will cause following compile error:*/
/*
/usr/include/c++/4.8/cstdlib: In function ¡®long long int std::abs(long long int)¡¯:
/usr/include/c++/4.8/cstdlib:174:20: error: declaration of C function ¡®long long int std::abs(long long int)¡¯ conflicts with
   abs(long long __x) { return __builtin_llabs (__x); }
                    ^
/usr/include/c++/4.8/cstdlib:166:3: error: previous declaration ¡®long int std::abs(long int)¡¯ here
   abs(long __i) { return __builtin_labs(__i); }

*/
#if 0
#ifdef __cplusplus
#include <cstdio>
#include <cstdlib>
#else
#include <stdio.h>
#include <stdlib.h>
#endif
#endif
/***** public type definitions ***********************************************/

/***** public function prototypes ********************************************/

#ifdef __cplusplus
}
#endif
#endif /* LINUX_COMPAT_H_ */

