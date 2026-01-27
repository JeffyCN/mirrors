/*
 *  Copyright (c) 2019 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "include/uAPI2/rk_aiq_user_api2_imgproc.h"
#include "uAPI2_c/rk_aiq_user_api2_common.h"
#include "uAPI2/rk_aiq_user_api2_ae.h"

#ifdef USE_NEWSTRUCT
#ifdef ISP_HW_V39
#include "rk_aiq_user_api2_isp39.h"
#elif  defined(ISP_HW_V33)
#include "rk_aiq_user_api2_isp33.h"
#elif  defined(ISP_HW_V32)
#include "rk_aiq_user_api2_isp32.h"
#elif  defined(ISP_HW_V35)
#include "rk_aiq_user_api2_isp35.h"
#endif
#endif

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#define RKAIQ_IMGPROC_CHECK_RET(ret, format, ...) \
    if (ret) { \
        LOGE(format, ##__VA_ARGS__); \
        return ret; \
    }

#define IMGPROC_FUNC_ENTER LOGD("%s: enter", __FUNCTION__);
#define IMGPROC_FUNC_EXIT LOGD("%s: exit", __FUNCTION__);

RKAIQ_BEGIN_DECLARE
#ifdef USE_IMPLEMENT_C
static bool isHDRmode(const rk_aiq_sys_ctx_t* ctx)
{
    RKAIQ_API_SMART_LOCK(ctx);
    int mode = RK_AIQ_WORKING_MODE_NORMAL;
    const rk_aiq_sys_ctx_t* sys_ctx = rk_aiq_user_api2_common_getSysCtx(ctx);
    mode = sys_ctx->_analyzer->mAlogsComSharedParams.working_mode;

    if (RK_AIQ_WORKING_MODE_NORMAL == mode)
        return false;
    else
        return true;
}

static int getHDRFrameNum(const rk_aiq_sys_ctx_t* ctx)
{
    RKAIQ_API_SMART_LOCK(ctx);
    int FrameNum = 1, working_mode = RK_AIQ_WORKING_MODE_NORMAL;

    const rk_aiq_sys_ctx_t* sys_ctx = rk_aiq_user_api2_common_getSysCtx(ctx);
    working_mode = sys_ctx->_analyzer->mAlogsComSharedParams.working_mode;

    switch (working_mode)
    {
    case RK_AIQ_WORKING_MODE_NORMAL:
        FrameNum = 1;
        break;
    case RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR:
    case RK_AIQ_ISP_HDR_MODE_2_LINE_HDR:
    case RK_AIQ_ISP_HDR_MODE_2_BUILTIN:
        FrameNum = 2;
        break;
    case RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR:
    case RK_AIQ_ISP_HDR_MODE_3_LINE_HDR:
    case RK_AIQ_ISP_HDR_MODE_3_BUILTIN:
        FrameNum = 3;
        break;
    default:
        FrameNum = 1;
        break;
    }
    return FrameNum;
}

/*
**********************************************************
*                        API of AEC module of V2
**********************************************************
*/

#ifndef USE_NEWSTRUCT
/*
*****************************
* Desc: set ae mode
* Argument:
*   mode contains: auto & manual
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setAeLock
(
    const rk_aiq_sys_ctx_t* ctx,
    bool on
) {

    IMGPROC_FUNC_ENTER
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttrV2_t expSwAttr;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "failed in getExpSwAttr!");
    expSwAttr.Enable = (!on);
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "failed in setExpSwAttr!");
    IMGPROC_FUNC_EXIT
    return (ret);

}
XCamReturn rk_aiq_uapi2_setExpMode
(
    const rk_aiq_sys_ctx_t* ctx,
    opMode_t mode
) {

    IMGPROC_FUNC_ENTER
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttrV2_t expSwAttr;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setExpMode failed in getExpSwAttr!");
    if (mode == OP_AUTO) {
        expSwAttr.AecOpType = RK_AIQ_OP_MODE_AUTO;
    } else if (mode == OP_MANUAL) {
        if (isHDRmode(ctx)) {
            expSwAttr.AecOpType = RK_AIQ_OP_MODE_MANUAL;
            expSwAttr.stManual.HdrAE.ManualGainEn = true;
            expSwAttr.stManual.HdrAE.ManualTimeEn = true;
        } else {
            expSwAttr.AecOpType = RK_AIQ_OP_MODE_MANUAL;
            expSwAttr.stManual.LinearAE.ManualGainEn = true;
            expSwAttr.stManual.LinearAE.ManualTimeEn = true;
        }
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "mode is not supported!");
    }
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setExpMode failed in setExpSwAttr!");
    IMGPROC_FUNC_EXIT
    return (ret);

}
XCamReturn rk_aiq_uapi2_getExpMode
(
    const rk_aiq_sys_ctx_t* ctx,
    opMode_t *mode
) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttrV2_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (mode == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }
    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getExpMode failed in getExpSwAttr!");
    if (expSwAttr.AecOpType == RK_AIQ_OP_MODE_AUTO) {
        *mode = OP_AUTO;
    } else if (expSwAttr.AecOpType == RK_AIQ_OP_MODE_MANUAL) {
        *mode = OP_MANUAL;
    }
    IMGPROC_FUNC_EXIT
    return (ret);
}

/*
*****************************
*
* Desc: set exposure parameter
* Argument:
*    auto exposure mode:
*      exposure gain will be adjust between [gain->min, gain->max]
*    manual exposure mode:
*      gain->min == gain->max
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setExpGainRange
(
    const rk_aiq_sys_ctx_t* ctx,
    paRange_t *gain
) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttrV2_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (gain == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }
    LOGD("set range: [%f, %f]", gain->min, gain->max);

    if (gain->min < 1.0f || gain->max < 1.0f || (gain->min > gain->max)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "gain range is wrong!");
    }
    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetExpGainRange failed!");

    expSwAttr.stAdvanced.SetAeRangeEn = true;
    if (isHDRmode(ctx)) {
        expSwAttr.stAdvanced.SetHdrAeRange.stGainRange[0].Max = gain->max;
        expSwAttr.stAdvanced.SetHdrAeRange.stGainRange[0].Min = gain->min;
        expSwAttr.stAdvanced.SetHdrAeRange.stGainRange[1].Max = gain->max;
        expSwAttr.stAdvanced.SetHdrAeRange.stGainRange[1].Min = gain->min;
        expSwAttr.stAdvanced.SetHdrAeRange.stGainRange[2].Max = gain->max;
        expSwAttr.stAdvanced.SetHdrAeRange.stGainRange[2].Min = gain->min;
    } else {
        expSwAttr.stAdvanced.SetLinAeRange.stGainRange.Max = gain->max;
        expSwAttr.stAdvanced.SetLinAeRange.stGainRange.Min = gain->min;
    }
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\nsetExpGainRange failed!");
    IMGPROC_FUNC_EXIT
    return (ret);

}
XCamReturn rk_aiq_uapi2_getExpGainRange
(
    const rk_aiq_sys_ctx_t* ctx,
    paRange_t *gain
) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttrV2_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (gain == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\ngetExpGainRange failed!");
    if (isHDRmode(ctx)) {
        int index = getHDRFrameNum(ctx);
        gain->max = expSwAttr.stAuto.HdrAeRange.stGainRange[index - 1].Max;
        gain->min = expSwAttr.stAuto.HdrAeRange.stGainRange[index - 1].Min;
    } else {
        gain->max = expSwAttr.stAuto.LinAeRange.stGainRange.Max;
        gain->min = expSwAttr.stAuto.LinAeRange.stGainRange.Min;
    }

    IMGPROC_FUNC_EXIT
    return (ret);

}
/*
*****************************
*
* Desc: set exposure parameter
* Argument:
*    auto exposure mode:
*       exposure time will be adjust between [time->min, time->max]
*    manual exposure mode:
*       exposure time will be set gain->min == gain->max;
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setExpTimeRange
(
    const rk_aiq_sys_ctx_t* ctx,
    paRange_t *time
) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttrV2_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (time == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    if ( time->min > time->max ) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "time range is wrong!");
    }
    LOGD("set range: [%f, %f]", time->min, time->max);
    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetExpTimeRange failed!");

    expSwAttr.stAdvanced.SetAeRangeEn = true;
    if (isHDRmode(ctx)) {
        expSwAttr.stAdvanced.SetHdrAeRange.stExpTimeRange[0].Max = time->max;
        expSwAttr.stAdvanced.SetHdrAeRange.stExpTimeRange[0].Min = time->min;
        expSwAttr.stAdvanced.SetHdrAeRange.stExpTimeRange[1].Max = time->max;
        expSwAttr.stAdvanced.SetHdrAeRange.stExpTimeRange[1].Min = time->min;
        expSwAttr.stAdvanced.SetHdrAeRange.stExpTimeRange[2].Max = time->max;
        expSwAttr.stAdvanced.SetHdrAeRange.stExpTimeRange[2].Min = time->min;
    } else {
        expSwAttr.stAdvanced.SetLinAeRange.stExpTimeRange.Max = time->max;
        expSwAttr.stAdvanced.SetLinAeRange.stExpTimeRange.Min = time->min;
    }
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\nsetExpTimeRange failed!");
    IMGPROC_FUNC_EXIT
    return (ret);

}
XCamReturn rk_aiq_uapi2_getExpTimeRange
(
    const rk_aiq_sys_ctx_t* ctx,
    paRange_t *time
) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttrV2_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (time == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\ngetExpTimeRange failed!");
    if (isHDRmode(ctx)) {
        int index = getHDRFrameNum(ctx);
        time->max = expSwAttr.stAuto.HdrAeRange.stExpTimeRange[index - 1].Max;
        time->min = expSwAttr.stAuto.HdrAeRange.stExpTimeRange[index - 1].Min;
    } else {
        time->max = expSwAttr.stAuto.LinAeRange.stExpTimeRange.Max;
        time->min = expSwAttr.stAuto.LinAeRange.stExpTimeRange.Min;
    }

    IMGPROC_FUNC_EXIT
    return (ret);

}

/*
*****************************
*
* Desc: backlight compensation
* Argument:
*      on:  1  on
*           0  off
*      areaType: backlight compensation area
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setBLCMode(const rk_aiq_sys_ctx_t* ctx, bool on, aeMeasAreaType_t areaType)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    Uapi_LinExpAttrV2_t LineExpAttr;
    memset(&LineExpAttr, 0x00, sizeof(Uapi_LinExpAttrV2_t));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    if (isHDRmode(ctx)) {
        ret = XCAM_RETURN_ERROR_FAILED;
        RKAIQ_IMGPROC_CHECK_RET(ret, "Not support in HDR mode!");
    }

    ret = rk_aiq_user_api2_ae_getLinExpAttr(ctx, &LineExpAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getLinExpAttr error!");
    LineExpAttr.Params.BackLightCtrl.Enable = on ? 1 : 0;
    LineExpAttr.Params.BackLightCtrl.MeasArea = (CalibDb_AecMeasAreaModeV2_t)areaType;
    LineExpAttr.Params.BackLightCtrl.StrBias = 0;
    ret = rk_aiq_user_api2_ae_setLinExpAttr(ctx, LineExpAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setBLCMode error!");
    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: backlight compensation strength,only available in normal mode
* Argument:
*      strength:  [1,100]
*****************************
*/
XCamReturn rk_aiq_uapi2_setBLCStrength(const rk_aiq_sys_ctx_t* ctx, int strength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_LinExpAttrV2_t LineExpAttr;
    memset(&LineExpAttr, 0x00, sizeof(Uapi_LinExpAttrV2_t));

    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    if (isHDRmode(ctx)) {
        ret = XCAM_RETURN_ERROR_FAILED;
        RKAIQ_IMGPROC_CHECK_RET(ret, "Not support in HDR mode!");
    } else {
        ret = rk_aiq_user_api2_ae_getLinExpAttr(ctx, &LineExpAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getLinExpAttr error!");
        if(0 == LineExpAttr.Params.BackLightCtrl.Enable)
            RKAIQ_IMGPROC_CHECK_RET(ret, "blc mode is not enabled!");
        LineExpAttr.Params.BackLightCtrl.StrBias = strength;
        ret = rk_aiq_user_api2_ae_setLinExpAttr(ctx, LineExpAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setBLCStrength error!");
    }

    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: highlight compensation
* Argument:
*      on:  1  on
*           0  off
*****************************
*/
XCamReturn rk_aiq_uapi2_setHLCMode(const rk_aiq_sys_ctx_t* ctx, bool on)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_LinExpAttrV2_t LinExpAttr;
    memset(&LinExpAttr, 0x00, sizeof(Uapi_LinExpAttrV2_t));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    if (isHDRmode(ctx)) {
        ret = XCAM_RETURN_ERROR_FAILED;
        RKAIQ_IMGPROC_CHECK_RET(ret, "Not support in HDR mode!");
    } else {
        ret = rk_aiq_user_api2_ae_getLinExpAttr(ctx, &LinExpAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\n setHLCMode failed!");
        LinExpAttr.Params.OverExpCtrl.Enable = on ? 1 : 0;
        LinExpAttr.Params.OverExpCtrl.StrBias = 0;
        ret = rk_aiq_user_api2_ae_setLinExpAttr(ctx, LinExpAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\n setHLCMode failed!");
    }
    IMGPROC_FUNC_EXIT
    return ret;

}

/*
*****************************
*
* Desc: highlight compensation strength
* Argument:
*      strength:  [1,100]
*****************************
*/
XCamReturn rk_aiq_uapi2_setHLCStrength(const rk_aiq_sys_ctx_t* ctx, int strength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_LinExpAttrV2_t LinExpAttr;
    memset(&LinExpAttr, 0x00, sizeof(Uapi_LinExpAttrV2_t));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    if (isHDRmode(ctx)) {
        ret = XCAM_RETURN_ERROR_FAILED;
        RKAIQ_IMGPROC_CHECK_RET(ret, "Not support in HDR mode!");
    } else {
        ret = rk_aiq_user_api2_ae_getLinExpAttr(ctx, &LinExpAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getLinExpAttr error!");
        if(0 == LinExpAttr.Params.OverExpCtrl.Enable)
            RKAIQ_IMGPROC_CHECK_RET(ret, "hlc mode is not enabled!");
        LinExpAttr.Params.OverExpCtrl.StrBias = strength;
        ret = rk_aiq_user_api2_ae_setLinExpAttr(ctx, LinExpAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setHLCStrength error!");
    }
    IMGPROC_FUNC_EXIT
    return ret;

}
/*
*****************************
*
* Desc: set anti-flicker mode
* Argument:
*    mode
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setAntiFlickerEn(const rk_aiq_sys_ctx_t* ctx, bool on)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttrV2_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetAntiFlickerEn failed!");
    expSwAttr.stAuto.stAntiFlicker.enable = on;

    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\nsetAntiFlickerEn failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}
/*
*****************************
*
* Desc: set anti-flicker mode
* Argument:
*    mode
*
*****************************
*/
XCamReturn rk_aiq_uapi2_getAntiFlickerEn(const rk_aiq_sys_ctx_t* ctx, bool* on)
{

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttrV2_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\ngetAntiFlickerEn!");
    *on = expSwAttr.stAuto.stAntiFlicker.enable;
    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set anti-flicker mode
* Argument:
*    mode
*
*****************************
*/

XCamReturn rk_aiq_uapi2_setAntiFlickerMode(const rk_aiq_sys_ctx_t* ctx, antiFlickerMode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttrV2_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetAntiFlickerMode failed!");
    if (mode == ANTIFLICKER_AUTO_MODE) {
        expSwAttr.stAuto.stAntiFlicker.enable = true;
        expSwAttr.stAuto.stAntiFlicker.Mode = AECV2_ANTIFLICKER_AUTO_MODE;
    } else if(mode == ANTIFLICKER_NORMAL_MODE) {
        expSwAttr.stAuto.stAntiFlicker.enable = true;
        expSwAttr.stAuto.stAntiFlicker.Mode = AECV2_ANTIFLICKER_NORMAL_MODE;
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "mode is invalid!");
    }
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\nsetAntiFlickerMode failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getAntiFlickerMode(const rk_aiq_sys_ctx_t* ctx, antiFlickerMode_t *mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttrV2_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);

    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\ngetAntiFlickerMode!");
    if (expSwAttr.stAuto.stAntiFlicker.Mode == AECV2_ANTIFLICKER_AUTO_MODE)
        *mode = ANTIFLICKER_AUTO_MODE;
    else if (expSwAttr.stAuto.stAntiFlicker.Mode == AECV2_ANTIFLICKER_NORMAL_MODE)
        *mode = ANTIFLICKER_NORMAL_MODE;
    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set power line frequence
* Argument:
*    freq
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setExpPwrLineFreqMode(const rk_aiq_sys_ctx_t* ctx, expPwrLineFreq_t freq)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttrV2_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetExpPwrLineFreqMode failed!");
    if (freq == EXP_PWR_LINE_FREQ_50HZ) {
        expSwAttr.stAuto.stAntiFlicker.enable = true;
        expSwAttr.stAuto.stAntiFlicker.Frequency = AECV2_FLICKER_FREQUENCY_50HZ;
    } else if (freq == EXP_PWR_LINE_FREQ_60HZ) {
        expSwAttr.stAuto.stAntiFlicker.enable = true;
        expSwAttr.stAuto.stAntiFlicker.Frequency = AECV2_FLICKER_FREQUENCY_60HZ;
    } else if (freq == EXP_PWR_LINE_FREQ_DIS) {
        expSwAttr.stAuto.stAntiFlicker.enable = true;
        expSwAttr.stAuto.stAntiFlicker.Frequency = AECV2_FLICKER_FREQUENCY_OFF;
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "freq is invalid!");
    }
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\nsetExpPwrLineFreqMode failed!");
    IMGPROC_FUNC_EXIT
    return (ret);
}
XCamReturn rk_aiq_uapi2_getExpPwrLineFreqMode
(
    const rk_aiq_sys_ctx_t* ctx,
    expPwrLineFreq_t *freq
) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttrV2_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetExpPwrLineFreqMode failed!");
    if (expSwAttr.stAuto.stAntiFlicker.Frequency == AECV2_FLICKER_FREQUENCY_50HZ) {
        *freq = EXP_PWR_LINE_FREQ_50HZ;
    } else if (expSwAttr.stAuto.stAntiFlicker.Frequency == AECV2_FLICKER_FREQUENCY_60HZ) {
        *freq = EXP_PWR_LINE_FREQ_60HZ;
    } else if (expSwAttr.stAuto.stAntiFlicker.Frequency == AECV2_FLICKER_FREQUENCY_OFF) {
        *freq = EXP_PWR_LINE_FREQ_DIS;
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "freq is invalid!");
    }
    IMGPROC_FUNC_EXIT
    return (ret);

}

#else
/*
*****************************
* Desc: set ae mode
* Argument:
*   mode contains: auto & manual
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setAeLock(
    const rk_aiq_sys_ctx_t* ctx,
    bool on)
{
    IMGPROC_FUNC_ENTER
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "failed in getExpSwAttr!");
    expSwAttr.commCtrl.sw_aeT_algo_en = (!on);
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "failed in setExpSwAttr!");
    IMGPROC_FUNC_EXIT
    return (ret);
}
XCamReturn rk_aiq_uapi2_setExpMode(
    const rk_aiq_sys_ctx_t* ctx,
    opMode_t mode)
{
    IMGPROC_FUNC_ENTER
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setExpMode failed in getExpSwAttr!");
    if (mode == OP_AUTO) {
        expSwAttr.commCtrl.sw_aeT_opt_mode = RK_AIQ_OP_MODE_AUTO;
    } else if (mode == OP_MANUAL) {
        if (isHDRmode(ctx)) {
            expSwAttr.commCtrl.sw_aeT_opt_mode = RK_AIQ_OP_MODE_MANUAL;
            expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_en = true;
            expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_en = true;
        } else {
            expSwAttr.commCtrl.sw_aeT_opt_mode = RK_AIQ_OP_MODE_MANUAL;
            expSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manTime_en = true;
            expSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manGain_en = true;
        }
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "mode is not supported!");
    }
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setExpMode failed in setExpSwAttr!");
    IMGPROC_FUNC_EXIT
    return (ret);
}
XCamReturn rk_aiq_uapi2_getExpMode(
    const rk_aiq_sys_ctx_t* ctx,
    opMode_t* mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (mode == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }
    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getExpMode failed in getExpSwAttr!");
    if (expSwAttr.commCtrl.sw_aeT_opt_mode == RK_AIQ_OP_MODE_AUTO) {
        *mode = OP_AUTO;
    } else if (expSwAttr.commCtrl.sw_aeT_opt_mode == RK_AIQ_OP_MODE_MANUAL) {
        *mode = OP_MANUAL;
    }
    IMGPROC_FUNC_EXIT
    return (ret);
}

/*
***********************************
* Desc: set/get exp time mode
* Argument:
*   mode contains: auto & manual
*
***********************************
*/
XCamReturn rk_aiq_uapi2_setExpTimeMode(
    const rk_aiq_sys_ctx_t* ctx,
    opMode_t mode)
{
    IMGPROC_FUNC_ENTER
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setExpTimeMode failed in getExpSwAttr!");
    if (expSwAttr.commCtrl.sw_aeT_opt_mode == RK_AIQ_OP_MODE_AUTO) {
        LOGE("setExpTimeMode faile, please use api rk_aiq_uapi2_setExpMode set ae to manual mode");
    } else if (mode == OP_AUTO) {
        if (isHDRmode(ctx)) {
            expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_en = false;
        } else {
            expSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manTime_en = false;
        }
    } else if (mode == OP_MANUAL) {
        if (isHDRmode(ctx)) {
            expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_en = true;
        } else {
            expSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manTime_en = true;
        }
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "mode is not supported!");
    }
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setExpTimeMode failed in setExpSwAttr!");
    IMGPROC_FUNC_EXIT
    return (ret);
}
XCamReturn rk_aiq_uapi2_getExpTimeMode(
    const rk_aiq_sys_ctx_t* ctx,
    opMode_t* mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (mode == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }
    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getExpTimeMode failed in getExpSwAttr!");
    if (expSwAttr.commCtrl.sw_aeT_opt_mode == RK_AIQ_OP_MODE_AUTO) {
        *mode = OP_AUTO;
    } else if (expSwAttr.commCtrl.sw_aeT_opt_mode == RK_AIQ_OP_MODE_MANUAL) {
        if (expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_en) {
            *mode = OP_MANUAL;
        } else {
            *mode = OP_AUTO;
        }
    }
    IMGPROC_FUNC_EXIT
    return (ret);
}

/*
***********************************
* Desc: set/get exp gain mode
* Argument:
*   mode contains: auto & manual
*
***********************************
*/
XCamReturn rk_aiq_uapi2_setExpGainMode(
    const rk_aiq_sys_ctx_t* ctx,
    opMode_t mode)
{
    IMGPROC_FUNC_ENTER
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setExpGainMode failed in getExpSwAttr!");
    if (expSwAttr.commCtrl.sw_aeT_opt_mode == RK_AIQ_OP_MODE_AUTO) {
        LOGE("setExpGainMode faile, please use api rk_aiq_uapi2_setExpMode set ae to manual mode");
    } else if (mode == OP_AUTO) {
        if (isHDRmode(ctx)) {
            expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_en = false;
        } else {
            expSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manGain_en = false;
        }
    } else if (mode == OP_MANUAL) {
        if (isHDRmode(ctx)) {
            expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_en = true;
        } else {
            expSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manGain_en = true;
        }
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "mode is not supported!");
    }
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setExpGainMode failed in setExpSwAttr!");
    IMGPROC_FUNC_EXIT
    return (ret);
}
XCamReturn rk_aiq_uapi2_getExpGainMode(
    const rk_aiq_sys_ctx_t* ctx,
    opMode_t* mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (mode == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }
    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getExpGainMode failed in getExpSwAttr!");
    if (expSwAttr.commCtrl.sw_aeT_opt_mode == RK_AIQ_OP_MODE_AUTO) {
        *mode = OP_AUTO;
    } else if (expSwAttr.commCtrl.sw_aeT_opt_mode == RK_AIQ_OP_MODE_MANUAL) {
        if (expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_en) {
            *mode = OP_MANUAL;
        } else {
            *mode = OP_AUTO;
        }
    }
    IMGPROC_FUNC_EXIT
    return (ret);
}

/*
*****************************
* Desc: set manual gain
* Argument:
*   gain > 1.0
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setExpManualGain(
    const rk_aiq_sys_ctx_t* ctx,
    float gain)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }
    LOGD("set manual gain: [%f]", gain);

    if (gain < 1.0f) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "gain is wrong!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetExpManualGain failed!");

    if (isHDRmode(ctx)) {
        expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_val[0] = gain;
        expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_val[1] = gain;
        expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_val[2] = gain;
    } else {
        expSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manGain_val = gain;
    }
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\nsetExpManualGain failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}
/*
*****************************
* Desc: set manual time
* Argument:
*   time
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setExpManualTime(
    const rk_aiq_sys_ctx_t* ctx,
    float time)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }
    LOGD("set manual time: [%f]", time);

    if (time < 0.0f) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "gain is wrong!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetExpManualTime failed!");

    if (isHDRmode(ctx)) {
        expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_val[0] = time;
        expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_val[1] = time;
        expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_val[2] = time;
    } else {
        expSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manTime_val = time;
    }
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\nsetExpManualTime failed!");
    IMGPROC_FUNC_EXIT
    return ret;

}
/*
*****************************
* Desc: set frame rate
* Argument:
*   info.mode OP_AUTO or OP_MANUAL
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setFrameRate(const rk_aiq_sys_ctx_t* ctx, frameRateInfo_t info)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (info.mode < OP_AUTO || info.mode >= OP_INVAL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetFrameRate failed!");
    if (info.mode == OP_AUTO) {
        expSwAttr.commCtrl.frmRate.sw_aeT_frmRate_mode = ae_frmRate_auto_mode;
        expSwAttr.commCtrl.frmRate.sw_aeT_frmRate_val  = info.fps;
    } else if (info.mode == OP_MANUAL) {
        expSwAttr.commCtrl.frmRate.sw_aeT_frmRate_mode = ae_frmRate_fix_mode;
        expSwAttr.commCtrl.frmRate.sw_aeT_frmRate_val  = info.fps;
    }
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);

    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\nsetFrameRate failed!");
    IMGPROC_FUNC_EXIT
    return ret;

}

XCamReturn rk_aiq_uapi2_setSnsVts(const rk_aiq_sys_ctx_t* ctx, uint32_t vts)
{
    IMGPROC_FUNC_ENTER
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx->_camHw && ctx->_camHw->_mSensorDev)
        AiqSensorHw_setUserVts(ctx->_camHw->_mSensorDev, vts);
    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set exposure parameter
* Argument:
*    auto exposure mode:
*      exposure gain will be adjust between [gain->min, gain->max]
*    manual exposure mode:
*      gain->min == gain->max
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setExpGainRange(
    const rk_aiq_sys_ctx_t* ctx,
    paRange_t* gain)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (gain == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }
    LOGD("set range: [%f, %f]", gain->min, gain->max);

    if (gain->min < 1.0f || gain->max < 1.0f || (gain->min > gain->max)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "gain range is wrong!");
    }
    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetExpGainRange failed!");

    expSwAttr.advanced.sw_aeT_advAeRange_en = true;
    if (isHDRmode(ctx)) {
        expSwAttr.advanced.hdrExpRange[0].sw_aeT_gain_max = gain->max;
        expSwAttr.advanced.hdrExpRange[0].sw_aeT_gain_min = gain->min;
        expSwAttr.advanced.hdrExpRange[1].sw_aeT_gain_max = gain->max;
        expSwAttr.advanced.hdrExpRange[1].sw_aeT_gain_min = gain->min;
        expSwAttr.advanced.hdrExpRange[2].sw_aeT_gain_max = gain->max;
        expSwAttr.advanced.hdrExpRange[2].sw_aeT_gain_min = gain->min;
    } else {
        expSwAttr.advanced.linExpRange.sw_aeT_gain_max = gain->max;
        expSwAttr.advanced.linExpRange.sw_aeT_gain_min = gain->min;
    }
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\nsetExpGainRange failed!");
    IMGPROC_FUNC_EXIT
    return (ret);
}
XCamReturn rk_aiq_uapi2_getExpGainRange(
    const rk_aiq_sys_ctx_t* ctx,
    paRange_t* gain)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_queryInfo_t queryInfo;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (gain == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api2_ae_queryExpResInfo(ctx, &queryInfo);
    RKAIQ_IMGPROC_CHECK_RET(ret, "query exp info failed!\ngetExpGainRange failed!");
    if (isHDRmode(ctx)) {
        int index = getHDRFrameNum(ctx);
        gain->max = queryInfo.hdrExpInfo.expRange[index - 1].sw_aeT_gain_max;
        gain->min = queryInfo.hdrExpInfo.expRange[index - 1].sw_aeT_gain_min;
    } else {
        gain->max = queryInfo.linExpInfo.expRange.sw_aeT_gain_max;
        gain->min = queryInfo.linExpInfo.expRange.sw_aeT_gain_min;
    }

    IMGPROC_FUNC_EXIT
    return (ret);
}
/*
*****************************
*
* Desc: set exposure parameter
* Argument:
*    auto exposure mode:
*       exposure time will be adjust between [time->min, time->max]
*    manual exposure mode:
*       exposure time will be set gain->min == gain->max;
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setExpTimeRange(
    const rk_aiq_sys_ctx_t* ctx,
    paRange_t* time)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (time == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    if (time->min > time->max) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "time range is wrong!");
    }
    LOGD("set range: [%f, %f]", time->min, time->max);
    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetExpTimeRange failed!");

    expSwAttr.advanced.sw_aeT_advAeRange_en = true;
    if (isHDRmode(ctx)) {
        expSwAttr.advanced.hdrExpRange[0].sw_aeT_time_max = time->max;
        expSwAttr.advanced.hdrExpRange[0].sw_aeT_time_min = time->min;
        expSwAttr.advanced.hdrExpRange[1].sw_aeT_time_max = time->max;
        expSwAttr.advanced.hdrExpRange[1].sw_aeT_time_min = time->min;
        expSwAttr.advanced.hdrExpRange[2].sw_aeT_time_max = time->max;
        expSwAttr.advanced.hdrExpRange[2].sw_aeT_time_min = time->min;
    } else {
        expSwAttr.advanced.linExpRange.sw_aeT_time_max = time->max;
        expSwAttr.advanced.linExpRange.sw_aeT_time_min = time->min;
    }
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\nsetExpTimeRange failed!");
    IMGPROC_FUNC_EXIT
    return (ret);
}
XCamReturn rk_aiq_uapi2_getExpTimeRange(
    const rk_aiq_sys_ctx_t* ctx,
    paRange_t* time)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_queryInfo_t queryInfo;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (time == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api2_ae_queryExpResInfo(ctx, &queryInfo);
    RKAIQ_IMGPROC_CHECK_RET(ret, "query exp info failed!\ngetExpTimeRange failed!");
    if (isHDRmode(ctx)) {
        int index = getHDRFrameNum(ctx);
        time->max = queryInfo.hdrExpInfo.expRange[index - 1].sw_aeT_time_max;
        time->min = queryInfo.hdrExpInfo.expRange[index - 1].sw_aeT_time_min;
    } else {
        time->max = queryInfo.linExpInfo.expRange.sw_aeT_time_max;
        time->min = queryInfo.linExpInfo.expRange.sw_aeT_time_min;
    }

    IMGPROC_FUNC_EXIT
    return (ret);
}
/*
*****************************
*
* Desc: backlight compensation
* Argument:
*      on:  1  on
*           0  off
*      areaType: backlight compensation area
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setBLCMode(const rk_aiq_sys_ctx_t* ctx, bool on, aeMeasAreaType_t areaType)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ae_api_linExpAttr_t LineExpAttr;
    memset(&LineExpAttr, 0x00, sizeof(ae_api_linExpAttr_t));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    if (isHDRmode(ctx)) {
        ret = XCAM_RETURN_ERROR_FAILED;
        RKAIQ_IMGPROC_CHECK_RET(ret, "Not support in HDR mode!");
    }

    ret = rk_aiq_user_api2_ae_getLinExpAttr(ctx, &LineExpAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getLinExpAttr error!");
    LineExpAttr.backLightCtrl.sw_aeT_backLit_en = on ? 1 : 0;
    LineExpAttr.backLightCtrl.sw_aeT_measArea_mode = (ae_measArea_mode_t)areaType;
    LineExpAttr.backLightCtrl.sw_aeT_backLitBias_strg = 0;
    ret = rk_aiq_user_api2_ae_setLinExpAttr(ctx, LineExpAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setBLCMode error!");
    IMGPROC_FUNC_EXIT
    return ret;
}
/*
*****************************
*
* Desc: backlight compensation strength,only available in normal mode
* Argument:
*      strength:  [1,100]
*****************************
*/
XCamReturn rk_aiq_uapi2_setBLCStrength(const rk_aiq_sys_ctx_t* ctx, int strength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_linExpAttr_t LineExpAttr;
    memset(&LineExpAttr, 0x00, sizeof(ae_api_linExpAttr_t));

    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    if (isHDRmode(ctx)) {
        ret = XCAM_RETURN_ERROR_FAILED;
        RKAIQ_IMGPROC_CHECK_RET(ret, "Not support in HDR mode!");
    } else {
        ret = rk_aiq_user_api2_ae_getLinExpAttr(ctx, &LineExpAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getLinExpAttr error!");
        if (0 == LineExpAttr.backLightCtrl.sw_aeT_backLit_en)
            RKAIQ_IMGPROC_CHECK_RET(ret, "blc mode is not enabled!");
        LineExpAttr.backLightCtrl.sw_aeT_backLitBias_strg = strength;
        ret = rk_aiq_user_api2_ae_setLinExpAttr(ctx, LineExpAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setBLCStrength error!");
    }

    IMGPROC_FUNC_EXIT
    return ret;
}
/*
*****************************
*
* Desc: highlight compensation
* Argument:
*      on:  1  on
*           0  off
*****************************
*/
XCamReturn rk_aiq_uapi2_setHLCMode(const rk_aiq_sys_ctx_t* ctx, bool on)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_linExpAttr_t LinExpAttr;
    memset(&LinExpAttr, 0x00, sizeof(ae_api_linExpAttr_t));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    if (isHDRmode(ctx)) {
        ret = XCAM_RETURN_ERROR_FAILED;
        RKAIQ_IMGPROC_CHECK_RET(ret, "Not support in HDR mode!");
    } else {
        ret = rk_aiq_user_api2_ae_getLinExpAttr(ctx, &LinExpAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\n setHLCMode failed!");
        LinExpAttr.overExpCtrl.sw_aeT_overExp_en = on ? 1 : 0;
        LinExpAttr.overExpCtrl.sw_aeT_overExpBias_strg = 0;
        ret = rk_aiq_user_api2_ae_setLinExpAttr(ctx, LinExpAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\n setHLCMode failed!");
    }
    IMGPROC_FUNC_EXIT
    return ret;
}
/*
*****************************
*
* Desc: highlight compensation strength
* Argument:
*      strength:  [1,100]
*****************************
*/
XCamReturn rk_aiq_uapi2_setHLCStrength(const rk_aiq_sys_ctx_t* ctx, int strength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_linExpAttr_t LinExpAttr;
    memset(&LinExpAttr, 0x00, sizeof(ae_api_linExpAttr_t));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    if (isHDRmode(ctx)) {
        ret = XCAM_RETURN_ERROR_FAILED;
        RKAIQ_IMGPROC_CHECK_RET(ret, "Not support in HDR mode!");
    } else {
        ret = rk_aiq_user_api2_ae_getLinExpAttr(ctx, &LinExpAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getLinExpAttr error!");
        if (0 == LinExpAttr.overExpCtrl.sw_aeT_overExp_en)
            RKAIQ_IMGPROC_CHECK_RET(ret, "hlc mode is not enabled!");
        LinExpAttr.overExpCtrl.sw_aeT_overExpBias_strg = strength;
        ret = rk_aiq_user_api2_ae_setLinExpAttr(ctx, LinExpAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setHLCStrength error!");
    }
    IMGPROC_FUNC_EXIT
    return ret;
}
/*
*****************************
*
* Desc: set anti-flicker mode
* Argument:
*    mode
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setAntiFlickerEn(const rk_aiq_sys_ctx_t* ctx, bool on)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetAntiFlickerEn failed!");
    expSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_en = on;

    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\nsetAntiFlickerEn failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}
/*
*****************************
*
* Desc: set anti-flicker mode
* Argument:
*    mode
*
*****************************
*/
XCamReturn rk_aiq_uapi2_getAntiFlickerEn(const rk_aiq_sys_ctx_t* ctx, bool* on)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\ngetAntiFlickerEn!");
    *on = expSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_en;
    IMGPROC_FUNC_EXIT
    return ret;
}
/*
*****************************
*
* Desc: set anti-flicker mode
* Argument:
*    mode
*
*****************************
*/

XCamReturn rk_aiq_uapi2_setAntiFlickerMode(const rk_aiq_sys_ctx_t* ctx, antiFlickerMode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetAntiFlickerMode failed!");
    if (mode == ANTIFLICKER_AUTO_MODE) {
        expSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_en = true;
        expSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_mode = ae_antiFlicker_auto_mode;
    } else if (mode == ANTIFLICKER_NORMAL_MODE) {
        expSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_en = true;
        expSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_mode = ae_antiFlicker_normal_mode;
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "mode is invalid!");
    }
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\nsetAntiFlickerMode failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}
XCamReturn rk_aiq_uapi2_getAntiFlickerMode(const rk_aiq_sys_ctx_t* ctx, antiFlickerMode_t* mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);

    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\ngetAntiFlickerMode!");
    if (expSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_mode == ae_antiFlicker_auto_mode)
        *mode = ANTIFLICKER_AUTO_MODE;
    else if (expSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_mode == ae_antiFlicker_normal_mode)
        *mode = ANTIFLICKER_NORMAL_MODE;
    IMGPROC_FUNC_EXIT
    return ret;
}
/*
*****************************
*
* Desc: set power line frequence
* Argument:
*    freq
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setExpPwrLineFreqMode(const rk_aiq_sys_ctx_t* ctx, expPwrLineFreq_t freq)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetExpPwrLineFreqMode failed!");
    if (freq == EXP_PWR_LINE_FREQ_50HZ) {
        expSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_en = true;
        expSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_freq = ae_antiFlicker_50hz_freq;
    } else if (freq == EXP_PWR_LINE_FREQ_60HZ) {
        expSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_en = true;
        expSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_freq = ae_antiFlicker_60hz_freq;
    } else if (freq == EXP_PWR_LINE_FREQ_DIS) {
        expSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_en = false;
        expSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_freq = ae_antiFlicker_off_freq;
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "freq is invalid!");
    }
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\nsetExpPwrLineFreqMode failed!");
    IMGPROC_FUNC_EXIT
    return (ret);
}
XCamReturn rk_aiq_uapi2_getExpPwrLineFreqMode(
    const rk_aiq_sys_ctx_t* ctx,
    expPwrLineFreq_t* freq)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetExpPwrLineFreqMode failed!");
    if (expSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_freq == ae_antiFlicker_50hz_freq) {
        *freq = EXP_PWR_LINE_FREQ_50HZ;
    } else if (expSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_freq == ae_antiFlicker_60hz_freq) {
        *freq = EXP_PWR_LINE_FREQ_60HZ;
    } else if (expSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_freq == ae_antiFlicker_off_freq) {
        *freq = EXP_PWR_LINE_FREQ_DIS;
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "freq is invalid!");
    }
    IMGPROC_FUNC_EXIT
    return (ret);
}
#endif

#endif
#ifdef USE_NEWSTRUCT
/*
*****************************
*
* Desc: Adjust image gamma
*
* Argument:
*   GammaCoef: [0, 100]
*   SlopeAtZero: [-0.05, 0.05]
*****************************
*/
XCamReturn rk_aiq_uapi2_setGammaCoef(const rk_aiq_sys_ctx_t* ctx, float GammaCoef, float SlopeAtZero)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, setGammaCoef failed!");
    }

    if (GammaCoef < 0 || GammaCoef > 100) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, GammaCoef range is [0,100]!");
    }
    if (SlopeAtZero < -0.05 || SlopeAtZero > 0.05) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, SlopeAtZero range is [-0.05,0.05]!");
    }

    gamma_api_attrib_t gammaAttr;
    memset(&gammaAttr, 0x0, sizeof(gamma_api_attrib_t));

    gammaAttr.opMode                                    = RK_AIQ_OP_MODE_AUTO;
    gammaAttr.en         = true;

    float gamma_X_v11[CALIBDB_GAMMA_KNOTS_NUM_V11]   = {
        0,    1,    2,    3,    4,    5,    6,    7,    8,    10,  12,   14,   16,
        20,   24,   28,   32,   40,   48,   56,   64,   80,   96,  112,  128,  160,
        192,  224,  256,  320,  384,  448,  512,  640,  768,  896, 1024, 1280, 1536,
        1792, 2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840, 4095
    };
    float gamma_Y_v11[CALIBDB_GAMMA_KNOTS_NUM_V11];
    for (int i = 0; i < GAMMA_ISO_STEP_MAX; i++) {
        gammaAttr.stAuto.dyn[i].hw_gammaT_outCurve_offset = 0;
        for (int j = 0; j < CALIBDB_GAMMA_KNOTS_NUM_V11; j++) {
            gamma_Y_v11[j] = 4095 * pow(gamma_X_v11[j] / 4095, 1 / GammaCoef + SlopeAtZero);
            gamma_Y_v11[j] = gamma_Y_v11[j] > 4095 ? 4095 : gamma_Y_v11[j] < 0 ? 0 : gamma_Y_v11[j];
            gammaAttr.stAuto.dyn[i].hw_gammaT_outCurve_val[j] = (int)(gamma_Y_v11[j] + 0.5);
        }
    }
    ret = rk_aiq_user_api2_gamma_SetAttrib(ctx, &gammaAttr);

    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set manual dehaze module enable
