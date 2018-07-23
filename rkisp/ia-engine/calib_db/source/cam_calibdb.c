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
 * @file cam_calibdb.c
 *
 * @brief
 *   Implementation of the CamEngine.
 *
 *****************************************************************************/
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <ebase/dct_assert.h>

//#include <common/utl_fixfloat.h>

/******************************************************************************
 * local macro definitions
 *****************************************************************************/
CREATE_TRACER(CAM_CALIBDB_INFO, "CAM-CALIBDB: ", INFO, 0);
CREATE_TRACER(CAM_CALIBDB_WARN, "CAM-CALIBDB: ", WARNING, 1);
CREATE_TRACER(CAM_CALIBDB_ERROR, "CAM-CALIBDB: ", ERROR, 1);
CREATE_TRACER(CAM_CALIBDB_DEBUG, "", INFO, 0);


