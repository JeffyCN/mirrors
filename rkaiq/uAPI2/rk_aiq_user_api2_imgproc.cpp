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

#if RKAIQ_HAVE_GAMMA_V10
    rk_aiq_gamma_v10_attr_t gammaAttr;
    memset(&gammaAttr, 0x0, sizeof(rk_aiq_gamma_v10_attr_t));
    gammaAttr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    gammaAttr.sync.done      = false;

    gammaAttr.mode                                    = RK_AIQ_GAMMA_MODE_AUTO;
    gammaAttr.stAuto.GammaTuningPara.Gamma_en         = true;
    gammaAttr.stAuto.GammaTuningPara.Gamma_out_segnum = GAMMATYPE_LOG;
    gammaAttr.stAuto.GammaTuningPara.Gamma_out_offset = 0;
    float gamma_X_v10[CALIBDB_AGAMMA_KNOTS_NUM_V10]   = {
        0,   1,   2,   3,   4,   5,   6,    7,    8,    10,   12,   14,   16,   20,   24,
        28,  32,  40,  48,  56,  64,  80,   96,   112,  128,  160,  192,  224,  256,  320,
        384, 448, 512, 640, 768, 896, 1024, 1280, 1536, 1792, 2048, 2560, 3072, 3584, 4095
    };
    float gamma_Y_v10[CALIBDB_AGAMMA_KNOTS_NUM_V10];
    for (int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V10; i++) {
        gamma_Y_v10[i] = 4095 * pow(gamma_X_v10[i] / 4095, 1 / GammaCoef + SlopeAtZero);
        gamma_Y_v10[i] = gamma_Y_v10[i] > 4095 ? 4095 : gamma_Y_v10[i] < 0 ? 0 : gamma_Y_v10[i];
        gammaAttr.stAuto.GammaTuningPara.Gamma_curve[i] = (int)(gamma_Y_v10[i] + 0.5);
    }
    ret = rk_aiq_user_api2_agamma_v10_SetAttrib(ctx, &gammaAttr);
#endif
#if RKAIQ_HAVE_GAMMA_V11
    rk_aiq_gamma_v11_attr_t gammaAttr;
    memset(&gammaAttr, 0x0, sizeof(rk_aiq_gamma_v11_attr_t));
    gammaAttr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    gammaAttr.sync.done      = false;

    gammaAttr.mode                                    = RK_AIQ_GAMMA_MODE_AUTO;
    gammaAttr.stAuto.GammaTuningPara.Gamma_en         = true;
    gammaAttr.stAuto.GammaTuningPara.Gamma_out_offset = 0;
    float gamma_X_v11[CALIBDB_AGAMMA_KNOTS_NUM_V11]   = {
        0,    1,    2,    3,    4,    5,    6,    7,    8,    10,  12,   14,   16,
        20,   24,   28,   32,   40,   48,   56,   64,   80,   96,  112,  128,  160,
        192,  224,  256,  320,  384,  448,  512,  640,  768,  896, 1024, 1280, 1536,
        1792, 2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840, 4095
    };
    float gamma_Y_v11[CALIBDB_AGAMMA_KNOTS_NUM_V11];
    for (int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V11; i++) {
        gamma_Y_v11[i] = 4095 * pow(gamma_X_v11[i] / 4095, 1 / GammaCoef + SlopeAtZero);
        gamma_Y_v11[i] = gamma_Y_v11[i] > 4095 ? 4095 : gamma_Y_v11[i] < 0 ? 0 : gamma_Y_v11[i];
        gammaAttr.stAuto.GammaTuningPara.Gamma_curve[i] = (int)(gamma_Y_v11[i] + 0.5);
    }
    ret = rk_aiq_user_api2_agamma_v11_SetAttrib(ctx, &gammaAttr);
#endif

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
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO
    adehaze_sw_v11_t attr_v11;
    memset(&attr_v11, 0, sizeof(adehaze_sw_v11_t));
    ret = rk_aiq_user_api2_adehaze_v11_getSwAttrib(ctx, &attr_v11);

    attr_v11.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v11.sync.done      = false;

    if (attr_v11.mode == DEHAZE_API_AUTO) {
        if (on)
            attr_v11.stAuto.DehazeTuningPara.Enable = true;
        else
            attr_v11.stAuto.DehazeTuningPara.Enable = false;
    } else if (attr_v11.mode == DEHAZE_API_MANUAL) {
        if (on)
            attr_v11.stManual.Enable = true;
        else
            attr_v11.stManual.Enable = false;
    }
    ret = rk_aiq_user_api2_adehaze_v11_setSwAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDehazeModuleEnable failed!");
#endif
#if RKAIQ_HAVE_DEHAZE_V12
    adehaze_sw_v12_t attr_v12;
    memset(&attr_v12, 0, sizeof(adehaze_sw_v12_t));
    ret = rk_aiq_user_api2_adehaze_v12_getSwAttrib(ctx, &attr_v12);

    attr_v12.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v12.sync.done      = false;

    if (attr_v12.mode == DEHAZE_API_AUTO) {
        if (on)
            attr_v12.stAuto.DehazeTuningPara.Enable = true;
        else
            attr_v12.stAuto.DehazeTuningPara.Enable = false;
    } else if (attr_v12.mode == DEHAZE_API_MANUAL) {
        if (on)
            attr_v12.stManual.Enable = true;
        else
            attr_v12.stManual.Enable = false;
    }
    ret = rk_aiq_user_api2_adehaze_v12_setSwAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDehazeModuleEnable failed!");
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
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO
    adehaze_sw_v11_t attr_v11;
    memset(&attr_v11, 0, sizeof(adehaze_sw_v11_t));
    ret = rk_aiq_user_api2_adehaze_v11_getSwAttrib(ctx, &attr_v11);

    attr_v11.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v11.sync.done      = false;

    if (attr_v11.mode == DEHAZE_API_AUTO) {
        if (on) {
            attr_v11.stAuto.DehazeTuningPara.dehaze_setting.en  = true;
            attr_v11.stAuto.DehazeTuningPara.enhance_setting.en = false;
        } else
            attr_v11.stAuto.DehazeTuningPara.dehaze_setting.en = false;
    } else if (attr_v11.mode == DEHAZE_API_MANUAL) {
        if (on) {
            attr_v11.stManual.dehaze_setting.en  = true;
            attr_v11.stManual.enhance_setting.en = false;
        } else
            attr_v11.stManual.dehaze_setting.en = false;
    }
    ret = rk_aiq_user_api2_adehaze_v11_setSwAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDehazeEnable failed!");
#endif
#if RKAIQ_HAVE_DEHAZE_V12
    adehaze_sw_v12_t attr_v12;
    memset(&attr_v12, 0, sizeof(adehaze_sw_v12_t));
    ret = rk_aiq_user_api2_adehaze_v12_getSwAttrib(ctx, &attr_v12);

    attr_v12.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v12.sync.done      = false;

    if (attr_v12.mode == DEHAZE_API_AUTO) {
        if (on) {
            attr_v12.stAuto.DehazeTuningPara.dehaze_setting.en  = true;
            attr_v12.stAuto.DehazeTuningPara.enhance_setting.en = false;
        } else
            attr_v12.stAuto.DehazeTuningPara.dehaze_setting.en = false;
    } else if (attr_v12.mode == DEHAZE_API_MANUAL) {
        if (on) {
            attr_v12.stManual.dehaze_setting.en  = true;
            attr_v12.stManual.enhance_setting.en = false;
        } else
            attr_v12.stManual.dehaze_setting.en = false;
    }
    ret = rk_aiq_user_api2_adehaze_v12_setSwAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDehazeEnable failed!");
#endif
    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set/get manual dehaze strength
*     this function is active for dehaze is manual mode
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
#if RKAIQ_HAVE_DEHAZE_V10
#if 0
    adehaze_sw_v10_t attr_v10;
    memset(&attr_v10, 0, sizeof(adehaze_sw_v10_t));
    ret = rk_aiq_user_api2_adehaze_v10_getSwAttrib(ctx, &attr_v10);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed!");

    attr_v10.sync.sync_mode                             = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v10.sync.done                                  = false;
    attr_v10.mode                                       = DEHAZE_API_AUTO;
    attr_v10.stAuto.DehazeTuningPara.Enable             = true;
    attr_v10.stAuto.DehazeTuningPara.dehaze_setting.en  = true;
    attr_v10.stAuto.DehazeTuningPara.enhance_setting.en = false;
    attr_v10.stAuto.DehazeTuningPara.cfg_alpha          = NORMALIZE_MAX;
    attr_v10.stAuto.DehazeTuningPara.ByPassThr          = NORMALIZE_MIN;
    // attr_v10.i.MDehazeStrth = level;
    for (int i = 0; i < attr_v10.stAuto.DehazeTuningPara.dehaze_setting.DehazeData.CtrlData_len;
            i++) {
        float level_diff = (float)(level - DEHAZE_DEFAULT_LEVEL);
        // sw_dhaz_cfg_wt
        attr_v10.stAuto.DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt[i] +=
            level_diff * DEHAZE_DEFAULT_CFG_WT_STEP;
        attr_v10.stAuto.DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt[i] =
            LIMIT_VALUE(attr_v10.stAuto.DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt[i],
                        NORMALIZE_MAX, NORMALIZE_MIN);

        // sw_dhaz_cfg_air
        attr_v10.stAuto.DehazeTuningPara.dehaze_setting.DehazeData.cfg_air[i] +=
            level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP;
        attr_v10.stAuto.DehazeTuningPara.dehaze_setting.DehazeData.cfg_air[i] = LIMIT_VALUE(
                    attr_v10.stAuto.DehazeTuningPara.dehaze_setting.DehazeData.cfg_air[i], 255, 0);

        // sw_dhaz_cfg_tmax
        attr_v10.stAuto.DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax[i] +=
            level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP;
        attr_v10.stAuto.DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax[i] =
            LIMIT_VALUE(attr_v10.stAuto.DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax[i],
                        NORMALIZE_MAX, NORMALIZE_MIN);
    }

    ret = rk_aiq_user_api2_adehaze_v10_setSwAttrib(ctx, &attr_v10);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMDhzStrth failed!");