*     enable/disable dehaze module function, including dehaze, enhance and hist
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setDehazeModuleEnable(const rk_aiq_sys_ctx_t* ctx, bool on) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
#if RKAIQ_HAVE_DEHAZE_V14
    dehaze_api_attrib_t attr;
    memset(&attr, 0, sizeof(dehaze_api_attrib_t));
    ret = rk_aiq_user_api2_dehaze_GetAttrib(ctx, &attr);

    if (on)
        attr.en = true;
    else
        attr.en = false;
    ret = rk_aiq_user_api2_dehaze_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDehazeModuleEnable failed!");
#else
    LOGE("not support to call %s for current chip", __FUNCTION__);
    ret = XCAM_RETURN_ERROR_UNKNOWN;
#endif
    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set manual dehaze enable
*     enable/disable dehaze function
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setDehazeEnable(const rk_aiq_sys_ctx_t* ctx, bool on) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
#if RKAIQ_HAVE_DEHAZE_V14
    dehaze_api_attrib_t attr;
    memset(&attr, 0, sizeof(dehaze_api_attrib_t));
    ret = rk_aiq_user_api2_dehaze_GetAttrib(ctx, &attr);

    if (attr.opMode == RK_AIQ_OP_MODE_AUTO) {
        if (on) {
            for (int i = 0; i < DEHAZE_ISO_STEP_MAX; i++)
                attr.stAuto.dyn[i].sw_dhazT_work_mode = dhaz_dehaze_mode;
        } else {
            for (int i = 0; i < DEHAZE_ISO_STEP_MAX; i++)
                attr.stAuto.dyn[i].sw_dhazT_work_mode = dhaz_enhance_mode;
        }
    } else if (attr.opMode == RK_AIQ_OP_MODE_MANUAL) {
        LOGW_ADEHAZE(
            "%s is only supported in AUTO mode, and you can set sw_dhazT_work_mode as "
            "dhaz_dehaze_mode in MANUAL mode.",
            __FUNCTION__);
    }
    ret = rk_aiq_user_api2_dehaze_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDehazeEnable failed!");
#elif RKAIQ_HAVE_HISTEQ_V10

    histeq_api_attrib_t attr;
    memset(&attr, 0, sizeof(histeq_api_attrib_t));
    ret = rk_aiq_user_api2_histeq_GetAttrib(ctx, &attr);

    if (!attr.en || attr.opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGK_AHISTEQ("%s: histeq status en %d opMode %d force histeq enable and switch to auto",
                __func__, attr.en, attr.opMode);
    }
    attr.en     = true;
    attr.opMode = RK_AIQ_OP_MODE_AUTO;

    ret = rk_aiq_user_api2_histeq_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDehazeEnable failed!");

    float strg;
    bool  en;
    rk_aiq_user_api2_histeq_GetUsrCfgStrg(ctx, &en, &strg);

    en = on;

    ret = rk_aiq_user_api2_histeq_SetUsrCfgStrg(ctx, en, strg);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDehazeEnable failed!");
#else
    LOGE("not support to call %s for current chip", __FUNCTION__);
    ret = XCAM_RETURN_ERROR_UNKNOWN;
