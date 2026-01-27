/*
 *  Copyright (c) 2024 Rockchip Corporation
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

#include "aiq_sensorHw.h"

#include "common/rk-camera-module.h"
#include "iq_parser_v2/RkAiqCalibDbTypesV2.h"
#include "aiq_CamHwBase.h"
#include "common/rkcif-config.h"

#if RKAIQ_HAVE_DUMPSYS
#include "aiq_sensorHwInfo.h"
#endif

static uint16_t SENSORHW_DEFAULT_POOL_SIZE = 22;

static XCamReturn SensorHw_getSensorDescriptor(AiqSensorHw_t* pBaseSns,
                                               rk_aiq_exposure_sensor_descriptor* sns_des);
static XCamReturn _SensorHw_set_sync_mode(AiqSensorHw_t* pSnsHw, uint32_t mode);
static XCamReturn _SensorHw_set_working_mode(AiqSensorHw_t* pSnsHw, int mode);
static XCamReturn _SensorHw_setI2cDAta(AiqSensorHw_t* pSnsHw, aiq_pending_split_exps_t* exps);
static XCamReturn _set_mirror_flip(AiqSensorHw_t* pSnsHw);

static int _SensorHw_getSensorFps(AiqV4l2SubDevice_t* sd, float* fps) {
    struct v4l2_subdev_frame_interval finterval;

    memset(&finterval, 0, sizeof(finterval));
    finterval.pad = 0;

    if (AiqV4l2SubDevice_ioctl(sd, VIDIOC_SUBDEV_G_FRAME_INTERVAL, &finterval) < 0) return -errno;

    *fps = (float)(finterval.interval.denominator) / finterval.interval.numerator;

    return 0;
}

static XCamReturn _SensorHw_setLinearSensorExposure(AiqSensorHw_t* pSnsHw,
                                                    RKAiqAecExpInfo_t* expPar) {
    ENTER_CAMHW_FUNCTION();
    int frame_line_length;
    struct v4l2_control ctrl;
    rk_aiq_exposure_sensor_descriptor sensor_desc;

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "camId: %d, frameId: %d: a-gain: %d, time: %d, dcg: %d, snr: %d\n",
                    pSnsHw->mCamPhyId, pSnsHw->_frame_sequence,
                    expPar->LinearExp.exp_sensor_params.analog_gain_code_global,
                    expPar->LinearExp.exp_sensor_params.coarse_integration_time,
                    expPar->LinearExp.exp_real_params.dcg_mode, expPar->CISFeature.SNR);

    // set vts before exposure time firstly
    SensorHw_getSensorDescriptor(pSnsHw, &sensor_desc);

    frame_line_length = expPar->frame_length_lines > sensor_desc.line_periods_per_field
                            ? expPar->frame_length_lines
                            : sensor_desc.line_periods_per_field;
#if RKAIQ_HAVE_DUMPSYS
    pSnsHw->desc.frame_length_lines = frame_line_length;
#endif

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id    = V4L2_CID_VBLANK;
    ctrl.value = frame_line_length - sensor_desc.sensor_output_height;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set vblank result(val: %d)",
                        pSnsHw->mCamPhyId, ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    int dcg_mode = expPar->LinearExp.exp_real_params.dcg_mode;
    int dcg_mode_drv;

    if (dcg_mode == 1 /*AEC_DCG_MODE_HCG*/)
        dcg_mode_drv = GAIN_MODE_HCG;
    else if (dcg_mode == 0 /*AEC_DCG_MODE_LCG*/)
        dcg_mode_drv = GAIN_MODE_LCG;
    else  // default
        dcg_mode_drv = -1;

    if (dcg_mode_drv != -1) {
        if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_SET_CONVERSION_GAIN, &dcg_mode_drv) < 0) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set conversion gain !",
                            pSnsHw->mCamPhyId);
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id    = V4L2_CID_ANALOGUE_GAIN;
    ctrl.value = expPar->LinearExp.exp_sensor_params.analog_gain_code_global;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGD_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to  set again result(val: %d)",
                        pSnsHw->mCamPhyId, ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    if (expPar->LinearExp.exp_sensor_params.digital_gain_global != 0) {
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id    = V4L2_CID_GAIN;
        ctrl.value = expPar->LinearExp.exp_sensor_params.digital_gain_global;
        if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set dgain result(val: %d)",
                            pSnsHw->mCamPhyId, ctrl.value);
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    if (expPar->LinearExp.exp_sensor_params.coarse_integration_time != 0) {
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id    = V4L2_CID_EXPOSURE;
        ctrl.value = expPar->LinearExp.exp_sensor_params.coarse_integration_time;
        if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set dgain result(val: %d)",
                            pSnsHw->mCamPhyId, ctrl.value);
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_setHdrSensorExposure(AiqSensorHw_t* pSnsHw, RKAiqAecExpInfo_t* expPar) {
    ENTER_CAMHW_FUNCTION();
    struct hdrae_exp_s hdrExp;
    int frame_line_length;
    struct v4l2_control ctrl;
    rk_aiq_exposure_sensor_descriptor sensor_desc;

    LOGD_CAMHW_SUBM(SENSOR_SUBM,
                    "camId: %d, frameId: %d: lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: 0x%x-0x%x, "
                    "l-dcg %d, m-dcg %d, s-dcg %d\n",
                    pSnsHw->mCamPhyId, pSnsHw->_frame_sequence,
                    expPar->HdrExp[2].exp_sensor_params.analog_gain_code_global,
                    expPar->HdrExp[2].exp_sensor_params.coarse_integration_time,
                    expPar->HdrExp[1].exp_sensor_params.analog_gain_code_global,
                    expPar->HdrExp[1].exp_sensor_params.coarse_integration_time,
                    expPar->HdrExp[0].exp_sensor_params.analog_gain_code_global,
                    expPar->HdrExp[0].exp_sensor_params.coarse_integration_time,
                    expPar->HdrExp[2].exp_real_params.dcg_mode,
                    expPar->HdrExp[1].exp_real_params.dcg_mode,
                    expPar->HdrExp[0].exp_real_params.dcg_mode);

    SensorHw_getSensorDescriptor(pSnsHw, &sensor_desc);

    frame_line_length = expPar->frame_length_lines > sensor_desc.line_periods_per_field
                            ? expPar->frame_length_lines
                            : sensor_desc.line_periods_per_field;
#if RKAIQ_HAVE_DUMPSYS
    pSnsHw->desc.frame_length_lines = frame_line_length;
#endif

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id    = V4L2_CID_VBLANK;
    ctrl.value = frame_line_length - sensor_desc.sensor_output_height;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set vblank result(val: %d)", ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id    = V4L2_CID_ANALOGUE_GAIN;
    ctrl.value = expPar->LinearExp.exp_sensor_params.analog_gain_code_global;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGD_CAMHW_SUBM(SENSOR_SUBM, "failed to  set again result(val: %d)", ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    memset(&hdrExp, 0, sizeof(hdrExp));
    hdrExp.long_exp_reg    = expPar->HdrExp[2].exp_sensor_params.coarse_integration_time;
    hdrExp.long_gain_reg   = expPar->HdrExp[2].exp_sensor_params.analog_gain_code_global;
    hdrExp.middle_exp_reg  = expPar->HdrExp[1].exp_sensor_params.coarse_integration_time;
    hdrExp.middle_gain_reg = expPar->HdrExp[1].exp_sensor_params.analog_gain_code_global;
    hdrExp.short_exp_reg   = expPar->HdrExp[0].exp_sensor_params.coarse_integration_time;
    hdrExp.short_gain_reg  = expPar->HdrExp[0].exp_sensor_params.analog_gain_code_global;

    int dcg_mode = expPar->HdrExp[2].exp_real_params.dcg_mode;

    if (dcg_mode == 1 /*AEC_DCG_MODE_HCG*/)
        hdrExp.long_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0 /*AEC_DCG_MODE_LCG*/)
        hdrExp.long_cg_mode = GAIN_MODE_LCG;
    else  // default
        hdrExp.long_cg_mode = GAIN_MODE_LCG;

    dcg_mode = expPar->HdrExp[1].exp_real_params.dcg_mode;

    if (dcg_mode == 1 /*AEC_DCG_MODE_HCG*/)
        hdrExp.middle_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0 /*AEC_DCG_MODE_LCG*/)
        hdrExp.middle_cg_mode = GAIN_MODE_LCG;
    else  // default
        hdrExp.middle_cg_mode = GAIN_MODE_LCG;

    dcg_mode = expPar->HdrExp[0].exp_real_params.dcg_mode;

    if (dcg_mode == 1 /*AEC_DCG_MODE_HCG*/)
        hdrExp.short_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0 /*AEC_DCG_MODE_LCG*/)
        hdrExp.short_cg_mode = GAIN_MODE_LCG;
    else  // default
        hdrExp.short_cg_mode = GAIN_MODE_LCG;

    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, SENSOR_CMD_SET_HDRAE_EXP, &hdrExp) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set hdrExp exp");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_setLinearSensorExposure2(AiqSensorHw_t* pSnsHw,
                                                     aiq_pending_split_exps_t* expPar) {
    ENTER_CAMHW_FUNCTION();
    int frame_line_length;
    struct v4l2_control ctrl;
    rk_aiq_exposure_sensor_descriptor sensor_desc;

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: cam%d frameid: %u, a-gain: %d, time: %d, dcg: %d\n",
                    __FUNCTION__, pSnsHw->mCamPhyId, pSnsHw->_frame_sequence,
                    expPar->rk_exp_res.sensor_params[0].analog_gain_code_global,
                    expPar->rk_exp_res.sensor_params[0].coarse_integration_time,
                    expPar->rk_exp_res.dcg_mode[0]);

    // set vts before exposure time firstly
    SensorHw_getSensorDescriptor(pSnsHw, &sensor_desc);

    frame_line_length = expPar->rk_exp_res.frame_length_lines > sensor_desc.line_periods_per_field
                            ? expPar->rk_exp_res.frame_length_lines
                            : sensor_desc.line_periods_per_field;
#if RKAIQ_HAVE_DUMPSYS
    pSnsHw->desc.frame_length_lines = frame_line_length;
