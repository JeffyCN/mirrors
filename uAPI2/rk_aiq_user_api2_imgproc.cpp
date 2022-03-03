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

    rk_aiq_gamma_attrib_V2_t gammaAttr;
    memset(&gammaAttr, 0x0, sizeof(rk_aiq_gamma_attrib_V2_t));
    gammaAttr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    gammaAttr.sync.done = false;
    if(CHECK_ISP_HW_V21()) {
        gammaAttr.atrrV21.mode = RK_AIQ_GAMMA_MODE_FAST;
        gammaAttr.atrrV21.stFast.en = true;
        gammaAttr.atrrV21.stFast.GammaCoef = GammaCoef;
        gammaAttr.atrrV21.stFast.SlopeAtZero = SlopeAtZero;
    }
    else if(CHECK_ISP_HW_V30()) {
        gammaAttr.atrrV30.mode = RK_AIQ_GAMMA_MODE_FAST;
        gammaAttr.atrrV30.stFast.en = true;
        gammaAttr.atrrV30.stFast.GammaCoef = GammaCoef;
        gammaAttr.atrrV30.stFast.SlopeAtZero = SlopeAtZero;
    }

    ret = rk_aiq_user_api2_agamma_SetAttrib(ctx, gammaAttr);
    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set manual dehaze strength