#endif
    IMGPROC_FUNC_EXIT
    return ret;
}
/*
*****************************
*
* Desc: set/get manual dehaze strength
*     This function is active for dehaze is manual mode.
*     Before setting this function, you should ensure sw_dhazT_work_mode == dhaz_dehaze_mode.
*     If sw_dhazT_work_mode != dhaz_dehaze_mode, plese use rk_aiq_uapi2_setDehazeEnable in advance.
* Argument:
*   level: [0, 100]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setMDehazeStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    if (level > 100) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, strength range is [0,100]!");
    }
#if RKAIQ_HAVE_DEHAZE_V14
    dehaze_api_attrib_t attr;
    memset(&attr, 0, sizeof(dehaze_api_attrib_t));
    ret = rk_aiq_user_api2_dehaze_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed!");
    if (attr.opMode == RK_AIQ_OP_MODE_MANUAL) {
        LOGW_ADEHAZE("%s is only supported in AUTO mode.", __FUNCTION__);
    }
    adehaze_strength_t ctrl;
    memset(&ctrl, 0, sizeof(adehaze_strength_t));
    ret = rk_aiq_user_api2_getDehazeEnhanceStrth(ctx, &ctrl);
    ctrl.MDehazeStrth = level;
    ret = rk_aiq_user_api2_setDehazeEnhanceStrth(ctx, ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMDhzStrth failed!");
#elif RKAIQ_HAVE_HISTEQ_V10
    float strg;
    bool  en;
    rk_aiq_user_api2_histeq_GetUsrCfgStrg(ctx, &en, &strg);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed!");

    if (!en) {
        LOGE("please enable dehaze by rk_aiq_uapi2_setDehazeEnable");
    }
    strg = level / 100.0f;

    ret = rk_aiq_user_api2_histeq_SetUsrCfgStrg(ctx, en, strg);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMDhzStrth failed!");
#else
    LOGE("not support to call %s for current chip", __FUNCTION__);
    ret = XCAM_RETURN_ERROR_UNKNOWN;
#endif

    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getMDehazeStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
#if RKAIQ_HAVE_DEHAZE_V14
    adehaze_strength_t ctrl;
    memset(&ctrl, 0, sizeof(adehaze_strength_t));
    ret = rk_aiq_user_api2_getDehazeEnhanceStrth(ctx, &ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed in get attrib!");
    *level = ctrl.MDehazeStrth;
#elif RKAIQ_HAVE_HISTEQ_V10
    float strg;
    bool  en;
    rk_aiq_user_api2_histeq_GetUsrCfgStrg(ctx, &en, &strg);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed!");

    *level = (unsigned int)(100 * strg);

#else
    LOGE("not support to call %s for current chip", __FUNCTION__);
    ret = XCAM_RETURN_ERROR_UNKNOWN;
#endif

    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set manual enhance enable
*     enable/disable enhance function
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setEnhanceEnable(const rk_aiq_sys_ctx_t* ctx, bool on) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
#if RKAIQ_HAVE_DEHAZE_V14
    dehaze_api_attrib_t attr;
    memset(&attr, 0, sizeof(dehaze_api_attrib_t));
    ret = rk_aiq_user_api2_dehaze_GetAttrib(ctx, &attr);

    if (attr.opMode == RK_AIQ_OP_MODE_AUTO) {
        if (on) {
            for (int i = 0; i < DEHAZE_ISO_STEP_MAX; i++)
                attr.stAuto.dyn[i].sw_dhazT_work_mode = dhaz_enhance_mode;
        } else {
            for (int i = 0; i < DEHAZE_ISO_STEP_MAX; i++)
                attr.stAuto.dyn[i].sw_dhazT_work_mode = dhaz_dehaze_mode;
        }
    } else if (attr.opMode == RK_AIQ_OP_MODE_MANUAL) {
        LOGW_ADEHAZE(
            "%s is only supported in AUTO mode, and you can set sw_dhazT_work_mode as "
            "dhaz_enhance_mode in MANUAL mode.",
            __FUNCTION__);
    }
    ret = rk_aiq_user_api2_dehaze_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setEnhanceEnable failed!");
#elif RKAIQ_HAVE_ENHANCE_V10
    enh_api_attrib_t attr;
    memset(&attr, 0, sizeof(enh_api_attrib_t));
    ret = rk_aiq_user_api2_enh_GetAttrib(ctx, &attr);
    attr.en = on;
    ret = rk_aiq_user_api2_enh_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setEnhanceEnable failed!");
#else
    LOGE("not support to call %s for current chip", __FUNCTION__);
    ret = XCAM_RETURN_ERROR_UNKNOWN;
#endif
    IMGPROC_FUNC_EXIT
    return ret;
}
/*
*****************************
*
* Desc: set/get manual enhance strength
*     This function is active for enhance auto mode.
*     Before setting this function, you should ensure sw_dhazT_work_mode == dhaz_enhance_mode.
*     If sw_dhazT_work_mode != dhaz_enhance_mode, plese use rk_aiq_uapi2_setEnhanceEnable in
*advance. Argument: level: [0, 100]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setMEnhanceStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    if (level > 100) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, level range is [0,100]!");
    }
#if RKAIQ_HAVE_DEHAZE_V14
    dehaze_api_attrib_t attr;
    memset(&attr, 0, sizeof(dehaze_api_attrib_t));
    ret = rk_aiq_user_api2_dehaze_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed!");
    if (attr.opMode == RK_AIQ_OP_MODE_MANUAL) {
        LOGW_ADEHAZE("%s is only supported in AUTO mode.", __FUNCTION__);
    }
    adehaze_strength_t ctrl;
    memset(&ctrl, 0, sizeof(adehaze_strength_t));
    ret = rk_aiq_user_api2_getDehazeEnhanceStrth(ctx, &ctrl);
    ctrl.MEnhanceStrth = level;
    ret = rk_aiq_user_api2_setDehazeEnhanceStrth(ctx, ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMEnhanceStrth failed!");
#elif RKAIQ_HAVE_ENHANCE_V10
    enh_api_attrib_t attr;
    memset(&attr, 0, sizeof(enh_api_attrib_t));
    ret = rk_aiq_user_api2_enh_GetAttrib(ctx, &attr);
    if (attr.opMode == RK_AIQ_OP_MODE_MANUAL) {
        LOGW_ADEHAZE("%s is only supported in AUTO mode.", __FUNCTION__);
    }
    aenh_strength_t strg;
    memset(&strg, 0, sizeof(aenh_strength_t));
    ret = rk_aiq_user_api2_enh_GetEnhanceStrth(ctx, &strg);
    strg.MEnhanceStrth = level;
    ret = rk_aiq_user_api2_enh_SetEnhanceStrth(ctx, &strg);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMEnhanceStrth failed!");
#else
    LOGE("not support to call %s for current chip", __FUNCTION__);
    ret = XCAM_RETURN_ERROR_UNKNOWN;
#endif

    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getMEnhanceStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
#if RKAIQ_HAVE_DEHAZE_V14
    adehaze_strength_t ctrl;
    memset(&ctrl, 0, sizeof(adehaze_strength_t));
    ret = rk_aiq_user_api2_getDehazeEnhanceStrth(ctx, &ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMEnhanceStrth failed in get attrib!");
    *level = ctrl.MEnhanceStrth;
#elif RKAIQ_HAVE_ENHANCE_V10
    aenh_strength_t strg;
    memset(&strg, 0, sizeof(aenh_strength_t));
    ret = rk_aiq_user_api2_enh_GetEnhanceStrth(ctx, &strg);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMEnhanceStrth failed!");
    *level = strg.MEnhanceStrth;
#else
    LOGE("not support to call %s for current chip", __FUNCTION__);
    ret = XCAM_RETURN_ERROR_UNKNOWN;
#endif
    IMGPROC_FUNC_EXIT
    return ret;
}
/*
*****************************
*
* Desc: set/get manual enhance chrome strength
*     This function is active for enhance auto mode.
*     Before setting this function, you should ensure sw_dhazT_work_mode == dhaz_enhance_mode.
*     If sw_dhazT_work_mode != dhaz_enhance_mode, plese use rk_aiq_uapi2_setEnhanceEnable in
*advance. Argument: level: [0, 100]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setMEnhanceChromeStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int level) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    if (level > 100) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, level range is [0,100]!");
    }
#if RKAIQ_HAVE_DEHAZE_V14
    dehaze_api_attrib_t attr;
    memset(&attr, 0, sizeof(dehaze_api_attrib_t));
    ret = rk_aiq_user_api2_dehaze_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed!");
    if (attr.opMode == RK_AIQ_OP_MODE_MANUAL) {
        LOGW_ADEHAZE(
            "%s is only supported in AUTO mode, and you can set sw_dhazT_work_mode as "
            "dhaz_enhance_mode in MANUAL mode.",
            __FUNCTION__);
    }
    adehaze_strength_t ctrl;
    memset(&ctrl, 0, sizeof(adehaze_strength_t));
    ret = rk_aiq_user_api2_getDehazeEnhanceStrth(ctx, &ctrl);
    ctrl.MEnhanceChromeStrth = level;
    ret = rk_aiq_user_api2_setDehazeEnhanceStrth(ctx, ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMEnhanceChromeStrth failed!");
#elif RKAIQ_HAVE_ENHANCE_V10
    enh_api_attrib_t attr;
    memset(&attr, 0, sizeof(enh_api_attrib_t));
    ret = rk_aiq_user_api2_enh_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed!");
    if (attr.opMode == RK_AIQ_OP_MODE_MANUAL) {
        LOGW_ADEHAZE("%s is only supported in AUTO mode",
                     __FUNCTION__);
    }

    aenh_strength_t strg;
    memset(&strg, 0, sizeof(aenh_strength_t));
    ret = rk_aiq_user_api2_enh_GetEnhanceStrth(ctx, &strg);
    strg.MEnhanceChromeStrth = level;
    ret = rk_aiq_user_api2_enh_SetEnhanceStrth(ctx, &strg);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMEnhanceChromeStrth failed!");
#else
    LOGE("not support to call %s for current chip", __FUNCTION__);
    ret = XCAM_RETURN_ERROR_UNKNOWN;
#endif

    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_getMEnhanceChromeStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int* level) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
#if RKAIQ_HAVE_DEHAZE_V14
    adehaze_strength_t ctrl;
    memset(&ctrl, 0, sizeof(adehaze_strength_t));
    ret = rk_aiq_user_api2_getDehazeEnhanceStrth(ctx, &ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMEnhanceChromeStrth failed in get attrib!");
    *level = ctrl.MEnhanceChromeStrth;
#elif RKAIQ_HAVE_ENHANCE_V10
    aenh_strength_t strg;
    memset(&strg, 0, sizeof(aenh_strength_t));
    ret = rk_aiq_user_api2_enh_GetEnhanceStrth(ctx, &strg);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMEnhanceChromeStrth failed in get attrib!");
    *level = strg.MEnhanceChromeStrth;
#else
    LOGE("not support to call %s for current chip", __FUNCTION__);
    ret = XCAM_RETURN_ERROR_UNKNOWN;
#endif
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_setDrcLocalData(const rk_aiq_sys_ctx_t* ctx, float LocalWeit, float GlobalContrast, float LoLitContrast, int LocalAutoEnable, float LocalAutoWeit)
{
    LOGE("not supported !");
    return XCAM_RETURN_ERROR_PARAM;
}

XCamReturn rk_aiq_uapi2_getDrcLocalData(const rk_aiq_sys_ctx_t* ctx, float* LocalWeit, float* GlobalContrast, float* LoLitContrast, int* LocalAutoEnable, float* LocalAutoWeit)
{
    LOGE("not supported !");
    return XCAM_RETURN_ERROR_PARAM;
}

/*
*****************************
*
* Desc: set/get manual drc Local Data
*     this function is active for DRC is Auto mode
*     use in rk3576
* Argument:
*   hw_drcT_bifiltOut_alpha: [0, 16]
*   hw_drcT_locDetail_strg: [0, 4095]
*   hw_drcT_hfDarkRegion_strg: [0, 4095]
*   hw_drcT_softThd_en: [0, 1]
*   hw_drcT_softThd_thred: [0, 2047]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setDrcLocalDataV2(const rk_aiq_sys_ctx_t* ctx,
        float hw_drcT_bifiltOut_alpha,
        float hw_drcT_locDetail_strg,
        float hw_drcT_hfDarkRegion_strg, int hw_drcT_softThd_en,
        float hw_drcT_softThd_thred) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    if (hw_drcT_bifiltOut_alpha < 0 || hw_drcT_bifiltOut_alpha > 16) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, hw_drcT_bifiltOut_alpha range is [0,16]!");
    }
    if (hw_drcT_locDetail_strg < 0 || hw_drcT_locDetail_strg > 4095) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, hw_drcT_locDetail_strg range is [0,4095]!");
    }
    if (hw_drcT_hfDarkRegion_strg < 0 || hw_drcT_hfDarkRegion_strg > 4095) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, hw_drcT_hfDarkRegion_strg range is [0,4095]!");
    }
    if (hw_drcT_softThd_en < 0 || hw_drcT_softThd_en > 1) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, hw_drcT_softThd_en range is [0,1]!");
    }
    if (hw_drcT_softThd_thred < 0 || hw_drcT_softThd_thred > 2047) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, hw_drcT_softThd_thred range is [0,2047]!");
    }

    drc_api_attrib_t attr;
    memset(&attr, 0, sizeof(drc_api_attrib_t));
    ret = rk_aiq_user_api2_drc_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcLocalData failed!");
    if (attr.opMode == RK_AIQ_OP_MODE_AUTO) {
        for (int i = 0; i < DRC_ISO_STEP_MAX; i++) {
            attr.stAuto.dyn[i].bifilt_filter.hw_drcT_bifiltOut_alpha      = hw_drcT_bifiltOut_alpha;
            attr.stAuto.dyn[i].drcProc.hw_drcT_locDetail_strg             = hw_drcT_locDetail_strg;
            attr.stAuto.dyn[i].drcProc.hw_drcT_hfDarkRegion_strg   = hw_drcT_hfDarkRegion_strg;
            attr.stAuto.dyn[i].bifilt_filter.hw_drcT_softThd_en = hw_drcT_softThd_en;
            attr.stAuto.dyn[i].bifilt_filter.hw_drcT_softThd_thred = hw_drcT_softThd_thred;
        }
    } else if (attr.opMode == RK_AIQ_OP_MODE_MANUAL) {
        attr.stMan.dyn.bifilt_filter.hw_drcT_bifiltOut_alpha       = hw_drcT_bifiltOut_alpha;
        attr.stMan.dyn.drcProc.hw_drcT_locDetail_strg              = hw_drcT_locDetail_strg;
        attr.stMan.dyn.drcProc.hw_drcT_hfDarkRegion_strg           = hw_drcT_hfDarkRegion_strg;
        attr.stMan.dyn.bifilt_filter.hw_drcT_softThd_en = hw_drcT_softThd_en;
        attr.stMan.dyn.bifilt_filter.hw_drcT_softThd_thred   = hw_drcT_softThd_thred;
    }
    ret = rk_aiq_user_api2_drc_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcLocalData failed!");

    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getDrcLocalDataV2(const rk_aiq_sys_ctx_t* ctx,
        float* hw_drcT_bifiltOut_alpha,
        float* hw_drcT_locDetail_strg,
        float* hw_drcT_hfDarkRegion_strg, int* hw_drcT_softThd_en,
        float* hw_drcT_softThd_thred) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }

    drc_status_t status;
    memset(&status, 0, sizeof(drc_status_t));
    ret = rk_aiq_user_api2_drc_QueryStatus(ctx, &status);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcLocalData failed in get attrib!");
    *hw_drcT_bifiltOut_alpha       = status.stMan.dyn.bifilt_filter.hw_drcT_bifiltOut_alpha;
    *hw_drcT_locDetail_strg        = status.stMan.dyn.drcProc.hw_drcT_locDetail_strg;
    *hw_drcT_hfDarkRegion_strg     = status.stMan.dyn.drcProc.hw_drcT_hfDarkRegion_strg;
    *hw_drcT_softThd_en = status.stMan.dyn.bifilt_filter.hw_drcT_softThd_en;
    *hw_drcT_softThd_thred   = status.stMan.dyn.bifilt_filter.hw_drcT_softThd_thred;

    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set/get manual drc HiLit
*     this function is active for DRC is Auto mode
*     use in rk3576
* Argument:
*   Strength: [0, 255]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setDrcHiLit(const rk_aiq_sys_ctx_t* ctx, float hw_drcT_midWgt_alpha)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    if (hw_drcT_midWgt_alpha < 0 || hw_drcT_midWgt_alpha > 255) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, hw_drcT_midWgt_alpha range is [0,255]!");
    }

    drc_api_attrib_t attr;
    memset(&attr, 0, sizeof(drc_api_attrib_t));
    ret = rk_aiq_user_api2_drc_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcHiLit failed!");

    if (attr.opMode == RK_AIQ_OP_MODE_AUTO) {
        for (int i = 0; i < DRC_ISO_STEP_MAX; i++)
            attr.stAuto.dyn[i].bifilt_filter.hw_drcT_midWgt_alpha = hw_drcT_midWgt_alpha;
    } else if (attr.opMode == RK_AIQ_OP_MODE_MANUAL) {
        attr.stMan.dyn.bifilt_filter.hw_drcT_midWgt_alpha = hw_drcT_midWgt_alpha;
    }
    ret = rk_aiq_user_api2_drc_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcHiLit failed!");

    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getDrcHiLit(const rk_aiq_sys_ctx_t* ctx, float * hw_drcT_midWgt_alpha)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }

    drc_status_t status;
    memset(&status, 0, sizeof(drc_status_t));
    ret = rk_aiq_user_api2_drc_QueryStatus(ctx, &status);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcHiLit failed in get attrib!");
    *hw_drcT_midWgt_alpha = status.stMan.dyn.bifilt_filter.hw_drcT_midWgt_alpha;

    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set/get manual drc sw_drcT_toneGain_maxLimit
*     this function is active for DRC is Auto mode
*     use in rk3576
* Argument:
*   sw_drcT_toneGain_maxLimit: [1, 8]
*   sw_drcT_toneCurveK_coeff: [0, 1]
*   hw_drcT_toneCurveIdx_scale: [0, 64]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setDrcGain(const rk_aiq_sys_ctx_t* ctx, float sw_drcT_toneGain_maxLimit, float sw_drcT_toneCurveK_coeff, float hw_drcT_toneCurveIdx_scale)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    if (sw_drcT_toneGain_maxLimit < 1 || sw_drcT_toneGain_maxLimit > 8) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, sw_drcT_toneGain_maxLimit range is [1,8]!");
    }
    if (sw_drcT_toneCurveK_coeff < 0 || sw_drcT_toneCurveK_coeff > 1) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, sw_drcT_toneCurveK_coeff range is [0,1]!");
    }
    if (hw_drcT_toneCurveIdx_scale < 0 || hw_drcT_toneCurveIdx_scale > 64) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, hw_drcT_toneCurveIdx_scale range is [0,64]!");
    }

    drc_api_attrib_t attr;
    memset(&attr, 0, sizeof(drc_api_attrib_t));
    ret = rk_aiq_user_api2_drc_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcGain failed!");

    if (attr.opMode == RK_AIQ_OP_MODE_AUTO) {
        for (int i = 0; i < DRC_ISO_STEP_MAX; i++) {
            attr.stAuto.dyn[i].preProc.toneCurveCtrl.sw_drcT_toneGain_maxLimit = sw_drcT_toneGain_maxLimit;
            attr.stAuto.dyn[i].preProc.toneCurveCtrl.sw_drcT_toneCurveK_coeff   = sw_drcT_toneCurveK_coeff;
            attr.stAuto.dyn[i].preProc.hw_drcT_toneCurveIdx_scale    = hw_drcT_toneCurveIdx_scale;
        }
    } else if (attr.opMode == RK_AIQ_OP_MODE_MANUAL) {
        attr.stMan.dyn.preProc.toneCurveCtrl.sw_drcT_toneGain_maxLimit = sw_drcT_toneGain_maxLimit;
        attr.stMan.dyn.preProc.toneCurveCtrl.sw_drcT_toneCurveK_coeff   = sw_drcT_toneCurveK_coeff;
        attr.stMan.dyn.preProc.hw_drcT_toneCurveIdx_scale    = hw_drcT_toneCurveIdx_scale;
    }
    ret = rk_aiq_user_api2_drc_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed!");

    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getDrcGain(const rk_aiq_sys_ctx_t* ctx, float * sw_drcT_toneGain_maxLimit, float * sw_drcT_toneCurveK_coeff, float * hw_drcT_toneCurveIdx_scale)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }

    drc_status_t status;
    memset(&status, 0, sizeof(drc_status_t));
    ret = rk_aiq_user_api2_drc_QueryStatus(ctx, &status);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed in get attrib!");
    *sw_drcT_toneGain_maxLimit  = status.stMan.dyn.preProc.toneCurveCtrl.sw_drcT_toneGain_maxLimit;
    *sw_drcT_toneCurveK_coeff = status.stMan.dyn.preProc.toneCurveCtrl.sw_drcT_toneCurveK_coeff;
    *hw_drcT_toneCurveIdx_scale  = status.stMan.dyn.preProc.hw_drcT_toneCurveIdx_scale;

    IMGPROC_FUNC_EXIT
    return ret;
}
#endif

#ifdef USE_IMPLEMENT_C
/*
*****************************
*
* Desc: set/get dark area boost strength
*    this function is active for normal mode
* Argument:
*   level: [1, 10]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_getDarkAreaBoostStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if RKAIQ_HAVE_DRC_V10
    ret = XCAM_RETURN_ERROR_PARAM;
    RKAIQ_IMGPROC_CHECK_RET(ret, "ISP2.1 do not support tmo api!");
#endif
#if RKAIQ_HAVE_DRC_V11
    ret = XCAM_RETURN_ERROR_PARAM;
    RKAIQ_IMGPROC_CHECK_RET(ret, "ISP3.0 do not support tmo api!");
#endif
#if RKAIQ_HAVE_DRC_V12
    ret = XCAM_RETURN_ERROR_PARAM;
    RKAIQ_IMGPROC_CHECK_RET(ret, "ISP3.2 do not support tmo api!");
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    ret = XCAM_RETURN_ERROR_PARAM;
    RKAIQ_IMGPROC_CHECK_RET(ret, "ISP3.2 lite do not support tmo api!");
#endif
#if RKAIQ_HAVE_DRC_V20
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }

    adrc_strength_t ctrl;
    memset(&ctrl, 0, sizeof(adrc_strength_t));
    ret = rk_aiq_user_api2_drc_GetStrength(ctx, &ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDarkAreaBoostStrth GetStrength failed!");

    *level = ctrl.darkAreaBoostStrength;

    IMGPROC_FUNC_EXIT
#endif

    return ret;
}

XCamReturn rk_aiq_uapi2_setDarkAreaBoostStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER

#if RKAIQ_HAVE_DRC_V10
    ret = XCAM_RETURN_ERROR_PARAM;
    RKAIQ_IMGPROC_CHECK_RET(ret, "ISP2.1 do not support tmo api!");
#endif
#if RKAIQ_HAVE_DRC_V11
    ret = XCAM_RETURN_ERROR_PARAM;
    RKAIQ_IMGPROC_CHECK_RET(ret, "ISP3.0 do not support tmo api!");
#endif
#if RKAIQ_HAVE_DRC_V12
    ret = XCAM_RETURN_ERROR_PARAM;
    RKAIQ_IMGPROC_CHECK_RET(ret, "ISP3.2 do not support tmo api!");
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    ret = XCAM_RETURN_ERROR_PARAM;
    RKAIQ_IMGPROC_CHECK_RET(ret, "ISP3.2 lite do not support tmo api!");
#endif
#if RKAIQ_HAVE_DRC_V20
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }

    if (level > 100) {
        LOGE("params error, level need in range [0, 100]");
        return XCAM_RETURN_ERROR_PARAM;
    }

    adrc_strength_t ctrl;
    memset(&ctrl, 0, sizeof(adrc_strength_t));
    ret = rk_aiq_user_api2_drc_GetStrength(ctx, &ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDarkAreaBoostStrth GetStrength failed!");

    ctrl.darkAreaBoostEn       = true;
    ctrl.darkAreaBoostStrength = level;

    ret = rk_aiq_user_api2_drc_SetStrength(ctx, ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDarkAreaBoostStrth SetStrength failed!");

#endif

    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set/get manual hdr strength
*    this function is active for HDR is manual mode
* Argument:
*   level: [1, 100]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setMHDRStrth(const rk_aiq_sys_ctx_t* ctx, bool on, unsigned int level)
{
    /*
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
        if(CHECK_ISP_HW_V20()) {
            atmo_attrib_t attr;
            memset(&attr, 0, sizeof(attr));
            IMGPROC_FUNC_ENTER
            if (ctx == NULL) {
                ret = XCAM_RETURN_ERROR_PARAM;
                RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, setMHDRStrth failed!");
            }

            if (!isHDRmode(ctx)) {
                ret = XCAM_RETURN_ERROR_FAILED;
                RKAIQ_IMGPROC_CHECK_RET(ret, "not in HDR mode!");
            }
            if (level < 1 || level > 100) {
                ret = XCAM_RETURN_ERROR_OUTOFRANGE;
                RKAIQ_IMGPROC_CHECK_RET(ret, "level(%d) is out of range, setMHDRStrth failed!");
            }

            attr.stSetLevel.level = level;
            attr.opMode = TMO_OPMODE_SET_LEVEL;
            ret = rk_aiq_user_api2_atmo_SetAttrib(ctx, attr);
            RKAIQ_IMGPROC_CHECK_RET(ret, "setMHDRStrth failed!");
            IMGPROC_FUNC_EXIT
        }
        else if(CHECK_ISP_HW_V21()) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "ISP2.1 do not support tmo api, ctx is NULL!");
        }
        else if(CHECK_ISP_HW_V30()) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "ISP3.0 do not support tmo api!");
        }
    */
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_uapi2_getMHDRStrth(const rk_aiq_sys_ctx_t* ctx, bool * on, unsigned int *level)
{
    /*
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
        if(CHECK_ISP_HW_V20()) {
            atmo_attrib_t attr;
            memset(&attr, 0, sizeof(attr));
            IMGPROC_FUNC_ENTER
            if (ctx == NULL) {
                ret = XCAM_RETURN_ERROR_PARAM;
                RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, getMHDRStrth failed!");
            }
            if (!isHDRmode(ctx)) {
                ret = XCAM_RETURN_ERROR_FAILED;
                RKAIQ_IMGPROC_CHECK_RET(ret, "not in HDR mode!");
            }
            ret = rk_aiq_user_api2_atmo_GetAttrib(ctx, &attr);
            RKAIQ_IMGPROC_CHECK_RET(ret, "getMHDRStrth failed in get attrib!");

            *level = attr.stSetLevel.level;
            IMGPROC_FUNC_EXIT

        }
        else if(CHECK_ISP_HW_V21()) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "ISP2.1 do not support tmo api, ctx is NULL!");
        }
        else if(CHECK_ISP_HW_V30()) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "ISP3.0 do not support tmo api!");
        }
    */
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

/*
*****************************
*
* Desc: set/get hdr strength
*    this function is active for HDR is manual mode
* Argument:
*   level: [0, 100]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setHDRStrth(const rk_aiq_sys_ctx_t* ctx, bool on, unsigned int level)
{

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }

    if ((level > 100)) {
        LOGE("params error, level need in range [0, 100]");
        return XCAM_RETURN_ERROR_PARAM;
    }

    adrc_strength_t ctrl;
    memset(&ctrl, 0, sizeof(adrc_strength_t));
    ret = rk_aiq_user_api2_drc_GetStrength(ctx, &ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setHDRStrth GetStrength failed!");

    ctrl.hdrStrengthEn = true;
    ctrl.hdrStrength   = level;

    ret = rk_aiq_user_api2_drc_SetStrength(ctx, ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setHDRStrth SetStrength failed!");

    IMGPROC_FUNC_EXIT
    return ret;

}
XCamReturn rk_aiq_uapi2_getHDRStrth(const rk_aiq_sys_ctx_t* ctx, bool *on, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }

    adrc_strength_t ctrl;
    memset(&ctrl, 0, sizeof(adrc_strength_t));
    ret = rk_aiq_user_api2_drc_GetStrength(ctx, &ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getHDRStrth GetStrength failed!");

    *level = ctrl.hdrStrength;
    *on    = ctrl.hdrStrengthEn;

    IMGPROC_FUNC_EXIT
    return ret;
}

/*
**********************************************************
* Noise reduction
**********************************************************
*/
/*
*****************************
*
* Desc: set noise reduction mode
* Argument:
*   mode:
*     auto: auto noise reduction
*     manualï¼manual noise reduction
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setNRMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER

    /*
        if (CHECK_ISP_HW_V20()) {
            rk_aiq_nr_attrib_t attr;
            ret = rk_aiq_user_api2_anr_GetAttrib(ctx, &attr);
            RKAIQ_IMGPROC_CHECK_RET(ret, "get anr attrib failed!,ret=%d", ret);
            if (mode == OP_AUTO) {
                attr.eMode = ANR_OP_MODE_AUTO;
            } else if (mode == OP_MANUAL) {
                attr.eMode = ANR_OP_MODE_MANUAL;
            } else {
                ret = XCAM_RETURN_ERROR_PARAM;
                RKAIQ_IMGPROC_CHECK_RET(ret, "Not supported mode!");
            }
            ret = rk_aiq_user_api2_anr_SetAttrib(ctx, &attr);
        }

        if (CHECK_ISP_HW_V21()) {
            rk_aiq_ynr_attrib_v2_t ynrV2_attr;
            rk_aiq_bayernr_attrib_v2_t bayernrV2_attr;
            rk_aiq_cnr_attrib_v1_t cnrV1_attr;
            ret = rk_aiq_user_api2_aynrV2_GetAttrib(ctx, &ynrV2_attr);
            ret = rk_aiq_user_api2_abayernrV2_GetAttrib(ctx, &bayernrV2_attr);
            ret = rk_aiq_user_api2_acnrV1_GetAttrib(ctx, &cnrV1_attr);
            RKAIQ_IMGPROC_CHECK_RET(ret, "get anr attrib failed!,ret=%d", ret);
            if (mode == OP_AUTO) {
                bayernrV2_attr.eMode = ABAYERNR_OP_MODE_AUTO;
                ynrV2_attr.eMode = AYNR_OP_MODE_AUTO;
                cnrV1_attr.eMode = ACNR_OP_MODE_AUTO;
            } else if (mode == OP_MANUAL) {
                bayernrV2_attr.eMode = ABAYERNR_OP_MODE_MANUAL;
                ynrV2_attr.eMode = AYNR_OP_MODE_MANUAL;
                cnrV1_attr.eMode = ACNR_OP_MODE_MANUAL;
            } else {
                ret = XCAM_RETURN_ERROR_PARAM;
                RKAIQ_IMGPROC_CHECK_RET(ret, "Not supported mode!");
            }
            ret = rk_aiq_user_api2_aynrV2_SetAttrib(ctx, &ynrV2_attr);
            ret = rk_aiq_user_api2_abayernrV2_SetAttrib(ctx, &bayernrV2_attr);
            ret = rk_aiq_user_api2_acnrV1_SetAttrib(ctx, &cnrV1_attr);
        }

        if (CHECK_ISP_HW_V30()) {
            rk_aiq_ynr_attrib_v3_t ynrV3_attr;
            ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_cnr_attrib_v2_t cnrV2_attr;
            cnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_bayer2dnr_attrib_v2_t bayer2dnrV2_attr;
            bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_bayertnr_attrib_v2_t bayertnrV2_attr;
            bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &ynrV3_attr);
            ret = rk_aiq_user_api2_acnrV2_GetAttrib(ctx, &cnrV2_attr);
            ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &bayer2dnrV2_attr);
            ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &bayertnrV2_attr);
            RKAIQ_IMGPROC_CHECK_RET(ret, "get anr attrib failed!,ret=%d", ret);

            if (mode == OP_AUTO) {
                ynrV3_attr.eMode = AYNRV3_OP_MODE_AUTO;
                cnrV2_attr.eMode = ACNRV2_OP_MODE_AUTO;
                bayer2dnrV2_attr.eMode = ABAYER2DNR_OP_MODE_AUTO;
                bayertnrV2_attr.eMode = ABAYERTNRV2_OP_MODE_AUTO;
            } else if (mode == OP_MANUAL) {
                ynrV3_attr.eMode = AYNRV3_OP_MODE_MANUAL;
                cnrV2_attr.eMode = ACNRV2_OP_MODE_MANUAL;
                bayer2dnrV2_attr.eMode = ABAYER2DNR_OP_MODE_MANUAL;
                bayertnrV2_attr.eMode = ABAYERTNRV2_OP_MODE_MANUAL;
            } else if(mode == OP_REG_MANUAL) {
                ynrV3_attr.eMode = AYNRV3_OP_MODE_REG_MANUAL;
                cnrV2_attr.eMode = ACNRV2_OP_MODE_REG_MANUAL;
                bayer2dnrV2_attr.eMode = ABAYER2DNR_OP_MODE_REG_MANUAL;
                bayertnrV2_attr.eMode = ABAYERTNRV2_OP_MODE_REG_MANUAL;
            } else {
                ret = XCAM_RETURN_ERROR_PARAM;
                RKAIQ_IMGPROC_CHECK_RET(ret, "Not supported mode!");
            }
            ret = rk_aiq_user_api2_aynrV3_SetAttrib(ctx, &ynrV3_attr);
            ret = rk_aiq_user_api2_acnrV2_SetAttrib(ctx, &cnrV2_attr);
            ret = rk_aiq_user_api2_abayer2dnrV2_SetAttrib(ctx, &bayer2dnrV2_attr);
            ret = rk_aiq_user_api2_abayertnrV2_SetAttrib(ctx, &bayertnrV2_attr);
        }

        if ( CHECK_ISP_HW_V32()) {
            rk_aiq_ynr_attrib_v22_t ynrV22_attr;
            ynrV22_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_cnr_attrib_v30_t cnrV30_attr;
            cnrV30_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_bayer2dnr_attrib_v23_t bayer2dnrV23_attr;
            bayer2dnrV23_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_bayertnr_attrib_v23_t bayertnrV23_attr;
            bayertnrV23_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_aynrV22_GetAttrib(ctx, &ynrV22_attr);
            ret = rk_aiq_user_api2_acnrV30_GetAttrib(ctx, &cnrV30_attr);
            ret = rk_aiq_user_api2_abayer2dnrV23_GetAttrib(ctx, &bayer2dnrV23_attr);
            ret = rk_aiq_user_api2_abayertnrV23_GetAttrib(ctx, &bayertnrV23_attr);
            RKAIQ_IMGPROC_CHECK_RET(ret, "get anr attrib failed!,ret=%d", ret);

            if (mode == OP_AUTO) {
                ynrV22_attr.eMode = AYNRV22_OP_MODE_AUTO;
                cnrV30_attr.eMode = ACNRV30_OP_MODE_AUTO;
                bayer2dnrV23_attr.eMode = ABAYER2DNR_V23_OP_MODE_AUTO;
                bayertnrV23_attr.eMode = ABAYERTNRV23_OP_MODE_AUTO;
            } else if (mode == OP_MANUAL) {
                ynrV22_attr.eMode = AYNRV22_OP_MODE_MANUAL;
                cnrV30_attr.eMode = ACNRV30_OP_MODE_MANUAL;
                bayer2dnrV23_attr.eMode = ABAYER2DNR_V23_OP_MODE_MANUAL;
                bayertnrV23_attr.eMode = ABAYERTNRV23_OP_MODE_MANUAL;
            } else if(mode == OP_REG_MANUAL) {
                ynrV22_attr.eMode = AYNRV22_OP_MODE_REG_MANUAL;
                cnrV30_attr.eMode = ACNRV30_OP_MODE_REG_MANUAL;
                bayer2dnrV23_attr.eMode = ABAYER2DNR_V23_OP_MODE_REG_MANUAL;
                bayertnrV23_attr.eMode = ABAYERTNRV23_OP_MODE_REG_MANUAL;
            } else {
                ret = XCAM_RETURN_ERROR_PARAM;
                RKAIQ_IMGPROC_CHECK_RET(ret, "Not supported mode!");
            }
            ret = rk_aiq_user_api2_aynrV22_SetAttrib(ctx, &ynrV22_attr);
            ret = rk_aiq_user_api2_acnrV30_SetAttrib(ctx, &cnrV30_attr);
            ret = rk_aiq_user_api2_abayer2dnrV23_SetAttrib(ctx, &bayer2dnrV23_attr);
            ret = rk_aiq_user_api2_abayertnrV23_SetAttrib(ctx, &bayertnrV23_attr);
        }

        if (CHECK_ISP_HW_V32_LITE()) {
            rk_aiq_ynr_attrib_v22_t ynrV22_attr;
            ynrV22_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_cnr_attrib_v30_t cnrV30_attr;
            cnrV30_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_bayertnr_attrib_v23L_t bayertnrV23_attr;
            bayertnrV23_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret                             = rk_aiq_user_api2_aynrV22_GetAttrib(ctx, &ynrV22_attr);
            ret                             = rk_aiq_user_api2_acnrV30_GetAttrib(ctx, &cnrV30_attr);
            ret = rk_aiq_user_api2_abayertnrV23Lite_GetAttrib(ctx, &bayertnrV23_attr);
            RKAIQ_IMGPROC_CHECK_RET(ret, "get anr attrib failed!,ret=%d", ret);

            if (mode == OP_AUTO) {
                ynrV22_attr.eMode      = AYNRV22_OP_MODE_AUTO;
                cnrV30_attr.eMode      = ACNRV30_OP_MODE_AUTO;
                bayertnrV23_attr.eMode = ABAYERTNRV23_OP_MODE_AUTO;
            } else if (mode == OP_MANUAL) {
                ynrV22_attr.eMode      = AYNRV22_OP_MODE_MANUAL;
                cnrV30_attr.eMode      = ACNRV30_OP_MODE_MANUAL;
                bayertnrV23_attr.eMode = ABAYERTNRV23_OP_MODE_MANUAL;
            } else if (mode == OP_REG_MANUAL) {
                ynrV22_attr.eMode      = AYNRV22_OP_MODE_REG_MANUAL;
                cnrV30_attr.eMode      = ACNRV30_OP_MODE_REG_MANUAL;
                bayertnrV23_attr.eMode = ABAYERTNRV23_OP_MODE_REG_MANUAL;
            } else {
                ret = XCAM_RETURN_ERROR_PARAM;
                RKAIQ_IMGPROC_CHECK_RET(ret, "Not supported mode!");
            }
            ret = rk_aiq_user_api2_aynrV22_SetAttrib(ctx, &ynrV22_attr);
            ret = rk_aiq_user_api2_acnrV30_SetAttrib(ctx, &cnrV30_attr);
            ret = rk_aiq_user_api2_abayertnrV23Lite_SetAttrib(ctx, &bayertnrV23_attr);
        }

        if (CHECK_ISP_HW_V39() ) {
            rk_aiq_ynr_attrib_v24_t ynrV24_attr;
            ynrV24_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_cnr_attrib_v31_t cnrV31_attr;
            cnrV31_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_bayertnr_attrib_v30_t bayertnrV30_attr;
            bayertnrV30_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_aynrV24_GetAttrib(ctx, &ynrV24_attr);
            ret = rk_aiq_user_api2_acnrV31_GetAttrib(ctx, &cnrV31_attr);
            ret = rk_aiq_user_api2_abayertnrV30_GetAttrib(ctx, &bayertnrV30_attr);
            RKAIQ_IMGPROC_CHECK_RET(ret, "get anr attrib failed!,ret=%d", ret);

            if (mode == OP_AUTO) {
                ynrV24_attr.eMode = AYNRV24_OP_MODE_AUTO;
                cnrV31_attr.eMode = ACNRV31_OP_MODE_AUTO;
                bayertnrV30_attr.eMode = ABAYERTNRV30_OP_MODE_AUTO;
            } else if (mode == OP_MANUAL) {
                ynrV24_attr.eMode = AYNRV24_OP_MODE_MANUAL;
                cnrV31_attr.eMode = ACNRV31_OP_MODE_MANUAL;
                bayertnrV30_attr.eMode = ABAYERTNRV30_OP_MODE_MANUAL;
            } else if(mode == OP_REG_MANUAL) {
                ynrV24_attr.eMode = AYNRV24_OP_MODE_REG_MANUAL;
                cnrV31_attr.eMode = ACNRV31_OP_MODE_REG_MANUAL;
                bayertnrV30_attr.eMode = ABAYERTNRV30_OP_MODE_REG_MANUAL;
            } else {
                ret = XCAM_RETURN_ERROR_PARAM;
                RKAIQ_IMGPROC_CHECK_RET(ret, "Not supported mode!");
            }
            ret = rk_aiq_user_api2_aynrV24_SetAttrib(ctx, &ynrV24_attr);
            ret = rk_aiq_user_api2_acnrV31_SetAttrib(ctx, &cnrV31_attr);
            ret = rk_aiq_user_api2_abayertnrV30_SetAttrib(ctx, &bayertnrV30_attr);
        }
    */

    if (CHECK_ISP_HW_V39()) {
        // ynr_api_attrib_t  ynr_attr;
        // cnr_api_attrib_t  cnr_attr;
        // btnr_api_attrib_t btnr_attr;

        // ret = rk_aiq_user_api2_ynr_GetAttrib(ctx, &ynr_attr);
        // ret = rk_aiq_user_api2_cnr_GetAttrib(ctx, &cnr_attr);
        // ret = rk_aiq_user_api2_btnr_GetAttrib(ctx, &btnr_attr);

        // if (mode == OP_AUTO) {
        //     ynr_attr.opMode = RK_AIQ_OP_MODE_AUTO;
        //     cnr_attr.opMode = RK_AIQ_OP_MODE_AUTO;
        //     btnr_attr.opMode = RK_AIQ_OP_MODE_AUTO;
        // } else if (mode == OP_MANUAL) {
        //     ynr_attr.opMode = RK_AIQ_OP_MODE_MANUAL;
        //     cnr_attr.opMode = RK_AIQ_OP_MODE_MANUAL;
        //     btnr_attr.opMode = RK_AIQ_OP_MODE_MANUAL;
        // } else {
        //     ret = XCAM_RETURN_ERROR_PARAM;
        //     RKAIQ_IMGPROC_CHECK_RET(ret, "Not supported mode!");
        // }

        // ret = rk_aiq_user_api2_ynr_SetAttrib(ctx, &ynr_attr);
        // ret = rk_aiq_user_api2_cnr_SetAttrib(ctx, &cnr_attr);
        // ret = rk_aiq_user_api2_btnr_SetAttrib(ctx, &btnr_attr);

        LOGE("not support to call %s for current chip", __FUNCTION__);
        ret = XCAM_RETURN_ERROR_UNKNOWN;
    }

    RKAIQ_IMGPROC_CHECK_RET(ret, "setNRMode failed!", ret);
    IMGPROC_FUNC_EXIT

    return ret;
}