#endif
#endif
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO
    adehaze_sw_v11_t attr_v11;
    memset(&attr_v11, 0, sizeof(adehaze_sw_v11_t));
    ret = rk_aiq_user_api2_adehaze_v11_getSwAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed!");
    if (attr_v11.mode == DEHAZE_API_AUTO) {
        if (!attr_v11.stAuto.DehazeTuningPara.Enable ||
                !attr_v11.stAuto.DehazeTuningPara.dehaze_setting.en ||
                attr_v11.stAuto.DehazeTuningPara.enhance_setting.en) {
            LOGW_ADEHAZE("%s: Dehaze is OFF! level is invalid\n", __FUNCTION__);
        }
    } else if (attr_v11.mode == DEHAZE_API_MANUAL) {
        if (!attr_v11.stManual.Enable || !attr_v11.stManual.dehaze_setting.en ||
                attr_v11.stManual.enhance_setting.en) {
            LOGW_ADEHAZE("%s: Dehaze is OFF! level is invalid\n", __FUNCTION__);
        }
    }

    attr_v11.sync.sync_mode                             = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v11.sync.done                                  = false;
    attr_v11.Info.updateMDehazeStrth                    = true;
    attr_v11.Info.MDehazeStrth                          = level;
    ret = rk_aiq_user_api2_adehaze_v11_setSwAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMDhzStrth failed!");
#endif
#if RKAIQ_HAVE_DEHAZE_V12
    adehaze_sw_v12_t attr_v12;
    memset(&attr_v12, 0, sizeof(adehaze_sw_v12_t));
    ret = rk_aiq_user_api2_adehaze_v12_getSwAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed!");
    if (attr_v12.mode == DEHAZE_API_AUTO) {
        if (!attr_v12.stAuto.DehazeTuningPara.Enable ||
                !attr_v12.stAuto.DehazeTuningPara.dehaze_setting.en ||
                attr_v12.stAuto.DehazeTuningPara.enhance_setting.en) {
            LOGW_ADEHAZE("%s: Dehaze is OFF! level is invalid\n", __FUNCTION__);
        }
    } else if (attr_v12.mode == DEHAZE_API_MANUAL) {
        if (!attr_v12.stManual.Enable || !attr_v12.stManual.dehaze_setting.en ||
                attr_v12.stManual.enhance_setting.en) {
            LOGW_ADEHAZE("%s: Dehaze is OFF! level is invalid\n", __FUNCTION__);
        }
    }

    attr_v12.sync.sync_mode                             = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v12.sync.done                                  = false;
    attr_v12.Info.updateMDehazeStrth                    = true;
    attr_v12.Info.MDehazeStrth                          = level;
    ret = rk_aiq_user_api2_adehaze_v12_setSwAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMDhzStrth failed!");
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
#if RKAIQ_HAVE_DEHAZE_V10
    adehaze_sw_v10_info_t attr_v10_info;
    memset(&attr_v10_info, 0, sizeof(adehaze_sw_v10_info_t));
    ret = rk_aiq_user_api2_adehaze_v10_getSwAttrib(ctx, &attr_v10_info);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed in get attrib!");
    *level = attr_v10_info.CtrlInfo.MDehazeStrth;
#endif
#if RKAIQ_HAVE_DEHAZE_V11
    adehaze_sw_v11_t attr_v11;
    memset(&attr_v11, 0, sizeof(adehaze_sw_v11_t));
    ret = rk_aiq_user_api2_adehaze_v11_getSwAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed in get attrib!");
    *level = attr_v11.Info.MDehazeStrth;
#endif
#if RKAIQ_HAVE_DEHAZE_V12
    adehaze_sw_v12_t attr_v12;
    memset(&attr_v12, 0, sizeof(adehaze_sw_v12_t));
    ret = rk_aiq_user_api2_adehaze_v12_getSwAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed in get attrib!");
    *level = attr_v12.Info.MDehazeStrth;
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
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO
    adehaze_sw_v11_t attr_v11;
    memset(&attr_v11, 0, sizeof(adehaze_sw_v11_t));
    ret = rk_aiq_user_api2_adehaze_v11_getSwAttrib(ctx, &attr_v11);

    attr_v11.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v11.sync.done      = false;

    if (attr_v11.mode == DEHAZE_API_AUTO) {
        if (on)
            attr_v11.stAuto.DehazeTuningPara.enhance_setting.en = true;
        else
            attr_v11.stAuto.DehazeTuningPara.enhance_setting.en = false;
    } else if (attr_v11.mode == DEHAZE_API_MANUAL) {
        if (on)
            attr_v11.stManual.enhance_setting.en = true;
        else
            attr_v11.stManual.enhance_setting.en = false;
    }
    ret = rk_aiq_user_api2_adehaze_v11_setSwAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setEnhanceEnable failed!");
#endif
#if RKAIQ_HAVE_DEHAZE_V12
    adehaze_sw_v12_t attr_v12;
    memset(&attr_v12, 0, sizeof(adehaze_sw_v12_t));
    ret = rk_aiq_user_api2_adehaze_v12_getSwAttrib(ctx, &attr_v12);

    attr_v12.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v12.sync.done      = false;

    if (attr_v12.mode == DEHAZE_API_AUTO) {
        if (on)
            attr_v12.stAuto.DehazeTuningPara.enhance_setting.en = true;
        else
            attr_v12.stAuto.DehazeTuningPara.enhance_setting.en = false;
    } else if (attr_v12.mode == DEHAZE_API_MANUAL) {
        if (on)
            attr_v12.stManual.enhance_setting.en = true;
        else
            attr_v12.stManual.enhance_setting.en = false;
    }
    ret = rk_aiq_user_api2_adehaze_v12_setSwAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setEnhanceEnable failed!");
#endif
    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set/get manual enhance strength
*     this function is active for dehaze is manual mode
* Argument:
*   level: [0, 100]
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
#if RKAIQ_HAVE_DEHAZE_V10
#if 0
    adehaze_sw_v10_info_t attr_v10_info;
    memset(&attr_v10_info, 0, sizeof(adehaze_sw_v10_info_t));
    ret = rk_aiq_user_api2_adehaze_v10_getSwAttribInfo(ctx, &attr_v10_info);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzInfo failed!");
    attr_v10_info.CtrlInfo.MEnhanceStrth = level;
    ret = rk_aiq_user_api2_adehaze_v10_setSwAttribInfo(ctx, &attr_v10_info);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMDhzInfo failed!");

    adehaze_sw_v10_t attr_v10;
    memset(&attr_v10, 0, sizeof(adehaze_sw_v10_t));
    ret = rk_aiq_user_api2_adehaze_v10_getSwAttrib(ctx, &attr_v10);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed!");
    attr_v10.sync.sync_mode                             = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v10.sync.done                                  = false;
    attr_v10.mode                                       = DEHAZE_API_AUTO;
    attr_v10.stAuto.DehazeTuningPara.Enable             = true;
    attr_v10.stAuto.DehazeTuningPara.dehaze_setting.en  = false;
    attr_v10.stAuto.DehazeTuningPara.enhance_setting.en = true;
    attr_v10.stAuto.DehazeTuningPara.cfg_alpha          = NORMALIZE_MAX;
    attr_v10.stAuto.DehazeTuningPara.ByPassThr          = NORMALIZE_MIN;
    ret = rk_aiq_user_api2_adehaze_v10_setSwAttrib(ctx, &attr_v10);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMEnhanceStrth failed!");
#endif
#endif
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO
    adehaze_sw_v11_t attr_v11;
    memset(&attr_v11, 0, sizeof(adehaze_sw_v11_t));
    ret = rk_aiq_user_api2_adehaze_v11_getSwAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed!");
    if (attr_v11.mode == DEHAZE_API_AUTO) {
        if (!attr_v11.stAuto.DehazeTuningPara.Enable ||
                !attr_v11.stAuto.DehazeTuningPara.enhance_setting.en) {
            LOGW_ADEHAZE("%s: Enhance is OFF! level is invalid\n", __FUNCTION__);
        }
    } else if (attr_v11.mode == DEHAZE_API_MANUAL) {
        if (!attr_v11.stManual.Enable || !attr_v11.stManual.enhance_setting.en) {
            LOGW_ADEHAZE("%s: Enhance is OFF! level is invalid\n", __FUNCTION__);
        }
    }

    attr_v11.sync.sync_mode                             = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v11.sync.done                                  = false;
    attr_v11.Info.updateMEnhanceStrth                   = true;
    attr_v11.Info.MEnhanceStrth                         = level;
    ret = rk_aiq_user_api2_adehaze_v11_setSwAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMEnhanceStrth failed!");
#endif
#if RKAIQ_HAVE_DEHAZE_V12
    adehaze_sw_v12_t attr_v12;
    memset(&attr_v12, 0, sizeof(adehaze_sw_v12_t));
    ret = rk_aiq_user_api2_adehaze_v12_getSwAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed!");
    if (attr_v12.mode == DEHAZE_API_AUTO) {
        if (!attr_v12.stAuto.DehazeTuningPara.Enable ||
                !attr_v12.stAuto.DehazeTuningPara.enhance_setting.en) {
            LOGW_ADEHAZE("%s: Enhance is OFF! level is invalid\n", __FUNCTION__);
        }
    } else if (attr_v12.mode == DEHAZE_API_MANUAL) {
        if (!attr_v12.stManual.Enable || !attr_v12.stManual.enhance_setting.en) {
            LOGW_ADEHAZE("%s: Enhance is OFF! level is invalid\n", __FUNCTION__);
        }
    }

    attr_v12.sync.sync_mode                             = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v12.sync.done                                  = false;
    attr_v12.Info.updateMEnhanceStrth                   = true;
    attr_v12.Info.MEnhanceStrth                         = level;
    ret = rk_aiq_user_api2_adehaze_v12_setSwAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMEnhanceStrth failed!");
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
#if RKAIQ_HAVE_DEHAZE_V10
    adehaze_sw_v10_info_t attr_v10_info;
    memset(&attr_v10_info, 0, sizeof(adehaze_sw_v10_info_t));
    ret = rk_aiq_user_api2_adehaze_v10_getSwAttribInfo(ctx, &attr_v10_info);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMEnhanceStrth failed in get attrib!");
    *level = attr_v10_info.CtrlInfo.MEnhanceStrth;
