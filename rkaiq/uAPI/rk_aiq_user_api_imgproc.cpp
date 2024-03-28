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

#include "rk_aiq_user_api_imgproc.h"

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
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)ctx;
        mode = camgroup_ctx->cam_ctxs_array[0]->_analyzer->mAlogsComSharedParams.working_mode;
#endif
    } else {
        mode = ctx->_analyzer->mAlogsComSharedParams.working_mode;
    }

    if (RK_AIQ_WORKING_MODE_NORMAL == mode)
        return false;
    else
        return true;
}

static int getHDRFrameNum(const rk_aiq_sys_ctx_t* ctx)
{
    RKAIQ_API_SMART_LOCK(ctx);
    int FrameNum = 1, working_mode = RK_AIQ_WORKING_MODE_NORMAL;

    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)ctx;
        working_mode = camgroup_ctx->cam_ctxs_array[0]->_analyzer->mAlogsComSharedParams.working_mode;
#endif
    } else {
        working_mode = ctx->_analyzer->mAlogsComSharedParams.working_mode;
    }

    switch (working_mode)
    {
    case RK_AIQ_WORKING_MODE_NORMAL:
        FrameNum = 1;
        break;
    case RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR:
    case RK_AIQ_ISP_HDR_MODE_2_LINE_HDR:
        FrameNum = 2;
        break;
    case RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR:
    case RK_AIQ_ISP_HDR_MODE_3_LINE_HDR:
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
*                        API of AEC module of V1
**********************************************************
*/

/*
*****************************
*
* Desc: set exposure control mode
* Argument:
*   mode:  auto: auto exposure
*          manual: manual exposure
*****************************
*/
XCamReturn rk_aiq_uapi_setExpMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{
    IMGPROC_FUNC_ENTER
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttr_t expSwAttr;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }

    ret = rk_aiq_user_api_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setExpMode failed in getExpSwAttr!");
    if (mode == OP_AUTO) {
        expSwAttr.AecOpType = RK_AIQ_OP_MODE_AUTO;
    } else if (mode == OP_MANUAL) {
        if (isHDRmode(ctx)) {
            expSwAttr.AecOpType = RK_AIQ_OP_MODE_MANUAL;
            expSwAttr.stManual.stHdrMe.ManualGainEn = true;
            expSwAttr.stManual.stHdrMe.ManualTimeEn = true;
        } else {
            expSwAttr.AecOpType = RK_AIQ_OP_MODE_MANUAL;
            expSwAttr.stManual.stLinMe.ManualGainEn = true;
            expSwAttr.stManual.stLinMe.ManualTimeEn = true;
        }
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "mode is not supported!");
    }
    ret = rk_aiq_user_api_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setExpMode failed in setExpSwAttr!");
    IMGPROC_FUNC_EXIT
    return (ret);
}

XCamReturn rk_aiq_uapi_getExpMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (mode == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }

    ret = rk_aiq_user_api_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getExpMode failed in getExpSwAttr!");
    if (expSwAttr.AecOpType == RK_AIQ_OP_MODE_AUTO) {
        *mode = OP_AUTO;
    } else if (expSwAttr.AecOpType == RK_AIQ_OP_MODE_MANUAL) {
        *mode = OP_MANUAL;
    }
    IMGPROC_FUNC_EXIT
    return (ret);
}

XCamReturn rk_aiq_uapi_setManualExp(const rk_aiq_sys_ctx_t* ctx, float gain, float time)
{
    IMGPROC_FUNC_ENTER
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttr_t expSwAttr;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }
    if (gain < 0.0f || time < 0.0f) {
        ret = XCAM_RETURN_NO_ERROR;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }
    ret = rk_aiq_user_api_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setManualExp failed in getExpSwAttr!");
    if (isHDRmode(ctx)) {
        expSwAttr.AecOpType = RK_AIQ_OP_MODE_MANUAL;
        expSwAttr.stManual.stHdrMe.ManualGainEn = true;
        expSwAttr.stManual.stHdrMe.ManualTimeEn = true;
        expSwAttr.stManual.stHdrMe.GainValue.fCoeff[0] = gain;
        expSwAttr.stManual.stHdrMe.GainValue.fCoeff[1] = gain;
        expSwAttr.stManual.stHdrMe.GainValue.fCoeff[2] = gain;
        expSwAttr.stManual.stHdrMe.TimeValue.fCoeff[0] = time;
        expSwAttr.stManual.stHdrMe.TimeValue.fCoeff[1] = time;
        expSwAttr.stManual.stHdrMe.TimeValue.fCoeff[2] = time;
    } else {
        expSwAttr.AecOpType = RK_AIQ_OP_MODE_MANUAL;
        expSwAttr.stManual.stLinMe.ManualGainEn = true;
        expSwAttr.stManual.stLinMe.ManualTimeEn = true;
        expSwAttr.stManual.stLinMe.GainValue = gain;
        expSwAttr.stManual.stLinMe.TimeValue = time;
    }
    ret = rk_aiq_user_api_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setManualExp failed in setExpSwAttr!");
    IMGPROC_FUNC_EXIT
    return (ret);
}

/*
*****************************
*
* Desc: set exposure parameter
* Argument:
*    auto exposure mode:
*      exposure gain will be adjust between [gain->min, gain->max]锛?*    manual exposure mode:
*      gain->min == gain->max
*
*****************************
*/
XCamReturn rk_aiq_uapi_setExpGainRange(const rk_aiq_sys_ctx_t* ctx, paRange_t *gain)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (gain == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param invalid!");
    }
    LOGD("set range: [%f, %f]", gain->min, gain->max);

    if (gain->min < 1.0f || gain->max < 1.0f || (gain->min - gain->max) > 0.0000001 ) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "gain range is wrong!");
    }
    ret = rk_aiq_user_api_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetExpGainRange failed!");

    expSwAttr.stAuto.SetAeRangeEn = true;
    if (isHDRmode(ctx)) {
        expSwAttr.stAuto.stHdrAeRange.stGainRange[0].Max = gain->max;
        expSwAttr.stAuto.stHdrAeRange.stGainRange[0].Min = gain->min;
        expSwAttr.stAuto.stHdrAeRange.stGainRange[1].Max = gain->max;
        expSwAttr.stAuto.stHdrAeRange.stGainRange[1].Min = gain->min;
        expSwAttr.stAuto.stHdrAeRange.stGainRange[2].Max = gain->max;
        expSwAttr.stAuto.stHdrAeRange.stGainRange[2].Min = gain->min;
    } else {
        expSwAttr.stAuto.stLinAeRange.stGainRange.Max = gain->max;
        expSwAttr.stAuto.stLinAeRange.stGainRange.Min = gain->min;
    }
    ret = rk_aiq_user_api_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\nsetExpGainRange failed!");
    IMGPROC_FUNC_EXIT
    return (ret);
}