XCamReturn rk_aiq_uapi2_getNRMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER

    /*
        if (CHECK_ISP_HW_V20()) {
            rk_aiq_nr_attrib_t attr;
            ret = rk_aiq_user_api2_anr_GetAttrib(ctx, &attr);
            RKAIQ_IMGPROC_CHECK_RET(ret, "get anr attrib failed!,ret=%d", ret);
            if (attr.eMode == ANR_OP_MODE_AUTO) {
                *mode = OP_AUTO;
            } else if (attr.eMode == ANR_OP_MODE_MANUAL) {
                *mode = OP_MANUAL;
            } else if (attr.eMode == ANR_OP_MODE_INVALID) {
                *mode = OP_INVAL;
            }
        }

        if (CHECK_ISP_HW_V21()) {
            rk_aiq_ynr_attrib_v2_t ynrV2_attr;
            rk_aiq_bayernr_attrib_v2_t bayernrV2_attr;
            rk_aiq_cnr_attrib_v1_t cnrV1_attr;
            ret = rk_aiq_user_api2_aynrV2_GetAttrib(ctx, &ynrV2_attr);
            ret = rk_aiq_user_api2_abayernrV2_GetAttrib(ctx, &bayernrV2_attr);
            ret = rk_aiq_user_api2_acnrV1_GetAttrib(ctx, &cnrV1_attr);
            RKAIQ_IMGPROC_CHECK_RET(ret, "get anr attrib failed!,ret=%d", ret);

            if (bayernrV2_attr.eMode == ABAYERNR_OP_MODE_AUTO
                    && ynrV2_attr.eMode == AYNR_OP_MODE_AUTO
                    && cnrV1_attr.eMode == ACNR_OP_MODE_AUTO) {
                *mode = OP_AUTO;
            } else if (bayernrV2_attr.eMode == ABAYERNR_OP_MODE_MANUAL
                       && ynrV2_attr.eMode == AYNR_OP_MODE_MANUAL
                       && cnrV1_attr.eMode == ACNR_OP_MODE_MANUAL) {
                *mode = OP_MANUAL;
            } else if (bayernrV2_attr.eMode == ABAYERNR_OP_MODE_INVALID
                       && ynrV2_attr.eMode == AYNR_OP_MODE_INVALID
                       && cnrV1_attr.eMode == ACNR_OP_MODE_INVALID) {
                *mode = OP_INVAL;
            } else {
                LOGE_ANR("bayernr.mode:%d  ynr.mode:%d cnr.mode:%d\n",
                         bayernrV2_attr.eMode,
                         ynrV2_attr.eMode,
                         cnrV1_attr.eMode);
            }

        }

        if (CHECK_ISP_HW_V30()) {
            rk_aiq_ynr_attrib_v3_t ynrV3_attr;
            ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_cnr_attrib_v2_t cnrV2_attr;
            cnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_bayer2dnr_attrib_v2_t bayer2dnrV2_attr;
            bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_bayertnr_attrib_v2_t bayertnrV2_attr;
            bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &ynrV3_attr);
            ret = rk_aiq_user_api2_acnrV2_GetAttrib(ctx, &cnrV2_attr);
            ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &bayer2dnrV2_attr);
            ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &bayertnrV2_attr);
            RKAIQ_IMGPROC_CHECK_RET(ret, "get anr attrib failed!,ret=%d", ret);

            if(ynrV3_attr.eMode == AYNRV3_OP_MODE_AUTO
                    && cnrV2_attr.eMode == ACNRV2_OP_MODE_AUTO
                    && bayer2dnrV2_attr.eMode == ABAYER2DNR_OP_MODE_AUTO
                    && bayertnrV2_attr.eMode == ABAYERTNRV2_OP_MODE_AUTO) {
                *mode = OP_AUTO;
            } else if(ynrV3_attr.eMode == AYNRV3_OP_MODE_MANUAL
                      && cnrV2_attr.eMode == ACNRV2_OP_MODE_MANUAL
                      && bayer2dnrV2_attr.eMode == ABAYER2DNR_OP_MODE_MANUAL
                      && bayertnrV2_attr.eMode == ABAYERTNRV2_OP_MODE_MANUAL) {
                *mode = OP_MANUAL;
            } else if(ynrV3_attr.eMode == AYNRV3_OP_MODE_REG_MANUAL
                      && cnrV2_attr.eMode == ACNRV2_OP_MODE_REG_MANUAL
                      && bayer2dnrV2_attr.eMode == ABAYER2DNR_OP_MODE_REG_MANUAL
                      && bayertnrV2_attr.eMode == ABAYERTNRV2_OP_MODE_REG_MANUAL) {
                *mode = OP_REG_MANUAL;
            } else {
                LOGE_ANR("ynr.mode:%d cnr.mode:%d bayer2dnr.mode:%d bayertnr.mode:%d\n",
                         ynrV3_attr.eMode,
                         cnrV2_attr.eMode,
                         bayer2dnrV2_attr.eMode,
                         bayertnrV2_attr.eMode);
            }
        }

        if (CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
            rk_aiq_ynr_attrib_v22_t ynrV22_attr;
            ynrV22_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_cnr_attrib_v30_t cnrV30_attr;
            cnrV30_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_bayer2dnr_attrib_v23_t bayer2dnrV23_attr;
            bayer2dnrV23_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_bayertnr_attrib_v23_t bayertnrV23_attr;
            bayertnrV23_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_aynrV22_GetAttrib(ctx, &ynrV22_attr);
            ret = rk_aiq_user_api2_acnrV30_GetAttrib(ctx, &cnrV30_attr);
            ret = rk_aiq_user_api2_abayer2dnrV23_GetAttrib(ctx, &bayer2dnrV23_attr);
            ret = rk_aiq_user_api2_abayertnrV23_GetAttrib(ctx, &bayertnrV23_attr);
            RKAIQ_IMGPROC_CHECK_RET(ret, "get anr attrib failed!,ret=%d", ret);

            if(ynrV22_attr.eMode == AYNRV22_OP_MODE_AUTO
                    && cnrV30_attr.eMode == ACNRV30_OP_MODE_AUTO
                    && bayer2dnrV23_attr.eMode == ABAYER2DNR_V23_OP_MODE_AUTO
                    && bayertnrV23_attr.eMode == ABAYERTNRV23_OP_MODE_AUTO) {
                *mode = OP_AUTO;
            } else if(ynrV22_attr.eMode == AYNRV22_OP_MODE_MANUAL
                      && cnrV30_attr.eMode == ACNRV30_OP_MODE_MANUAL
                      && bayer2dnrV23_attr.eMode == ABAYER2DNR_V23_OP_MODE_MANUAL
                      && bayertnrV23_attr.eMode == ABAYERTNRV23_OP_MODE_MANUAL) {
                *mode = OP_MANUAL;
            } else if(ynrV22_attr.eMode == AYNRV22_OP_MODE_REG_MANUAL
                      && cnrV30_attr.eMode == ACNRV30_OP_MODE_REG_MANUAL
                      && bayer2dnrV23_attr.eMode == ABAYER2DNR_V23_OP_MODE_REG_MANUAL
                      && bayertnrV23_attr.eMode == ABAYERTNRV23_OP_MODE_REG_MANUAL) {
                *mode = OP_REG_MANUAL;
            } else {
                LOGE_ANR("ynr.mode:%d cnr.mode:%d bayer2dnr.mode:%d bayertnr.mode:%d\n",
                         ynrV22_attr.eMode,
                         cnrV30_attr.eMode,
                         bayer2dnrV23_attr.eMode,
                         bayertnrV23_attr.eMode);
            }
        }

        if (CHECK_ISP_HW_V39() ) {
            rk_aiq_ynr_attrib_v24_t ynrV24_attr;
            ynrV24_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_cnr_attrib_v31_t cnrV31_attr;
            cnrV31_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            rk_aiq_bayertnr_attrib_v30_t bayertnrV30_attr;
            bayertnrV30_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_aynrV24_GetAttrib(ctx, &ynrV24_attr);
            ret = rk_aiq_user_api2_acnrV31_GetAttrib(ctx, &cnrV31_attr);
            ret = rk_aiq_user_api2_abayertnrV30_GetAttrib(ctx, &bayertnrV30_attr);
            RKAIQ_IMGPROC_CHECK_RET(ret, "get anr attrib failed!,ret=%d", ret);

            if(ynrV24_attr.eMode == AYNRV24_OP_MODE_AUTO
                    && cnrV31_attr.eMode == ACNRV31_OP_MODE_AUTO
                    && bayertnrV30_attr.eMode == ABAYERTNRV30_OP_MODE_AUTO) {
                *mode = OP_AUTO;
            } else if(ynrV24_attr.eMode == AYNRV24_OP_MODE_MANUAL
                      && cnrV31_attr.eMode == ACNRV31_OP_MODE_MANUAL
                      && bayertnrV30_attr.eMode == ABAYERTNRV30_OP_MODE_MANUAL) {
                *mode = OP_MANUAL;
            } else if(ynrV24_attr.eMode == AYNRV24_OP_MODE_REG_MANUAL
                      && cnrV31_attr.eMode == ACNRV31_OP_MODE_REG_MANUAL
                      && bayertnrV30_attr.eMode == ABAYERTNRV30_OP_MODE_REG_MANUAL) {
                *mode = OP_REG_MANUAL;
            } else {
                LOGE_ANR("ynr.mode:%d cnr.mode:%d bayertnr.mode:%d\n",
                         ynrV24_attr.eMode,
                         cnrV31_attr.eMode,
                         bayertnrV30_attr.eMode);
            }
        }
    */

    ynr_api_attrib_t  ynr_attr;
    cnr_api_attrib_t  cnr_attr;
    btnr_api_attrib_t btnr_attr;

    ret = rk_aiq_user_api2_ynr_GetAttrib(ctx, &ynr_attr);
    ret = rk_aiq_user_api2_cnr_GetAttrib(ctx, &cnr_attr);
    ret = rk_aiq_user_api2_btnr_GetAttrib(ctx, &btnr_attr);

    if (ynr_attr.opMode == RK_AIQ_OP_MODE_AUTO &&
            cnr_attr.opMode == RK_AIQ_OP_MODE_AUTO &&
            btnr_attr.opMode == RK_AIQ_OP_MODE_AUTO) {
        *mode = OP_AUTO;
    } else if (ynr_attr.opMode == RK_AIQ_OP_MODE_MANUAL &&
               cnr_attr.opMode == RK_AIQ_OP_MODE_MANUAL &&
               btnr_attr.opMode == RK_AIQ_OP_MODE_MANUAL) {
        *mode = OP_MANUAL;
    } else {
        LOGE_ANR("ynr.opMode:%d cnr.opMode:%d bayertnr.opMode:%d\n",
                 ynr_attr.opMode,
                 cnr_attr.opMode,
                 btnr_attr.opMode);
    }

    IMGPROC_FUNC_EXIT

    return ret;
}