*     this function is active for dehaze is manual mode
* Argument:
*   level: [0, 100]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setMDehazeStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    adehaze_sw_V2_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    if (level < 1 || level > 100) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, strength range is [1,10]!");
    }
    attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr.sync.done = false;
    attr.mode = DEHAZE_API_DEHAZE_MANUAL;
    attr.stDehazeManu.level = level;
    ret = rk_aiq_user_api2_adehaze_setSwAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMDhzStrth failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getMDehazeStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    adehaze_sw_V2_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    ret = rk_aiq_user_api2_adehaze_getSwAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMDhzStrth failed in get attrib!");
    if (attr.mode != DEHAZE_API_DEHAZE_MANUAL) {
        LOGE("Not in Dehaze manual mode!");
        *level = 0;
    } else
        *level = attr.stDehazeManu.level;
    IMGPROC_FUNC_EXIT
    return ret;
}
/*
*****************************
*
* Desc: set manual enhance strength
*     this function is active for dehaze is manual mode
* Argument:
*   level: [0, 100]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setMEnhanceStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    adehaze_sw_V2_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER

    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    if (level < 1 || level > 100) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, level range is [1,10]!");
    }
    attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr.sync.done = false;
    attr.mode = DEHAZE_API_ENHANCE_MANUAL;
    attr.stEnhanceManu.level = level;
    ret = rk_aiq_user_api2_adehaze_setSwAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setMEnhanceStrth failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getMEnhanceStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    adehaze_sw_V2_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
    }
    ret = rk_aiq_user_api2_adehaze_getSwAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getMEnhanceStrth failed in get attrib!");
    if (attr.mode != DEHAZE_API_ENHANCE_MANUAL) {
        LOGE("Not in Enhance manual mode!");
        *level = 0;
    } else
        *level = attr.stEnhanceManu.level;
    IMGPROC_FUNC_EXIT
    return ret;
}

/*
*****************************
*
* Desc: set manual drc Local TMO
*     this function is active for DRC is DRC Gain mode
*     use in RK356x
* Argument:
*   LocalWeit: [0, 1]
*   GlobalContrast: [0, 1]
*   LoLitContrast: [0, 1]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setDrcLocalTMO(const rk_aiq_sys_ctx_t* ctx, float LocalWeit, float GlobalContrast, float LoLitContrast)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(CHECK_ISP_HW_V20()) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ISP2.0 do not support drc api!");
    }
    else if(CHECK_ISP_HW_V30()) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "RK356x do not support rk_aiq_uapi2_setDrcLocalTMO! Plesea use rk_aiq_uapi2_setDrcLocalData");
    }
    else if(CHECK_ISP_HW_V21()) {
        drc_attrib_t attr;
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
        attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
        attr.sync.done = false;
        attr.opMode = DRC_OPMODE_LOCAL_TMO;
        attr.stLocalDataV21.LocalWeit = LocalWeit;
        attr.stLocalDataV21.GlobalContrast = GlobalContrast;
        attr.stLocalDataV21.LoLitContrast = LoLitContrast;

        ret = rk_aiq_user_api2_adrc_SetAttrib(ctx, attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed!");
        IMGPROC_FUNC_EXIT
    }

    return ret;
}

XCamReturn rk_aiq_uapi2_getDrcLocalTMO(const rk_aiq_sys_ctx_t* ctx, float * LocalWeit, float * GlobalContrast, float * LoLitContrast)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(CHECK_ISP_HW_V20()) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ISP2.0 do not support drc api!");
    }
    else if(CHECK_ISP_HW_V30()) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "RK356x do not support rk_aiq_uapi2_getDrcLocalTMO! Plesea use rk_aiq_uapi2_getDrcLocalData");
    }
    else if(CHECK_ISP_HW_V21()) {
        drc_attrib_t attr;
        IMGPROC_FUNC_ENTER
        if (ctx == NULL) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
        }
        ret = rk_aiq_user_api2_adrc_GetAttrib(ctx, &attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed in get attrib!");

        *LocalWeit = attr.stLocalDataV21.LocalWeit;
        *GlobalContrast = attr.stLocalDataV21.GlobalContrast;
        *LoLitContrast = attr.stLocalDataV21.LoLitContrast;

        IMGPROC_FUNC_EXIT
    }

    return ret;
}
/*
*****************************
*
* Desc: set manual drc Local Data
*     this function is active for DRC is DRC Gain mode
*     use in RK3588
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

    if(CHECK_ISP_HW_V20()) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ISP2.0 do not support drc api!");
    }
    else if(CHECK_ISP_HW_V21()) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "RK3588 do not support rk_aiq_uapi2_setDrcLocalData! Plesea use rk_aiq_uapi2_setDrcLocalTMO");
    }
    else if(CHECK_ISP_HW_V30()) {
        drc_attrib_t attr;
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
        attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
        attr.sync.done = false;
        attr.opMode = DRC_OPMODE_LOCAL_TMO;
        attr.stLocalDataV30.LocalWeit = LocalWeit;
        attr.stLocalDataV30.GlobalContrast = GlobalContrast;
        attr.stLocalDataV30.LoLitContrast = LoLitContrast;
        attr.stLocalDataV30.LocalAutoEnable = LocalAutoEnable;
        attr.stLocalDataV30.LocalAutoWeit = LocalAutoWeit;

        ret = rk_aiq_user_api2_adrc_SetAttrib(ctx, attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed!");
        IMGPROC_FUNC_EXIT
    }

    return ret;
}

XCamReturn rk_aiq_uapi2_getDrcLocalData(const rk_aiq_sys_ctx_t* ctx, float * LocalWeit, float * GlobalContrast,
                                        float * LoLitContrast, int* LocalAutoEnable, float* LocalAutoWeit)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(CHECK_ISP_HW_V20()) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ISP2.0 do not support drc api!");
    }
    else if(CHECK_ISP_HW_V21()) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "RK3588 do not support rk_aiq_uapi2_setDrcLocalData! Plesea use rk_aiq_uapi2_setDrcLocalTMO");
    }
    else if(CHECK_ISP_HW_V30()) {
        drc_attrib_t attr;
        IMGPROC_FUNC_ENTER
        if (ctx == NULL) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
        }
        ret = rk_aiq_user_api2_adrc_GetAttrib(ctx, &attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed in get attrib!");

        *LocalWeit = attr.stLocalDataV30.LocalWeit;
        *GlobalContrast = attr.stLocalDataV30.GlobalContrast;
        *LoLitContrast = attr.stLocalDataV30.LoLitContrast;
        *LocalAutoEnable = attr.stLocalDataV30.LocalAutoEnable;
        *LocalAutoWeit = attr.stLocalDataV30.LocalAutoWeit;

        IMGPROC_FUNC_EXIT
    }

    return ret;
}

/*
*****************************
*
* Desc: set manual drc HiLit
*     this function is active for DRC is HiLit mode
* Argument:
*   Strength: [0, 1]
*
*****************************
*/
XCamReturn rk_aiq_uapi2_setDrcHiLit(const rk_aiq_sys_ctx_t* ctx, float Strength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(CHECK_ISP_HW_V20()) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ISP2.0 do not support drc api!");
    }
    else if(CHECK_ISP_HW_V21() || CHECK_ISP_HW_V30()) {
        drc_attrib_t attr;
        IMGPROC_FUNC_ENTER

        if (ctx == NULL) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
        }
        if (Strength < 0 || Strength > 1) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error, Strength range is [0,1]!");
        }
        attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
        attr.sync.done = false;
        attr.opMode = DRC_OPMODE_HILIT;
        attr.stHiLit.Strength = Strength;

        ret = rk_aiq_user_api2_adrc_SetAttrib(ctx, attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed!");
        IMGPROC_FUNC_EXIT
    }

    return ret;
}