#endif

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id    = V4L2_CID_VBLANK;
    ctrl.value = frame_line_length - sensor_desc.sensor_output_height;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "cid: %d failed to set vblank result(val: %d)",
                        pSnsHw->mCamPhyId, ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    if (expPar->rk_exp_res.update_bits & (1 << RK_EXP_UPDATE_DCG)) {
        int dcg_mode = expPar->rk_exp_res.dcg_mode[0];

        if (dcg_mode == 1 /*AEC_DCG_MODE_HCG*/)
            pSnsHw->dcg_mode = GAIN_MODE_HCG;
        else if (dcg_mode == 0 /*AEC_DCG_MODE_LCG*/)
            pSnsHw->dcg_mode = GAIN_MODE_LCG;
        else  // default
            pSnsHw->dcg_mode = -1;

        if (pSnsHw->dcg_mode != -1) {
            if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_SET_CONVERSION_GAIN,
                                       &pSnsHw->dcg_mode) < 0) {
                LOGD_CAMHW_SUBM(SENSOR_SUBM, "failed to set conversion gain !");
                return XCAM_RETURN_ERROR_IOCTL;
            }
        }
    }

    if (expPar->rk_exp_res.update_bits & (1 << RK_EXP_UPDATE_GAIN)) {
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id    = V4L2_CID_ANALOGUE_GAIN;
        ctrl.value = expPar->rk_exp_res.sensor_params[0].analog_gain_code_global;
        if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "cid:%d failed to  set again result(val: %d)",
                            pSnsHw->mCamPhyId, ctrl.value);
            return XCAM_RETURN_ERROR_IOCTL;
        }

        if (expPar->rk_exp_res.sensor_params[0].digital_gain_global != 0) {
            memset(&ctrl, 0, sizeof(ctrl));
            ctrl.id    = V4L2_CID_GAIN;
            ctrl.value = expPar->rk_exp_res.sensor_params[0].digital_gain_global;
            if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
                LOGD_CAMHW_SUBM(SENSOR_SUBM, "cid:%d failed to set dgain result(val: %d)",
                                pSnsHw->mCamPhyId, ctrl.value);
                return XCAM_RETURN_ERROR_IOCTL;
            }
        }
    }

    if (expPar->rk_exp_res.update_bits & (1 << RK_EXP_UPDATE_TIME)) {
        if (expPar->rk_exp_res.sensor_params[0].coarse_integration_time != 0) {
            memset(&ctrl, 0, sizeof(ctrl));
            ctrl.id    = V4L2_CID_EXPOSURE;
            ctrl.value = expPar->rk_exp_res.sensor_params[0].coarse_integration_time;
            if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
                LOGD_CAMHW_SUBM(SENSOR_SUBM, "cid:%d failed to set dgain result(val: %d)",
                                pSnsHw->mCamPhyId, ctrl.value);
                return XCAM_RETURN_ERROR_IOCTL;
            }
        }
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_setHdrSensorExposure2(AiqSensorHw_t* pSnsHw,
                                                  aiq_pending_split_exps_t* expPar) {
    ENTER_CAMHW_FUNCTION();
    struct hdrae_exp_s hdrExp;
    int frame_line_length;
    struct v4l2_control ctrl;
    rk_aiq_exposure_sensor_descriptor sensor_desc;

    LOGD_CAMHW_SUBM(SENSOR_SUBM,
                    "camId: %d, frameId: %d: lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: 0x%x-0x%x, "
                    "l-dcg %d, m-dcg %d, s-dcg %d\n",
                    pSnsHw->mCamPhyId, pSnsHw->_frame_sequence,
                    expPar->rk_exp_res.sensor_params[2].analog_gain_code_global,
                    expPar->rk_exp_res.sensor_params[2].coarse_integration_time,
                    expPar->rk_exp_res.sensor_params[1].analog_gain_code_global,
                    expPar->rk_exp_res.sensor_params[1].coarse_integration_time,
                    expPar->rk_exp_res.sensor_params[0].analog_gain_code_global,
                    expPar->rk_exp_res.sensor_params[0].coarse_integration_time,
                    expPar->rk_exp_res.dcg_mode[2], expPar->rk_exp_res.dcg_mode[1],
                    expPar->rk_exp_res.dcg_mode[0]);

    SensorHw_getSensorDescriptor(pSnsHw, &sensor_desc);

    frame_line_length = expPar->rk_exp_res.frame_length_lines > sensor_desc.line_periods_per_field
                            ? expPar->rk_exp_res.frame_length_lines
                            : sensor_desc.line_periods_per_field;
#if RKAIQ_HAVE_DUMPSYS
    pSnsHw->desc.frame_length_lines = frame_line_length;
#endif

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id    = V4L2_CID_VBLANK;
    ctrl.value = frame_line_length - sensor_desc.sensor_output_height;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "cid:%d failed to set vblank result(val: %d)",
                        pSnsHw->mCamPhyId, ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    memset(&hdrExp, 0, sizeof(hdrExp));
    hdrExp.long_exp_reg    = expPar->rk_exp_res.sensor_params[2].coarse_integration_time;
    hdrExp.long_gain_reg   = expPar->rk_exp_res.sensor_params[2].analog_gain_code_global;
    hdrExp.middle_exp_reg  = expPar->rk_exp_res.sensor_params[1].coarse_integration_time;
    hdrExp.middle_gain_reg = expPar->rk_exp_res.sensor_params[1].analog_gain_code_global;
    hdrExp.short_exp_reg   = expPar->rk_exp_res.sensor_params[0].coarse_integration_time;
    hdrExp.short_gain_reg  = expPar->rk_exp_res.sensor_params[0].analog_gain_code_global;

    int dcg_mode = expPar->rk_exp_res.dcg_mode[2];

    if (dcg_mode == 1 /*AEC_DCG_MODE_HCG*/)
        hdrExp.long_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0 /*AEC_DCG_MODE_LCG*/)
        hdrExp.long_cg_mode = GAIN_MODE_LCG;
    else  // default
        hdrExp.long_cg_mode = GAIN_MODE_LCG;

    dcg_mode = expPar->rk_exp_res.dcg_mode[1];

    if (dcg_mode == 1 /*AEC_DCG_MODE_HCG*/)
        hdrExp.middle_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0 /*AEC_DCG_MODE_LCG*/)
        hdrExp.middle_cg_mode = GAIN_MODE_LCG;
    else  // default
        hdrExp.middle_cg_mode = GAIN_MODE_LCG;

    dcg_mode = expPar->rk_exp_res.dcg_mode[0];

    if (dcg_mode == 1 /*AEC_DCG_MODE_HCG*/)
        hdrExp.short_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0 /*AEC_DCG_MODE_LCG*/)
        hdrExp.short_cg_mode = GAIN_MODE_LCG;
    else  // default
        hdrExp.short_cg_mode = GAIN_MODE_LCG;

    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, SENSOR_CMD_SET_HDRAE_EXP, &hdrExp) < 0) {
        LOGD_CAMHW_SUBM(SENSOR_SUBM, "cid:%d failed to set hdrExp exp", pSnsHw->mCamPhyId);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_setSensorDpcc(AiqSensorHw_t* pSnsHw,
                                          Sensor_dpcc_res_t* SensorDpccInfo) {
    struct rkmodule_dpcc_cfg dpcc_cfg;

    dpcc_cfg.enable            = SensorDpccInfo->enable;
    dpcc_cfg.cur_single_dpcc   = SensorDpccInfo->cur_single_dpcc;
    dpcc_cfg.cur_multiple_dpcc = SensorDpccInfo->cur_multiple_dpcc;
    dpcc_cfg.total_dpcc        = SensorDpccInfo->total_dpcc;
    LOG1_CAMHW_SUBM(SENSOR_SUBM, "camId: %d, frameId: %d: enable:%d,single:%d,multi:%d,total:%d",
                    pSnsHw->mCamPhyId, pSnsHw->_frame_sequence, dpcc_cfg.enable,
                    dpcc_cfg.cur_single_dpcc, dpcc_cfg.cur_multiple_dpcc, dpcc_cfg.total_dpcc);
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_SET_DPCC_CFG, &dpcc_cfg) < 0) {
        // LOGE_CAMHW_SUBM(SENSOR_SUBM,"failed to set sensor dpcc");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_composeExpParam(AiqSensorHw_t* pSnsHw, RKAiqAecExpInfo_t* timeValid,
                                            RKAiqAecExpInfo_t* gainValid,
                                            RKAiqAecExpInfo_t* dcgGainModeValid,
                                            RKAiqAecExpInfo_t* newExp) {
    *newExp = *timeValid;
    if (pSnsHw->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        newExp->LinearExp.exp_sensor_params.analog_gain_code_global =
            gainValid->LinearExp.exp_sensor_params.analog_gain_code_global;
        newExp->LinearExp.exp_sensor_params.coarse_integration_time =
            timeValid->LinearExp.exp_sensor_params.coarse_integration_time;
        newExp->LinearExp.exp_real_params.dcg_mode =
            dcgGainModeValid->LinearExp.exp_real_params.dcg_mode;
    } else {
        newExp->HdrExp[2].exp_sensor_params.analog_gain_code_global =
            gainValid->HdrExp[2].exp_sensor_params.analog_gain_code_global;
        newExp->HdrExp[2].exp_sensor_params.coarse_integration_time =
            timeValid->HdrExp[2].exp_sensor_params.coarse_integration_time;
        newExp->HdrExp[2].exp_real_params.dcg_mode =
            dcgGainModeValid->HdrExp[2].exp_real_params.dcg_mode;
        newExp->HdrExp[1].exp_sensor_params.analog_gain_code_global =
            gainValid->HdrExp[1].exp_sensor_params.analog_gain_code_global;
        newExp->HdrExp[1].exp_sensor_params.coarse_integration_time =
            timeValid->HdrExp[1].exp_sensor_params.coarse_integration_time;
        newExp->HdrExp[1].exp_real_params.dcg_mode =
            dcgGainModeValid->HdrExp[1].exp_real_params.dcg_mode;
        newExp->HdrExp[0].exp_sensor_params.analog_gain_code_global =
            gainValid->HdrExp[0].exp_sensor_params.analog_gain_code_global;
        newExp->HdrExp[0].exp_sensor_params.coarse_integration_time =
            timeValid->HdrExp[0].exp_sensor_params.coarse_integration_time;
        newExp->HdrExp[0].exp_real_params.dcg_mode =
            dcgGainModeValid->HdrExp[0].exp_real_params.dcg_mode;
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_split_locked(AiqSensorHw_t* pSnsHw, AiqSensorExpInfo_t* exp_param,
                                         uint32_t sof_id) {
    ENTER_CAMHW_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    uint32_t dst_id = 0, max_dst_id = 0;
    // custom mode
    RKAiqExpI2cParam_t* i2c_param = exp_param->exp_i2c_params;
    if (i2c_param->bValid) {
        unsigned int num_regs = i2c_param->nNumRegs;
        uint32_t i            = 0;
        AiqMapItem_t* pItem   = NULL;

        LOG1_CAMHW_SUBM(SENSOR_SUBM, "i2c_exp_res num_regs %d!", num_regs);
        for (i = 0; i < num_regs; i++) {
            dst_id = sof_id + i2c_param->DelayFrames[i];
            LOG1_CAMHW_SUBM(SENSOR_SUBM, "i2c_exp_res delay: %d, dst_id %d",
                            i2c_param->DelayFrames[i], dst_id);
            if (max_dst_id < dst_id) max_dst_id = dst_id;

            pItem = aiqMap_get(pSnsHw->_pending_spilt_map, (void*)(intptr_t)dst_id);
            if (!pItem) {
                aiq_pending_split_exps_t new_exps;
                memset(&new_exps, 0, sizeof(aiq_pending_split_exps_t));
                new_exps.is_rk_exp_res               = false;
                new_exps.i2c_exp_res.RegAddr[0]      = i2c_param->RegAddr[i];
                new_exps.i2c_exp_res.RegValue[0]     = i2c_param->RegValue[i];
                new_exps.i2c_exp_res.AddrByteNum[0]  = i2c_param->AddrByteNum[i];
                new_exps.i2c_exp_res.ValueByteNum[0] = i2c_param->ValueByteNum[i];
                new_exps.i2c_exp_res.nNumRegs        = 1;
                pItem =
                    aiqMap_insert(pSnsHw->_pending_spilt_map, (void*)(intptr_t)dst_id, &new_exps);
                if (pItem == NULL) {
                    LOGE_CAMHW_SUBM(SENSOR_SUBM, "insert %d failed %d!", dst_id);
                }
            } else {
                aiq_pending_split_exps_t* tmp = (aiq_pending_split_exps_t*)(pItem->_pData);
                unsigned int num_regs         = tmp->i2c_exp_res.nNumRegs;

                if (num_regs >= MAX_I2CDATA_LEN) {
                    LOGE_CAMHW_SUBM(SENSOR_SUBM, "i2c_exp_res array overflow for frame %d!",
                                    dst_id);
                    return XCAM_RETURN_ERROR_FAILED;
                }
                tmp->i2c_exp_res.RegAddr[num_regs]      = i2c_param->RegAddr[i];
                tmp->i2c_exp_res.RegValue[num_regs]     = i2c_param->RegValue[i];
                tmp->i2c_exp_res.AddrByteNum[num_regs]  = i2c_param->AddrByteNum[i];
                tmp->i2c_exp_res.ValueByteNum[num_regs] = i2c_param->ValueByteNum[i];
                tmp->i2c_exp_res.nNumRegs++;
            }
        }

        if (max_dst_id < sof_id) max_dst_id = sof_id + 1;

        exp_param->_base.frame_id = max_dst_id + 1;
        pItem = aiqMap_insert(pSnsHw->_effecting_exp_map, (void*)(intptr_t)(max_dst_id + 1),
                              &exp_param);
        if (pItem == NULL) {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "insert effMap %d failed!", max_dst_id + 1);
        }
        AIQ_REF_BASE_REF(&exp_param->_base._ref_base);

        LOGD_CAMHW_SUBM(SENSOR_SUBM, "cid: %d, num_reg:%d, efid:%d, isp_dgain:%0.3f \n", num_regs,
                        pSnsHw->mCamPhyId, max_dst_id + 1,
                        exp_param->aecExpInfo.LinearExp.exp_real_params.isp_dgain);
    } else {
        RKAiqAecExpInfo_t* exp_info = &exp_param->aecExpInfo;

        uint32_t dst_time_id = sof_id;
        uint32_t dst_gain_id = sof_id + pSnsHw->_time_delay - pSnsHw->_gain_delay;
        uint32_t dst_dcg_id  = sof_id + pSnsHw->_time_delay - pSnsHw->_dcg_gain_mode_delay;

        aiq_pending_split_exps_t new_exps;
        aiq_pending_split_exps_t* p_new_exps = NULL;
        bool is_id_exist                     = true;
        max_dst_id                           = sof_id + pSnsHw->_time_delay;

        int i = 0, ret1 = 0;

        struct {
            uint32_t dst_id;
            uint32_t type;
        } update_exps[3] = {{dst_time_id, RK_EXP_UPDATE_TIME},
                            {dst_gain_id, RK_EXP_UPDATE_GAIN},
                            {dst_dcg_id, RK_EXP_UPDATE_DCG}};

        AiqMapItem_t* pItem = NULL;

        for (i = 0; i < 3; i++) {
            dst_id = update_exps[i].dst_id;

            pItem = aiqMap_get(pSnsHw->_pending_spilt_map, (void*)(intptr_t)dst_id);
            if (!pItem) {
                p_new_exps = &new_exps;
                memset(p_new_exps, 0, sizeof(aiq_pending_split_exps_t));
                is_id_exist = false;
            } else {
                p_new_exps  = (aiq_pending_split_exps_t*)(pItem->_pData);
                is_id_exist = true;
            }

            p_new_exps->is_rk_exp_res = true;
            p_new_exps->rk_exp_res.update_bits |= 1 << update_exps[i].type;
            p_new_exps->rk_exp_res.line_length_pixels   = exp_info->line_length_pixels;
            p_new_exps->rk_exp_res.frame_length_lines   = exp_info->frame_length_lines;
            p_new_exps->rk_exp_res.pixel_clock_freq_mhz = exp_info->pixel_clock_freq_mhz;

            if (pSnsHw->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                if (update_exps[i].type == RK_EXP_UPDATE_TIME) {
                    p_new_exps->rk_exp_res.sensor_params[0].coarse_integration_time =
                        exp_info->LinearExp.exp_sensor_params.coarse_integration_time;
                    p_new_exps->rk_exp_res.sensor_params[0].fine_integration_time =
                        exp_info->LinearExp.exp_sensor_params.fine_integration_time;
                } else if (update_exps[i].type == RK_EXP_UPDATE_GAIN) {
                    p_new_exps->rk_exp_res.sensor_params[0].analog_gain_code_global =
                        exp_info->LinearExp.exp_sensor_params.analog_gain_code_global;
                    p_new_exps->rk_exp_res.sensor_params[0].digital_gain_global =
                        exp_info->LinearExp.exp_sensor_params.digital_gain_global;
                } else if (update_exps[i].type == RK_EXP_UPDATE_DCG) {
                    p_new_exps->rk_exp_res.dcg_mode[0] =
                        exp_info->LinearExp.exp_real_params.dcg_mode;
                } else {
                    LOGE_CAMHW_SUBM(SENSOR_SUBM, "wrong exposure params type %d!",
                                    update_exps[i].type);
                    return XCAM_RETURN_ERROR_FAILED;
                }
            } else {
                if (update_exps[i].type == RK_EXP_UPDATE_TIME) {
                    p_new_exps->rk_exp_res.sensor_params[0].coarse_integration_time =
                        exp_info->HdrExp[0].exp_sensor_params.coarse_integration_time;
                    p_new_exps->rk_exp_res.sensor_params[0].fine_integration_time =
                        exp_info->HdrExp[0].exp_sensor_params.fine_integration_time;

                    p_new_exps->rk_exp_res.sensor_params[1].coarse_integration_time =
                        exp_info->HdrExp[1].exp_sensor_params.coarse_integration_time;
                    p_new_exps->rk_exp_res.sensor_params[1].fine_integration_time =
                        exp_info->HdrExp[1].exp_sensor_params.fine_integration_time;

                    p_new_exps->rk_exp_res.sensor_params[2].coarse_integration_time =
                        exp_info->HdrExp[2].exp_sensor_params.coarse_integration_time;
                    p_new_exps->rk_exp_res.sensor_params[2].fine_integration_time =
                        exp_info->HdrExp[2].exp_sensor_params.fine_integration_time;
                } else if (update_exps[i].type == RK_EXP_UPDATE_GAIN) {
                    p_new_exps->rk_exp_res.sensor_params[0].analog_gain_code_global =
                        exp_info->HdrExp[0].exp_sensor_params.analog_gain_code_global;
                    p_new_exps->rk_exp_res.sensor_params[0].digital_gain_global =
                        exp_info->HdrExp[0].exp_sensor_params.digital_gain_global;

                    p_new_exps->rk_exp_res.sensor_params[1].analog_gain_code_global =
                        exp_info->HdrExp[1].exp_sensor_params.analog_gain_code_global;
                    p_new_exps->rk_exp_res.sensor_params[1].digital_gain_global =
                        exp_info->HdrExp[1].exp_sensor_params.digital_gain_global;

                    p_new_exps->rk_exp_res.sensor_params[2].analog_gain_code_global =
                        exp_info->HdrExp[2].exp_sensor_params.analog_gain_code_global;
                    p_new_exps->rk_exp_res.sensor_params[2].digital_gain_global =
                        exp_info->HdrExp[2].exp_sensor_params.digital_gain_global;
                } else if (update_exps[i].type == RK_EXP_UPDATE_DCG) {
                    p_new_exps->rk_exp_res.dcg_mode[0] =
                        exp_info->HdrExp[0].exp_real_params.dcg_mode;
                    p_new_exps->rk_exp_res.dcg_mode[1] =
                        exp_info->HdrExp[1].exp_real_params.dcg_mode;
                    p_new_exps->rk_exp_res.dcg_mode[2] =
                        exp_info->HdrExp[2].exp_real_params.dcg_mode;
                } else {
                    LOGE_CAMHW_SUBM(SENSOR_SUBM, "wrong exposure params type %d!",
                                    update_exps[i].type);
                    return XCAM_RETURN_ERROR_FAILED;
                }
            }

            if (!is_id_exist) {
                pItem =
                    aiqMap_insert(pSnsHw->_pending_spilt_map, (void*)(intptr_t)dst_id, p_new_exps);
                if (pItem == NULL) {
                    LOGE_CAMHW_SUBM(SENSOR_SUBM, "insert %d failed %d!", dst_id);
                }
            }
        }
        exp_param->_base.frame_id = max_dst_id + 1;
        pItem = aiqMap_insert(pSnsHw->_effecting_exp_map, (void*)(intptr_t)(max_dst_id + 1),
                              &exp_param);
        if (pItem == NULL) {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "insert effMap %d failed!", max_dst_id + 1);
        }
        AIQ_REF_BASE_REF(&exp_param->_base._ref_base);
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn _SensorHw_handleSofInternal(AiqSensorHw_t* pSns, int64_t time, uint32_t frameid) {
    ENTER_CAMHW_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SensorHwExpItem_t snsExpListItem;
    AiqSensorExpInfo_t* new_exp           = NULL;
    bool set_new_exp                      = false;
    AiqMapItem_t* pItem                   = NULL;
    aiq_pending_split_exps_t* pending_exp = NULL;

    aiqMutex_lock(&pSns->_mutex);
    if (pSns->_frame_sequence != (uint32_t)(-1) && (frameid - pSns->_frame_sequence > 1))
        LOGE_CAMHW_SUBM(SENSOR_SUBM,
                        "cam%d !!!!frame losed,last frameid:%u,current farmeid:%u!!!!\n",
                        pSns->mCamPhyId, pSns->_frame_sequence, frameid);

    pSns->_frame_sequence = frameid;

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: cam%d frameid=%u, exp_list size=%d, gain_list size=%d",
                    __FUNCTION__, pSns->mCamPhyId, frameid, aiqList_size(pSns->_exp_list),
                    aiqList_size(pSns->_delayed_gain_list));

    while (aiqMap_size(pSns->_effecting_exp_map) > 7) {
        pItem = aiqMap_begin(pSns->_effecting_exp_map);
        if (pItem) {
            AIQ_REF_BASE_UNREF(&(*((AiqSensorExpInfo_t**)(pItem->_pData)))->_base._ref_base);
			aiqMap_erase(pSns->_effecting_exp_map, pItem->_key);
        }
    }

    if (aiqList_size(pSns->_exp_list) > 0) {
        aiqList_get(pSns->_exp_list, &snsExpListItem);
        new_exp = snsExpListItem._pSnsExp;
        _SensorHw_split_locked(pSns, new_exp, frameid);
    }

    // update flip, skip _frame_sequence
    if (pSns->_update_mirror_flip) {
        _set_mirror_flip(pSns);
        pSns->_update_mirror_flip = false;
    }

    pItem = aiqMap_begin(pSns->_pending_spilt_map);
    while (pItem) {
        if ((unsigned long)(pItem->_key) <= (uint32_t)frameid) {
            pending_exp = (aiq_pending_split_exps_t*)(pItem->_pData);
            aiqMutex_unlock(&pSns->_mutex);
            if (pending_exp->is_rk_exp_res) {
                if (pSns->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                    ret = _SensorHw_setLinearSensorExposure2(pSns, pending_exp);
                } else {
                    ret = _SensorHw_setHdrSensorExposure2(pSns, pending_exp);
                }
            } else {
                _SensorHw_setI2cDAta(pSns, pending_exp);
            }
            aiqMutex_lock(&pSns->_mutex);
            pItem = aiqMap_erase(pSns->_pending_spilt_map, pItem->_key);
        } else {
            break;
        }
    }

    if (aiqMap_size(pSns->_pending_spilt_map) > 100) {
        LOGW_CAMHW_SUBM(SENSOR_SUBM, "cam%d _pending_spilt_map size %d > 100, may be error",
                        pSns->mCamPhyId, aiqMap_size(pSns->_pending_spilt_map));
    }

    aiqMutex_unlock(&pSns->_mutex);

    if (!pSns->_is_i2c_exp && new_exp) _SensorHw_setSensorDpcc(pSns, &new_exp->SensorDpccInfo);

    AIQ_REF_BASE_UNREF(&new_exp->_base._ref_base);

    EXIT_CAMHW_FUNCTION();

    return ret;
}

static XCamReturn _SensorHw_setI2cDAta(AiqSensorHw_t* pSnsHw, aiq_pending_split_exps_t* exps) {
    struct rkmodule_reg regs;

    regs.num_regs         = (__u64)(exps->i2c_exp_res.nNumRegs);
    regs.preg_addr        = (__u64)(unsigned long)(exps->i2c_exp_res.RegAddr);
    regs.preg_value       = (__u64)(unsigned long)(exps->i2c_exp_res.RegValue);
    regs.preg_addr_bytes  = (__u64)(unsigned long)(exps->i2c_exp_res.AddrByteNum);
    regs.preg_value_bytes = (__u64)(unsigned long)(exps->i2c_exp_res.ValueByteNum);

    LOG1_CAMHW_SUBM(SENSOR_SUBM, "set sensor reg array num %d ------", exps->i2c_exp_res.nNumRegs);
    if (exps->i2c_exp_res.nNumRegs <= 0) return XCAM_RETURN_NO_ERROR;

    for (uint32_t i = 0; i < regs.num_regs; i++) {
        LOG1_CAMHW_SUBM(SENSOR_SUBM, "reg:(0x%04x,%d,0x%04x,%d)", exps->i2c_exp_res.RegAddr[i],
                        exps->i2c_exp_res.AddrByteNum[i], exps->i2c_exp_res.RegValue[i],
                        exps->i2c_exp_res.ValueByteNum[i]);
    }

    if (AiqV4l2SubDevice_ioctl(pSnsHw, RKMODULE_SET_REGISTER, &regs) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set i2c regs !");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    return XCAM_RETURN_NO_ERROR;
}

static void _SensorHw_setUserVts(AiqSensorHw_t* pBaseSns) {
    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    if (pBaseSns->userVts) {
        ctrl.id    = V4L2_CID_EXPOSURE;
        if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, VIDIOC_G_CTRL, &ctrl) < 0)
            return;
        if (ctrl.value > (int32_t)(pBaseSns->userVts)) {
            LOGI_CAMHW_SUBM(SENSOR_SUBM, "cam%d ignore user vts:%d < exp:%d",
                            pBaseSns->mCamPhyId, ctrl.value, ctrl.value);
            return;
        }
        memset(&ctrl, 0, sizeof(ctrl));
        rk_aiq_exposure_sensor_descriptor sensor_desc;
        SensorHw_getSensorDescriptor(pBaseSns, &sensor_desc);
        ctrl.id    = V4L2_CID_VBLANK;
        ctrl.value = pBaseSns->userVts - sensor_desc.sensor_output_height;
        LOGI_CAMHW_SUBM(SENSOR_SUBM, "cam%d use user vts:%d",
                        pBaseSns->mCamPhyId, ctrl.value);

        if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set vblank result(val: %d)",
                            pBaseSns->mCamPhyId, ctrl.value);
        }
    }
}