#endif
#if RKAIQ_HAVE_DEHAZE_V11
    adehaze_sw_v11_t attr_v11;
    memset(&attr_v11, 0, sizeof(adehaze_sw_v11_t));
    ret = rk_aiq_user_api2_adehaze_v11_getSwAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMEnhanceStrth failed in get attrib!");
    *level = attr_v11.Info.MEnhanceStrth;
#endif
#if RKAIQ_HAVE_DEHAZE_V12
    adehaze_sw_v12_t attr_v12;
    memset(&attr_v12, 0, sizeof(adehaze_sw_v12_t));
    ret = rk_aiq_user_api2_adehaze_v12_getSwAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMEnhanceStrth failed in get attrib!");
    *level = attr_v12.Info.MEnhanceStrth;
#endif
    IMGPROC_FUNC_EXIT
    return ret;
}
/*
*****************************
*
* Desc: set/get manual enhance chrome strength
*     this function is active for dehaze is manual mode
* Argument:
*   level: [0, 100]
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
#if RKAIQ_HAVE_DEHAZE_V10
#if 0
    adehaze_sw_v10_info_t attr_v10_info;
    memset(&attr_v10_info, 0, sizeof(adehaze_sw_v10_info_t));
    ret = rk_aiq_user_api2_adehaze_v10_getSwAttribInfo(ctx, &attr_v10_info);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzInfo failed!");
    attr_v10_info.CtrlInfo.MEnhanceChromeStrth = level;
    ret = rk_aiq_user_api2_adehaze_v10_setSwAttribInfo(ctx, &attr_v10_info);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMDhzInfo failed!");

    adehaze_sw_v10_t attr_v10;
    memset(&attr_v10, 0, sizeof(adehaze_sw_v10_t));
    ret = rk_aiq_user_api2_adehaze_v10_getSwAttrib(ctx, &attr_v10);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed!");

    attr_v10.sync.sync_mode                             = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v10.sync.done                                  = false;
    attr_v10.mode                                       = DEHAZE_API_AUTO;
    attr_v10.stAuto.DehazeTuningPara.Enable             = true;
    attr_v10.stAuto.DehazeTuningPara.dehaze_setting.en  = false;
    attr_v10.stAuto.DehazeTuningPara.enhance_setting.en = true;
    attr_v10.stAuto.DehazeTuningPara.cfg_alpha          = NORMALIZE_MAX;
    attr_v10.stAuto.DehazeTuningPara.ByPassThr          = NORMALIZE_MIN;
    float level_diff = (float)(level - ENHANCE_DEFAULT_LEVEL);
    for (int i = 0; i < attr_v10.stAuto.DehazeTuningPara.enhance_setting.EnhanceData.CtrlData_len;
            i++) {
        // enhance_chroma
        attr_v10.stAuto.DehazeTuningPara.enhance_setting.EnhanceData[i].enhance_chroma +=
            level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        attr_v10.stAuto.DehazeTuningPara.enhance_setting.EnhanceData[i].enhance_chroma =
            LIMIT_VALUE(
                attr_v10.stAuto.DehazeTuningPara.enhance_setting.EnhanceData[i].enhance_chroma,
                16.0, 1.0);
    }
    ret = rk_aiq_user_api2_adehaze_v10_setSwAttrib(ctx, &attr_v10);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMEnhanceStrth failed!");
#endif
#endif
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO
    adehaze_sw_v11_t attr_v11;
    memset(&attr_v11, 0, sizeof(adehaze_sw_v11_t));
    ret = rk_aiq_user_api2_adehaze_v11_getSwAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed!");
    if (attr_v11.mode == DEHAZE_API_AUTO) {
        if (!attr_v11.stAuto.DehazeTuningPara.Enable ||
                !attr_v11.stAuto.DehazeTuningPara.enhance_setting.en) {
            LOGW_ADEHAZE("%s: Enhance is OFF! level is invalid\n", __FUNCTION__);
        }
    } else if (attr_v11.mode == DEHAZE_API_MANUAL) {
        if (!attr_v11.stManual.Enable || !attr_v11.stManual.enhance_setting.en) {
            LOGW_ADEHAZE("%s: Enhance is OFF! level is invalid\n", __FUNCTION__);
        }
    }

    attr_v11.sync.sync_mode                             = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v11.sync.done                                  = false;
    attr_v11.Info.updateMEnhanceChromeStrth             = true;
    attr_v11.Info.MEnhanceChromeStrth                   = level;
    ret = rk_aiq_user_api2_adehaze_v11_setSwAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMEnhanceStrth failed!");
#endif
#if RKAIQ_HAVE_DEHAZE_V12
    adehaze_sw_v12_t attr_v12;
    memset(&attr_v12, 0, sizeof(adehaze_sw_v12_t));
    ret = rk_aiq_user_api2_adehaze_v12_getSwAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed!");
    if (attr_v12.mode == DEHAZE_API_AUTO) {
        if (!attr_v12.stAuto.DehazeTuningPara.Enable ||
                !attr_v12.stAuto.DehazeTuningPara.enhance_setting.en) {
            LOGW_ADEHAZE("%s: Enhance is OFF! level is invalid\n", __FUNCTION__);
        }
    } else if (attr_v12.mode == DEHAZE_API_MANUAL) {
        if (!attr_v12.stManual.Enable || !attr_v12.stManual.enhance_setting.en) {
            LOGW_ADEHAZE("%s: Enhance is OFF! level is invalid\n", __FUNCTION__);
        }
    }

    attr_v12.sync.sync_mode                             = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v12.sync.done                                  = false;
    attr_v12.Info.updateMEnhanceChromeStrth             = true;
    attr_v12.Info.MEnhanceChromeStrth                   = level;
    ret = rk_aiq_user_api2_adehaze_v12_setSwAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMEnhanceStrth failed!");
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
#if RKAIQ_HAVE_DEHAZE_V10
    adehaze_sw_v10_info_t attr_v10_info;
    memset(&attr_v10_info, 0, sizeof(adehaze_sw_v10_info_t));
    ret = rk_aiq_user_api2_adehaze_v10_getSwAttribInfo(ctx, &attr_v10_info);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMEnhanceChromeStrth failed in get attrib!");
    *level = attr_v10_info.CtrlInfo.MEnhanceChromeStrth;
#endif
#if RKAIQ_HAVE_DEHAZE_V11
    adehaze_sw_v11_t attr_v11;
    memset(&attr_v11, 0, sizeof(adehaze_sw_v11_t));
    ret = rk_aiq_user_api2_adehaze_v11_getSwAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMEnhanceChromeStrth failed in get attrib!");
    *level = attr_v11.Info.MEnhanceChromeStrth;
#endif
#if RKAIQ_HAVE_DEHAZE_V12
    adehaze_sw_v12_t attr_v12;
    memset(&attr_v12, 0, sizeof(adehaze_sw_v12_t));
    ret = rk_aiq_user_api2_adehaze_v12_getSwAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMEnhanceChromeStrth failed in get attrib!");
    *level = attr_v12.Info.MEnhanceChromeStrth;
#endif
    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set/get manual drc Local TMO
*     this function is active for DRC is Auto mode
*     use in RK356x, RV1106
* Argument:
*   LocalWeit: [0, 1]
*   GlobalContrast: [0, 1]
*   LoLitContrast: [0, 1]
*   only valid in RK356x
*****************************
*/
XCamReturn rk_aiq_uapi2_setDrcLocalTMO(const rk_aiq_sys_ctx_t* ctx, float LocalWeit, float GlobalContrast, float LoLitContrast)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    if (LocalWeit < 0 || LocalWeit > 1) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, LocalWeit range is [0,1]!");
    }
    if (GlobalContrast < 0 || GlobalContrast > 1) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, GlobalContrast range is [0,1]!");
    }
    if (LoLitContrast < 0 || LoLitContrast > 1) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, LoLitContrast range is [0,1]!");
    }

#if RKAIQ_HAVE_DRC_V10
    drcAttrV10_t attr_v10;
    memset(&attr_v10, 0, sizeof(drcAttrV10_t));
    ret = rk_aiq_user_api2_adrc_v10_GetAttrib(ctx, &attr_v10);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcLocalTMO failed in get attrib!");
    attr_v10.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v10.sync.done      = false;

    if (attr_v10.opMode == DRC_OPMODE_AUTO) {
        for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++) {
            attr_v10.stAuto.DrcTuningPara.LocalTMOSetting.LocalTMOData.LocalWeit[i] = LocalWeit;
            attr_v10.stAuto.DrcTuningPara.LocalTMOSetting.LocalTMOData.GlobalContrast[i] =
                GlobalContrast;
            attr_v10.stAuto.DrcTuningPara.LocalTMOSetting.LocalTMOData.LoLitContrast[i] =
                LoLitContrast;
        }
    } else if (attr_v10.opMode == DRC_OPMODE_MANUAL) {
        attr_v10.stManual.LocalTMOSetting.LocalTMOData.LocalWeit      = LocalWeit;
        attr_v10.stManual.LocalTMOSetting.LocalTMOData.GlobalContrast = GlobalContrast;
        attr_v10.stManual.LocalTMOSetting.LocalTMOData.LoLitContrast  = LoLitContrast;
    }
    ret = rk_aiq_user_api2_adrc_v10_SetAttrib(ctx, &attr_v10);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcLocalTMO failed!");
#endif
#if RKAIQ_HAVE_DRC_V11
    ret = XCAM_RETURN_ERROR_PARAM;
    RKAIQ_IMGPROC_CHECK_RET(ret,
                            "RK3588 do not support rk_aiq_uapi2_setDrcLocalTMO! Please use "
                            "rk_aiq_uapi2_setDrcLocalData");
