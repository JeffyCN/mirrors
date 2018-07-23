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
 * @file return_codes.h
 *
 * @brief
 *   This header files contains general return codes .
 *
 *****************************************************************************/
#ifndef __RETURN_CODES_H__
#define __RETURN_CODES_H__

typedef int RESULT;

#define RET_SUCCESS             0   //!< this has to be 0, if clauses rely on it
#define RET_FAILURE             1   //!< general failure
#define RET_NOTSUPP             2   //!< feature not supported
#define RET_BUSY                3   //!< there's already something going on...
#define RET_CANCELED            4   //!< operation canceled
#define RET_OUTOFMEM            5   //!< out of memory
#define RET_OUTOFRANGE          6   //!< parameter/value out of range
#define RET_IDLE                7   //!< feature/subsystem is in idle state
#define RET_WRONG_HANDLE        8   //!< handle is wrong
#define RET_NULL_POINTER        9   //!< the/one/all parameter(s) is a(are) NULL pointer(s)
#define RET_NOTAVAILABLE       10   //!< profile not available
#define RET_DIVISION_BY_ZERO   11   //!< a divisor equals ZERO
#define RET_WRONG_STATE        12   //!< state machine in wrong state
#define RET_INVALID_PARM       13   //!< invalid parameter
#define RET_PENDING            14   //!< command pending
#define RET_WRONG_CONFIG       15   //!< given configuration is invalid
#define RET_SOC_AF             16   //!< SOC AF   zyh@rock-chips.com: v0.0x20.0

#define UPDATE_RESULT( cur_res, new_res ) { RESULT __lres__ = (new_res); if (cur_res == RET_SUCCESS) cur_res = __lres__; } //!< Keeps first non-success result; cur_res must be a modifiable L-value; new_res can be any type of R-value including function call.

#define RETURN_RESULT_IF_DIFFERENT( cur_res, exp_res ) if ( exp_res != cur_res ) { return ( cur_res ); }

#endif /* __RETURN_CODES_H__ */

