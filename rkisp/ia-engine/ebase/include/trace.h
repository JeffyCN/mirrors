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
 *   @file trace.h
 *
 *   This file defines the API for the tracing facility of the embedded lib.
 *
 *   WARNING:    Due to use of variadic macros which were introduced into C in
 *               c99 this can not be used with c++ code in pedantic mode.
 *
 *****************************************************************************/
/**
 * @defgroup module_tracer Trace System
 *
 *
 * Example use of the trace system:
 *
 * - Create a file like tracer_cfg.c
 *
 * @code
 * CREATE_TRACER(MODULE_INFO,       "ModInfo:   ", INFO,    1);
 * CREATE_TRACER(MODULE_WARNING,    "ModWarn:   ", WARNING, 1);
 * CREATE_TRACER(MODULE_ERROR,      "ModError:  ", ERROR,   1);
 * @endcode
 *
 * - In your source file import the tracer you like to use
 *
 * @code
 * USE_TRACER(MODULE_INFO);
 * USE_TRACER(MODULE_WARNING);
 * USE_TRACER(MODULE_ERROR);
 *
 * void foo()
 * {
 *     TRACE(MODULE_INFO, "enter %s\n", __FUNCTION);
 *     TRACE(MODULE_INFO, "leave %s\n", __FUNCTION);
 * }
 * @endcode
 *
 * @{
 *
 *****************************************************************************/
#ifndef TRACE_H_
#define TRACE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
/* must be defined even for release */
#include "linux_compat.h"
#include "types.h"

enum {
  TRACE_OFF   = 0x00,
  INFO        = 0x01,
  TRACE_DEBUG = 0x02,
  TRACE_NOTICE1 = 0x04,
  TRACE_NOTICE0 = 0x08,
  WARNING     = 0x10,
  ERROR       = 0x20,
  MAX_LEVEL   = 0x3F
};

typedef struct tracer_s {
  FILE*               fp;
  char_t*             prefix;
  int16_t             level;
  int8_t              enabled;
  int8_t              linked;
  char_t*             name;
  struct tracer_s*    next;
} Tracer;

#ifndef NDEBUG
int getTraceLevel(void);
void setTraceLevel(int);
void enableTracer(Tracer*);
void disableTracer(Tracer*);
void setTracerFile(Tracer*, FILE*);
void flushTracer(const Tracer*);
void trace(Tracer*, const CHAR*, ...);
Tracer* getTracerList(void);

#if !defined(USE_SDRAM_FOR_TRACE)
#define TRACER_DATA
#else
#define TRACER_DATA  DRAM_DATA
#endif
/**
 *
 *          This macro creates a Tracer. Every Tracer has its own output like
 *          stdout or a file and a trace level associated. A tracer may be
 *          enabled and disabled. If the global trace level is lower than the
 *          Tracers level, than output send to an enabled tracer is output to
 *          its file or stdout. If eg the global trace level is INFO and the
 *          Tracer has the level WARNING, its output is active. If the global
 *          trace level were ERROR instead, output of the tracer were
 *          suppressed.
 *
 *          \warning THIS MACRO MUST BE USED IN GLOBAL SCOPE.
 *
 *
 *  @param    name          Name of the tracer.
 *  @param    arg_prefix    All output of this tracer is preceeded by
 *                          <arg_prefix>.
 *  @param    arg_level     Initial trace level.
 *  @param    arg_enabled   Decide wether the tracer starts in enabled state.
 *
 *  @return   No return value.
 *
 *****************************************************************************/
#define CREATE_TRACER(name, arg_prefix, arg_level, arg_enabled) \
  CHAR tracerName##name[] TRACER_DATA = #name; \
  CHAR tracerPrefix##name[] TRACER_DATA = arg_prefix; \
  Tracer instance__##name TRACER_DATA =   \
                                          {                                       \
                                                                                  0,                                  \
                                                                                  &tracerPrefix##name[0],             \
                                                                                  arg_level,                          \
                                                                                  arg_enabled,                        \
                                                                                  0,                                  \
                                                                                  &tracerName##name[0],               \
                                                                                  NULL                                \
                                          };                                      \
  Tracer *name = &instance__##name