static XCamReturn _SensorHw_updateBlcWbgain(AiqSensorHw_t* pSnsHw, uint32_t frmId) {
    AiqCamHwBase_t* pCamHw = pSnsHw->_mCamHw;

    if (pCamHw && (pCamHw->_airms_en || pCamHw->mSnsDes.compr_bit)) {
        aiq_isp_effect_params_t* ispParams = NULL;
        AiqCamHw_getEffectiveIspParams(pCamHw, &ispParams, frmId);

        if (ispParams) {
            struct rkmodule_blc_info blc_cfg_info;
            xcam_mem_clear(blc_cfg_info);
            if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_GET_BLC_INFO, &blc_cfg_info) < 0) {
                LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to get blc_cfg_info");
            }
            //LOGE_CAMHW_SUBM(SENSOR_SUBM, "blc_cfg_info:%d", blc_cfg_info.bit_width);
            struct rkmodule_wb_gain_info wbg_cfg_info;
            xcam_mem_clear(wbg_cfg_info);
            if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_GET_WB_GAIN_INFO, &wbg_cfg_info) < 0) {
                LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to get wbg_cfg_info");
            }
            //LOGE_CAMHW_SUBM(SENSOR_SUBM, "wbg_cfg_info:%d,%d", wbg_cfg_info.coarse_bit,wbg_cfg_info.fine_bit);
            // pDstEff->bls_cfg
            struct rkmodule_blc_group blc;
            xcam_mem_clear(blc);
            blc.group_num = 4;
            float meanBlc = 0;
            meanBlc += ispParams->bls_cfg.bls1_val.r;
            meanBlc += ispParams->bls_cfg.bls1_val.gr;
            meanBlc += ispParams->bls_cfg.bls1_val.gb;
            meanBlc += ispParams->bls_cfg.bls1_val.b;
            meanBlc /= 4;
            int blc_shift = 12 - blc_cfg_info.bit_width;//12bit to 10 bit
            //LOGE_AWB("en:%d,meanBlc:%0x, blc_shift:%d", ispParams->bls_cfg.enable_auto, (__u32)meanBlc, blc_shift);
            if(blc_shift > 0) {
                for(uint32_t i = 0; i < blc.group_num; i++) {
                    blc.blc_type[i] = (enum rkmodule_blc_type)i;
                    blc.blc[i] = (__u32) (meanBlc) >> blc_shift;
                }
            } else {
                for(uint32_t i = 0; i < blc.group_num; i++) {
                    blc.blc_type[i] = (enum rkmodule_blc_type)i;
                    blc.blc[i] = (__u32) (meanBlc) << (-blc_shift);
                }
            }
            //LOGE_AWB("blc:%0x, %0x, %0x, %0x", blc.blc[0], blc.blc[1], blc.blc[2], blc.blc[3]);
            //if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_SET_BLC, &blc) < 0) {
            //    LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set blc:%d", blc.group_num);
            //}
            // pDstEff->awb_gain_cfg
            struct rkmodule_wb_gain_group wbgain;
            xcam_mem_clear(wbgain);
            wbgain.group_num = 4;
            int wbgain_shift = 8 - wbg_cfg_info.fine_bit;//8bit to 10bit
            if (pSnsHw->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                if(wbgain_shift < 0) {
                    for(uint32_t i = 0; i < wbgain.group_num; i++) {
                        wbgain.wb_gain_type[i] = (enum rkmodule_wb_type)i;
                        wbgain.wb_gain[i].r_gain = ispParams->awb_gain_cfg.awb1_gain_r << (-wbgain_shift);
                        wbgain.wb_gain[i].gr_gain = ispParams->awb_gain_cfg.awb1_gain_gr <<(-wbgain_shift);
                        wbgain.wb_gain[i].gb_gain = ispParams->awb_gain_cfg.awb1_gain_gb << (-wbgain_shift);
                        wbgain.wb_gain[i].b_gain = ispParams->awb_gain_cfg.awb1_gain_b << (-wbgain_shift);
                    }
                } else {
                    for(uint32_t i = 0; i < wbgain.group_num; i++) {
                        wbgain.wb_gain_type[i] = (enum rkmodule_wb_type)i;
                        wbgain.wb_gain[i].r_gain = ispParams->awb_gain_cfg.awb1_gain_r >> wbgain_shift;
                        wbgain.wb_gain[i].gr_gain = ispParams->awb_gain_cfg.awb1_gain_gr >> wbgain_shift;
                        wbgain.wb_gain[i].gb_gain = ispParams->awb_gain_cfg.awb1_gain_gb >> wbgain_shift;
                        wbgain.wb_gain[i].b_gain = ispParams->awb_gain_cfg.awb1_gain_b >> wbgain_shift;
                    }
                }
            } else {
                if(wbgain_shift < 0) {
                    for(uint32_t i = 0; i < wbgain.group_num; i++) {
                        wbgain.wb_gain_type[i] = (enum rkmodule_wb_type)i;
                        wbgain.wb_gain[i].r_gain = ispParams->awb_gain_cfg.gain0_red << (-wbgain_shift);
                        wbgain.wb_gain[i].gr_gain = ispParams->awb_gain_cfg.gain0_green_r <<(-wbgain_shift);
                        wbgain.wb_gain[i].gb_gain = ispParams->awb_gain_cfg.gain0_green_b << (-wbgain_shift);
                        wbgain.wb_gain[i].b_gain = ispParams->awb_gain_cfg.gain0_blue << (-wbgain_shift);
                    }
                }else {
                    for(uint32_t i = 0; i < wbgain.group_num; i++) {
                        wbgain.wb_gain_type[i] = (enum rkmodule_wb_type)i;
                        wbgain.wb_gain[i].r_gain = ispParams->awb_gain_cfg.gain0_red >> wbgain_shift;
                        wbgain.wb_gain[i].gr_gain = ispParams->awb_gain_cfg.gain0_green_r >> wbgain_shift;
                        wbgain.wb_gain[i].gb_gain = ispParams->awb_gain_cfg.gain0_green_b >> wbgain_shift;
                        wbgain.wb_gain[i].b_gain = ispParams->awb_gain_cfg.gain0_blue >> wbgain_shift;
                    }
                }
            }

            //LOGE_AWB("wbgain:%0x, %0x, %0x, %0x", wbgain.wb_gain[0].r_gain, wbgain.wb_gain[0].gr_gain, wbgain.wb_gain[0].gb_gain, wbgain.wb_gain[0].b_gain);
            if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_SET_WB_GAIN, &wbgain) < 0) {
                LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set wbgain:%d", wbgain.group_num);
            }
            AIQ_REF_BASE_UNREF(&ispParams->_ref_base);
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "fid:%d, set wbgain & blc", frmId);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn SensorHw_handle_sof(AiqSensorHw_t* pBaseSns, int64_t time, uint32_t frameid) {
    ENTER_CAMHW_FUNCTION();
    int effecting_frame_id            = 0;
    XCamReturn ret                    = XCAM_RETURN_NO_ERROR;
    AiqSensorHw_t* pSns               = (AiqSensorHw_t*)pBaseSns;
    AiqSensorExpInfo_t* exp_time      = NULL;
    AiqSensorExpInfo_t* exp_gain      = NULL;
    AiqSensorExpInfo_t* dcg_gain_mode = NULL;
    SensorHwExpItem_t snsExpListItem;
    struct rkcif_exp _rkcif_exp;
    struct rkcif_effect_exp _rkcif_effect_map;
    bool set_time = false, set_gain = false, set_dcg_gain_mode = false;
    AiqMapItem_t* pItem = NULL;

    // TODO: only i2c exps using new handler now
    if (pSns->_is_i2c_exp) {
        return _SensorHw_handleSofInternal(pBaseSns, time, frameid);
    }

    aiqMutex_lock(&pSns->_mutex);
    if (pSns->_frame_sequence != (uint32_t)(-1) && frameid - pSns->_frame_sequence > 1)
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "!!!!frame losed,last frameid:%u,current farmeid:%u!!!!\n",
                        pSns->_frame_sequence, frameid);

    pSns->_frame_sequence = frameid;
    LOGV_CAMHW_SUBM(SENSOR_SUBM, "%s:cam%d frameid=%u, exp_list size=%d, gain_list size=%d",
                    __FUNCTION__, pSns->mCamPhyId, frameid, aiqList_size(pSns->_exp_list),
                    aiqList_size(pSns->_delayed_gain_list));

    while (aiqMap_size(pSns->_effecting_exp_map) > 5) {
        pItem = aiqMap_begin(pSns->_effecting_exp_map);
        if (pItem) {
            AIQ_REF_BASE_UNREF(&(*((AiqSensorExpInfo_t**)(pItem->_pData)))->_base._ref_base);
            aiqMap_erase(pSns->_effecting_exp_map, pItem->_key);
        }
    }

    if (aiqList_size(pSns->_exp_list) > 0) {
        aiqList_get(pSns->_exp_list, &snsExpListItem);
        AIQ_REF_BASE_UNREF(&pSns->_last_exp_time->_base._ref_base);
        exp_time = pSns->_last_exp_time = snsExpListItem._pSnsExp;
        set_time                        = true;
    } else {
        exp_time = pSns->_last_exp_time;
    }

    if (aiqList_size(pSns->_delayed_gain_list) > 0) {
        aiqList_get(pSns->_delayed_gain_list, &exp_gain);
        AIQ_REF_BASE_UNREF(&pSns->_last_exp_gain->_base._ref_base);
        pSns->_last_exp_gain = exp_gain;
        set_gain             = true;
    } else {
        exp_gain = pSns->_last_exp_gain;
    }

    if (aiqList_size(pSns->_delayed_dcg_gain_mode_list) > 0) {
        aiqList_get(pSns->_delayed_dcg_gain_mode_list, &dcg_gain_mode);
        AIQ_REF_BASE_UNREF(&pSns->_last_dcg_gain_mode->_base._ref_base);
        pSns->_last_dcg_gain_mode = dcg_gain_mode;
        set_dcg_gain_mode         = true;
    } else {
        dcg_gain_mode = pSns->_last_dcg_gain_mode;
    }

    aiqMutex_unlock(&pSns->_mutex);
    // update flip, skip _frame_sequence
    if (pSns->_update_mirror_flip) {
        _set_mirror_flip(pSns);
        pSns->_update_mirror_flip = false;
    }

    LOGD_CAMHW_SUBM(
        SENSOR_SUBM, "%s: cam%d working_mode=%d,frameid=%u, status: set_time=%d,set_gain=%d\n",
        __FUNCTION__, pSns->mCamPhyId, pSns->_working_mode, frameid, set_time, set_gain);

    if (set_time || set_gain || set_dcg_gain_mode) {
        if (pSns->mVicapExpFd >= 0) {
            _rkcif_exp.time = exp_time->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time;
            _rkcif_exp.gain = exp_time->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global;
            LOGV_CAMHW_SUBM(SENSOR_SUBM, "%s:cam%d sof_id[%d], time: %d, a-gain: %d\n",
                            __FUNCTION__, pSns->mCamPhyId, frameid, _rkcif_exp.time, _rkcif_exp.gain);
            if (xcam_device_ioctl(pSns->mVicapExpFd, RKCIF_CMD_SET_EXPOSURE, &_rkcif_exp) < 0) {
                LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set rkcif_exp!");
                return XCAM_RETURN_ERROR_IOCTL;
            }
        } else {
            RKAiqAecExpInfo_t *ptr_new_exp = NULL, new_exp;
            if (pSns->_dcg_gain_mode_delayed) {
                // _gain_delayed should be false
                _SensorHw_composeExpParam(pSns, &exp_time->aecExpInfo, &exp_time->aecExpInfo,
                                          &dcg_gain_mode->aecExpInfo, &new_exp);
                ptr_new_exp = &new_exp;
            } else {
                if (pSns->_gain_delayed) {
                    if (pSns->_dcg_gain_mode_with_time)
                        dcg_gain_mode = exp_time;
                    else
                        dcg_gain_mode = exp_gain;
                    _SensorHw_composeExpParam(pSns, &exp_time->aecExpInfo, &exp_gain->aecExpInfo,
                                              &dcg_gain_mode->aecExpInfo, &new_exp);
                    ptr_new_exp = &new_exp;
                } else {
                    ptr_new_exp = &exp_time->aecExpInfo;
                }
            }

            if (pSns->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                ret = _SensorHw_setLinearSensorExposure(pSns, ptr_new_exp);
            } else {
                ret = _SensorHw_setHdrSensorExposure(pSns, ptr_new_exp);
            }
        }

        if (ret != XCAM_RETURN_NO_ERROR)
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: sof_id[%u]: set exposure failed!!!\n", __FUNCTION__,
                            frameid);

        _SensorHw_setSensorDpcc(pSns, &exp_time->SensorDpccInfo);
    }

    _SensorHw_setUserVts(pSns);

    if (set_time) {
        aiqMutex_lock(&pSns->_mutex);

        if (pSns->mVicapExpFd >= 0) {
            if (xcam_device_ioctl(pSns->mVicapExpFd, RKCIF_CMD_GET_EFFECT_EXPOSURE, &_rkcif_effect_map) < 0) {
                aiqMutex_unlock(&pSns->_mutex);
                LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to get rkcif_exp effecting map!");
                return XCAM_RETURN_ERROR_FAILED;
            }
            aiqMap_insert(pSns->_effecting_exp_map, (void*)(intptr_t)_rkcif_effect_map.sequence, &exp_time);
            AIQ_REF_BASE_REF(&exp_time->_base._ref_base);
        } else {
            if (pSns->_gain_delayed) {
                aiqList_push(pSns->_delayed_gain_list, &exp_time);
                AIQ_REF_BASE_REF(&exp_time->_base._ref_base);
            }

            if (pSns->_dcg_gain_mode_delayed) {
                aiqList_push(pSns->_delayed_dcg_gain_mode_list, &exp_time);
                AIQ_REF_BASE_REF(&exp_time->_base._ref_base);
            }
            effecting_frame_id = frameid + pSns->_time_delay;
            if (pSns->mPauseFlag && (pSns->mIsSingleMode || (frameid == pSns->mPauseId)) &&
                pSns->_time_delay > 1) {
                effecting_frame_id = frameid + 1;
            }

            exp_time->_base.frame_id = effecting_frame_id;
            aiqMap_insert(pSns->_effecting_exp_map, (void*)(intptr_t)effecting_frame_id, &exp_time);
            AIQ_REF_BASE_REF(&exp_time->_base._ref_base);

            if (pSns->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                LOGV_CAMHW_SUBM(
                    SENSOR_SUBM,
                    "%s:cam%d sof_id[%d], _effecting_exp_map: add %d, a-gain: %d, time: %d, snr: %d\n",
                    __FUNCTION__, pSns->mCamPhyId, frameid, effecting_frame_id,
                    exp_time->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
                    exp_time->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time,
                    exp_time->aecExpInfo.CISFeature.SNR);
            } else {
                LOGV_CAMHW_SUBM(
                    SENSOR_SUBM,
                    "%s:cam%d sof_id[%d], _effecting_exp_map: add %d, lexp: 0x%x-0x%x, mexp: "
                    "0x%x-0x%x, sexp: 0x%x-0x%x\n",
                    __FUNCTION__, pSns->mCamPhyId, frameid, effecting_frame_id,
                    exp_time->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
                    exp_time->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
                    exp_time->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
                    exp_time->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
                    exp_time->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global,
                    exp_time->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time);
            }
        }
        aiqMutex_unlock(&pSns->_mutex);
    }

    _SensorHw_updateBlcWbgain(pSns, frameid + 1);

    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn _set_mirror_flip(AiqSensorHw_t* pSnsHw) {
    struct v4l2_control ctrl;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id    = V4L2_CID_HFLIP;
    ctrl.value = pSnsHw->_mirror ? 1 : 0;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set hflip (val: %d)", ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    ctrl.id    = V4L2_CID_VFLIP;
    ctrl.value = pSnsHw->_flip ? 1 : 0;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set vflip (val: %d)", ctrl.value);
    }

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "set mirror %d, flip %d", pSnsHw->_mirror, pSnsHw->_flip);

    return XCAM_RETURN_NO_ERROR;
}