#endif
#if RKAIQ_HAVE_DRC_V12
    ret = XCAM_RETURN_ERROR_PARAM;
    RKAIQ_IMGPROC_CHECK_RET(ret,
                            "RV1106 do not support rk_aiq_uapi2_setDrcLocalTMO! Please use "
                            "rk_aiq_uapi2_setDrcLocalData");
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    ret = XCAM_RETURN_ERROR_PARAM;
    RKAIQ_IMGPROC_CHECK_RET(ret,
                            "RK3562 do not support rk_aiq_uapi2_setDrcLocalTMO! Please use "
                            "rk_aiq_uapi2_setDrcLocalData");
#endif

    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getDrcLocalTMO(const rk_aiq_sys_ctx_t* ctx, float * LocalWeit, float * GlobalContrast, float * LoLitContrast)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }

#if RKAIQ_HAVE_DRC_V10
    drcAttrV10_t attr_v10;
    memset(&attr_v10, 0, sizeof(drcAttrV10_t));
    ret = rk_aiq_user_api2_adrc_v10_GetAttrib(ctx, &attr_v10);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcLocalTMO failed in get attrib!");
    *LocalWeit      = attr_v10.Info.ValidParams.LocalTMOSetting.LocalTMOData.LocalWeit;
    *GlobalContrast = attr_v10.Info.ValidParams.LocalTMOSetting.LocalTMOData.GlobalContrast;
    *LoLitContrast  = attr_v10.Info.ValidParams.LocalTMOSetting.LocalTMOData.LoLitContrast;
#endif
#if RKAIQ_HAVE_DRC_V11
    ret = XCAM_RETURN_ERROR_PARAM;
    RKAIQ_IMGPROC_CHECK_RET(ret,
                            "RK3588 do not support rk_aiq_uapi2_getDrcLocalTMO! Please use "
                            "rk_aiq_uapi2_getDrcLocalData");
#endif
#if RKAIQ_HAVE_DRC_V12
    ret = XCAM_RETURN_ERROR_PARAM;
    RKAIQ_IMGPROC_CHECK_RET(ret,
                            "RV1106 do not support rk_aiq_uapi2_getDrcLocalTMO! Please use "
                            "rk_aiq_uapi2_getDrcLocalData");
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    ret = XCAM_RETURN_ERROR_PARAM;
    RKAIQ_IMGPROC_CHECK_RET(ret,
                            "RK3562 do not support rk_aiq_uapi2_getDrcLocalTMO! Please use "
                            "rk_aiq_uapi2_getDrcLocalData");