XCamReturn rk_aiq_uapi_getExpGainRange(const rk_aiq_sys_ctx_t* ctx, paRange_t *gain)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (gain == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\ngetExpGainRange failed!");
    if (isHDRmode(ctx)) {
        int index = getHDRFrameNum(ctx);
        gain->max = expSwAttr.stAuto.stHdrAeRange.stGainRange[index - 1].Max;
        gain->min = expSwAttr.stAuto.stHdrAeRange.stGainRange[index - 1].Min;
    } else {
        gain->max = expSwAttr.stAuto.stLinAeRange.stGainRange.Max;
        gain->min = expSwAttr.stAuto.stLinAeRange.stGainRange.Min;
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
*       exposure time will be adjust between [time->min, time->max]锛?*    manual exposure mode:
*       exposure time will be set gain->min == gain->max;
*
*****************************
*/
XCamReturn rk_aiq_uapi_setExpTimeRange(const rk_aiq_sys_ctx_t* ctx, paRange_t *time)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (time == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    if ( (time->min - time->max) > 0.0000001 ) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "time range is wrong!");
    }
    LOGD("set range: [%f, %f]", time->min, time->max);
    ret = rk_aiq_user_api_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetExpTimeRange failed!");

    expSwAttr.stAuto.SetAeRangeEn = true;
    if (isHDRmode(ctx)) {
        expSwAttr.stAuto.stHdrAeRange.stExpTimeRange[0].Max = time->max;
        expSwAttr.stAuto.stHdrAeRange.stExpTimeRange[0].Min = time->min;
        expSwAttr.stAuto.stHdrAeRange.stExpTimeRange[1].Max = time->max;
        expSwAttr.stAuto.stHdrAeRange.stExpTimeRange[1].Min = time->min;
        expSwAttr.stAuto.stHdrAeRange.stExpTimeRange[2].Max = time->max;
        expSwAttr.stAuto.stHdrAeRange.stExpTimeRange[2].Min = time->min;
    } else {
        expSwAttr.stAuto.stLinAeRange.stExpTimeRange.Max = time->max;
        expSwAttr.stAuto.stLinAeRange.stExpTimeRange.Min = time->min;
    }
    ret = rk_aiq_user_api_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\nsetExpTimeRange failed!");
    IMGPROC_FUNC_EXIT
    return (ret);
}

