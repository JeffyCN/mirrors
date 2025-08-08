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

#ifdef ISP_HW_V39
#include "rk_aiq_user_api2_isp39.h"
#elif  defined(ISP_HW_V33)
#include "rk_aiq_user_api2_isp33.h"
#elif  defined(ISP_HW_V32)
#include "rk_aiq_user_api2_isp32.h"
#elif defined(ISP_HW_V35)
#include "rk_aiq_user_api2_isp35.h"
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

/*
*****************************
* Desc: set ae mode
* Argument:
*   mode contains: auto & manual
*
*****************************
*/
XCamReturn rk_aiq_uapi_setAeLock(
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
XCamReturn rk_aiq_uapi_setExpMode(
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
XCamReturn rk_aiq_uapi_getExpMode(
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

XCamReturn rk_aiq_uapi_setManualExp(const rk_aiq_sys_ctx_t* ctx, float gain, float time)
{
    IMGPROC_FUNC_ENTER
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }
    if (gain < 0.0f || time < 0.0f) {
        ret = XCAM_RETURN_NO_ERROR;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }
    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setManualExp failed in getExpSwAttr!");
    if (isHDRmode(ctx)) {
        expSwAttr.commCtrl.sw_aeT_opt_mode = RK_AIQ_OP_MODE_MANUAL;
        expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_en = true;
        expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_en = true;
        expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_val[0] = gain;
        expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_val[1] = gain;
        expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_val[2] = gain;
        expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_val[0] = time;
        expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_val[1] = time;
        expSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_val[2] = time;
    } else {
        expSwAttr.commCtrl.sw_aeT_opt_mode = RK_AIQ_OP_MODE_MANUAL;
        expSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manGain_en = true;
        expSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manTime_en = true;
        expSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manGain_val = gain;
        expSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manTime_val = time;
    }
    ret = rk_aiq_user_api2_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setManualExp failed in setExpSwAttr!");
    IMGPROC_FUNC_EXIT
    return (ret);
}

/*
*****************************
* Desc: set frame rate
* Argument:
*   info.mode OP_AUTO or OP_MANUAL
*
*****************************
*/
XCamReturn rk_aiq_uapi_setFrameRate(const rk_aiq_sys_ctx_t* ctx, frameRateInfo_t info)
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

XCamReturn rk_aiq_uapi_getFrameRate(const rk_aiq_sys_ctx_t* ctx, frameRateInfo_t *info)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    ret = rk_aiq_user_api2_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetFrameRate failed!");
    if (expSwAttr.commCtrl.frmRate.sw_aeT_frmRate_mode == ae_frmRate_auto_mode) {
        info->fps = expSwAttr.commCtrl.frmRate.sw_aeT_frmRate_val;
        info->mode = OP_AUTO;
    } else if (expSwAttr.commCtrl.frmRate.sw_aeT_frmRate_mode == ae_frmRate_fix_mode) {
        info->fps = expSwAttr.commCtrl.frmRate.sw_aeT_frmRate_val;
        info->mode = OP_MANUAL;
    }
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
XCamReturn rk_aiq_uapi_setExpGainRange(
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
XCamReturn rk_aiq_uapi_getExpGainRange(
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
XCamReturn rk_aiq_uapi_setExpTimeRange(
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
XCamReturn rk_aiq_uapi_getExpTimeRange(
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
XCamReturn rk_aiq_uapi_setBLCMode(const rk_aiq_sys_ctx_t* ctx, bool on, aeMeasAreaType_t areaType)
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
XCamReturn rk_aiq_uapi_setBLCStrength(const rk_aiq_sys_ctx_t* ctx, int strength)
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
XCamReturn rk_aiq_uapi_setHLCMode(const rk_aiq_sys_ctx_t* ctx, bool on)
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
XCamReturn rk_aiq_uapi_setHLCStrength(const rk_aiq_sys_ctx_t* ctx, int strength)
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
XCamReturn rk_aiq_uapi_setAntiFlickerEn(const rk_aiq_sys_ctx_t* ctx, bool on)
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
XCamReturn rk_aiq_uapi_getAntiFlickerEn(const rk_aiq_sys_ctx_t* ctx, bool* on)
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

XCamReturn rk_aiq_uapi_setAntiFlickerMode(const rk_aiq_sys_ctx_t* ctx, antiFlickerMode_t mode)
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
XCamReturn rk_aiq_uapi_getAntiFlickerMode(const rk_aiq_sys_ctx_t* ctx, antiFlickerMode_t* mode)
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
XCamReturn rk_aiq_uapi_setExpPwrLineFreqMode(const rk_aiq_sys_ctx_t* ctx, expPwrLineFreq_t freq)
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
XCamReturn rk_aiq_uapi_getExpPwrLineFreqMode(
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
XCamReturn rk_aiq_uapi_setWBMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
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
XCamReturn rk_aiq_uapi_getWBMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
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
XCamReturn rk_aiq_uapi_lockAWB(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_awb_Lock(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_unlockAWB(const rk_aiq_sys_ctx_t* ctx)
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
XCamReturn rk_aiq_uapi_setMWBScene(const rk_aiq_sys_ctx_t* ctx, rk_aiq_wb_scene_t scene)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi_getMWBScene(const rk_aiq_sys_ctx_t* ctx, rk_aiq_wb_scene_t *scene)
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
XCamReturn rk_aiq_uapi_setMWBGain(const rk_aiq_sys_ctx_t* ctx, rk_aiq_wb_gain_t *gain)
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

XCamReturn rk_aiq_uapi_getWBGain(const rk_aiq_sys_ctx_t* ctx, rk_aiq_wb_gain_t *gain)
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
XCamReturn rk_aiq_uapi_setMWBCT(const rk_aiq_sys_ctx_t* ctx, unsigned int ct)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi_getWBCT(const rk_aiq_sys_ctx_t* ctx, unsigned int *ct)
{
    LOGE("not support to call %s for current chip", __FUNCTION__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_uapi_setFocusMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
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

XCamReturn rk_aiq_uapi_getFocusMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
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

XCamReturn rk_aiq_uapi_setFocusWin(const rk_aiq_sys_ctx_t* ctx, paRect_t *rect)
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

XCamReturn rk_aiq_uapi_getFocusWin(const rk_aiq_sys_ctx_t* ctx, paRect_t *rect)
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

XCamReturn rk_aiq_uapi_lockFocus(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_Lock(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_unlockFocus(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_Unlock(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_oneshotFocus(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_Oneshot(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_manualTrigerFocus(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_ManualTriger(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_trackingFocus(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_Tracking(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_getSearchPath(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_sec_path_t* path)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_GetSearchPath(ctx, path);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_getSearchResult(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_result_t* result)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_GetSearchResult(ctx, result);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_getZoomRange(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_zoomrange * range)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_GetZoomRange(ctx, range);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_getFocusRange(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_focusrange* range)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_GetFocusRange(ctx, range);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_setOpZoomPosition(const rk_aiq_sys_ctx_t* ctx, int pos)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_SetZoomIndex(ctx, pos);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_getOpZoomPosition(const rk_aiq_sys_ctx_t* ctx, int *pos)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_GetZoomIndex(ctx, pos);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_endOpZoomChange(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_EndZoomChg(ctx);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_startZoomCalib(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_StartZoomCalib(ctx);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_resetZoom(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_resetZoom(ctx);
    IMGPROC_FUNC_EXIT

    return ret;
}


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
XCamReturn rk_aiq_uapi_getDarkAreaBoostStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
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

XCamReturn rk_aiq_uapi_setDarkAreaBoostStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
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
XCamReturn rk_aiq_uapi_setMHDRStrth(const rk_aiq_sys_ctx_t* ctx, bool on, unsigned int level)
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

XCamReturn rk_aiq_uapi_getMHDRStrth(const rk_aiq_sys_ctx_t* ctx, bool * on, unsigned int *level)
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
XCamReturn rk_aiq_uapi_setNRMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER

    if (CHECK_ISP_HW_V39()) {
        ynr_api_attrib_t  ynr_attr;
        cnr_api_attrib_t  cnr_attr;
        btnr_api_attrib_t btnr_attr;

        ret = rk_aiq_user_api2_ynr_GetAttrib(ctx, &ynr_attr);
        ret = rk_aiq_user_api2_cnr_GetAttrib(ctx, &cnr_attr);
        ret = rk_aiq_user_api2_btnr_GetAttrib(ctx, &btnr_attr);

        if (mode == OP_AUTO) {
            ynr_attr.opMode = RK_AIQ_OP_MODE_AUTO;
            cnr_attr.opMode = RK_AIQ_OP_MODE_AUTO;
            btnr_attr.opMode = RK_AIQ_OP_MODE_AUTO;
        } else if (mode == OP_MANUAL) {
            ynr_attr.opMode = RK_AIQ_OP_MODE_MANUAL;
            cnr_attr.opMode = RK_AIQ_OP_MODE_MANUAL;
            btnr_attr.opMode = RK_AIQ_OP_MODE_MANUAL;
        } else {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "Not supported mode!");
        }

        ret = rk_aiq_user_api2_ynr_SetAttrib(ctx, &ynr_attr);
        ret = rk_aiq_user_api2_cnr_SetAttrib(ctx, &cnr_attr);
        ret = rk_aiq_user_api2_btnr_SetAttrib(ctx, &btnr_attr);

        LOGE("not support to call %s for current chip", __FUNCTION__);
        ret = XCAM_RETURN_ERROR_UNKNOWN;
    }

    RKAIQ_IMGPROC_CHECK_RET(ret, "setNRMode failed!", ret);
    IMGPROC_FUNC_EXIT

    return ret;
}


XCamReturn rk_aiq_uapi_getNRMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER

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
XCamReturn rk_aiq_uapi_setANRStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, setANRStrth failed!");
    }

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


XCamReturn rk_aiq_uapi_getANRStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    float percent = 0.0f;

    IMGPROC_FUNC_ENTER

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, getANRStrth failed!");
    }

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
XCamReturn rk_aiq_uapi_setMSpaNRStrth(const rk_aiq_sys_ctx_t* ctx, bool on, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, setMSpaNRStrth failed!");
    }

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
XCamReturn rk_aiq_uapi_getMSpaNRStrth(const rk_aiq_sys_ctx_t* ctx, bool * on, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    float percent = 0.0f;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, getMSpaNRStrth failed!");
    }

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
XCamReturn rk_aiq_uapi_setMTNRStrth(const rk_aiq_sys_ctx_t* ctx, bool on, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    LOGD("level=%d", level);
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, setMTNRStrth failed!");
    }

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
XCamReturn rk_aiq_uapi_getMTNRStrth(const rk_aiq_sys_ctx_t* ctx, bool * on, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    float percent = 0.0f;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, getMTNRStrth failed!");
    }

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
XCamReturn rk_aiq_uapi_setSharpness(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
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

    asharp_strength_t sharpStrength;
    sharpStrength.en = true;
    sharpStrength.percent = fPercent;
    ret = rk_aiq_user_api2_sharp_SetStrength(ctx, &sharpStrength);

    RKAIQ_IMGPROC_CHECK_RET(ret, "set sharpeness failed!");
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_getSharpness(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    float fPercent = 0.0f;

    IMGPROC_FUNC_ENTER
    if (level == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get sharpeness failed!");
    }

    asharp_strength_t sharpStrength;
    ret = rk_aiq_user_api2_sharp_GetStrength(ctx, &sharpStrength);
    fPercent = sharpStrength.percent;
    RKAIQ_IMGPROC_CHECK_RET(ret, "get sharpeness failed!");

    *level = (unsigned int)(fPercent * 100);
    IMGPROC_FUNC_EXIT

    return ret;
}


/*
*****************************
*
* Desc: Adjust image contrast level
* Argument:
*    level: contrast level, [0, 255]
*****************************
*/
XCamReturn rk_aiq_uapi_setContrast(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
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

XCamReturn rk_aiq_uapi_getContrast(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
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
XCamReturn rk_aiq_uapi_setBrightness(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    cp_api_attrib_t attrib;

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
    ret = rk_aiq_user_api2_cp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAttrib error,set brightness failed!");
    if(attrib.opMode == RK_AIQ_OP_MODE_AUTO)
        attrib.stAuto.sta.brightness = level;
    else
        attrib.stMan.sta.brightness = level;
    ret = rk_aiq_user_api2_cp_SetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set brightness failed!");
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_getBrightness(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER
    cp_api_attrib_t attrib;
    if (level == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get brightness failed!");
    }
    ret = rk_aiq_user_api2_cp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get brightness failed!");
    if(attrib.opMode == RK_AIQ_OP_MODE_AUTO)
        *level = attrib.stAuto.sta.brightness;
    else
        *level = attrib.stMan.sta.brightness;
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
XCamReturn rk_aiq_uapi_setSaturation(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    cp_api_attrib_t attrib;

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
    ret = rk_aiq_user_api2_cp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAttrib error,set saturation failed!");
    if(attrib.opMode == RK_AIQ_OP_MODE_AUTO)
        attrib.stAuto.sta.saturation = level;
    else
        attrib.stMan.sta.saturation = level;
    ret = rk_aiq_user_api2_cp_SetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set saturation failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_getSaturation(const rk_aiq_sys_ctx_t* ctx, unsigned int* level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    cp_api_attrib_t attrib;
    if (level == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get saturation failed!");
    }
    ret = rk_aiq_user_api2_cp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get saturation failed!");
    if(attrib.opMode == RK_AIQ_OP_MODE_AUTO)
        *level = attrib.stAuto.sta.saturation;
    else
        *level = attrib.stMan.sta.saturation;
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
XCamReturn rk_aiq_uapi_setHue(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    cp_api_attrib_t attrib;

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
    ret = rk_aiq_user_api2_cp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAttrib error,set hue failed!");
    if(attrib.opMode == RK_AIQ_OP_MODE_AUTO)
        attrib.stAuto.sta.hue = level;
    else
        attrib.stMan.sta.hue = level;
    ret = rk_aiq_user_api2_cp_SetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set hue failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_getHue(const rk_aiq_sys_ctx_t* ctx, unsigned int* level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    cp_api_attrib_t attrib;
    if (level == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get hue failed!");
    }
    ret = rk_aiq_user_api2_cp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get hue failed!");
    if(attrib.opMode == RK_AIQ_OP_MODE_AUTO)
        *level = attrib.stAuto.sta.hue;
    else
        *level = attrib.stMan.sta.hue;
    IMGPROC_FUNC_EXIT
    return ret;
}

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
XCamReturn rk_aiq_uapi_setGammaCoef(const rk_aiq_sys_ctx_t* ctx, float GammaCoef, float SlopeAtZero)
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

XCamReturn rk_aiq_uapi_setGrayMode(const rk_aiq_sys_ctx_t* ctx, rk_aiq_gray_mode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(ctx);
    for (int i = 0; i < ctx_array.num; i++) {
        ret = AiqCore_setGrayMode(ctx_array.ctx[i]->_analyzer, mode);
    }

    return ret;
}

rk_aiq_gray_mode_t rk_aiq_uapi_getGrayMode(const rk_aiq_sys_ctx_t* ctx)
{
    const rk_aiq_sys_ctx_t* sys_ctx = rk_aiq_user_api2_common_getSysCtx(ctx);
    return AiqCore_getGrayMode(sys_ctx->_analyzer);
}

XCamReturn rk_aiq_uapi_setMirrorFlip(const rk_aiq_sys_ctx_t* ctx, bool mirror, bool flip,
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

XCamReturn rk_aiq_uapi_getMirrorFlip(const rk_aiq_sys_ctx_t* ctx, bool* mirror, bool* flip) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL || mirror == NULL || flip == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    return AiqManager_getMirrorFlip(ctx->_rkAiqManager, mirror, flip);
}

XCamReturn rk_aiq_uapi_setLdchEn(const rk_aiq_sys_ctx_t* ctx, bool en)
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

XCamReturn rk_aiq_uapi_setLdchCorrectLevel(const rk_aiq_sys_ctx_t* ctx, int correctLevel)
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

#include "rk_aiq_uapi1_rv1126b.h"

XCamReturn
rk_aiq_user_api_ae_queryExpResInfo(const rk_aiq_sys_ctx_t* sys_ctx, Uapi_ExpQueryInfo_t* pExpResInfo)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (sys_ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ae_api_queryInfo_t aeExpResInfo;
    ret = rk_aiq_user_api2_ae_queryExpResInfo(sys_ctx, &aeExpResInfo);
    if (ret != XCAM_RETURN_ERROR_PARAM) {
        LOGE_AEC("%s maybe error", __func__);
        return ret;
    }

    pExpResInfo->EnvChange              = aeExpResInfo.envChange;
    pExpResInfo->Fps                    = aeExpResInfo.fps;
    pExpResInfo->GlobalEnvLv            = aeExpResInfo.gblEnvLv;
    pExpResInfo->HighLightROIPdf        = aeExpResInfo.hiLitROIPdf;
    pExpResInfo->IsConverged            = aeExpResInfo.isConverged;
    pExpResInfo->IsExpMax               = aeExpResInfo.isExpMax;
    pExpResInfo->IsExpMin               = aeExpResInfo.isExpMin;
    pExpResInfo->LinePeriodsPerField    = aeExpResInfo.vts;
    pExpResInfo->LowLightROIPdf         = aeExpResInfo.loLitROIPdf;
    pExpResInfo->OverExpROIPdf          = aeExpResInfo.oeROIPdf;
    pExpResInfo->PixelClockFreqMHZ      = aeExpResInfo.pclk;
    pExpResInfo->PixelPeriodsPerLine    = aeExpResInfo.hts;

    // liner info
    pExpResInfo->LinAeInfo.MeanLuma                         = aeExpResInfo.linExpInfo.meanLuma;
    pExpResInfo->LinAeInfo.LumaDeviation                    = aeExpResInfo.linExpInfo.devLuma;
    pExpResInfo->LinAeInfo.LinAeRange.stExpTimeRange.Max    = aeExpResInfo.linExpInfo.expRange.sw_aeT_time_max;
    pExpResInfo->LinAeInfo.LinAeRange.stExpTimeRange.Min    = aeExpResInfo.linExpInfo.expRange.sw_aeT_time_min;
    pExpResInfo->LinAeInfo.LinAeRange.stGainRange.Max       = aeExpResInfo.linExpInfo.expRange.sw_aeT_gain_max;
    pExpResInfo->LinAeInfo.LinAeRange.stGainRange.Min       = aeExpResInfo.linExpInfo.expRange.sw_aeT_gain_min;
    pExpResInfo->LinAeInfo.LinAeRange.stIspDGainRange.Max   = aeExpResInfo.linExpInfo.expRange.sw_aeT_ispDGain_max;
    pExpResInfo->LinAeInfo.LinAeRange.stIspDGainRange.Min   = aeExpResInfo.linExpInfo.expRange.sw_aeT_ispDGain_min;
    pExpResInfo->LinAeInfo.LinAeRange.stPIrisRange.Max      = aeExpResInfo.linExpInfo.expRange.sw_aeT_pIrisGain_max;
    pExpResInfo->LinAeInfo.LinAeRange.stPIrisRange.Min      = aeExpResInfo.linExpInfo.expRange.sw_aeT_pIrisGain_min;
    pExpResInfo->LinAeInfo.LinearExp.analog_gain            = aeExpResInfo.linExpInfo.expParam.analog_gain;
    pExpResInfo->LinAeInfo.LinearExp.dcg_mode               = aeExpResInfo.linExpInfo.expParam.dcg_mode;
    pExpResInfo->LinAeInfo.LinearExp.digital_gain           = aeExpResInfo.linExpInfo.expParam.digital_gain;
    pExpResInfo->LinAeInfo.LinearExp.integration_time       = aeExpResInfo.linExpInfo.expParam.integration_time;
    pExpResInfo->LinAeInfo.LinearExp.iso                    = aeExpResInfo.linExpInfo.expParam.iso;
    pExpResInfo->LinAeInfo.LinearExp.isp_dgain              = aeExpResInfo.linExpInfo.expParam.isp_dgain;
    pExpResInfo->LinAeInfo.LinearExp.longfrm_mode           = aeExpResInfo.linExpInfo.expParam.longfrm_mode;

    // hdr info
    pExpResInfo->HdrAeInfo.Frm0Luma                          = aeExpResInfo.hdrExpInfo.frm0Luma;
    pExpResInfo->HdrAeInfo.Frm1Luma                          = aeExpResInfo.hdrExpInfo.frm1Luma;
    pExpResInfo->HdrAeInfo.Frm2Luma                          = aeExpResInfo.hdrExpInfo.frm2Luma;
    pExpResInfo->HdrAeInfo.HdrLumaDeviation[0]               = aeExpResInfo.hdrExpInfo.devLuma[0];
    pExpResInfo->HdrAeInfo.HdrLumaDeviation[1]               = aeExpResInfo.hdrExpInfo.devLuma[1];
    pExpResInfo->HdrAeInfo.HdrLumaDeviation[2]               = aeExpResInfo.hdrExpInfo.devLuma[2];
    pExpResInfo->HdrAeInfo.HdrAeRange.stExpTimeRange[0].Max  = aeExpResInfo.hdrExpInfo.expRange[0].sw_aeT_time_max;
    pExpResInfo->HdrAeInfo.HdrAeRange.stExpTimeRange[1].Max  = aeExpResInfo.hdrExpInfo.expRange[1].sw_aeT_time_max;
    pExpResInfo->HdrAeInfo.HdrAeRange.stExpTimeRange[2].Max  = aeExpResInfo.hdrExpInfo.expRange[2].sw_aeT_time_max;
    pExpResInfo->HdrAeInfo.HdrAeRange.stExpTimeRange[0].Min  = aeExpResInfo.hdrExpInfo.expRange[0].sw_aeT_time_min;
    pExpResInfo->HdrAeInfo.HdrAeRange.stExpTimeRange[1].Min  = aeExpResInfo.hdrExpInfo.expRange[1].sw_aeT_time_min;
    pExpResInfo->HdrAeInfo.HdrAeRange.stExpTimeRange[2].Min  = aeExpResInfo.hdrExpInfo.expRange[2].sw_aeT_time_min;
    pExpResInfo->HdrAeInfo.HdrAeRange.stGainRange[0].Max     = aeExpResInfo.hdrExpInfo.expRange[0].sw_aeT_gain_max;
    pExpResInfo->HdrAeInfo.HdrAeRange.stGainRange[1].Max     = aeExpResInfo.hdrExpInfo.expRange[1].sw_aeT_gain_max;
    pExpResInfo->HdrAeInfo.HdrAeRange.stGainRange[2].Max     = aeExpResInfo.hdrExpInfo.expRange[2].sw_aeT_gain_max;
    pExpResInfo->HdrAeInfo.HdrAeRange.stGainRange[0].Min     = aeExpResInfo.hdrExpInfo.expRange[0].sw_aeT_gain_min;
    pExpResInfo->HdrAeInfo.HdrAeRange.stGainRange[1].Min     = aeExpResInfo.hdrExpInfo.expRange[1].sw_aeT_gain_min;
    pExpResInfo->HdrAeInfo.HdrAeRange.stGainRange[2].Min     = aeExpResInfo.hdrExpInfo.expRange[2].sw_aeT_gain_min;
    pExpResInfo->HdrAeInfo.HdrAeRange.stIspDGainRange[0].Max = aeExpResInfo.hdrExpInfo.expRange[0].sw_aeT_ispDGain_max;
    pExpResInfo->HdrAeInfo.HdrAeRange.stIspDGainRange[1].Max = aeExpResInfo.hdrExpInfo.expRange[1].sw_aeT_ispDGain_max;
    pExpResInfo->HdrAeInfo.HdrAeRange.stIspDGainRange[2].Max = aeExpResInfo.hdrExpInfo.expRange[2].sw_aeT_ispDGain_max;
    pExpResInfo->HdrAeInfo.HdrAeRange.stIspDGainRange[0].Min = aeExpResInfo.hdrExpInfo.expRange[0].sw_aeT_ispDGain_min;
    pExpResInfo->HdrAeInfo.HdrAeRange.stIspDGainRange[1].Min = aeExpResInfo.hdrExpInfo.expRange[1].sw_aeT_ispDGain_min;
    pExpResInfo->HdrAeInfo.HdrAeRange.stIspDGainRange[2].Min = aeExpResInfo.hdrExpInfo.expRange[2].sw_aeT_ispDGain_min;
    pExpResInfo->HdrAeInfo.HdrAeRange.stPIrisRange.Max       = aeExpResInfo.hdrExpInfo.expRange->sw_aeT_pIrisGain_max;
    pExpResInfo->HdrAeInfo.HdrAeRange.stPIrisRange.Min       = aeExpResInfo.hdrExpInfo.expRange->sw_aeT_pIrisGain_min;
    pExpResInfo->HdrAeInfo.HdrExp[0].analog_gain             = aeExpResInfo.hdrExpInfo.expParam[0].analog_gain;
    pExpResInfo->HdrAeInfo.HdrExp[1].analog_gain             = aeExpResInfo.hdrExpInfo.expParam[1].analog_gain;
    pExpResInfo->HdrAeInfo.HdrExp[2].analog_gain             = aeExpResInfo.hdrExpInfo.expParam[2].analog_gain;
    pExpResInfo->HdrAeInfo.HdrExp[0].dcg_mode                = aeExpResInfo.hdrExpInfo.expParam[0].dcg_mode;
    pExpResInfo->HdrAeInfo.HdrExp[1].dcg_mode                = aeExpResInfo.hdrExpInfo.expParam[1].dcg_mode;
    pExpResInfo->HdrAeInfo.HdrExp[2].dcg_mode                = aeExpResInfo.hdrExpInfo.expParam[2].dcg_mode;
    pExpResInfo->HdrAeInfo.HdrExp[0].digital_gain            = aeExpResInfo.hdrExpInfo.expParam[0].digital_gain;
    pExpResInfo->HdrAeInfo.HdrExp[1].digital_gain            = aeExpResInfo.hdrExpInfo.expParam[1].digital_gain;
    pExpResInfo->HdrAeInfo.HdrExp[2].digital_gain            = aeExpResInfo.hdrExpInfo.expParam[2].digital_gain;
    pExpResInfo->HdrAeInfo.HdrExp[0].integration_time        = aeExpResInfo.hdrExpInfo.expParam[0].integration_time;
    pExpResInfo->HdrAeInfo.HdrExp[1].integration_time        = aeExpResInfo.hdrExpInfo.expParam[1].integration_time;
    pExpResInfo->HdrAeInfo.HdrExp[2].integration_time        = aeExpResInfo.hdrExpInfo.expParam[2].integration_time;
    pExpResInfo->HdrAeInfo.HdrExp[0].iso                     = aeExpResInfo.hdrExpInfo.expParam[0].iso;
    pExpResInfo->HdrAeInfo.HdrExp[1].iso                     = aeExpResInfo.hdrExpInfo.expParam[1].iso;
    pExpResInfo->HdrAeInfo.HdrExp[2].iso                     = aeExpResInfo.hdrExpInfo.expParam[2].iso;
    pExpResInfo->HdrAeInfo.HdrExp[0].isp_dgain               = aeExpResInfo.hdrExpInfo.expParam[0].isp_dgain;
    pExpResInfo->HdrAeInfo.HdrExp[1].isp_dgain               = aeExpResInfo.hdrExpInfo.expParam[1].isp_dgain;
    pExpResInfo->HdrAeInfo.HdrExp[2].isp_dgain               = aeExpResInfo.hdrExpInfo.expParam[2].isp_dgain;
    pExpResInfo->HdrAeInfo.HdrExp[0].longfrm_mode            = aeExpResInfo.hdrExpInfo.expParam[0].longfrm_mode;
    pExpResInfo->HdrAeInfo.HdrExp[1].longfrm_mode            = aeExpResInfo.hdrExpInfo.expParam[1].longfrm_mode;
    pExpResInfo->HdrAeInfo.HdrExp[2].longfrm_mode            = aeExpResInfo.hdrExpInfo.expParam[2].longfrm_mode;

    return ret;
}

XCamReturn rk_aiq_user_api_ae_setExpSwAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_ExpSwAttr_t expSwAttr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    ae_api_expSwAttr_t newExpSwAttr;
    ret = rk_aiq_user_api2_ae_getExpSwAttr(sys_ctx, &newExpSwAttr);

    newExpSwAttr.commCtrl.sw_aeT_algo_en        = expSwAttr.enable;
    newExpSwAttr.commCtrl.sw_aeT_rawStats_mode  = (ae_rawStats_mode_t)expSwAttr.RawStatsMode;
    newExpSwAttr.commCtrl.sw_aeT_histStats_mode = (ae_histStats_mode_t)expSwAttr.HistStatsMode;
    newExpSwAttr.commCtrl.sw_aeT_yRange_mode    = (ae_yRange_mode_t)expSwAttr.YRangeMode;
    newExpSwAttr.commCtrl.sw_aeT_algo_interval  = expSwAttr.AecRunInterval;
    newExpSwAttr.commCtrl.sw_aeT_opt_mode       = expSwAttr.AecOpType;

    if (expSwAttr.DayWeightNum == AECV2_MAX_GRIDWEIGHT_NUM) {
        memcpy(newExpSwAttr.commCtrl.sw_aeT_grid_wgt, expSwAttr.DayGridWeights.uCoeff, sizeof(expSwAttr.DayGridWeights.uCoeff));
    } else {
        LOGW("%s not support to set DayWeightNum (%d) for current chip", __FUNCTION__, expSwAttr.DayWeightNum);
    }

    // NightGridWeights/NightWeightNum/DNTrigger/DNMode/FillLightMode can't convert

    // stAuto
    if (expSwAttr.stAuto.SetAeRangeEn) {
        LOGW("%s not support to SetAeRangeEn directly for current chip", __FUNCTION__);
    }
    newExpSwAttr.commCtrl.speed.sw_aeT_smooth_en              = expSwAttr.stAuto.stAeSpeed.SmoothEn;
    newExpSwAttr.commCtrl.speed.dynDamp.sw_aeT_dynDamp_en     = expSwAttr.stAuto.stAeSpeed.DyDampEn;
    newExpSwAttr.commCtrl.speed.sw_aeT_damp_over              = expSwAttr.stAuto.stAeSpeed.DampOver;
    newExpSwAttr.commCtrl.speed.sw_aeT_damp_under             = expSwAttr.stAuto.stAeSpeed.DampUnder;
    newExpSwAttr.commCtrl.speed.sw_aeT_damp_dark2Bright       = expSwAttr.stAuto.stAeSpeed.DampDark2Bright;
    newExpSwAttr.commCtrl.speed.sw_aeT_damp_bright2Dark       = expSwAttr.stAuto.stAeSpeed.DampBright2Dark;

    newExpSwAttr.commCtrl.delay.sw_aeT_delay_mode             = ae_delay_frame_mode;
    newExpSwAttr.commCtrl.delay.sw_aeT_blackDelay_val         = expSwAttr.stAuto.BlackDelayFrame;
    newExpSwAttr.commCtrl.delay.sw_aeT_whiteDelay_val         = expSwAttr.stAuto.WhiteDelayFrame;
    newExpSwAttr.commCtrl.frmRate.sw_aeT_frmRate_val          = expSwAttr.stAuto.stFrmRate.FpsValue;
    newExpSwAttr.commCtrl.frmRate.sw_aeT_frmRate_mode         = (ae_frmRate_mode_t)expSwAttr.stAuto.stFrmRate.isFpsFix;

    newExpSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_en   = expSwAttr.stAntiFlicker.enable;
    newExpSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_freq = (ae_antiFlicker_freq_t)expSwAttr.stAntiFlicker.Frequency;
    newExpSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_mode = (ae_antiFlicker_mode_t)expSwAttr.stAntiFlicker.Mode;

    // stManual
    newExpSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manGain_en         = expSwAttr.stManual.stLinMe.ManualGainEn;
    newExpSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manGain_val        = expSwAttr.stManual.stLinMe.GainValue;
    newExpSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manIspDGain_en     = expSwAttr.stManual.stLinMe.ManualIspDgainEn;
    newExpSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manIspDGain_val    = expSwAttr.stManual.stLinMe.IspDGainValue;
    newExpSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manTime_en         = expSwAttr.stManual.stLinMe.ManualTimeEn;
    newExpSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manTime_val        = expSwAttr.stManual.stLinMe.TimeValue;
    newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_en         = expSwAttr.stManual.stHdrMe.ManualGainEn;
    newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_val[0]     = expSwAttr.stManual.stHdrMe.GainValue.fCoeff[0];
    newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_val[1]     = expSwAttr.stManual.stHdrMe.GainValue.fCoeff[1];
    newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_val[2]     = expSwAttr.stManual.stHdrMe.GainValue.fCoeff[2];
    newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manIspDGain_en     = expSwAttr.stManual.stHdrMe.ManualIspDgainEn;
    newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manIspDGain_val[0] = expSwAttr.stManual.stHdrMe.IspDGainValue.fCoeff[0];
    newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manIspDGain_val[1] = expSwAttr.stManual.stHdrMe.IspDGainValue.fCoeff[1];
    newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manIspDGain_val[2] = expSwAttr.stManual.stHdrMe.IspDGainValue.fCoeff[2];
    newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_en         = expSwAttr.stManual.stHdrMe.ManualTimeEn;
    newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_val[0]     = expSwAttr.stManual.stHdrMe.TimeValue.fCoeff[0];
    newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_val[1]     = expSwAttr.stManual.stHdrMe.TimeValue.fCoeff[1];
    newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_val[2]     = expSwAttr.stManual.stHdrMe.TimeValue.fCoeff[2];

    // advanced
    newExpSwAttr.advanced.sw_aeT_advGridWgt_en                = expSwAttr.stAdvanced.enable;
    newExpSwAttr.advanced.sw_aeT_advAeRange_en                = expSwAttr.stAdvanced.SetAeRangeEn;
    memcpy(newExpSwAttr.advanced.sw_aeT_advGrid_wgt, expSwAttr.stAdvanced.GridWeights, sizeof(expSwAttr.stAdvanced.GridWeights));

    newExpSwAttr.advanced.linExpRange.sw_aeT_time_max         = expSwAttr.stAdvanced.SetLinAeRange.stExpTimeRange.Max;
    newExpSwAttr.advanced.linExpRange.sw_aeT_time_min         = expSwAttr.stAdvanced.SetLinAeRange.stExpTimeRange.Min;
    newExpSwAttr.advanced.linExpRange.sw_aeT_gain_max         = expSwAttr.stAdvanced.SetLinAeRange.stGainRange.Max;
    newExpSwAttr.advanced.linExpRange.sw_aeT_gain_min         = expSwAttr.stAdvanced.SetLinAeRange.stGainRange.Min;
    newExpSwAttr.advanced.linExpRange.sw_aeT_ispDGain_max     = expSwAttr.stAdvanced.SetLinAeRange.stIspDGainRange.Max;
    newExpSwAttr.advanced.linExpRange.sw_aeT_ispDGain_min     = expSwAttr.stAdvanced.SetLinAeRange.stIspDGainRange.Min;
    newExpSwAttr.advanced.linExpRange.sw_aeT_pIrisGain_max    = expSwAttr.stAdvanced.SetLinAeRange.stPIrisRange.Max;
    newExpSwAttr.advanced.linExpRange.sw_aeT_pIrisGain_min    = expSwAttr.stAdvanced.SetLinAeRange.stPIrisRange.Min;

    newExpSwAttr.advanced.hdrExpRange[0].sw_aeT_time_max      = expSwAttr.stAdvanced.SetHdrAeRange.stExpTimeRange[0].Max;
    newExpSwAttr.advanced.hdrExpRange[1].sw_aeT_time_max      = expSwAttr.stAdvanced.SetHdrAeRange.stExpTimeRange[1].Max;
    newExpSwAttr.advanced.hdrExpRange[2].sw_aeT_time_max      = expSwAttr.stAdvanced.SetHdrAeRange.stExpTimeRange[2].Max;
    newExpSwAttr.advanced.hdrExpRange[0].sw_aeT_time_min      = expSwAttr.stAdvanced.SetHdrAeRange.stExpTimeRange[0].Min;
    newExpSwAttr.advanced.hdrExpRange[1].sw_aeT_time_min      = expSwAttr.stAdvanced.SetHdrAeRange.stExpTimeRange[1].Min;
    newExpSwAttr.advanced.hdrExpRange[2].sw_aeT_time_min      = expSwAttr.stAdvanced.SetHdrAeRange.stExpTimeRange[2].Min;
    newExpSwAttr.advanced.hdrExpRange[0].sw_aeT_gain_max      = expSwAttr.stAdvanced.SetHdrAeRange.stGainRange[0].Max;
    newExpSwAttr.advanced.hdrExpRange[1].sw_aeT_gain_max      = expSwAttr.stAdvanced.SetHdrAeRange.stGainRange[1].Max;
    newExpSwAttr.advanced.hdrExpRange[2].sw_aeT_gain_max      = expSwAttr.stAdvanced.SetHdrAeRange.stGainRange[2].Max;
    newExpSwAttr.advanced.hdrExpRange[0].sw_aeT_gain_min      = expSwAttr.stAdvanced.SetHdrAeRange.stGainRange[0].Min;
    newExpSwAttr.advanced.hdrExpRange[1].sw_aeT_gain_min      = expSwAttr.stAdvanced.SetHdrAeRange.stGainRange[1].Min;
    newExpSwAttr.advanced.hdrExpRange[2].sw_aeT_gain_min      = expSwAttr.stAdvanced.SetHdrAeRange.stGainRange[2].Min;
    newExpSwAttr.advanced.hdrExpRange[0].sw_aeT_ispDGain_max  = expSwAttr.stAdvanced.SetHdrAeRange.stIspDGainRange[0].Max;
    newExpSwAttr.advanced.hdrExpRange[1].sw_aeT_ispDGain_max  = expSwAttr.stAdvanced.SetHdrAeRange.stIspDGainRange[1].Max;
    newExpSwAttr.advanced.hdrExpRange[2].sw_aeT_ispDGain_max  = expSwAttr.stAdvanced.SetHdrAeRange.stIspDGainRange[2].Max;
    newExpSwAttr.advanced.hdrExpRange[0].sw_aeT_ispDGain_min  = expSwAttr.stAdvanced.SetHdrAeRange.stIspDGainRange[0].Min;
    newExpSwAttr.advanced.hdrExpRange[1].sw_aeT_ispDGain_min  = expSwAttr.stAdvanced.SetHdrAeRange.stIspDGainRange[1].Min;
    newExpSwAttr.advanced.hdrExpRange[2].sw_aeT_ispDGain_min  = expSwAttr.stAdvanced.SetHdrAeRange.stIspDGainRange[2].Min;
    newExpSwAttr.advanced.hdrExpRange[0].sw_aeT_pIrisGain_max = expSwAttr.stAdvanced.SetHdrAeRange.stPIrisRange.Max;
    newExpSwAttr.advanced.hdrExpRange[0].sw_aeT_pIrisGain_min = expSwAttr.stAdvanced.SetHdrAeRange.stPIrisRange.Min;
    newExpSwAttr.advanced.hdrExpRange[1].sw_aeT_pIrisGain_max = expSwAttr.stAdvanced.SetHdrAeRange.stPIrisRange.Max;
    newExpSwAttr.advanced.hdrExpRange[1].sw_aeT_pIrisGain_min = expSwAttr.stAdvanced.SetHdrAeRange.stPIrisRange.Min;
    newExpSwAttr.advanced.hdrExpRange[2].sw_aeT_pIrisGain_max = expSwAttr.stAdvanced.SetHdrAeRange.stPIrisRange.Max;
    newExpSwAttr.advanced.hdrExpRange[2].sw_aeT_pIrisGain_min = expSwAttr.stAdvanced.SetHdrAeRange.stPIrisRange.Min;

    ret = rk_aiq_user_api2_ae_setExpSwAttr(sys_ctx, newExpSwAttr);

    ae_api_linExpAttr_t linAttr;
    ret = rk_aiq_user_api2_ae_getLinExpAttr(sys_ctx, &linAttr);

    linAttr.initExp.sw_aeT_initTime_val     = expSwAttr.stInitExp.stLinExpInitExp.InitTimeValue;
    linAttr.initExp.sw_aeT_initGain_val     = expSwAttr.stInitExp.stLinExpInitExp.InitGainValue;
    linAttr.initExp.sw_aeT_initIspDGain_val = expSwAttr.stInitExp.stLinExpInitExp.InitIspDGainValue;

    ret = rk_aiq_user_api2_ae_setLinExpAttr(sys_ctx, linAttr);

    ae_api_hdrExpAttr_t hdrAttr;
    ret = rk_aiq_user_api2_ae_getHdrExpAttr(sys_ctx, &hdrAttr);

    hdrAttr.initExp.sw_aeT_initTime_val[0]     = expSwAttr.stInitExp.stHdrExpInitExp.InitTimeValue.fCoeff[0];
    hdrAttr.initExp.sw_aeT_initGain_val[0]     = expSwAttr.stInitExp.stHdrExpInitExp.InitGainValue.fCoeff[0];
    hdrAttr.initExp.sw_aeT_initIspDGain_val[0] = expSwAttr.stInitExp.stHdrExpInitExp.InitIspDGainValue.fCoeff[0];
    hdrAttr.initExp.sw_aeT_initTime_val[1]     = expSwAttr.stInitExp.stHdrExpInitExp.InitTimeValue.fCoeff[1];
    hdrAttr.initExp.sw_aeT_initGain_val[1]     = expSwAttr.stInitExp.stHdrExpInitExp.InitGainValue.fCoeff[1];
    hdrAttr.initExp.sw_aeT_initIspDGain_val[1] = expSwAttr.stInitExp.stHdrExpInitExp.InitIspDGainValue.fCoeff[1];
    hdrAttr.initExp.sw_aeT_initTime_val[2]     = expSwAttr.stInitExp.stHdrExpInitExp.InitTimeValue.fCoeff[2];
    hdrAttr.initExp.sw_aeT_initGain_val[2]     = expSwAttr.stInitExp.stHdrExpInitExp.InitGainValue.fCoeff[2];
    hdrAttr.initExp.sw_aeT_initIspDGain_val[2] = expSwAttr.stInitExp.stHdrExpInitExp.InitIspDGainValue.fCoeff[2];

    ret = rk_aiq_user_api2_ae_setHdrExpAttr(sys_ctx, hdrAttr);

    ae_api_irisAttr_t irisAttr;
    ret = rk_aiq_user_api2_ae_getIrisAttr(sys_ctx, &irisAttr);

    irisAttr.initIris.sw_aeT_initDCIrisHold_val  = expSwAttr.stInitExp.stLinExpInitExp.InitDCIrisDutyValue;
    irisAttr.initIris.sw_aeT_initHDCIrisGain_val = expSwAttr.stInitExp.stLinExpInitExp.InitHDCIrisTargetValue;
    irisAttr.initIris.sw_aeT_initPIrisGain_val   = expSwAttr.stInitExp.stLinExpInitExp.InitPIrisGainValue;

    irisAttr.manIris.sw_aeT_manIris_en = expSwAttr.stManual.stLinMe.ManualIrisEn;
    irisAttr.manIris.sw_aeT_manPIrisGain_val = expSwAttr.stManual.stLinMe.PIrisGainValue;
    irisAttr.manIris.sw_aeT_manDCIrisHold_val = expSwAttr.stManual.stLinMe.DCIrisValue;
    irisAttr.manIris.sw_aeT_manHDCIrisGain_val = expSwAttr.stManual.stLinMe.HDCIrisValue;

    irisAttr.sw_aeT_iris_en   = expSwAttr.stIris.enable;
    irisAttr.sw_aeT_iris_type = (ae_iris_type_t)expSwAttr.stIris.IrisType;

    irisAttr.pIrisCtrl.sw_aeT_zeroIsMax_en  = expSwAttr.stIris.PIrisAttr.ZeroIsMax;
    irisAttr.pIrisCtrl.sw_aeT_effcStep_val  = expSwAttr.stIris.PIrisAttr.EffcStep;
    irisAttr.pIrisCtrl.sw_aeT_totalStep_val = expSwAttr.stIris.PIrisAttr.TotalStep;
    memcpy(irisAttr.pIrisCtrl.sw_aeT_step2Gain_table, expSwAttr.stIris.PIrisAttr.StepTable, sizeof(expSwAttr.stIris.PIrisAttr.StepTable));

    irisAttr.dcIrisCtrl.sw_aeT_dcIris_Kp     = expSwAttr.stIris.DCIrisAttr.Kp;
    irisAttr.dcIrisCtrl.sw_aeT_dcIris_Ki     = expSwAttr.stIris.DCIrisAttr.Ki;
    irisAttr.dcIrisCtrl.sw_aeT_dcIris_Kd     = expSwAttr.stIris.DCIrisAttr.Kd;
    irisAttr.dcIrisCtrl.sw_aeT_pwmDuty_min   = expSwAttr.stIris.DCIrisAttr.MinPwmDuty;
    irisAttr.dcIrisCtrl.sw_aeT_pwmDuty_max   = expSwAttr.stIris.DCIrisAttr.MaxPwmDuty;
    irisAttr.dcIrisCtrl.sw_aeT_pwmDuty_open  = expSwAttr.stIris.DCIrisAttr.OpenPwmDuty;
    irisAttr.dcIrisCtrl.sw_aeT_pwmDuty_close = expSwAttr.stIris.DCIrisAttr.ClosePwmDuty;

    irisAttr.hdcIrisCtrl.sw_aeT_damp_over    = expSwAttr.stIris.HDCIrisAttr.DampOver;
    irisAttr.hdcIrisCtrl.sw_aeT_damp_under   = expSwAttr.stIris.HDCIrisAttr.DampUnder;
    irisAttr.hdcIrisCtrl.sw_aeT_zeroIsMax_en = expSwAttr.stIris.HDCIrisAttr.ZeroIsMax;
    irisAttr.hdcIrisCtrl.sw_aeT_target_min   = expSwAttr.stIris.HDCIrisAttr.MinTarget;
    irisAttr.hdcIrisCtrl.sw_aeT_target_max   = expSwAttr.stIris.HDCIrisAttr.MaxTarget;

    irisAttr.hdcIrisCtrl.zoom2Iris.sw_aeC_zoom2Iris_len = expSwAttr.stIris.HDCIrisAttr.zoom_array_size;
    memcpy(irisAttr.hdcIrisCtrl.zoom2Iris.sw_aeC_zoom2Iris_idx, expSwAttr.stIris.HDCIrisAttr.ZoomDot, sizeof(expSwAttr.stIris.HDCIrisAttr.ZoomDot));
    memcpy(irisAttr.hdcIrisCtrl.zoom2Iris.sw_aeC_zoom2Iris_val, expSwAttr.stIris.HDCIrisAttr.ZoomTargetDot, sizeof(expSwAttr.stIris.HDCIrisAttr.ZoomTargetDot));
    irisAttr.hdcIrisCtrl.iris2Gain.sw_aeC_iris2Gain_len = expSwAttr.stIris.HDCIrisAttr.iris_array_size;
    memcpy(irisAttr.hdcIrisCtrl.iris2Gain.sw_aeC_iris2Gain_idx, expSwAttr.stIris.HDCIrisAttr.IrisTargetDot, sizeof(expSwAttr.stIris.HDCIrisAttr.IrisTargetDot));
    memcpy(irisAttr.hdcIrisCtrl.iris2Gain.sw_aeC_iris2Gain_val, expSwAttr.stIris.HDCIrisAttr.GainDot, sizeof(expSwAttr.stIris.HDCIrisAttr.GainDot));

    ret = rk_aiq_user_api2_ae_setIrisAttr(sys_ctx, irisAttr);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_user_api_ae_getExpSwAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_ExpSwAttr_t*        pExpSwAttr
)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    ae_api_expSwAttr_t newExpSwAttr;
    ret = rk_aiq_user_api2_ae_getExpSwAttr(sys_ctx, &newExpSwAttr);

    pExpSwAttr->enable           = newExpSwAttr.commCtrl.sw_aeT_algo_en;
    pExpSwAttr->RawStatsMode     = (CalibDb_CamRawStatsMode_t)newExpSwAttr.commCtrl.sw_aeT_rawStats_mode;
    pExpSwAttr->HistStatsMode    = (CalibDb_CamHistStatsMode_t)newExpSwAttr.commCtrl.sw_aeT_histStats_mode;
    pExpSwAttr->YRangeMode       = (CalibDb_CamYRangeMode_t)newExpSwAttr.commCtrl.sw_aeT_yRange_mode;
    pExpSwAttr->AecRunInterval   = newExpSwAttr.commCtrl.sw_aeT_algo_interval;
    pExpSwAttr->AecOpType        = newExpSwAttr.commCtrl.sw_aeT_opt_mode;

    pExpSwAttr->DayWeightNum = AECV2_MAX_GRIDWEIGHT_NUM;
    memcpy(pExpSwAttr->DayGridWeights.uCoeff, newExpSwAttr.commCtrl.sw_aeT_grid_wgt, sizeof(pExpSwAttr->DayGridWeights.uCoeff));

    // NightGridWeights/NightWeightNum/DNTrigger/DNMode/FillLightMode can't convert

    // stAuto: get lin/hdr ae range by rk_aiq_user_api_ae_queryExpResInfo
    pExpSwAttr->stAuto.SetAeRangeEn              = false;
    pExpSwAttr->stAuto.stAeSpeed.SmoothEn        = newExpSwAttr.commCtrl.speed.sw_aeT_smooth_en;
    pExpSwAttr->stAuto.stAeSpeed.DyDampEn        = newExpSwAttr.commCtrl.speed.dynDamp.sw_aeT_dynDamp_en;
    pExpSwAttr->stAuto.stAeSpeed.DampOver        = newExpSwAttr.commCtrl.speed.sw_aeT_damp_over;
    pExpSwAttr->stAuto.stAeSpeed.DampUnder       = newExpSwAttr.commCtrl.speed.sw_aeT_damp_under;
    pExpSwAttr->stAuto.stAeSpeed.DampDark2Bright = newExpSwAttr.commCtrl.speed.sw_aeT_damp_dark2Bright;
    pExpSwAttr->stAuto.stAeSpeed.DampBright2Dark = newExpSwAttr.commCtrl.speed.sw_aeT_damp_bright2Dark;
    pExpSwAttr->stAuto.BlackDelayFrame           = newExpSwAttr.commCtrl.delay.sw_aeT_blackDelay_val;
    pExpSwAttr->stAuto.WhiteDelayFrame           = newExpSwAttr.commCtrl.delay.sw_aeT_whiteDelay_val;
    pExpSwAttr->stAuto.stFrmRate.FpsValue        = newExpSwAttr.commCtrl.frmRate.sw_aeT_frmRate_val;
    pExpSwAttr->stAuto.stFrmRate.isFpsFix        = (newExpSwAttr.commCtrl.frmRate.sw_aeT_frmRate_mode == ae_frmRate_auto_mode) ? false : true;

    pExpSwAttr->stAntiFlicker.enable     = newExpSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_en;
    pExpSwAttr->stAntiFlicker.Frequency  = (CalibDb_FlickerFreq_t)newExpSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_freq;
    pExpSwAttr->stAntiFlicker.Mode       = (CalibDb_AntiFlickerMode_t)newExpSwAttr.commCtrl.antiFlicker.sw_aeT_antiFlicker_mode;

    // stManual
    pExpSwAttr->stManual.stLinMe.ManualGainEn            = newExpSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manGain_en;
    pExpSwAttr->stManual.stLinMe.GainValue               = newExpSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manGain_val;
    pExpSwAttr->stManual.stLinMe.ManualIspDgainEn        = newExpSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manIspDGain_en;
    pExpSwAttr->stManual.stLinMe.IspDGainValue           = newExpSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manIspDGain_val;
    pExpSwAttr->stManual.stLinMe.ManualTimeEn            = newExpSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manTime_en;
    pExpSwAttr->stManual.stLinMe.TimeValue               = newExpSwAttr.commCtrl.meCtrl.linMe.sw_aeT_manTime_val;
    pExpSwAttr->stManual.stHdrMe.ManualGainEn            = newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_en;
    pExpSwAttr->stManual.stHdrMe.GainValue.fCoeff[0]     = newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_val[0];
    pExpSwAttr->stManual.stHdrMe.GainValue.fCoeff[1]     = newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_val[1];
    pExpSwAttr->stManual.stHdrMe.GainValue.fCoeff[2]     = newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manGain_val[2];
    pExpSwAttr->stManual.stHdrMe.ManualIspDgainEn        = newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manIspDGain_en;
    pExpSwAttr->stManual.stHdrMe.IspDGainValue.fCoeff[0] = newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manIspDGain_val[0];
    pExpSwAttr->stManual.stHdrMe.IspDGainValue.fCoeff[1] = newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manIspDGain_val[1];
    pExpSwAttr->stManual.stHdrMe.IspDGainValue.fCoeff[2] = newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manIspDGain_val[2];
    pExpSwAttr->stManual.stHdrMe.ManualTimeEn            = newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_en;
    pExpSwAttr->stManual.stHdrMe.TimeValue.fCoeff[0]     = newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_val[0];
    pExpSwAttr->stManual.stHdrMe.TimeValue.fCoeff[1]     = newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_val[1];
    pExpSwAttr->stManual.stHdrMe.TimeValue.fCoeff[2]     = newExpSwAttr.commCtrl.meCtrl.hdrMe.sw_aeT_manTime_val[2];

    // advanced
    pExpSwAttr->stAdvanced.enable       = newExpSwAttr.advanced.sw_aeT_advGridWgt_en;
    pExpSwAttr->stAdvanced.SetAeRangeEn = newExpSwAttr.advanced.sw_aeT_advAeRange_en;
    memcpy(pExpSwAttr->stAdvanced.GridWeights, newExpSwAttr.advanced.sw_aeT_advGrid_wgt, sizeof(pExpSwAttr->stAdvanced.GridWeights));

    pExpSwAttr->stAdvanced.SetLinAeRange.stExpTimeRange.Max  = newExpSwAttr.advanced.linExpRange.sw_aeT_time_max;
    pExpSwAttr->stAdvanced.SetLinAeRange.stExpTimeRange.Min  = newExpSwAttr.advanced.linExpRange.sw_aeT_time_min;
    pExpSwAttr->stAdvanced.SetLinAeRange.stGainRange.Max     = newExpSwAttr.advanced.linExpRange.sw_aeT_gain_max;
    pExpSwAttr->stAdvanced.SetLinAeRange.stGainRange.Min     = newExpSwAttr.advanced.linExpRange.sw_aeT_gain_min;
    pExpSwAttr->stAdvanced.SetLinAeRange.stIspDGainRange.Max = newExpSwAttr.advanced.linExpRange.sw_aeT_ispDGain_max;
    pExpSwAttr->stAdvanced.SetLinAeRange.stIspDGainRange.Min = newExpSwAttr.advanced.linExpRange.sw_aeT_ispDGain_min;
    pExpSwAttr->stAdvanced.SetLinAeRange.stPIrisRange.Max    = newExpSwAttr.advanced.linExpRange.sw_aeT_pIrisGain_max;
    pExpSwAttr->stAdvanced.SetLinAeRange.stPIrisRange.Min    = newExpSwAttr.advanced.linExpRange.sw_aeT_pIrisGain_min;

    pExpSwAttr->stAdvanced.SetHdrAeRange.stExpTimeRange[0].Max  = newExpSwAttr.advanced.hdrExpRange[0].sw_aeT_time_max;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stExpTimeRange[1].Max  = newExpSwAttr.advanced.hdrExpRange[1].sw_aeT_time_max;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stExpTimeRange[2].Max  = newExpSwAttr.advanced.hdrExpRange[2].sw_aeT_time_max;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stExpTimeRange[0].Min  = newExpSwAttr.advanced.hdrExpRange[0].sw_aeT_time_min;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stExpTimeRange[1].Min  = newExpSwAttr.advanced.hdrExpRange[1].sw_aeT_time_min;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stExpTimeRange[2].Min  = newExpSwAttr.advanced.hdrExpRange[2].sw_aeT_time_min;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stGainRange[0].Max     = newExpSwAttr.advanced.hdrExpRange[0].sw_aeT_gain_max;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stGainRange[1].Max     = newExpSwAttr.advanced.hdrExpRange[1].sw_aeT_gain_max;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stGainRange[2].Max     = newExpSwAttr.advanced.hdrExpRange[2].sw_aeT_gain_max;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stGainRange[0].Min     = newExpSwAttr.advanced.hdrExpRange[0].sw_aeT_gain_min;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stGainRange[1].Min     = newExpSwAttr.advanced.hdrExpRange[1].sw_aeT_gain_min;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stGainRange[2].Min     = newExpSwAttr.advanced.hdrExpRange[2].sw_aeT_gain_min;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stIspDGainRange[0].Max = newExpSwAttr.advanced.hdrExpRange[0].sw_aeT_ispDGain_max;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stIspDGainRange[1].Max = newExpSwAttr.advanced.hdrExpRange[1].sw_aeT_ispDGain_max;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stIspDGainRange[2].Max = newExpSwAttr.advanced.hdrExpRange[2].sw_aeT_ispDGain_max;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stIspDGainRange[0].Min = newExpSwAttr.advanced.hdrExpRange[0].sw_aeT_ispDGain_min;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stIspDGainRange[1].Min = newExpSwAttr.advanced.hdrExpRange[1].sw_aeT_ispDGain_min;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stIspDGainRange[2].Min = newExpSwAttr.advanced.hdrExpRange[2].sw_aeT_ispDGain_min;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stPIrisRange.Max       = newExpSwAttr.advanced.hdrExpRange[0].sw_aeT_pIrisGain_max;
    pExpSwAttr->stAdvanced.SetHdrAeRange.stPIrisRange.Min       = newExpSwAttr.advanced.hdrExpRange[0].sw_aeT_pIrisGain_min;

    ae_api_linExpAttr_t linAttr;
    ret = rk_aiq_user_api2_ae_getLinExpAttr(sys_ctx, &linAttr);

    pExpSwAttr->stInitExp.stLinExpInitExp.InitTimeValue     = linAttr.initExp.sw_aeT_initTime_val;
    pExpSwAttr->stInitExp.stLinExpInitExp.InitGainValue     = linAttr.initExp.sw_aeT_initGain_val;
    pExpSwAttr->stInitExp.stLinExpInitExp.InitIspDGainValue = linAttr.initExp.sw_aeT_initIspDGain_val;

    ae_api_hdrExpAttr_t hdrAttr;
    ret = rk_aiq_user_api2_ae_getHdrExpAttr(sys_ctx, &hdrAttr);

    pExpSwAttr->stInitExp.stHdrExpInitExp.InitTimeValue.fCoeff[0]     = hdrAttr.initExp.sw_aeT_initTime_val[0];
    pExpSwAttr->stInitExp.stHdrExpInitExp.InitGainValue.fCoeff[0]     = hdrAttr.initExp.sw_aeT_initGain_val[0];
    pExpSwAttr->stInitExp.stHdrExpInitExp.InitIspDGainValue.fCoeff[0] = hdrAttr.initExp.sw_aeT_initIspDGain_val[0];
    pExpSwAttr->stInitExp.stHdrExpInitExp.InitTimeValue.fCoeff[1]     = hdrAttr.initExp.sw_aeT_initTime_val[1];
    pExpSwAttr->stInitExp.stHdrExpInitExp.InitGainValue.fCoeff[1]     = hdrAttr.initExp.sw_aeT_initGain_val[1];
    pExpSwAttr->stInitExp.stHdrExpInitExp.InitIspDGainValue.fCoeff[1] = hdrAttr.initExp.sw_aeT_initIspDGain_val[1];
    pExpSwAttr->stInitExp.stHdrExpInitExp.InitTimeValue.fCoeff[2]     = hdrAttr.initExp.sw_aeT_initTime_val[2];
    pExpSwAttr->stInitExp.stHdrExpInitExp.InitGainValue.fCoeff[2]     = hdrAttr.initExp.sw_aeT_initGain_val[2];
    pExpSwAttr->stInitExp.stHdrExpInitExp.InitIspDGainValue.fCoeff[2] = hdrAttr.initExp.sw_aeT_initIspDGain_val[2];

    ae_api_irisAttr_t irisAttr;
    ret = rk_aiq_user_api2_ae_getIrisAttr(sys_ctx, &irisAttr);

    pExpSwAttr->stInitExp.stLinExpInitExp.InitDCIrisDutyValue    = irisAttr.initIris.sw_aeT_initDCIrisHold_val;
    pExpSwAttr->stInitExp.stLinExpInitExp.InitHDCIrisTargetValue = irisAttr.initIris.sw_aeT_initHDCIrisGain_val;
    pExpSwAttr->stInitExp.stLinExpInitExp.InitPIrisGainValue     = irisAttr.initIris.sw_aeT_initPIrisGain_val;

    pExpSwAttr->stManual.stLinMe.ManualIrisEn   = irisAttr.manIris.sw_aeT_manIris_en;
    pExpSwAttr->stManual.stLinMe.PIrisGainValue = irisAttr.manIris.sw_aeT_manPIrisGain_val;
    pExpSwAttr->stManual.stLinMe.DCIrisValue    = irisAttr.manIris.sw_aeT_manDCIrisHold_val;
    pExpSwAttr->stManual.stLinMe.HDCIrisValue   = irisAttr.manIris.sw_aeT_manHDCIrisGain_val;

    pExpSwAttr->stIris.enable   = irisAttr.sw_aeT_iris_en;
    pExpSwAttr->stIris.IrisType = (CalibDb_IrisType_t)irisAttr.sw_aeT_iris_type;

    pExpSwAttr->stIris.PIrisAttr.ZeroIsMax = irisAttr.pIrisCtrl.sw_aeT_zeroIsMax_en;
    pExpSwAttr->stIris.PIrisAttr.EffcStep  = irisAttr.pIrisCtrl.sw_aeT_effcStep_val;
    pExpSwAttr->stIris.PIrisAttr.TotalStep = irisAttr.pIrisCtrl.sw_aeT_totalStep_val;
    memcpy(pExpSwAttr->stIris.PIrisAttr.StepTable, irisAttr.pIrisCtrl.sw_aeT_step2Gain_table, sizeof(pExpSwAttr->stIris.PIrisAttr.StepTable));

    pExpSwAttr->stIris.DCIrisAttr.Kp           = irisAttr.dcIrisCtrl.sw_aeT_dcIris_Kp;
    pExpSwAttr->stIris.DCIrisAttr.Ki           = irisAttr.dcIrisCtrl.sw_aeT_dcIris_Ki;
    pExpSwAttr->stIris.DCIrisAttr.Kd           = irisAttr.dcIrisCtrl.sw_aeT_dcIris_Kd;
    pExpSwAttr->stIris.DCIrisAttr.MinPwmDuty   = irisAttr.dcIrisCtrl.sw_aeT_pwmDuty_min;
    pExpSwAttr->stIris.DCIrisAttr.MaxPwmDuty   = irisAttr.dcIrisCtrl.sw_aeT_pwmDuty_max;
    pExpSwAttr->stIris.DCIrisAttr.OpenPwmDuty  = irisAttr.dcIrisCtrl.sw_aeT_pwmDuty_open;
    pExpSwAttr->stIris.DCIrisAttr.ClosePwmDuty = irisAttr.dcIrisCtrl.sw_aeT_pwmDuty_close;

    pExpSwAttr->stIris.HDCIrisAttr.DampOver  = irisAttr.hdcIrisCtrl.sw_aeT_damp_over;
    pExpSwAttr->stIris.HDCIrisAttr.DampUnder = irisAttr.hdcIrisCtrl.sw_aeT_damp_under;
    pExpSwAttr->stIris.HDCIrisAttr.ZeroIsMax = irisAttr.hdcIrisCtrl.sw_aeT_zeroIsMax_en;
    pExpSwAttr->stIris.HDCIrisAttr.MinTarget = irisAttr.hdcIrisCtrl.sw_aeT_target_min;
    pExpSwAttr->stIris.HDCIrisAttr.MaxTarget = irisAttr.hdcIrisCtrl.sw_aeT_target_max;

    pExpSwAttr->stIris.HDCIrisAttr.zoom_array_size = irisAttr.hdcIrisCtrl.zoom2Iris.sw_aeC_zoom2Iris_len;
    memcpy(pExpSwAttr->stIris.HDCIrisAttr.ZoomDot, irisAttr.hdcIrisCtrl.zoom2Iris.sw_aeC_zoom2Iris_idx, sizeof(pExpSwAttr->stIris.HDCIrisAttr.ZoomDot));
    memcpy(pExpSwAttr->stIris.HDCIrisAttr.ZoomTargetDot, irisAttr.hdcIrisCtrl.zoom2Iris.sw_aeC_zoom2Iris_val, sizeof(pExpSwAttr->stIris.HDCIrisAttr.ZoomTargetDot));
    pExpSwAttr->stIris.HDCIrisAttr.iris_array_size = irisAttr.hdcIrisCtrl.iris2Gain.sw_aeC_iris2Gain_len;
    memcpy(pExpSwAttr->stIris.HDCIrisAttr.IrisTargetDot, irisAttr.hdcIrisCtrl.iris2Gain.sw_aeC_iris2Gain_idx, sizeof(pExpSwAttr->stIris.HDCIrisAttr.IrisTargetDot));
    memcpy(pExpSwAttr->stIris.HDCIrisAttr.GainDot, irisAttr.hdcIrisCtrl.iris2Gain.sw_aeC_iris2Gain_val, sizeof(pExpSwAttr->stIris.HDCIrisAttr.GainDot));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_user_api_ae_setLinExpAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_LinExpAttr_t linExpAttr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    ae_api_linExpAttr_t linAttr;
    ret = rk_aiq_user_api2_ae_getLinExpAttr(sys_ctx, &linAttr);

    // RawStatsEn/NightSetPoint can't convert

    linAttr.sw_aeT_tolerance_in  = linExpAttr.ToleranceIn;
    linAttr.sw_aeT_tolerance_out = linExpAttr.ToleranceOut;
    linAttr.sw_aeT_evBias_strg   = linExpAttr.Evbias;
    if (linExpAttr.StrategyMode <= RKAIQ_AEC_STRATEGY_MODE_LOWLIGHT_PRIOR)
        linAttr.sw_aeT_strategy_mode = ae_strategy_lowlight_mode;
    else
        linAttr.sw_aeT_strategy_mode = ae_strategy_highlight_mode;

    linAttr.dynSetpoint.sw_aeT_dynSetpoint_len = linExpAttr.DySetpoint[AEC_DNMODE_DAY].array_size;
    for (int i = 0; i < linAttr.dynSetpoint.sw_aeT_dynSetpoint_len; i++) {
        linAttr.dynSetpoint.sw_aeT_expLevel_dot[i]    = linExpAttr.DySetpoint[AEC_DNMODE_DAY].ExpValue[i];
        linAttr.dynSetpoint.sw_aeT_dynSetpoint_dot[i] = linExpAttr.DySetpoint[AEC_DNMODE_DAY].DySetpoint[i];
    }

    if (linExpAttr.DySetPointEn == false) {
        for (int i = 0; i < linAttr.dynSetpoint.sw_aeT_dynSetpoint_len; i++)
            linAttr.dynSetpoint.sw_aeT_dynSetpoint_dot[i] = linExpAttr.SetPoint;
    }

    linAttr.backLightCtrl.sw_aeT_backLit_en       = (linExpAttr.BackLightConf.enable == 0) ? false : true;
    linAttr.backLightCtrl.sw_aeT_backLitBias_strg = linExpAttr.BackLightConf.StrBias;
    linAttr.backLightCtrl.sw_aeT_measArea_mode    = (ae_measArea_mode_t)linExpAttr.BackLightConf.MeasArea;
    linAttr.backLightCtrl.sw_aeT_oeROILow_thred   = linExpAttr.BackLightConf.OEROILowTh;
    linAttr.backLightCtrl.sw_aeT_lumaDist_thred   = linExpAttr.BackLightConf.LumaDistTh;
    linAttr.backLightCtrl.sw_aeT_loLv_thred       = linExpAttr.BackLightConf.LvLowTh;
    linAttr.backLightCtrl.sw_aeT_hiLv_thred       = linExpAttr.BackLightConf.LvHightTh;
    memcpy(linAttr.backLightCtrl.backLitSetpoint.sw_aeT_expLevel_dot, linExpAttr.BackLightConf.ExpLevel.fCoeff, sizeof(linExpAttr.BackLightConf.ExpLevel.fCoeff));
    memcpy(linAttr.backLightCtrl.backLitSetpoint.sw_aeT_nonOEPdfTh_dot, linExpAttr.BackLightConf.NonOEPdfTh.fCoeff, sizeof(linExpAttr.BackLightConf.NonOEPdfTh.fCoeff));
    memcpy(linAttr.backLightCtrl.backLitSetpoint.sw_aeT_loLitPdfTh_dot, linExpAttr.BackLightConf.LowLightPdfTh.fCoeff, sizeof(linExpAttr.BackLightConf.LowLightPdfTh.fCoeff));
    memcpy(linAttr.backLightCtrl.backLitSetpoint.sw_aeT_loLitSetpoint_dot, linExpAttr.BackLightConf.TargetLLLuma.fCoeff, sizeof(linExpAttr.BackLightConf.TargetLLLuma.fCoeff));
    linAttr.backLightCtrl.backLitSetpoint.sw_aeT_backLitSetpoint_len = 6;

    linAttr.overExpCtrl.sw_aeT_overExp_en       = (linExpAttr.OverExpCtrl.enable == 0) ? false : true;
    linAttr.overExpCtrl.sw_aeT_overExpBias_strg = linExpAttr.OverExpCtrl.StrBias;
    linAttr.overExpCtrl.sw_aeT_overExpWgt_max   = linExpAttr.OverExpCtrl.MaxWeight;
    linAttr.overExpCtrl.sw_aeT_hiLit_thred      = linExpAttr.OverExpCtrl.HighLightTh;
    linAttr.overExpCtrl.sw_aeT_loLit_thred      = linExpAttr.OverExpCtrl.LowLightTh;
    memcpy(linAttr.overExpCtrl.overExpSetpoint.sw_aeT_oePdf_dot, linExpAttr.OverExpCtrl.OEpdf.fCoeff, sizeof(linExpAttr.OverExpCtrl.OEpdf.fCoeff));
    memcpy(linAttr.overExpCtrl.overExpSetpoint.sw_aeT_loLitWgt_dot, linExpAttr.OverExpCtrl.LowLightWeight.fCoeff, sizeof(linExpAttr.OverExpCtrl.LowLightWeight.fCoeff));
    memcpy(linAttr.overExpCtrl.overExpSetpoint.sw_aeT_hiLitWgt_dot, linExpAttr.OverExpCtrl.HighLightWeight.fCoeff, sizeof(linExpAttr.OverExpCtrl.HighLightWeight.fCoeff));
    linAttr.overExpCtrl.overExpSetpoint.sw_aeT_overExpSetpoint_len = 6;

    rk_aiq_user_api2_ae_setLinExpAttr(sys_ctx, linAttr);

    return ret;
}

XCamReturn rk_aiq_user_api_ae_getLinExpAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_LinExpAttr_t* pLinExpAttr
)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    ae_api_linExpAttr_t linAttr;
    ret = rk_aiq_user_api2_ae_getLinExpAttr(sys_ctx, &linAttr);

    pLinExpAttr->RawStatsEn   = 1;
    pLinExpAttr->ToleranceIn  = linAttr.sw_aeT_tolerance_in;
    pLinExpAttr->ToleranceOut = linAttr.sw_aeT_tolerance_out;

    pLinExpAttr->Evbias = linAttr.sw_aeT_evBias_strg;

    if (linAttr.sw_aeT_strategy_mode <= ae_strategy_lowlight_mode)
        pLinExpAttr->StrategyMode = RKAIQ_AEC_STRATEGY_MODE_LOWLIGHT_PRIOR;
    else
        pLinExpAttr->StrategyMode = RKAIQ_AEC_STRATEGY_MODE_HIGHLIGHT_PRIOR;

    pLinExpAttr->SetPoint      = linAttr.dynSetpoint.sw_aeT_dynSetpoint_dot[0];
    pLinExpAttr->NightSetPoint = linAttr.dynSetpoint.sw_aeT_dynSetpoint_dot[0];
    pLinExpAttr->DySetPointEn  = true;

    pLinExpAttr->DySetpoint[AEC_DNMODE_DAY].array_size = MIN(AEC_SETPOINT_MAX_NODES, linAttr.dynSetpoint.sw_aeT_dynSetpoint_len);
    memcpy(pLinExpAttr->DySetpoint[AEC_DNMODE_DAY].ExpValue, linAttr.dynSetpoint.sw_aeT_expLevel_dot, pLinExpAttr->DySetpoint[AEC_DNMODE_DAY].array_size * sizeof(float));
    memcpy(pLinExpAttr->DySetpoint[AEC_DNMODE_DAY].DySetpoint, linAttr.dynSetpoint.sw_aeT_dynSetpoint_dot, pLinExpAttr->DySetpoint[AEC_DNMODE_DAY].array_size * sizeof(float));

    pLinExpAttr->BackLightConf.enable     = (linAttr.backLightCtrl.sw_aeT_backLit_en == false) ? 0 : 1;
    pLinExpAttr->BackLightConf.StrBias    = linAttr.backLightCtrl.sw_aeT_backLitBias_strg;
    pLinExpAttr->BackLightConf.MeasArea   = (CalibDb_AecMeasAreaMode_t)linAttr.backLightCtrl.sw_aeT_measArea_mode;
    pLinExpAttr->BackLightConf.OEROILowTh = linAttr.backLightCtrl.sw_aeT_oeROILow_thred;
    pLinExpAttr->BackLightConf.LumaDistTh = linAttr.backLightCtrl.sw_aeT_lumaDist_thred;
    pLinExpAttr->BackLightConf.LvLowTh    = linAttr.backLightCtrl.sw_aeT_loLv_thred;
    pLinExpAttr->BackLightConf.LvHightTh  = linAttr.backLightCtrl.sw_aeT_hiLv_thred;
    memcpy(pLinExpAttr->BackLightConf.ExpLevel.fCoeff, linAttr.backLightCtrl.backLitSetpoint.sw_aeT_expLevel_dot, sizeof(pLinExpAttr->BackLightConf.ExpLevel.fCoeff));
    memcpy(pLinExpAttr->BackLightConf.NonOEPdfTh.fCoeff, linAttr.backLightCtrl.backLitSetpoint.sw_aeT_nonOEPdfTh_dot, sizeof(pLinExpAttr->BackLightConf.NonOEPdfTh.fCoeff));
    memcpy(pLinExpAttr->BackLightConf.LowLightPdfTh.fCoeff, linAttr.backLightCtrl.backLitSetpoint.sw_aeT_loLitPdfTh_dot, sizeof(pLinExpAttr->BackLightConf.LowLightPdfTh.fCoeff));
    memcpy(pLinExpAttr->BackLightConf.TargetLLLuma.fCoeff, linAttr.backLightCtrl.backLitSetpoint.sw_aeT_loLitSetpoint_dot, sizeof(pLinExpAttr->BackLightConf.TargetLLLuma.fCoeff));

    pLinExpAttr->OverExpCtrl.enable      = (linAttr.overExpCtrl.sw_aeT_overExp_en == false) ? 0 : 1;
    pLinExpAttr->OverExpCtrl.StrBias     = linAttr.overExpCtrl.sw_aeT_overExpBias_strg;
    pLinExpAttr->OverExpCtrl.MaxWeight   = linAttr.overExpCtrl.sw_aeT_overExpWgt_max;
    pLinExpAttr->OverExpCtrl.HighLightTh = linAttr.overExpCtrl.sw_aeT_hiLit_thred;
    pLinExpAttr->OverExpCtrl.LowLightTh  = linAttr.overExpCtrl.sw_aeT_loLit_thred;
    memcpy(pLinExpAttr->OverExpCtrl.OEpdf.fCoeff, linAttr.overExpCtrl.overExpSetpoint.sw_aeT_oePdf_dot, sizeof(pLinExpAttr->OverExpCtrl.OEpdf.fCoeff));
    memcpy(pLinExpAttr->OverExpCtrl.LowLightWeight.fCoeff, linAttr.overExpCtrl.overExpSetpoint.sw_aeT_loLitWgt_dot, sizeof(pLinExpAttr->OverExpCtrl.LowLightWeight.fCoeff));
    memcpy(pLinExpAttr->OverExpCtrl.HighLightWeight.fCoeff, linAttr.overExpCtrl.overExpSetpoint.sw_aeT_hiLitWgt_dot, sizeof(pLinExpAttr->OverExpCtrl.HighLightWeight.fCoeff));

    return ret;
}