/*
*****************************
*
* Desc: set normal noise reduction strength
* Argument:
*   level: [0, 100]
* Normal mode
*****************************
*/
XCamReturn rk_aiq_uapi2_setANRStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, setANRStrth failed!");
    }

    /*
        if (CHECK_ISP_HW_V20()) {
            ret = rk_aiq_user_api2_anr_SetLumaSFStrength(ctx, level / 100.0);
            ret = rk_aiq_user_api2_anr_SetLumaTFStrength(ctx, level / 100.0);
            RKAIQ_IMGPROC_CHECK_RET(ret, "setANRStrth failed!", ret);
        }

        if (CHECK_ISP_HW_V21()) {
            ret = rk_aiq_user_api2_aynrV2_SetStrength(ctx, level / 100.0);
            ret = rk_aiq_user_api2_abayernrV2_SetSFStrength(ctx, level / 100.0);
            ret = rk_aiq_user_api2_abayernrV2_SetTFStrength(ctx, level / 100.0);
            RKAIQ_IMGPROC_CHECK_RET(ret, "setANRStrth failed!", ret);
        }

        if (CHECK_ISP_HW_V30()) {
            rk_aiq_ynr_strength_v3_t ynrStrenght;
            ynrStrenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ynrStrenght.percent = level / 100.0;
            ret = rk_aiq_user_api2_aynrV3_SetStrength(ctx, &ynrStrenght);
            rk_aiq_bayer2dnr_strength_v2_t bayer2dnrV2Strenght;
            bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            bayer2dnrV2Strenght.percent = level / 100.0;
            ret = rk_aiq_user_api2_abayer2dnrV2_SetStrength(ctx, &bayer2dnrV2Strenght);
            rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
            bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            bayertnrV2Strenght.percent = level / 100.0;
            ret = rk_aiq_user_api2_abayertnrV2_SetStrength(ctx, &bayertnrV2Strenght);
            RKAIQ_IMGPROC_CHECK_RET(ret, "setANRStrth failed!", ret);
        }

        if (CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
            rk_aiq_ynr_strength_v22_t ynrV22Strength;
            ynrV22Strength.strength_enable = true;
            ynrV22Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ynrV22Strength.percent = level / 100.0;
            ret = rk_aiq_user_api2_aynrV22_SetStrength(ctx, &ynrV22Strength);
            rk_aiq_bayer2dnr_strength_v23_t bayer2dnrV23Strength;
            bayer2dnrV23Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            bayer2dnrV23Strength.strength_enable = true;
            bayer2dnrV23Strength.percent = level / 100.0;
            ret = rk_aiq_user_api2_abayer2dnrV23_SetStrength(ctx, &bayer2dnrV23Strength);
            rk_aiq_bayertnr_strength_v23_t bayertnrV23Strength;
            bayertnrV23Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            bayertnrV23Strength.strength_enable = true;
            bayertnrV23Strength.percent = level / 100.0;
            ret = rk_aiq_user_api2_abayertnrV23_SetStrength(ctx, &bayertnrV23Strength);
            RKAIQ_IMGPROC_CHECK_RET(ret, "setANRStrth failed!", ret);
        }

    */

    aynr_strength_t ynrStrength;
    ynrStrength.en = true;
    ynrStrength.percent = level / 100.0;
    ret = rk_aiq_user_api2_ynr_SetStrength(ctx, &ynrStrength);
    abtnr_strength_t btnrStrength;
    btnrStrength.en = true;
    btnrStrength.percent = level / 100.0;
    ret = rk_aiq_user_api2_btnr_SetStrength(ctx, &btnrStrength);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setANRStrth failed!", ret);


    IMGPROC_FUNC_EXIT
    return ret;
}


XCamReturn rk_aiq_uapi2_getANRStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    float percent = 0.0f;

    IMGPROC_FUNC_ENTER

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, getANRStrth failed!");
    }

    /*
        if (CHECK_ISP_HW_V20()) {
            ret = rk_aiq_user_api2_anr_GetLumaTFStrength(ctx, &percent);
            RKAIQ_IMGPROC_CHECK_RET(ret, "getANRStrth failed!", ret);
            *level = (unsigned int)(percent * 100);
        }

        if (CHECK_ISP_HW_V21()) {
            ret = rk_aiq_user_api2_abayernrV2_GetTFStrength(ctx, &percent);
            RKAIQ_IMGPROC_CHECK_RET(ret, "getANRStrth failed!", ret);
            *level = (unsigned int)(percent * 100);
        }

        if (CHECK_ISP_HW_V30()) {
            rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
            bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_abayertnrV2_GetStrength(ctx, &bayertnrV2Strenght);
            percent = bayertnrV2Strenght.percent;
            RKAIQ_IMGPROC_CHECK_RET(ret, "getANRStrth failed!", ret);
            *level = (unsigned int)(percent * 100);
        }

        if ( CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
            rk_aiq_bayertnr_strength_v23_t bayertnrV23Strength;
            bayertnrV23Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_abayertnrV23_GetStrength(ctx, &bayertnrV23Strength);
            RKAIQ_IMGPROC_CHECK_RET(ret, "getANRStrth failed!", ret);
            *level = (unsigned int)(bayertnrV23Strength.percent * 100);
        }

        if (CHECK_ISP_HW_V39()) {
            rk_aiq_bayertnr_strength_v30_t bayertnrV30Strength;
            bayertnrV30Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_abayertnrV30_GetStrength(ctx, &bayertnrV30Strength);
            RKAIQ_IMGPROC_CHECK_RET(ret, "getANRStrth failed!", ret);
            *level = (unsigned int)(bayertnrV30Strength.percent * 100);
        }
    */

    abtnr_strength_t btnrStrength;
    ret = rk_aiq_user_api2_btnr_GetStrength(ctx, &btnrStrength);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setANRStrth failed!", ret);
    *level = (unsigned int)(btnrStrength.percent * 100);


    IMGPROC_FUNC_EXIT

    return ret;
}


/*
*****************************
*
* Desc: set manual spatial noise reduction strength
*    this function is active for NR is manual mode
* Argument:
*   level: [0, 100]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setMSpaNRStrth(const rk_aiq_sys_ctx_t* ctx, bool on, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, setMSpaNRStrth failed!");
    }

    /*
        if (CHECK_ISP_HW_V20()) {
            ret = rk_aiq_user_api2_anr_SetLumaSFStrength(ctx, level / 100.0);
        }

        if (CHECK_ISP_HW_V21()) {
            ret = rk_aiq_user_api2_abayernrV2_SetSFStrength(ctx, level / 100.0);
            ret = rk_aiq_user_api2_aynrV2_SetStrength(ctx, level / 100.0);
        }

        if (CHECK_ISP_HW_V30()) {
            rk_aiq_ynr_strength_v3_t ynrStrenght;
            ynrStrenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ynrStrenght.strength_enable = true;
            ynrStrenght.percent = level / 100.0;
            ret = rk_aiq_user_api2_aynrV3_SetStrength(ctx, &ynrStrenght);
            rk_aiq_bayer2dnr_strength_v2_t bayer2dnrV2Strenght;
            bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            bayer2dnrV2Strenght.strength_enable = true;
            bayer2dnrV2Strenght.percent = level / 100.0;
            ret = rk_aiq_user_api2_abayer2dnrV2_SetStrength(ctx, &bayer2dnrV2Strenght);
        }

        if (CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
            rk_aiq_ynr_strength_v22_t ynrStrength;
            ynrStrength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ynrStrength.strength_enable = true;
            ynrStrength.percent = level / 100.0;
            ret = rk_aiq_user_api2_aynrV22_SetStrength(ctx, &ynrStrength);
            rk_aiq_bayer2dnr_strength_v23_t bayer2dnrV23Strength;
            bayer2dnrV23Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            bayer2dnrV23Strength.strength_enable = true;
            bayer2dnrV23Strength.percent = level / 100.0;
            ret = rk_aiq_user_api2_abayer2dnrV23_SetStrength(ctx, &bayer2dnrV23Strength);
        }

        if (CHECK_ISP_HW_V39() ) {
            rk_aiq_ynr_strength_v24_t ynrStrength;
            ynrStrength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ynrStrength.strength_enable = true;
            ynrStrength.percent = level / 100.0;
            ret = rk_aiq_user_api2_aynrV24_SetStrength(ctx, &ynrStrength);
        }

    */
    aynr_strength_t ynrStrength;
    ynrStrength.en = true;
    ynrStrength.percent = level / 100.0;
    ret = rk_aiq_user_api2_ynr_SetStrength(ctx, &ynrStrength);

    RKAIQ_IMGPROC_CHECK_RET(ret, "setMSpaNRStrth failed!", ret);
    IMGPROC_FUNC_EXIT

    return ret;
}



/*
*****************************
*
* Desc: get manual spatial noise reduction strength
*    this function is active for NR is manual mode
* Argument:
*   level: [0, 100]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_getMSpaNRStrth(const rk_aiq_sys_ctx_t* ctx, bool * on, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    float percent = 0.0f;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, getMSpaNRStrth failed!");
    }

    /*
        if (CHECK_ISP_HW_V20()) {
            ret = rk_aiq_user_api2_anr_GetLumaSFStrength(ctx, &percent);
        }

        if (CHECK_ISP_HW_V21()) {
            ret = rk_aiq_user_api2_abayernrV2_GetSFStrength(ctx, &percent);
        }

        if (CHECK_ISP_HW_V30()) {
            rk_aiq_bayer2dnr_strength_v2_t bayer2dnrV2Strenght;
            bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_abayer2dnrV2_GetStrength(ctx, &bayer2dnrV2Strenght);
            percent = bayer2dnrV2Strenght.percent;
        }

        if (CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
            rk_aiq_bayer2dnr_strength_v23_t bayer2dnrV23Strength;
            bayer2dnrV23Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_abayer2dnrV23_GetStrength(ctx, &bayer2dnrV23Strength);
            percent = bayer2dnrV23Strength.percent;
        }

        if (CHECK_ISP_HW_V39() ) {
            rk_aiq_ynr_strength_v24_t ynrV24Strength;
            ynrV24Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_aynrV24_GetStrength(ctx, &ynrV24Strength);
            percent = ynrV24Strength.percent;
        }
    */

    aynr_strength_t ynrStrength;
    ret = rk_aiq_user_api2_ynr_GetStrength(ctx, &ynrStrength);
    percent = ynrStrength.percent;

    RKAIQ_IMGPROC_CHECK_RET(ret, "getMSpaNRStrth failed!", ret);
    *level = (unsigned int)(percent * 100);
    IMGPROC_FUNC_EXIT

    return ret;
}



/*
*****************************
*
* Desc: set manual time noise reduction strength
*     this function is active for NR is manual mode
* Argument:
*   level: [0, 100]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setMTNRStrth(const rk_aiq_sys_ctx_t* ctx, bool on, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    LOGD("level=%d", level);
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, setMTNRStrth failed!");
    }

    /*
        if (CHECK_ISP_HW_V20()) {
            ret = rk_aiq_user_api2_anr_SetLumaTFStrength(ctx, level / 100.0);
        }

        if (CHECK_ISP_HW_V21()) {
            ret = rk_aiq_user_api2_abayernrV2_SetTFStrength(ctx, level / 100.0);
        }

        if (CHECK_ISP_HW_V30()) {
            rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
            bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            bayertnrV2Strenght.strength_enable = true;
            bayertnrV2Strenght.percent = level / 100.0;
            ret = rk_aiq_user_api2_abayertnrV2_SetStrength(ctx, &bayertnrV2Strenght);
        }

        if ( CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
            rk_aiq_bayertnr_strength_v23_t bayertnrV23Strength;
            bayertnrV23Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            bayertnrV23Strength.strength_enable = true;
            bayertnrV23Strength.percent = level / 100.0;
            ret = rk_aiq_user_api2_abayertnrV23_SetStrength(ctx, &bayertnrV23Strength);
        }

        if (CHECK_ISP_HW_V39() ) {
            rk_aiq_bayertnr_strength_v30_t bayertnrV30Strength;
            bayertnrV30Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            bayertnrV30Strength.strength_enable = true;
            bayertnrV30Strength.percent = level / 100.0;
            ret = rk_aiq_user_api2_abayertnrV30_SetStrength(ctx, &bayertnrV30Strength);
        }
    */
    abtnr_strength_t btnrStrength;
    btnrStrength.en = true;
    btnrStrength.percent = level / 100.0;
    ret = rk_aiq_user_api2_btnr_SetStrength(ctx, &btnrStrength);

    RKAIQ_IMGPROC_CHECK_RET(ret, "setMTNRStrth failed!", ret);
    IMGPROC_FUNC_EXIT

    return ret;
}



/*
*****************************
*
* Desc: get manual time noise reduction strength
*     this function is active for NR is manual mode
* Argument:
*   level: [0, 100]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_getMTNRStrth(const rk_aiq_sys_ctx_t* ctx, bool * on, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    float percent = 0.0f;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, getMTNRStrth failed!");
    }

    /*
        if (CHECK_ISP_HW_V20()) {
            ret = rk_aiq_user_api2_anr_GetLumaTFStrength(ctx, &percent);
        }

        if (CHECK_ISP_HW_V21()) {
            ret = rk_aiq_user_api2_abayernrV2_GetTFStrength(ctx, &percent);
        }

        if (CHECK_ISP_HW_V30()) {
            rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
            bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_abayertnrV2_GetStrength(ctx, &bayertnrV2Strenght);
            percent = bayertnrV2Strenght.percent;
        }

        if ( CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
            rk_aiq_bayertnr_strength_v23_t bayertnrV23Strength;
            bayertnrV23Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_abayertnrV23_GetStrength(ctx, &bayertnrV23Strength);
            percent = bayertnrV23Strength.percent;
        }

        if (CHECK_ISP_HW_V39() ) {
            rk_aiq_bayertnr_strength_v30_t bayertnrV30Strength;
            bayertnrV30Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_abayertnrV30_GetStrength(ctx, &bayertnrV30Strength);
            percent = bayertnrV30Strength.percent;
        }
    */

    abtnr_strength_t btnrStrength;
    ret = rk_aiq_user_api2_btnr_GetStrength(ctx, &btnrStrength);
    percent = btnrStrength.percent;

    RKAIQ_IMGPROC_CHECK_RET(ret, "getMTNRStrth failed!", ret);
    *level = (unsigned int)(percent * 100);
    IMGPROC_FUNC_EXIT
    return ret;
}


/*
*****************************
*
* Desc: Adjust image sharpness level
* Argument:
*    level: sharpness level, [0, 100]
*****************************
*/
XCamReturn rk_aiq_uapi2_setSharpness(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    float fPercent = 0.0f;

    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, set sharpeness failed!");
    }

    LOGD("setSharpness enter, level=%d\n", level);
    if ((int)level < 0 || level > 100) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "level out of range, set sharpeness failed!");
    }
    fPercent = level / 100.0f;

    /*
        if (CHECK_ISP_HW_V21()) {
            ret = rk_aiq_user_api2_asharpV3_SetStrength(ctx, fPercent);
        }

        if (CHECK_ISP_HW_V30()) {
            rk_aiq_sharp_strength_v4_t sharpV4Strenght;
            sharpV4Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            sharpV4Strenght.strength_enable = true;
            sharpV4Strenght.percent = fPercent;
            ret = rk_aiq_user_api2_asharpV4_SetStrength(ctx, &sharpV4Strenght);
        }

        if (CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
            rk_aiq_sharp_strength_v33_t sharpV33Strength;
            sharpV33Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            sharpV33Strength.strength_enable = true;
            sharpV33Strength.percent = fPercent;
            ret = rk_aiq_user_api2_asharpV33_SetStrength(ctx, &sharpV33Strength);
        }

        if (CHECK_ISP_HW_V39()) {
            rk_aiq_sharp_strength_v34_t sharpV34Strength;
            sharpV34Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            sharpV34Strength.strength_enable = true;
            sharpV34Strength.percent = fPercent;
            ret = rk_aiq_user_api2_asharpV34_SetStrength(ctx, &sharpV34Strength);
        }
    */

    asharp_strength_t sharpStrength;
    sharpStrength.en = true;
    sharpStrength.percent = fPercent;
    ret = rk_aiq_user_api2_sharp_SetStrength(ctx, &sharpStrength);

    RKAIQ_IMGPROC_CHECK_RET(ret, "set sharpeness failed!");
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_getSharpness(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    float fPercent = 0.0f;

    IMGPROC_FUNC_ENTER
    if (level == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get sharpeness failed!");
    }

    /*
        if (CHECK_ISP_HW_V21()) {
            ret = rk_aiq_user_api2_asharpV3_GetStrength(ctx, &fPercent);
        }

        if (CHECK_ISP_HW_V30()) {
            rk_aiq_sharp_strength_v4_t sharpV4Strenght;
            sharpV4Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_asharpV4_GetStrength(ctx, &sharpV4Strenght);
            fPercent = sharpV4Strenght.percent;
        }

        if (CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
            rk_aiq_sharp_strength_v33_t sharpV33Strength;
            sharpV33Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_asharpV33_GetStrength(ctx, &sharpV33Strength);
            fPercent = sharpV33Strength.percent;
        }

        if (CHECK_ISP_HW_V39() ) {
            rk_aiq_sharp_strength_v34_t sharpV34Strength;
            sharpV34Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
            ret = rk_aiq_user_api2_asharpV34_GetStrength(ctx, &sharpV34Strength);
            fPercent = sharpV34Strength.percent;
        }
    */

    asharp_strength_t sharpStrength;
    ret = rk_aiq_user_api2_sharp_GetStrength(ctx, &sharpStrength);
    fPercent = sharpStrength.percent;

    RKAIQ_IMGPROC_CHECK_RET(ret, "get sharpeness failed!");

    *level = (unsigned int)(fPercent * 100);
    IMGPROC_FUNC_EXIT

    return ret;
}