XCamReturn rk_aiq_uapi_getExpTimeRange(const rk_aiq_sys_ctx_t* ctx, paRange_t *time)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (time == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\ngetExpTimeRange failed!");
    if (isHDRmode(ctx)) {
        int index = getHDRFrameNum(ctx);
        time->max = expSwAttr.stAuto.stHdrAeRange.stExpTimeRange[index - 1].Max;
        time->min = expSwAttr.stAuto.stHdrAeRange.stExpTimeRange[index - 1].Min;
    } else {
        time->max = expSwAttr.stAuto.stLinAeRange.stExpTimeRange.Max;
        time->min = expSwAttr.stAuto.stLinAeRange.stExpTimeRange.Min;
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

    Uapi_LinExpAttr_t lineExpAttr;
    memset(&lineExpAttr, 0x00, sizeof(Uapi_LinExpAttr_t));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    if (isHDRmode(ctx)) {
        ret = XCAM_RETURN_ERROR_FAILED;
        RKAIQ_IMGPROC_CHECK_RET(ret, "Not support in HDR mode!");
    }

    ret = rk_aiq_user_api_ae_getLinExpAttr(ctx, &lineExpAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getLinExpAttr error!");
    lineExpAttr.BackLightConf.enable = on ? 1 : 0;
    lineExpAttr.BackLightConf.MeasArea = (CalibDb_AecMeasAreaMode_t)areaType;
    lineExpAttr.BackLightConf.StrBias = 0;
    ret = rk_aiq_user_api_ae_setLinExpAttr(ctx, lineExpAttr);
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
    Uapi_LinExpAttr_t lineExpAttr;
    memset(&lineExpAttr, 0x00, sizeof(Uapi_LinExpAttr_t));

    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    if (isHDRmode(ctx)) {
        ret = XCAM_RETURN_ERROR_FAILED;
        RKAIQ_IMGPROC_CHECK_RET(ret, "Not support in HDR mode!");
    } else {
        ret = rk_aiq_user_api_ae_getLinExpAttr(ctx, &lineExpAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getLinExpAttr error!");
        if(0 == lineExpAttr.BackLightConf.enable)
            RKAIQ_IMGPROC_CHECK_RET(ret, "blc mode is not enabled!");
        lineExpAttr.BackLightConf.StrBias = strength;
        ret = rk_aiq_user_api_ae_setLinExpAttr(ctx, lineExpAttr);
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
    Uapi_LinExpAttr_t linExpAttr;
    memset(&linExpAttr, 0x00, sizeof(Uapi_LinExpAttr_t));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    if (isHDRmode(ctx)) {
        ret = XCAM_RETURN_ERROR_FAILED;
        RKAIQ_IMGPROC_CHECK_RET(ret, "Not support in HDR mode!");
    } else {
        ret = rk_aiq_user_api_ae_getLinExpAttr(ctx, &linExpAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\n setHLCMode failed!");
        linExpAttr.OverExpCtrl.enable = on ? 1 : 0;
        linExpAttr.OverExpCtrl.StrBias = 0;
        ret = rk_aiq_user_api_ae_setLinExpAttr(ctx, linExpAttr);
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
    Uapi_LinExpAttr_t lineExpAttr;
    memset(&lineExpAttr, 0x00, sizeof(Uapi_LinExpAttr_t));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    if (isHDRmode(ctx)) {
        ret = XCAM_RETURN_ERROR_FAILED;
        RKAIQ_IMGPROC_CHECK_RET(ret, "Not support in HDR mode!");
    } else {
        ret = rk_aiq_user_api_ae_getLinExpAttr(ctx, &lineExpAttr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getLinExpAttr error!");
        if(0 == lineExpAttr.OverExpCtrl.enable)
            RKAIQ_IMGPROC_CHECK_RET(ret, "hlc mode is not enabled!");
        for(int i = 0; i < 6; i++)
            lineExpAttr.OverExpCtrl.HighLightWeight.fCoeff[i] = (float)strength / 100.0f * lineExpAttr.OverExpCtrl.MaxWeight;
        ret = rk_aiq_user_api_ae_setLinExpAttr(ctx, lineExpAttr);
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

XCamReturn rk_aiq_uapi_setAntiFlickerMode(const rk_aiq_sys_ctx_t* ctx, antiFlickerMode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetAntiFlickerMode failed!");
    if (mode == ANTIFLICKER_AUTO_MODE) {
        expSwAttr.stAntiFlicker.enable = true;
        expSwAttr.stAntiFlicker.Mode = AEC_ANTIFLICKER_AUTO_MODE;
    } else if(mode == ANTIFLICKER_NORMAL_MODE) {
        expSwAttr.stAntiFlicker.enable = true;
        expSwAttr.stAntiFlicker.Mode = AEC_ANTIFLICKER_NORMAL_MODE;
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "mode is invalid!");
    }
    ret = rk_aiq_user_api_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\nsetAntiFlickerMode failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_getAntiFlickerMode(const rk_aiq_sys_ctx_t* ctx, antiFlickerMode_t *mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\ngetAntiFlickerMode!");
    if (expSwAttr.stAntiFlicker.Mode == AEC_ANTIFLICKER_AUTO_MODE)
        *mode = ANTIFLICKER_AUTO_MODE;
    else if (expSwAttr.stAntiFlicker.Mode == AEC_ANTIFLICKER_NORMAL_MODE)
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
    Uapi_ExpSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    ret = rk_aiq_user_api_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetExpPwrLineFreqMode failed!");
    if (freq == EXP_PWR_LINE_FREQ_50HZ) {
        expSwAttr.stAntiFlicker.enable = true;
        expSwAttr.stAntiFlicker.Frequency = AEC_FLICKER_FREQUENCY_50HZ;
    } else if (freq == EXP_PWR_LINE_FREQ_60HZ) {
        expSwAttr.stAntiFlicker.enable = true;
        expSwAttr.stAntiFlicker.Frequency = AEC_FLICKER_FREQUENCY_60HZ;
    } else if (freq == EXP_PWR_LINE_FREQ_DIS) {
        expSwAttr.stAntiFlicker.enable = true;
        expSwAttr.stAntiFlicker.Frequency = AEC_FLICKER_FREQUENCY_OFF;
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "freq is invalid!");
    }
    ret = rk_aiq_user_api_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\nsetExpPwrLineFreqMode failed!");
    IMGPROC_FUNC_EXIT
    return (ret);
}

XCamReturn rk_aiq_uapi_getExpPwrLineFreqMode(const rk_aiq_sys_ctx_t* ctx, expPwrLineFreq_t *freq)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }

    ret = rk_aiq_user_api_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\nsetExpPwrLineFreqMode failed!");
    if (expSwAttr.stAntiFlicker.Frequency == AEC_FLICKER_FREQUENCY_50HZ) {
        *freq = EXP_PWR_LINE_FREQ_50HZ;
    } else if (expSwAttr.stAntiFlicker.Frequency == AEC_FLICKER_FREQUENCY_60HZ) {
        *freq = EXP_PWR_LINE_FREQ_60HZ;
    } else if (expSwAttr.stAntiFlicker.Frequency == AEC_FLICKER_FREQUENCY_OFF) {
        *freq = EXP_PWR_LINE_FREQ_DIS;
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "freq is invalid!");
    }
    IMGPROC_FUNC_EXIT
    return (ret);
}

/*
*****************************
*
* Desc: set fill light mode
* Argument:
*    mode
*
*****************************
*/
XCamReturn rk_aiq_uapi_setFLightMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    return ret;
}

XCamReturn rk_aiq_uapi_getFLightMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    return ret;
}

/*
*****************************
*
* Desc: set maual fill light mode
* Argument:
*    on:  1: on
*         0: off
*
*****************************
*/
XCamReturn rk_aiq_uapi_setMFLight(const rk_aiq_sys_ctx_t* ctx, bool on)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    return ret;
}

XCamReturn rk_aiq_uapi_getMFLight(const rk_aiq_sys_ctx_t* ctx, bool *on)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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
XCamReturn rk_aiq_uapi_setWBMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_wb_attrib_t attr;
    IMGPROC_FUNC_ENTER
    if (mode >= OP_INVAL || mode < OP_AUTO) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "mode is invalid!");
    }
    ret = rk_aiq_user_api_awb_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setWBMode failed in getting awb attrib!");

    if (mode == OP_AUTO) {
        attr.mode = RK_AIQ_WB_MODE_AUTO;
    } else if (mode == OP_MANUAL) {
        rk_aiq_wb_querry_info_t wb_querry_info;
        ret = rk_aiq_user_api_awb_QueryWBInfo(ctx, &wb_querry_info);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setWBMode failed in query info!");
        attr.mode = RK_AIQ_WB_MODE_MANUAL;
        attr.stManual.mode = RK_AIQ_MWB_MODE_WBGAIN;
        attr.stManual.para.gain = wb_querry_info.gain;
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "Not supported mode!");
    }
    ret = rk_aiq_user_api_awb_SetAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setWbMode failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_getWBMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_wb_attrib_t attr;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_awb_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getWBMode failed!");
    if (attr.mode == RK_AIQ_WB_MODE_AUTO) {
        *mode = OP_AUTO;
    } else if (attr.mode == RK_AIQ_WB_MODE_MANUAL) {
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
    ret = rk_aiq_user_api_awb_Lock(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_unlockAWB(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_awb_Unlock(ctx);
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
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_wb_attrib_t attr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, setMWBScene failed!");
    }

    if (scene < RK_AIQ_WBCT_INCANDESCENT || scene > RK_AIQ_WBCT_SHADE) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "invalid scene mode, setMWBScene failed!");
    }

    ret = rk_aiq_user_api_awb_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMWBScene failed!");
    attr.mode = RK_AIQ_WB_MODE_MANUAL;
    attr.stManual.mode = RK_AIQ_MWB_MODE_SCENE;
    attr.stManual.para.scene = scene;
    ret = rk_aiq_user_api_awb_SetAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMWBScene failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_getMWBScene(const rk_aiq_sys_ctx_t* ctx, rk_aiq_wb_scene_t *scene)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_wb_attrib_t attr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (scene == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getMWBScene failed!");
    }
    //attr.mode = RK_AIQ_WB_MODE_MANUAL;
    attr.stManual.mode = RK_AIQ_MWB_MODE_SCENE;
    ret = rk_aiq_user_api_awb_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMWBScene failed!");
    *scene = attr.stManual.para.scene;
    IMGPROC_FUNC_EXIT
    return ret;
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
    rk_aiq_wb_attrib_t attr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (gain == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, setMWBGain failed!");
    }

    ret = rk_aiq_user_api_awb_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMWBGain failed!");
    attr.mode = RK_AIQ_WB_MODE_MANUAL;
    attr.stManual.mode = RK_AIQ_MWB_MODE_WBGAIN;
    attr.stManual.para.gain = *gain;
    ret = rk_aiq_user_api_awb_SetAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMWBGain failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_getMWBGain(const rk_aiq_sys_ctx_t* ctx, rk_aiq_wb_gain_t *gain)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_wb_querry_info_t query_info;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (gain == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getMWBGain failed!");
    }
    ret = rk_aiq_user_api_awb_QueryWBInfo(ctx, &query_info);
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
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_wb_attrib_t attr;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, setMWBCT failed!");
    }

    ret = rk_aiq_user_api_awb_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMWBCT failed!");
    attr.mode = RK_AIQ_WB_MODE_MANUAL;
    attr.stManual.mode = RK_AIQ_MWB_MODE_CCT;
    attr.stManual.para.cct.CCT = (float)ct;
    attr.stManual.para.cct.CCRI = 0.0f;
    ret = rk_aiq_user_api_awb_SetAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMWBCT failed!");
    IMGPROC_FUNC_EXIT
    return ret;

}

XCamReturn rk_aiq_uapi_getMWBCT(const rk_aiq_sys_ctx_t* ctx, unsigned int *ct)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_wb_cct_t cct;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (ct == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getMWBCT failed!");
    }

    ret = rk_aiq_user_api_awb_GetCCT(ctx, &cct);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMWBCT failed!");
    *ct = (unsigned int)cct.CCT;
    IMGPROC_FUNC_EXIT
    return ret;
}


