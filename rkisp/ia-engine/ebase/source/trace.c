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
 *   @file trace.c
 *
 *  This file defines the implementation for the tracing facility of the
 *  embedded lib.
 *
 *****************************************************************************/


#include "trace.h"

#include <stdarg.h>

#include "dct_assert.h"

#ifndef NDEBUG

static int glb_level = MAX_LEVEL;

static Tracer*  tracerListHead = NULL;

static const int BUFFSIZE = 1024;

/*  For some stupid reason beyond my imagination, gccs stdio.h doesnt */
/*  support vsnprintf() in strict c99 mode. Only happens in cygwin.   */
#if defined(__GNUC__) && defined(__CYGWIN__) && !defined(PICO)
int vsnprintf(char*, size_t, const char*, __VALIST);
#endif


int getTraceLevel(void) {
  return glb_level;
}
void setTraceLevel(int new_level) {
  if (TRACE_OFF != new_level) {
    new_level = (~(((uint32_t)(new_level)) - 1u)) & MAX_LEVEL;
  }
  glb_level = new_level;
}

void enableTracer(Tracer* t) {
  DCT_ASSERT(t);
  t->enabled = 1;
}


void disableTracer(Tracer* t) {
  DCT_ASSERT(t);
  t->enabled = 0;
}


void setTracerFile(Tracer* t, FILE* f) {
  DCT_ASSERT(t);
  t->fp = f;
}

void flushTracer(const Tracer* t) {
  if (t->fp) {
    (void) fflush(t->fp);
  }
}

static void addToList(Tracer* tracer) {
  if (tracerListHead) {
    tracer->next = tracerListHead;
  }
  tracer->linked = 1;
  tracerListHead = tracer;
}

Tracer* getTracerList(void) {
  return tracerListHead;
}

void trace(Tracer* tracer, const CHAR* sFormat, ...) {
  char buffer[BUFFSIZE];
  int length;
  va_list args;

  DCT_ASSERT(tracer);

  if (!tracer->linked) {
    addToList(tracer);
  }
  if ((tracer->level & glb_level) && (tracer->enabled != 0)) {
    va_start(args, sFormat);
    length = vsnprintf(buffer, BUFFSIZE, sFormat, args);
    if (!((length > 0) && (length < BUFFSIZE))) {
      /* message was truncated */
      fprintf(stderr, "Warning: Trace output truncated !");
    }
    va_end(args);

    if (tracer->fp == 0) {
      tracer->fp = stdout;
    }

    fprintf(tracer->fp, "%s%s\n", tracer->prefix, buffer);
    (void) fflush(tracer->fp);
  }
}

#endif  /* NDEBUG */
