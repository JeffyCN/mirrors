/*
 * aiq_handler.cpp - AIQ handler
 *
 *  Copyright (c) 2012-2015 Intel Corporation
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
 * Author: Wind Yuan <feng.yuan@intel.com>
 * Author: Yan Zhang <yan.y.zhang@intel.com>
 */

#include "rkiq_handler.h"
#include "rk_params_translate.h"
#include "x3a_isp_config.h"
#include <interface/rkcamera_vendor_tags.h>

#include <string.h>
#include <math.h>

#define MAX_STATISTICS_WIDTH 150
#define MAX_STATISTICS_HEIGHT 150

//#define USE_RGBS_GRID_WEIGHTING
#define USE_HIST_GRID_WEIGHTING

namespace XCam {

struct IspInputParameters {

    IspInputParameters ()
    {}
};

static double
_calculate_new_value_by_speed (double start, double end, double speed)
{
    XCAM_ASSERT (speed >= 0.0 && speed <= 1.0);
    static const double value_equal_range = 0.000001;

    if (fabs (end - start) <= value_equal_range)
        return end;
    return (start * (1.0 - speed) + end * speed);
}

static double
_imx185_sensor_gain_code_to_mutiplier (uint32_t code)
{
    /* 185 sensor code : DB = 160 : 48 */
    double db;
    db = code * 48.0 / 160.0;
    return pow (10.0, db / 20.0);
}

static uint32_t
_mutiplier_to_imx185_sensor_gain_code (double mutiplier)
{
    double db = log10 (mutiplier) * 20;
    if (db > 48)
        db = 48;
    return (uint32_t) (db * 160 / 48);
}

static uint32_t
_time_to_coarse_line (const ia_aiq_exposure_sensor_descriptor *desc, uint32_t time_us)
{
    float value =  time_us * desc->pixel_clock_freq_mhz;

    value = (value + desc->pixel_periods_per_line / 2) / desc->pixel_periods_per_line;
    return (uint32_t)(value);
}

static uint32_t
_coarse_line_to_time (const ia_aiq_exposure_sensor_descriptor *desc, uint32_t coarse_line)
{
    return coarse_line * desc->pixel_periods_per_line / desc->pixel_clock_freq_mhz;
}

AiqAeHandler::AiqAeResult::AiqAeResult()
{
    xcam_mem_clear (ae_result);
    xcam_mem_clear (ae_exp_ret);
    xcam_mem_clear (aiq_exp_param);
    xcam_mem_clear (sensor_exp_param);
    xcam_mem_clear (weight_grid);
    xcam_mem_clear (flash_param);
}

void
AiqAeHandler::AiqAeResult::copy (ia_aiq_ae_results *result)
{
    XCAM_ASSERT (result);

    this->ae_result = *result;
    this->aiq_exp_param = *result->exposures[0].exposure;
    this->sensor_exp_param = *result->exposures[0].sensor_exposure;
    this->weight_grid = *result->weight_grid;

    this->ae_exp_ret.exposure = &this->aiq_exp_param;
    this->ae_exp_ret.sensor_exposure = &this->sensor_exp_param;
    this->ae_result.exposures = &this->ae_exp_ret;
    this->ae_result.weight_grid = &this->weight_grid;

    this->ae_result.num_exposures = 1;
}

AiqAeHandler::AiqAeHandler (X3aAnalyzerRKiq *analyzer, SmartPtr<RKiqCompositor> &aiq_compositor)
    : _aiq_compositor (aiq_compositor)
    , _analyzer (analyzer)
    , _started (false)
{
    xcam_mem_clear (_ia_ae_window);
    xcam_mem_clear (_sensor_descriptor);
    xcam_mem_clear (_manual_limits);
    xcam_mem_clear (_input);
    mAeState = new RkAEStateMachine();
}

bool
AiqAeHandler::set_description (struct rkisp_sensor_mode_data *sensor_data)
{
    XCAM_ASSERT (sensor_data);

    _sensor_descriptor.pixel_clock_freq_mhz = sensor_data->vt_pix_clk_freq_mhz / 1000000.0f;
    _sensor_descriptor.pixel_periods_per_line = sensor_data->line_length_pck;
    _sensor_descriptor.line_periods_per_field = sensor_data->frame_length_lines;
    _sensor_descriptor.line_periods_vertical_blanking = sensor_data->frame_length_lines
            - (sensor_data->crop_vertical_end - sensor_data->crop_vertical_start + 1)
            / sensor_data->binning_factor_y;
    _sensor_descriptor.fine_integration_time_min = sensor_data->fine_integration_time_def;
    _sensor_descriptor.fine_integration_time_max_margin = sensor_data->line_length_pck - sensor_data->fine_integration_time_def;
    _sensor_descriptor.coarse_integration_time_min = sensor_data->coarse_integration_time_min;
    _sensor_descriptor.coarse_integration_time_max_margin = sensor_data->coarse_integration_time_max_margin;

    return true;
}

bool
AiqAeHandler::ensure_ia_parameters ()
{
    bool ret = true;
    return ret;
}

bool AiqAeHandler::ensure_ae_mode ()
{
    return true;
}
bool AiqAeHandler::ensure_ae_metering_mode ()
{
    return true;
}

bool AiqAeHandler::ensure_ae_priority_mode ()
{
    return true;
}

bool AiqAeHandler::ensure_ae_flicker_mode ()
{
    return true;
}

bool AiqAeHandler::ensure_ae_manual ()
{
    return true;
}

bool AiqAeHandler::ensure_ae_ev_shift ()
{
    return true;
}

SmartPtr<X3aResult>
AiqAeHandler::pop_result ()
{
    X3aIspExposureResult *result = new X3aIspExposureResult(XCAM_IMAGE_PROCESS_ONCE);
    struct rkisp_exposure sensor;
    XCam3aResultExposure exposure;

    xcam_mem_clear (sensor);
    sensor.coarse_integration_time = _result.regIntegrationTime;
    sensor.analog_gain = _result.regGain;
    sensor.digital_gain = 0;
    sensor.frame_line_length = (uint32_t)(_result.LinePeriodsPerField + 0.5);
    sensor.IsHdrExp = _result.IsHdrExp;
    sensor.NormalExpRatio = _result.NormalExpRatio;
    sensor.LongExpRatio = _result.LongExpRatio;
    for (int i = 0; i < 3; i++) {
        sensor.RegHdrGains[i] = _result.RegHdrGains[i];
        sensor.RegHdrTime[i] = _result.RegHdrTime[i];
        sensor.HdrGains[i] = _result.HdrGains[i];
        sensor.HdrIntTimes[i] = _result.HdrIntTimes[i];
    }
    for (int i = 0; i < 3; i++) {
        sensor.RegSmoothGains[i] = _result.exp_smooth_results[i].regGain;
        sensor.RegSmoothTime[i] = _result.exp_smooth_results[i].regIntegrationTime;
        sensor.SmoothGains[i] = _result.exp_smooth_results[i].analog_gain_code_global;
        sensor.SmoothIntTimes[i] = _result.exp_smooth_results[i].coarse_integration_time;
        sensor.RegSmoothFll[i] = _result.exp_smooth_results[i].LinePeriodsPerField;
    }
    result->set_isp_config (sensor);

    xcam_mem_clear (exposure);
    exposure.exposure_time = _result.coarse_integration_time * 1000000;
    exposure.analog_gain = _result.analog_gain_code_global;
    exposure.digital_gain = 1.0f;
    exposure.aperture = _result.aperture_fn;
    result->set_standard_result (exposure);

#if 0
    XCAM_LOG_INFO ("AiqAeHandler, time-gain=[%d-%d]",
        _result.regIntegrationTime,
        _result.regGain);
#endif
    return result;
}

XCamReturn
AiqAeHandler::processAeMetaResults(AecResult_t aec_results, X3aResultList &output)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    camera_metadata_entry entry;
    SmartPtr<AiqInputParams> inputParams = _aiq_compositor->getAiqInputParams();
    SmartPtr<XmetaResult> res;
    bool is_first_param = false;

    for (X3aResultList::iterator iter = output.begin ();
            iter != output.end (); iter++)
    {
        is_first_param = (*iter)->is_first_params ();
        if ((*iter)->get_type() == XCAM_3A_METADATA_RESULT_TYPE) {
            res = (*iter).dynamic_cast_ptr<XmetaResult> ();
            break ;
        }

    }

    if (!res.ptr()) {
        res = new XmetaResult(XCAM_IMAGE_PROCESS_ONCE);
        XCAM_ASSERT (res.ptr());
        output.push_back(res);
    }
    CameraMetadata* metadata = res->get_metadata_result();