static uint32_t sensorHw_get_v4l2_pixelformat(uint32_t pixelcode) {
    uint32_t pixelformat = -1;

    switch (pixelcode) {
        case MEDIA_BUS_FMT_SRGGB8_1X8:
            pixelformat = V4L2_PIX_FMT_SRGGB8;
            break;
        case MEDIA_BUS_FMT_SBGGR8_1X8:
            pixelformat = V4L2_PIX_FMT_SBGGR8;
            break;
        case MEDIA_BUS_FMT_SGBRG8_1X8:
            pixelformat = V4L2_PIX_FMT_SGBRG8;
            break;
        case MEDIA_BUS_FMT_SGRBG8_1X8:
            pixelformat = V4L2_PIX_FMT_SGRBG8;
            break;
        case MEDIA_BUS_FMT_SBGGR10_1X10:
            pixelformat = V4L2_PIX_FMT_SBGGR10;
            break;
        case MEDIA_BUS_FMT_SRGGB10_1X10:
            pixelformat = V4L2_PIX_FMT_SRGGB10;
            break;
        case MEDIA_BUS_FMT_SGBRG10_1X10:
            pixelformat = V4L2_PIX_FMT_SGBRG10;
            break;
        case MEDIA_BUS_FMT_SGRBG10_1X10:
            pixelformat = V4L2_PIX_FMT_SGRBG10;
            break;
        case MEDIA_BUS_FMT_SRGGB12_1X12:
            pixelformat = V4L2_PIX_FMT_SRGGB12;
            break;
        case MEDIA_BUS_FMT_SBGGR12_1X12:
            pixelformat = V4L2_PIX_FMT_SBGGR12;
            break;
        case MEDIA_BUS_FMT_SGBRG12_1X12:
            pixelformat = V4L2_PIX_FMT_SGBRG12;
            break;
        case MEDIA_BUS_FMT_SGRBG12_1X12:
            pixelformat = V4L2_PIX_FMT_SGRBG12;
            break;
        case MEDIA_BUS_FMT_Y8_1X8:
            pixelformat = V4L2_PIX_FMT_GREY;
            break;
        case MEDIA_BUS_FMT_Y10_1X10:
            pixelformat = V4L2_PIX_FMT_Y10;
            break;
        case MEDIA_BUS_FMT_Y12_1X12:
            pixelformat = V4L2_PIX_FMT_Y12;
            break;
        case MEDIA_BUS_FMT_SBGGR14_1X14:
            pixelformat = V4L2_PIX_FMT_SBGGR14;
            break;
        case MEDIA_BUS_FMT_SGBRG14_1X14:
            pixelformat = V4L2_PIX_FMT_SGBRG14;
            break;
        case MEDIA_BUS_FMT_SGRBG14_1X14:
            pixelformat = V4L2_PIX_FMT_SGRBG14;
            break;
        case MEDIA_BUS_FMT_SRGGB14_1X14:
            pixelformat = V4L2_PIX_FMT_SRGGB14;
            break;
        case MEDIA_BUS_FMT_SBGGR16_1X16:
            pixelformat = V4L2_PIX_FMT_SBGGR16;
            break;
        case MEDIA_BUS_FMT_SGBRG16_1X16:
            pixelformat = V4L2_PIX_FMT_SGBRG16;
            break;
        case MEDIA_BUS_FMT_SGRBG16_1X16:
            pixelformat = V4L2_PIX_FMT_SGRBG16;
            break;
        case MEDIA_BUS_FMT_SRGGB16_1X16:
            pixelformat = V4L2_PIX_FMT_SRGGB16;
            break;
        default:
            // TODO add other
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s no support pixelcode:0x%x\n", __func__, pixelcode);
    }
    return pixelformat;
}

static int SensorHw_getSensorDesc(AiqSensorHw_t* pBaseSns,
                                  rk_aiq_exposure_sensor_descriptor* sns_des) {
    struct v4l2_subdev_format fmt;

    memset(&fmt, 0, sizeof(fmt));
    fmt.pad   = 0;
    fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;

    if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, VIDIOC_SUBDEV_G_FMT, &fmt) < 0) return -errno;

    sns_des->sensor_output_width  = fmt.format.width;
    sns_des->sensor_output_height = fmt.format.height;
    sns_des->sensor_pixelformat   = sensorHw_get_v4l2_pixelformat(fmt.format.code);
    return 0;
}

static int SensorHw_getDcgRatio(AiqSensorHw_t* pBaseSns, rk_aiq_sensor_dcg_ratio_t* dcg_ratio) {
    struct rkmodule_dcg_ratio dcg_ratio_drv;

    if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, RKMODULE_GET_DCG_RATIO, &dcg_ratio_drv) < 0) {
        // LOGD_CAMHW_SUBM(SENSOR_SUBM,"failed to get sensor dcg_ratio");
        dcg_ratio->valid = false;
        return XCAM_RETURN_ERROR_IOCTL;
    }

    dcg_ratio->valid     = true;
    dcg_ratio->integer   = dcg_ratio_drv.integer;
    dcg_ratio->decimal   = dcg_ratio_drv.decimal;
    dcg_ratio->div_coeff = dcg_ratio_drv.div_coeff;

    return 0;
}

static int SensorHw_getSpdRatio(AiqSensorHw_t* pBaseSns, rk_aiq_sensor_dcg_ratio_t* spd_ratio) {
    struct rkmodule_dcg_ratio spd_ratio_drv;
    if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, RKMODULE_GET_SPD_RATIO, &spd_ratio_drv) < 0) {
        LOGD_CAMHW_SUBM(SENSOR_SUBM, "failed to get sensor spd_ratio");
        spd_ratio->valid = false;
        return XCAM_RETURN_ERROR_IOCTL;
    }
    spd_ratio->valid     = true;
    spd_ratio->integer   = spd_ratio_drv.integer;
    spd_ratio->decimal   = spd_ratio_drv.decimal;
    spd_ratio->div_coeff = spd_ratio_drv.div_coeff;
    return 0;
}

static int SensorHw_getNrSwitch(AiqSensorHw_t* pBaseSns, rk_aiq_sensor_nr_switch_t* nr_switch) {
    struct rkmodule_nr_switch_threshold nr_switch_drv;

    if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, RKMODULE_GET_NR_SWITCH_THRESHOLD, &nr_switch_drv) <
        0) {
        // LOGE_CAMHW_SUBM(SENSOR_SUBM,"failed to get sensor nr switch");
        nr_switch->valid = false;
        return XCAM_RETURN_ERROR_IOCTL;
    }

    nr_switch->valid      = true;
    nr_switch->direct     = nr_switch_drv.direct;
    nr_switch->up_thres   = nr_switch_drv.up_thres;
    nr_switch->down_thres = nr_switch_drv.down_thres;
    nr_switch->div_coeff  = nr_switch_drv.div_coeff;

    return 0;
}

static int SensorHw_getExposureRange(AiqSensorHw_t* pBaseSns,
                                     rk_aiq_exposure_sensor_descriptor* sns_des) {
    struct v4l2_queryctrl ctrl;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_EXPOSURE;

    if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, VIDIOC_QUERYCTRL, &ctrl) < 0) return -errno;

    sns_des->coarse_integration_time_min        = ctrl.minimum;
    sns_des->coarse_integration_time_max_margin = 10;

    return 0;
}

static int SensorHw_getPixel(AiqSensorHw_t* pBaseSns, rk_aiq_exposure_sensor_descriptor* sns_des) {
    struct v4l2_ext_controls controls;
    struct v4l2_ext_control ext_control;
    signed long pixel;

    memset(&controls, 0, sizeof(controls));
    memset(&ext_control, 0, sizeof(ext_control));

    ext_control.id      = V4L2_CID_PIXEL_RATE;
    controls.ctrl_class = V4L2_CTRL_ID2CLASS(ext_control.id);
    controls.count      = 1;
    controls.controls   = &ext_control;

    if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, VIDIOC_G_EXT_CTRLS, &controls) < 0) return -errno;

    pixel = ext_control.value64;

    sns_des->pixel_clock_freq_mhz = (float)pixel / 1000000;

    return 0;
}

