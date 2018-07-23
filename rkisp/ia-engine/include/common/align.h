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
 * @file align.h
 *
 * @brief
 *
 *****************************************************************************/
#ifndef __ALIGN_H__
#define __ALIGN_H__

#define ALIGN_UP(addr, align)   ( ((addr) + ((align)-1)) & ~((align)-1) ) //!< Aligns addr to next higher aligned addr; align must be a power of two.
#define ALIGN_DOWN(addr, align)   ( ((addr)              ) & ~((align)-1) ) //!< Aligns addr to next lower aligned addr; align must be a power of two.

#define ALIGN_SIZE_1K               ( 0x400 )
#define ALIGN_UP_1K(addr)     ( ALIGN_UP(addr, ALIGN_SIZE_1K) )

#define MAX_ALIGNED_SIZE(size, align) ( ALIGN_UP(size, align) + align ) //!< Calcs max size of memory required to be able to hold a block of size bytes with a start address aligned to align.

#endif /* __ALIGN_H__ */