    XCamAeParam &aeParams = inputParams->aeInputParams.aeParams;
    uint8_t sceneFlickerMode = ANDROID_STATISTICS_SCENE_FLICKER_NONE;
    switch (aeParams.flicker_mode) {
    case XCAM_AE_FLICKER_MODE_50HZ:
        sceneFlickerMode = ANDROID_STATISTICS_SCENE_FLICKER_50HZ;
        break;
    case XCAM_AE_FLICKER_MODE_60HZ:
        sceneFlickerMode = ANDROID_STATISTICS_SCENE_FLICKER_60HZ;
        break;
    default:
        sceneFlickerMode = ANDROID_STATISTICS_SCENE_FLICKER_NONE;
    }
    //# ANDROID_METADATA_Dynamic android.statistics.sceneFlicker done
    metadata->update(ANDROID_STATISTICS_SCENE_FLICKER,
                                    &sceneFlickerMode, 1);

    struct CamIA10_SensorModeData &sensor_desc = _aiq_compositor->get_sensor_mode_data();
    ParamsTranslate::convert_from_rkisp_aec_result(&_rkaiq_result, &aec_results, &sensor_desc);
    /* exposure in sensor_desc is the actual effective, and the one in
     * aec_results is the latest result calculated from 3a stats and
     * will be effective in future
     */
    if (!is_first_param) {
        _rkaiq_result.exposure.exposure_time_us = sensor_desc.exp_time_seconds * 1000 * 1000;
        _rkaiq_result.exposure.analog_gain = sensor_desc.gains;
    }

    LOGD("%s exp_time=%d gain=%f, is_first_parms %d", __FUNCTION__,
            _rkaiq_result.exposure.exposure_time_us,
            _rkaiq_result.exposure.analog_gain,
            is_first_param);

    ret = mAeState->processResult(_rkaiq_result, *metadata,
                            inputParams->reqId);

    //# ANDROID_METADATA_Dynamic android.control.aeRegions done
    entry = inputParams->settings.find(ANDROID_CONTROL_AE_REGIONS);
    if (entry.count == 5)
        metadata->update(ANDROID_CONTROL_AE_REGIONS, inputParams->aeInputParams.aeRegion, entry.count);

    //# ANDROID_METADATA_Dynamic android.control.aeExposureCompensation done
    // TODO get step size (currently 1/3) from static metadata
    int32_t exposureCompensation =
            round((aeParams.ev_shift) * 3);

    metadata->update(ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION,
                                    &exposureCompensation,
                                    1);

    int64_t exposureTime = 0;
    uint16_t pixels_per_line = 0;
    uint16_t lines_per_frame = 0;
    int64_t manualExpTime = 1;

    // return exposure time always
    if (/*inputParams->aaaControls.ae.aeMode != ANDROID_CONTROL_AE_MODE_OFF*/1) {

        // Calculate frame duration from AE results and sensor descriptor
        pixels_per_line = _rkaiq_result.sensor_exposure.line_length_pixels;
        lines_per_frame = _rkaiq_result.sensor_exposure.frame_length_lines;

        /*
         * Android wants the frame duration in nanoseconds
         */
        int64_t frameDuration = (pixels_per_line * lines_per_frame) /
                                sensor_desc.pixel_clock_freq_mhz;
        frameDuration *= 1000;
        metadata->update(ANDROID_SENSOR_FRAME_DURATION,
                                             &frameDuration, 1);

#if 0
        /*
         * AE reports exposure in usecs but Android wants it in nsecs
         * In manual mode, use input value if delta to expResult is small.
         */
        exposureTime = _rkaiq_result.exposure.exposure_time_us / 1000;
        manualExpTime = aeParams.manual_exposure_time;

        if (exposureTime == 0 ||
            (manualExpTime > 0 &&
            fabs((float)exposureTime/manualExpTime - 1) < 0.01)) {

            if (exposureTime == 0)
                LOGW("sensor exposure time is Zero, copy input value");
            // copy input value
            exposureTime = manualExpTime;
        }
        exposureTime = exposureTime * 1000 * 1000; // to ns.
#else
        exposureTime = _rkaiq_result.exposure.exposure_time_us * 1000;
#endif
        metadata->update(ANDROID_SENSOR_EXPOSURE_TIME,
                                         &exposureTime, 1);

        int32_t ExposureGain = _rkaiq_result.exposure.analog_gain * 100;
        metadata->update(ANDROID_SENSOR_SENSITIVITY,
                                         &ExposureGain, 1);
    }

    int32_t value = ANDROID_SENSOR_TEST_PATTERN_MODE_OFF;
    CameraMetadata *settings = &inputParams->settings;
    entry = settings->find(ANDROID_SENSOR_TEST_PATTERN_MODE);
    if (entry.count == 1)
        value = entry.data.i32[0];

    metadata->update(ANDROID_SENSOR_TEST_PATTERN_MODE,
                                     &value, 1);

    // update expousre range
    int64_t exptime_range_us[2];
    entry = settings->find(ANDROID_SENSOR_INFO_EXPOSURE_TIME_RANGE);
    if (entry.count == 2) {
        exptime_range_us[0] = entry.data.i64[0];
        exptime_range_us[1] = entry.data.i64[1];
        metadata->update(ANDROID_SENSOR_INFO_EXPOSURE_TIME_RANGE, exptime_range_us, 2);
    }

    int32_t sensitivity_range[2];
    entry = settings->find(ANDROID_SENSOR_INFO_SENSITIVITY_RANGE);
    if (entry.count == 2) {
        sensitivity_range[0] = entry.data.i32[0];
        sensitivity_range[1] = entry.data.i32[1];
        metadata->update(ANDROID_SENSOR_INFO_SENSITIVITY_RANGE, sensitivity_range, 2);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
AiqAwbHandler::processAwbMetaResults(CamIA10_AWB_Result_t awb_results, X3aResultList &output)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<AiqInputParams> inputParams = _aiq_compositor->getAiqInputParams();
    SmartPtr<XmetaResult> res;
    camera_metadata_entry entry;
    LOGI("@%s %d: enter", __FUNCTION__, __LINE__);

    for (X3aResultList::iterator iter = output.begin ();
            iter != output.end (); iter++)
    {
        if ((*iter)->get_type() == XCAM_3A_METADATA_RESULT_TYPE) {
            res = (*iter).dynamic_cast_ptr<XmetaResult> ();
            break ;
        }
    }

    if (!res.ptr()) {
        res = new XmetaResult(XCAM_IMAGE_PROCESS_ONCE);
        XCAM_ASSERT (res.ptr());
        output.push_back(res);
    }

    CameraMetadata* metadata = res->get_metadata_result();
    struct CamIA10_SensorModeData &sensor_desc = _aiq_compositor->get_sensor_mode_data();
    ParamsTranslate::convert_from_rkisp_awb_result(&_rkaiq_result, &awb_results, &sensor_desc);

    ret = mAwbState->processResult(_rkaiq_result, *metadata);

    metadata->update(ANDROID_COLOR_CORRECTION_MODE,
                  &inputParams->aaaControls.awb.colorCorrectionMode,
                  1);
    metadata->update(ANDROID_COLOR_CORRECTION_ABERRATION_MODE,
                  &inputParams->aaaControls.awb.colorCorrectionAberrationMode,
                  1);
    /*
     * TODO: Consider moving this to common code in 3A class
     */
    float gains[4] = {1.0, 1.0, 1.0, 1.0};
    gains[0] = _rkaiq_result.awb_gain_cfg.awb_gains.red_gain;
    gains[1] = _rkaiq_result.awb_gain_cfg.awb_gains.green_r_gain;
    gains[2] = _rkaiq_result.awb_gain_cfg.awb_gains.green_b_gain;
    gains[3] = _rkaiq_result.awb_gain_cfg.awb_gains.blue_gain;
    metadata->update(ANDROID_COLOR_CORRECTION_GAINS, gains, 4);

    //# ANDROID_METADATA_Dynamic android.control.awbRegions done
    entry = inputParams->settings.find(ANDROID_CONTROL_AWB_REGIONS);
    if (entry.count == 5)
        metadata->update(ANDROID_CONTROL_AWB_REGIONS, inputParams->awbInputParams.awbRegion, entry.count);
    /*
     * store the results in row major order
     */
    if (mAwbState->getState() != ANDROID_CONTROL_AWB_STATE_LOCKED) {
        camera_metadata_rational_t transformMatrix[9];
        const int32_t COLOR_TRANSFORM_PRECISION = 10000;
        for (int i = 0; i < 9; i++) {
            transformMatrix[i].numerator =
                (int32_t)(_rkaiq_result.ctk_config.ctk_matrix.coeff[i] * COLOR_TRANSFORM_PRECISION);
            transformMatrix[i].denominator = COLOR_TRANSFORM_PRECISION;

        }

        metadata->update(ANDROID_COLOR_CORRECTION_TRANSFORM,
                         transformMatrix, 9);
    } else {
        entry = inputParams->settings.find(ANDROID_COLOR_CORRECTION_TRANSFORM);
        if (entry.count == 9) {
            metadata->update(ANDROID_COLOR_CORRECTION_TRANSFORM, entry.data.r, entry.count);
        }
    }
    return ret;
}

XCamReturn
AiqAfHandler::processAfMetaResults(XCam3aResultFocus af_results, X3aResultList &output)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<AiqInputParams> inputParams = _aiq_compositor->getAiqInputParams();
    SmartPtr<XmetaResult> res;
    camera_metadata_entry entry;
    LOGI("@%s %d: enter", __FUNCTION__, __LINE__);