static int SensorHw_getBlank(AiqSensorHw_t* pBaseSns, rk_aiq_exposure_sensor_descriptor* sns_des) {
    struct v4l2_queryctrl ctrl;
    int horzBlank, vertBlank;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_HBLANK;
    if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, VIDIOC_QUERYCTRL, &ctrl) < 0) {
        return -errno;
    }
    horzBlank = ctrl.minimum;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_VBLANK;
    if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, VIDIOC_QUERYCTRL, &ctrl) < 0) {
        return -errno;
    }
    vertBlank = ctrl.minimum;

    sns_des->pixel_periods_per_line = horzBlank + sns_des->sensor_output_width;
    sns_des->line_periods_per_field = vertBlank + sns_des->sensor_output_height;

    return 0;
}

static int SensorHw_getBayerMode(AiqSensorHw_t* pBaseSns, enum rkmodule_bayer_mode *bayer_mode) {
    enum rkmodule_bayer_mode mode;

    if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, RKMODULE_GET_BAYER_MODE, &mode) < 0) {
        *bayer_mode = RKMODULE_NORMAL_BAYER;
    }

    *bayer_mode = mode;

    return 0;
}

XCamReturn SensorHw_getSensorModeData(AiqSensorHw_t* pBaseSns, const char* sns_ent_name,
                                      rk_aiq_exposure_sensor_descriptor* sns_des) {
    rk_aiq_exposure_sensor_descriptor sensor_desc;

    SensorHw_getSensorDescriptor(pBaseSns, &sensor_desc);

    pBaseSns->_sns_entity_name                  = sns_ent_name;
    sns_des->coarse_integration_time_min        = sensor_desc.coarse_integration_time_min;
    sns_des->coarse_integration_time_max_margin = sensor_desc.coarse_integration_time_max_margin;
    sns_des->fine_integration_time_min          = sensor_desc.fine_integration_time_min;
    sns_des->fine_integration_time_max_margin   = sensor_desc.fine_integration_time_max_margin;

    sns_des->frame_length_lines   = sensor_desc.line_periods_per_field;
    sns_des->line_length_pck      = sensor_desc.pixel_periods_per_line;
    sns_des->vt_pix_clk_freq_hz   = sensor_desc.pixel_clock_freq_mhz * 1000000;
    sns_des->pixel_clock_freq_mhz = sensor_desc.pixel_clock_freq_mhz /* * 1000000 */;

    // add nr_switch
    sns_des->nr_switch = sensor_desc.nr_switch;
    sns_des->dcg_ratio = sensor_desc.dcg_ratio;
    sns_des->spd_ratio = sensor_desc.spd_ratio;

    sns_des->sensor_output_width  = sensor_desc.sensor_output_width;
    sns_des->sensor_output_height = sensor_desc.sensor_output_height;
    sns_des->sensor_pixelformat   = sensor_desc.sensor_pixelformat;

    pBaseSns->_sensor_desc = *sns_des;

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "vts-hts-pclk: %d-%d-%d-%f, rect: [%dx%d]\n",
                    sns_des->frame_length_lines, sns_des->line_length_pck,
                    sns_des->vt_pix_clk_freq_hz, sns_des->pixel_clock_freq_mhz,
                    sns_des->sensor_output_width, sns_des->sensor_output_height);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn SensorHw_getSensorDescriptor(AiqSensorHw_t* pBaseSns,
                                               rk_aiq_exposure_sensor_descriptor* sns_des) {
    float fps = 0;
    memset(sns_des, 0, sizeof(rk_aiq_exposure_sensor_descriptor));

    if (SensorHw_getSensorDesc(pBaseSns, sns_des)) return XCAM_RETURN_ERROR_IOCTL;

    if (SensorHw_getBlank(pBaseSns, sns_des)) return XCAM_RETURN_ERROR_IOCTL;

    /*
     * pixel rate is not equal to pclk sometimes
     * prefer to use pclk = ppl * lpp * fps
     */
    if (_SensorHw_getSensorFps(pBaseSns->mSd, &fps) == 0)
        sns_des->pixel_clock_freq_mhz = (float)(sns_des->pixel_periods_per_line) *
                                        sns_des->line_periods_per_field * fps / 1000000.0;
    else if (SensorHw_getPixel(pBaseSns, sns_des))
        return XCAM_RETURN_ERROR_IOCTL;

    if (SensorHw_getExposureRange(pBaseSns, sns_des)) return XCAM_RETURN_ERROR_IOCTL;

    if (SensorHw_getNrSwitch(pBaseSns, &sns_des->nr_switch)) {
        // do nothing;
    }
    if (SensorHw_getDcgRatio(pBaseSns, &sns_des->dcg_ratio)) {
        // do nothing;
    }
    if (SensorHw_getSpdRatio(pBaseSns, &sns_des->spd_ratio)) {
        // do nothing;
    }
    SensorHw_getBayerMode(pBaseSns, &sns_des->bayer_mode);

    AiqSensorHw_getMergeWgtCurve(pBaseSns, &sns_des->mge_oe_wgt);

#if RKAIQ_HAVE_DUMPSYS
    memcpy(&pBaseSns->desc, sns_des, sizeof(*sns_des));
#endif

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn SensorHw_setExposureParams(AiqSensorHw_t* pBaseSns,
                                             AiqAecExpInfoWrapper_t* expPar) {
    ENTER_CAMHW_FUNCTION();

    AiqSensorHw_t* pSns             = (AiqSensorHw_t*)pBaseSns;
    AiqAecExpInfoWrapper_t* aec_exp = expPar;
    AiqSensorExpInfo_t* pSnsExp     = NULL;
    AiqMapItem_t* pItem             = NULL;
    int ret                         = 0;

    aiqMutex_lock(&pSns->_mutex);

    if (pSns->_first) {
        if (aec_exp->algo_id == 0) {
            if (aec_exp->ae_proc_res_rk.exp_set_cnt > 0) {
                int lastIdx         = aec_exp->ae_proc_res_rk.exp_set_cnt - 1;
                aec_exp->new_ae_exp = aec_exp->ae_proc_res_rk.exp_set_tbl[lastIdx];
            }
        }
        if (!aec_exp->exp_i2c_params.bValid) {
            pSns->_is_i2c_exp = false;
            if (pSns->_working_mode == RK_AIQ_WORKING_MODE_NORMAL)
                _SensorHw_setLinearSensorExposure(pSns, &aec_exp->new_ae_exp);
            else
                _SensorHw_setHdrSensorExposure(pSns, &aec_exp->new_ae_exp);
            _SensorHw_setSensorDpcc(pSns, &aec_exp->SensorDpccInfo);
        } else {
            pSns->_is_i2c_exp = true;
            aiq_pending_split_exps_t new_exps;
            uint32_t i = 0;
            memset(&new_exps, 0, sizeof(aiq_pending_split_exps_t));
            new_exps.i2c_exp_res.nNumRegs = aec_exp->exp_i2c_params.nNumRegs;
            for (i = 0; i < aec_exp->exp_i2c_params.nNumRegs; i++) {
                new_exps.i2c_exp_res.RegAddr[i]      = aec_exp->exp_i2c_params.RegAddr[i];
                new_exps.i2c_exp_res.RegValue[i]     = aec_exp->exp_i2c_params.RegValue[i];
                new_exps.i2c_exp_res.AddrByteNum[i]  = aec_exp->exp_i2c_params.AddrByteNum[i];
                new_exps.i2c_exp_res.ValueByteNum[i] = aec_exp->exp_i2c_params.ValueByteNum[i];
            }
            _SensorHw_setI2cDAta(pSns, &new_exps);
        }

        if (aiqPool_freeNums(pSns->_expParamsPool)) {
            AiqPoolItem_t* pItem = aiqPool_getFree(pSns->_expParamsPool);
            pSnsExp              = (AiqSensorExpInfo_t*)(pItem->_pData);
        } else {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: no free params buffer!\n", __FUNCTION__);
            aiqMutex_unlock(&pSns->_mutex);
            return XCAM_RETURN_ERROR_MEM;
        }

        pSnsExp->aecExpInfo     = aec_exp->new_ae_exp;
        pSnsExp->SensorDpccInfo = aec_exp->SensorDpccInfo;
        pSnsExp->exp_i2c_params = &aec_exp->exp_i2c_params;

        pSnsExp->_base.frame_id = 0;
        pItem = aiqMap_insert(pSns->_effecting_exp_map, (void*)(intptr_t)0, &pSnsExp);
        if (!pItem) {
            AIQ_REF_BASE_UNREF(&pSnsExp->_base._ref_base);
            aiqMutex_unlock(&pSns->_mutex);
            return XCAM_RETURN_ERROR_FAILED;
        }
        pSns->_first = false;

        if (pSns->_last_exp_time) AIQ_REF_BASE_UNREF(&pSns->_last_exp_time->_base._ref_base);
        pSns->_last_exp_time = pSnsExp;
        AIQ_REF_BASE_REF(&pSnsExp->_base._ref_base);

        if (pSns->_last_exp_gain) AIQ_REF_BASE_UNREF(&pSns->_last_exp_gain->_base._ref_base);
        pSns->_last_exp_gain = pSnsExp;
        AIQ_REF_BASE_REF(&pSnsExp->_base._ref_base);

        if (pSns->_last_dcg_gain_mode)
            AIQ_REF_BASE_UNREF(&pSns->_last_dcg_gain_mode->_base._ref_base);
        pSns->_last_dcg_gain_mode = pSnsExp;
        AIQ_REF_BASE_REF(&pSnsExp->_base._ref_base);

        aec_exp->exp_i2c_params.bValid      = false;
        aec_exp->ae_proc_res_rk.exp_set_cnt = 0;
        LOGD_CAMHW_SUBM(SENSOR_SUBM,
                        "exp-sync: first set exp, add id[0] to the effected exp map\n");
    } else {
        if (aec_exp->algo_id == 0) {
            if (aec_exp->ae_proc_res_rk.exp_set_cnt > 0) {
                SensorHwExpItem_t tmp;
                aiq_memset(&tmp, 0, sizeof(SensorHwExpItem_t));

                LOGV_CAMHW_SUBM(SENSOR_SUBM, "%s: exp_tbl_size:%d, exp_list remain:%d\n",
                                __FUNCTION__, aec_exp->ae_proc_res_rk.exp_set_cnt,
                                aiqList_size(pSns->_exp_list));
                /* when new exp-table comes, remove elem until meet the first one of last exp-table
                 */
				AiqListItem_t* pItem = NULL;
				bool rm = false;
				AIQ_LIST_FOREACH(pSns->_exp_list, pItem, rm) {
					SensorHwExpItem_t* pSnsExp = (SensorHwExpItem_t*)pItem->_pData;
					if (!pSnsExp->_isFirst) {
						AIQ_REF_BASE_UNREF(&pSnsExp->_pSnsExp->_base._ref_base);
						pItem = aiqList_erase_item_locked(pSns->_exp_list, pItem);
						rm = true;
					}
				}

                for (int i = 0; i < aec_exp->ae_proc_res_rk.exp_set_cnt; i++) {
                    if (aiqPool_freeNums(pSns->_expParamsPool)) {
                        AiqPoolItem_t* pItem = aiqPool_getFree(pSns->_expParamsPool);
                        pSnsExp              = (AiqSensorExpInfo_t*)(pItem->_pData);
                    } else {
                        LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: no free params buffer!\n", __FUNCTION__);
                        aiqMutex_unlock(&pSns->_mutex);
                        return XCAM_RETURN_ERROR_MEM;
                    }

                    pSnsExp->aecExpInfo = aec_exp->new_ae_exp;
                    pSnsExp->aecExpInfo.LinearExp =
                        aec_exp->ae_proc_res_rk.exp_set_tbl[i].LinearExp;
                    pSnsExp->aecExpInfo.HdrExp[0] =
                        aec_exp->ae_proc_res_rk.exp_set_tbl[i].HdrExp[0];
                    pSnsExp->aecExpInfo.HdrExp[1] =
                        aec_exp->ae_proc_res_rk.exp_set_tbl[i].HdrExp[1];
                    pSnsExp->aecExpInfo.HdrExp[2] =
                        aec_exp->ae_proc_res_rk.exp_set_tbl[i].HdrExp[2];
                    pSnsExp->aecExpInfo.frame_length_lines =
                        aec_exp->ae_proc_res_rk.exp_set_tbl[i].frame_length_lines;
                    pSnsExp->aecExpInfo.CISFeature.SNR =
                        aec_exp->ae_proc_res_rk.exp_set_tbl[i].CISFeature.SNR;
                    pSnsExp->SensorDpccInfo = aec_exp->SensorDpccInfo;
                    pSnsExp->exp_i2c_params = &aec_exp->exp_i2c_params;

                    /* set a flag when it's fisrt elem of exp-table*/
                    tmp._pSnsExp = pSnsExp;
                    tmp._isFirst = (i == 0 ? true : false);
                    ret          = aiqList_push(pSns->_exp_list, &tmp);
                    if (ret) {
                        LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: pending list is full !\n", __FUNCTION__);
                        AIQ_REF_BASE_UNREF(&pSnsExp->_base._ref_base);
                        aiqMutex_unlock(&pSns->_mutex);
                        return XCAM_RETURN_ERROR_MEM;
                    }

                    if (pSns->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                        LOGV_CAMHW_SUBM(
                            SENSOR_SUBM,
                            "%s:cam%d add tbl[%d] to list: a-gain: %d, time: %d, snr: %d\n",
                            __FUNCTION__, pSns->mCamPhyId, i,
                            pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
                            pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time,
                            pSnsExp->aecExpInfo.CISFeature.SNR);
                    } else {
                        LOGV_CAMHW_SUBM(
                            SENSOR_SUBM,
                            "%s:cam%d add tbl[%d] to list: lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: "
                            "0x%x-0x%x\n",
                            __FUNCTION__, pSns->mCamPhyId, i,
                            pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
                            pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
                            pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
                            pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
                            pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global,
                            pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time);
                    }
                }
                aec_exp->exp_i2c_params.bValid      = false;
                aec_exp->ae_proc_res_rk.exp_set_cnt = 0;
            }
        } else {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "unsurpported now !");
        }
    }
    aiqMutex_unlock(&pSns->_mutex);
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

AiqSensorExpInfo_t* SensorHw_getEffectiveExpParams(AiqSensorHw_t* pSnsHw, uint32_t frame_id) {
    ENTER_CAMHW_FUNCTION();

    XCamReturn ret              = XCAM_RETURN_NO_ERROR;
    AiqSensorHw_t* pSns         = (AiqSensorHw_t*)pSnsHw;
    AiqSensorExpInfo_t* pSnsExp = NULL;
    AiqMapItem_t* pItem         = NULL;
    uint32_t search_id          = frame_id == (uint32_t)(-1) ? 0 : frame_id;
    aiqMutex_lock(&pSns->_mutex);

    pItem = aiqMap_get(pSns->_effecting_exp_map, (void*)(intptr_t)search_id);
    // havn't found
    if (!pItem) {
        /* use the latest */
		AiqMapItem_t* pLastItem         = NULL;
		bool rm = false;
		AIQ_MAP_FOREACH(pSns->_effecting_exp_map, pItem, rm) {
			if ((uint32_t)(long)pItem->_key >= search_id)
				break;
			pLastItem = pItem;
		}

		pItem = pLastItem;
        if (pItem) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM,
                            "use effecting exposure of %d for %d, may be something wrong !",
                            (uint32_t)(long)(pItem->_key), search_id);
        } else {
            LOGE_CAMHW_SUBM(SENSOR_SUBM,
                            "can't find the latest effecting exposure for id %d, impossible case !",
                            search_id);
            aiqMutex_unlock(&pSns->_mutex);
            return NULL;
        }

        pSnsExp = *((AiqSensorExpInfo_t**)(pItem->_pData));
    } else {
        pSnsExp = *((AiqSensorExpInfo_t**)(pItem->_pData));
    }

    if (pSns->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        LOG1_CAMHW_SUBM(SENSOR_SUBM, "%s:cam%d search_id: %d, get-last %d, a-gain: %d, time: %d\n",
                        __FUNCTION__, pSns->mCamPhyId, search_id, (uint32_t)(long)(pItem->_key),
                        pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
                        pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time);
    } else {
        LOG1_CAMHW_SUBM(SENSOR_SUBM,
                        "%s:cam%d search_id: %d, get-last %d, lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, "
                        "sexp: 0x%x-0x%x\n",
                        __FUNCTION__, pSns->mCamPhyId, search_id, (uint32_t)(long)(pItem->_key),
                        pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
                        pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
                        pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
                        pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
                        pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global,
                        pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time);
    }

    AIQ_REF_BASE_REF(&pSnsExp->_base._ref_base);
    aiqMutex_unlock(&pSns->_mutex);
    EXIT_CAMHW_FUNCTION();

    return pSnsExp;
}