/*
**********************************************************
* White balance & Color
**********************************************************
*/
/*
*****************************
*
* Desc: set white balance mode
* Argument:
*   mode:  auto: auto white balance
*          manual: manual white balance
*****************************
*/
XCamReturn rk_aiq_uapi2_setWBMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_op_mode_t mode2;
    IMGPROC_FUNC_ENTER
    if (mode >= OP_INVAL || mode < OP_AUTO) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "mode is invalid!");
    }
    if (mode == OP_AUTO) {
        mode2 = RK_AIQ_OP_MODE_AUTO;
    } else if (mode == OP_MANUAL) {
        mode2 = RK_AIQ_OP_MODE_MANUAL;
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "Not supported mode!");
    }
    awb_gainCtrl_t attr;
    ret = rk_aiq_user_api2_awb_GetWbGainCtrlAttrib(ctx, &attr );
    RKAIQ_IMGPROC_CHECK_RET(ret, "GetWbGainCtrlAttrib failed!");
    attr.opMode = mode2;
    ret = rk_aiq_user_api2_awb_SetWbGainCtrlAttrib(ctx, &attr );
    RKAIQ_IMGPROC_CHECK_RET(ret, "GetWbGainCtrlAttrib failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}
XCamReturn rk_aiq_uapi2_getWBMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    rk_aiq_wb_querry_info_t query_info;
    ret = rk_aiq_user_api2_awb_QueryWBInfo(ctx, &query_info);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getWBMode failed!");
    if (query_info.opMode == RK_AIQ_OP_MODE_AUTO) {
        *mode = OP_AUTO;
    } else if (query_info.opMode == RK_AIQ_OP_MODE_MANUAL) {
        *mode = OP_MANUAL;
    } else {
        *mode = OP_INVAL;
    }
    IMGPROC_FUNC_EXIT
    return ret;
}



/*
*****************************
*
* Desc: lock/unlock auto white balance
* Argument:
*
*
*****************************
*/
XCamReturn rk_aiq_uapi2_lockAWB(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_awb_Lock(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_unlockAWB(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_awb_Unlock(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set manual white balance scene mode
* Argument:
*   ct_scene:
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setMWBScene(const rk_aiq_sys_ctx_t* ctx, rk_aiq_wb_scene_t scene)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_getMWBScene(const rk_aiq_sys_ctx_t* ctx, rk_aiq_wb_scene_t *scene)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}


/*
*****************************
*
* Desc: set manual white balance r/b gain
* Argument:
*   ct_scene:
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setMWBGain(const rk_aiq_sys_ctx_t* ctx, rk_aiq_wb_gain_t *gain)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (gain == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, setMWBGain failed!");
    }
    awb_gainCtrl_t attr;
    ret = rk_aiq_user_api2_awb_GetWbGainCtrlAttrib(ctx, &attr );
    RKAIQ_IMGPROC_CHECK_RET(ret, "GetWbGainCtrlAttrib failed!");
    attr.manualPara.mode = mwb_mode_wbgain;
    attr.manualPara.cfg.manual_wbgain[0] = gain->rgain;
    attr.manualPara.cfg.manual_wbgain[1] = gain->grgain;
    attr.manualPara.cfg.manual_wbgain[2] = gain->gbgain;
    attr.manualPara.cfg.manual_wbgain[3] = gain->bgain;
    ret = rk_aiq_user_api2_awb_SetWbGainCtrlAttrib(ctx, &attr );
    RKAIQ_IMGPROC_CHECK_RET(ret, "GetWbGainCtrlAttrib failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getWBGain(const rk_aiq_sys_ctx_t* ctx, rk_aiq_wb_gain_t *gain)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_wb_querry_info_t query_info;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (gain == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getMWBGain failed!");
    }
    ret = rk_aiq_user_api2_awb_QueryWBInfo(ctx, &query_info);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMWBGain failed!");
    *gain = query_info.gain;
    IMGPROC_FUNC_EXIT

    return ret;
}
/*
*****************************
*
* Desc: set manual white balance color temperature
* Argument:
*   ct: color temperature value [2800, 7500]K
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setMWBCT(const rk_aiq_sys_ctx_t* ctx, unsigned int ct)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_getWBCT(const rk_aiq_sys_ctx_t* ctx, unsigned int *ct)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}


XCamReturn rk_aiq_uapi2_setAwbGainOffsetAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wb_awb_wbGainOffset_t offset)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_getAwbGainOffsetAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wb_awb_wbGainOffset_t *offset)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_uapi2_setAwbGainAdjustAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wb_awb_wbGainAdjust_t adjust)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_getAwbGainAdjustAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wb_awb_wbGainAdjust_t *adjust)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_setAwbMultiWindowAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wb_awb_mulWindow_t multiwindow)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_getAwbMultiWindowAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wb_awb_mulWindow_t *multiwindow)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_setAwbV30AllAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wbV30_attrib_t attr)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_getAwbV30AllAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wbV30_attrib_t *attr)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_setAwbV21AllAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wbV21_attrib_t attr)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_getAwbV21AllAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wbV21_attrib_t *attr)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}


/*
**********************************************************
* Focus & Zoom
**********************************************************
*/
#if defined(ISP_HW_V33)
XCamReturn rk_aiq_uapi2_setFocusMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_getFocusMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_setFocusPosition(const rk_aiq_sys_ctx_t* ctx, short code)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_getFocusPosition(const rk_aiq_sys_ctx_t* ctx, short * code)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_setFocusWin(const rk_aiq_sys_ctx_t* ctx, paRect_t *rect)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_getFocusWin(const rk_aiq_sys_ctx_t* ctx, paRect_t *rect)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_lockFocus(const rk_aiq_sys_ctx_t* ctx)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_unlockFocus(const rk_aiq_sys_ctx_t* ctx)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_oneshotFocus(const rk_aiq_sys_ctx_t* ctx)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_manualTrigerFocus(const rk_aiq_sys_ctx_t* ctx)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_trackingFocus(const rk_aiq_sys_ctx_t* ctx)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_getSearchPath(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_sec_path_t* path)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_getSearchResult(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_result_t* result)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_setOpZoomPosition(const rk_aiq_sys_ctx_t* ctx, int pos)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_getOpZoomPosition(const rk_aiq_sys_ctx_t* ctx, int *pos)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_endOpZoomChange(const rk_aiq_sys_ctx_t* ctx)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_getZoomRange(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_zoomrange * range)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_getFocusRange(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_focusrange* range)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_startZoomCalib(const rk_aiq_sys_ctx_t* ctx)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_resetZoom(const rk_aiq_sys_ctx_t* ctx)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_setAngleZ(const rk_aiq_sys_ctx_t* ctx, float angleZ)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_setPdafLibInput(const rk_aiq_sys_ctx_t* ctx, rk_aiq_pdlib_input* pdlib_input)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_getPdafLibOutput(const rk_aiq_sys_ctx_t* ctx, rk_aiq_pdlib_output* pdlib_output, int timeout_ms)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

#else
XCamReturn rk_aiq_uapi2_setFocusMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_af_attrib_t attr;
    ret = rk_aiq_user_api2_af_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setFocusMode failed!");
    if (mode == OP_AUTO) {
        attr.AfMode = RKAIQ_AF_MODE_CONTINUOUS_PICTURE;
    } else if (mode == OP_MANUAL) {
        attr.AfMode = RKAIQ_AF_MODE_FIXED;
    } else if (mode == OP_SEMI_AUTO) {
        attr.AfMode = RKAIQ_AF_MODE_ONESHOT_AFTER_ZOOM;
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "Not supported mode!");
    }

    attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
    ret = rk_aiq_user_api2_af_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setFocusMode failed!");
    return ret;
}

XCamReturn rk_aiq_uapi2_getFocusMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_af_attrib_t attr;
    ret = rk_aiq_user_api2_af_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getFocusMode failed!");
    if (attr.AfMode == RKAIQ_AF_MODE_FIXED) {
        *mode = OP_MANUAL;
    } else if (attr.AfMode == RKAIQ_AF_MODE_NOT_SET) {
        *mode = OP_INVAL;
    } else if (attr.AfMode == RKAIQ_AF_MODE_ONESHOT_AFTER_ZOOM) {
        *mode = OP_SEMI_AUTO;
    } else {
        *mode = OP_AUTO;
    }

    return ret;
}

XCamReturn rk_aiq_uapi2_setFocusPosition(const rk_aiq_sys_ctx_t* ctx, short code)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_af_attrib_t attr;

    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setFixedModeCode failed!");
    attr.fixedModeDefCode = code;
    attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
    ret = rk_aiq_user_api2_af_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setFixedModeCode failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getFocusPosition(const rk_aiq_sys_ctx_t* ctx, short * code)
{
    bool zoom_support = ctx->_analyzer->mAlogsComSharedParams.snsDes.lens_des.zoom_support;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_af_attrib_t attr;
    IMGPROC_FUNC_ENTER
    if (zoom_support) {
        ret = rk_aiq_user_api2_af_GetAttrib(ctx, &attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getFixedModeCode failed!");
        *code = attr.fixedModeDefCode;
    } else {
        int pos;
        ret = rk_aiq_user_api2_af_GetFocusPos(ctx, &pos);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getFixedModeCode failed!");
        *code = pos;
    }
    // LOGD_AF("%s: focus position %d", __func__, *code);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_setFocusWin(const rk_aiq_sys_ctx_t* ctx, paRect_t *rect)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_af_attrib_t attr;

    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setFocusWin failed!");

    attr.h_offs = rect->x;
    attr.v_offs = rect->y;
    attr.h_size = rect->w;
    attr.v_size = rect->h;
    attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
    ret = rk_aiq_user_api2_af_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setFocusWin failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getFocusWin(const rk_aiq_sys_ctx_t* ctx, paRect_t *rect)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_af_attrib_t attr;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getFocusWin failed!");

    rect->x = attr.h_offs;
    rect->y = attr.v_offs;
    rect->w = attr.h_size;
    rect->h = attr.v_size;
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_lockFocus(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_Lock(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_unlockFocus(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_Unlock(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_oneshotFocus(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_Oneshot(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_manualTrigerFocus(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_ManualTriger(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_trackingFocus(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_Tracking(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getSearchPath(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_sec_path_t* path)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_GetSearchPath(ctx, path);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_getSearchResult(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_result_t* result)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_GetSearchResult(ctx, result);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_setOpZoomPosition(const rk_aiq_sys_ctx_t* ctx, int pos)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_SetZoomIndex(ctx, pos);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_getOpZoomPosition(const rk_aiq_sys_ctx_t* ctx, int *pos)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_GetZoomIndex(ctx, pos);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_endOpZoomChange(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_EndZoomChg(ctx);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_getZoomRange(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_zoomrange * range)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_GetZoomRange(ctx, range);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_getFocusRange(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_focusrange* range)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_GetFocusRange(ctx, range);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_startZoomCalib(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_StartZoomCalib(ctx);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_resetZoom(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_resetZoom(ctx);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_setAngleZ(const rk_aiq_sys_ctx_t* ctx, float angleZ)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_setAngleZ(ctx, angleZ);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_setPdafLibInput(const rk_aiq_sys_ctx_t* ctx, rk_aiq_pdlib_input* pdlib_input)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_SetPdafLibInput(ctx, pdlib_input);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_getPdafLibOutput(const rk_aiq_sys_ctx_t* ctx, rk_aiq_pdlib_output* pdlib_output, int timeout_ms)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_GetPdafLibOutput(ctx, pdlib_output, timeout_ms);
    IMGPROC_FUNC_EXIT

    return ret;
}

#endif

XCamReturn rk_aiq_uapi2_setAcolorSwInfo(const rk_aiq_sys_ctx_t* ctx,
                                        rk_aiq_color_info_t aColor_sw_info)
{
    /*
        XCamReturn ret = XCAM_RETURN_NO_ERROR;
        IMGPROC_FUNC_ENTER
        ret = rk_aiq_user_api2_accm_SetAcolorSwInfo(ctx, aColor_sw_info);
        ret = rk_aiq_user_api2_alsc_SetAcolorSwInfo(ctx, aColor_sw_info);
        ret = rk_aiq_user_api2_a3dlut_SetAcolorSwInfo(ctx, aColor_sw_info);
        IMGPROC_FUNC_EXIT
    */
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}



/*
**********************************************************
* Color Correction
**********************************************************
*/
/*
*****************************
*
* Desc: set/get color correction mode
* Argument:
*   mode:
*     auto: auto color correction
*     manual: manual color correction
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setCCMMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    /*
    #if RKAIQ_HAVE_CCM_V1
        rk_aiq_ccm_attrib_t attr;
    #elif RKAIQ_HAVE_CCM_V2
        rk_aiq_ccm_v2_attrib_t attr;
    #elif RKAIQ_HAVE_CCM_V3
        rk_aiq_ccm_v3_attrib_t attr;
    #else
        return ret;
    #endif
        memset(&attr, 0, sizeof(attr));
        IMGPROC_FUNC_ENTER
        if (mode >= OP_INVAL || mode < OP_AUTO) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "mode is invalid!");
        }
    #if RKAIQ_HAVE_CCM_V1
        ret = rk_aiq_user_api2_accm_GetAttrib(ctx, &attr);
    #elif RKAIQ_HAVE_CCM_V2
        ret = rk_aiq_user_api2_accm_v2_GetAttrib(ctx, &attr);
    #elif RKAIQ_HAVE_CCM_V3
        ret = rk_aiq_user_api2_accm_v3_GetAttrib(ctx, &attr);
    #endif
        RKAIQ_IMGPROC_CHECK_RET(ret, "setCCMMode failed in getting accm attrib!");

        attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
        if (mode == OP_AUTO) {
            attr.mode = RK_AIQ_CCM_MODE_AUTO;
        } else if (mode == OP_MANUAL) {
            attr.mode = RK_AIQ_CCM_MODE_MANUAL;
        } else {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "Not supported mode!");
        }
    #if RKAIQ_HAVE_CCM_V1
        ret = rk_aiq_user_api2_accm_SetAttrib(ctx, &attr);
    #elif RKAIQ_HAVE_CCM_V2
        ret = rk_aiq_user_api2_accm_v2_SetAttrib(ctx, &attr);
    #elif RKAIQ_HAVE_CCM_V3
        ret = rk_aiq_user_api2_accm_v3_SetAttrib(ctx, &attr);
    #endif

        RKAIQ_IMGPROC_CHECK_RET(ret, "setCCMMode failed!");
        IMGPROC_FUNC_EXIT
    */

    LOGE_ACCM("no support, please use ccm rk_aiq_user_api2_ccm_SetAttrib api to set mode to manual");
    ret = XCAM_RETURN_ERROR_FAILED;

    return ret;
}

XCamReturn rk_aiq_uapi2_getCCMMode(const rk_aiq_sys_ctx_t* ctx, opMode_t* mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    /*
    #if RKAIQ_HAVE_CCM_V1
        rk_aiq_ccm_attrib_t attr;
    #elif RKAIQ_HAVE_CCM_V2
        rk_aiq_ccm_v2_attrib_t attr;
    #elif RKAIQ_HAVE_CCM_V3
        rk_aiq_ccm_v3_attrib_t attr;
    #else
        return ret;
    #endif
        memset(&attr, 0, sizeof(attr));
        IMGPROC_FUNC_ENTER
    #if RKAIQ_HAVE_CCM_V1
        ret = rk_aiq_user_api2_accm_GetAttrib(ctx, &attr);
    #elif RKAIQ_HAVE_CCM_V2
        ret = rk_aiq_user_api2_accm_v2_GetAttrib(ctx, &attr);
    #elif RKAIQ_HAVE_CCM_V3
        ret = rk_aiq_user_api2_accm_v3_GetAttrib(ctx, &attr);
    #endif
        RKAIQ_IMGPROC_CHECK_RET(ret, "getCCMMode failed!");
        if (attr.mode == RK_AIQ_CCM_MODE_AUTO) {
            *mode = OP_AUTO;
        } else if (attr.mode == RK_AIQ_CCM_MODE_MANUAL) {
            *mode = OP_MANUAL;
        } else {
            *mode = OP_INVAL;
        }
        IMGPROC_FUNC_EXIT
    */

    IMGPROC_FUNC_ENTER

    ccm_api_attrib_t ccm_attr;
    ret = rk_aiq_user_api2_ccm_GetAttrib(ctx, &ccm_attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getCCMMode failed!");
    if (ccm_attr.opMode == RK_AIQ_OP_MODE_AUTO) {
        *mode = OP_AUTO;
    } else if (ccm_attr.opMode == RK_AIQ_OP_MODE_MANUAL) {
        *mode = OP_MANUAL;
    } else {
        *mode = OP_INVAL;
    }
    IMGPROC_FUNC_EXIT

    return ret;
}

/*
*****************************
*
* Desc: set manual color correction matrix
*     this function is active for color correction is manual mode
        get color correction matrix
* Argument:
* mccm:
*                   3x3 matrix
*                   1x3 offset
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setMCcCoef(const rk_aiq_sys_ctx_t* ctx, rk_aiq_ccm_matrix_t* mccm)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    /*
    #if RKAIQ_HAVE_CCM_V1
        rk_aiq_ccm_attrib_t attr;
    #elif RKAIQ_HAVE_CCM_V2
        rk_aiq_ccm_v2_attrib_t attr;
    #elif RKAIQ_HAVE_CCM_V3
        rk_aiq_ccm_v3_attrib_t attr;
    #else
        return ret;
    #endif
        memset(&attr, 0, sizeof(attr));
        IMGPROC_FUNC_ENTER
        if ((ctx == NULL) || (mccm == NULL) ) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error, set CCM Manual Matrix failed!");
        }
    #if RKAIQ_HAVE_CCM_V1
        ret = rk_aiq_user_api2_accm_GetAttrib(ctx, &attr);
    #elif RKAIQ_HAVE_CCM_V2
        ret = rk_aiq_user_api2_accm_v2_GetAttrib(ctx, &attr);
    #elif RKAIQ_HAVE_CCM_V3
        ret = rk_aiq_user_api2_accm_v3_GetAttrib(ctx, &attr);
    #endif
        RKAIQ_IMGPROC_CHECK_RET(ret, "Set CCM Manual Matrix failed in getting accm attrib!!");

        attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
        attr.mode = RK_AIQ_CCM_MODE_MANUAL;
        memcpy(attr.stManual.Matrix.ccMatrix, mccm->ccMatrix, sizeof(float) * 9);
        memcpy(attr.stManual.Matrix.ccOffsets, mccm->ccOffsets, sizeof(float) * 3);
    #if RKAIQ_HAVE_CCM_V1
        ret = rk_aiq_user_api2_accm_SetAttrib(ctx, &attr);
    #elif RKAIQ_HAVE_CCM_V2
        ret = rk_aiq_user_api2_accm_v2_SetAttrib(ctx, &attr);
    #elif RKAIQ_HAVE_CCM_V3
        ret = rk_aiq_user_api2_accm_v3_SetAttrib(ctx, &attr);
    #endif
        RKAIQ_IMGPROC_CHECK_RET(ret, "set CCM Manual Matrix failed!");
        IMGPROC_FUNC_EXIT
    */

    LOGE("not support to call %s for current chip", __FUNCTION__);
    ret = XCAM_RETURN_ERROR_UNKNOWN;

    return ret;
}

XCamReturn rk_aiq_uapi2_getMCcCoef(const rk_aiq_sys_ctx_t* ctx, rk_aiq_ccm_matrix_t* mccm)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    /*
        rk_aiq_ccm_querry_info_t ccm_querry_info;
        IMGPROC_FUNC_ENTER
        if ((ctx == NULL) || (mccm == NULL)) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get CCM Matrix failed!");
        }

        ret = rk_aiq_user_api2_accm_QueryCcmInfo(ctx, &ccm_querry_info);
        RKAIQ_IMGPROC_CHECK_RET(ret, "get CCM Matrix failed!");

        for (int i = 0; i < 9; i++)
            mccm->ccMatrix[i] = ccm_querry_info.Matrix.ccMatrix[i];
        for (int i = 0; i < 3; i++)
            mccm->ccOffsets[i] = ccm_querry_info.Matrix.ccOffsets[i];
        IMGPROC_FUNC_EXIT
    */

    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (mccm == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get CCM Matrix failed!");
    }

    ccm_status_t ccm_status;
    ret = rk_aiq_user_api2_ccm_QueryStatus(ctx, &ccm_status);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get CCM Matrix failed!");

    for (int i = 0; i < 9; i++)
        mccm->ccMatrix[i] = ccm_status.stMan.dyn.ccMatrix.hw_ccmC_matrix_coeff[i];
    for (int i = 0; i < 3; i++)
        mccm->ccOffsets[i] = ccm_status.stMan.dyn.ccMatrix.hw_ccmC_matrix_offset[i];
    IMGPROC_FUNC_EXIT

    return ret;
}

/*
*****************************
*
* Desc: get auto color correction saturation
*     this function is active for color correction is auto mode
* Argument:
*   finalsat : range in [0, 100]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_getACcmSat(const rk_aiq_sys_ctx_t* ctx, float* finalsat)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    /*
        rk_aiq_ccm_querry_info_t ccm_querry_info;
        IMGPROC_FUNC_ENTER
        if ((ctx == NULL) || (finalsat == NULL)) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get Auto CCM Saturation failed!");
        }

        ret = rk_aiq_user_api2_accm_QueryCcmInfo(ctx, &ccm_querry_info);
        RKAIQ_IMGPROC_CHECK_RET(ret, "get CCM Auto CCM Saturation failed!");
        *finalsat = ccm_querry_info.finalSat;
        IMGPROC_FUNC_EXIT
    */

    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (finalsat == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get Auto CCM Saturation failed!");
    }

    ccm_status_t ccm_status;
    ret = rk_aiq_user_api2_ccm_QueryStatus(ctx, &ccm_status);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get CCM Auto CCM Saturation failed!");
    *finalsat = ccm_status.accmStatus.sw_ccmC_ccmSat_val;

    return ret;
}

/*
*****************************
*
* Desc: get auto color correction used ccm name
*     this function is active for color correction is auto mode
* Argument:
*    ccm_name[2]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_getACcmMatrixName(const rk_aiq_sys_ctx_t* ctx, char** ccm_name)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    /*
        rk_aiq_ccm_querry_info_t ccm_querry_info;
        IMGPROC_FUNC_ENTER
        if ((ctx == NULL) || (ccm_name == NULL)) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get Auto CCM Name failed!");
        }

        ret = rk_aiq_user_api2_accm_QueryCcmInfo(ctx, &ccm_querry_info);
        RKAIQ_IMGPROC_CHECK_RET(ret, "get CCM Auto CCM Name failed!");
        strcpy(ccm_name[0], ccm_querry_info.ccmname1);
        strcpy(ccm_name[1], ccm_querry_info.ccmname2);
        IMGPROC_FUNC_EXIT
    */
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (ccm_name == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get Auto CCM Name failed!");
    }
    ccm_status_t ccm_status;
    ret = rk_aiq_user_api2_ccm_QueryStatus(ctx, &ccm_status);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get CCM Auto CCM Name failed!");
    strcpy(ccm_name[0], ccm_status.accmStatus.sw_ccmC_illuUsed_name);

    IMGPROC_FUNC_EXIT

    return ret;
}

/*
**********************************************************
* 3-Dimensional Look Up Table
**********************************************************
*/
/*
*****************************
*
* Desc: set/get 3dlut mode
* Argument:
*   mode:
*     auto: auto 3dlut
*     manual: manual 3dlut
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setLut3dMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    /*
        rk_aiq_lut3d_attrib_t attr;
        memset(&attr, 0, sizeof(attr));
        IMGPROC_FUNC_ENTER
        if (mode >= OP_INVAL || mode < OP_AUTO) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "mode is invalid!");
        }
        ret = rk_aiq_user_api2_a3dlut_GetAttrib(ctx, &attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "set 3DLUT Mode failed in getting 3dlut attrib!");

        attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
        if (mode == OP_AUTO) {
            attr.mode = RK_AIQ_LUT3D_MODE_AUTO;
        } else if (mode == OP_MANUAL) {
            attr.mode = RK_AIQ_LUT3D_MODE_MANUAL;
        } else {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "Not supported mode!");
        }
        ret = rk_aiq_user_api2_a3dlut_SetAttrib(ctx, &attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "set 3DLUT Mode failed!");
        IMGPROC_FUNC_EXIT
    */

    LOGE("not support to call %s for current chip", __FUNCTION__);
    ret = XCAM_RETURN_ERROR_UNKNOWN;

    return ret;

}

XCamReturn rk_aiq_uapi2_getLut3dMode(const rk_aiq_sys_ctx_t* ctx, opMode_t* mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    /*
        rk_aiq_lut3d_attrib_t attr;
        memset(&attr, 0, sizeof(attr));
        IMGPROC_FUNC_ENTER
        ret = rk_aiq_user_api2_a3dlut_GetAttrib(ctx, &attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "get 3DLUT Mode failed!");
        if (attr.mode == RK_AIQ_LUT3D_MODE_AUTO) {
            *mode = OP_AUTO;
        } else if (attr.mode == RK_AIQ_LUT3D_MODE_MANUAL) {
            *mode = OP_MANUAL;
        } else {
            *mode = OP_INVAL;
        }
        IMGPROC_FUNC_EXIT
    */

#ifdef ISP_HW_V39
    IMGPROC_FUNC_ENTER

    lut3d_api_attrib_t lut3d_attr;
    ret = rk_aiq_user_api2_3dlut_GetAttrib(ctx, &lut3d_attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getCCMMode failed!");
    if (lut3d_attr.opMode == RK_AIQ_OP_MODE_AUTO) {
        *mode = OP_AUTO;
    } else if (lut3d_attr.opMode == RK_AIQ_OP_MODE_MANUAL) {
        *mode = OP_MANUAL;
    } else {
        *mode = OP_INVAL;
    }
    IMGPROC_FUNC_EXIT

#else
    LOGE("not support to call %s for current chip", __FUNCTION__);
    ret = XCAM_RETURN_ERROR_UNKNOWN;
#endif
    return ret;
}

/*
*****************************
*
* Desc: set manual 3d Look-up-table
*     this function is active for 3dlut is manual mode
        get 3d Look-up-table
* Argument:
*     mlut:
*           lut_r[729]
*           lut_g[729]
*           lut_b[729]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setM3dLut(const rk_aiq_sys_ctx_t* ctx, rk_aiq_lut3d_table_t* mlut)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    /*
        rk_aiq_lut3d_attrib_t attr;
        memset(&attr, 0, sizeof(attr));
        IMGPROC_FUNC_ENTER
        if ((ctx == NULL) || (mlut == NULL) ) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error, set manual 3d Look-up-table failed!");
        }

        ret = rk_aiq_user_api2_a3dlut_GetAttrib(ctx, &attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "set manual 3d Look-up-table in getting 3dlut attrib!");

        attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
        attr.mode = RK_AIQ_LUT3D_MODE_MANUAL;
        memcpy(attr.stManual.look_up_table_r, mlut->look_up_table_r, sizeof(unsigned short) * 729);
        memcpy(attr.stManual.look_up_table_g, mlut->look_up_table_g, sizeof(unsigned short) * 729);
        memcpy(attr.stManual.look_up_table_b, mlut->look_up_table_b, sizeof(unsigned short) * 729);
        ret = rk_aiq_user_api2_a3dlut_SetAttrib(ctx, &attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "set manual 3d Look-up-table failed!");
        IMGPROC_FUNC_EXIT
    */

    LOGE_A3DLUT("no support, please use 3dlut api rk_aiq_user_api2_3dlut_SetAttrib to set param");
    ret = XCAM_RETURN_ERROR_FAILED;

    return ret;
}

XCamReturn rk_aiq_uapi2_getM3dLut(const rk_aiq_sys_ctx_t* ctx, rk_aiq_lut3d_table_t* mlut)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    /*
        rk_aiq_lut3d_querry_info_t lut3d_querry_info;
        IMGPROC_FUNC_ENTER
        if ((ctx == NULL) || (mlut == NULL)) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get 3d Look-up-table failed!");
        }

        ret = rk_aiq_user_api2_a3dlut_Query3dlutInfo(ctx, &lut3d_querry_info);
        RKAIQ_IMGPROC_CHECK_RET(ret, "get 3d Look-up-table failed!");

        memcpy(mlut->look_up_table_r, lut3d_querry_info.look_up_table_r, sizeof(unsigned short) * 729);
        memcpy(mlut->look_up_table_g, lut3d_querry_info.look_up_table_g, sizeof(unsigned short) * 729);
        memcpy(mlut->look_up_table_b, lut3d_querry_info.look_up_table_b, sizeof(unsigned short) * 729);

        IMGPROC_FUNC_EXIT
    */
#if defined(ISP_HW_V39)
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (mlut == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get 3d Look-up-table failed!");
    }

    lut3d_status_t lut3d_status;
    ret = rk_aiq_user_api2_3dlut_QueryStatus(ctx, &lut3d_status);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get 3d Look-up-table failed!");

    memcpy(mlut->look_up_table_r, lut3d_status.stMan.dyn.meshGain.hw_lut3dC_lutR_val, sizeof(unsigned short) * 729);
    memcpy(mlut->look_up_table_g, lut3d_status.stMan.dyn.meshGain.hw_lut3dC_lutG_val, sizeof(unsigned short) * 729);
    memcpy(mlut->look_up_table_b, lut3d_status.stMan.dyn.meshGain.hw_lut3dC_lutB_val, sizeof(unsigned short) * 729);

    IMGPROC_FUNC_EXIT
#else
    LOGE("not support to call %s for current chip", __FUNCTION__);
    ret = XCAM_RETURN_ERROR_UNKNOWN;
#endif

    return ret;
}

/*
*****************************
*
* Desc: get auto 3d Look-up-table strength
*     this function is active for 3d Look-up-table is auto mode
* Argument:
*   alpha : range in [0, 1]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_getA3dLutStrth(const rk_aiq_sys_ctx_t* ctx, float* alpha)
{
    /*
        XCamReturn ret = XCAM_RETURN_NO_ERROR;
        rk_aiq_lut3d_querry_info_t lut3d_querry_info;
        IMGPROC_FUNC_ENTER
        if ((ctx == NULL) || (alpha == NULL)) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getAuto3DLUT 3d Look-up-table Strength failed!");
        }

        ret = rk_aiq_user_api2_a3dlut_Query3dlutInfo(ctx, &lut3d_querry_info);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getAuto3DLUT 3d Look-up-table Strength failed!");
        *alpha = lut3d_querry_info.alpha;
        IMGPROC_FUNC_EXIT
    */
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

/*
*****************************
*
* Desc: get auto 3d Look-up-table used lut name
*     this function is active for 3d Look-up-table is auto mode
* Argument:
*    name
*
*****************************
*/
XCamReturn rk_aiq_uapi2_getA3dLutName(const rk_aiq_sys_ctx_t* ctx, char* name)
{
    /*
        XCamReturn ret = XCAM_RETURN_NO_ERROR;
        rk_aiq_lut3d_querry_info_t lut3d_querry_info;
        IMGPROC_FUNC_ENTER
        if ((ctx == NULL) || (name == NULL)) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getAuto3DLUT 3d Look-up-table Name failed!");
        }

        ret = rk_aiq_user_api2_a3dlut_Query3dlutInfo(ctx, &lut3d_querry_info);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getAuto3DLUT 3d Look-up-table Name failed!");
        strcpy(name, lut3d_querry_info.name);
        IMGPROC_FUNC_EXIT
    */
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

#endif

#ifdef USE_NEWSTRUCT
XCamReturn rk_aiq_uapi2_setLdchEn(const rk_aiq_sys_ctx_t* ctx, bool en)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if RKAIQ_HAVE_LDCH_V22
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ldc_api_attrib_t attr;
    ret = rk_aiq_user_api2_ldc_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "Failed to get LDCH attrib.");

    attr.en     = en;
    attr.opMode = RK_AIQ_OP_MODE_AUTO;
    attr.bypass = 0;

    ret = rk_aiq_user_api2_ldc_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "Failed to set LDCH attrib.");

    IMGPROC_FUNC_EXIT
#else
    LOGE("not support to call %s for current chip", __FUNCTION__);
    ret = XCAM_RETURN_ERROR_UNKNOWN;
#endif
    return ret;
}

XCamReturn rk_aiq_uapi2_setLdchCorrectLevel(const rk_aiq_sys_ctx_t* ctx, int correctLevel)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if RKAIQ_HAVE_LDCH_V22
    ldc_api_attrib_t attr;

    ret = rk_aiq_user_api2_ldc_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "Failed to get LDCH attrib.");

    attr.tunning.autoGenMesh.sw_ldcT_correctStrg_val = correctLevel;

    ret = rk_aiq_user_api2_ldc_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "Failed to set LDCH attrib.");
#else
    LOGE("not support to call %s for current chip", __FUNCTION__);
    ret = XCAM_RETURN_ERROR_UNKNOWN;
#endif
    return ret;
}

XCamReturn rk_aiq_uapi2_setLdchLdcvEn(const rk_aiq_sys_ctx_t* ctx, bool en) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if RKAIQ_HAVE_LDCV
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ldc_api_attrib_t attr;
    ret = rk_aiq_user_api2_ldc_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "Failed to get LDCH attrib.");

    attr.en     = en;
    attr.opMode = RK_AIQ_OP_MODE_AUTO;
    attr.bypass = 0;

    attr.tunning.enMode = LDC_LDCH_LDCV_EN;

    ret = rk_aiq_user_api2_ldc_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "Failed to set LDCH attrib.");

    IMGPROC_FUNC_EXIT
#else
    LOGE("not support to call %s for current chip", __FUNCTION__);
    ret = XCAM_RETURN_ERROR_UNKNOWN;
#endif
    return ret;
}

