/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Hunan Goke,Chengdu Goke,Shandong Goke. 2021. All rights reserved.
 */

#ifndef __GK_TYPE_H__
#define __GK_TYPE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*-----------------------------------------------------------------------
 * Defintion of basic data types. The data types are applicable to both
 * the application layer and kernel codes. *
 *-----------------------------------------------------------------------
 */
/*************************** Structure Definition ***************************/
/** Constant Definition */
typedef enum
{
    GK_FALSE    = 0,
    GK_TRUE     = 1,
} gk_bool;

#ifndef NULL
#define NULL                0L
#endif

#define GK_NULL             0L
#define GK_NULL_PTR         0L

#define GK_SUCCESS          0
#define GK_FAILURE          (-1)

#define GK_INVALID_HANDLE   (0xffffffff)

#define GK_INVALID_PTS      (0xffffffff)
#define GK_INVALID_TIME     (0xffffffff)

#define GK_OS_LINUX     0xabcd
#define GK_OS_WIN32     0xcdef

#ifdef _WIN32
#define GK_OS_TYPE      GK_OS_WIN32
#else
#define __OS_LINUX__
#define GK_OS_TYPE      GK_OS_LINUX
#endif

#ifdef GK_ADVCA_SUPPORT
#define __INIT__
#define __EXIT__
#else
#define __INIT__  __init
#define __EXIT__  __exit
#endif

/**
 * define of gk_handle(unsigned int):
 *bit31                                                           bit0
 *|<----   16bit --------->|<---   8bit    --->|<---  8bit   --->|
 *|--------------------------------------------------------------|
 *|      GK_MOD_ID_E       |  mod defined data |     chnID       |
 *|--------------------------------------------------------------|
 *mod defined data: private data define by each module
 *(for example: sub-mod id), usually, set to 0.
 */

#define GK_HANDLE_MAKEHANDLE(mod, privatedata, chnid)\
	(unsigned int)((((mod) & 0xffff) << 16) |\
	(((privatedata) & 0xff) << 8) | ((chnid) & 0xff))

#define GK_HANDLE_GET_MODID(handle)     (((handle) >> 16) & 0xffff)
#define GK_HANDLE_GET_PriDATA(handle)   (((handle) >> 8) & 0xff)
#define GK_HANDLE_GET_CHNID(handle)     (((handle)) & 0xff)

#define UNUSED(x) ((x)=(x))

/** @} */  /** <!-- ==== Structure Definition end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