static XCamReturn _SensorHw_getExpMode(AiqSensorHw_t* pSnsHw, uint32_t *mode) {
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_GET_EXP_MODE, mode) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to get exp mode");
        return XCAM_RETURN_ERROR_IOCTL;
    }
    LOGD_CAMHW_SUBM(SENSOR_SUBM, "get exposure mode: %d\n", *mode);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_setExpMode(AiqSensorHw_t* pSnsHw, uint32_t mode) {
    uint32_t defMode = 0;
    if (mode == (uint32_t)-1) {
        _SensorHw_updateBlcWbgain(pSnsHw, 0);
        return XCAM_RETURN_NO_ERROR;
    }

    if (_SensorHw_getExpMode(pSnsHw, &defMode))
        return XCAM_RETURN_ERROR_IOCTL;

    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_SET_EXP_MODE, &mode) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set exp mode %d, use mode %d", mode, defMode);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    pSnsHw->mCisHdrMode = mode;

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "set exposure mode: %d\n", mode);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_set_working_mode(AiqSensorHw_t* pSnsHw, int mode) {
    struct rkmodule_hdr_cfg hdr_cfg;
    struct rkmodule_hdr_cfg hdr_cfg_get;
    __u32 hdr_mode = NO_HDR;

    xcam_mem_clear(hdr_cfg);
    if (mode == RK_AIQ_WORKING_MODE_NORMAL) {
        hdr_mode = NO_HDR;
    } else if (mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR || mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
        hdr_mode = HDR_X2;
    } else if (mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR || mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
        hdr_mode = HDR_X3;
    } else if (RK_AIQ_HDR_IS_SENSOR_BUILTIN(mode)) {
        hdr_mode = HDR_CIS_MERGE;
    } else {
        LOGW_CAMHW_SUBM(SENSOR_SUBM, "failed to set hdr mode to %d", mode);
        return XCAM_RETURN_ERROR_FAILED;
    }
    hdr_cfg.hdr_mode = hdr_mode;

    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_GET_HDR_CFG, &hdr_cfg_get) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set hdr mode %d", hdr_mode);
    } else {
        if (hdr_cfg.hdr_mode == hdr_cfg_get.hdr_mode) {
            goto out;
        }
    }

    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_SET_HDR_CFG, &hdr_cfg) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set hdr mode %d", hdr_mode);
        // return XCAM_RETURN_ERROR_IOCTL;
    }

out:
    pSnsHw->_working_mode = mode;

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s _working_mode: %d\n", __func__, pSnsHw->_working_mode);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_set_exp_delay_info(AiqSensorHw_t* pSnsHw, int time_delay,
                                               int gain_delay, int hcg_lcg_mode_delay) {
    pSnsHw->_time_delay          = time_delay;
    pSnsHw->_gain_delay          = gain_delay;
    pSnsHw->_dcg_gain_mode_delay = hcg_lcg_mode_delay;

    LOG1_CAMHW_SUBM(SENSOR_SUBM, "%s _time_delay: %d, _gain_delay:%d, _dcg_delay:%d\n", __func__,
                    pSnsHw->_time_delay, pSnsHw->_gain_delay, pSnsHw->_dcg_gain_mode_delay);
    if (pSnsHw->_time_delay > pSnsHw->_gain_delay) {
        pSnsHw->_gain_delayed = true;
    } else if (pSnsHw->_time_delay == pSnsHw->_gain_delay) {
        pSnsHw->_gain_delayed = false;
    } else {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "Not support gain's delay greater than time's delay!");
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (pSnsHw->_dcg_gain_mode_delay > time_delay) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "Not support dcg gain's delay %d, greater than time_delay %d!",
                        pSnsHw->_dcg_gain_mode_delay, time_delay);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (pSnsHw->_dcg_gain_mode_delay > 0 && pSnsHw->_dcg_gain_mode_delay != time_delay &&
        pSnsHw->_dcg_gain_mode_delay != pSnsHw->_gain_delay) {
        pSnsHw->_dcg_gain_mode_delayed = true;
    } else {
        if (pSnsHw->_dcg_gain_mode_delay == time_delay)
            pSnsHw->_dcg_gain_mode_with_time = true;
        else
            pSnsHw->_dcg_gain_mode_with_time = false;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_set_mirror_flip(AiqSensorHw_t* pSnsHw, bool mirror, bool flip,
                                            int32_t* skip_frame_sequence) {
    aiqMutex_lock(&pSnsHw->_mutex);

    pSnsHw->_flip   = flip;
    pSnsHw->_mirror = mirror;
    _set_mirror_flip(pSnsHw);
    *skip_frame_sequence = pSnsHw->_frame_sequence;
    if (*skip_frame_sequence < 0) *skip_frame_sequence = 0;

    // if (pSnsHw->_mirror != mirror || pSnsHw->_flip != flip) {
    //     pSnsHw->_flip   = flip;
    //     pSnsHw->_mirror = mirror;
    //     // will be set at _frame_sequence + 1
    //     pSnsHw->_update_mirror_flip = true;
    //     // skip pre and current frame
    //     *skip_frame_sequence = pSnsHw->_frame_sequence;
    //     if (*skip_frame_sequence < 0) *skip_frame_sequence = 0;
    // } else
    //     *skip_frame_sequence = -1;

    aiqMutex_unlock(&pSnsHw->_mutex);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_get_mirror_flip(AiqSensorHw_t* pSnsHw, bool* mirror, bool* flip) {
    struct v4l2_control ctrl;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_HFLIP;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_G_CTRL, &ctrl) < 0) {
        LOGW_CAMHW_SUBM(SENSOR_SUBM, "failed to set hflip (val: %d)", ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    *mirror = ctrl.value ? true : false;

    ctrl.id = V4L2_CID_VFLIP;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_G_CTRL, &ctrl) < 0) {
        LOGW_CAMHW_SUBM(SENSOR_SUBM, "failed to set vflip (val: %d)", ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    *flip = ctrl.value ? true : false;

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_start(AiqSensorHw_t* pSnsHw, bool prepared) {
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)pSnsHw->mSd;
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;

    ENTER_CAMHW_FUNCTION();

    ret = (*v4l2_dev->start)(v4l2_dev, prepared);

    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn _SensorHw_stop(AiqSensorHw_t* pSnsHw) {
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)pSnsHw->mSd;
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;

    ENTER_CAMHW_FUNCTION();
    aiqMutex_lock(&pSnsHw->_mutex);

    _SensorHw_set_sync_mode(pSnsHw, NO_SYNC_MODE);
    ret = (*v4l2_dev->stop)(v4l2_dev);
    pSnsHw->userVts = 0;
    EXIT_CAMHW_FUNCTION();
    aiqMutex_unlock(&pSnsHw->_mutex);

    return ret;
}

void AiqSensorHw_clean(AiqSensorHw_t* pSnsHw)
{
    pSnsHw->_frame_sequence = -1;
    pSnsHw->_first          = true;

    while (aiqList_size(pSnsHw->_exp_list) > 0) {
        AiqListItem_t* item = aiqList_get_item(pSnsHw->_exp_list, NULL);
        AIQ_REF_BASE_UNREF(&(((SensorHwExpItem_t*)(item->_pData))->_pSnsExp)->_base._ref_base);
        aiqList_erase_item(pSnsHw->_exp_list, item);
    }
    aiqList_reset(pSnsHw->_exp_list);

    if (pSnsHw->_last_exp_time) {
        AIQ_REF_BASE_UNREF(&pSnsHw->_last_exp_time->_base._ref_base);
        pSnsHw->_last_exp_time = NULL;
    }
    if (pSnsHw->_last_exp_gain) {
        AIQ_REF_BASE_UNREF(&pSnsHw->_last_exp_gain->_base._ref_base);
        pSnsHw->_last_exp_gain = NULL;
    }
    if (pSnsHw->_last_dcg_gain_mode) {
        AIQ_REF_BASE_UNREF(&pSnsHw->_last_dcg_gain_mode->_base._ref_base);
        pSnsHw->_last_dcg_gain_mode = NULL;
    }

    while (aiqMap_size(pSnsHw->_effecting_exp_map) > 0) {
        AiqMapItem_t* pItem = aiqMap_begin(pSnsHw->_effecting_exp_map);
        if (pItem) {
            AIQ_REF_BASE_UNREF(&(*((AiqSensorExpInfo_t**)(pItem->_pData)))->_base._ref_base);
            aiqMap_erase(pSnsHw->_effecting_exp_map, pItem->_key);
        }
    }
    aiqMap_reset(pSnsHw->_effecting_exp_map);

    while (aiqList_size(pSnsHw->_delayed_gain_list) > 0) {
        AiqListItem_t* item = aiqList_get_item(pSnsHw->_delayed_gain_list, NULL);
        AIQ_REF_BASE_UNREF(&(*((AiqSensorExpInfo_t**)(item->_pData)))->_base._ref_base);
        aiqList_erase_item(pSnsHw->_delayed_gain_list, item);
    }
    aiqList_reset(pSnsHw->_delayed_gain_list);

    while (aiqList_size(pSnsHw->_delayed_dcg_gain_mode_list) > 0) {
        AiqListItem_t* item = aiqList_get_item(pSnsHw->_delayed_dcg_gain_mode_list, NULL);
        AIQ_REF_BASE_UNREF(&(*((AiqSensorExpInfo_t**)(item->_pData)))->_base._ref_base);
        aiqList_erase_item(pSnsHw->_delayed_dcg_gain_mode_list, item);
    }
    aiqList_reset(pSnsHw->_delayed_dcg_gain_mode_list);

    while (aiqMap_size(pSnsHw->_pending_spilt_map) > 0) {
        AiqMapItem_t* pItem = aiqMap_begin(pSnsHw->_pending_spilt_map);
        if (pItem) {
            aiqMap_erase(pSnsHw->_effecting_exp_map, pItem->_key);
        }
    }
    aiqMap_reset(pSnsHw->_pending_spilt_map);

    pSnsHw->_frame_sequence = -1;
    pSnsHw->_first          = true;
}

static XCamReturn _SensorHw_open(AiqSensorHw_t* pBaseSns)
{
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)pBaseSns->mSd;
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;

    ENTER_CAMHW_FUNCTION();

    ret = (*v4l2_dev->open)(v4l2_dev, false);

    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn _SensorHw_close(AiqSensorHw_t* pBaseSns)
{
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)pBaseSns->mSd;
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;

    ENTER_CAMHW_FUNCTION();

    ret = (*v4l2_dev->close)(v4l2_dev);

    EXIT_CAMHW_FUNCTION();
	return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_set_sync_mode(AiqSensorHw_t* pSnsHw, uint32_t mode) {
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_SET_SYNC_MODE, &mode) < 0) {
        LOGW_CAMHW_SUBM(SENSOR_SUBM, "failed to set sync mode %d", mode);
        // return XCAM_RETURN_ERROR_IOCTL;
    }

    LOGI_CAMHW_SUBM(SENSOR_SUBM, "set sync mode %d", mode);

    return XCAM_RETURN_NO_ERROR;
}

static bool _SensorHw_getIsSingleMode(AiqSensorHw_t* pSnsHw) { return pSnsHw->mIsSingleMode; }