XCamReturn rk_aiq_uapi2_getDrcHiLit(const rk_aiq_sys_ctx_t* ctx, float * Strength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(CHECK_ISP_HW_V20()) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ISP2.0 do not support drc api!");
    }
    else if(CHECK_ISP_HW_V21() || CHECK_ISP_HW_V30()) {
        drc_attrib_t attr;
        IMGPROC_FUNC_ENTER
        if (ctx == NULL) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
        }
        ret = rk_aiq_user_api2_adrc_GetAttrib(ctx, &attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed in get attrib!");

        *Strength = attr.stHiLit.Strength;

        IMGPROC_FUNC_EXIT
    }

    return ret;
}

/*
*****************************
*
* Desc: set manual drc Gain
*     this function is active for DRC is DRC Gain mode
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

    if(CHECK_ISP_HW_V20()) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ISP2.0 do not support drc api!");
    }
    else if(CHECK_ISP_HW_V21() || CHECK_ISP_HW_V30()) {
        drc_attrib_t attr;
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
        attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
        attr.sync.done = false;
        attr.opMode = DRC_OPMODE_DRC_GAIN;
        attr.stDrcGain.DrcGain = Gain;
        attr.stDrcGain.Alpha = Alpha;
        attr.stDrcGain.Clip = Clip;

        ret = rk_aiq_user_api2_adrc_SetAttrib(ctx, attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed!");
        IMGPROC_FUNC_EXIT
    }

    return ret;
}

XCamReturn rk_aiq_uapi2_getDrcGain(const rk_aiq_sys_ctx_t* ctx, float * Gain, float * Alpha, float * Clip)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(CHECK_ISP_HW_V20()) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ISP2.0 do not support drc api!");
    }
    else if(CHECK_ISP_HW_V21() || CHECK_ISP_HW_V30()) {
        drc_attrib_t attr;
        IMGPROC_FUNC_ENTER
        if (ctx == NULL) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "param error, ctx is NULL!");
        }
        ret = rk_aiq_user_api2_adrc_GetAttrib(ctx, &attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "setDrcGain failed in get attrib!");
        *Gain = attr.stDrcGain.DrcGain;
        *Alpha = attr.stDrcGain.Alpha;
        *Clip = attr.stDrcGain.Clip;

        IMGPROC_FUNC_EXIT
    }

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
XCamReturn rk_aiq_uapi2_getDarkAreaBoostStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(CHECK_ISP_HW_V20()) {
        atmo_attrib_t attr;
        IMGPROC_FUNC_ENTER
        if (ctx == NULL) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, getDarkAreaBoostStrth failed!");
        }
        if (isHDRmode(ctx)) {
            ret = XCAM_RETURN_ERROR_FAILED;
            RKAIQ_IMGPROC_CHECK_RET(ret, "Not valid in tmo mode!");
        }
        ret = rk_aiq_user_api2_atmo_GetAttrib(ctx, &attr);
        RKAIQ_IMGPROC_CHECK_RET(ret, "getDarkAreaBoostStrth failed!");
        if (attr.opMode == TMO_OPMODE_DARKAREA)
            *level = attr.stDarkArea.level;
        else
            *level = 0;
        IMGPROC_FUNC_EXIT
    }
    else if(CHECK_ISP_HW_V21()) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ISP2.1 do not support tmo api!");
    }
    else if(CHECK_ISP_HW_V30()) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ISP3.0 do not support tmo api!");
    }

    return ret;
}
XCamReturn rk_aiq_uapi2_setDarkAreaBoostStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(CHECK_ISP_HW_V20()) {
        atmo_attrib_t attr;
        IMGPROC_FUNC_ENTER
        if (ctx == NULL) {
            ret = XCAM_RETURN_ERROR_PARAM;
            RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, getDarkAreaBoostStrth failed!");
        }
        if (isHDRmode(ctx)) {
            atmo_attrib_t attr;
            IMGPROC_FUNC_ENTER
            if (ctx == NULL) {
                ret = XCAM_RETURN_ERROR_PARAM;
                RKAIQ_IMGPROC_CHECK_RET(ret, "ctx is null, setMHDRStrth failed!");
            }

            if (isHDRmode(ctx)) {
                ret = XCAM_RETURN_ERROR_FAILED;
                RKAIQ_IMGPROC_CHECK_RET(ret, "Not valid in HDR mode!");
            }
            if (level > 10) {
                ret = XCAM_RETURN_ERROR_OUTOFRANGE;
                RKAIQ_IMGPROC_CHECK_RET(ret, "level(%d) is out of range, setDarkAreaBoostStrth failed!");
            }
            attr.stDarkArea.level = level;
            attr.opMode = TMO_OPMODE_DARKAREA;
            ret = rk_aiq_user_api2_atmo_SetAttrib(ctx, attr);
            RKAIQ_IMGPROC_CHECK_RET(ret, "setDarkAreaBoostStrth failed!");
            IMGPROC_FUNC_EXIT
        }
    }
    else if(CHECK_ISP_HW_V21()) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ISP2.1 do not support tmo api!");
    }
    else if(CHECK_ISP_HW_V30()) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "ISP3.0 do not support tmo api!");
    }

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

    return ret;
}
XCamReturn rk_aiq_uapi2_getMHDRStrth(const rk_aiq_sys_ctx_t* ctx, bool * on, unsigned int *level)
{
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
        ynrStrenght.strength_enable = true;
        ynrStrenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        ynrStrenght.percent = level / 100.0;
        ret = rk_aiq_user_api2_aynrV3_SetStrength(ctx, &ynrStrenght);
        rk_aiq_bayer2dnr_strength_v2_t bayer2dnrV2Strenght;
        bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        bayer2dnrV2Strenght.strength_enable = true;
        bayer2dnrV2Strenght.percent = level / 100.0;
        ret = rk_aiq_user_api2_abayer2dnrV2_SetStrength(ctx, &bayer2dnrV2Strenght);
        rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
        bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        bayertnrV2Strenght.strength_enable = true;
        bayertnrV2Strenght.percent = level / 100.0;
        ret = rk_aiq_user_api2_abayertnrV2_SetStrength(ctx, &bayertnrV2Strenght);
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
    if (level < 0 || level > 100) {
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
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_af_attrib_t attr;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "getFixedModeCode failed!");
    *code = attr.fixedModeDefCode;
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

XCamReturn rk_aiq_uapi2_getCustomAfRes(const rk_aiq_sys_ctx_t* ctx, rk_tool_customAf_res_t *attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_getCustomAfRes(ctx, attr);
    IMGPROC_FUNC_EXIT

    return ret;
}

XCamReturn rk_aiq_uapi2_setCustomAfRes(const rk_aiq_sys_ctx_t* ctx, rk_tool_customAf_res_t *attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_af_setCustomAfRes(ctx, attr);
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
    rk_aiq_ccm_attrib_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER
    if (mode >= OP_INVAL || mode < OP_AUTO) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "mode is invalid!");
    }
    ret = rk_aiq_user_api2_accm_GetAttrib(ctx, &attr);
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
    ret = rk_aiq_user_api2_accm_SetAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "setCCMMode failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getCCMMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_ccm_attrib_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER
    ret = rk_aiq_user_api2_accm_GetAttrib(ctx, &attr);
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
XCamReturn rk_aiq_uapi2_setMCcCoef(const rk_aiq_sys_ctx_t* ctx,  rk_aiq_ccm_matrix_t * mccm)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_ccm_attrib_t attr;
    memset(&attr, 0, sizeof(attr));
    IMGPROC_FUNC_ENTER
    if ((ctx == NULL) || (mccm == NULL) ) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_IMGPROC_CHECK_RET(ret, "param error, set CCM Manual Matrix failed!");
    }

    ret = rk_aiq_user_api2_accm_GetAttrib(ctx, &attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "Set CCM Manual Matrix failed in getting accm attrib!!");

    attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
    attr.mode = RK_AIQ_CCM_MODE_MANUAL;
    memcpy(attr.stManual.ccMatrix, mccm->ccMatrix, sizeof(float) * 9);
    memcpy(attr.stManual.ccOffsets, mccm->ccOffsets, sizeof(float) * 3);
    ret = rk_aiq_user_api2_accm_SetAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set CCM Manual Matrix failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getMCcCoef(const rk_aiq_sys_ctx_t* ctx,  rk_aiq_ccm_matrix_t * mccm)
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
        mccm->ccMatrix[i] = ccm_querry_info.ccMatrix[i];
    for (int i = 0; i < 3; i++)
        mccm->ccOffsets[i] = ccm_querry_info.ccOffsets[i];
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
XCamReturn rk_aiq_uapi2_getACcmSat(const rk_aiq_sys_ctx_t* ctx,  float *finalsat)
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
XCamReturn rk_aiq_uapi2_getACcmMatrixName(const rk_aiq_sys_ctx_t* ctx,  char **ccm_name)
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
    ret = rk_aiq_user_api2_a3dlut_SetAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set 3DLUT Mode failed!");
    IMGPROC_FUNC_EXIT
    return ret;

}

XCamReturn rk_aiq_uapi2_getLut3dMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
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
XCamReturn rk_aiq_uapi2_setM3dLut(const rk_aiq_sys_ctx_t* ctx,  rk_aiq_lut3d_table_t *mlut)
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
    ret = rk_aiq_user_api2_a3dlut_SetAttrib(ctx, attr);
    RKAIQ_IMGPROC_CHECK_RET(ret, "set manual 3d Look-up-table failed!");
    IMGPROC_FUNC_EXIT
    return ret;
}

XCamReturn rk_aiq_uapi2_getM3dLut(const rk_aiq_sys_ctx_t* ctx,  rk_aiq_lut3d_table_t *mlut)
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
XCamReturn rk_aiq_uapi2_getA3dLutStrth(const rk_aiq_sys_ctx_t* ctx,  float *alpha)
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
XCamReturn rk_aiq_uapi2_getA3dLutName(const rk_aiq_sys_ctx_t* ctx,  char *name)
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
    ret = rk_aiq_user_api2_aldch_SetAttrib(ctx, ldchAttr);
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
    ret = rk_aiq_user_api2_aldch_SetAttrib(ctx, ldchAttr);
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

RKAIQ_END_DECLARE