XCamReturn rk_aiq_uapi2_setLdchLdcvCorrectLevel(const rk_aiq_sys_ctx_t* ctx, int correctLevel) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if RKAIQ_HAVE_LDCV
    ldc_api_attrib_t attr;

    ret = rk_aiq_user_api2_ldc_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "Failed to get LDCH attrib.");

    attr.tunning.autoGenMesh.sw_ldcT_correctStrg_val = correctLevel;

    ret = rk_aiq_user_api2_ldc_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "Failed to set LDCH attrib.");
#else
    LOGE("not support to call %s for current chip", __FUNCTION__);
    ret = XCAM_RETURN_ERROR_UNKNOWN;
#endif
    return ret;
}

#endif
#ifdef USE_IMPLEMENT_C
XCamReturn rk_aiq_uapi2_setFecEn(const rk_aiq_sys_ctx_t* ctx, bool en)
{
    /*
        XCamReturn ret = XCAM_RETURN_NO_ERROR;
        IMGPROC_FUNC_ENTER
        if (ctx == NULL) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
        }
        rk_aiq_fec_attrib_t fecAttr;
        ret = rk_aiq_user_api2_afec_GetAttrib(ctx, &fecAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "get fec attrib failed!");
        fecAttr.en = en;
        ret = rk_aiq_user_api2_afec_SetAttrib(ctx, fecAttr);
        IMGPROC_FUNC_EXIT
    */
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_setFecBypass(const rk_aiq_sys_ctx_t* ctx, bool bypass)
{
    /*
        XCamReturn ret = XCAM_RETURN_NO_ERROR;
        IMGPROC_FUNC_ENTER
        if (ctx == NULL) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
        }
        rk_aiq_fec_attrib_t fecAttr;
        ret = rk_aiq_user_api2_afec_GetAttrib(ctx, &fecAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "get fec attrib failed!");
        fecAttr.bypass = bypass;
        ret = rk_aiq_user_api2_afec_SetAttrib(ctx, fecAttr);
        IMGPROC_FUNC_EXIT
    */
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_setFecCorrectLevel(const rk_aiq_sys_ctx_t* ctx, int correctLevel)
{
    /*
        XCamReturn ret = XCAM_RETURN_NO_ERROR;
        IMGPROC_FUNC_ENTER
        if (ctx == NULL) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
        }
        rk_aiq_fec_attrib_t fecAttr;
        ret = rk_aiq_user_api2_afec_GetAttrib(ctx, &fecAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "get fec attrib failed!");
        fecAttr.correct_level = correctLevel;
        ret = rk_aiq_user_api2_afec_SetAttrib(ctx, fecAttr);
        IMGPROC_FUNC_EXIT
    */
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_setFecCorrectDirection(const rk_aiq_sys_ctx_t* ctx,
        const fec_correct_direction_t direction)
{
    /*
        XCamReturn ret = XCAM_RETURN_NO_ERROR;
        IMGPROC_FUNC_ENTER
        if (ctx == NULL) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
        }
        rk_aiq_fec_attrib_t fecAttr;
        ret = rk_aiq_user_api2_afec_GetAttrib(ctx, &fecAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "get fec attrib failed!");
        fecAttr.direction = direction;
        ret = rk_aiq_user_api2_afec_SetAttrib(ctx, fecAttr);
        IMGPROC_FUNC_EXIT
    */
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_setFecCorrectMode(const rk_aiq_sys_ctx_t* ctx,
        const fec_correct_mode_t mode)
{
    /*
        XCamReturn ret = XCAM_RETURN_NO_ERROR;
        IMGPROC_FUNC_ENTER
        if (ctx == NULL) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
        }
        rk_aiq_fec_attrib_t fecAttr;
        ret = rk_aiq_user_api2_afec_GetAttrib(ctx, &fecAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "get fec attrib failed!");
        fecAttr.mode = mode;
        ret = rk_aiq_user_api2_afec_SetAttrib(ctx, fecAttr);
        IMGPROC_FUNC_EXIT
    */
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi2_setMirrorFlip(const rk_aiq_sys_ctx_t* ctx, bool mirror, bool flip,
                                      int skip_frm_cnt) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    bool set_btnr_bypass = false;
    btnr_status_t btnr_sta;
    btnr_api_attrib_t btnr_attr;
    ret = rk_aiq_user_api2_btnr_GetAttrib(ctx, &btnr_attr);
    if (btnr_sta.en && !btnr_sta.bypass) {
        btnr_attr.bypass = true;
        rk_aiq_user_api2_btnr_SetAttrib(ctx, &btnr_attr);
        int wait_param_effect_sleep_cnt = 13;
        do {
            if (wait_param_effect_sleep_cnt == 0) {
                LOGW("BTNR bypass no current effect before set mirror/flip");
                break;
            }
            usleep(5 * 1000);
            rk_aiq_user_api2_btnr_QueryStatus(ctx, &btnr_sta);
            wait_param_effect_sleep_cnt--;
        } while((btnr_sta.en && !btnr_sta.bypass));
        set_btnr_bypass = true;
    }
    ret = AiqManager_setMirrorFlip(ctx->_rkAiqManager, mirror, flip, skip_frm_cnt);
    if (set_btnr_bypass) {
        rk_aiq_user_api2_btnr_GetAttrib(ctx, &btnr_attr);
        btnr_attr.bypass = false;
        rk_aiq_user_api2_btnr_SetAttrib(ctx, &btnr_attr);
    }
    return ret;
}

XCamReturn rk_aiq_uapi2_getMirrorFlip(const rk_aiq_sys_ctx_t* ctx, bool* mirror, bool* flip) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL || mirror == NULL || flip == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    return AiqManager_getMirrorFlip(ctx->_rkAiqManager, mirror, flip);
}

#endif
#ifdef USE_NEWSTRUCT
/*
**********************************************************
* Color Process
**********************************************************
*/

/*
*****************************
*
* Desc: Adjust image contrast level
* Argument:
*    level: contrast level, [0, 255]
*****************************
*/
XCamReturn rk_aiq_uapi2_setContrast(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER

    LOGD("setContrast enter, level=%d\n", level);
    if ((int)level < 0 || level > 255) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "level out of range, setContrast failed!");
    }
#if RKAIQ_HAVE_DEHAZE_V14
    bool update_attr = false;
    dehaze_api_attrib_t attr;
    memset(&attr, 0, sizeof(dehaze_api_attrib_t));
    ret = rk_aiq_user_api2_dehaze_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set contrast(dehaze GetAttrib) failed!");
    if (attr.opMode == RK_AIQ_OP_MODE_MANUAL || attr.en == false) {
        attr.en = true;
        attr.opMode = RK_AIQ_OP_MODE_AUTO;
        update_attr = true;
        LOGW_ADEHAZE("%s is only supported in AUTO mode.", __FUNCTION__);
    }

    for (int i = 0; i < DEHAZE_ISO_STEP_MAX; i++) {
        if (attr.stAuto.dyn[i].sw_dhazT_work_mode != dhaz_enhance_mode) {
            update_attr = true;
            attr.stAuto.dyn[i].sw_dhazT_work_mode = dhaz_enhance_mode;
        }
    }
    if (update_attr) {
        ret = rk_aiq_user_api2_dehaze_SetAttrib(ctx, &attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "set contrast(dehaze SetAttrib) failed!");
    }

    adehaze_strength_t ctrl;
    memset(&ctrl, 0, sizeof(adehaze_strength_t));
    ret = rk_aiq_user_api2_getDehazeEnhanceStrth(ctx, &ctrl);
    level /= 2.55;
    ctrl.MEnhanceStrth = level;
    ret = rk_aiq_user_api2_setDehazeEnhanceStrth(ctx, ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set contrast(setMEnhanceStrth) failed!");

#elif RKAIQ_HAVE_ENHANCE_V10
    enh_api_attrib_t attr;
    memset(&attr, 0, sizeof(enh_api_attrib_t));
    ret = rk_aiq_user_api2_enh_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set contrast(enh GetAttrib) failed!");
    if (attr.opMode == RK_AIQ_OP_MODE_MANUAL || attr.en == false || attr.bypass) {
        attr.en = true;
        attr.bypass = false;
        attr.opMode = RK_AIQ_OP_MODE_AUTO;
        LOGW_ADEHAZE("%s is only supported in AUTO mode.", __FUNCTION__);
        ret = rk_aiq_user_api2_enh_SetAttrib(ctx, &attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "set contrast(enh SetAttrib) failed!");
    }

    aenh_strength_t strg;
    memset(&strg, 0, sizeof(aenh_strength_t));
    ret = rk_aiq_user_api2_enh_GetEnhanceStrth(ctx, &strg);
    level /= 2.55;
    strg.MEnhanceStrth = level;
    ret = rk_aiq_user_api2_enh_SetEnhanceStrth(ctx, &strg);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set contrast(setEnhanceStrth) failed!");

#else
    cp_api_attrib_t attrib;
    ret = rk_aiq_user_api2_cp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAttrib error,set contrast failed!");
    if(attrib.opMode == RK_AIQ_OP_MODE_AUTO)
        attrib.stAuto.sta.contrast = level;
    else
        attrib.stMan.sta.contrast = level;
    ret = rk_aiq_user_api2_cp_SetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set contrast failed!");
#endif
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_getContrast(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER

#if RKAIQ_HAVE_DEHAZE_V14
    adehaze_strength_t ctrl;
    memset(&ctrl, 0, sizeof(adehaze_strength_t));
    ret = rk_aiq_user_api2_getDehazeEnhanceStrth(ctx, &ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get contrast(getMEnhanceStrth) failed!");
    *level = ctrl.MEnhanceStrth * 2.55;

#elif RKAIQ_HAVE_ENHANCE_V10
    aenh_strength_t strg;
    memset(&strg, 0, sizeof(aenh_strength_t));
    ret = rk_aiq_user_api2_enh_GetEnhanceStrth(ctx, &strg);
    *level = strg.MEnhanceStrth * 2.55;
    RKAIQ_IMGPROC_CHECK_RET(ret, "get contrast(getEnhanceStrth) failed!");

#else
    cp_api_attrib_t attrib;
    if (level == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getContrast failed!");
    }
    ret = rk_aiq_user_api2_cp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get contrast failed!");
    if(attrib.opMode == RK_AIQ_OP_MODE_AUTO)
        *level = attrib.stAuto.sta.contrast;
    else
        *level = attrib.stMan.sta.contrast;
#endif
    IMGPROC_FUNC_EXIT

    return ret;
}

/*
*****************************
*
* Desc: Adjust image brightness level
* Argument:
*    level: brightness level, [0, 255]
*****************************
*/
XCamReturn rk_aiq_uapi2_setBrightness(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, set brightness failed!");
    }

    LOGD("setBrightness enter, level=%d\n", level);
    if ((int)level < 0 || level > 255) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "level out of range, set brightness failed!");
    }
#ifdef ISP_HW_V35
    bool update_attr = false;
    hsv_api_attrib_t attrib;
    memset(&attrib, 0, sizeof(hsv_api_attrib_t));
    ret = rk_aiq_user_api2_hsv_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set brightness (hsv GetAttrib) failed!");
    if (attrib.opMode == RK_AIQ_OP_MODE_MANUAL && attrib.en) {
        attrib.opMode = RK_AIQ_OP_MODE_AUTO;
        update_attr = true;
        LOGW_AHSV("%s is only supported in AUTO mode.", __FUNCTION__);
    } else if (attrib.en == false) {
        attrib.en = true;
        attrib.opMode = RK_AIQ_OP_MODE_AUTO;
        update_attr = true;
        LOGW_AHSV("%s is only supported in AUTO mode.", __FUNCTION__);
    }

    if (update_attr) {
        ret = rk_aiq_user_api2_hsv_SetAttrib(ctx, &attrib);
        RKAIQ_IMGPROC_CHECK_RET(ret, "set brightness (hsv GetAttrib) failed!");
    }

    ahsv_valOffset_t ctrl;
    memset(&ctrl, 0, sizeof(ahsv_valOffset_t));
    ret = rk_aiq_user_api2_getHsvValOffset(ctx, &ctrl);
    ctrl.en = true;
    ctrl.valOffset = (level - 127) << 3;
    ret = rk_aiq_user_api2_setHsvValOffset(ctx, ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set brightness(setHsvValOffset) failed!");

#else
    cp_api_attrib_t attrib;

    ret = rk_aiq_user_api2_cp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAttrib error,set brightness failed!");
    if(attrib.opMode == RK_AIQ_OP_MODE_AUTO)
        attrib.stAuto.sta.brightness = level;
    else
        attrib.stMan.sta.brightness = level;
    ret = rk_aiq_user_api2_cp_SetAttrib(ctx, &attrib);
#endif
    RKAIQ_IMGPROC_CHECK_RET(ret, "set brightness failed!");
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_getBrightness(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER
    if (level == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get brightness failed!");
    }
#if defined(ISP_HW_V35)
    ahsv_valOffset_t ctrl;
    memset(&ctrl, 0, sizeof(ahsv_valOffset_t));
    ret = rk_aiq_user_api2_getHsvValOffset(ctx, &ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get brightness(getHsvValOffset) failed!");
    *level = ctrl.en ? ((ctrl.valOffset + 4) >> 3) + 127 : 0;
#else
    cp_api_attrib_t attrib;
    ret = rk_aiq_user_api2_cp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get brightness failed!");
    if(attrib.opMode == RK_AIQ_OP_MODE_AUTO)
        *level = attrib.stAuto.sta.brightness;
    else
        *level = attrib.stMan.sta.brightness;
#endif
    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: Adjust image saturation level
* Argument:
*    level: saturation level, [0, 255]
*****************************
*/
XCamReturn rk_aiq_uapi2_setSaturation(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, set saturation failed!");
    }

    LOGD("setSaturation enter, level=%d\n", level);
    if ((int)level < 0 || level > 255) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "level out of range, set saturation failed!");
    }
#if defined(ISP_HW_V35)
    bool update_attr = false;
    hsv_api_attrib_t attrib;
    memset(&attrib, 0, sizeof(hsv_api_attrib_t));
    ret = rk_aiq_user_api2_hsv_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set saturation (hsv GetAttrib) failed!");
    if (attrib.opMode == RK_AIQ_OP_MODE_MANUAL && attrib.en) {
        attrib.opMode = RK_AIQ_OP_MODE_AUTO;
        update_attr = true;
        LOGW_AHSV("%s is only supported in AUTO mode.", __FUNCTION__);
    } else if (attrib.en == false) {
        attrib.en = true;
        attrib.opMode = RK_AIQ_OP_MODE_AUTO;
        update_attr = true;
        LOGW_AHSV("%s is only supported in AUTO mode.", __FUNCTION__);
    }

    if (update_attr) {
        ret = rk_aiq_user_api2_hsv_SetAttrib(ctx, &attrib);
        RKAIQ_IMGPROC_CHECK_RET(ret, "set saturation (hsv GetAttrib) failed!");
    }

    ahsv_satStrg_t ctrl;
    memset(&ctrl, 0, sizeof(ahsv_satStrg_t));
    ret = rk_aiq_user_api2_getHsvSatStrth(ctx, &ctrl);
    ctrl.en = true;
    ctrl.satStrg = level / 127.0;
    ret = rk_aiq_user_api2_setHsvSatStrth(ctx, ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set saturation(setHsvSatStrth) failed!");

#else
    cp_api_attrib_t attrib;
    ret = rk_aiq_user_api2_cp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAttrib error,set saturation failed!");
    if(attrib.opMode == RK_AIQ_OP_MODE_AUTO)
        attrib.stAuto.sta.saturation = level;
    else
        attrib.stMan.sta.saturation = level;
    ret = rk_aiq_user_api2_cp_SetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set saturation failed!");
#endif
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getSaturation(const rk_aiq_sys_ctx_t* ctx, unsigned int* level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (level == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get saturation failed!");
    }
#if defined(ISP_HW_V35)
    ahsv_satStrg_t ctrl;
    memset(&ctrl, 0, sizeof(ahsv_satStrg_t));
    ret = rk_aiq_user_api2_getHsvSatStrth(ctx, &ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get saturation(getHsvSatStrth) failed!");
    *level = ctrl.en ? ctrl.satStrg * 127.0 : 1;
#else
    cp_api_attrib_t attrib;
    
    ret = rk_aiq_user_api2_cp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get saturation failed!");
    if(attrib.opMode == RK_AIQ_OP_MODE_AUTO)
        *level = attrib.stAuto.sta.saturation;
    else
        *level = attrib.stMan.sta.saturation;
#endif
    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: Adjust image hue level
* Argument:
*    level: hue level, [0, 255]
*****************************
*/
XCamReturn rk_aiq_uapi2_setHue(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, set hue failed!");
    }

    LOGD("setHue enter, level=%d\n", level);
    if ((int)level < 0 || level > 255) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "level out of range, set hue failed!");
    }
#if  defined(ISP_HW_V35)
    bool update_attr = false;
    hsv_api_attrib_t attrib;
    memset(&attrib, 0, sizeof(hsv_api_attrib_t));
    ret = rk_aiq_user_api2_hsv_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set hue (hsv GetAttrib) failed!");
    if (attrib.opMode == RK_AIQ_OP_MODE_MANUAL && attrib.en) {
        attrib.opMode = RK_AIQ_OP_MODE_AUTO;
        update_attr = true;
        LOGW_AHSV("%s is only supported in AUTO mode.", __FUNCTION__);
    } else if (attrib.en == false) {
        attrib.en = true;
        attrib.opMode = RK_AIQ_OP_MODE_AUTO;
        update_attr = true;
        LOGW_AHSV("%s is only supported in AUTO mode.", __FUNCTION__);
    }

    if (update_attr) {
        ret = rk_aiq_user_api2_hsv_SetAttrib(ctx, &attrib);
        RKAIQ_IMGPROC_CHECK_RET(ret, "set hue (hsv GetAttrib) failed!");
    }

    ahsv_hueOffset_t ctrl;
    memset(&ctrl, 0, sizeof(ahsv_hueOffset_t));
    ret = rk_aiq_user_api2_getHsvHueOffset(ctx, &ctrl);
    ctrl.en = true;
    ctrl.hueOffset = (127 - level) * 2;
    ret = rk_aiq_user_api2_setHsvHueOffset(ctx, ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set hue(setHsvHueOffset) failed!");

#else
    cp_api_attrib_t attrib;

    ret = rk_aiq_user_api2_cp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAttrib error,set hue failed!");
    if(attrib.opMode == RK_AIQ_OP_MODE_AUTO)
        attrib.stAuto.sta.hue = level;
    else
        attrib.stMan.sta.hue = level;
    ret = rk_aiq_user_api2_cp_SetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set hue failed!");
#endif
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getHue(const rk_aiq_sys_ctx_t* ctx, unsigned int* level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (level == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get hue failed!");
    }
#if defined(ISP_HW_V35)
    ahsv_hueOffset_t ctrl;
    memset(&ctrl, 0, sizeof(ahsv_hueOffset_t));
    ret = rk_aiq_user_api2_getHsvHueOffset(ctx, &ctrl);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get hue(getHsvHueOffset) failed!");
    *level = ctrl.en ? 127 - ((ctrl.hueOffset + 1) >> 1) : 0;
#else
    cp_api_attrib_t attrib;
    ret = rk_aiq_user_api2_cp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get hue failed!");
    if(attrib.opMode == RK_AIQ_OP_MODE_AUTO)
        *level = attrib.stAuto.sta.hue;
    else
        *level = attrib.stMan.sta.hue;
#endif
    IMGPROC_FUNC_EXIT
    return ret;
}
#endif
#ifdef USE_IMPLEMENT_C
/*
**********************************************************
* Image Effect
**********************************************************
*/

/*
*****************************
*
* Desc: Set Color Mode
* Argument:
*    mode:
*        [0  NONE
*         1  BW
*         2  NEGATIVE
*         3  SEPIA
*         4  EMBOSS
*         5  SKETCH
*         6  SHARPEN]
* Note: rv1106 only support {0,1}
*****************************
*/
XCamReturn rk_aiq_uapi2_setColorMode(const rk_aiq_sys_ctx_t* ctx, unsigned int mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    /*
        IMGPROC_FUNC_ENTER

        LOGD("setColorMode enter, mode=%d\n", mode);
        aie_attrib_t attrib;
        if ((int)mode < 0 || mode > 6) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "mode out of range, setColorMode failed!");
        }

        ret = rk_aiq_user_api2_aie_GetAttrib(ctx, &attrib);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getAttrib error,set ColorMode failed!");
        attrib.mode = (rk_aiq_ie_effect_t)mode;
        ret = rk_aiq_user_api2_aie_SetAttrib(ctx, &attrib);
        RKAIQ_IMGPROC_CHECK_RET(ret, "set ColorMode failed!");
        IMGPROC_FUNC_EXIT
    */

    IMGPROC_FUNC_ENTER
    ie_api_attrib_t attr;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, setColorMode failed!");
    }
    if ((int)mode < 0 || mode > 6) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "mode out of range, setColorMode failed!");
    }
    ret = rk_aiq_user_api2_ie_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get ColorMode failed!");
    if (mode == 1) {
        attr.en = true;
    } else if (mode == 0) {
        attr.en = false;
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        RKAIQ_IMGPROC_CHECK_RET(ret, "RK3576 only support 0 or 1, mode %u is no support!", mode);
    }
    ret = rk_aiq_user_api2_ie_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set ColorMode failed!");
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_getColorMode(const rk_aiq_sys_ctx_t* ctx, unsigned int *mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    /*
        IMGPROC_FUNC_ENTER
        aie_attrib_t attrib;
        if (mode == NULL || ctx == NULL) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getColorMode failed!");
        }
        ret = rk_aiq_user_api2_aie_GetAttrib(ctx, &attrib);
        RKAIQ_IMGPROC_CHECK_RET(ret, "get ColorMode failed!");
        *mode = (unsigned int)attrib.mode;
        IMGPROC_FUNC_EXIT
    */

    IMGPROC_FUNC_ENTER
    ie_api_attrib_t attr;
    if (mode == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getColorMode failed!");
    }
    ret = rk_aiq_user_api2_ie_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get ColorMode failed!");
    if (attr.en) {
        *mode = 1;
    } else {
        *mode = 0;
    }
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_setGrayMode(const rk_aiq_sys_ctx_t* ctx, rk_aiq_gray_mode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(ctx);
    for (int i = 0; i < ctx_array.num; i++) {
        ret = AiqCore_setGrayMode(ctx_array.ctx[i]->_analyzer, mode);
    }

    return ret;
}

rk_aiq_gray_mode_t rk_aiq_uapi2_getGrayMode(const rk_aiq_sys_ctx_t* ctx)
{
    const rk_aiq_sys_ctx_t* sys_ctx = rk_aiq_user_api2_common_getSysCtx(ctx);
    return AiqCore_getGrayMode(sys_ctx->_analyzer);
}
#endif

#ifdef USE_NEWSTRUCT
/*
**********************************************************
* Color Space
**********************************************************
*/

/*
*****************************
*
* Desc: Set Color Space
* Argument:
*    Cspace:
*        [0  BT.601 FULL
*         1  BT.601 LIMIT
*         2  BT.709 FULL
*         3  BT.709 LIMIT
*         253 OTHER FULL
*         254 OTHER LIMIT
*         255 OTHER]
*****************************
*/
XCamReturn rk_aiq_uapi2_setColorSpace(const rk_aiq_sys_ctx_t* ctx, int Cspace)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER

    LOGD("setColorSpace enter, Cspace=%d\n", Cspace);
    csm_api_attrib_t csm_attrib;
    cgc_api_attrib_t cgc_attrib;
    memset(&csm_attrib, 0, sizeof(csm_api_attrib_t));
    memset(&cgc_attrib, 0, sizeof(cgc_api_attrib_t));
    if (Cspace < 0 || Cspace > 255) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "mode out of range, setColorSpace failed!");
    }

    XCamReturn ret1 = XCAM_RETURN_NO_ERROR;
    XCamReturn ret2 = XCAM_RETURN_NO_ERROR;

    ret1 = rk_aiq_user_api2_csm_GetAttrib(ctx, &csm_attrib);
    ret2 = rk_aiq_user_api2_cgc_GetAttrib(ctx, &cgc_attrib);
    if (ret1 != XCAM_RETURN_NO_ERROR || ret2 != XCAM_RETURN_NO_ERROR) {
        LOGE("getAttrib error,set ColorSpace failed!");
        return XCAM_RETURN_ERROR_FAILED;
    }

    csm_attrib.opMode = RK_AIQ_OP_MODE_MANUAL;
    cgc_attrib.opMode = RK_AIQ_OP_MODE_MANUAL;
    switch (Cspace) {
    case CSPACE_MODE_BT601_FULL:
        csm_attrib.stMan.sta.hw_csmT_full_range = true;
        csm_attrib.stMan.sta.hw_csmT_y_offset = 0;
        csm_attrib.stMan.sta.hw_csmT_c_offset = 0;
        csm_attrib.stMan.sta.sw_csmT_coeff[0] = 0.299;
        csm_attrib.stMan.sta.sw_csmT_coeff[1] = 0.587;
        csm_attrib.stMan.sta.sw_csmT_coeff[2] = 0.114;
        csm_attrib.stMan.sta.sw_csmT_coeff[3] = -0.169;
        csm_attrib.stMan.sta.sw_csmT_coeff[4] = -0.331;
        csm_attrib.stMan.sta.sw_csmT_coeff[5] = 0.5;
        csm_attrib.stMan.sta.sw_csmT_coeff[6] = 0.5;
        csm_attrib.stMan.sta.sw_csmT_coeff[7] = -0.419;
        csm_attrib.stMan.sta.sw_csmT_coeff[8] = -0.081;
        cgc_attrib.stMan.sta.cgc_ratio_en = 0;
        cgc_attrib.stMan.sta.cgc_yuv_limit = 0;
        break;
    case CSPACE_MODE_BT601_LIMIT:
        csm_attrib.stMan.sta.hw_csmT_full_range = true;
        csm_attrib.stMan.sta.hw_csmT_y_offset = 0;
        csm_attrib.stMan.sta.hw_csmT_c_offset = 0;
        csm_attrib.stMan.sta.sw_csmT_coeff[0] = 0.257;
        csm_attrib.stMan.sta.sw_csmT_coeff[1] = 0.504;
        csm_attrib.stMan.sta.sw_csmT_coeff[2] = 0.098;
        csm_attrib.stMan.sta.sw_csmT_coeff[3] = -0.148;
        csm_attrib.stMan.sta.sw_csmT_coeff[4] = -0.291;
        csm_attrib.stMan.sta.sw_csmT_coeff[5] = 0.439;
        csm_attrib.stMan.sta.sw_csmT_coeff[6] = 0.439;
        csm_attrib.stMan.sta.sw_csmT_coeff[7] = -0.368;
        csm_attrib.stMan.sta.sw_csmT_coeff[8] = -0.071;
        cgc_attrib.stMan.sta.cgc_ratio_en = 0;
        cgc_attrib.stMan.sta.cgc_yuv_limit = 1;
        break;
    case CSPACE_MODE_BT709_FULL:
        csm_attrib.stMan.sta.hw_csmT_full_range = true;
        csm_attrib.stMan.sta.hw_csmT_y_offset = 0;
        csm_attrib.stMan.sta.hw_csmT_c_offset = 0;
        csm_attrib.stMan.sta.sw_csmT_coeff[0] = 0.2126;
        csm_attrib.stMan.sta.sw_csmT_coeff[1] = 0.7152;
        csm_attrib.stMan.sta.sw_csmT_coeff[2] = 0.0722;
        csm_attrib.stMan.sta.sw_csmT_coeff[3] = -0.1146;
        csm_attrib.stMan.sta.sw_csmT_coeff[4] = -0.3854;
        csm_attrib.stMan.sta.sw_csmT_coeff[5] = 0.5;
        csm_attrib.stMan.sta.sw_csmT_coeff[6] = 0.5;
        csm_attrib.stMan.sta.sw_csmT_coeff[7] = -0.4542;
        csm_attrib.stMan.sta.sw_csmT_coeff[8] = -0.0468;
        cgc_attrib.stMan.sta.cgc_ratio_en = 0;
        cgc_attrib.stMan.sta.cgc_yuv_limit = 0;
        break;
    case CSPACE_MODE_BT709_LIMIT:
        csm_attrib.stMan.sta.hw_csmT_full_range = true;
        csm_attrib.stMan.sta.hw_csmT_y_offset = 0;
        csm_attrib.stMan.sta.hw_csmT_c_offset = 0;
        csm_attrib.stMan.sta.sw_csmT_coeff[0] = 0.1826;
        csm_attrib.stMan.sta.sw_csmT_coeff[1] = 0.6142;
        csm_attrib.stMan.sta.sw_csmT_coeff[2] = 0.0620;
        csm_attrib.stMan.sta.sw_csmT_coeff[3] = -0.1006;
        csm_attrib.stMan.sta.sw_csmT_coeff[4] = -0.3386;
        csm_attrib.stMan.sta.sw_csmT_coeff[5] = 0.4392;
        csm_attrib.stMan.sta.sw_csmT_coeff[6] = 0.4392;
        csm_attrib.stMan.sta.sw_csmT_coeff[7] = -0.3989;
        csm_attrib.stMan.sta.sw_csmT_coeff[8] = -0.0403;
        cgc_attrib.stMan.sta.cgc_ratio_en = 0;
        cgc_attrib.stMan.sta.cgc_yuv_limit = 1;
        break;
    case CSPACE_MODE_OTHER_FULL:
        csm_attrib.stMan.sta.hw_csmT_full_range = true;
        cgc_attrib.stMan.sta.cgc_ratio_en = 0;
        cgc_attrib.stMan.sta.cgc_yuv_limit = 0;
        break;
    case CSPACE_MODE_OTHER_LIMIT:
        csm_attrib.stMan.sta.hw_csmT_full_range = true;
        cgc_attrib.stMan.sta.cgc_ratio_en = 0;
        cgc_attrib.stMan.sta.cgc_yuv_limit = 1;
        break;
    default:
        break;
    }
    ret1 = rk_aiq_user_api2_csm_SetAttrib(ctx, &csm_attrib);
    ret2 = rk_aiq_user_api2_cgc_SetAttrib(ctx, &cgc_attrib);
    if (ret1 != XCAM_RETURN_NO_ERROR || ret2 != XCAM_RETURN_NO_ERROR) {
        LOGE("set ColorSpace failed!");
        return XCAM_RETURN_ERROR_FAILED;
    }
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_getColorSpace(const rk_aiq_sys_ctx_t* ctx, int *Cspace)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER
    csm_api_attrib_t csm_attrib;
    cgc_api_attrib_t cgc_attrib;
    memset(&csm_attrib, 0, sizeof(csm_api_attrib_t));
    memset(&cgc_attrib, 0, sizeof(cgc_api_attrib_t));
    if (Cspace == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getColorSpace failed!");
    }
    XCamReturn ret1 = XCAM_RETURN_NO_ERROR;
    XCamReturn ret2 = XCAM_RETURN_NO_ERROR;

    ret1 = rk_aiq_user_api2_csm_GetAttrib(ctx, &csm_attrib);
    ret2 = rk_aiq_user_api2_cgc_GetAttrib(ctx, &cgc_attrib);
    if (ret1 != XCAM_RETURN_NO_ERROR || ret2 != XCAM_RETURN_NO_ERROR) {
        LOGE("getAttrib error,get ColorSpace failed!");
        return XCAM_RETURN_ERROR_FAILED;
    }

    static csm_params_static_t g_csm_601fa_def = {
        .hw_csmT_full_range = true,
        .hw_csmT_y_offset = 0,
        .hw_csmT_c_offset = 0,
        .sw_csmT_coeff = {
            0.299, 0.587, 0.114,
            -0.169, -0.331, 0.5,
            0.5, -0.419, -0.081
        }
    };
    static csm_params_static_t g_csm_601fm_def = {
        .hw_csmT_full_range = true,
        .hw_csmT_y_offset = 0,
        .hw_csmT_c_offset = 0,
        .sw_csmT_coeff = {
            0.299, 0.587, 0.114,
            -0.169, -0.331, 0.5,
            0.5, -0.419, -0.081
        }
    };
    static csm_params_static_t g_csm_601l_def = {
        .hw_csmT_full_range = true,
        .hw_csmT_y_offset = 0,
        .hw_csmT_c_offset = 0,
        .sw_csmT_coeff = {
            0.257, 0.504, 0.098,
            -0.148, -0.291, 0.439,
            0.439, -0.368, -0.071
        }
    };
    static csm_params_static_t g_csm_709fa_def = {
        .hw_csmT_full_range = true,
        .hw_csmT_y_offset = 0,
        .hw_csmT_c_offset = 0,
        .sw_csmT_coeff = {
            0.2126, 0.7152, 0.0722,
            -0.1146, -0.3854, 0.5000,
            0.5000, -0.4542, -0.0468,
        }
    };
    static csm_params_static_t g_csm_709fm_def = {
        .hw_csmT_full_range = true,
        .hw_csmT_y_offset = 0,
        .hw_csmT_c_offset = 0,
        .sw_csmT_coeff = {
            0.2126, 0.7152, 0.0722,
            -0.1146, -0.3854, 0.5000,
            0.5000, -0.4542, -0.0468,
        }
    };
    static csm_params_static_t g_csm_709l_def = {
        .hw_csmT_full_range = true,
        .hw_csmT_y_offset = 0,
        .hw_csmT_c_offset = 0,
        .sw_csmT_coeff = {
            0.1826, 0.6142, 0.0620,
            -0.1006, -0.3386, 0.4392,
            0.4392, -0.3989, -0.0403
        }
    };
    if (cgc_attrib.stMan.sta.cgc_yuv_limit == true) {
        if (!memcmp(&csm_attrib.stMan.sta, &g_csm_601l_def, sizeof(g_csm_601l_def)))
            *Cspace = CSPACE_MODE_BT601_LIMIT;
        else if (!memcmp(&csm_attrib.stMan.sta, &g_csm_709l_def, sizeof(g_csm_709l_def)))
            *Cspace = CSPACE_MODE_BT709_LIMIT;
        else
            *Cspace = CSPACE_MODE_OTHER_LIMIT;
    } else {
        if (!memcmp(&csm_attrib.stMan.sta, &g_csm_601fa_def, sizeof(g_csm_601fa_def)) ||
                !memcmp(&csm_attrib.stMan.sta, &g_csm_601fm_def, sizeof(g_csm_601fm_def)))
            *Cspace = CSPACE_MODE_BT601_FULL;
        else if (!memcmp(&csm_attrib.stMan.sta, &g_csm_709fa_def, sizeof(g_csm_709fa_def)) ||
                 !memcmp(&csm_attrib.stMan.sta, &g_csm_709fm_def, sizeof(g_csm_709fm_def)))
            *Cspace = CSPACE_MODE_BT709_FULL;
        else
            *Cspace = CSPACE_MODE_OTHER_FULL;
    }

    IMGPROC_FUNC_EXIT
    return ret;
}
#endif
RKAIQ_END_DECLARE
