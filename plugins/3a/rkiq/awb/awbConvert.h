#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <ebase/utl_fixfloat.h>
#include <common/return_codes.h>
#include "awb.h"

USE_TRACER(AWB_INFO);
USE_TRACER(AWB_WARNING);
USE_TRACER(AWB_ERROR);
#ifdef __cplusplus
extern "C"
{
#endif
/*****************************************************************************/
/**
 * @brief   This function converts float based gains into CamerIC 2.8 fixpoint
 *          format.
 *
 * @param   pAwbGains           gains in float based format
 * @param   pCamerIcGains       gains in fix point format (2.8)
 *
 * @return                      Returns the result of the function call.
 * @retval  RET_SUCCESS         gains sucessfully converted
 * @retval  RET_NULL_POINTER    null pointer parameter
 *
 *****************************************************************************/
RESULT AwbGains2CamerIcGains
(
    AwbGains_t*      pAwbGains,
    CamerIcGains_t*  pCamerIcGains
);



/*****************************************************************************/
/**
 * @brief   This function converts CamerIC 2.8 fixpoint format into float
 *          based gains.
 *
 * @param   pCamerIcGains       gains in fix point format (2.8)
 * @param   pAwbGains           gains in float based format
 *
 * @return                      Returns the result of the function call.
 * @retval  RET_SUCCESS         gains sucessfully converted
 * @retval  RET_NULL_POINTER    null pointer parameter
 *
 *****************************************************************************/
RESULT CamerIcGains2AwbGains
(
    CamerIcGains_t*  pCamerIcGains,
    AwbGains_t*      pAwbGains
);


/*****************************************************************************/
/**
 * @brief       This function converts float based Color correction matrix
 *              values into CamerIC 4.7 fixpoint format.
 *
 * @param[in]   pAwbXTalkOffset     offset as float values
 * @param[out]  pCamerIcXTalkOffset offsets as 2's complement integer
 *
 * @return                          Returns the result of the function call.
 * @retval      RET_SUCCESS         offsets sucessfully converted
 * @retval      RET_NULL_POINTER    null pointer parameter
 *
 *****************************************************************************/
RESULT AwbXtalk2CamerIcXtalk
(
    Cam3x3FloatMatrix_t* pAwbXTalkMatrix,
    CamerIc3x3Matrix_t*  pXTalkMatrix
);



/*****************************************************************************/
/**
 * @brief       This function converts CamerIC 4.7 fixpoint format based Color
 *              correction matrix into float based values.
 *
 * @param[in]   pCamerIcXTalkOffset offsets as 2's complement integer
 * @param[out]  pAwbXTalkOffset     offset as float values
 *
 * @return                          Returns the result of the function call.
 * @retval      RET_SUCCESS         offsets sucessfully converted
 * @retval      RET_NULL_POINTER    null pointer parameter
 *
 *****************************************************************************/
RESULT CamerIcXtalk2AwbXtalk
(
    CamerIc3x3Matrix_t*  pXTalkMatrix,
    Cam3x3FloatMatrix_t* pAwbXTalkMatrix
);



/*****************************************************************************/
/**
 * @brief   This function converts float based offset values into CamerIC 12.0
 *          fix point format based offset values.
 *
 * @param   pAwbXTalkOffset     offset as float values
 * @param   pCamerIcXTalkOffset offsets as 2's complement integer
 *
 * @return                      Returns the result of the function call.
 * @retval  RET_SUCCESS         offsets sucessfully converted
 * @retval  RET_NULL_POINTER    null pointer parameter
 *
 *****************************************************************************/
RESULT AwbXTalkOffset2CamerIcXTalkOffset
(
    Cam1x3FloatMatrix_t*     pAwbXTalkOffset,
    CamerIcXTalkOffset_t*    pCamerIcXTalkOffset
);



/*****************************************************************************/
/**
 * @brief   This function converts CamerIC 12.0 fix point format based offset
 *          values into float based offset value.
 *
 * @param   pCamerIcXTalkOffset offsets as 2's complement integer
 * @param   pAwbXTalkOffset     offset as float values
 *
 * @return                      Returns the result of the function call.
 * @retval  RET_SUCCESS         offsets sucessfully converted
 * @retval  RET_NULL_POINTER    null pointer parameter
 *
 *****************************************************************************/
RESULT CamerIcXTalkOffset2AwbXTalkOffset
(
    CamerIcXTalkOffset_t*    pCamerIcXTalkOffset,
    AwbXTalkOffset_t*        pAwbXTalkOffset
);

RESULT CamerIcAwbMeasure2AwbMeasure
(
    CamerIcAwbMeasuringResult_t*   pCamericMeasResult,
    AwbMeasuringResult_t*          pAwbMeasuringResult

);

RESULT AwbMeasure2CamerIcAwbMeasure
(
    CamerIcAwbMeasuringResult_t*   pCamericMeasResult,
    AwbMeasuringResult_t*          pAwbMeasuringResult

);
#ifdef __cplusplus
}
#endif