#endif

    IMGPROC_FUNC_EXIT
    return ret;
}
/*
*****************************
*
* Desc: set/get manual drc Local Data
*     this function is active for DRC is Auto mode
*     use in RK356x, RV1106
* Argument:
*   LocalWeit: [0, 1]
*   GlobalContrast: [0, 1]
*   LoLitContrast: [0, 1]
*   LocalAutoEnable: [0, 1]
*   LocalAutoWeit: [0, 1]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setDrcLocalData(const rk_aiq_sys_ctx_t* ctx, float LocalWeit, float GlobalContrast,
                                        float LoLitContrast, int LocalAutoEnable, float LocalAutoWeit)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    if (LocalWeit < 0 || LocalWeit > 1) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, LocalWeit range is [0,1]!");
    }
    if (GlobalContrast < 0 || GlobalContrast > 1) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, GlobalContrast range is [0,1]!");
    }
    if (LoLitContrast < 0 || LoLitContrast > 1) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, LoLitContrast range is [0,1]!");
    }
    if (LocalAutoEnable < 0 || LocalAutoEnable > 1) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, LocalAutoEnable range is [0,1]!");
    }
    if (LocalAutoWeit < 0 || LocalAutoWeit > 1) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, LocalAutoWeit range is [0,1]!");
    }
#if RKAIQ_HAVE_DRC_V10
    ret = XCAM_RETURN_ERROR_PARAM;
    RKAIQ_IMGPROC_CHECK_RET(ret,
                            "RK356x do not support rk_aiq_uapi2_setDrcLocalData! Please use "
                            "rk_aiq_uapi2_setDrcLocalTMO");
#endif
#if RKAIQ_HAVE_DRC_V11
    drcAttrV11_t attr_v11;
    memset(&attr_v11, 0, sizeof(drcAttrV11_t));
    ret = rk_aiq_user_api2_adrc_v11_GetAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcLocalData failed!");
    attr_v11.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v11.sync.done      = false;

    if (attr_v11.opMode == DRC_OPMODE_AUTO) {
        for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++) {
            attr_v11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit[i]      = LocalWeit;
            attr_v11.stAuto.DrcTuningPara.LocalSetting.LocalData.GlobalContrast[i] = GlobalContrast;
            attr_v11.stAuto.DrcTuningPara.LocalSetting.LocalData.LoLitContrast[i]  = LoLitContrast;
            attr_v11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoEnable[i] =
                LocalAutoEnable;
            attr_v11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoWeit[i] = LocalAutoWeit;
        }
    } else if (attr_v11.opMode == DRC_OPMODE_MANUAL) {
        attr_v11.stManual.LocalSetting.LocalData.LocalWeit       = LocalWeit;
        attr_v11.stManual.LocalSetting.LocalData.GlobalContrast  = GlobalContrast;
        attr_v11.stManual.LocalSetting.LocalData.LoLitContrast   = LoLitContrast;
        attr_v11.stManual.LocalSetting.LocalData.LocalAutoEnable = LocalAutoEnable;
        attr_v11.stManual.LocalSetting.LocalData.LocalAutoWeit   = LocalAutoWeit;
    }
    ret = rk_aiq_user_api2_adrc_v11_SetAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed!");
#endif
#if RKAIQ_HAVE_DRC_V12
    drcAttrV12_t attr_v12;
    memset(&attr_v12, 0, sizeof(drcAttrV12_t));
    ret = rk_aiq_user_api2_adrc_v12_GetAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcLocalData failed!");
    attr_v12.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v12.sync.done      = false;
    if (attr_v12.opMode == DRC_OPMODE_AUTO) {
        for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++) {
            attr_v12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit[i]      = LocalWeit;
            attr_v12.stAuto.DrcTuningPara.LocalSetting.LocalData.GlobalContrast[i] = GlobalContrast;
            attr_v12.stAuto.DrcTuningPara.LocalSetting.LocalData.LoLitContrast[i]  = LoLitContrast;
            attr_v12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoEnable[i] =
                LocalAutoEnable;
            attr_v12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoWeit[i] = LocalAutoWeit;
        }
    } else if (attr_v12.opMode == DRC_OPMODE_MANUAL) {
        attr_v12.stManual.LocalSetting.LocalData.LocalWeit       = LocalWeit;
        attr_v12.stManual.LocalSetting.LocalData.GlobalContrast  = GlobalContrast;
        attr_v12.stManual.LocalSetting.LocalData.LoLitContrast   = LoLitContrast;
        attr_v12.stManual.LocalSetting.LocalData.LocalAutoEnable = LocalAutoEnable;
        attr_v12.stManual.LocalSetting.LocalData.LocalAutoWeit   = LocalAutoWeit;
    }
    ret = rk_aiq_user_api2_adrc_v12_SetAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed!");
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    drcAttrV12Lite_t attr_v12_lite;
    memset(&attr_v12_lite, 0, sizeof(drcAttrV12Lite_t));
    ret = rk_aiq_user_api2_adrc_v12_lite_GetAttrib(ctx, &attr_v12_lite);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcLocalData failed!");
    attr_v12_lite.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v12_lite.sync.done      = false;
    if (attr_v12_lite.opMode == DRC_OPMODE_AUTO) {
        for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++) {
            attr_v12_lite.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit[i] = LocalWeit;
            attr_v12_lite.stAuto.DrcTuningPara.LocalSetting.LocalData.GlobalContrast[i] =
                GlobalContrast;
            attr_v12_lite.stAuto.DrcTuningPara.LocalSetting.LocalData.LoLitContrast[i] =
                LoLitContrast;
            attr_v12_lite.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoEnable[i] =
                LocalAutoEnable;
            attr_v12_lite.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoWeit[i] =
                LocalAutoWeit;
        }
    } else if (attr_v12_lite.opMode == DRC_OPMODE_MANUAL) {
        attr_v12_lite.stManual.LocalSetting.LocalData.LocalWeit       = LocalWeit;
        attr_v12_lite.stManual.LocalSetting.LocalData.GlobalContrast  = GlobalContrast;
        attr_v12_lite.stManual.LocalSetting.LocalData.LoLitContrast   = LoLitContrast;
        attr_v12_lite.stManual.LocalSetting.LocalData.LocalAutoEnable = LocalAutoEnable;
        attr_v12_lite.stManual.LocalSetting.LocalData.LocalAutoWeit   = LocalAutoWeit;
    }
    ret = rk_aiq_user_api2_adrc_v12_lite_SetAttrib(ctx, &attr_v12_lite);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed!");
#endif

    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getDrcLocalData(const rk_aiq_sys_ctx_t* ctx, float * LocalWeit, float * GlobalContrast,
                                        float * LoLitContrast, int* LocalAutoEnable, float* LocalAutoWeit)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }

#if RKAIQ_HAVE_DRC_V10
    ret = XCAM_RETURN_ERROR_PARAM;
    RKAIQ_IMGPROC_CHECK_RET(ret,
                            "RK356x do not support rk_aiq_uapi2_getDrcLocalData! Please use "
                            "rk_aiq_uapi2_getDrcLocalTMO");
#endif
#if RKAIQ_HAVE_DRC_V11
    drcAttrV11_t attr_v11;
    memset(&attr_v11, 0, sizeof(drcAttrV11_t));
    ret = rk_aiq_user_api2_adrc_v11_GetAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcLocalData failed in get attrib!");
    *LocalWeit       = attr_v11.Info.ValidParams.LocalSetting.LocalData.LocalWeit;
    *GlobalContrast  = attr_v11.Info.ValidParams.LocalSetting.LocalData.GlobalContrast;
    *LoLitContrast   = attr_v11.Info.ValidParams.LocalSetting.LocalData.LoLitContrast;
    *LocalAutoEnable = attr_v11.Info.ValidParams.LocalSetting.LocalData.LocalAutoEnable;
    *LocalAutoWeit   = attr_v11.Info.ValidParams.LocalSetting.LocalData.LocalAutoWeit;
#endif
#if RKAIQ_HAVE_DRC_V12
    drcAttrV12_t attr_v12;
    memset(&attr_v12, 0, sizeof(drcAttrV12_t));
    ret = rk_aiq_user_api2_adrc_v12_GetAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcLocalData failed in get attrib!");
    *LocalWeit       = attr_v12.Info.ValidParams.LocalSetting.LocalData.LocalWeit;
    *GlobalContrast  = attr_v12.Info.ValidParams.LocalSetting.LocalData.GlobalContrast;
    *LoLitContrast   = attr_v12.Info.ValidParams.LocalSetting.LocalData.LoLitContrast;
    *LocalAutoEnable = attr_v12.Info.ValidParams.LocalSetting.LocalData.LocalAutoEnable;
    *LocalAutoWeit   = attr_v12.Info.ValidParams.LocalSetting.LocalData.LocalAutoWeit;
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    drcAttrV12Lite_t attr_v12_lite;
    memset(&attr_v12_lite, 0, sizeof(drcAttrV12Lite_t));
    ret = rk_aiq_user_api2_adrc_v12_lite_GetAttrib(ctx, &attr_v12_lite);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcLocalData failed in get attrib!");
    *LocalWeit       = attr_v12_lite.Info.ValidParams.LocalSetting.LocalData.LocalWeit;
    *GlobalContrast  = attr_v12_lite.Info.ValidParams.LocalSetting.LocalData.GlobalContrast;
    *LoLitContrast   = attr_v12_lite.Info.ValidParams.LocalSetting.LocalData.LoLitContrast;
    *LocalAutoEnable = attr_v12_lite.Info.ValidParams.LocalSetting.LocalData.LocalAutoEnable;
    *LocalAutoWeit   = attr_v12_lite.Info.ValidParams.LocalSetting.LocalData.LocalAutoWeit;
#endif

    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set/get manual drc HiLit
*     this function is active for DRC is Auto mode
*     use in RK356x, RV1106
* Argument:
*   Strength: [0, 1]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setDrcHiLit(const rk_aiq_sys_ctx_t* ctx, float Strength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    if (Strength < 0 || Strength > 1) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, Strength range is [0,1]!");
    }
#if RKAIQ_HAVE_DRC_V10
    drcAttrV10_t attr_v10;
    memset(&attr_v10, 0, sizeof(drcAttrV10_t));
    ret = rk_aiq_user_api2_adrc_v10_GetAttrib(ctx, &attr_v10);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcHiLit failed!");
    attr_v10.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v10.sync.done      = false;

    if (attr_v10.opMode == DRC_OPMODE_AUTO) {
        for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++)
            attr_v10.stAuto.DrcTuningPara.HiLight.Strength[i] = Strength;
    } else if (attr_v10.opMode == DRC_OPMODE_MANUAL) {
        attr_v10.stManual.HiLight.Strength = Strength;
    }
    ret = rk_aiq_user_api2_adrc_v10_SetAttrib(ctx, &attr_v10);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcHiLit failed!");
#endif
#if RKAIQ_HAVE_DRC_V11
    drcAttrV11_t attr_v11;
    memset(&attr_v11, 0, sizeof(drcAttrV11_t));
    ret = rk_aiq_user_api2_adrc_v11_GetAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcHiLit failed!");
    attr_v11.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v11.sync.done      = false;

    if (attr_v11.opMode == DRC_OPMODE_AUTO) {
        for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++)
            attr_v11.stAuto.DrcTuningPara.HiLight.Strength[i] = Strength;
    } else if (attr_v11.opMode == DRC_OPMODE_MANUAL) {
        attr_v11.stManual.HiLight.Strength = Strength;
    }
    ret = rk_aiq_user_api2_adrc_v11_SetAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcHiLit failed!");
#endif
#if RKAIQ_HAVE_DRC_V12
    drcAttrV12_t attr_v12;
    memset(&attr_v12, 0, sizeof(drcAttrV12_t));
    ret = rk_aiq_user_api2_adrc_v12_GetAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcHiLit failed!");
    attr_v12.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v12.sync.done      = false;

    if (attr_v12.opMode == DRC_OPMODE_AUTO) {
        for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++)
            attr_v12.stAuto.DrcTuningPara.HiLight.HiLightData.Strength[i] = Strength;
    } else if (attr_v12.opMode == DRC_OPMODE_MANUAL) {
        attr_v12.stManual.HiLight.HiLightData.Strength = Strength;
    }
    ret = rk_aiq_user_api2_adrc_v12_SetAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcHiLit failed!");
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    drcAttrV12Lite_t attr_v12_lite;
    memset(&attr_v12_lite, 0, sizeof(drcAttrV12Lite_t));
    ret = rk_aiq_user_api2_adrc_v12_lite_GetAttrib(ctx, &attr_v12_lite);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcHiLit failed!");
    attr_v12_lite.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v12_lite.sync.done      = false;

    if (attr_v12_lite.opMode == DRC_OPMODE_AUTO) {
        for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++)
            attr_v12_lite.stAuto.DrcTuningPara.HiLight.HiLightData.Strength[i] = Strength;
    } else if (attr_v12_lite.opMode == DRC_OPMODE_MANUAL) {
        attr_v12_lite.stManual.HiLight.HiLightData.Strength = Strength;
    }
    ret = rk_aiq_user_api2_adrc_v12_lite_SetAttrib(ctx, &attr_v12_lite);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcHiLit failed!");
#endif

    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getDrcHiLit(const rk_aiq_sys_ctx_t* ctx, float * Strength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
#if RKAIQ_HAVE_DRC_V10
    drcAttrV10_t attr_v10;
    memset(&attr_v10, 0, sizeof(drcAttrV10_t));
    ret = rk_aiq_user_api2_adrc_v10_GetAttrib(ctx, &attr_v10);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcHiLit failed in get attrib!");
    *Strength = attr_v10.Info.ValidParams.HiLight.Strength;
#endif
#if RKAIQ_HAVE_DRC_V11
    drcAttrV11_t attr_v11;
    memset(&attr_v11, 0, sizeof(drcAttrV11_t));
    ret = rk_aiq_user_api2_adrc_v11_GetAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcHiLit failed in get attrib!");
    *Strength = attr_v11.Info.ValidParams.HiLight.Strength;
#endif
#if RKAIQ_HAVE_DRC_V12
    drcAttrV12_t attr_v12;
    memset(&attr_v12, 0, sizeof(drcAttrV12_t));
    ret = rk_aiq_user_api2_adrc_v12_GetAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcHiLit failed in get attrib!");
    *Strength = attr_v12.Info.ValidParams.HiLight.HiLightData.Strength;
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    drcAttrV12Lite_t attr_v12_lite;
    memset(&attr_v12_lite, 0, sizeof(drcAttrV12Lite_t));
    ret = rk_aiq_user_api2_adrc_v12_lite_GetAttrib(ctx, &attr_v12_lite);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcHiLit failed in get attrib!");
    *Strength = attr_v12_lite.Info.ValidParams.HiLight.HiLightData.Strength;
#endif

    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set/get manual drc Gain
*     this function is active for DRC is Auto mode
*     use in RK356x, RV1106
* Argument:
*   Gain: [1, 8]
*   Alpha: [0, 1]
*   Clip: [0, 64]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setDrcGain(const rk_aiq_sys_ctx_t* ctx, float Gain, float Alpha, float Clip)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    IMGPROC_FUNC_ENTER

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    if (Gain < 1 || Gain > 8) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, Gain range is [1,8]!");
    }
    if (Alpha < 0 || Alpha > 1) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, Alpha range is [0,1]!");
    }
    if (Clip < 0 || Clip > 64) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, Clip range is [0,64]!");
    }
#if RKAIQ_HAVE_DRC_V10
    drcAttrV10_t attr_v10;
    memset(&attr_v10, 0, sizeof(drcAttrV10_t));
    ret = rk_aiq_user_api2_adrc_v10_GetAttrib(ctx, &attr_v10);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcGain failed!");
    attr_v10.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v10.sync.done      = false;

    if (attr_v10.opMode == DRC_OPMODE_AUTO) {
        for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++) {
            attr_v10.stAuto.DrcTuningPara.DrcGain.DrcGain[i] = Gain;
            attr_v10.stAuto.DrcTuningPara.DrcGain.Alpha[i]   = Alpha;
            attr_v10.stAuto.DrcTuningPara.DrcGain.Clip[i]    = Clip;
        }
    } else if (attr_v10.opMode == DRC_OPMODE_MANUAL) {
        attr_v10.stManual.DrcGain.DrcGain = Gain;
        attr_v10.stManual.DrcGain.Alpha   = Alpha;
        attr_v10.stManual.DrcGain.Clip    = Clip;
    }
    ret = rk_aiq_user_api2_adrc_v10_SetAttrib(ctx, &attr_v10);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed!");
#endif
#if RKAIQ_HAVE_DRC_V11
    drcAttrV11_t attr_v11;
    memset(&attr_v11, 0, sizeof(drcAttrV11_t));
    ret = rk_aiq_user_api2_adrc_v11_GetAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcGain failed!");
    attr_v11.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v11.sync.done      = false;

    if (attr_v11.opMode == DRC_OPMODE_AUTO) {
        for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++) {
            attr_v11.stAuto.DrcTuningPara.DrcGain.DrcGain[i] = Gain;
            attr_v11.stAuto.DrcTuningPara.DrcGain.Alpha[i]   = Alpha;
            attr_v11.stAuto.DrcTuningPara.DrcGain.Clip[i]    = Clip;
        }
    } else if (attr_v11.opMode == DRC_OPMODE_MANUAL) {
        attr_v11.stManual.DrcGain.DrcGain = Gain;
        attr_v11.stManual.DrcGain.Alpha   = Alpha;
        attr_v11.stManual.DrcGain.Clip    = Clip;
    }
    ret = rk_aiq_user_api2_adrc_v11_SetAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed!");
#endif
#if RKAIQ_HAVE_DRC_V12
    drcAttrV12_t attr_v12;
    memset(&attr_v12, 0, sizeof(drcAttrV12_t));
    ret = rk_aiq_user_api2_adrc_v12_GetAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcGain failed!");
    attr_v12.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v12.sync.done      = false;

    if (attr_v12.opMode == DRC_OPMODE_AUTO) {
        for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++) {
            attr_v12.stAuto.DrcTuningPara.DrcGain.DrcGain[i] = Gain;
            attr_v12.stAuto.DrcTuningPara.DrcGain.Alpha[i]   = Alpha;
            attr_v12.stAuto.DrcTuningPara.DrcGain.Clip[i]    = Clip;
        }
    } else if (attr_v12.opMode == DRC_OPMODE_MANUAL) {
        attr_v12.stManual.DrcGain.DrcGain = Gain;
        attr_v12.stManual.DrcGain.Alpha   = Alpha;
        attr_v12.stManual.DrcGain.Clip    = Clip;
    }
    ret = rk_aiq_user_api2_adrc_v12_SetAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed!");
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    drcAttrV12Lite_t attr_v12_lite;
    memset(&attr_v12_lite, 0, sizeof(drcAttrV12Lite_t));
    ret = rk_aiq_user_api2_adrc_v12_lite_GetAttrib(ctx, &attr_v12_lite);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getDrcGain failed!");
    attr_v12_lite.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr_v12_lite.sync.done      = false;

    if (attr_v12_lite.opMode == DRC_OPMODE_AUTO) {
        for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++) {
            attr_v12_lite.stAuto.DrcTuningPara.DrcGain.DrcGain[i] = Gain;
            attr_v12_lite.stAuto.DrcTuningPara.DrcGain.Alpha[i]   = Alpha;
            attr_v12_lite.stAuto.DrcTuningPara.DrcGain.Clip[i]    = Clip;
        }
    } else if (attr_v12_lite.opMode == DRC_OPMODE_MANUAL) {
        attr_v12_lite.stManual.DrcGain.DrcGain = Gain;
        attr_v12_lite.stManual.DrcGain.Alpha   = Alpha;
        attr_v12_lite.stManual.DrcGain.Clip    = Clip;
    }
    ret = rk_aiq_user_api2_adrc_v12_lite_SetAttrib(ctx, &attr_v12_lite);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed!");
#endif

    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getDrcGain(const rk_aiq_sys_ctx_t* ctx, float * Gain, float * Alpha, float * Clip)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
#if RKAIQ_HAVE_DRC_V10
    drcAttrV10_t attr_v10;
    memset(&attr_v10, 0, sizeof(drcAttrV10_t));
    ret = rk_aiq_user_api2_adrc_v10_GetAttrib(ctx, &attr_v10);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed in get attrib!");
    *Gain  = attr_v10.Info.ValidParams.DrcGain.DrcGain;
    *Alpha = attr_v10.Info.ValidParams.DrcGain.Alpha;
    *Clip  = attr_v10.Info.ValidParams.DrcGain.Clip;
#endif
#if RKAIQ_HAVE_DRC_V11
    drcAttrV11_t attr_v11;
    memset(&attr_v11, 0, sizeof(drcAttrV11_t));
    ret = rk_aiq_user_api2_adrc_v11_GetAttrib(ctx, &attr_v11);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed in get attrib!");
    *Gain  = attr_v11.Info.ValidParams.DrcGain.DrcGain;
    *Alpha = attr_v11.Info.ValidParams.DrcGain.Alpha;
    *Clip  = attr_v11.Info.ValidParams.DrcGain.Clip;
#endif
#if RKAIQ_HAVE_DRC_V12
    drcAttrV12_t attr_v12;
    memset(&attr_v12, 0, sizeof(drcAttrV12_t));
    ret = rk_aiq_user_api2_adrc_v12_GetAttrib(ctx, &attr_v12);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed in get attrib!");
    *Gain  = attr_v12.Info.ValidParams.DrcGain.DrcGain;
    *Alpha = attr_v12.Info.ValidParams.DrcGain.Alpha;
    *Clip  = attr_v12.Info.ValidParams.DrcGain.Clip;
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    drcAttrV12Lite_t attr_v12_lite;
    memset(&attr_v12_lite, 0, sizeof(drcAttrV12Lite_t));
    ret = rk_aiq_user_api2_adrc_v12_lite_GetAttrib(ctx, &attr_v12_lite);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed in get attrib!");
    *Gain  = attr_v12_lite.Info.ValidParams.DrcGain.DrcGain;
    *Alpha = attr_v12_lite.Info.ValidParams.DrcGain.Alpha;
    *Clip  = attr_v12_lite.Info.ValidParams.DrcGain.Clip;
#endif

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
*   only valid in RV1109/RV1126
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

    return ret;
}

XCamReturn rk_aiq_uapi2_setDarkAreaBoostStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
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
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    /*
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
    return ret;
}
XCamReturn rk_aiq_uapi2_getMHDRStrth(const rk_aiq_sys_ctx_t* ctx, bool * on, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    /*
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

    RKAIQ_IMGPROC_CHECK_RET(ret, "setNRMode failed!", ret);
    IMGPROC_FUNC_EXIT

    return ret;
}


XCamReturn rk_aiq_uapi2_getNRMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER

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

    if (CHECK_ISP_HW_V39() ) {
        rk_aiq_ynr_strength_v24_t ynrV24Strength;
        ynrV24Strength.strength_enable = true;
        ynrV24Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        ynrV24Strength.percent = level / 100.0;
        ret = rk_aiq_user_api2_aynrV24_SetStrength(ctx, &ynrV24Strength);
        rk_aiq_bayertnr_strength_v30_t bayertnrV30Strength;
        bayertnrV30Strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        bayertnrV30Strength.strength_enable = true;
        bayertnrV30Strength.percent = level / 100.0;
        ret = rk_aiq_user_api2_abayertnrV30_SetStrength(ctx, &bayertnrV30Strength);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setANRStrth failed!", ret);
    }


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
    rk_aiq_uapiV2_wb_opMode_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER
    if (mode >= OP_INVAL || mode < OP_AUTO) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "mode is invalid!");
    }
    if (mode == OP_AUTO) {
        attr.mode = RK_AIQ_WB_MODE_AUTO;
    } else if (mode == OP_MANUAL) {
        attr.mode = RK_AIQ_WB_MODE_MANUAL;
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "Not supported mode!");
    }
    ret = rk_aiq_user_api2_awb_SetWpModeAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setWbMode failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getWBMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_uapiV2_wb_opMode_t attr;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_awb_GetWpModeAttrib(ctx, &attr);
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
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_wb_mwb_attrib_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, setMWBScene failed!");
    }

    if (scene < RK_AIQ_WBCT_INCANDESCENT || scene > RK_AIQ_WBCT_SHADE) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "invalid scene mode, setMWBScene failed!");
    }
    rk_aiq_uapiV2_wb_opMode_t opMode;
    memset(&opMode, 0, sizeof(opMode));
    opMode.mode = RK_AIQ_WB_MODE_MANUAL;
    ret = rk_aiq_user_api2_awb_SetWpModeAttrib(ctx, opMode);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setWbMode failed!");
    attr.mode = RK_AIQ_MWB_MODE_SCENE;
    attr.para.scene = scene;
    ret = rk_aiq_user_api2_awb_SetMwbAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMWBScene failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getMWBScene(const rk_aiq_sys_ctx_t* ctx, rk_aiq_wb_scene_t *scene)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_wb_mwb_attrib_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (scene == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getMWBScene failed!");
    }
    //attr.mode = RK_AIQ_WB_MODE_MANUAL;
    ret = rk_aiq_user_api2_awb_GetMwbAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMWBScene failed!");
    if(attr.mode == RK_AIQ_MWB_MODE_SCENE) {
        *scene = attr.para.scene;
    } else {
        LOGE("get manual wb scene failed, since current manual mode is not RK_AIQ_MWB_MODE_SCENE ");
    }
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
XCamReturn rk_aiq_uapi2_setMWBGain(const rk_aiq_sys_ctx_t* ctx, rk_aiq_wb_gain_t *gain)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_wb_mwb_attrib_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (gain == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, setMWBGain failed!");
    }
    rk_aiq_uapiV2_wb_opMode_t opMode;
    memset(&opMode, 0, sizeof(opMode));
    opMode.mode = RK_AIQ_WB_MODE_MANUAL;
    ret = rk_aiq_user_api2_awb_SetWpModeAttrib(ctx, opMode);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setWbMode failed!");
    attr.mode = RK_AIQ_MWB_MODE_WBGAIN;
    attr.para.gain = *gain;
    ret = rk_aiq_user_api2_awb_SetMwbAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMWBGain failed!");
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
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_wb_mwb_attrib_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, setMWBCT failed!");
    }
    rk_aiq_uapiV2_wb_opMode_t opMode;
    memset(&opMode, 0, sizeof(opMode));
    opMode.mode = RK_AIQ_WB_MODE_MANUAL;
    ret = rk_aiq_user_api2_awb_SetWpModeAttrib(ctx, opMode);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setWbMode failed!");
    attr.mode = RK_AIQ_MWB_MODE_CCT;
    attr.para.cct.CCT = (float)ct;
    attr.para.cct.CCRI = 0.0f;
    ret = rk_aiq_user_api2_awb_SetMwbAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMWBCT failed!");
    IMGPROC_FUNC_EXIT
    return ret;

}

XCamReturn rk_aiq_uapi2_getWBCT(const rk_aiq_sys_ctx_t* ctx, unsigned int *ct)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_wb_cct_t cct;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (ct == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getMWBCT failed!");
    }
    ret = rk_aiq_user_api2_awb_GetCCT(ctx, &cct);
    *ct = (unsigned int)cct.CCT;

    IMGPROC_FUNC_EXIT
    return ret;
}


XCamReturn rk_aiq_uapi2_setAwbGainOffsetAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wb_awb_wbGainOffset_t offset)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, setWbGainOffsetAttrib failed!");
    }
    rk_aiq_uapiV2_wb_opMode_t opMode;
    memset(&opMode, 0, sizeof(opMode));
    opMode.mode = RK_AIQ_WB_MODE_AUTO;
    opMode.sync.sync_mode = offset.sync.sync_mode;
    ret = rk_aiq_user_api2_awb_SetWpModeAttrib(ctx, opMode);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setWbMode failed!");
    ret = rk_aiq_user_api2_awb_SetWbGainOffsetAttrib(ctx, offset);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setWbGainOffsetAttrib failed!");
    IMGPROC_FUNC_EXIT
    return ret;

}

XCamReturn rk_aiq_uapi2_getAwbGainOffsetAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wb_awb_wbGainOffset_t *offset)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (offset == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getWbGainOffsetAttrib( failed!");
    }
    ret = rk_aiq_user_api2_awb_GetWbGainOffsetAttrib(ctx, offset);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getWbGainOffsetAttrib failed!");

    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_setAwbGainAdjustAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wb_awb_wbGainAdjust_t adjust)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, setWbGainAdjustAttrib failed!");
    }
    rk_aiq_uapiV2_wb_opMode_t opMode;

    memset(&opMode, 0, sizeof(opMode));
    opMode.sync.sync_mode = adjust.sync.sync_mode;
    opMode.mode = RK_AIQ_WB_MODE_AUTO;
    ret = rk_aiq_user_api2_awb_SetWpModeAttrib(ctx, opMode);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setWbMode failed!");
    ret = rk_aiq_user_api2_awb_SetWbGainAdjustAttrib(ctx, adjust);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setWbGainAdjustAttrib failed!");
    IMGPROC_FUNC_EXIT
    return ret;

}

XCamReturn rk_aiq_uapi2_getAwbGainAdjustAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wb_awb_wbGainAdjust_t *adjust)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (adjust == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getWbGainAdjustAttrib failed!");
    }
    ret = rk_aiq_user_api2_awb_GetWbGainAdjustAttrib(ctx, adjust);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getWbGainAdjustAttrib failed!");

    IMGPROC_FUNC_EXIT
    return ret;
}


XCamReturn rk_aiq_uapi2_setAwbMultiWindowAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wb_awb_mulWindow_t multiwindow)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, setAwbMultiWindowAttrib failed!");
    }
    rk_aiq_uapiV2_wb_opMode_t opMode;
    memset(&opMode, 0, sizeof(opMode));
    opMode.mode = RK_AIQ_WB_MODE_AUTO;
    opMode.sync.sync_mode = multiwindow.sync.sync_mode;
    ret = rk_aiq_user_api2_awb_SetWpModeAttrib(ctx, opMode);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setWbMode failed!");
    ret = rk_aiq_user_api2_awb_SetMultiWindowAttrib(ctx, multiwindow);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setAwbMultiWindowAttrib failed!");
    IMGPROC_FUNC_EXIT
    return ret;

}

XCamReturn rk_aiq_uapi2_getAwbMultiWindowAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wb_awb_mulWindow_t *multiwindow)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (multiwindow == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getAwbMultiWindowAttrib failed!");
    }
    ret = rk_aiq_user_api2_awb_GetMultiWindowAttrib(ctx, multiwindow);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAwbMultiWindowAttrib failed!");

    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_setAwbV30AllAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wbV30_attrib_t attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, setAwbV30AllAttrib failed!");
    }
    ret = rk_aiq_user_api2_awbV30_SetAllAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setAwbV30AllAttrib failed!");
    IMGPROC_FUNC_EXIT
    return ret;

}

XCamReturn rk_aiq_uapi2_getAwbV30AllAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wbV30_attrib_t *attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (attr == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getAwbV30AllAttrib failed!");
    }
    ret = rk_aiq_user_api2_awbV30_GetAllAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAwbV30AllAttrib failed!");

    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_setAwbV21AllAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wbV21_attrib_t attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, setAwbV21AllAttrib failed!");
    }
    ret = rk_aiq_user_api2_awbV21_SetAllAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setAwbV21AllAttrib failed!");
    IMGPROC_FUNC_EXIT
    return ret;

}

XCamReturn rk_aiq_uapi2_getAwbV21AllAttrib(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapiV2_wbV21_attrib_t *attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (attr == NULL)) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getAwbV21AllAttrib failed!");
    }
    ret = rk_aiq_user_api2_awbV21_GetAllAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAwbV21AllAttrib failed!");

    IMGPROC_FUNC_EXIT
    return ret;
}


/*
**********************************************************
* Focus & Zoom
**********************************************************
*/
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
    ret = rk_aiq_user_api_af_GetAttrib(ctx, &attr);
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


