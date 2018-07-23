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
 * @file return_types.h
 *
 * @brief
 *   Return types used by dct functions.
 *
 *****************************************************************************/
#ifndef __RETURN_TYPES_H__
#define __RETURN_TYPES_H__


#if defined (__cplusplus)
extern "C" {
#endif

#include "types.h"


/******************************************************************************/
/**
 *  @brief The standard dct return value type.
 *
 * All dct functions return a value of this type. Negative values denote an
 * error, positive values a success. For a list of possible standard return
 * values see @ref DCT_RESULT, for a list of module function specific return values
 * see the respective module's description
 */
typedef int32_t    DctReturn_t;


/******************************************************************************/
/**
 *  @brief The standard dct return values
 *
 * This enumeration lists the possible standard dct return values. Note that
 * each module can define its own return values for its module functions.
 * These module specific return values should start at @ref
 * DCT_RET_MODULE_ERROR_START for error values (descending from there) and at @ref
 * DCT_RET_MODULE_SUCCESS_START for success values (ascending).
 */
enum DCT_RESULT {
  DCT_RET_SUCCESS                =  0x00,    /**< The operation was successfully completed */
  DCT_RET_FAILURE                = -0x01,    /**< Generic error */
  DCT_RET_PENDING                = -0x02,    /**< The operation is ongoing */
  DCT_RET_INVALID_MODULE_TYPE_ID = -0x03,    /**< The given module type id is invalid */
  DCT_RET_NOT_SUPPORTED          = -0x04,    /**< The requested function is not supported */
  DCT_RET_OUT_OF_MEMORY          = -0x05,    /**< Insufficient memory */
  DCT_RET_TIMEOUT                = -0x06,    /**< Something timed out */
  DCT_RET_BUSY                   = -0x07,    /**< The requested operation can't be fulfilled because the resource is busy */
  DCT_RET_INVALID_PARM_1         = -0x08,    /**< First parameter is invalid */
  DCT_RET_INVALID_PARM_2         = -0x09,    /**< Second parameter is invalid */
  DCT_RET_INVALID_PARM_3         = -0x0A,    /**< Third parameter is invalid */
  DCT_RET_INVALID_PARM_4         = -0x0B,    /**< Fourth parameter is invalid */
  DCT_RET_INVALID_PARM_5         = -0x0C,    /**< Fifth parameter is invalid */
  DCT_RET_INVALID_PARM_6         = -0x0D,    /**< Sixth parameter is invalid */
  DCT_RET_CANCELLED              = -0x0E,    /**< The operation was cancelled */
  DCT_RET_NO_DATA                = -0x0F,    /**< Insufficient data for the operation to complete */
  DCT_RET_FULL                   = -0x10,    /**< Something is full */
  DCT_RET_OVERFLOW               = -0x11,    /**< Something was overflowing */
  DCT_RET_EMPTY                  = -0x12,    /**< Something is empty */
  DCT_RET_UNDERFLOW              = -0x13,    /**< Something was underflowing */
  DCT_RET_NOT_FOUND              = -0x14,    /**< Unsuccessful find operation */
  DCT_RET_INVALID_HANDLE         = -0x15,    /**< The given handle is invalid */
  DCT_RET_INVALID_CONFIG         = -0x16,    /**< Configuration is not allowed */
  DCT_RET_NO_HARDWARE            = -0x17,    /**< Hardware not present */
  DCT_RET_WRONG_STATE            = -0x18,    /**< Module is in wrong state */
  DCT_RET_WRITE_ERROR            = -0x19,    /**< Write error */
  DCT_RET_READ_ERROR             = -0x1A,    /**< Reads error */
  DCT_RET_INVALID_ADDRESS        = -0x1B,    /**< Invalid address */
  DCT_RET_ONLY_ONE_INSTANCE      = -0x1C,    /**< Only one instance is allowed */
  DCT_RET_NOT_AVAILABLE          = -0x1D,    /**< Something is not available */
  DCT_RET_INVALID_STREAM         = -0x1E,    /**< Stream could not be recognized as a valid stream of the specified standard / feature set */
  DCT_RET_MODULE_ERROR_START     = -0x100,   /**< Start of module specific error result codes */
  DCT_RET_MODULE_SUCCESS_START   =  0x100    /**< Start of module specific success result codes  */
};


#if defined (__cplusplus)
}
#endif

#endif /* __RETURN_TYPES_H__ */
