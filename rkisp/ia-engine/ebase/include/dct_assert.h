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
 *   @file dct_assert.h
 *
 *  This file defines the API for the assertion facility of the embedded lib.
 *
 *****************************************************************************/
/*****************************************************************************/
/**
 * @defgroup module_assert Assert macros
 *
 *
 * Example use of the assert system:
 *
 *
 * - In your source file just use the macro
 *
 * @code
 * void foo( uint8_t* pData, size_t size)
 * {
 *     DCT_ASSERT(pData != NULL);
 *     DCT_ASSERT(size > 0);
 * }
 * @endcode
 *
 * @{
 *
 *****************************************************************************/
#ifndef ASSERT_H_
#define ASSERT_H_


/**
 * @brief   The type of the assert handler. @see assert_handler
 *
 *****************************************************************************/
typedef void (*ASSERT_HANDLER)(void);


/**
 *          The assert handler is a function that is called in case an
 *          assertion failed. If no handler is registered, which is the
 *          default, exit() is called.
 *
 *****************************************************************************/
extern ASSERT_HANDLER   assert_handler;

/**
 *          Compile time assert. Use e.g. to check the size of certain data
 *           types. As this is evaluated at compile time, it will neither cause
 *           size nor speed overhead, and thus is does not need to be inside
 *           the NDEBUG.
 *
 *****************************************************************************/
/* we need several levels of indirection to make unique enum names working
 * we need unique enum names to be able to use DCT_ASSERT_STATIC more than
 * one time per compilation unit
 */
#define UNIQUE_ENUM_NAME(u)     assert_static__ ## u
#define GET_ENUM_NAME(x)        UNIQUE_ENUM_NAME(x)
#define DCT_ASSERT_STATIC(e)    enum { GET_ENUM_NAME(__LINE__) = 1/(e) }

#if defined(ENABLE_ASSERT) || !defined(NDEBUG)
/**
 *              Dump information on stderr and exit.
 *
 *  @param      file  Filename where assertion occured.
 *  @param      line  Linenumber where assertion occured.
 *
 *****************************************************************************/
#ifdef __cplusplus
extern "C"
#endif
void exit_(const char* file, int line);


/**
 *              The assert macro.
 *
 *  @param      exp Expression which assumed to be true.
 *
 *****************************************************************************/
#define DCT_ASSERT(exp) do { if (!(exp)){ static CHAR filename[] = __FILE__; exit_(&filename[0], __LINE__);}}while(0)
#else
#define DCT_ASSERT(exp) ((void)0)
#endif

/* @} module_tracer*/

#endif /*ASSERT_H_*/