XCamReturn rk_aiq_uapi2_setAcolorSwInfo(const rk_aiq_sys_ctx_t* ctx,
                                        rk_aiq_color_info_t aColor_sw_info)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_accm_SetAcolorSwInfo(ctx, aColor_sw_info);
    ret = rk_aiq_user_api2_alsc_SetAcolorSwInfo(ctx, aColor_sw_info);
    ret = rk_aiq_user_api2_a3dlut_SetAcolorSwInfo(ctx, aColor_sw_info);
    IMGPROC_FUNC_EXIT

    return ret;
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
    return ret;
}

XCamReturn rk_aiq_uapi2_getCCMMode(const rk_aiq_sys_ctx_t* ctx, opMode_t* mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
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
    return ret;
}

XCamReturn rk_aiq_uapi2_getMCcCoef(const rk_aiq_sys_ctx_t* ctx, rk_aiq_ccm_matrix_t* mccm)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
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
    return ret;

}

XCamReturn rk_aiq_uapi2_getLut3dMode(const rk_aiq_sys_ctx_t* ctx, opMode_t* mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
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
    return ret;
}

XCamReturn rk_aiq_uapi2_getM3dLut(const rk_aiq_sys_ctx_t* ctx, rk_aiq_lut3d_table_t* mlut)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
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
    return ret;
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
    return ret;
}

