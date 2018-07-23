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
#ifndef __ARRAY_SIZE_H__
#define __ARRAY_SIZE_H__

#define ARRAY_SIZE(arr)     ( sizeof(arr) / sizeof((arr)[0]) )

#endif /* __ARRAY_SIZE_H__ */