static XCamReturn _SensorHw_setEffExpMap(AiqSensorHw_t* pSns, uint32_t sequence, void* exp_ptr,
                                         int mode) {
    aiqMutex_lock(&pSns->_mutex);
    AiqMapItem_t* pItem         = NULL;
    AiqSensorExpInfo_t* pSnsExp = NULL;
    int ret                     = 0;
    rk_aiq_frame_info_t off_finfo;

    if (mode) {
        while (aiqMap_size(pSns->_effecting_exp_map) > 4) {
            pItem = aiqMap_begin(pSns->_effecting_exp_map);
            if (pItem) {
                AIQ_REF_BASE_UNREF(&(*((AiqSensorExpInfo_t**)(pItem->_pData)))->_base._ref_base);
				aiqMap_erase(pSns->_effecting_exp_map, pItem->_key);
            }
        }
        if (aiqPool_freeNums(pSns->_expParamsPool)) {
            AiqPoolItem_t* pItem = aiqPool_getFree(pSns->_expParamsPool);
            pSnsExp              = (AiqSensorExpInfo_t*)(pItem->_pData);
        } else {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: no free params buffer!\n", __FUNCTION__);
            aiqMutex_unlock(&pSns->_mutex);
            return XCAM_RETURN_ERROR_MEM;
        }

        memcpy(&off_finfo, exp_ptr, sizeof(off_finfo));
        pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global =
            (uint32_t)off_finfo.normal_gain_reg;
        pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time =
            (uint32_t)off_finfo.normal_exp_reg;
        pSnsExp->aecExpInfo.LinearExp.exp_real_params.analog_gain = (float)off_finfo.normal_gain;
        pSnsExp->aecExpInfo.LinearExp.exp_real_params.integration_time =
            (float)off_finfo.normal_exp;
        pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.digital_gain_global = 1;
        pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.isp_digital_gain    = 1;
        pSnsExp->aecExpInfo.LinearExp.exp_real_params.digital_gain          = 1.0f;
        pSnsExp->aecExpInfo.LinearExp.exp_real_params.isp_dgain             = 1.0f;

        pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global =
            (uint32_t)off_finfo.hdr_gain_l;
        pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time =
            (uint32_t)off_finfo.hdr_exp_l;
        pSnsExp->aecExpInfo.HdrExp[2].exp_real_params.analog_gain = (float)off_finfo.hdr_gain_l_reg;
        pSnsExp->aecExpInfo.HdrExp[2].exp_real_params.integration_time =
            (float)off_finfo.hdr_exp_l_reg;
        pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.digital_gain_global = 1;
        pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.isp_digital_gain    = 1;
        pSnsExp->aecExpInfo.HdrExp[2].exp_real_params.digital_gain          = 1.0f;
        pSnsExp->aecExpInfo.HdrExp[2].exp_real_params.isp_dgain             = 1.0f;

        pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global =
            (uint32_t)off_finfo.hdr_gain_m;
        pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time =
            (uint32_t)off_finfo.hdr_exp_m;
        pSnsExp->aecExpInfo.HdrExp[1].exp_real_params.analog_gain = (float)off_finfo.hdr_gain_m_reg;
        pSnsExp->aecExpInfo.HdrExp[1].exp_real_params.integration_time =
            (float)off_finfo.hdr_exp_m_reg;
        pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.digital_gain_global = 1;
        pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.isp_digital_gain    = 1;
        pSnsExp->aecExpInfo.HdrExp[1].exp_real_params.digital_gain          = 1.0f;
        pSnsExp->aecExpInfo.HdrExp[1].exp_real_params.isp_dgain             = 1.0f;

        pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global =
            (uint32_t)off_finfo.hdr_gain_s;
        pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time =
            (uint32_t)off_finfo.hdr_exp_s;
        pSnsExp->aecExpInfo.HdrExp[0].exp_real_params.analog_gain = (float)off_finfo.hdr_gain_s_reg;
        pSnsExp->aecExpInfo.HdrExp[0].exp_real_params.integration_time =
            (float)off_finfo.hdr_exp_s_reg;
        pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.digital_gain_global = 1;
        pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.isp_digital_gain    = 1;
        pSnsExp->aecExpInfo.HdrExp[0].exp_real_params.digital_gain          = 1.0f;
        pSnsExp->aecExpInfo.HdrExp[0].exp_real_params.isp_dgain             = 1.0f;

        pSnsExp->_base.frame_id = sequence;
        pItem = aiqMap_insert(pSns->_effecting_exp_map, (void*)(intptr_t)sequence, &pSnsExp);
        if (!pItem) {
            AIQ_REF_BASE_UNREF(&pSnsExp->_base._ref_base);
            aiqMutex_unlock(&pSns->_mutex);
            return XCAM_RETURN_ERROR_FAILED;
        }
    } else {
        rk_aiq_exposure_params_t* senosrExp = (rk_aiq_exposure_params_t*)exp_ptr;

        while (aiqMap_size(pSns->_effecting_exp_map) > 0) {
            pItem = aiqMap_begin(pSns->_effecting_exp_map);
            if (pItem) {
                AIQ_REF_BASE_UNREF(&(*((AiqSensorExpInfo_t**)(pItem->_pData)))->_base._ref_base);
                aiqMap_erase(pSns->_effecting_exp_map, pItem->_key);
            }
        }

        while (aiqList_size(pSns->_exp_list) > 0) {
            AiqListItem_t* item = aiqList_get_item(pSns->_exp_list, NULL);
            AIQ_REF_BASE_UNREF(&(((SensorHwExpItem_t*)(item->_pData))->_pSnsExp)->_base._ref_base);
            aiqList_erase_item(pSns->_exp_list, item);
        }

        while (aiqMap_size(pSns->_pending_spilt_map) > 0) {
            pItem = aiqMap_begin(pSns->_pending_spilt_map);
			aiqMap_erase(pSns->_effecting_exp_map, pItem->_key);
        }

        if (aiqPool_freeNums(pSns->_expParamsPool)) {
            AiqPoolItem_t* pItem = aiqPool_getFree(pSns->_expParamsPool);
            pSnsExp              = (AiqSensorExpInfo_t*)(pItem->_pData);
        } else {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: no free params buffer!\n", __FUNCTION__);
            aiqMutex_unlock(&pSns->_mutex);
            return XCAM_RETURN_ERROR_MEM;
        }

        memcpy(&pSnsExp->aecExpInfo, senosrExp, sizeof(rk_aiq_exposure_params_t));
        pSnsExp->_base.frame_id = sequence;
        pItem = aiqMap_insert(pSns->_effecting_exp_map, (void*)(intptr_t)sequence, &pSnsExp);
        if (pItem) {
            AIQ_REF_BASE_UNREF(&pSnsExp->_base._ref_base);
            aiqMutex_unlock(&pSns->_mutex);
            return XCAM_RETURN_ERROR_FAILED;
        }
    }
    aiqMutex_unlock(&pSns->_mutex);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_setPauseFlag(AiqSensorHw_t* pSnsHw, bool mode, uint32_t frameId,
                                         bool isSingleMode) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    uint32_t new_exp_id, last_exp_id;
    RKAiqAecExpInfo_t* ptr_new_exp = NULL;
    AiqMapItem_t* pItem            = NULL;
    pSnsHw->mPauseFlag             = mode;

    aiqMutex_lock(&pSnsHw->_mutex);
    if (pSnsHw->mPauseFlag && pSnsHw->_time_delay > 1) {
        pSnsHw->mPauseId = frameId;
        pSnsHw->mIsSingleMode = isSingleMode;
        new_exp_id       = frameId + pSnsHw->_time_delay;
        pItem            = aiqMap_get(pSnsHw->_effecting_exp_map, (void*)(intptr_t)new_exp_id);
        if (pItem) {
            AIQ_REF_BASE_UNREF(&(*((AiqSensorExpInfo_t**)(pItem->_pData)))->_base._ref_base);
            aiqMap_erase(pSnsHw->_effecting_exp_map, pItem->_key);
            pItem = aiqMap_rbegin(pSnsHw->_effecting_exp_map);
            if (pItem) {
                AiqSensorExpInfo_t* pSnsExp = *((AiqSensorExpInfo_t**)(pItem->_pData));
                ptr_new_exp                 = &pSnsExp->aecExpInfo;
                if (pSnsHw->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                    ret = _SensorHw_setLinearSensorExposure(pSnsHw, ptr_new_exp);
                } else {
                    ret = _SensorHw_setHdrSensorExposure(pSnsHw, ptr_new_exp);
                }
                LOGD_CAMHW("erase effect exp id %u, set new exp id is %u", new_exp_id,
                           (uint32_t)(long)(pItem->_key));
            }
        }
        LOGD_CAMHW_SUBM(SENSOR_SUBM,
                        "switch to %s mode, pauseId %u, handle sof id %u, _time_delay %d",
                        pSnsHw->mIsSingleMode ? "single" : "multi", pSnsHw->mPauseId,
                        pSnsHw->_frame_sequence, pSnsHw->_time_delay);
    }
    aiqMutex_unlock(&pSnsHw->_mutex);
    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_DUMPSYS
static int _SensorHw_dump(void* dumper, st_string* result, int argc, void* argv[]) {
    sensor_dump_mod_param((AiqSensorHw_t*)dumper, result);
    sensor_dump_dev_attr1((AiqSensorHw_t*)dumper, result);
    sensor_dump_dev_attr2((AiqSensorHw_t*)dumper, result);
    sensor_dump_reg_effect_delay((AiqSensorHw_t*)dumper, result);
    sensor_dump_exp_list_size((AiqSensorHw_t*)dumper, result);
    sensor_dump_configured_exp((AiqSensorHw_t*)dumper, result);
    return 0;
}
#endif

/*
 *    S E T    S C E N E    C I S    P A R A M S
 */

static XCamReturn _SensorHw_set_regSetting(AiqSensorHw_t* pSnsHw,
                                           calibdb_cis_reg_setting_t* regSetting) {
    struct rkmodule_reg_setting reg_setting;
    reg_setting.setting_id   = regSetting->hw_cisCfg_setting_id;
    reg_setting.binning_mode = regSetting->hw_cisCfg_ds_mode;
    if (regSetting->hw_cisCfg_ds_mode == cis_bayerBinning2X2_mode) {
        reg_setting.binning_mode = BAYER_BINNING_2X2;
    } else if (regSetting->hw_cisCfg_ds_mode == cis_bayerSkipN_mode) {
        reg_setting.binning_mode = BAYER_SKIP_2X2;
    } else if (regSetting->hw_cisCfg_ds_mode == cis_qbcBinning2X2_mode) {
        reg_setting.binning_mode = QBC_BINNING_2X2;
    } else {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "Invalid binning mode %d, set BAYER_BINNING_2X2 mode",
                        regSetting->hw_cisCfg_ds_mode);
        reg_setting.binning_mode = BAYER_BINNING_2X2;
    }
    reg_setting.reg_num = regSetting->regCtrl_num;
    for (uint32_t i = 0; i < reg_setting.reg_num; i++) {
        reg_setting.reg_list[i].reg_addr = regSetting->regCtrl[i].regAddr;
        reg_setting.reg_list[i].reg_val  = regSetting->regCtrl[i].regVal;
    }

    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_SET_REG_SETTING, &reg_setting) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "Failed to set reg setting for CIS");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "Set reg setting for CIS");
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_set_blc(AiqSensorHw_t* pSnsHw, calibdb_cis_blc_t* blc) {
    memcpy(&pSnsHw->cis_blc, blc, sizeof(*blc));

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_set_hdr(AiqSensorHw_t* pSnsHw, calibdb_cis_hdr_t* hdr) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (hdr->sta.regCtrl_num) {
        struct rkmodule_reg_group reg_setting;
        __u32 preg_addr[256] = {0}, preg_value[256] = {0}, preg_addr_bytes[256] = {0},
              preg_value_bytes[256] = {0};

        for (int i = 0; i < hdr->sta.regCtrl_num; i++) {
            preg_addr[i]        = hdr->sta.regCtrl[i].regAddr;
            preg_value[i]       = hdr->sta.regCtrl[i].regVal;
            preg_addr_bytes[i]  = 2;
            preg_value_bytes[i] = 1;
        }

        reg_setting.type                       = RKMODULE_REG_GROUP_MERGE;
        reg_setting.reg_group.num_regs         = (__u64)(hdr->sta.regCtrl_num);
        reg_setting.reg_group.preg_addr        = (__u64)(unsigned long)(preg_addr);
        reg_setting.reg_group.preg_value       = (__u64)(unsigned long)(preg_value);
        reg_setting.reg_group.preg_addr_bytes  = (__u64)(unsigned long)(preg_addr_bytes);
        reg_setting.reg_group.preg_value_bytes = (__u64)(unsigned long)(preg_value_bytes);

        LOGD_CAMHW_SUBM(SENSOR_SUBM,
                        " hdr->sta.regCtrl_num:%d regCtrl[0].regAddr:0x%x regCtrl[0].regVal:0x%x",
                        hdr->sta.regCtrl_num, preg_addr[0], preg_value[0]);

        if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_SET_REGISTER_GROUP, &reg_setting) < 0) {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "Failed to set reg setting for CIS hdr");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    return ret;
}

static XCamReturn _SensorHw_set_qbcRmsc(AiqSensorHw_t* pSnsHw, calibdb_cis_qbc_rmsc_t* qbcRmsc) {
    struct rkmodule_bayer_param qbc_rmsc;

    qbc_rmsc.bayer_mode = qbcRmsc->en;
    qbc_rmsc.reg_num    = qbcRmsc->sta.regCtrl_num;
    for (uint32_t i = 0; i < qbc_rmsc.reg_num; i++) {
        qbc_rmsc.reg_list[i].reg_addr = qbcRmsc->sta.regCtrl[i].regAddr;
        qbc_rmsc.reg_list[i].reg_val  = qbcRmsc->sta.regCtrl[i].regVal;
    }

    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_SET_BAYER_MODE, &qbc_rmsc) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "Failed to set qbc_rmsc for CIS");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "Set reg setting for CIS");
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_set_cmpsOut(AiqSensorHw_t* pSnsHw, calibdb_cis_cmps_out_t* cmpsOut) {
    if (!cmpsOut->en) {
        LOGD_CAMHW_SUBM(SENSOR_SUBM, "cis cmpsOut is not enabled, skip setting");
        return XCAM_RETURN_BYPASS;
    }

    uint32_t cmps_mode = 0;

    if (cmpsOut->sta.hw_cisCfgCmps_mode == cis_cmpsLowBW_mode) {
        cmps_mode = CMPS_LOW_BIT_WIDTH_MODE;
    } else if (cmpsOut->sta.hw_cisCfgCmps_mode == cis_cmpsHighBW_mode)
        cmps_mode = CMPS_HIGH_BIT_WIDTH_MODE;
    else {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "Invalid cmpsOut mode %d", cmpsOut->sta.hw_cisCfgCmps_mode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_SET_CMPS_MODE, &cmps_mode) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "Failed to set qbc_rmsc for CIS");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "Set cmps mode %d for CIS", cmps_mode);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqSensorHw_getHdrComprCurve(AiqSensorHw_t* pSnsHw, RkAiqHdrCompr_t* compr) {
    if (!compr) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "compr is NULL");
        return XCAM_RETURN_ERROR_PARAM;
    }

    struct rkmodule_hdr_cfg hdr_cfg = {0};

    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_GET_HDR_CFG, &hdr_cfg) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to get hdr compr curve");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    compr->point   = hdr_cfg.compr.point;
    compr->src_bit = hdr_cfg.compr.src_bit;
    compr->k_shift = hdr_cfg.compr.k_shift;
    memcpy(compr->data_compr, hdr_cfg.compr.data_compr, sizeof(compr->data_compr));
    memcpy(compr->data_src, hdr_cfg.compr.data_src, sizeof(compr->data_src));
    memcpy(compr->slope_k, hdr_cfg.compr.slope_k, sizeof(compr->slope_k));

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "HDR compr: point %d, src_bit %d, k_shift %d", compr->point,
                    compr->src_bit, compr->k_shift);

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "HDR data compr: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                    compr->data_compr[0], compr->data_compr[1], compr->data_compr[2],
                    compr->data_compr[3], compr->data_compr[4], compr->data_compr[5],
                    compr->data_compr[6], compr->data_compr[7], compr->data_compr[8],
                    compr->data_compr[9]);

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "HDR data src: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d", compr->data_src[0],
                    compr->data_src[1], compr->data_src[2], compr->data_src[3], compr->data_src[4],
                    compr->data_src[5], compr->data_src[6], compr->data_src[7], compr->data_src[8],
                    compr->data_src[9]);

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "HDR  slope_k: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d", compr->slope_k[0],
                    compr->slope_k[1], compr->slope_k[2], compr->slope_k[3], compr->slope_k[4],
                    compr->slope_k[5], compr->slope_k[6], compr->slope_k[7], compr->slope_k[8],
                    compr->slope_k[9]);

    return XCAM_RETURN_NO_ERROR;
}

static void _SensorHw_setVicapFd(AiqSensorHw_t* pSnsHw, int fd) {
    pSnsHw->mVicapExpFd = fd;
}