XCamReturn rk_aiq_uapi2_setLdchEn(const rk_aiq_sys_ctx_t* ctx, bool en)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    rk_aiq_ldch_attrib_t ldchAttr;
    ret = rk_aiq_user_api2_aldch_GetAttrib(ctx, &ldchAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get ldch attrib failed!");
    ldchAttr.en = en;
    ret = rk_aiq_user_api2_aldch_SetAttrib(ctx, &ldchAttr);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_setLdchCorrectLevel(const rk_aiq_sys_ctx_t* ctx, int correctLevel)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    rk_aiq_ldch_attrib_t ldchAttr;
    ret = rk_aiq_user_api2_aldch_GetAttrib(ctx, &ldchAttr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get ldch attrib failed!");
    ldchAttr.correct_level = correctLevel;
    ret = rk_aiq_user_api2_aldch_SetAttrib(ctx, &ldchAttr);
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_setFecEn(const rk_aiq_sys_ctx_t* ctx, bool en)
{
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
    return ret;
}

XCamReturn rk_aiq_uapi2_setFecBypass(const rk_aiq_sys_ctx_t* ctx, bool bypass)
{
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
    return ret;
}

XCamReturn rk_aiq_uapi2_setFecCorrectLevel(const rk_aiq_sys_ctx_t* ctx, int correctLevel)
{
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
    return ret;
}

XCamReturn rk_aiq_uapi2_setFecCorrectDirection(const rk_aiq_sys_ctx_t* ctx,
        const fec_correct_direction_t direction)
{
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
    return ret;
}

XCamReturn rk_aiq_uapi2_setFecCorrectMode(const rk_aiq_sys_ctx_t* ctx,
        const fec_correct_mode_t mode)
{
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
    return ret;
}

XCamReturn rk_aiq_uapi2_setMirrorFlip(const rk_aiq_sys_ctx_t* ctx, bool mirror, bool flip,
                                      int skip_frm_cnt) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    return ctx->_rkAiqManager->setMirrorFlip(mirror, flip, skip_frm_cnt);
}

XCamReturn rk_aiq_uapi2_getMirrorFlip(const rk_aiq_sys_ctx_t* ctx, bool* mirror, bool* flip) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    if (ctx == NULL || mirror == NULL || flip == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error!");
    }
    return ctx->_rkAiqManager->getMirrorFlip(*mirror, *flip);
}

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
    acp_attrib_t attrib;
    if ((int)level < 0 || level > 255) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "level out of range, setContrast failed!");
    }

    ret = rk_aiq_user_api2_acp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAttrib error,set contrast failed!");
    attrib.contrast = level;
    ret = rk_aiq_user_api2_acp_SetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set contrast failed!");
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_getContrast(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    acp_attrib_t attrib;
    if (level == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getContrast failed!");
    }
    ret = rk_aiq_user_api2_acp_GetAttrib(ctx, &attrib);
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
XCamReturn rk_aiq_uapi2_setBrightness(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
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
    ret = rk_aiq_user_api2_acp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAttrib error,set brightness failed!");
    attrib.brightness = level;
    ret = rk_aiq_user_api2_acp_SetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set brightness failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getBrightness(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    acp_attrib_t attrib;
    if (level == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get brightness failed!");
    }
    ret = rk_aiq_user_api2_acp_GetAttrib(ctx, &attrib);
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
XCamReturn rk_aiq_uapi2_setSaturation(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
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
    ret = rk_aiq_user_api2_acp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAttrib error,set saturation failed!");
    attrib.saturation = level;
    ret = rk_aiq_user_api2_acp_SetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set saturation failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getSaturation(const rk_aiq_sys_ctx_t* ctx, unsigned int* level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    acp_attrib_t attrib;
    if (level == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get saturation failed!");
    }
    ret = rk_aiq_user_api2_acp_GetAttrib(ctx, &attrib);
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
XCamReturn rk_aiq_uapi2_setHue(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
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
    ret = rk_aiq_user_api2_acp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getAttrib error,set hue failed!");
    attrib.hue = level;
    ret = rk_aiq_user_api2_acp_SetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set hue failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getHue(const rk_aiq_sys_ctx_t* ctx, unsigned int* level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    acp_attrib_t attrib;
    if (level == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, get hue failed!");
    }
    ret = rk_aiq_user_api2_acp_GetAttrib(ctx, &attrib);
    RKAIQ_IMGPROC_CHECK_RET(ret, "get hue failed!");
    *level = attrib.hue;
    IMGPROC_FUNC_EXIT
    return ret;
}

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

    return ret;
}

XCamReturn rk_aiq_uapi2_getColorMode(const rk_aiq_sys_ctx_t* ctx, unsigned int *mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
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
    return ret;
}

XCamReturn rk_aiq_uapi2_setGrayMode(const rk_aiq_sys_ctx_t* ctx, rk_aiq_gray_mode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            ret = camCtx->_analyzer->setGrayMode(mode);
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        ret = ctx->_analyzer->setGrayMode(mode);
    }

    return ret;
}

rk_aiq_gray_mode_t rk_aiq_uapi2_getGrayMode(const rk_aiq_sys_ctx_t* ctx)
{
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            return camCtx->_analyzer->getGrayMode();
        }
#else
        return RK_AIQ_GRAY_MODE_OFF;
#endif
    } else {
        return ctx->_analyzer->getGrayMode();
    }

    return RK_AIQ_GRAY_MODE_OFF;
}

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
*         255 OTHER]
*****************************
*/
XCamReturn rk_aiq_uapi2_setColorSpace(const rk_aiq_sys_ctx_t* ctx, int Cspace)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER

    LOGD("setColorSpace enter, Cspace=%d\n", Cspace);
    rk_aiq_uapi_acsm_attrib_t csm_attrib;
    rk_aiq_uapi_acgc_attrib_t cgc_attrib;
    memset(&csm_attrib, 0, sizeof(rk_aiq_uapi_acsm_attrib_t));
    memset(&cgc_attrib, 0, sizeof(rk_aiq_uapi_acgc_attrib_t));
    if (Cspace < 0 || Cspace > 3) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "mode out of range, setColorSpace failed!");
    }

    XCamReturn ret1 = XCAM_RETURN_NO_ERROR;
    XCamReturn ret2 = XCAM_RETURN_NO_ERROR;

    ret1 = rk_aiq_user_api2_acsm_GetAttrib(ctx, &csm_attrib);
    ret2 = rk_aiq_user_api2_acgc_GetAttrib(ctx, &cgc_attrib);
    if (ret1 != XCAM_RETURN_NO_ERROR || ret2 != XCAM_RETURN_NO_ERROR) {
        LOGE("getAttrib error,set ColorSpace failed!");
        return XCAM_RETURN_ERROR_FAILED;
    }

    csm_attrib.param.op_mode = RK_AIQ_OP_MODE_MANUAL;
    cgc_attrib.param.op_mode = RK_AIQ_OP_MODE_MANUAL;
    switch (Cspace) {
    case 0:
        csm_attrib.param.full_range = true;
        csm_attrib.param.y_offset = 0;
        csm_attrib.param.c_offset = 0;
        csm_attrib.param.coeff[0] = 0.299;
        csm_attrib.param.coeff[1] = 0.587;
        csm_attrib.param.coeff[2] = 0.114;
        csm_attrib.param.coeff[3] = -0.169;
        csm_attrib.param.coeff[4] = -0.331;
        csm_attrib.param.coeff[5] = 0.5;
        csm_attrib.param.coeff[6] = 0.5;
        csm_attrib.param.coeff[7] = -0.419;
        csm_attrib.param.coeff[8] = -0.081;
        cgc_attrib.param.cgc_ratio_en = 0;
        cgc_attrib.param.cgc_yuv_limit = 0;
        break;
    case 1:
        csm_attrib.param.full_range = true;
        csm_attrib.param.y_offset = 0;
        csm_attrib.param.c_offset = 0;
        csm_attrib.param.coeff[0] = 0.257;
        csm_attrib.param.coeff[1] = 0.504;
        csm_attrib.param.coeff[2] = 0.098;
        csm_attrib.param.coeff[3] = -0.148;
        csm_attrib.param.coeff[4] = -0.291;
        csm_attrib.param.coeff[5] = 0.439;
        csm_attrib.param.coeff[6] = 0.439;
        csm_attrib.param.coeff[7] = -0.368;
        csm_attrib.param.coeff[8] = -0.071;
        cgc_attrib.param.cgc_ratio_en = 0;
        cgc_attrib.param.cgc_yuv_limit = 1;
        break;
    case 2:
        csm_attrib.param.full_range = true;
        csm_attrib.param.y_offset = 0;
        csm_attrib.param.c_offset = 0;
        csm_attrib.param.coeff[0] = 0.2126;
        csm_attrib.param.coeff[1] = 0.7152;
        csm_attrib.param.coeff[2] = 0.0722;
        csm_attrib.param.coeff[3] = -0.1146;
        csm_attrib.param.coeff[4] = -0.3854;
        csm_attrib.param.coeff[5] = 0.5;
        csm_attrib.param.coeff[6] = 0.5;
        csm_attrib.param.coeff[7] = -0.4542;
        csm_attrib.param.coeff[8] = -0.0468;
        cgc_attrib.param.cgc_ratio_en = 0;
        cgc_attrib.param.cgc_yuv_limit = 0;
        break;
    case 3:
        csm_attrib.param.full_range = true;
        csm_attrib.param.y_offset = 0;
        csm_attrib.param.c_offset = 0;
        csm_attrib.param.coeff[0] = 0.1826;
        csm_attrib.param.coeff[1] = 0.6142;
        csm_attrib.param.coeff[2] = 0.0620;
        csm_attrib.param.coeff[3] = -0.1006;
        csm_attrib.param.coeff[4] = -0.3386;
        csm_attrib.param.coeff[5] = 0.4392;
        csm_attrib.param.coeff[6] = 0.4392;
        csm_attrib.param.coeff[7] = -0.3989;
        csm_attrib.param.coeff[8] = -0.0403;
        cgc_attrib.param.cgc_ratio_en = 0;
        cgc_attrib.param.cgc_yuv_limit = 1;
        break;
    default:
        break;
    }
    ret1 = rk_aiq_user_api2_acsm_SetAttrib(ctx, &csm_attrib);
    ret2 = rk_aiq_user_api2_acgc_SetAttrib(ctx, &cgc_attrib);
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
    rk_aiq_uapi_acsm_attrib_t csm_attrib;
    rk_aiq_uapi_acgc_attrib_t cgc_attrib;
    memset(&csm_attrib, 0, sizeof(rk_aiq_uapi_acsm_attrib_t));
    memset(&cgc_attrib, 0, sizeof(rk_aiq_uapi_acgc_attrib_t));
    if (Cspace == NULL || ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, getColorSpace failed!");
    }
    XCamReturn ret1 = XCAM_RETURN_NO_ERROR;
    XCamReturn ret2 = XCAM_RETURN_NO_ERROR;

    ret1 = rk_aiq_user_api2_acsm_GetAttrib(ctx, &csm_attrib);
    ret2 = rk_aiq_user_api2_acgc_GetAttrib(ctx, &cgc_attrib);
    if (ret1 != XCAM_RETURN_NO_ERROR || ret2 != XCAM_RETURN_NO_ERROR) {
        LOGE("getAttrib error,get ColorSpace failed!");
        return XCAM_RETURN_ERROR_FAILED;
    }

    static Csm_Param_t g_csm_601fa_def = {
        .op_mode = RK_AIQ_OP_MODE_AUTO,
        .full_range = true,
        .y_offset = 0,
        .c_offset = 0,
        .coeff = {
            0.299, 0.587, 0.114,
            -0.169, -0.331, 0.5,
            0.5, -0.419, -0.081
        }
    };
    static Csm_Param_t g_csm_601fm_def = {
        .op_mode = RK_AIQ_OP_MODE_MANUAL,
        .full_range = true,
        .y_offset = 0,
        .c_offset = 0,
        .coeff = {
            0.299, 0.587, 0.114,
            -0.169, -0.331, 0.5,
            0.5, -0.419, -0.081
        }
    };
    static Csm_Param_t g_csm_601l_def = {
        .op_mode = RK_AIQ_OP_MODE_MANUAL,
        .full_range = true,
        .y_offset = 0,
        .c_offset = 0,
        .coeff = {
            0.257, 0.504, 0.098,
            -0.148, -0.291, 0.439,
            0.439, -0.368, -0.071
        }
    };
    static Csm_Param_t g_csm_709fa_def = {
        .op_mode = RK_AIQ_OP_MODE_AUTO,
        .full_range = true,
        .y_offset = 0,
        .c_offset = 0,
        .coeff = {
            0.2126, 0.7152, 0.0722,
            -0.1146, -0.3854, 0.5000,
            0.5000, -0.4542, -0.0468,
        }
    };
    static Csm_Param_t g_csm_709fm_def = {
        .op_mode = RK_AIQ_OP_MODE_MANUAL,
        .full_range = true,
        .y_offset = 0,
        .c_offset = 0,
        .coeff = {
            0.2126, 0.7152, 0.0722,
            -0.1146, -0.3854, 0.5000,
            0.5000, -0.4542, -0.0468,
        }
    };
    static Csm_Param_t g_csm_709l_def = {
        .op_mode = RK_AIQ_OP_MODE_MANUAL,
        .full_range = true,
        .y_offset = 0,
        .c_offset = 0,
        .coeff = {
            0.1826, 0.6142, 0.0620,
            -0.1006, -0.3386, 0.4392,
            0.4392, -0.3989, -0.0403
        }
    };
    if (cgc_attrib.param.cgc_yuv_limit == true) {
        if (!memcmp(&csm_attrib.param, &g_csm_601l_def, sizeof(g_csm_601l_def)))
            *Cspace = 1;
        else if (!memcmp(&csm_attrib.param, &g_csm_709l_def, sizeof(g_csm_709l_def)))
            *Cspace = 3;
        else
            *Cspace = 255;
    } else {
        if (!memcmp(&csm_attrib.param, &g_csm_601fa_def, sizeof(g_csm_601fa_def)) ||
                !memcmp(&csm_attrib.param, &g_csm_601fm_def, sizeof(g_csm_601fm_def)))
            *Cspace = 0;
        else if (!memcmp(&csm_attrib.param, &g_csm_709fa_def, sizeof(g_csm_709fa_def)) ||
                 !memcmp(&csm_attrib.param, &g_csm_709fm_def, sizeof(g_csm_709fm_def)))
            *Cspace = 2;
        else
            *Cspace = 255;
    }

    IMGPROC_FUNC_EXIT
    return ret;
}
RKAIQ_END_DECLARE