/*
*****************************
*
* Desc: set color supperssion level
* Argument:
*   level: [0, 100]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setCrSuppsn(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
#if RKAIQ_HAVE_CCM_V1
    rk_aiq_ccm_attrib_t ccm;
    rk_aiq_user_api_accm_GetAttrib(ctx, &ccm);
    for(int i = 0; i < RK_AIQ_ACCM_COLOR_GAIN_NUM; i++) {
        ccm.stAuto.color_inhibition.sensorGain[i] = 2.0f;
        ccm.stAuto.color_inhibition.level[i] = level;
    }
    ret = rk_aiq_user_api_accm_SetAttrib(ctx, &ccm);
#elif RKAIQ_HAVE_CCM_V2
    rk_aiq_ccm_v2_attrib_t ccm;
    rk_aiq_user_api_accm_v2_GetAttrib(ctx, &ccm);
    for(int i = 0; i < RK_AIQ_ACCM_COLOR_GAIN_NUM; i++) {
        ccm.stAuto.color_inhibition.sensorGain[i] = 2.0f;
        ccm.stAuto.color_inhibition.level[i] = level;
    }
    ret = rk_aiq_user_api_accm_v2_SetAttrib(ctx, &ccm);
#endif
    RKAIQ_IMGPROC_CHECK_RET(ret, "setCrSuppsn failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}


XCamReturn rk_aiq_uapi_getCrSuppsn(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    rk_aiq_ccm_querry_info_t ccm_querry_info;
    ret = rk_aiq_user_api_accm_QueryCcmInfo(ctx, &ccm_querry_info);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getCrSuppsn failed!");
    *level = (int)ccm_querry_info.color_inhibition_level;
    IMGPROC_FUNC_EXIT
    return ret;
}


/*
**********************************************************
* Focus & Zoom
**********************************************************
*/
/*
*****************************
*
* Desc: set focus mode
* Argument:
*   mode:  auto: auto focus
*          manual: manual focus
*          semi-auto: semi-auto focus
*****************************
*/
XCamReturn rk_aiq_uapi_setFocusMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_af_attrib_t attr;
    ret = rk_aiq_user_api_af_GetAttrib(ctx, &attr);
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

    ret = rk_aiq_user_api_af_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setFocusMode failed!");
    return ret;
}

XCamReturn rk_aiq_uapi_getFocusMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_af_attrib_t attr;
    ret = rk_aiq_user_api_af_GetAttrib(ctx, &attr);
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

