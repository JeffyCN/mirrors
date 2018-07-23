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
 *   @file dct_assert.c
 *
 *   This file defines the implementation for the assertion facility of the
 *   embedded lib.
 *
 *****************************************************************************/

#include "linux_compat.h"
#include "dct_assert.h"


#if defined(ENABLE_ASSERT) || !defined(NDEBUG)

ASSERT_HANDLER assert_handler = 0;

void exit_(const char* file, int line) {
  (void) fflush(stdout);
  (void) fflush(stderr);
  fprintf(stdout, "\n*** ASSERT: In File %s, line %d ***\n", file, line);

  if (assert_handler != 0) {
    /* If a handler is registered call it. */

    assert_handler();
  } else {
    exit(0);
  }
}
#endif