/**
 *
 *              If tracer was created in another compile unit this macro
 *              makes the named tracer available in the current unit.
 *
 *  @param      ...     First parameter is name of tracer.Use variable argument
 *                      list like printf.
 *  @return     No return value.
 *
 *****************************************************************************/
#define USE_TRACER(name)\
  extern Tracer *name

/**
 *
 *              Send output to a tracer.
 *
 *  @param      ...     First parameter is name of tracer.Use variable argument
 *                      list like printf.
 *  @return             No return value.
 *
 *****************************************************************************/
#define TRACE(...) trace(__VA_ARGS__)

/**
 *
 *              Send output to a tracer, If DEBUG_LEVEL if high enough
 *
 *  @param      ...     First parameter is the required DEBUG_LEVEL to get the
*                      output, second parameter is name of tracer. Use variable argument
 *                      list like printf.
 *  @return             No return value.
 *
 *****************************************************************************/
#if defined (DEBUG_LEVEL)
#define DL_TRACE(level, ...) if (DEBUG_LEVEL >= level) { trace(__VA_ARGS__); }
#else
#define DL_TRACE(level, ...) (void)0
#endif

/**
 *
 *              Enable a tracer.
 *
 *  @param      T   name of tracer.
 *  @return     No return value.
 *
 *****************************************************************************/
#define ENABLE_TRACER(T) enableTracer(T)

/**
 *
 *              Disable a tracer.
 *
 *  @param      T   name of tracer.
 *  @return     No return value.
 *
 *****************************************************************************/
#define DISABLE_TRACER(T) disableTracer(T)

/**
 *
 *              Set the global trace level.
 *
 *  @param      L   Trace level.
 *  @return     No return value.
 *
 *****************************************************************************/
#define SET_TRACE_LEVEL(L) setTraceLevel(L)

/**
 *
 *              Redirect a tracer to a file.
 *
 *  @param      T   name of tracer.
 *  @param      F   A valid FILE*.
 *
 *  @return     No return value.
 *
 *****************************************************************************/
#define SET_TRACER_FILE(T, F) setTracerFile(T, F)

/**
 *
 *              Flush a tracer.
 *
 *  @param      T   name of tracer.
 *
 *  @return     No return value.
 *
 *****************************************************************************/
#define FLUSH_TRACER(T)   flushTracer(T)
#define GET_TRACE_LEVEL()   getTraceLevel()
#define GET_TRACER_LIST()   getTracerList()

/* this macro can be used to define statements or variables which are only
 * active if NDEBUG is not defined:
 */
#define IF_TRACE_ON(x)              x
#else
/* The macro for CREATE_TRACER can not just expand to nothing. Otherwise we
* would have a single semicolon which C does not allow outside of function
* bodies. For this reason we expand to this extern declaration. As the
* external variable is never used linking should not be a problem. */
#define CREATE_TRACER(name, arg_prefix, arg_level, arg_enabled) extern int32_t name
#define USE_TRACER(name)            extern int32_t use##name
#define TRACE(...)                  (void)0
#define DL_TRACE(level, ...)    (void)0
#define ENABLE_TRACER(T)            (void)0
#define DISABLE_TRACER(T)           (void)0
#define SET_TRACE_LEVEL(L)          (void)0
#define SET_TRACER_FILE(T, F)       (void)0
#define FLUSH_TRACER(T)             (void)0
#define GET_TRACE_LEVEL()           (void)0
#define GET_TRACER_LIST()           (void)0

/* this macro can be used to define statements or variables which are only
 * active if NDEBUG is not defined:
 */
#define IF_TRACE_ON(x)
#endif /* NDEBUG */

#ifdef __cplusplus
}
#endif

/* @} module_tracer*/

#endif /*TRACE_H_*/