/*
*****************************
*
* Desc: set fixed mode code
* Argument:
*
*****************************
*/
XCamReturn rk_aiq_uapi_setFixedModeCode(const rk_aiq_sys_ctx_t* ctx, short code)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_af_attrib_t attr;

    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setFixedModeCode failed!");
    attr.fixedModeDefCode = code;
    ret = rk_aiq_user_api_af_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setFixedModeCode failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_getFixedModeCode(const rk_aiq_sys_ctx_t* ctx, short *code)
{
    bool zoom_support = ctx->_analyzer->mAlogsComSharedParams.snsDes.lens_des.zoom_support;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_af_attrib_t attr;
    IMGPROC_FUNC_ENTER
    if (zoom_support) {
        ret = rk_aiq_user_api_af_GetAttrib(ctx, &attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getFixedModeCode failed!");
        *code = attr.fixedModeDefCode;
    } else {
        int pos;
        ret = rk_aiq_user_api_af_GetFocusPos(ctx, &pos);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getFixedModeCode failed!");
        *code = pos;
    }
    // LOGD_AF("%s: focus position %d", __func__, *code);
    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set focus window
* Argument:
*
*****************************
*/

XCamReturn rk_aiq_uapi_setFocusWin(const rk_aiq_sys_ctx_t* ctx, paRect_t *rect)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_af_attrib_t attr;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setFocusWin failed!");

    attr.h_offs = rect->x;
    attr.v_offs = rect->y;
    attr.h_size = rect->w;
    attr.v_size = rect->h;
    ret = rk_aiq_user_api_af_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setFocusWin failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_getFocusWin(const rk_aiq_sys_ctx_t* ctx, paRect_t *rect)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_af_attrib_t attr;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getFocusWin failed!");

    rect->x = attr.h_offs;
    rect->y = attr.v_offs;
    rect->w = attr.h_size;
    rect->h = attr.v_size;
    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set focus meas config
* Argument:
*
*****************************
*/
XCamReturn rk_aiq_uapi_setFocusMeasCfg(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_algo_meas_t* meascfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_af_attrib_t attr;
    ret = rk_aiq_user_api_af_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setFocusMeasCfg failed!");
    attr.manual_meascfg = *meascfg;

    ret = rk_aiq_user_api_af_SetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setFocusMeasCfg failed!");
    return ret;
}

XCamReturn rk_aiq_uapi_getFocusMeasCfg(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_algo_meas_t* meascfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_af_attrib_t attr;
    ret = rk_aiq_user_api_af_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getFocusMeasCfg failed!");
    *meascfg = attr.manual_meascfg;

    return ret;
}

XCamReturn rk_aiq_uapi_lockFocus(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_Lock(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_unlockFocus(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_Unlock(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_oneshotFocus(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_Oneshot(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_manualTrigerFocus(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_ManualTriger(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_trackingFocus(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_Tracking(ctx);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_setVcmCfg(const rk_aiq_sys_ctx_t* ctx, rk_aiq_lens_vcmcfg* cfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_SetVcmCfg(ctx, cfg);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_getVcmCfg(const rk_aiq_sys_ctx_t* ctx, rk_aiq_lens_vcmcfg* cfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_GetVcmCfg(ctx, cfg);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_getSearchPath(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_sec_path_t* path)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_GetSearchPath(ctx, path);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_getSearchResult(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_result_t* result)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_GetSearchResult(ctx, result);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_setOpZoomPosition(const rk_aiq_sys_ctx_t* ctx, int pos)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_SetZoomIndex(ctx, pos);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_getOpZoomPosition(const rk_aiq_sys_ctx_t* ctx, int *pos)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_GetZoomIndex(ctx, pos);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_endOpZoomChange(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_EndZoomChg(ctx);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_getZoomRange(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_zoomrange * range)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_GetZoomRange(ctx, range);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_getFocusRange(const rk_aiq_sys_ctx_t* ctx, rk_aiq_af_focusrange* range)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_GetFocusRange(ctx, range);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_startZoomCalib(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_StartZoomCalib(ctx);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_resetZoom(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_resetZoom(ctx);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_setAngleZ(const rk_aiq_sys_ctx_t* ctx, float angleZ)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api_af_setAngleZ(ctx, angleZ);
    IMGPROC_FUNC_EXIT

    return ret;
}

/*
*****************************
*
* Desc: set minimum focus distance
* Argument:
*   disrance:  unint is cm
*****************************
*/
XCamReturn rk_aiq_uapi_setMinFocusDis(const rk_aiq_sys_ctx_t* ctx, unsigned int distance)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    return ret;
}

XCamReturn rk_aiq_uapi_getMinFocusDis(const rk_aiq_sys_ctx_t* ctx, unsigned int *distance)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    return ret;
}


/*
*****************************
*
* Desc: set optical zoom range
* Argument:
*   range:  [1.0, 100.0]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setOpZoomRange(const rk_aiq_sys_ctx_t* ctx, paRange_t *range)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    return ret;
}

XCamReturn rk_aiq_uapi_getOpZoomRange(const rk_aiq_sys_ctx_t* ctx, paRange_t *range)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    return ret;
}

/*
*****************************
*
* Desc: set optical zoom speed
* Argument:
*   level:  [1, 10]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setOpZoomSpeed(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    return ret;
}

XCamReturn rk_aiq_uapi_getOpZoomSpeed(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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
*     manual锛歮anual noise reduction
*
*****************************
*/
XCamReturn rk_aiq_uapi_setNRMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER

    if (CHECK_ISP_HW_V20()) {
        rk_aiq_nr_attrib_t attr;
        ret = rk_aiq_user_api_anr_GetAttrib(ctx, &attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "get anr attrib failed!,ret=%d", ret);
        if (mode == OP_AUTO) {
            attr.eMode = ANR_OP_MODE_AUTO;
        } else if (mode == OP_MANUAL) {
            attr.eMode = ANR_OP_MODE_MANUAL;
        } else {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "Not supported mode!");
        }
        ret = rk_aiq_user_api_anr_SetAttrib(ctx, &attr);
    }

    if (CHECK_ISP_HW_V21()) {
        rk_aiq_ynr_attrib_v2_t ynrV2_attr;
        rk_aiq_bayernr_attrib_v2_t bayernrV2_attr;
        rk_aiq_cnr_attrib_v1_t cnrV1_attr;
        ret = rk_aiq_user_api_aynrV2_GetAttrib(ctx, &ynrV2_attr);
        ret = rk_aiq_user_api_abayernrV2_GetAttrib(ctx, &bayernrV2_attr);
        ret = rk_aiq_user_api_acnrV1_GetAttrib(ctx, &cnrV1_attr);
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
        ret = rk_aiq_user_api_aynrV2_SetAttrib(ctx, &ynrV2_attr);
        ret = rk_aiq_user_api_abayernrV2_SetAttrib(ctx, &bayernrV2_attr);
        ret = rk_aiq_user_api_acnrV1_SetAttrib(ctx, &cnrV1_attr);
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
        ret = rk_aiq_user_api_aynrV3_GetAttrib(ctx, &ynrV3_attr);
        ret = rk_aiq_user_api_acnrV2_GetAttrib(ctx, &cnrV2_attr);
        ret = rk_aiq_user_api_abayer2dnrV2_SetAttrib(ctx, &bayer2dnrV2_attr);
        ret = rk_aiq_user_api_abayertnrV2_SetAttrib(ctx, &bayertnrV2_attr);
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
        ret = rk_aiq_user_api_aynrV3_SetAttrib(ctx, &ynrV3_attr);
        ret = rk_aiq_user_api_acnrV2_SetAttrib(ctx, &cnrV2_attr);
        ret = rk_aiq_user_api_abayer2dnrV2_SetAttrib(ctx, &bayer2dnrV2_attr);
        ret = rk_aiq_user_api_abayertnrV2_SetAttrib(ctx, &bayertnrV2_attr);
    }


    if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        rk_aiq_ynr_attrib_v22_t ynrV22_attr;
        ynrV22_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        rk_aiq_cnr_attrib_v30_t cnrV30_attr;
        cnrV30_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        rk_aiq_bayer2dnr_attrib_v23_t bayer2dnrV23_attr;
        bayer2dnrV23_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        rk_aiq_bayertnr_attrib_v23_t bayertnrV23_attr;
        bayertnrV23_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        ret = rk_aiq_user_api_aynrV22_GetAttrib(ctx, &ynrV22_attr);
        ret = rk_aiq_user_api_acnrV30_GetAttrib(ctx, &cnrV30_attr);
        ret = rk_aiq_user_api_abayer2dnrV23_GetAttrib(ctx, &bayer2dnrV23_attr);
        ret = rk_aiq_user_api_abayertnrV23_GetAttrib(ctx, &bayertnrV23_attr);
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
        ret = rk_aiq_user_api_aynrV22_SetAttrib(ctx, &ynrV22_attr);
        ret = rk_aiq_user_api_acnrV30_SetAttrib(ctx, &cnrV30_attr);
        ret = rk_aiq_user_api_abayer2dnrV23_SetAttrib(ctx, &bayer2dnrV23_attr);
        ret = rk_aiq_user_api_abayertnrV23_SetAttrib(ctx, &bayertnrV23_attr);
    }
    RKAIQ_IMGPROC_CHECK_RET(ret, "setNRMode failed!", ret);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_getNRMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER

    if (CHECK_ISP_HW_V20()) {
        rk_aiq_nr_attrib_t attr;
        ret = rk_aiq_user_api_anr_GetAttrib(ctx, &attr);
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
        ret = rk_aiq_user_api_aynrV2_GetAttrib(ctx, &ynrV2_attr);
        ret = rk_aiq_user_api_abayernrV2_GetAttrib(ctx, &bayernrV2_attr);
        ret = rk_aiq_user_api_acnrV1_GetAttrib(ctx, &cnrV1_attr);
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
        ret = rk_aiq_user_api_aynrV3_GetAttrib(ctx, &ynrV3_attr);
        ret = rk_aiq_user_api_acnrV2_GetAttrib(ctx, &cnrV2_attr);
        ret = rk_aiq_user_api_abayer2dnrV2_GetAttrib(ctx, &bayer2dnrV2_attr);
        ret = rk_aiq_user_api_abayertnrV2_GetAttrib(ctx, &bayertnrV2_attr);
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
            LOGE_ANR("bayer2dnr.mode:%d bayertnr.mode:%d ynr.mode:%d cnr.mode:%d\n",
                     bayer2dnrV2_attr.eMode,
                     bayertnrV2_attr.eMode,
                     ynrV3_attr.eMode,
                     cnrV2_attr.eMode);
        }
    }

    if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        rk_aiq_ynr_attrib_v22_t ynrV22_attr;
        ynrV22_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        rk_aiq_cnr_attrib_v30_t cnrV30_attr;
        cnrV30_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        rk_aiq_bayer2dnr_attrib_v23_t bayer2dnrV23_attr;
        bayer2dnrV23_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        rk_aiq_bayertnr_attrib_v23_t bayertnrV23_attr;
        bayertnrV23_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        ret = rk_aiq_user_api_aynrV22_GetAttrib(ctx, &ynrV22_attr);
        ret = rk_aiq_user_api_acnrV30_GetAttrib(ctx, &cnrV30_attr);
        ret = rk_aiq_user_api_abayer2dnrV23_GetAttrib(ctx, &bayer2dnrV23_attr);
        ret = rk_aiq_user_api_abayertnrV23_GetAttrib(ctx, &bayertnrV23_attr);
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

    if (CHECK_ISP_HW_V20()) {
        ret = rk_aiq_user_api_anr_SetLumaSFStrength(ctx, level / 100.0);
        ret = rk_aiq_user_api_anr_SetLumaTFStrength(ctx, level / 100.0);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setANRStrth failed!", ret);
    }

    if (CHECK_ISP_HW_V21()) {
        ret = rk_aiq_user_api_aynrV2_SetStrength(ctx, level / 100.0);
        ret = rk_aiq_user_api_abayernrV2_SetSFStrength(ctx, level / 100.0);
        ret = rk_aiq_user_api_abayernrV2_SetTFStrength(ctx, level / 100.0);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setANRStrth failed!", ret);
    }

    if (CHECK_ISP_HW_V30()) {
        rk_aiq_uapi_sync_t sync;
        sync.sync_mode =  RK_AIQ_UAPI_MODE_SYNC;
        rk_aiq_ynr_strength_v3_t ynrStrenght;
        ynrStrenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        ynrStrenght.strength_enable = true;
        ynrStrenght.percent = level / 100.0;
        ret = rk_aiq_user_api_aynrV3_SetStrength(ctx, &ynrStrenght);
        rk_aiq_bayer2dnr_strength_v2_t bayer2dnrV2Strenght;
        bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        bayer2dnrV2Strenght.strength_enable = true;
        bayer2dnrV2Strenght.percent = level / 100.0;
        ret = rk_aiq_user_api_abayer2dnrV2_SetStrength(ctx, &bayer2dnrV2Strenght);
        rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
        bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        bayertnrV2Strenght.strength_enable = true;
        bayertnrV2Strenght.percent = level / 100.0;
        ret = rk_aiq_user_api_abayertnrV2_SetStrength(ctx, &bayertnrV2Strenght);
    }

    if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        rk_aiq_ynr_strength_v22_t ynrV22Strength;
        ynrV22Strength.strength_enable = true;
        ynrV22Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        ynrV22Strength.percent = level / 100.0;
        ret = rk_aiq_user_api_aynrV22_SetStrength(ctx, &ynrV22Strength);
        rk_aiq_bayer2dnr_strength_v23_t bayer2dnrV23Strength;
        bayer2dnrV23Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        bayer2dnrV23Strength.strength_enable = true;
        bayer2dnrV23Strength.percent = level / 100.0;
        ret = rk_aiq_user_api_abayer2dnrV23_SetStrength(ctx, &bayer2dnrV23Strength);
        rk_aiq_bayertnr_strength_v23_t bayertnrV23Strength;
        bayertnrV23Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        bayertnrV23Strength.strength_enable = true;
        bayertnrV23Strength.percent = level / 100.0;
        ret = rk_aiq_user_api_abayertnrV23_SetStrength(ctx, &bayertnrV23Strength);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setANRStrth failed!", ret);
    }
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

    if (CHECK_ISP_HW_V20()) {
        ret = rk_aiq_user_api_anr_GetLumaTFStrength(ctx, &percent);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getANRStrth failed!", ret);
        *level = (unsigned int)(percent * 100);
    }

    if (CHECK_ISP_HW_V21()) {
        ret = rk_aiq_user_api_abayernrV2_GetTFStrength(ctx, &percent);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getANRStrth failed!", ret);
        *level = (unsigned int)(percent * 100);
    }

    if (CHECK_ISP_HW_V30()) {
        rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
        bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        ret = rk_aiq_user_api_abayertnrV2_GetStrength(ctx, &bayertnrV2Strenght);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getANRStrth failed!", ret);
        percent = bayertnrV2Strenght.percent;
        *level = (unsigned int)(percent * 100);
    }

    if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        rk_aiq_bayertnr_strength_v23_t bayertnrV23Strength;
        bayertnrV23Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        ret = rk_aiq_user_api_abayertnrV23_GetStrength(ctx, &bayertnrV23Strength);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getANRStrth failed!", ret);
        *level = (unsigned int)(bayertnrV23Strength.percent * 100);
    }

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

    if (CHECK_ISP_HW_V20()) {
        ret = rk_aiq_user_api_anr_SetLumaSFStrength(ctx, level / 100.0);
    }

    if (CHECK_ISP_HW_V21()) {
        ret = rk_aiq_user_api_abayernrV2_SetSFStrength(ctx, level / 100.0);
        ret = rk_aiq_user_api_aynrV2_SetStrength(ctx, level / 100.0);
    }

    if (CHECK_ISP_HW_V30()) {
        rk_aiq_uapi_sync_t sync;
        sync.sync_mode =  RK_AIQ_UAPI_MODE_SYNC;
        rk_aiq_ynr_strength_v3_t ynrStrenght;
        ynrStrenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        ynrStrenght.strength_enable = true;
        ynrStrenght.percent = level / 100.0;
        ret = rk_aiq_user_api_aynrV3_SetStrength(ctx, &ynrStrenght);
        rk_aiq_bayer2dnr_strength_v2_t bayer2dnrV2Strenght;
        bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        bayer2dnrV2Strenght.strength_enable = true;
        bayer2dnrV2Strenght.percent = level / 100.0;
        ret = rk_aiq_user_api_abayer2dnrV2_SetStrength(ctx, &bayer2dnrV2Strenght);
    }

    if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        rk_aiq_ynr_strength_v22_t ynrStrength;
        ynrStrength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        ynrStrength.strength_enable = true;
        ynrStrength.percent = level / 100.0;
        ret = rk_aiq_user_api_aynrV22_SetStrength(ctx, &ynrStrength);
        rk_aiq_bayer2dnr_strength_v23_t bayer2dnrV23Strength;
        bayer2dnrV23Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        bayer2dnrV23Strength.strength_enable = true;
        bayer2dnrV23Strength.percent = level / 100.0;
        ret = rk_aiq_user_api_abayer2dnrV23_SetStrength(ctx, &bayer2dnrV23Strength);
    }

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
XCamReturn rk_aiq_uapi_getMSpaNRStrth(const rk_aiq_sys_ctx_t* ctx, bool *on, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    float percent = 0.0f;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, getMSpaNRStrth failed!");
    }

    if (CHECK_ISP_HW_V20()) {
        ret = rk_aiq_user_api_anr_GetLumaSFStrength(ctx, &percent);
    }

    if (CHECK_ISP_HW_V21()) {
        ret = rk_aiq_user_api_abayernrV2_GetSFStrength(ctx, &percent);
    }

    if (CHECK_ISP_HW_V30()) {
        rk_aiq_bayer2dnr_strength_v2_t bayer2dnrV2Strenght;
        bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        ret = rk_aiq_user_api_abayer2dnrV2_GetStrength(ctx, &bayer2dnrV2Strenght);
        percent = bayer2dnrV2Strenght.percent;
    }

    if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        rk_aiq_bayer2dnr_strength_v23_t bayer2dnrV23Strength;
        bayer2dnrV23Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        ret = rk_aiq_user_api_abayer2dnrV23_GetStrength(ctx, &bayer2dnrV23Strength);
        percent = bayer2dnrV23Strength.percent;
    }

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

    if (CHECK_ISP_HW_V20()) {
        ret = rk_aiq_user_api_anr_SetLumaTFStrength(ctx, level / 100.0);
    }

    if (CHECK_ISP_HW_V21()) {
        ret = rk_aiq_user_api_abayernrV2_SetTFStrength(ctx, level / 100.0);
    }

    if (CHECK_ISP_HW_V30()) {
        rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
        bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        bayertnrV2Strenght.strength_enable = true;
        bayertnrV2Strenght.percent = level / 100.0;
        ret = rk_aiq_user_api_abayertnrV2_SetStrength(ctx, &bayertnrV2Strenght);
    }

    if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        rk_aiq_bayertnr_strength_v23_t bayertnrV23Strength;
        bayertnrV23Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        bayertnrV23Strength.strength_enable = true;
        bayertnrV23Strength.percent = level / 100.0;
        ret = rk_aiq_user_api_abayertnrV23_SetStrength(ctx, &bayertnrV23Strength);
    }

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
XCamReturn rk_aiq_uapi_getMTNRStrth(const rk_aiq_sys_ctx_t* ctx, bool *on, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    float percent = 0.0f;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, getMTNRStrth failed!");
    }

    if (CHECK_ISP_HW_V20()) {
        ret = rk_aiq_user_api_anr_GetLumaTFStrength(ctx, &percent);
    }

    if (CHECK_ISP_HW_V21()) {
        ret = rk_aiq_user_api_abayernrV2_GetTFStrength(ctx, &percent);
    }

    if (CHECK_ISP_HW_V30()) {
        rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
        bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        ret = rk_aiq_user_api_abayertnrV2_GetStrength(ctx, &bayertnrV2Strenght);
        percent = bayertnrV2Strenght.percent;
    }

    if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        rk_aiq_bayertnr_strength_v23_t bayertnrV23Strength;
        bayertnrV23Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        ret = rk_aiq_user_api_abayertnrV23_GetStrength(ctx, &bayertnrV23Strength);
        percent = bayertnrV23Strength.percent;
    }

    RKAIQ_IMGPROC_CHECK_RET(ret, "getMTNRStrth failed!", ret);
    *level = (unsigned int)(percent * 100);
    IMGPROC_FUNC_EXIT
    return ret;
}

/*
**********************************************************
* Dehaze
**********************************************************
*/
/*
*****************************
*
* Desc: set/get dehaze mode
* Argument:
*   mode:
*     auto: auto dehaze, when use auto, equal use rk_aiq_uapi_enableDhz
*     manual: Manual dehaze, when needs to use manual, please use rk_aiq_uapi_setMDhzStrth
*
*****************************
*//*
XCamReturn rk_aiq_uapi_setDhzMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    adehaze_sw_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    attr.byPass = false;
    if (mode == OP_AUTO) {
        attr.mode = RK_AIQ_DEHAZE_MODE_AUTO;
        ret = rk_aiq_user_api_adehaze_setSwAttrib(ctx, attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setDhzMode auto failed!");
    } else if (mode == OP_MANUAL) {
        attr.mode = RK_AIQ_DEHAZE_MODE_MANUAL;
        RKAIQ_IMGPROC_CHECK_RET(ret, "setDhzMode manual failed!");
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        LOGE("mode is invalid!");
    }
    IMGPROC_FUNC_EXIT
    return ret;
}

//not implemet
XCamReturn rk_aiq_uapi_getDhzMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    adehaze_sw_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getDhzMode failed!");
    }
    ret = rk_aiq_user_api_adehaze_getSwAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed in get attrib!");
    if (attr.mode == RK_AIQ_DEHAZE_MODE_AUTO) {
        *mode = OP_AUTO;
    } else if (attr.mode == RK_AIQ_DEHAZE_MODE_MANUAL) {
        *mode = OP_MANUAL;
    } else {
        *mode = OP_INVAL;
    }

    IMGPROC_FUNC_EXIT
    return ret;
}
*/
/*
*****************************
*
* Desc: set manual dehaze strength
*     this function is active for dehaze is manual mode
* Argument:
*   level: [0, 10]
*   Do not need to use rk_aiq_uapi_enableDhz and rk_aiq_uapi_setDhzMode before use this
*
*****************************
*//*
XCamReturn rk_aiq_uapi_setMDhzStrth(const rk_aiq_sys_ctx_t* ctx, bool on, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    adehaze_sw_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    if (!on) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "Error use, param 'on' should be 'true'!");
    }
    if (level < 1 || level > 10) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, strength range is [1,10]!");
    }
    attr.byPass = false;
    attr.mode = RK_AIQ_DEHAZE_MODE_MANUAL;
    attr.stManual.strength = level;
    ret = rk_aiq_user_api_adehaze_setSwAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMDhzStrth failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_getMDhzStrth(const rk_aiq_sys_ctx_t* ctx, bool *on, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    adehaze_sw_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    ret = rk_aiq_user_api_adehaze_getSwAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed in get attrib!");

    if (attr.mode != RK_AIQ_DEHAZE_MODE_MANUAL) {
        LOGE("Not in manual mode!");
        *level = 0;
        *on = false;
    } else {
        *level = attr.stManual.strength;
        *on = true;
    }


    IMGPROC_FUNC_EXIT
    return ret;
}*/
/*
*****************************
*
* Desc: enable dehaze
* Argument:
*   When dehaze enable, dehaze on and para use use IQ xml
*   When dehaze disable, dehaze off and enhance para use use IQ xml
*
*****************************
*//*
XCamReturn rk_aiq_uapi_enableDhz(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    adehaze_sw_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    attr.byPass = false;
    attr.mode = RK_AIQ_DEHAZE_MODE_AUTO;
    ret = rk_aiq_user_api_adehaze_setSwAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "enable dehaze failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_disableDhz(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    adehaze_sw_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    attr.byPass = false;
    attr.mode = RK_AIQ_DEHAZE_MODE_OFF;
    ret = rk_aiq_user_api_adehaze_setSwAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "disable dehaze failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}
*/
/*
**********************************************************
* Image adjust
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
XCamReturn rk_aiq_uapi_setContrast(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER

    LOGD("setContrast enter, level=%d\n", level);
    acp_attrib_t attrib;
    if ((int)level < 0 || level > 255) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "level out of range, setContrast failed!");
    }

    ret = rk_aiq_user_api_acp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAttrib error,set contrast failed!");
    attrib.contrast = level;
    ret = rk_aiq_user_api_acp_SetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set contrast failed!");
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi_getContrast(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    acp_attrib_t attrib;
    if (level == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getContrast failed!");
    }
    ret = rk_aiq_user_api_acp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get contrast failed!");
    *level = attrib.contrast;
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
    acp_attrib_t attrib;

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
    ret = rk_aiq_user_api_acp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAttrib error,set brightness failed!");
    attrib.brightness = level;
    ret = rk_aiq_user_api_acp_SetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set brightness failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_getBrightness(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    acp_attrib_t attrib;
    if (level == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get brightness failed!");
    }
    ret = rk_aiq_user_api_acp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get brightness failed!");
    *level = attrib.brightness;
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
    acp_attrib_t attrib;

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
    ret = rk_aiq_user_api_acp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAttrib error,set saturation failed!");
    attrib.saturation = level;
    ret = rk_aiq_user_api_acp_SetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set saturation failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_getSaturation(const rk_aiq_sys_ctx_t* ctx, unsigned int* level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    acp_attrib_t attrib;
    if (level == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get saturation failed!");
    }
    ret = rk_aiq_user_api_acp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get saturation failed!");
    *level = attrib.saturation;
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
    acp_attrib_t attrib;

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
    ret = rk_aiq_user_api_acp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAttrib error,set hue failed!");
    attrib.hue = level;
    ret = rk_aiq_user_api_acp_SetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set hue failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_getHue(const rk_aiq_sys_ctx_t* ctx, unsigned int* level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    acp_attrib_t attrib;
    if (level == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get hue failed!");
    }
    ret = rk_aiq_user_api_acp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get hue failed!");
    *level = attrib.hue;
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
    if (CHECK_ISP_HW_V20()) {
        ret = rk_aiq_user_api_asharp_SetStrength(ctx, fPercent);
    }

    if (CHECK_ISP_HW_V21()) {
        ret = rk_aiq_user_api_asharpV3_SetStrength(ctx, fPercent);
    }

    if (CHECK_ISP_HW_V30()) {
        rk_aiq_sharp_strength_v4_t sharpV4Strenght;
        sharpV4Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        sharpV4Strenght.strength_enable = true;
        sharpV4Strenght.percent = fPercent;
        ret = rk_aiq_user_api_asharpV4_SetStrength(ctx, &sharpV4Strenght);
    }

    if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        rk_aiq_sharp_strength_v33_t sharpV33Strength;
        sharpV33Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        sharpV33Strength.strength_enable = true;
        sharpV33Strength.percent = fPercent;
        ret = rk_aiq_user_api_asharpV33_SetStrength(ctx, &sharpV33Strength);
    }
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

    if (CHECK_ISP_HW_V20()) {
        ret = rk_aiq_user_api_asharp_GetStrength(ctx, &fPercent);
    }

    if (CHECK_ISP_HW_V21()) {
        ret = rk_aiq_user_api_asharpV3_GetStrength(ctx, &fPercent);
    }

    if (CHECK_ISP_HW_V30()) {
        rk_aiq_sharp_strength_v4_t sharpV4Strenght;
        sharpV4Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        ret = rk_aiq_user_api_asharpV4_GetStrength(ctx, &sharpV4Strenght);
        fPercent = sharpV4Strenght.percent;
    }

    if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        rk_aiq_sharp_strength_v33_t sharpV33Strength;
        sharpV33Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        ret = rk_aiq_user_api_asharpV33_GetStrength(ctx, &sharpV33Strength);
        fPercent = sharpV33Strength.percent;
    }
    RKAIQ_IMGPROC_CHECK_RET(ret, "get sharpeness failed!");

    *level = (unsigned int)(fPercent * 100);
    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: Adjust image gamma
*
*****************************
*/
XCamReturn rk_aiq_uapi_setGammaCoef(const rk_aiq_sys_ctx_t* ctx, rk_aiq_gamma_attrib_t gammaAttr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, setGammaCoef failed!");
    }
    ret = rk_aiq_user_api_agamma_GetAttrib(ctx, &gammaAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get gamma attrib failed!");
    ret = rk_aiq_user_api_agamma_SetAttrib(ctx, gammaAttr);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_setGrayMode(const rk_aiq_sys_ctx_t* ctx, rk_aiq_gray_mode_t mode)
{
    return ctx->_analyzer->setGrayMode(mode);
}

rk_aiq_gray_mode_t rk_aiq_uapi_getGrayMode(const rk_aiq_sys_ctx_t* ctx)
{
    return ctx->_analyzer->getGrayMode();
}

XCamReturn rk_aiq_uapi_setFrameRate(const rk_aiq_sys_ctx_t* ctx, frameRateInfo_t info)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (info.mode < OP_AUTO || info.mode >= OP_INVAL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    ret = rk_aiq_user_api_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\n setFrameRate failed!");
    if(info.mode == OP_AUTO) {
        expSwAttr.stAuto.stFrmRate.isFpsFix = false;
    } else if (info.mode == OP_MANUAL) {
        expSwAttr.stAuto.stFrmRate.isFpsFix = true;
        expSwAttr.stAuto.stFrmRate.FpsValue = info.fps;
    }
    ret = rk_aiq_user_api_ae_setExpSwAttr(ctx, expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set exp attr failed!\n setFrameRate failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_getFrameRate(const rk_aiq_sys_ctx_t* ctx, frameRateInfo_t *info)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttr_t expSwAttr;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (info == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    ret = rk_aiq_user_api_ae_getExpSwAttr(ctx, &expSwAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get exp attr failed!\n getFrameRate failed!");
    if(expSwAttr.stAuto.stFrmRate.isFpsFix) {
        info->mode = OP_MANUAL;
        info->fps = expSwAttr.stAuto.stFrmRate.FpsValue;
    } else {
        info->mode = OP_AUTO;
        info->fps = expSwAttr.stAuto.stFrmRate.FpsValue;
    }
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_setMirroFlip(const rk_aiq_sys_ctx_t* ctx, bool mirror, bool flip, int skip_frm_cnt)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    return ctx->_rkAiqManager->setMirrorFlip(mirror, flip, skip_frm_cnt);
}

XCamReturn rk_aiq_uapi_getMirrorFlip(const rk_aiq_sys_ctx_t* ctx, bool* mirror, bool* flip)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL || mirror == NULL || flip == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    return ctx->_rkAiqManager->getMirrorFlip(*mirror, *flip);
}

XCamReturn rk_aiq_uapi_setFecEn(const rk_aiq_sys_ctx_t* ctx, bool en)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    rk_aiq_fec_attrib_t fecAttr;
    ret = rk_aiq_user_api_afec_GetAttrib(ctx, &fecAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get fec attrib failed!");
    fecAttr.en = en;
    ret = rk_aiq_user_api_afec_SetAttrib(ctx, fecAttr);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_setFecBypass(const rk_aiq_sys_ctx_t* ctx, bool bypass)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    rk_aiq_fec_attrib_t fecAttr;
    ret = rk_aiq_user_api_afec_GetAttrib(ctx, &fecAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get fec attrib failed!");
    fecAttr.bypass = bypass;
    ret = rk_aiq_user_api_afec_SetAttrib(ctx, fecAttr);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_setFecCorrectLevel(const rk_aiq_sys_ctx_t* ctx, int correctLevel)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    rk_aiq_fec_attrib_t fecAttr;
    ret = rk_aiq_user_api_afec_GetAttrib(ctx, &fecAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get fec attrib failed!");
    fecAttr.correct_level = correctLevel;
    ret = rk_aiq_user_api_afec_SetAttrib(ctx, fecAttr);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_setFecCorrectDirection(const rk_aiq_sys_ctx_t* ctx,
        const fec_correct_direction_t direction)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    rk_aiq_fec_attrib_t fecAttr;
    ret = rk_aiq_user_api_afec_GetAttrib(ctx, &fecAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get fec attrib failed!");
    fecAttr.direction = direction;
    ret = rk_aiq_user_api_afec_SetAttrib(ctx, fecAttr);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_setFecCorrectMode(const rk_aiq_sys_ctx_t* ctx,
        const fec_correct_mode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    rk_aiq_fec_attrib_t fecAttr;
    ret = rk_aiq_user_api_afec_GetAttrib(ctx, &fecAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get fec attrib failed!");
    fecAttr.mode = mode;
    ret = rk_aiq_user_api_afec_SetAttrib(ctx, fecAttr);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_setLdchEn(const rk_aiq_sys_ctx_t* ctx, bool en)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    rk_aiq_ldch_attrib_t ldchAttr;
    ret = rk_aiq_user_api_aldch_GetAttrib(ctx, &ldchAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get ldch attrib failed!");
    ldchAttr.en = en;
    ret = rk_aiq_user_api_aldch_SetAttrib(ctx, &ldchAttr);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi_setLdchCorrectLevel(const rk_aiq_sys_ctx_t* ctx, int correctLevel)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    rk_aiq_ldch_attrib_t ldchAttr;
    ret = rk_aiq_user_api_aldch_GetAttrib(ctx, &ldchAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get ldch attrib failed!");
    ldchAttr.correct_level = correctLevel;
    ret = rk_aiq_user_api_aldch_SetAttrib(ctx, &ldchAttr);
    IMGPROC_FUNC_EXIT
    return ret;
}

RKAIQ_END_DECLARE
