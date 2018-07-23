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
 * @file misc.h
 *
 * @brief   Some often used little helpers; mainly implemented as macros.
 *
 *****************************************************************************/
#ifndef __MISC_H__
#define __MISC_H__

/* beware of macro side effects! */

#ifndef __FLT_EPSILON__
#define __FLT_EPSILON__     0.000000119209289550781250000000
#endif /* __FLT_EPSILON__ */

#ifndef FLT_EPSILON
#define FLT_EPSILON         __FLT_EPSILON__
#endif /* FLT_EPSILON */

#ifndef FLT_MAX
#define FLT_MAX     ((float)3.40282346638528860e+38)
#endif /* FLT_MAX */

#ifndef MIN
#define MIN(a, b)   ( ((a)<(b)) ? (a) : (b) )
#endif /* MIN */

#ifndef MAX
#define MAX(a, b)   ( ((a)>(b)) ? (a) : (b) )
#endif /* MAX */

#ifndef ABS
#define ABS(a)      ( ((a)<0) ? -(a) : (a) )
#endif /*ABS */

#ifndef SIGN
#define SIGN(a)     ( ((a)<0) ? -1 : ((a)>0) ? 1 : 0 )
#endif /* SIGN */

#endif /* __MISC_H__ */