int AiqSensorHwsendMergeWgtCurve2NR(struct rkmodule_mge_oeWgt* mge_oe_wgt,
                                    mergeLuma2Wgt_t* pMergeLuma2Wgt) {
#if MGE_WGT_USE_OLD_STRUCT
    for (int j = 0; j < 17; j++) {
        pMergeLuma2Wgt->luma_idx[j] = mge_oe_wgt->wgtCurve[0].idx[j];
        pMergeLuma2Wgt->luma_wgt[j] = (float)mge_oe_wgt->wgtCurve[0].val[j] / 1024.0f;
    }
    LOGD_CAMHW_SUBM(
        SENSOR_SUBM, " MS_wgt_idx: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
        pMergeLuma2Wgt->luma_idx[0], pMergeLuma2Wgt->luma_idx[1], pMergeLuma2Wgt->luma_idx[2],
        pMergeLuma2Wgt->luma_idx[3], pMergeLuma2Wgt->luma_idx[4], pMergeLuma2Wgt->luma_idx[5],
        pMergeLuma2Wgt->luma_idx[6], pMergeLuma2Wgt->luma_idx[7], pMergeLuma2Wgt->luma_idx[8],
        pMergeLuma2Wgt->luma_idx[9], pMergeLuma2Wgt->luma_idx[10], pMergeLuma2Wgt->luma_idx[11],
        pMergeLuma2Wgt->luma_idx[12], pMergeLuma2Wgt->luma_idx[13], pMergeLuma2Wgt->luma_idx[14],
        pMergeLuma2Wgt->luma_idx[15], pMergeLuma2Wgt->luma_idx[16]);
#else
    pMergeLuma2Wgt->wgtCurve_num = mge_oe_wgt->wgtCurve_num;
    for (int i = 0; i < mge_oe_wgt->wgtCurve_num; i++) {
        for (int j = 0; j < 17; j++) {
            pMergeLuma2Wgt->wgtCurve[i].idx[j] = mge_oe_wgt->wgtCurve[i].idx[j];
            pMergeLuma2Wgt->wgtCurve[i].val[j] = (float)mge_oe_wgt->wgtCurve[i].val[j] / 1024.0f;
        }
    }

    LOGD_CAMHW_SUBM(SENSOR_SUBM, " wgtCurve_num %d", pMergeLuma2Wgt->wgtCurve_num);
    LOGD_CAMHW_SUBM(SENSOR_SUBM, " MS_wgt_idx: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                    pMergeLuma2Wgt->wgtCurve[0].idx[0], pMergeLuma2Wgt->wgtCurve[0].idx[1],
                    pMergeLuma2Wgt->wgtCurve[0].idx[2], pMergeLuma2Wgt->wgtCurve[0].idx[3],
                    pMergeLuma2Wgt->wgtCurve[0].idx[4], pMergeLuma2Wgt->wgtCurve[0].idx[5],
                    pMergeLuma2Wgt->wgtCurve[0].idx[6], pMergeLuma2Wgt->wgtCurve[0].idx[7],
                    pMergeLuma2Wgt->wgtCurve[0].idx[8], pMergeLuma2Wgt->wgtCurve[0].idx[9],
                    pMergeLuma2Wgt->wgtCurve[0].idx[10], pMergeLuma2Wgt->wgtCurve[0].idx[11],
                    pMergeLuma2Wgt->wgtCurve[0].idx[12], pMergeLuma2Wgt->wgtCurve[0].idx[13],
                    pMergeLuma2Wgt->wgtCurve[0].idx[14], pMergeLuma2Wgt->wgtCurve[0].idx[15],
                    pMergeLuma2Wgt->wgtCurve[0].idx[16]);
    LOGD_CAMHW_SUBM(SENSOR_SUBM, " MS_wgt_val: %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
                    pMergeLuma2Wgt->wgtCurve[0].val[0], pMergeLuma2Wgt->wgtCurve[0].val[1],
                    pMergeLuma2Wgt->wgtCurve[0].val[2], pMergeLuma2Wgt->wgtCurve[0].val[3],
                    pMergeLuma2Wgt->wgtCurve[0].val[4], pMergeLuma2Wgt->wgtCurve[0].val[5],
                    pMergeLuma2Wgt->wgtCurve[0].val[6], pMergeLuma2Wgt->wgtCurve[0].val[7],
                    pMergeLuma2Wgt->wgtCurve[0].val[8], pMergeLuma2Wgt->wgtCurve[0].val[9],
                    pMergeLuma2Wgt->wgtCurve[0].val[10], pMergeLuma2Wgt->wgtCurve[0].val[11],
                    pMergeLuma2Wgt->wgtCurve[0].val[12], pMergeLuma2Wgt->wgtCurve[0].val[13],
                    pMergeLuma2Wgt->wgtCurve[0].val[14], pMergeLuma2Wgt->wgtCurve[0].val[15],
                    pMergeLuma2Wgt->wgtCurve[0].val[16]);
#endif
    return 0;
}

int AiqSensorHw_getMergeWgtCurve(AiqSensorHw_t* pSnsHw, struct rkmodule_mge_oeWgt* mge_oe_wgt) {
    if (!mge_oe_wgt) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "mge_oe_wgt is NULL");
        return -1;
    }

    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_GET_MERGE_WGT_CURVE, mge_oe_wgt) < 0) {
        //LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to get merge wgt curve");
        return -1;
    }

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "wgtCurve_num: %d", mge_oe_wgt->wgtCurve_num);
    for (int i = 0; i < mge_oe_wgt->wgtCurve_num; i++) {
        LOGD_CAMHW_SUBM(SENSOR_SUBM,
                        "wgtCurve_idx[%d]: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", i,
                        mge_oe_wgt->wgtCurve[i].idx[0], mge_oe_wgt->wgtCurve[i].idx[1],
                        mge_oe_wgt->wgtCurve[i].idx[2], mge_oe_wgt->wgtCurve[i].idx[3],
                        mge_oe_wgt->wgtCurve[i].idx[4], mge_oe_wgt->wgtCurve[i].idx[5],
                        mge_oe_wgt->wgtCurve[i].idx[6], mge_oe_wgt->wgtCurve[i].idx[7],
                        mge_oe_wgt->wgtCurve[i].idx[8], mge_oe_wgt->wgtCurve[i].idx[9],
                        mge_oe_wgt->wgtCurve[i].idx[10], mge_oe_wgt->wgtCurve[i].idx[11],
                        mge_oe_wgt->wgtCurve[i].idx[12], mge_oe_wgt->wgtCurve[i].idx[13],
                        mge_oe_wgt->wgtCurve[i].idx[14], mge_oe_wgt->wgtCurve[i].idx[15],
                        mge_oe_wgt->wgtCurve[i].idx[16]);
        LOGD_CAMHW_SUBM(SENSOR_SUBM,
                        "wgtCurve_val[%d]: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", i,
                        mge_oe_wgt->wgtCurve[i].val[0], mge_oe_wgt->wgtCurve[i].val[1],
                        mge_oe_wgt->wgtCurve[i].val[2], mge_oe_wgt->wgtCurve[i].val[3],
                        mge_oe_wgt->wgtCurve[i].val[4], mge_oe_wgt->wgtCurve[i].val[5],
                        mge_oe_wgt->wgtCurve[i].val[6], mge_oe_wgt->wgtCurve[i].val[7],
                        mge_oe_wgt->wgtCurve[i].val[8], mge_oe_wgt->wgtCurve[i].val[9],
                        mge_oe_wgt->wgtCurve[i].val[10], mge_oe_wgt->wgtCurve[i].val[11],
                        mge_oe_wgt->wgtCurve[i].val[12], mge_oe_wgt->wgtCurve[i].val[13],
                        mge_oe_wgt->wgtCurve[i].val[14], mge_oe_wgt->wgtCurve[i].val[15],
                        mge_oe_wgt->wgtCurve[i].val[16]);
    }

    return 0;
}

int AiqSensorHw_getBuiltInHdrSingleBit(
    AiqSensorHw_t* pSnsHw, struct rkmodule_hdr_compr_single_frame_info* compr_single_info) {
    if (!compr_single_info) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "compr_single_info is NULL");
        return -1;
    }

    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_GET_HDR_COMPR_SINGLE_FRAME_INFO,
                               compr_single_info) < 0) {
        //LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to get BuiltInHdr Single Bit");
        return -1;
    }

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "single_bitwidth: %d", compr_single_info->single_bitwidth);

    return 0;
}

void AiqSensorHw_init(AiqSensorHw_t* pSnsHw, const char* name, int cid) {
    ENTER_CAMHW_FUNCTION();

    memset(pSnsHw, 0, sizeof(AiqSensorHw_t));
    pSnsHw->mSd = (AiqV4l2SubDevice_t*)aiq_malloc(sizeof(AiqV4l2SubDevice_t));
    AiqV4l2SubDevice_init(pSnsHw->mSd, name);
    pSnsHw->_working_mode   = RK_AIQ_WORKING_MODE_NORMAL;
    pSnsHw->_first          = true;
    pSnsHw->_frame_sequence = -1;
    pSnsHw->mCamPhyId       = cid;
    pSnsHw->_mirror         = 0;
    pSnsHw->_flip           = 0;
    pSnsHw->dcg_mode        = -1;
    pSnsHw->mCisHdrMode     = 0;

    pSnsHw->mVicapExpFd     = -1;
    {
        // init pool
        AiqPoolConfig_t snsExpPoolCfg;
        AiqPoolItem_t* pItem     = NULL;
        int i                    = 0;
        snsExpPoolCfg._name      = "SensorLocalExpParams";
        snsExpPoolCfg._item_nums = SENSORHW_DEFAULT_POOL_SIZE;
        snsExpPoolCfg._item_size = sizeof(AiqSensorExpInfo_t);
        pSnsHw->_expParamsPool   = aiqPool_init(&snsExpPoolCfg);
        if (!pSnsHw->_expParamsPool)
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "cId:%d init %s error", cid, snsExpPoolCfg._name);
        for (i = 0; i < pSnsHw->_expParamsPool->_item_nums; i++) {
            pItem = &pSnsHw->_expParamsPool->_item_array[i];
            AIQ_REF_BASE_INIT(&((AiqSensorExpInfo_t*)(pItem->_pData))->_base._ref_base, pItem,
                              aiqPoolItem_ref, aiqPoolItem_unref);
        }
    }
    {
        // init list
        AiqListConfig_t snsExpListCfg;
        snsExpListCfg._name      = "snsExpPendingList";
        snsExpListCfg._item_nums = SENSORHW_DEFAULT_POOL_SIZE;
        snsExpListCfg._item_size = sizeof(SensorHwExpItem_t);
        pSnsHw->_exp_list        = aiqList_init(&snsExpListCfg);
        if (!pSnsHw->_exp_list)
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "cId:%d init %s error", cid, snsExpListCfg._name);

        snsExpListCfg._name        = "snsDelayGainList";
        snsExpListCfg._item_nums   = SENSORHW_DEFAULT_POOL_SIZE;
        snsExpListCfg._item_size   = sizeof(AiqSensorExpInfo_t*);
        pSnsHw->_delayed_gain_list = aiqList_init(&snsExpListCfg);
        if (!pSnsHw->_delayed_gain_list)
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "cId:%d init %s error", cid, snsExpListCfg._name);

        snsExpListCfg._name                 = "snsDelayDcgGainModeList";
        snsExpListCfg._item_nums            = SENSORHW_DEFAULT_POOL_SIZE;
        snsExpListCfg._item_size            = sizeof(AiqSensorExpInfo_t*);
        pSnsHw->_delayed_dcg_gain_mode_list = aiqList_init(&snsExpListCfg);
        if (!pSnsHw->_delayed_dcg_gain_mode_list)
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "cId:%d init %s error", cid, snsExpListCfg._name);
    }
    {
        // init map
        AiqMapConfig_t snsExpEffMapCfg;
        snsExpEffMapCfg._name      = "snsExpEffMap";

        snsExpEffMapCfg._key_type = AIQ_MAP_KEY_TYPE_UINT32;
        snsExpEffMapCfg._item_nums = SENSORHW_DEFAULT_POOL_SIZE;
        snsExpEffMapCfg._item_size = sizeof(AiqSensorExpInfo_t*);
        pSnsHw->_effecting_exp_map = aiqMap_init(&snsExpEffMapCfg);
        if (!pSnsHw->_effecting_exp_map)
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "cId:%d init %s error", cid, snsExpEffMapCfg._name);
    }
    {
        AiqMapConfig_t snsExpPendingSplitMapCfg;
        snsExpPendingSplitMapCfg._name      = "snsExpPendingSplitMap";
        snsExpPendingSplitMapCfg._item_nums = SENSORHW_DEFAULT_POOL_SIZE;
        snsExpPendingSplitMapCfg._item_size = sizeof(aiq_pending_split_exps_t);
        snsExpPendingSplitMapCfg._key_type  = AIQ_MAP_KEY_TYPE_UINT32;
        pSnsHw->_pending_spilt_map          = aiqMap_init(&snsExpPendingSplitMapCfg);
        if (!pSnsHw->_pending_spilt_map)
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "cId:%d init %s error", cid,
                            snsExpPendingSplitMapCfg._name);
    }

    aiqMutex_init(&pSnsHw->_mutex);

    pSnsHw->setExposureParams     = SensorHw_setExposureParams;
    pSnsHw->getSensorModeData     = SensorHw_getSensorModeData;
    pSnsHw->handle_sof            = SensorHw_handle_sof;
    pSnsHw->get_sensor_descriptor = SensorHw_getSensorDescriptor;
    pSnsHw->getEffectiveExpParams = SensorHw_getEffectiveExpParams;

    pSnsHw->set_working_mode      = _SensorHw_set_working_mode;
    pSnsHw->set_exp_delay_info    = _SensorHw_set_exp_delay_info;
    pSnsHw->set_mirror_flip       = _SensorHw_set_mirror_flip;
    pSnsHw->get_mirror_flip       = _SensorHw_get_mirror_flip;
    pSnsHw->start                 = _SensorHw_start;
    pSnsHw->stop                  = _SensorHw_stop;
    pSnsHw->close                 = _SensorHw_close;
    pSnsHw->open			 	  = _SensorHw_open;
    pSnsHw->set_sync_mode         = _SensorHw_set_sync_mode;
    pSnsHw->get_is_single_mode    = _SensorHw_getIsSingleMode;
    pSnsHw->set_effecting_exp_map = _SensorHw_setEffExpMap;
    pSnsHw->set_pause_flag        = _SensorHw_setPauseFlag;
#if RKAIQ_HAVE_DUMPSYS
    pSnsHw->dump                  = _SensorHw_dump;
#endif
    pSnsHw->set_exposure_mode     = _SensorHw_setExpMode;

    // set scene_cis parameters
    pSnsHw->set_regSetting = _SensorHw_set_regSetting;
    pSnsHw->set_blc        = _SensorHw_set_blc;
    pSnsHw->set_hdr        = _SensorHw_set_hdr;
    pSnsHw->set_qbcRmsc    = _SensorHw_set_qbcRmsc;
    pSnsHw->set_cmpsOut    = _SensorHw_set_cmpsOut;

    pSnsHw->setVicapExpFd  = _SensorHw_setVicapFd;

    EXIT_CAMHW_FUNCTION();
}

void AiqSensorHw_deinit(AiqSensorHw_t* pSnsHw) {
    ENTER_CAMHW_FUNCTION();
    aiqMap_deinit(pSnsHw->_pending_spilt_map);
    aiqMap_deinit(pSnsHw->_effecting_exp_map);
    aiqList_deinit(pSnsHw->_exp_list);
    aiqList_deinit(pSnsHw->_delayed_gain_list);
    aiqList_deinit(pSnsHw->_delayed_dcg_gain_mode_list);
    aiqPool_deinit(pSnsHw->_expParamsPool);
    if (pSnsHw->mSd) {
        AiqV4l2SubDevice_deinit(pSnsHw->mSd);
        aiq_free(pSnsHw->mSd);
    }
    aiqMutex_deInit(&pSnsHw->_mutex);
    EXIT_CAMHW_FUNCTION();
}