    for (X3aResultList::iterator iter = output.begin ();
            iter != output.end (); iter++)
    {
        LOGD("get_type() %x ",(*iter)->get_type());
        if ((*iter)->get_type() == XCAM_3A_METADATA_RESULT_TYPE) {
            res = (*iter).dynamic_cast_ptr<XmetaResult> ();
            break ;
        }
    }

    if (!res.ptr()) {
        res = new XmetaResult(XCAM_IMAGE_PROCESS_ONCE);
        XCAM_ASSERT (res.ptr());
        output.push_back(res);
    }

    CameraMetadata* metadata = res->get_metadata_result();
    struct CamIA10_SensorModeData &sensor_desc = _aiq_compositor->get_sensor_mode_data();
    ParamsTranslate::convert_from_rkisp_af_result(&_rkaiq_result, &af_results, &sensor_desc);

    XCamAfParam &afParams = inputParams->afInputParams.afParams;
    entry = inputParams->settings.find(ANDROID_CONTROL_AF_REGIONS);
    if (entry.count == 5)
        metadata->update(ANDROID_CONTROL_AF_REGIONS, inputParams->afInputParams.afRegion, entry.count);

    ret = mAfState->processResult(_rkaiq_result, afParams, *metadata);

    return ret;
}

XCamReturn AiqCommonHandler::initTonemaps()
{
#define TONEMAP_MAX_CURVE_POINTS 1024
    mMaxCurvePoints = TONEMAP_MAX_CURVE_POINTS;

    mRGammaLut = new float[mMaxCurvePoints * 2];
    mGGammaLut = new float[mMaxCurvePoints * 2];
    mBGammaLut = new float[mMaxCurvePoints * 2];

    // Initialize P_IN, P_OUT values [(P_IN, P_OUT), ..]
    for (unsigned int i = 0; i < mMaxCurvePoints; i++) {
        mRGammaLut[i * 2] = (float) i / (mMaxCurvePoints - 1);
        mRGammaLut[i * 2 + 1] = (float) i / (mMaxCurvePoints - 1);
        mGGammaLut[i * 2] = (float) i / (mMaxCurvePoints - 1);
        mGGammaLut[i * 2 + 1] = (float) i / (mMaxCurvePoints - 1);
        mBGammaLut[i * 2] = (float) i / (mMaxCurvePoints - 1);
        mBGammaLut[i * 2 + 1] = (float) i / (mMaxCurvePoints - 1);
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
AiqCommonHandler::fillTonemapCurve(CamerIcIspGocConfig_t goc, AiqInputParams* inputParams, CameraMetadata* metadata)
{
    int multiplier = 1;
    CameraMetadata* staticMeta  = inputParams->staticMeta;
    XCAM_ASSERT (staticMeta);
    camera_metadata_entry_t rw_entry;
    rw_entry = staticMeta->find(ANDROID_TONEMAP_AVAILABLE_TONE_MAP_MODES);
    if (rw_entry.count == 2) {
        if (((rw_entry.data.u8[0] != ANDROID_TONEMAP_MODE_FAST) && (rw_entry.data.u8[0] != ANDROID_TONEMAP_MODE_HIGH_QUALITY))||
            ((rw_entry.data.u8[1] != ANDROID_TONEMAP_MODE_FAST) && (rw_entry.data.u8[1] != ANDROID_TONEMAP_MODE_HIGH_QUALITY))) {
            LOGE("@%s %d: only support fast and high_quality tonemaps mode, modify camera3_profile.xml", __FUNCTION__, __LINE__);
            return XCAM_RETURN_NO_ERROR;
        }
    } else {
        LOGW("@%s %d: only support fast and high_quality tonemaps mode, modify camera3_profile.xml", __FUNCTION__, __LINE__);
        return XCAM_RETURN_NO_ERROR;
    }

    const CameraMetadata* settings  = &inputParams->settings;
    camera_metadata_ro_entry entry = settings->find(ANDROID_TONEMAP_MODE);
    if (entry.count == 1) {
        if ((entry.data.u8[0] != ANDROID_TONEMAP_MODE_FAST) && (entry.data.u8[0] != ANDROID_TONEMAP_MODE_HIGH_QUALITY)) {
            LOGE("@%s %d: not support the tonemap mode:%d", __FUNCTION__, __LINE__, entry.data.u8[0]);
            return XCAM_RETURN_NO_ERROR;
        }
        metadata->update(ANDROID_TONEMAP_MODE, entry.data.u8, entry.count);
    } else {
        LOGE("@%s %d: do not find the tonemap mode in settings", __FUNCTION__, __LINE__);
        return XCAM_RETURN_NO_ERROR;
    }

    if (mMaxCurvePoints < CAMERIC_ISP_GAMMA_CURVE_SIZE && mMaxCurvePoints > 0) {
        multiplier = CAMERIC_ISP_GAMMA_CURVE_SIZE / mMaxCurvePoints;
        LOGI("Not enough curve points. Linear interpolation is used."); } else {
        mMaxCurvePoints = CAMERIC_ISP_GAMMA_CURVE_SIZE;
        if (mMaxCurvePoints > CIFISP_GAMMA_OUT_MAX_SAMPLES)
            mMaxCurvePoints = CIFISP_GAMMA_OUT_MAX_SAMPLES;
    }

    if (mRGammaLut == nullptr ||
        mGGammaLut == nullptr ||
        mBGammaLut == nullptr) {
        LOGE("Lut tables are not initialized.");
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    unsigned short gamma_y_max = mMaxCurvePoints > 0 ? goc.gamma_y.GammaY[mMaxCurvePoints - 1] :
        goc.gamma_y.GammaY[0];
    for (uint32_t i=0; i < mMaxCurvePoints; i++) {
        if (mMaxCurvePoints > 1)
            mRGammaLut[i * 2] = (float) i / (mMaxCurvePoints - 1);
        mRGammaLut[i * 2 + 1] = (float)goc.gamma_y.GammaY[i * multiplier] / gamma_y_max;
        if (mMaxCurvePoints > 1)
            mGGammaLut[i * 2] = (float) i / (mMaxCurvePoints - 1);
        mGGammaLut[i * 2 + 1] = (float)goc.gamma_y.GammaY[i * multiplier] / gamma_y_max;
        if (mMaxCurvePoints > 1)
            mBGammaLut[i * 2] = (float) i / (mMaxCurvePoints - 1);
        mBGammaLut[i * 2 + 1] = (float)goc.gamma_y.GammaY[i * multiplier] / gamma_y_max;
    }
    metadata->update(ANDROID_TONEMAP_CURVE_RED,
                     mRGammaLut,
                     mMaxCurvePoints * 2);
    metadata->update(ANDROID_TONEMAP_CURVE_GREEN,
                     mGGammaLut,
                     mMaxCurvePoints * 2);
    metadata->update(ANDROID_TONEMAP_CURVE_BLUE,
                     mBGammaLut,
                     mMaxCurvePoints * 2);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
AiqCommonHandler::processMiscMetaResults(X3aResultList &output)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<XmetaResult> res;
    camera_metadata_entry entry;
    LOGI("@%s %d: enter", __FUNCTION__, __LINE__);

    for (X3aResultList::iterator iter = output.begin ();
            iter != output.end (); iter++)
    {
        if ((*iter)->get_type() == XCAM_3A_METADATA_RESULT_TYPE) {
            res = (*iter).dynamic_cast_ptr<XmetaResult> ();
            break ;
        }
    }

    if (!res.ptr()) {
        res = new XmetaResult(XCAM_IMAGE_PROCESS_ONCE);
        XCAM_ASSERT (res.ptr());
        output.push_back(res);
    }

    CameraMetadata* metadata = res->get_metadata_result();

    int64_t effect_frame_id = (int)(_aiq_compositor->get_3a_isp_stats().frame_id);
    metadata->update(RKCAMERA3_PRIVATEDATA_EFFECTIVE_DRIVER_FRAME_ID,
                     &effect_frame_id,
                     1);

    int64_t frame_sof_ts = _aiq_compositor->get_3a_ia10_stats ().stats_sof_ts;
    metadata->update(RKCAMERA3_PRIVATEDATA_FRAME_SOF_TIMESTAMP,
                     &frame_sof_ts,
                     1);

    // Update reqId for the result in order to match the setting param
    int reqId = _aiq_compositor->getAiqInputParams().ptr() ? _aiq_compositor->getAiqInputParams()->reqId : -1;
    metadata->update(ANDROID_REQUEST_ID, &reqId, 1);

    return ret;
}

XCamReturn
AiqCommonHandler::processToneMapsMetaResults(CamerIcIspGocConfig_t goc, X3aResultList &output)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<AiqInputParams> inputParams = _aiq_compositor->getAiqInputParams();
    SmartPtr<XmetaResult> res;
    camera_metadata_entry entry;
    LOGI("@%s %d: enter", __FUNCTION__, __LINE__);

    for (X3aResultList::iterator iter = output.begin ();
            iter != output.end (); iter++)
    {
        if ((*iter)->get_type() == XCAM_3A_METADATA_RESULT_TYPE) {
            res = (*iter).dynamic_cast_ptr<XmetaResult> ();
            break ;
        }
    }

    if (!res.ptr()) {
        res = new XmetaResult(XCAM_IMAGE_PROCESS_ONCE);
        XCAM_ASSERT (res.ptr());
        output.push_back(res);
    }

    CameraMetadata* metadata = res->get_metadata_result();
    ret = fillTonemapCurve(goc, inputParams.ptr(), metadata);

    return ret;
}

XCamReturn
AiqAeHandler::analyze (X3aResultList &output, bool first)
{
    XCAM_ASSERT (_analyzer);
    SmartPtr<AiqInputParams> inputParams = _aiq_compositor->getAiqInputParams();
    bool forceAeRun = first ? true : false;

    if (inputParams.ptr()) {
        bool forceAeRun = _latestInputParams.aeInputParams.aeParams.ev_shift !=
            inputParams->aeInputParams.aeParams.ev_shift;

        // process state when the request is actually processed
        mAeState->processState(inputParams->aaaControls.controlMode,
                               inputParams->aaaControls.ae);

        _latestInputParams = *inputParams.ptr();
    }

    if (forceAeRun || mAeState->getState() != ANDROID_CONTROL_AE_STATE_LOCKED) {

        SmartPtr<X3aResult> result;
        if (inputParams.ptr())
            this->update_parameters (inputParams->aeInputParams.aeParams);
        XCamAeParam param = this->get_params_unlock ();

        if (_aiq_compositor->_isp10_engine->runAe(&param, &_result, first) != 0)
            return XCAM_RETURN_NO_ERROR;
        result = pop_result ();
        mLastestAeresult = result;
        if (result.ptr())
            output.push_back (result);

    }

    return XCAM_RETURN_NO_ERROR;
}

bool
AiqAeHandler::manual_control_result (
    ia_aiq_exposure_sensor_parameters &cur_res,
    ia_aiq_exposure_parameters &cur_aiq_exp,
    const ia_aiq_exposure_sensor_parameters &last_res)
{
    adjust_ae_speed (cur_res, cur_aiq_exp, last_res, this->get_speed_unlock());
    adjust_ae_limitation (cur_res, cur_aiq_exp);

    return true;
}

void
AiqAeHandler::adjust_ae_speed (
    ia_aiq_exposure_sensor_parameters &cur_res,
    ia_aiq_exposure_parameters &cur_aiq_exp,
    const ia_aiq_exposure_sensor_parameters &last_res,
    double ae_speed)
{
    double last_gain, input_gain, ret_gain;
    ia_aiq_exposure_sensor_parameters tmp_res;

    if (XCAM_DOUBLE_EQUAL_AROUND(ae_speed, 1.0 ))
        return;
    xcam_mem_clear (tmp_res);
    tmp_res.coarse_integration_time = _calculate_new_value_by_speed (
                                          last_res.coarse_integration_time,
                                          cur_res.coarse_integration_time,
                                          ae_speed);

    last_gain = _imx185_sensor_gain_code_to_mutiplier (last_res.analog_gain_code_global);
    input_gain = _imx185_sensor_gain_code_to_mutiplier (cur_res.analog_gain_code_global);
    ret_gain = _calculate_new_value_by_speed (last_gain, input_gain, ae_speed);

    tmp_res.analog_gain_code_global = _mutiplier_to_imx185_sensor_gain_code (ret_gain);

    XCAM_LOG_DEBUG ("AE speed: from (shutter:%d, gain:%d[%.03f]) to (shutter:%d, gain:%d[%.03f])",
                    cur_res.coarse_integration_time, cur_res.analog_gain_code_global, input_gain,
                    tmp_res.coarse_integration_time, tmp_res.analog_gain_code_global, ret_gain);

    cur_res.coarse_integration_time = tmp_res.coarse_integration_time;
    cur_res.analog_gain_code_global = tmp_res.analog_gain_code_global;
    cur_aiq_exp.exposure_time_us = _coarse_line_to_time (&_sensor_descriptor,
                                   cur_res.coarse_integration_time);
    cur_aiq_exp.analog_gain = ret_gain;
}

void
AiqAeHandler::adjust_ae_limitation (ia_aiq_exposure_sensor_parameters &cur_res,
                                    ia_aiq_exposure_parameters &cur_aiq_exp)
{
    ia_aiq_exposure_sensor_descriptor * desc = &_sensor_descriptor;
    uint64_t exposure_min = 0, exposure_max = 0;
    double analog_max = get_max_analog_gain_unlock ();
    uint32_t min_coarse_value = desc->coarse_integration_time_min;
    uint32_t max_coarse_value = desc->line_periods_per_field - desc->coarse_integration_time_max_margin;
    uint32_t value;

    get_exposure_time_range_unlock (exposure_min, exposure_max);

    if (exposure_min) {
        value = _time_to_coarse_line (desc, (uint32_t)exposure_min);
        min_coarse_value = (value > min_coarse_value) ? value : min_coarse_value;
    }
    if (cur_res.coarse_integration_time < min_coarse_value) {
        cur_res.coarse_integration_time = min_coarse_value;
        cur_aiq_exp.exposure_time_us = _coarse_line_to_time (desc, min_coarse_value);
    }

    if (exposure_max) {
        value = _time_to_coarse_line (desc, (uint32_t)exposure_max);
        max_coarse_value = (value < max_coarse_value) ? value : max_coarse_value;
    }
    if (cur_res.coarse_integration_time > max_coarse_value) {
        cur_res.coarse_integration_time = max_coarse_value;
        cur_aiq_exp.exposure_time_us = _coarse_line_to_time (desc, max_coarse_value);
    }

    if (analog_max >= 1.0) {
        /* limit gains */
        double gain = _imx185_sensor_gain_code_to_mutiplier (cur_res.analog_gain_code_global);
        if (gain > analog_max) {
            cur_res.analog_gain_code_global = _mutiplier_to_imx185_sensor_gain_code (analog_max);
            cur_aiq_exp.analog_gain = analog_max;
        }
    }
}

XCamFlickerMode
AiqAeHandler::get_flicker_mode ()
{
    {
        AnalyzerHandler::HandlerLock lock(this);
    }
    return AeHandler::get_flicker_mode ();
}

int64_t
AiqAeHandler::get_current_exposure_time ()
{
    AnalyzerHandler::HandlerLock lock(this);

    return (int64_t)_result.coarse_integration_time;
}

float
AiqAeHandler::get_current_exposure_time_us ()
{
    AnalyzerHandler::HandlerLock lock(this);

    return _result.coarse_integration_time * 1000000;
}

double
AiqAeHandler::get_current_analog_gain ()
{
    AnalyzerHandler::HandlerLock lock(this);
    return (double)_result.analog_gain_code_global;
}

double
AiqAeHandler::get_max_analog_gain ()
{
    {
        AnalyzerHandler::HandlerLock lock(this);
    }
    return AeHandler::get_max_analog_gain ();
}

XCamReturn
AiqAeHandler::set_RGBS_weight_grid (ia_aiq_rgbs_grid **out_rgbs_grid)
{
    AnalyzerHandler::HandlerLock lock(this);

    rgbs_grid_block *rgbs_grid_ptr = (*out_rgbs_grid)->blocks_ptr;
    uint32_t rgbs_grid_index = 0;
    uint16_t rgbs_grid_width = (*out_rgbs_grid)->grid_width;
    uint16_t rgbs_grid_height = (*out_rgbs_grid)->grid_height;

    XCAM_LOG_DEBUG ("rgbs_grid_width = %d, rgbs_grid_height = %d", rgbs_grid_width, rgbs_grid_height);

    uint64_t weight_sum = 0;

    uint32_t image_width = 0;
    uint32_t image_height = 0;
    _aiq_compositor->get_size (image_width, image_height);
    XCAM_LOG_DEBUG ("image_width = %d, image_height = %d", image_width, image_height);

    uint32_t hor_pixels_per_grid = (image_width + (rgbs_grid_width >> 1)) / rgbs_grid_width;
    uint32_t vert_pixels_per_gird = (image_height + (rgbs_grid_height >> 1)) / rgbs_grid_height;
    XCAM_LOG_DEBUG ("rgbs grid: %d x %d pixels per grid cell", hor_pixels_per_grid, vert_pixels_per_gird);

    XCam3AWindow weighted_window = this->get_window_unlock ();
    uint32_t weighted_grid_width = ((weighted_window.x_end - weighted_window.x_start + 1) +
                                    (hor_pixels_per_grid >> 1)) / hor_pixels_per_grid;
    uint32_t weighted_grid_height = ((weighted_window.y_end - weighted_window.y_start + 1) +
                                     (vert_pixels_per_gird >> 1)) / vert_pixels_per_gird;
    XCAM_LOG_DEBUG ("weighted_grid_width = %d, weighted_grid_height = %d", weighted_grid_width, weighted_grid_height);

    uint32_t *weighted_avg_gr = (uint32_t*)xcam_malloc0 (5 * weighted_grid_width * weighted_grid_height * sizeof(uint32_t));
    if (NULL == weighted_avg_gr) {
        return XCAM_RETURN_ERROR_MEM;
    }
    uint32_t *weighted_avg_r = weighted_avg_gr + (weighted_grid_width * weighted_grid_height);
    uint32_t *weighted_avg_b = weighted_avg_r + (weighted_grid_width * weighted_grid_height);
    uint32_t *weighted_avg_gb = weighted_avg_b + (weighted_grid_width * weighted_grid_height);
    uint32_t *weighted_sat = weighted_avg_gb + (weighted_grid_width * weighted_grid_height);

    for (uint32_t win_index = 0; win_index < XCAM_AE_MAX_METERING_WINDOW_COUNT; win_index++) {
        XCAM_LOG_DEBUG ("window start point(%d, %d), end point(%d, %d), weight = %d",
                        _params.window_list[win_index].x_start, _params.window_list[win_index].y_start,
                        _params.window_list[win_index].x_end, _params.window_list[win_index].y_end,
                        _params.window_list[win_index].weight);

        if ((_params.window_list[win_index].weight <= 0) ||
                (_params.window_list[win_index].x_start < 0) ||
                ((uint32_t)_params.window_list[win_index].x_end > image_width) ||
                (_params.window_list[win_index].y_start < 0) ||
                ((uint32_t)_params.window_list[win_index].y_end > image_height) ||
                (_params.window_list[win_index].x_start >= _params.window_list[win_index].x_end) ||
                (_params.window_list[win_index].y_start >= _params.window_list[win_index].y_end) ||
                ((uint32_t)_params.window_list[win_index].x_end - (uint32_t)_params.window_list[win_index].x_start > image_width) ||
                ((uint32_t)_params.window_list[win_index].y_end - (uint32_t)_params.window_list[win_index].y_start > image_height)) {
            XCAM_LOG_DEBUG ("skip window index = %d ", win_index);
            continue;
        }

        rgbs_grid_index = (_params.window_list[win_index].x_start +
                           (hor_pixels_per_grid >> 1)) / hor_pixels_per_grid +
                          ((_params.window_list[win_index].y_start + (vert_pixels_per_gird >> 1))
                           / vert_pixels_per_gird) * rgbs_grid_width;

        weight_sum += _params.window_list[win_index].weight;

        XCAM_LOG_DEBUG ("cumulate rgbs grid statistic, window index = %d ", win_index);
        for (uint32_t i = 0; i < weighted_grid_height; i++) {
            for (uint32_t j = 0; j < weighted_grid_width; j++) {
                weighted_avg_gr[j + i * weighted_grid_width] += rgbs_grid_ptr[rgbs_grid_index + j +
                        i * rgbs_grid_width].avg_gr * _params.window_list[win_index].weight;
                weighted_avg_r[j + i * weighted_grid_width] += rgbs_grid_ptr[rgbs_grid_index + j +
                        i * rgbs_grid_width].avg_r * _params.window_list[win_index].weight;
                weighted_avg_b[j + i * weighted_grid_width] += rgbs_grid_ptr[rgbs_grid_index + j +
                        i * rgbs_grid_width].avg_b * _params.window_list[win_index].weight;
                weighted_avg_gb[j + i * weighted_grid_width] += rgbs_grid_ptr[rgbs_grid_index + j +
                        i * rgbs_grid_width].avg_gb * _params.window_list[win_index].weight;
                weighted_sat[j + i * weighted_grid_width] += rgbs_grid_ptr[rgbs_grid_index + j +
                        i * rgbs_grid_width].sat * _params.window_list[win_index].weight;
            }
        }
    }
    XCAM_LOG_DEBUG ("sum of weighing factor = %" PRIu64, weight_sum);

    rgbs_grid_index = (weighted_window.x_start + (hor_pixels_per_grid >> 1)) / hor_pixels_per_grid +
                      (weighted_window.y_start + (vert_pixels_per_gird >> 1)) / vert_pixels_per_gird * rgbs_grid_width;
    for (uint32_t i = 0; i < weighted_grid_height; i++) {
        for (uint32_t j = 0; j < weighted_grid_width; j++) {
            rgbs_grid_ptr[rgbs_grid_index + j + i * rgbs_grid_width].avg_gr =
                weighted_avg_gr[j + i * weighted_grid_width] / weight_sum;
            rgbs_grid_ptr[rgbs_grid_index + j + i * rgbs_grid_width].avg_r =
                weighted_avg_r[j + i * weighted_grid_width] / weight_sum;
            rgbs_grid_ptr[rgbs_grid_index + j + i * rgbs_grid_width].avg_b =
                weighted_avg_b[j + i * weighted_grid_width] / weight_sum;
            rgbs_grid_ptr[rgbs_grid_index + j + i * rgbs_grid_width].avg_gb =
                weighted_avg_gb[j + i * weighted_grid_width] / weight_sum;
            rgbs_grid_ptr[rgbs_grid_index + j + i * rgbs_grid_width].sat =
                weighted_sat[j + i * weighted_grid_width] / weight_sum;
        }
    }

    xcam_free (weighted_avg_gr);

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
AiqAeHandler::set_hist_weight_grid (ia_aiq_hist_weight_grid **out_weight_grid)
{
    AnalyzerHandler::HandlerLock lock(this);

    uint16_t hist_grid_width = (*out_weight_grid)->width;
    uint16_t hist_grid_height = (*out_weight_grid)->height;
    uint32_t hist_grid_index = 0;

    unsigned char* weights_map_ptr = (*out_weight_grid)->weights;

    uint32_t image_width = 0;
    uint32_t image_height = 0;
    _aiq_compositor->get_size (image_width, image_height);

    uint32_t hor_pixels_per_grid = (image_width + (hist_grid_width >> 1)) / hist_grid_width;
    uint32_t vert_pixels_per_gird = (image_height + (hist_grid_height >> 1)) / hist_grid_height;
    XCAM_LOG_DEBUG ("hist weight grid: %d x %d pixels per grid cell", hor_pixels_per_grid, vert_pixels_per_gird);

    memset (weights_map_ptr, 0, hist_grid_width * hist_grid_height);

    for (uint32_t win_index = 0; win_index < XCAM_AE_MAX_METERING_WINDOW_COUNT; win_index++) {
        XCAM_LOG_DEBUG ("window start point(%d, %d), end point(%d, %d), weight = %d",
                        _params.window_list[win_index].x_start, _params.window_list[win_index].y_start,
                        _params.window_list[win_index].x_end, _params.window_list[win_index].y_end,
                        _params.window_list[win_index].weight);

        if ((_params.window_list[win_index].weight <= 0) ||
                (_params.window_list[win_index].weight > 15) ||
                (_params.window_list[win_index].x_start < 0) ||
                ((uint32_t)_params.window_list[win_index].x_end > image_width) ||
                (_params.window_list[win_index].y_start < 0) ||
                ((uint32_t)_params.window_list[win_index].y_end > image_height) ||
                (_params.window_list[win_index].x_start >= _params.window_list[win_index].x_end) ||
                (_params.window_list[win_index].y_start >= _params.window_list[win_index].y_end) ||
                ((uint32_t)_params.window_list[win_index].x_end - (uint32_t)_params.window_list[win_index].x_start > image_width) ||
                ((uint32_t)_params.window_list[win_index].y_end - (uint32_t)_params.window_list[win_index].y_start > image_height)) {
            XCAM_LOG_DEBUG ("skip window index = %d ", win_index);
            continue;
        }

        uint32_t weighted_grid_width =
            ((_params.window_list[win_index].x_end - _params.window_list[win_index].x_start + 1) +
             (hor_pixels_per_grid >> 1)) / hor_pixels_per_grid;
        uint32_t weighted_grid_height =
            ((_params.window_list[win_index].y_end - _params.window_list[win_index].y_start + 1) +
             (vert_pixels_per_gird >> 1)) / vert_pixels_per_gird;

        hist_grid_index = (_params.window_list[win_index].x_start + (hor_pixels_per_grid >> 1)) / hor_pixels_per_grid +
                          ((_params.window_list[win_index].y_start + (vert_pixels_per_gird >> 1)) /
                           vert_pixels_per_gird) * hist_grid_width;

        for (uint32_t i = 0; i < weighted_grid_height; i++) {
            for (uint32_t j = 0; j < weighted_grid_width; j++) {
                weights_map_ptr[hist_grid_index + j + i * hist_grid_width] = _params.window_list[win_index].weight;
            }
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
AiqAeHandler::dump_hist_weight_grid (const ia_aiq_hist_weight_grid *weight_grid)
{
    XCAM_LOG_DEBUG ("E dump_hist_weight_grid");
    if (NULL == weight_grid) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    uint16_t grid_width = weight_grid->width;
    uint16_t grid_height = weight_grid->height;

    for (uint32_t i = 0; i < grid_height; i++) {
        for (uint32_t j = 0; j < grid_width; j++) {
            printf ("%d  ", weight_grid->weights[j + i * grid_width]);
        }
        printf("\n");
    }

    XCAM_LOG_DEBUG ("X dump_hist_weight_grid");
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
AiqAeHandler::dump_RGBS_grid (const ia_aiq_rgbs_grid *rgbs_grid)
{
    XCAM_LOG_DEBUG ("E dump_RGBS_grid");
    if (NULL == rgbs_grid) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    uint16_t grid_width = rgbs_grid->grid_width;
    uint16_t grid_height = rgbs_grid->grid_height;

    printf("AVG B\n");
    for (uint32_t i = 0; i < grid_height; i++) {
        for (uint32_t j = 0; j < grid_width; j++) {
            printf ("%d  ", rgbs_grid->blocks_ptr[j + i * grid_width].avg_b);
        }
        printf("\n");
    }
    printf("AVG Gb\n");
    for (uint32_t i = 0; i < grid_height; i++) {
        for (uint32_t j = 0; j < grid_width; j++) {
            printf ("%d  ", rgbs_grid->blocks_ptr[j + i * grid_width].avg_gb);
        }
        printf("\n");
    }
    printf("AVG Gr\n");
    for (uint32_t i = 0; i < grid_height; i++) {
        for (uint32_t j = 0; j < grid_width; j++) {
            printf ("%d  ", rgbs_grid->blocks_ptr[j + i * grid_width].avg_gr);
        }
        printf("\n");
    }
    printf("AVG R\n");
    for (uint32_t i = 0; i < grid_height; i++) {
        for (uint32_t j = 0; j < grid_width; j++) {
            printf ("%d  ", rgbs_grid->blocks_ptr[j + i * grid_width].avg_r);
            //printf ("%d  ", rgbs_grid->blocks_ptr[j + i * grid_width].sat);
        }
        printf("\n");
    }

    XCAM_LOG_DEBUG ("X dump_RGBS_grid");
    return XCAM_RETURN_NO_ERROR;
}

AiqAwbHandler::AiqAwbHandler (X3aAnalyzerRKiq *analyzer, SmartPtr<RKiqCompositor> &aiq_compositor)
    : _aiq_compositor (aiq_compositor)
    , _analyzer (analyzer)
    , _started (false)
{
    xcam_mem_clear (_cct_range);
    xcam_mem_clear (_result);
    xcam_mem_clear (_history_result);
    xcam_mem_clear (_cct_range);
    xcam_mem_clear (_input);
    mAwbState = new RkAWBStateMachine();
}

XCamReturn
AiqAwbHandler::analyze (X3aResultList &output, bool first)
{

    XCAM_ASSERT (_analyzer);
    SmartPtr<AiqInputParams> inputParams = _aiq_compositor->getAiqInputParams();
    bool forceAwbRun = first ? true : false;

    if (inputParams.ptr()) {
        bool forceAwbRun = (inputParams->reqId == 0);

        // process state when the request is actually processed
        mAwbState->processState(inputParams->aaaControls.controlMode,
                               inputParams->aaaControls.awb);
    }

    if (forceAwbRun || mAwbState->getState() != ANDROID_CONTROL_AWB_STATE_LOCKED) {

        if (inputParams.ptr())
            this->update_parameters (inputParams->awbInputParams.awbParams);

        //ensure_ia_parameters();
        XCamAwbParam param = this->get_params_unlock ();
        if (_aiq_compositor->_isp10_engine->runAwb(&param, &_result, first) != 0)
            return XCAM_RETURN_NO_ERROR;
    }

    return XCAM_RETURN_NO_ERROR;
}

bool
AiqAwbHandler::ensure_ia_parameters ()
{
    bool ret = true;

    ret = ret && ensure_awb_mode ();
    return ret;
}

bool
AiqAwbHandler::ensure_awb_mode ()
{
    return true;
}

void
AiqAwbHandler::adjust_speed (const ia_aiq_awb_results &last_ret)
{
/* TODO
    _result.final_r_per_g =
        _calculate_new_value_by_speed (
            last_ret.final_r_per_g, _result.final_r_per_g, get_speed_unlock ());
    _result.final_b_per_g =
        _calculate_new_value_by_speed (
            last_ret.final_b_per_g, _result.final_b_per_g, get_speed_unlock ());
*/
}

uint32_t
AiqAwbHandler::get_current_estimate_cct ()
{
    AnalyzerHandler::HandlerLock lock(this);
    // TODO
    return 0;//(uint32_t)_result.cct_estimate;
}

AiqAfHandler::AiqAfHandler (SmartPtr<RKiqCompositor> &aiq_compositor)
    : _aiq_compositor (aiq_compositor)
{
    mAfState = new RkAFStateMachine();
}

XCamReturn
AiqAfHandler::analyze (X3aResultList &output, bool first)
{
    // TODO
    XCAM_UNUSED (output);
    XCam3aResultFocus isp_result;
    xcam_mem_clear(isp_result);
    XCamAfParam param = this->get_params_unlock();
    SmartPtr<AiqInputParams> inputParams = _aiq_compositor->getAiqInputParams();
    if (inputParams.ptr()) {
        mAfState->processTriggers(inputParams->aaaControls.af.afTrigger,
                                    inputParams->aaaControls.af.afMode, 0,
                                    inputParams->afInputParams.afParams);
    }

    if (_aiq_compositor->_isp10_engine->runAf(&param, &isp_result, first) != 0)
        return XCAM_RETURN_NO_ERROR;

    XCAM_LOG_INFO ("AiqAfHandler, position: %d",
        isp_result.next_lens_position);

    X3aIspFocusResult *result = new X3aIspFocusResult(XCAM_IMAGE_PROCESS_ONCE);
    struct rkisp_focus focus;
    focus.next_lens_position = isp_result.next_lens_position;
    result->set_isp_config (focus);
    result->set_standard_result (isp_result);
    output.push_back (result);
    return XCAM_RETURN_NO_ERROR;
}

AiqCommonHandler::AiqCommonHandler (SmartPtr<RKiqCompositor> &aiq_compositor)
    : _aiq_compositor (aiq_compositor)
    , _gbce_result (NULL)
{
    initTonemaps();
}
AiqCommonHandler::~AiqCommonHandler ()
{
    delete mRGammaLut;
    delete mGGammaLut;
    delete mBGammaLut;
}

XCamReturn
AiqCommonHandler::analyze (X3aResultList &output, bool first)
{
    //XCAM_LOG_INFO ("---------------run analyze");

    return XCAM_RETURN_NO_ERROR;
}

void
RKiqCompositor::convert_window_to_ia (const XCam3AWindow &window, ia_rectangle &ia_window)
{
    ia_rectangle source;
    ia_coordinate_system source_system;
    ia_coordinate_system target_system = {IA_COORDINATE_TOP, IA_COORDINATE_LEFT, IA_COORDINATE_BOTTOM, IA_COORDINATE_RIGHT};

    source_system.left = 0;
    source_system.top = 0;
    source_system.right = this->_width;
    source_system.bottom = this->_height;
    XCAM_ASSERT (_width && _height);

    source.left = window.x_start;
    source.top = window.y_start;
    source.right = window.x_end;
    source.bottom = window.y_end;
    //ia_coordinate_convert_rect (&source_system, &source, &target_system, &ia_window);
}

RKiqCompositor::RKiqCompositor ()
    : _inputParams(NULL)
    , _ia_handle (NULL)
    , _ia_mkn (NULL)
    , _pa_result (NULL)
    , _frame_use (ia_aiq_frame_use_video)
    , _width (0)
    , _height (0)
    , _isp10_engine(NULL)
    , _all_stats_meas_types(0)
{
    xcam_mem_clear (_frame_params);
    xcam_mem_clear (_isp_stats);
    xcam_mem_clear (_ia_stat);
    xcam_mem_clear (_ia_dcfg);
    xcam_mem_clear (_ia_results);
    xcam_mem_clear (_isp_cfg);
    _isp_stats.frame_id = -1;
    _handle_manager = new X3aHandlerManager();
#if 1
    _ae_desc = _handle_manager->get_ae_handler_desc();
    _awb_desc = _handle_manager->get_awb_handler_desc();
    _af_desc = _handle_manager->get_af_handler_desc();
    _ae_handler = NULL;
    _awb_handler = NULL;
    _af_handler = NULL;
    _common_handler = NULL;
#else
    _ae_desc = X3aHandlerManager::instance()->get_ae_handler_desc();
    _awb_desc = X3aHandlerManager::instance()->get_awb_handler_desc();
    _af_desc = X3aHandlerManager::instance()->get_af_handler_desc();
#endif
    XCAM_LOG_DEBUG ("RKiqCompositor constructed");
}

RKiqCompositor::~RKiqCompositor ()
{
    if (!_isp10_engine) {
        delete _isp10_engine;
        _isp10_engine = NULL;
    }

    XCAM_LOG_DEBUG ("~RKiqCompositor destructed");
}

bool
RKiqCompositor::open (ia_binary_data &cpf)
{
    XCAM_LOG_DEBUG ("Aiq compositor opened");
    return true;
}

void
RKiqCompositor::close ()
{
    XCAM_LOG_DEBUG ("Aiq compositor closed");
}

void RKiqCompositor::set_isp_ctrl_device(Isp10Engine* dev) {
    if (dev == NULL) {
        XCAM_LOG_ERROR ("ISP control device is null");
        return;
    }

    _isp10_engine = dev;
    _isp10_engine->setExternalAEHandlerDesc(_ae_desc);
    _isp10_engine->setExternalAWBHandlerDesc(_awb_desc);
    _isp10_engine->setExternalAFHandlerDesc(_af_desc);
}

bool
RKiqCompositor::set_sensor_mode_data (struct isp_supplemental_sensor_mode_data *sensor_mode)
{
    if (!_isp10_engine) {
        XCAM_LOG_ERROR ("ISP control device is null");
        return false;
    }

    _isp10_engine->getSensorModedata(sensor_mode,  &_ia_dcfg.sensor_mode);
    if (_inputParams.ptr()) {
        ParamsTranslate::convert_to_rkisp_awb_config(&_inputParams->awbInputParams.awbParams,
                                                     &_ia_dcfg.awb_cfg, &_ia_dcfg.sensor_mode);
        ParamsTranslate::convert_to_rkisp_aec_config(&_inputParams->aeInputParams.aeParams,
                                                     &_ia_dcfg.aec_cfg, &_ia_dcfg.sensor_mode);
        ParamsTranslate::convert_to_rkisp_af_config(&_inputParams->afInputParams.afParams,
                                                     &_ia_dcfg.afc_cfg, &_ia_dcfg.sensor_mode);
        AAAControls *aaaControls = &_inputParams->aaaControls;
        // update ae lock
        _ia_dcfg.aaa_locks &= ~HAL_3A_LOCKS_EXPOSURE;
        _ia_dcfg.aaa_locks |= aaaControls->ae.aeLock ? HAL_3A_LOCKS_EXPOSURE : 0;
        // update awb lock
        _ia_dcfg.aaa_locks &= ~HAL_3A_LOCKS_WB;
        _ia_dcfg.aaa_locks |= aaaControls->awb.awbLock ? HAL_3A_LOCKS_WB : 0;
        // update af lock
    }
    _isp10_engine->updateDynamicConfig(&_ia_dcfg);
    _ia_stat.sensor_mode = _ia_dcfg.sensor_mode;

    return true;
}

bool
RKiqCompositor::init_dynamic_config ()
{
    if (!_isp10_engine) {
        XCAM_LOG_ERROR ("ISP control device is null");
        return false;
    }
    _ia_dcfg = *(_isp10_engine->getDynamicISPConfig());

    return true;
}

bool
RKiqCompositor::set_vcm_time (struct rk_cam_vcm_tim *vcm_tim)
{
    if (!_isp10_engine) {
        XCAM_LOG_ERROR ("ISP control device is null");
        return false;
    }

    _ia_stat.vcm_tim = *vcm_tim;

    return true;
}

bool
RKiqCompositor::set_frame_softime (int64_t sof_tim)
{
    if (!_isp10_engine) {
        XCAM_LOG_ERROR ("ISP control device is null");
        return false;
    }

    _ia_stat.sof_tim = sof_tim;

    return true;
}

bool
RKiqCompositor::set_effect_ispparams (struct rkisp_parameters& isp_params)
{
    if (!_isp10_engine) {
        XCAM_LOG_ERROR ("ISP control device is null");
        return false;
    }

    _ia_stat.effct_awb_gains.fRed = isp_params.awb_algo_results.fRedGain;
    _ia_stat.effct_awb_gains.fGreenR = isp_params.awb_algo_results.fGreenRGain;
    _ia_stat.effct_awb_gains.fGreenB = isp_params.awb_algo_results.fGreenBGain;
    _ia_stat.effct_awb_gains.fBlue = isp_params.awb_algo_results.fBlueGain;
    memcpy(&_ia_stat.effect_CtMatrix, isp_params.awb_algo_results.fCtCoeff,
           sizeof(isp_params.awb_algo_results.fCtCoeff));
    memcpy(&_ia_stat.effect_CtOffset, isp_params.awb_algo_results.fCtOffset,
           sizeof(isp_params.awb_algo_results.fCtOffset));
    _ia_stat.stats_sof_ts = isp_params.frame_sof_ts;

    return true;
}
bool
RKiqCompositor::set_3a_stats (SmartPtr<X3aIspStatistics> &stats)
{
    int64_t frame_ts;
    int64_t vcm_ts;
    float cur_exptime = 0;

    if (!_isp10_engine) {
        XCAM_LOG_ERROR ("ISP control device is null");
        return false;
    }

    _isp_stats = *(struct cifisp_stat_buffer*)stats->get_isp_stats();
    frame_ts = _ia_stat.stats_sof_ts / 1000;
    XCAM_LOG_DEBUG ("set_3a_stats meas type: %d", _isp_stats.meas_type);

    vcm_ts = (int64_t)_ia_stat.vcm_tim.vcm_end_t.tv_sec * 1000 * 1000 +
             (int64_t)_ia_stat.vcm_tim.vcm_end_t.tv_usec;

    cur_exptime = _ia_stat.sensor_mode.exp_time_seconds * 1000 * 1000;

    if (vcm_ts + cur_exptime <= frame_ts)
      _ia_stat.af.cameric.MoveStatus = AFM_VCM_MOVE_END;
    else
      _ia_stat.af.cameric.MoveStatus = AFM_VCM_MOVE_RUNNING;

    XCAM_LOG_DEBUG ("MoveStatus: %d, vcm_ts %lld, cur_exptime %f, frame_ts %lld",
        _ia_stat.af.cameric.MoveStatus, vcm_ts / 1000, cur_exptime / 1000, frame_ts / 1000);

    // clear old value 
    _ia_stat.meas_type = 0;
    _isp10_engine->convertIspStats(&_isp_stats, &_ia_stat);
    // record all stats types fore same frame before,
    // stats of one frame may come in several times
    _all_stats_meas_types |= _ia_stat.meas_type;
    _isp10_engine->setStatistics(&_ia_stat);
    return true;
}

XCamReturn RKiqCompositor::convert_color_effect (IspInputParameters &isp_input)
{
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RKiqCompositor::apply_gamma_table (struct rkisp_parameters *isp_param)
{
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RKiqCompositor::apply_night_mode (struct rkisp_parameters *isp_param)
{
    return XCAM_RETURN_NO_ERROR;
}

double
RKiqCompositor::calculate_value_by_factor (double factor, double min, double mid, double max)
{
    XCAM_ASSERT (factor >= -1.0 && factor <= 1.0);
    XCAM_ASSERT (min <= mid && max >= mid);

    if (factor >= 0.0)
        return (mid * (1.0 - factor) + max * factor);
    else
        return (mid * (1.0 + factor) + min * (-factor));
}

XCamReturn
RKiqCompositor::limit_nr_levels (struct rkisp_parameters *isp_param)
{
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RKiqCompositor::integrate (X3aResultList &results)
{
    SmartPtr<X3aResult> isp_results;
    struct rkisp_parameters isp_3a_result;

    if (!_isp10_engine)
        XCAM_LOG_ERROR ("ISP control device is null");

    xcam_mem_clear(isp_3a_result);

    //_isp10_engine->runIA(&_ia_dcfg, &_ia_stat, &_ia_results);
    _isp10_engine->getIAResult(&_ia_results);

    if (!_isp10_engine->runISPManual(&_ia_results, BOOL_TRUE)) {
        XCAM_LOG_ERROR("%s:run ISP manual failed!", __func__);
    }

    if (_ae_handler && _awb_handler && _inputParams.ptr()) {
        if (_all_stats_meas_types ==
            (CAMIA10_AEC_MASK | CAMIA10_HST_MASK | CAMIA10_AWB_MEAS_MASK | CAMIA10_AFC_MASK)) {
            LOGD("%s:%d, complete all 3A stats analysis, report results",
                 __FUNCTION__, __LINE__);
            _ae_handler->processAeMetaResults(_ia_results.aec, results);
            _awb_handler->processAwbMetaResults(_ia_results.awb, results);
            _af_handler->processAfMetaResults(_ia_results.af, results);
            _common_handler->processToneMapsMetaResults(_ia_results.goc, results);
            _common_handler->processMiscMetaResults(results);
            _all_stats_meas_types = 0;
        }
    }

    _isp10_engine->convertIAResults(&_isp_cfg, &_ia_results);

    isp_3a_result.active_configs = _isp_cfg.active_configs;
    isp_3a_result.dpcc_config = _isp_cfg.configs.dpcc_config;
    isp_3a_result.bls_config = _isp_cfg.configs.bls_config;
    isp_3a_result.sdg_config = _isp_cfg.configs.sdg_config;
    isp_3a_result.hst_config = _isp_cfg.configs.hst_config;
    isp_3a_result.lsc_config = _isp_cfg.configs.lsc_config;
    isp_3a_result.awb_gain_config = _isp_cfg.configs.awb_gain_config;
    isp_3a_result.awb_meas_config = _isp_cfg.configs.awb_meas_config;
    isp_3a_result.flt_config = _isp_cfg.configs.flt_config;
    isp_3a_result.bdm_config = _isp_cfg.configs.bdm_config;
    isp_3a_result.ctk_config = _isp_cfg.configs.ctk_config;
    isp_3a_result.goc_config = _isp_cfg.configs.goc_config;
    isp_3a_result.cproc_config = _isp_cfg.configs.cproc_config;
    isp_3a_result.aec_config = _isp_cfg.configs.aec_config;
    isp_3a_result.afc_config = _isp_cfg.configs.afc_config;
    isp_3a_result.ie_config = _isp_cfg.configs.ie_config;
    isp_3a_result.dpf_config = _isp_cfg.configs.dpf_config;
    isp_3a_result.dpf_strength_config = _isp_cfg.configs.dpf_strength_config;
    isp_3a_result.aec_config = _isp_cfg.configs.aec_config;
    isp_3a_result.flt_denoise_level= _isp_cfg.configs.flt_denoise_level;
    isp_3a_result.flt_sharp_level= _isp_cfg.configs.flt_sharp_level;

    isp_3a_result.wdr_config = _isp_cfg.configs.wdr_config;
    isp_3a_result.demosaiclp_config = _isp_cfg.configs.demosaicLp_config;
    isp_3a_result.rkiesharp_config = _isp_cfg.configs.rkIESharp_config;
    // copy awb algo results
    isp_3a_result.awb_algo_results.fRedGain =
        _ia_results.awb.GainsAlgo.fRed;
    isp_3a_result.awb_algo_results.fGreenRGain =
        _ia_results.awb.GainsAlgo.fGreenR;
    isp_3a_result.awb_algo_results.fGreenBGain =
        _ia_results.awb.GainsAlgo.fGreenB;
    isp_3a_result.awb_algo_results.fBlueGain =
        _ia_results.awb.GainsAlgo.fBlue;
    memcpy(isp_3a_result.awb_algo_results.fCtCoeff,
           _ia_results.awb.CtMatrixAlgo.fCoeff,
           sizeof(_ia_results.awb.CtMatrixAlgo.fCoeff));
    memcpy(isp_3a_result.awb_algo_results.fCtOffset,
           &_ia_results.awb.CtOffsetAlgo,
           sizeof(_ia_results.awb.CtOffsetAlgo));

    // copy otp info
    isp_3a_result.otp_info_avl =
        _ia_results.otp_info_avl;
    isp_3a_result.awb_otp_info.enable =
        _ia_results.otp_info.awb.enable;
    isp_3a_result.awb_otp_info.golden_r_value =
        _ia_results.otp_info.awb.golden_r_value;
    isp_3a_result.awb_otp_info.golden_gr_value =
        _ia_results.otp_info.awb.golden_gr_value;
    isp_3a_result.awb_otp_info.golden_gb_value =
        _ia_results.otp_info.awb.golden_gb_value;
    isp_3a_result.awb_otp_info.golden_b_value =
        _ia_results.otp_info.awb.golden_b_value;
    // unsupport lsc/af otp now
    isp_3a_result.af_otp_info.enable = 0;
    isp_3a_result.lsc_otp_info.enable =
        _ia_results.otp_info.lsc.enable;

    for (int i=0; i < HAL_ISP_MODULE_MAX_ID_ID + 1; i++) {
        isp_3a_result.enabled[i] = _isp_cfg.enabled[i];
    }

    isp_results = generate_3a_configs (&isp_3a_result);
    results.push_back (isp_results);

    _isp10_engine->applyIspConfig(&_isp_cfg);

    return XCAM_RETURN_NO_ERROR;
}

SmartPtr<X3aResult>
RKiqCompositor::generate_3a_configs (struct rkisp_parameters *parameters)
{
    SmartPtr<X3aResult> ret;

    X3aAtomIspParametersResult *x3a_result =
        new X3aAtomIspParametersResult (XCAM_IMAGE_PROCESS_ONCE);
    x3a_result->set_isp_config (*parameters);
    ret = x3a_result;

    return ret;
}

void
RKiqCompositor::set_ae_handler (SmartPtr<AiqAeHandler> &handler)
{
    XCAM_ASSERT (handler.ptr());
    _ae_handler = handler.ptr();
}

void
RKiqCompositor::set_awb_handler (SmartPtr<AiqAwbHandler> &handler)
{
    XCAM_ASSERT (handler.ptr());
    _awb_handler = handler.ptr();
}

void
RKiqCompositor::set_af_handler (SmartPtr<AiqAfHandler> &handler)
{
    XCAM_ASSERT (handler.ptr());
    _af_handler = handler.ptr();
}

void
RKiqCompositor::set_common_handler (SmartPtr<AiqCommonHandler> &handler)
{
    XCAM_ASSERT (handler.ptr());
    _common_handler = handler.ptr();
}


};
