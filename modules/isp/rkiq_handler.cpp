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
#include "ebase/utl_fixfloat.h"

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
    // copy carefully, cause we use the same structures, so just easily copy
    // here
    memcpy(sensor.Hdrexp_smooth_setting, _result.Hdrexp_smooth_results,
           sizeof(sensor.Hdrexp_smooth_setting));
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
    CameraMetadata* staticMeta  = inputParams->staticMeta;

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
    entry = staticMeta->find(ANDROID_SENSOR_INFO_EXPOSURE_TIME_RANGE);
    if (entry.count == 2) {
        exptime_range_us[0] = entry.data.i64[0];
        exptime_range_us[1] = entry.data.i64[1];
        metadata->update(ANDROID_SENSOR_INFO_EXPOSURE_TIME_RANGE, exptime_range_us, 2);
    }

    int32_t sensitivity_range[2];
    entry = staticMeta->find(ANDROID_SENSOR_INFO_SENSITIVITY_RANGE);
    if (entry.count == 2) {
        sensitivity_range[0] = entry.data.i32[0];
        sensitivity_range[1] = entry.data.i32[1];
        metadata->update(ANDROID_SENSOR_INFO_SENSITIVITY_RANGE, sensitivity_range, 2);
    }

    entry = settings->find(ANDROID_CONTROL_AE_MODE);
    if (entry.count == 1 &&
        aec_results.flashModeState != AEC_FLASH_PREFLASH &&
        aec_results.flashModeState != AEC_FLASH_MAINFLASH) {
        uint8_t stillcap_sync = false;
        if (entry.data.u8[0] == ANDROID_CONTROL_AE_MODE_ON_ALWAYS_FLASH ||
            (entry.data.u8[0] == ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH &&
            aec_results.require_flash))
            stillcap_sync = true;
        else
            stillcap_sync = false;
        metadata->update(RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_NEEDED,
                         &stillcap_sync, 1);
    }

/*
    metadata->update(RKCAMERA3_PRIVATEDATA_GRID_WEIGHTS,
                     _rkaiq_result.hist_config_result.weights,
                    _rkaiq_result.hist_config_result.weights_cnt);
*/
    struct cifisp_stat_buffer& isp_stat = _aiq_compositor->get_3a_isp_stats();
/*
    metadata->update(RKCAMERA3_PRIVATEDATA_HIST_BINS,
                    (int*)isp_stat.params.hist.hist_bins,
                    CIFISP_HIST_BIN_N_MAX);
*/
    uint8_t hist_mode = ANDROID_STATISTICS_HISTOGRAM_MODE_ON;
    metadata->update(ANDROID_STATISTICS_HISTOGRAM_MODE,
                    &hist_mode,
                    1);
    metadata->update(ANDROID_STATISTICS_HISTOGRAM,
                    (int*)isp_stat.params.hist.hist_bins,
                    CIFISP_HIST_BIN_N_MAX);
    int32_t hist_cnt = CIFISP_HIST_BIN_N_MAX;
    metadata->update(ANDROID_STATISTICS_INFO_HISTOGRAM_BUCKET_COUNT,
                    &hist_cnt,
                    1);
    metadata->update(ANDROID_STATISTICS_INFO_MAX_HISTOGRAM_COUNT,
                    &hist_cnt,
                    1);
   metadata->update(RKCAMERA3_PRIVATEDATA_EXP_MEANS,
                    isp_stat.params.ae.exp_mean,
                    CIFISP_AE_MEAN_MAX);
   int32_t ae_mean_cnt = CIFISP_AE_MEAN_MAX;
   metadata->update(RKCAMERA3_PRIVATEDATA_EXP_MEANS_COUNT,
                    &ae_mean_cnt,
                    1);

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

static char *strupr(char *str)
{
    char *orign=str;
    for (; *str!='\0'; str++)
        *str = toupper(*str);
    return orign;
}

static char *strlowr(char *str)
{
    char *orign=str;
    for (; *str!='\0'; str++)
        *str = tolower(*str);
    return orign;
}

XCamReturn
AiqCommonHandler::processMiscMetaResults(struct CamIA10_Results &ia10_results, X3aResultList &output, bool first)
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
    int en = _aiq_compositor->getAiqInputParams().ptr() ? _aiq_compositor->getAiqInputParams()->tuningFlag : 0;
    if(en){
        processTuningToolModuleInfoMetaResults(metadata);
        processTuningToolSensorInfoMetaResults(metadata);
        processTuningToolProtocolInfoMetaResults(metadata);
        processTuningToolBlsMetaResults(metadata, ia10_results);
        processTuningToolLscMetaResults(metadata, ia10_results);
        processTuningToolCcmMetaResults(metadata, ia10_results);
        processTuningToolAwbMetaResults(metadata, ia10_results);
        processTuningToolAwbWpMetaResults(metadata, ia10_results);
        processTuningToolAwbCurvMetaResults(metadata);
        processTuningToolAwbRefGainMetaResults(metadata, ia10_results);
        processTuningToolGocMetaResults(metadata, ia10_results);
        processTuningToolCprocMetaResults(metadata, ia10_results);
        processTuningToolDpfMetaResults(metadata, ia10_results);
        processTuningToolFltMetaResults(metadata, ia10_results);
    }
    processExifMakernote(metadata, ia10_results);
    // Update reqId for the result in order to match the setting param
    int reqId = _aiq_compositor->getAiqInputParams().ptr() ? _aiq_compositor->getAiqInputParams()->reqId : -1;
    metadata->update(ANDROID_REQUEST_ID, &reqId, 1);

    // set in _ae_handler->processAeMetaResults, called before
    // processMiscMetaResults
    entry = metadata->find(RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_NEEDED);
    if (entry.count == 1) {
        _stillcap_sync_needed = !!entry.data.u8[0];
    }

    // update flash states
    CameraMetadata* staticMeta  =
        _aiq_compositor->getAiqInputParams()->staticMeta;
    entry = staticMeta->find(ANDROID_FLASH_INFO_AVAILABLE);
    if (entry.count == 1) {
        if (entry.data.u8[0] == ANDROID_FLASH_INFO_AVAILABLE_TRUE) {
            const CameraMetadata* settings  =
                &_aiq_compositor->getAiqInputParams()->settings;
            uint8_t flash_mode = ANDROID_FLASH_MODE_OFF;
            camera_metadata_ro_entry entry_flash =
                settings->find(ANDROID_FLASH_MODE);

            if (entry_flash.count == 1) {
                flash_mode = entry_flash.data.u8[0];
            }
            metadata->update(ANDROID_FLASH_MODE, &flash_mode, 1);

            uint8_t flashState = ANDROID_FLASH_STATE_READY;

            struct CamIA10_Stats& camia10_stats =
                _aiq_compositor->get_3a_ia10_stats ();

            if (camia10_stats.frame_status == CAMIA10_FRAME_STATUS_FLASH_EXPOSED ||
                camia10_stats.flash_status.flash_mode == HAL_FLASH_TORCH ||
                /* CTS required */
                flash_mode == ANDROID_FLASH_MODE_SINGLE||
                flash_mode == ANDROID_FLASH_MODE_TORCH)
                flashState = ANDROID_FLASH_STATE_FIRED;
            else if (camia10_stats.frame_status == CAMIA10_FRAME_STATUS_FLASH_PARTIAL)
                flashState = ANDROID_FLASH_STATE_PARTIAL;
            metadata->update(ANDROID_FLASH_STATE, &flashState, 1);
            entry = staticMeta->find(ANDROID_FLASH_INFO_AVAILABLE);
            // sync needed and main flash on
            if ((_stillcap_sync_state == STILLCAP_SYNC_STATE_START &&
                /* camia10_stats.flash_status.flash_mode == HAL_FLASH_MAIN && */
                camia10_stats.frame_status == CAMIA10_FRAME_STATUS_FLASH_EXPOSED) ||
                first) {
                uint8_t stillcap_sync = RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_CMD_SYNCDONE;
                metadata->update(RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_CMD, &stillcap_sync, 1);
                _stillcap_sync_state = STILLCAP_SYNC_STATE_WAITING_END;
                LOGD("%s:%d, stillcap_sync done", __FUNCTION__, __LINE__);
            }
        }
    }

    return ret;
}

void
AiqCommonHandler::processTuningToolModuleInfoMetaResults(CameraMetadata* metadata)
{
     uint8_t moduleinfo[234], *pchr;
     char sensornam[32], modulenam[32], lensnam[32], *pstr,*pstart, *pend;

     memset(sensornam,0,sizeof(sensornam));
     memset(modulenam,0,sizeof(modulenam));
     memset(lensnam,0,sizeof(lensnam));
     memset(moduleinfo, 0, sizeof(moduleinfo));
     pchr = moduleinfo;
     memcpy(pchr, _iq_name, strlen(_iq_name));
     pchr += 64;
     pstr = strdup(_iq_name);
     pstart = strrchr(pstr,'/');
     pend = strrchr(pstr,'.');
     if(pstart == NULL || pend == NULL){
        return;
     }
     *pend = 0;
     sscanf(pstart+1,"%[^_]_%[^_]_%s",sensornam,modulenam,lensnam);
     memcpy(pchr, sensornam, sizeof(sensornam));
     pchr += sizeof(sensornam);
     memcpy(pchr, modulenam, sizeof(modulenam));
     pchr += sizeof(modulenam);
     memcpy(pchr, lensnam, sizeof(lensnam));
     pchr += sizeof(lensnam);
     *pchr++ = _otp_info.awb.enable|_otp_info.lsc.enable<<1;
     memcpy(pchr, &_otp_info.awb.golden_r_value, sizeof(_otp_info.awb.golden_r_value));
     pchr += sizeof(_otp_info.awb.golden_r_value);
     memcpy(pchr, &_otp_info.awb.golden_gr_value, sizeof(_otp_info.awb.golden_gr_value));
     pchr += sizeof(_otp_info.awb.golden_gr_value);
     memcpy(pchr, &_otp_info.awb.golden_gb_value, sizeof(_otp_info.awb.golden_gb_value));
     pchr += sizeof(_otp_info.awb.golden_gb_value);
     memcpy(pchr, &_otp_info.awb.golden_b_value, sizeof(_otp_info.awb.golden_b_value));
     pchr += sizeof(_otp_info.awb.golden_b_value);
     metadata->update(RKCAMERA3_PRIVATEDATA_ISP_MODULE_INFO,(uint8_t*)moduleinfo,sizeof(moduleinfo));
}

void
AiqCommonHandler::processTuningToolProtocolInfoMetaResults(CameraMetadata* metadata)
{
     uint8_t protocolinfo[4];
     uint32_t magicCode;
     magicCode = _aiq_compositor->_isp10_engine->getCalibdbMagicVerCode();
     memset(protocolinfo, 0, sizeof(protocolinfo));
     memcpy(protocolinfo, &magicCode, sizeof(magicCode));
     metadata->update(RKCAMERA3_PRIVATEDATA_ISP_PROTOCOL_INFO,(uint8_t*)protocolinfo,sizeof(protocolinfo));
}

void
AiqCommonHandler::processTuningToolSensorInfoMetaResults(CameraMetadata* metadata)
{
    CamCalibDbHandle_t hCalib;
    CamCalibDbMetaData_t meta;
    struct CamIA10_SensorModeData &sensor_desc = _aiq_compositor->get_sensor_mode_data();
    uint8_t sensor_info[12];
    short tempval;

    memset(sensor_info, 0, sizeof(sensor_info));
    sensor_info[0] = 0;//mirror flip
    tempval = (short)((sensor_desc.line_periods_per_field)&0xffff);
    memcpy(&sensor_info[1],&tempval,2);
    tempval = (short)((sensor_desc.pixel_periods_per_line)&0xffff);
    memcpy(&sensor_info[3],&tempval,2);
    memcpy(&sensor_info[5], &sensor_desc.pixel_clock_freq_mhz, 4);
    sensor_info[9] = 0;//bining or full
    _aiq_compositor->_isp10_engine->getCalibdbHandle(&hCalib);
    CamCalibDbGetMetaData(hCalib, &meta);
    if(meta.isp_output_type == isp_gray_output_type)
        sensor_info[10] = 1;
    else
        sensor_info[10] = 0;
    metadata->update(RKCAMERA3_PRIVATEDATA_ISP_SENSOR_INFO,(uint8_t*)sensor_info,sizeof(sensor_info));

}

void
AiqCommonHandler::processTuningToolBlsMetaResults(CameraMetadata* metadata, struct CamIA10_Results &ia10_results)
{
    uint8_t blc_param[30];
    uint8_t *pbuf;
    memset(blc_param, 0, sizeof(blc_param));
    pbuf = blc_param;
    blc_param[0] = ia10_results.bls.enabled;
    pbuf++;
    blc_param[1] = _aiq_compositor->tool_isp_params.bls_config.enable_auto;
    pbuf++;
    if(blc_param[1]){
        if(_aiq_compositor->tool_isp_params.bls_config.en_windows == 0)
            blc_param[2] = 1;
        else if(_aiq_compositor->tool_isp_params.bls_config.en_windows == 1)
            blc_param[2] = 2;
    }else{
        blc_param[2] = 0;
    }

    pbuf++;

    memcpy(pbuf, &_aiq_compositor->tool_isp_params.bls_config.bls_window1.h_offs, 2);
    pbuf += 2;
    memcpy(pbuf, &_aiq_compositor->tool_isp_params.bls_config.bls_window1.v_offs, 2);
    pbuf += 2;
    memcpy(pbuf, &_aiq_compositor->tool_isp_params.bls_config.bls_window1.h_size, 2);
    pbuf += 2;
    memcpy(pbuf, &_aiq_compositor->tool_isp_params.bls_config.bls_window1.v_size, 2);
    pbuf += 2;

    memcpy(pbuf, &_aiq_compositor->tool_isp_params.bls_config.bls_window2.h_offs, 2);
    pbuf += 2;
    memcpy(pbuf, &_aiq_compositor->tool_isp_params.bls_config.bls_window2.v_offs, 2);
    pbuf += 2;
    memcpy(pbuf, &_aiq_compositor->tool_isp_params.bls_config.bls_window2.h_size, 2);
    pbuf += 2;
    memcpy(pbuf, &_aiq_compositor->tool_isp_params.bls_config.bls_window2.v_size, 2);
    pbuf += 2;

    *pbuf = _aiq_compositor->tool_isp_params.bls_config.bls_samples;
    pbuf++;
    memcpy(pbuf, &_aiq_compositor->tool_isp_params.bls_config.fixed_val.b,2);
    pbuf += 2;
    memcpy(pbuf, &_aiq_compositor->tool_isp_params.bls_config.fixed_val.gb,2);
    pbuf += 2;
    memcpy(pbuf, &_aiq_compositor->tool_isp_params.bls_config.fixed_val.gr,2);
    pbuf += 2;
    memcpy(pbuf, &_aiq_compositor->tool_isp_params.bls_config.fixed_val.r,2);
    metadata->update(RKCAMERA3_PRIVATEDATA_ISP_BLS,(uint8_t*)blc_param,sizeof(blc_param));

}

void
AiqCommonHandler::processTuningToolLscMetaResults(CameraMetadata* metadata, struct CamIA10_Results &ia10_results)
{
     CamLscProfileName_t lscProfileName;
     CamLscProfile_t *plsc = NULL;
     uint8_t lsc_param[2404], *pbuf;

    CamCalibDbHandle_t hCalib;
    _aiq_compositor->_isp10_engine->getCalibdbHandle(&hCalib);
    memset(lsc_param, 0, sizeof(lsc_param));

    pbuf = lsc_param;
    if (strcmp((char*)ia10_results.awb.LscNameUp, "null"))
        strcpy((char*)lscProfileName, (char*)ia10_results.awb.LscNameUp);
    else if (strcmp((char*)ia10_results.awb.LscNameDn, "null"))
        strcpy((char*)lscProfileName, (char*)ia10_results.awb.LscNameDn);

    CamCalibDbGetLscProfileByName(hCalib, lscProfileName, &plsc);
    if (plsc != NULL)
    {
        pbuf[0] = ia10_results.lsc_enabled;
        pbuf++;
        memcpy(pbuf, ia10_results.awb.LscNameUp, sizeof(ia10_results.awb.LscNameUp));
        pbuf += sizeof(plsc->name);
        memcpy(pbuf, ia10_results.awb.LscNameDn, sizeof(ia10_results.awb.LscNameDn));
        pbuf += sizeof(plsc->name);
        *((uint16_t*)pbuf) = plsc->LscSectors;
        pbuf += 2;
        *((uint16_t*)pbuf) = plsc->LscNo;
        pbuf += 2;
        *((uint16_t*)pbuf) = plsc->LscXo;
        pbuf += 2;
        *((uint16_t*)pbuf) = plsc->LscYo;
        pbuf += 2;
        memcpy(pbuf, plsc->LscXSizeTbl, sizeof(plsc->LscXSizeTbl));
        pbuf += sizeof(plsc->LscXSizeTbl);
        memcpy(pbuf, plsc->LscYSizeTbl, sizeof(plsc->LscYSizeTbl));
        pbuf += sizeof(plsc->LscYSizeTbl);
        memcpy(pbuf, plsc->LscMatrix, sizeof(plsc->LscMatrix));
        metadata->update(RKCAMERA3_PRIVATEDATA_ISP_LSC_GET, (uint8_t*)lsc_param, sizeof(lsc_param));
    }
}

void
AiqCommonHandler::processTuningToolCcmMetaResults(CameraMetadata* metadata, struct CamIA10_Results &ia10_results)
{
    uint8_t ccm_param[100], *pbuf;

    memset(ccm_param, 0, sizeof(ccm_param));
    pbuf = ccm_param;
    //pbuf[0] = ia10_results.ctk_enabled;
    pbuf++;
    memcpy(pbuf, ia10_results.awb.CcNameUp, sizeof(ia10_results.awb.CcNameUp));
    pbuf += sizeof(ia10_results.awb.CcNameUp);
    memcpy(pbuf, ia10_results.awb.CcNameDn, sizeof(ia10_results.awb.CcNameDn));
    pbuf += sizeof(ia10_results.awb.CcNameDn);
#if 0
    float fVal[12];

    fVal[0] = UtlFixToFloat_S0407(ia10_results.awb.CcMatrix.Coeff[0]);
    memcpy(pbuf, &fVal[0], sizeof(fVal[0]));
    pbuf += sizeof(fVal[0]);
    fVal[1] = UtlFixToFloat_S0407(ia10_results.awb.CcMatrix.Coeff[1]);
    memcpy(pbuf, &fVal[1], sizeof(fVal[1]));
    pbuf += sizeof(fVal[1]);
    fVal[2] = UtlFixToFloat_S0407(ia10_results.awb.CcMatrix.Coeff[2]);
    memcpy(pbuf, &fVal[2], sizeof(fVal[2]));
    pbuf += sizeof(fVal[2]);
    fVal[3] = UtlFixToFloat_S0407(ia10_results.awb.CcMatrix.Coeff[3]);
    memcpy(pbuf, &fVal[3], sizeof(fVal[3]));
    pbuf += sizeof(fVal[3]);
    fVal[4] = UtlFixToFloat_S0407(ia10_results.awb.CcMatrix.Coeff[4]);
    memcpy(pbuf, &fVal[4], sizeof(fVal[4]));
    pbuf += sizeof(fVal[4]);
    fVal[5] = UtlFixToFloat_S0407(ia10_results.awb.CcMatrix.Coeff[5]);
    memcpy(pbuf, &fVal[5], sizeof(fVal[5]));
    pbuf += sizeof(fVal[5]);
    fVal[6] = UtlFixToFloat_S0407(ia10_results.awb.CcMatrix.Coeff[6]);
    memcpy(pbuf, &fVal[6], sizeof(fVal[6]));
    pbuf += sizeof(fVal[6]);
    fVal[7] = UtlFixToFloat_S0407(ia10_results.awb.CcMatrix.Coeff[7]);
    memcpy(pbuf, &fVal[7], sizeof(fVal[7]));
    pbuf += sizeof(fVal[7]);
    fVal[8] = UtlFixToFloat_S0407(ia10_results.awb.CcMatrix.Coeff[8]);
    memcpy(pbuf, &fVal[8], sizeof(fVal[8]));
    pbuf += sizeof(fVal[8]);
    fVal[9] = UtlFixToFloat_S1200(ia10_results.awb.CcOffset.Red);
    memcpy(pbuf, &fVal[9], sizeof(fVal[9]));
    pbuf += sizeof(fVal[9]);
    fVal[10] = UtlFixToFloat_S1200(ia10_results.awb.CcOffset.Green);
    memcpy(pbuf, &fVal[10], sizeof(fVal[10]));
    pbuf += sizeof(fVal[10]);
    fVal[11] = UtlFixToFloat_S1200(ia10_results.awb.CcOffset.Blue);
    memcpy(pbuf, &fVal[11], sizeof(fVal[11]));
    pbuf += sizeof(fVal[11]);
    if((fVal[0]==1.0) && (fVal[1]==0.0) && (fVal[2]==0.0) && (fVal[3]==0.0) &&
       (fVal[4]==1.0) && (fVal[5]==0.0) && (fVal[6]==0.0) && (fVal[7]==0.0) && (fVal[8]==1.0))
    {
        ccm_param[0] = 0;
    }else{
        ccm_param[0] = 1;
    }

#else
    CamCcProfile_t*  pCcProfile = NULL;
    CamCalibDbHandle_t hCalib;
    CamCcProfileName_t  name;
    float *fVal;

    if(strcmp(ia10_results.awb.CcNameUp, "null"))
        strcpy(name, ia10_results.awb.CcNameUp);
    else if(strcmp(ia10_results.awb.CcNameDn, "null"))
        strcpy(name, ia10_results.awb.CcNameDn);

    _aiq_compositor->_isp10_engine->getCalibdbHandle(&hCalib);
    CamCalibDbGetCcProfileByName(hCalib, name, &pCcProfile);
    if(pCcProfile){
        memcpy(pbuf, pCcProfile->CrossTalkCoeff.fCoeff, sizeof(pCcProfile->CrossTalkCoeff.fCoeff));
        pbuf += sizeof(pCcProfile->CrossTalkCoeff.fCoeff);
        memcpy(pbuf, pCcProfile->CrossTalkOffset.fCoeff, sizeof(pCcProfile->CrossTalkOffset.fCoeff));
        pbuf += sizeof(pCcProfile->CrossTalkOffset.fCoeff);
        fVal = pCcProfile->CrossTalkCoeff.fCoeff;
        if((fVal[0]==1.0) && (fVal[1]==0.0) && (fVal[2]==0.0) && (fVal[3]==0.0) &&
           (fVal[4]==1.0) && (fVal[5]==0.0) && (fVal[6]==0.0) && (fVal[7]==0.0) && (fVal[8]==1.0))
        {
            ccm_param[0] = 0;
        }else{
            ccm_param[0] = 1;
        }
    }
#endif

    metadata->update(RKCAMERA3_PRIVATEDATA_ISP_CCM_GET, (uint8_t*)ccm_param, sizeof(ccm_param));
}

void
AiqCommonHandler::processTuningToolAwbMetaResults(CameraMetadata* metadata, struct CamIA10_Results &ia10_results)
{
    uint8_t awb_param[39], *pbuf;

    memset(awb_param, 0, sizeof(awb_param));
    pbuf = awb_param;
    *pbuf++ = (ia10_results.awb.forceWbGainFlag==BOOL_TRUE) ? 0 : 1;
    memcpy(pbuf, &ia10_results.awb.forceWbGains, sizeof(ia10_results.awb.forceWbGains));
    pbuf += sizeof(ia10_results.awb.forceWbGains);
    *pbuf++ = (ia10_results.awb.forceIlluFlag==BOOL_TRUE) ? 1 : 0;
    strcpy((char*)pbuf, ia10_results.awb.forceIllName);
    metadata->update(RKCAMERA3_PRIVATEDATA_ISP_AWB_GET, (uint8_t*)awb_param, sizeof(awb_param));
}

void
AiqCommonHandler::processTuningToolAwbWpMetaResults(CameraMetadata* metadata, struct CamIA10_Results &ia10_results)
{
    struct cifisp_stat_buffer& isp_stat = _aiq_compositor->get_3a_isp_stats();

    uint8_t awb_wp[53], *pbuf;
    short tempval;
    memset(awb_wp, 0, sizeof(awb_wp));
    pbuf = awb_wp;
    *((uint16_t *)pbuf) = _aiq_compositor->tool_isp_params.awb_meas_config.awb_wnd.h_offs;
    pbuf += 2;
    *((uint16_t *)pbuf) = _aiq_compositor->tool_isp_params.awb_meas_config.awb_wnd.v_offs;
    pbuf += 2;
    *((uint16_t *)pbuf) = _aiq_compositor->tool_isp_params.awb_meas_config.awb_wnd.h_size;
    pbuf += 2;
    *((uint16_t *)pbuf) = _aiq_compositor->tool_isp_params.awb_meas_config.awb_wnd.v_size;
    pbuf += 2;
    if(_aiq_compositor->tool_isp_params.awb_meas_config.awb_mode == CIFISP_AWB_MODE_RGB)
        *pbuf++ = 0;
    else if(_aiq_compositor->tool_isp_params.awb_meas_config.awb_mode == CIFISP_AWB_MODE_YCBCR)
        *pbuf++ = 1;
    else
        *pbuf++ = 1;
    memcpy(pbuf, &isp_stat.params.awb.awb_mean[0].cnt, sizeof(isp_stat.params.awb.awb_mean[0].cnt));
    pbuf += sizeof(isp_stat.params.awb.awb_mean[0].cnt);
    *pbuf++ = isp_stat.params.awb.awb_mean[0].mean_y_or_g;//mean y_g
    *pbuf++ = isp_stat.params.awb.awb_mean[0].mean_cb_or_b;//mean cb
    *pbuf++ = isp_stat.params.awb.awb_mean[0].mean_cr_or_r;//mean cr
    tempval = isp_stat.params.awb.awb_mean[0].mean_cr_or_r;
    memcpy(pbuf, &tempval, sizeof(tempval));
    pbuf += sizeof(tempval);
    tempval = isp_stat.params.awb.awb_mean[0].mean_cb_or_b;
    memcpy(pbuf, &tempval, sizeof(tempval));
    pbuf += sizeof(tempval);
    tempval = isp_stat.params.awb.awb_mean[0].mean_y_or_g;
    memcpy(pbuf, &tempval, sizeof(tempval));
    pbuf += sizeof(tempval);
    *pbuf++ = _aiq_compositor->tool_isp_params.awb_meas_config.awb_ref_cr;
    *pbuf++ = _aiq_compositor->tool_isp_params.awb_meas_config.awb_ref_cb;
    *pbuf++ = _aiq_compositor->tool_isp_params.awb_meas_config.min_y;
    *pbuf++ = _aiq_compositor->tool_isp_params.awb_meas_config.max_y;
    *pbuf++ = _aiq_compositor->tool_isp_params.awb_meas_config.min_c;
    *pbuf++ = _aiq_compositor->tool_isp_params.awb_meas_config.max_csum;
    memcpy(pbuf, &ia10_results.awb.RgProj, 4);//RgProj
    pbuf += 4;
    memcpy(pbuf, &ia10_results.awb.RegionSize, 4);//RegionSize
    pbuf += 4;
    memcpy(pbuf, &ia10_results.awb.WbClippedGainsOverG.GainROverG, 4);//wbClipGainOver.GainROverG
    pbuf += 4;
    memcpy(pbuf, &ia10_results.awb.WbGainsOverG.GainROverG, 4);//wbGainOver.GainROverG
    pbuf += 4;
    memcpy(pbuf, &ia10_results.awb.WbClippedGainsOverG.GainBOverG, 4);//wbClipGainOver.GainBOverG
    pbuf += 4;
    memcpy(pbuf, &ia10_results.awb.WbGainsOverG.GainBOverG, 4);//wbGainOver.GainBOverG
    metadata->update(RKCAMERA3_PRIVATEDATA_ISP_AWB_WP, (uint8_t*)awb_wp, sizeof(awb_wp));
}

void
AiqCommonHandler::processTuningToolAwbCurvMetaResults(CameraMetadata* metadata)
{
    uint8_t awb_cur[530], *pbuf;
    CamCalibDbHandle_t hCalib;
    CamCalibAwb_V11_Global_t* pAwbGlobal;
    char cur_resolution[15];

    memset(awb_cur, 0, sizeof(awb_cur));
    pbuf = awb_cur;
    _aiq_compositor->_isp10_engine->getCalibdbHandle(&hCalib);
    struct CamIA10_SensorModeData &sensor_mode = _aiq_compositor->get_sensor_mode_data();
    sprintf(cur_resolution,"%dx%d",sensor_mode.sensor_output_width,sensor_mode.sensor_output_height);

    CamCalibDbGetAwb_V11_GlobalByResolution(hCalib, cur_resolution, &pAwbGlobal);
    if (pAwbGlobal){
        memcpy(pbuf,  &pAwbGlobal->CenterLine, sizeof(pAwbGlobal->CenterLine));
        pbuf += sizeof(pAwbGlobal->CenterLine);
        *((float*)pbuf) = pAwbGlobal->KFactor.fCoeff[0];
        pbuf += 4;
        memcpy(pbuf, pAwbGlobal->AwbClipParam.pRg1, pAwbGlobal->AwbClipParam.ArraySize1*4);
        pbuf += pAwbGlobal->AwbClipParam.ArraySize1*4;
        memcpy(pbuf, pAwbGlobal->AwbClipParam.pMaxDist1, pAwbGlobal->AwbClipParam.ArraySize1*4);
        pbuf += pAwbGlobal->AwbClipParam.ArraySize1*4;
        memcpy(pbuf, pAwbGlobal->AwbClipParam.pRg2, pAwbGlobal->AwbClipParam.ArraySize2*4);
        pbuf += pAwbGlobal->AwbClipParam.ArraySize2*4;
        memcpy(pbuf, pAwbGlobal->AwbClipParam.pMaxDist2, pAwbGlobal->AwbClipParam.ArraySize2*4);
        pbuf += pAwbGlobal->AwbClipParam.ArraySize2*4;
        memcpy(pbuf, pAwbGlobal->AwbGlobalFadeParm.pGlobalFade1, pAwbGlobal->AwbGlobalFadeParm.ArraySize1*4);
        pbuf += pAwbGlobal->AwbGlobalFadeParm.ArraySize1*4;
        memcpy(pbuf, pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance1, pAwbGlobal->AwbGlobalFadeParm.ArraySize1*4);
        pbuf += pAwbGlobal->AwbGlobalFadeParm.ArraySize1*4;
        memcpy(pbuf, pAwbGlobal->AwbGlobalFadeParm.pGlobalFade2, pAwbGlobal->AwbGlobalFadeParm.ArraySize2*4);
        pbuf += pAwbGlobal->AwbGlobalFadeParm.ArraySize2*4;
        memcpy(pbuf, pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance2, pAwbGlobal->AwbGlobalFadeParm.ArraySize2*4);
        pbuf += pAwbGlobal->AwbGlobalFadeParm.ArraySize2*4;
        metadata->update(RKCAMERA3_PRIVATEDATA_ISP_AWB_CURV, (uint8_t*)awb_cur, sizeof(awb_cur));
    }
}

void
AiqCommonHandler::processTuningToolAwbRefGainMetaResults(CameraMetadata* metadata, struct CamIA10_Results &ia10_results)
{
    uint8_t awb_ref_gain_param[38], *pbuf;

    memset(awb_ref_gain_param, 0, sizeof(awb_ref_gain_param));
    pbuf = awb_ref_gain_param;
    memcpy(pbuf, &ia10_results.awb.curIllName, sizeof(ia10_results.awb.curIllName));
    pbuf += sizeof(ia10_results.awb.curIllName);
    memcpy(pbuf, &ia10_results.awb.refWbgain, sizeof(ia10_results.awb.refWbgain));
    metadata->update(RKCAMERA3_PRIVATEDATA_ISP_AWB_REFGAIN, (uint8_t*)awb_ref_gain_param, sizeof(awb_ref_gain_param));
}

void
AiqCommonHandler::processTuningToolGocMetaResults(CameraMetadata* metadata, struct CamIA10_Results &ia10_results)
{
    CamGOCProfileName_t goc_name[2] =
    {
        "normal",
        "night"
    };
    uint8_t  goc_param[92], *pbuf;
    CamCalibDbHandle_t hCalib;
    CamGOCProfileName_t name;
    CamCalibGocProfile_t* pGocProfile;
    memset(goc_param, 0, sizeof(goc_param));
    _aiq_compositor->_isp10_engine->getCalibdbHandle(&hCalib);
    for (uint8_t i=0; i<2; i++) {
        pbuf = goc_param;
        CamCalibDbGetGocProfileByName(hCalib, strupr(goc_name[i]), &pGocProfile);
        if (pGocProfile){
            pbuf[0] = (uint8_t)ia10_results.goc.enabled;
            pbuf++;
            memcpy(pbuf, pGocProfile->name, sizeof(pGocProfile->name));
            pbuf += sizeof(pGocProfile->name);
            *pbuf++ = (uint8_t)ia10_results.wdr.enabled;//wdr status;
            *pbuf++ = (uint8_t)pGocProfile->def_cfg_mode;
            memcpy(pbuf, pGocProfile->GammaY, sizeof(pGocProfile->GammaY));
            metadata->update(RKCAMERA3_PRIVATEDATA_ISP_GOC_NORMAL+i,(uint8_t*)goc_param,sizeof(goc_param));
        }
    }

}

void
AiqCommonHandler::processTuningToolCprocMetaResults(CameraMetadata* metadata, struct CamIA10_Results &ia10_results)
{
    uint8_t cproc_param[16], *pbuf;
    float temp;
    uint32_t temp32;
    memset(cproc_param, 0, sizeof(cproc_param));
    pbuf = cproc_param;
    *pbuf++ = ia10_results.cproc.enabled;
    *pbuf++ = 0;
    temp = ((float)ia10_results.cproc.contrast)/128.0f;
    memcpy(pbuf, &temp, 4);
    pbuf += 4;
    temp = ((float)((ia10_results.cproc.hue) * 90) / 128.0f);
    memcpy(pbuf, &temp, 4);
    pbuf += 4;
    temp = ((float)ia10_results.cproc.saturation)/128.0f;
    memcpy(pbuf, &temp, 4);
    pbuf += 4;
    temp32 = (uint32_t)(ia10_results.cproc.brightness);
    if ((temp32 & 0x0080) == 0) {
        *pbuf = ia10_results.cproc.brightness;
    } else {
        temp32 |= ~0x0080;
        temp32--;
        temp32 = ~temp32;
        temp = (float)temp32;
        temp = -temp;
        *pbuf = (int8_t)temp;
    }
    metadata->update(RKCAMERA3_PRIVATEDATA_ISP_CPROC_PREVIEW,(uint8_t*)cproc_param,sizeof(cproc_param));
}

void
AiqCommonHandler::processTuningToolDpfMetaResults(CameraMetadata* metadata, struct CamIA10_Results &ia10_results)
{
    uint8_t dpf_param[85], *pbuf;
    CamDpfProfile_t*  pDpfProfile;
    CamCalibDbHandle_t hCalib;
    char cur_resolution[20];
    struct CamIA10_SensorModeData &sensor_mode = _aiq_compositor->get_sensor_mode_data();

    sprintf(cur_resolution,"%dx%d",sensor_mode.sensor_output_width,sensor_mode.sensor_output_height);
    _aiq_compositor->_isp10_engine->getCalibdbHandle(&hCalib);
    memset(dpf_param, 0, sizeof(dpf_param));

    pbuf = dpf_param;
    CamCalibDbGetDpfProfileByResolution(hCalib, strlowr(cur_resolution), &pDpfProfile);
    if (pDpfProfile) {
        memcpy(pbuf, pDpfProfile->resolution, sizeof(pDpfProfile->resolution));
        pbuf += 20;
        *pbuf++ = ia10_results.adpf_enabled;//pDpfProfile->ADPFEnable;
        *pbuf++ = pDpfProfile->nll_segmentation;
        memcpy(pbuf, pDpfProfile->nll_coeff.uCoeff, sizeof(pDpfProfile->nll_coeff));
        pbuf += sizeof(pDpfProfile->nll_coeff);
        *((uint16_t *)pbuf) = pDpfProfile->SigmaGreen;
        pbuf +=2;
        *((uint16_t *)pbuf) = pDpfProfile->SigmaRedBlue;
        pbuf +=2;
        memcpy(pbuf, &pDpfProfile->fGradient, 4);
        pbuf +=4;
        memcpy(pbuf, &pDpfProfile->fOffset, 4);
        pbuf +=4;
        memcpy(pbuf, &pDpfProfile->NfGains.fCoeff[0], 4);
        pbuf +=4;
        memcpy(pbuf, &pDpfProfile->NfGains.fCoeff[1], 4);
        pbuf +=4;
        memcpy(pbuf, &pDpfProfile->NfGains.fCoeff[2], 4);
        pbuf +=4;
        memcpy(pbuf, &pDpfProfile->NfGains.fCoeff[3], 4);
        metadata->update(RKCAMERA3_PRIVATEDATA_ISP_DPF_GET,(uint8_t*)dpf_param,sizeof(dpf_param));
    }
}

void
AiqCommonHandler::processTuningToolFltMetaResults(CameraMetadata* metadata, struct CamIA10_Results &ia10_results)
{
    CamFilterProfileName_t flt_name[2] = {"normal","night"};
    uint8_t flt_param[250], *pbuf;
    CamFilterProfile_t* pFilterProfile;
    CamDpfProfile_t*    pDpfProfile;
    CamCalibDbHandle_t  hCalib;
    _aiq_compositor->_isp10_engine->getCalibdbHandle(&hCalib);
    char cur_resolution[20];
    struct CamIA10_SensorModeData &sensor_mode = _aiq_compositor->get_sensor_mode_data();
    memset(cur_resolution, 0, sizeof(cur_resolution));
    sprintf(cur_resolution,"%dx%d",sensor_mode.sensor_output_width,sensor_mode.sensor_output_height);

    CamCalibDbGetDpfProfileByResolution(hCalib, strlowr(cur_resolution), &pDpfProfile);
    if (pDpfProfile) {
        for(int i=0; i<2; i++) {
            pbuf = flt_param;
            memset(flt_param, 0, sizeof(flt_param));
            CamCalibDbGetFilterProfileByName(hCalib, pDpfProfile, strupr(flt_name[i]), &pFilterProfile);
            if (pFilterProfile) {
                memcpy(pbuf, cur_resolution, sizeof(cur_resolution));
                pbuf += sizeof(cur_resolution);
                *pbuf++ = ia10_results.flt.enabled;

                *pbuf++ = (uint8_t)pFilterProfile->DenoiseLevelCurve.pSensorGain[0];
                *pbuf++ = (uint8_t)pFilterProfile->DenoiseLevelCurve.pSensorGain[1];
                *pbuf++ = (uint8_t)pFilterProfile->DenoiseLevelCurve.pSensorGain[2];
                *pbuf++ = (uint8_t)pFilterProfile->DenoiseLevelCurve.pSensorGain[3];
                *pbuf++ = (uint8_t)pFilterProfile->DenoiseLevelCurve.pSensorGain[4];
                *pbuf++ = ((uint8_t)pFilterProfile->DenoiseLevelCurve.pDlevel[0]-1);
                *pbuf++ = ((uint8_t)pFilterProfile->DenoiseLevelCurve.pDlevel[1]-1);
                *pbuf++ = ((uint8_t)pFilterProfile->DenoiseLevelCurve.pDlevel[2]-1);
                *pbuf++ = ((uint8_t)pFilterProfile->DenoiseLevelCurve.pDlevel[3]-1);
                *pbuf++ = ((uint8_t)pFilterProfile->DenoiseLevelCurve.pDlevel[4]-1);
                *pbuf++ = (uint8_t)pFilterProfile->SharpeningLevelCurve.pSensorGain[0];
                *pbuf++ = (uint8_t)pFilterProfile->SharpeningLevelCurve.pSensorGain[1];
                *pbuf++ = (uint8_t)pFilterProfile->SharpeningLevelCurve.pSensorGain[2];
                *pbuf++ = (uint8_t)pFilterProfile->SharpeningLevelCurve.pSensorGain[3];
                *pbuf++ = (uint8_t)pFilterProfile->SharpeningLevelCurve.pSensorGain[4];
                *pbuf++ = ((uint8_t)pFilterProfile->SharpeningLevelCurve.pSlevel[0]-1);
                *pbuf++ = ((uint8_t)pFilterProfile->SharpeningLevelCurve.pSlevel[1]-1);
                *pbuf++ = ((uint8_t)pFilterProfile->SharpeningLevelCurve.pSlevel[2]-1);
                *pbuf++ = ((uint8_t)pFilterProfile->SharpeningLevelCurve.pSlevel[3]-1);
                *pbuf++ = ((uint8_t)pFilterProfile->SharpeningLevelCurve.pSlevel[4]-1);
                *pbuf++ = pFilterProfile->FiltLevelRegConf.FiltLevelRegConfEnable;

                memcpy(pbuf, pFilterProfile->FiltLevelRegConf.p_FiltLevel,pFilterProfile->FiltLevelRegConf.ArraySize);
                pbuf += pFilterProfile->FiltLevelRegConf.ArraySize;
                memcpy(pbuf,pFilterProfile->FiltLevelRegConf.p_grn_stage1,pFilterProfile->FiltLevelRegConf.ArraySize);
                pbuf += pFilterProfile->FiltLevelRegConf.ArraySize;
                memcpy(pbuf,pFilterProfile->FiltLevelRegConf.p_chr_h_mode,pFilterProfile->FiltLevelRegConf.ArraySize);
                pbuf += pFilterProfile->FiltLevelRegConf.ArraySize;
                memcpy(pbuf,pFilterProfile->FiltLevelRegConf.p_chr_v_mode,pFilterProfile->FiltLevelRegConf.ArraySize);
                pbuf += pFilterProfile->FiltLevelRegConf.ArraySize;
                memcpy(pbuf,pFilterProfile->FiltLevelRegConf.p_thresh_bl0,pFilterProfile->FiltLevelRegConf.ArraySize*4);
                pbuf += pFilterProfile->FiltLevelRegConf.ArraySize*4;
                memcpy(pbuf,pFilterProfile->FiltLevelRegConf.p_thresh_bl1,pFilterProfile->FiltLevelRegConf.ArraySize*4);
                pbuf += pFilterProfile->FiltLevelRegConf.ArraySize*4;
                memcpy(pbuf,pFilterProfile->FiltLevelRegConf.p_thresh_sh0,pFilterProfile->FiltLevelRegConf.ArraySize*4);
                pbuf += pFilterProfile->FiltLevelRegConf.ArraySize*4;
                memcpy(pbuf,pFilterProfile->FiltLevelRegConf.p_thresh_sh1,pFilterProfile->FiltLevelRegConf.ArraySize*4);
                pbuf += pFilterProfile->FiltLevelRegConf.ArraySize*4;
                memcpy(pbuf,pFilterProfile->FiltLevelRegConf.p_fac_sh1,pFilterProfile->FiltLevelRegConf.ArraySize*4);
                pbuf += pFilterProfile->FiltLevelRegConf.ArraySize*4;
                memcpy(pbuf,pFilterProfile->FiltLevelRegConf.p_fac_sh0,pFilterProfile->FiltLevelRegConf.ArraySize*4);
                pbuf += pFilterProfile->FiltLevelRegConf.ArraySize*4;
                memcpy(pbuf,pFilterProfile->FiltLevelRegConf.p_fac_mid,pFilterProfile->FiltLevelRegConf.ArraySize*4);
                pbuf += pFilterProfile->FiltLevelRegConf.ArraySize*4;
                memcpy(pbuf,pFilterProfile->FiltLevelRegConf.p_fac_bl0,pFilterProfile->FiltLevelRegConf.ArraySize*4);
                pbuf += pFilterProfile->FiltLevelRegConf.ArraySize*4;
                memcpy(pbuf,pFilterProfile->FiltLevelRegConf.p_fac_bl1,pFilterProfile->FiltLevelRegConf.ArraySize*4);
                metadata->update(RKCAMERA3_PRIVATEDATA_ISP_FLT_NORMAL+i, (uint8_t*)flt_param, sizeof(flt_param));
            }
        }
    }
}

void
AiqCommonHandler::processExifMakernote(CameraMetadata* metadata, struct CamIA10_Results &ia10_results)
{
    char makernote[600], str[64], illName[32];
    char *pbuf = makernote;
    int noIlluProfiles = 0;
    CamCalibDbHandle_t  hCalib;

    memset(makernote, 0, sizeof(makernote));
    memset(str, 0, sizeof(str));
    memset(illName, 0, sizeof(illName));
    snprintf(makernote,sizeof(makernote)-1,"magicCode=%u  Rg_Proj=%0.5f   s=%0.5f   s_max1=%0.5f  "
             "s_max2=%0.5f  Bg1=%0.5f   Rg1=%0.5f   Bg2=%0.5f   Rg2=%0.5f   colortemperature=%s   "
             "ExpPriorIn=%0.2f   ExpPriorOut=%0.2f    region=%d   ",
             _aiq_compositor->_isp10_engine->getCalibdbMagicVerCode(),ia10_results.awb.RgProj,ia10_results.awb.Wb_s,
             ia10_results.awb.Wb_s_max1,ia10_results.awb.Wb_s_max2,ia10_results.awb.Wb_bg,ia10_results.awb.Wb_rg,
             ia10_results.awb.WbClippedGainsOverG.GainBOverG,ia10_results.awb.WbClippedGainsOverG.GainROverG,
             ia10_results.awb.curIllName,ia10_results.awb.ExpPriorIn,ia10_results.awb.ExpPriorOut,ia10_results.awb.Region);

    _aiq_compositor->_isp10_engine->getCalibdbHandle(&hCalib);
    CamCalibDbGetNoOfAwbIlluminations(hCalib, &noIlluProfiles);
    for(int i=0; i<noIlluProfiles; i++)
    {
        CamCalibDbGetAwbIlluminationNameByIdx(hCalib, i, illName);
        snprintf(str,sizeof(str)-1, "illuName[%d]=%s   ", i, illName);
        strncat(makernote, str, sizeof(makernote)-strlen(makernote)-1);
        snprintf(str,sizeof(str)-1, "likehood[%d]=%0.2f   ", i, ia10_results.awb.likehood[i]);
        strncat(makernote, str, sizeof(makernote)-strlen(makernote)-1);
        snprintf(str,sizeof(str)-1, "weight[%d]=%0.2f   ", i, ia10_results.awb.weight[i]);
        strncat(makernote, str, sizeof(makernote)-strlen(makernote)-1);
    }
    metadata->update(RKCAMERA3_PRIVATEDATA_STILLCAP_ISP_PARAM, (uint8_t*)makernote, sizeof(makernote));
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
    , _stillcap_sync_needed(false)
    , _stillcap_sync_state(STILLCAP_SYNC_STATE_IDLE)
{
    initTonemaps();
    memset(&_otp_info, 0, sizeof(_otp_info));
}
AiqCommonHandler::~AiqCommonHandler ()
{
    delete[] mRGammaLut;
    delete[] mGGammaLut;
    delete[] mBGammaLut;
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
    , _delay_still_capture(false)
    , _capture_to_preview_delay(0)
    ,_flt_en_for_tool(false)
    ,_ctk_en_for_tool(false)
    ,_dpf_en_for_tool(false)
    ,_tuning_flag(false)
    ,_skip_frame(false)
    ,_procReqId(-1)
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
RKiqCompositor::set_sensor_mode_data (struct isp_supplemental_sensor_mode_data *sensor_mode,
                                      bool first)
{
    if (!_isp10_engine) {
        XCAM_LOG_ERROR ("ISP control device is null");
        return false;
    }

    if (_ae_handler && _inputParams.ptr()) {
        uint8_t new_aestate = _ae_handler->mAeState->getState();
        enum USE_CASE cur_usecase = _ia_dcfg.uc;
        enum USE_CASE new_usecase = _ia_dcfg.uc;
        AiqFrameUseCase frameUseCase = _inputParams->frameUseCase;

        if (new_aestate == ANDROID_CONTROL_AE_STATE_PRECAPTURE &&
            _inputParams->aeInputParams.aeParams.flash_mode != AE_FLASH_MODE_TORCH &&
            /* ignore the video snapshot case */
            _inputParams->aeInputParams.aeParams.flash_mode != AE_FLASH_MODE_OFF
            ) {
            new_usecase = UC_PRE_CAPTRUE;
            if (frameUseCase == AIQ_FRAME_USECASE_STILL_CAPTURE)
                _delay_still_capture = true;
        } else {
            switch (cur_usecase ) {
            case UC_PREVIEW:
                // TODO: preview to capture directly, don't change usecase now
                /* if (frameUseCase == AIQ_FRAME_USECASE_STILL_CAPTURE) */
                /*     new_usecase = UC_CAPTURE; */
                if (frameUseCase == AIQ_FRAME_USECASE_VIDEO_RECORDING)
                    new_usecase = UC_RECORDING;
                break;
            case UC_PRE_CAPTRUE:
                if ((new_aestate == ANDROID_CONTROL_AE_STATE_CONVERGED ||
                    new_aestate == ANDROID_CONTROL_AE_STATE_LOCKED ||
                    new_aestate == ANDROID_CONTROL_AE_STATE_FLASH_REQUIRED) &&
                    (frameUseCase == AIQ_FRAME_USECASE_STILL_CAPTURE ||
                     first || _delay_still_capture)) {
                    _delay_still_capture = false;
                    new_usecase = UC_CAPTURE;
                    if (_common_handler->_stillcap_sync_needed) {
                        // no need to sync for resolution changed
                        if (first)
                            _common_handler->_stillcap_sync_state =
                                AiqCommonHandler::STILLCAP_SYNC_STATE_START;
                        else
                            _common_handler->_stillcap_sync_state =
                                AiqCommonHandler::STILLCAP_SYNC_STATE_WAITING_START;
                    }
                }
                // cancel precap
                if (new_aestate == ANDROID_CONTROL_AE_STATE_INACTIVE)
                    new_usecase = UC_PREVIEW;
                break;
            case UC_CAPTURE:
                if (_common_handler->_stillcap_sync_state == AiqCommonHandler::STILLCAP_SYNC_STATE_WAITING_START &&
                    _inputParams->stillCapSyncCmd == RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_CMD_SYNCSTART)
                    _common_handler->_stillcap_sync_state =
                        AiqCommonHandler::STILLCAP_SYNC_STATE_START;

                if (/*_capture_to_preview_delay++ > 20 ||*/
                    (_common_handler->_stillcap_sync_state == AiqCommonHandler::STILLCAP_SYNC_STATE_WAITING_END &&
                    _inputParams->stillCapSyncCmd == RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_CMD_SYNCEND) ||
                    !_common_handler->_stillcap_sync_needed) {
                    _capture_to_preview_delay = 0;
                    _common_handler->_stillcap_sync_needed = 0;
                    new_usecase = UC_PREVIEW;
                    _common_handler->_stillcap_sync_state =
                        AiqCommonHandler::STILLCAP_SYNC_STATE_IDLE;
                }
                break;
            case UC_RECORDING:
                if (frameUseCase == AIQ_FRAME_USECASE_PREVIEW)
                    new_usecase = UC_PREVIEW;
                break;
            case UC_RAW:
                break;
            default:
                new_usecase = UC_PREVIEW;
                LOGE("wrong usecase %d", cur_usecase);
            }
        }
        LOGD("stats id %d, usecase %d -> %d, frameUseCase %d, new_aestate %d, "
             "stillcap_sync_needed %d, sync_cmd %d, sync_state %d",
             _isp_stats.frame_id, cur_usecase, new_usecase, frameUseCase,
             new_aestate, _common_handler->_stillcap_sync_needed,
             _inputParams->stillCapSyncCmd, _common_handler->_stillcap_sync_state);
        _ia_dcfg.uc = new_usecase;
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
        // update flash mode
        XCamAeParam* aeParam = &_inputParams->aeInputParams.aeParams;

        if (aeParam->flash_mode == AE_FLASH_MODE_AUTO)
            _ia_dcfg.flash_mode = HAL_FLASH_AUTO;
        else if (aeParam->flash_mode == AE_FLASH_MODE_ON)
            _ia_dcfg.flash_mode = HAL_FLASH_ON;
        else if (aeParam->flash_mode == AE_FLASH_MODE_TORCH)
            _ia_dcfg.flash_mode = HAL_FLASH_TORCH;
        else
            _ia_dcfg.flash_mode = HAL_FLASH_OFF;
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
    _ia_stat.effect_DomIlluIdx = isp_params.awb_algo_results.DomIlluIdx;
    memcpy(&_ia_stat.effect_CtMatrix, isp_params.awb_algo_results.fCtCoeff,
           sizeof(isp_params.awb_algo_results.fCtCoeff));
    memcpy(&_ia_stat.effect_CtOffset, isp_params.awb_algo_results.fCtOffset,
           sizeof(isp_params.awb_algo_results.fCtOffset));
    _ia_stat.stats_sof_ts = isp_params.frame_sof_ts;
    memcpy(&tool_isp_params,&isp_params, sizeof(struct rkisp_parameters));

    return true;
}

bool
RKiqCompositor::set_flash_status_info (rkisp_flash_setting_t& flash_info) {
    _ia_stat.uc = flash_info.uc;
    _ia_stat.flash_status.strobe = flash_info.strobe;
    _ia_stat.flash_status.flash_timeout_ms = flash_info.timeout_ms;
    _ia_stat.flash_status.effect_ts = flash_info.effect_ts;
    switch (flash_info.flash_mode) {
    case RKISP_FLASH_MODE_OFF :
        _ia_stat.flash_status.flash_mode = HAL_FLASH_OFF ;
        break;
    case RKISP_FLASH_MODE_TORCH:
        _ia_stat.flash_status.flash_mode = HAL_FLASH_TORCH;
        break;
    case RKISP_FLASH_MODE_FLASH_PRE:
        _ia_stat.flash_status.flash_mode = HAL_FLASH_PRE;
        break;
    case RKISP_FLASH_MODE_FLASH_MAIN:
        _ia_stat.flash_status.flash_mode = HAL_FLASH_MAIN;
        break;
    case RKISP_FLASH_MODE_FLASH:
        _ia_stat.flash_status.flash_mode = HAL_FLASH_ON;
        break;
    default:
        LOGD("not support flash mode %d", flash_info.flash_mode);
    }
    // set frame status in set_3a_stats

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

    //set flash frame status
    if ((_ia_stat.uc == UC_PRE_CAPTRUE || _ia_stat.uc == UC_CAPTURE) &&
        (_ia_stat.flash_status.flash_mode == HAL_FLASH_PRE ||
         _ia_stat.flash_status.flash_mode == HAL_FLASH_MAIN)) {

        if(_ia_stat.flash_status.flash_mode == HAL_FLASH_PRE ) {
            if (_ia_stat.flash_status.effect_ts > 0 &&
                (_ia_stat.flash_status.effect_ts + cur_exptime <= frame_ts))
                _ia_stat.frame_status = CAMIA10_FRAME_STATUS_FLASH_EXPOSED;
            else
                _ia_stat.frame_status = CAMIA10_FRAME_STATUS_FLASH_PARTIAL;
        } else if (_ia_stat.flash_status.flash_mode == HAL_FLASH_MAIN) {
            if (_ia_stat.flash_status.effect_ts > 0 &&
                (_ia_stat.flash_status.effect_ts + cur_exptime <= frame_ts) &&
               (frame_ts < _ia_stat.flash_status.effect_ts + _ia_stat.flash_status.flash_timeout_ms * 1000))
                _ia_stat.frame_status = CAMIA10_FRAME_STATUS_FLASH_EXPOSED;
            else
                _ia_stat.frame_status = CAMIA10_FRAME_STATUS_FLASH_PARTIAL;

        }
        XCAM_LOG_DEBUG ("stats id %d,frame_status: %d, effect_ts %lld, cur_exptime %f, frame_ts %lld",
            _isp_stats.frame_id,  _ia_stat.frame_status,
            _ia_stat.flash_status.effect_ts / 1000, cur_exptime / 1000, frame_ts / 1000);
    } else
        _ia_stat.frame_status = CAMIA10_FRAME_STATUS_OK;
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

void RKiqCompositor::pre_process_3A_states()
{
    if (_ae_handler && _awb_handler && _af_handler && _inputParams.ptr()) {
        // we'll use the latest inputparams if no new one is comming,
        // so should ignore the processed triggers
        if (_procReqId == _inputParams->reqId) {
            if (_inputParams->aaaControls.ae.aePreCaptureTrigger == ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER_START)
                _inputParams->aaaControls.ae.aePreCaptureTrigger = 0;
            if (_inputParams->aaaControls.af.afTrigger == ANDROID_CONTROL_AF_TRIGGER_START)
                _inputParams->aaaControls.af.afTrigger = 0;
            /* _inputParams->stillCapSyncCmd = 0; */
        } else
            _procReqId = _inputParams->reqId;
        _ae_handler->mAeState->processState(_inputParams->aaaControls.controlMode,
                                            _inputParams->aaaControls.ae);
        _awb_handler->mAwbState->processState(_inputParams->aaaControls.controlMode,
                                              _inputParams->aaaControls.awb);
        _af_handler->mAfState->processTriggers(_inputParams->aaaControls.af.afTrigger,
                                               _inputParams->aaaControls.af.afMode, 0,
                                               _inputParams->afInputParams.afParams);
    }
}

void RKiqCompositor::tuning_tool_set_bls()
{
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_bls_cfg_s isp_bls_cfg;

    if(_inputParams.ptr() && _inputParams->blsInputParams.updateFlag){
        _inputParams->blsInputParams.updateFlag = false;
        if(_inputParams->blsInputParams.enable){
            //now using fixed mode,so no need modify win
            memset(&cfg, 0, sizeof(cfg));
            cfg.bls_cfg = &isp_bls_cfg;
            cfg.bls_cfg->fixed_blue = _inputParams->blsInputParams.fixedVal.fixed_b;
            cfg.bls_cfg->fixed_greenB = _inputParams->blsInputParams.fixedVal.fixed_gb;
            cfg.bls_cfg->fixed_greenR = _inputParams->blsInputParams.fixedVal.fixed_gr;
            cfg.bls_cfg->fixed_red = _inputParams->blsInputParams.fixedVal.fixed_r;
            LOGV("bls: %d,%d,%d,%d",cfg.bls_cfg->fixed_blue,cfg.bls_cfg->fixed_greenB,cfg.bls_cfg->fixed_greenR,cfg.bls_cfg->fixed_red);
            cfg.updated_mask = HAL_ISP_BLS_MASK;
            cfg.enabled[HAL_ISP_BLS_ID] = HAL_ISP_ACTIVE_SETTING;
            _isp10_engine->configureISP(&cfg);
        }else{
            struct HAL_ISP_cfg_s cfg;
            memset(&cfg,0,sizeof(cfg));
            cfg.updated_mask = HAL_ISP_BLS_MASK;
            cfg.enabled[HAL_ISP_BLS_ID] = HAL_ISP_ACTIVE_FALSE;
            _isp10_engine->configureISP(&cfg);
        }
    }
}

void RKiqCompositor::tuning_tool_set_lsc()
{
    CamLscProfile_t lscprofile;
    CamCalibDbHandle_t hCalib;

    if(_inputParams.ptr() && _inputParams->lscInputParams.updateFlag){
        _inputParams->lscInputParams.updateFlag = false;
        if(_inputParams->lscInputParams.on){
            _isp10_engine->getCalibdbHandle(&hCalib);
            memset(&lscprofile, 0, sizeof(lscprofile));
            memcpy(lscprofile.name,_inputParams->lscInputParams.LscName,sizeof(lscprofile.name));
            lscprofile.LscSectors = _inputParams->lscInputParams.LscSectors;
            lscprofile.LscNo = _inputParams->lscInputParams.LscNo;
            lscprofile.LscXo = _inputParams->lscInputParams.LscXo;
            lscprofile.LscYo = _inputParams->lscInputParams.LscYo;
            memcpy(lscprofile.LscXSizeTbl,_inputParams->lscInputParams.LscXSizeTbl,sizeof(lscprofile.LscXSizeTbl));
            LOGD("lscXTbl:%d,%d,%d,%d,%d,%d,%d,%d",lscprofile.LscXSizeTbl[0],lscprofile.LscXSizeTbl[1],lscprofile.LscXSizeTbl[2],
            lscprofile.LscXSizeTbl[3],lscprofile.LscXSizeTbl[4],lscprofile.LscXSizeTbl[5],lscprofile.LscXSizeTbl[6],lscprofile.LscXSizeTbl[7]);
            memcpy(lscprofile.LscYSizeTbl,_inputParams->lscInputParams.LscYSizeTbl,sizeof(lscprofile.LscYSizeTbl));
            LOGD("lscYTbl:%d,%d,%d,%d,%d,%d,%d,%d",lscprofile.LscYSizeTbl[0],lscprofile.LscYSizeTbl[1],lscprofile.LscYSizeTbl[2],
            lscprofile.LscYSizeTbl[3],lscprofile.LscYSizeTbl[4],lscprofile.LscYSizeTbl[5],lscprofile.LscYSizeTbl[6],lscprofile.LscYSizeTbl[7]);
            memcpy(lscprofile.LscMatrix,_inputParams->lscInputParams.LscMatrix,sizeof(lscprofile.LscMatrix));
            LOGD("lscMatrix[0]:%d,%d,%d",lscprofile.LscMatrix[0].uCoeff[0],lscprofile.LscMatrix[0].uCoeff[1],lscprofile.LscMatrix[0].uCoeff[2]);
            LOGD("lscMatrix[1]:%d,%d,%d",lscprofile.LscMatrix[1].uCoeff[0],lscprofile.LscMatrix[1].uCoeff[1],lscprofile.LscMatrix[1].uCoeff[2]);
            LOGD("lscMatrix[2]:%d,%d,%d",lscprofile.LscMatrix[2].uCoeff[0],lscprofile.LscMatrix[2].uCoeff[1],lscprofile.LscMatrix[2].uCoeff[2]);
            LOGD("lscMatrix[3]:%d,%d,%d",lscprofile.LscMatrix[3].uCoeff[0],lscprofile.LscMatrix[3].uCoeff[1],lscprofile.LscMatrix[3].uCoeff[2]);
            if (0==strcasecmp(lscprofile.name, "all")){
                LOGD("lsc: replace all");
                CamCalibDbReplaceLscProfileAll(hCalib, &lscprofile);
            }else{
                CamLscProfile_t *plsc = NULL;
                CamCalibDbGetLscProfileByName(hCalib, lscprofile.name, &plsc);
                if(plsc){
                    LOGD("lsc: replace single %s",lscprofile.name);
                    plsc->LscSectors = lscprofile.LscSectors;
                    plsc->LscNo = lscprofile.LscNo;
                    plsc->LscXo = lscprofile.LscXo;
                    plsc->LscYo = lscprofile.LscYo;
                    memcpy(plsc->LscXSizeTbl,lscprofile.LscXSizeTbl,sizeof(lscprofile.LscXSizeTbl));
                    memcpy(plsc->LscYSizeTbl,lscprofile.LscYSizeTbl,sizeof(lscprofile.LscYSizeTbl));
                    memcpy(plsc->LscMatrix,lscprofile.LscMatrix,sizeof(lscprofile.LscMatrix));
                }
            }
            struct HAL_ISP_cfg_s cfg;
            memset(&cfg, 0, sizeof(cfg));
            cfg.updated_mask = HAL_ISP_LSC_MASK;
            cfg.enabled[HAL_ISP_LSC_ID] = HAL_ISP_ACTIVE_DEFAULT;
            _isp10_engine->configureISP(&cfg);
            _isp10_engine->setTuningToolAwbParams(NULL);
        }else{
            struct HAL_ISP_cfg_s cfg;
            memset(&cfg, 0, sizeof(cfg));
            cfg.updated_mask = HAL_ISP_LSC_MASK;
            cfg.enabled[HAL_ISP_LSC_ID] = HAL_ISP_ACTIVE_FALSE;
            _isp10_engine->configureISP(&cfg);
        }
    }
}

void RKiqCompositor::tuning_tool_set_ccm(CamIA10_AWB_Result_t &awb_results)
{
    CamCcProfile_t ccProfile;
    CamAwb_V11_IlluProfile_t illum;
    CamCalibDbHandle_t hCalib;
    char ill_name[20];
    int saturation;
    if(_inputParams.ptr() && _inputParams->ccmInputParams.updateFlag){
        _inputParams->ccmInputParams.updateFlag = false;
        if(_inputParams->ccmInputParams.on){
            memset(&ccProfile,0, sizeof(ccProfile));
            memcpy(ccProfile.name,_inputParams->ccmInputParams.name,sizeof(ccProfile.name));
            memcpy(ccProfile.CrossTalkCoeff.fCoeff,_inputParams->ccmInputParams.matrix,sizeof(ccProfile.CrossTalkCoeff));
            memcpy(ccProfile.CrossTalkOffset.fCoeff,_inputParams->ccmInputParams.offsets,sizeof(ccProfile.CrossTalkOffset));
            illum.CrossTalkCoeff = ccProfile.CrossTalkCoeff;
            illum.CrossTalkOffset = ccProfile.CrossTalkOffset;
            _isp10_engine->getCalibdbHandle(&hCalib);

            if(0==strcasecmp(ccProfile.name, "all")){
                ccProfile.saturation = 100.0;
                CamCalibDbReplaceCcProfileAll(hCalib, &ccProfile);
                CamCalibDbReplaceAwb_V11_IlluminationAll(hCalib, &illum);
            }else{
               memset(ill_name, 0, sizeof(ill_name));
               sscanf((const char *)ccProfile.name, "%[A-Z,a-z,0-9]_%d", ill_name, &saturation);
               ccProfile.saturation = (float)(saturation);
               strcpy(illum.name, ill_name);
               CamCalibDbReplaceCcProfileByName(hCalib, &ccProfile);
               CamCalibDbReplaceAwb_V11_IlluminationByName(hCalib, &illum);
            }
            #if 0
            struct HAL_ISP_cfg_s cfg;
            struct HAL_ISP_ctk_cfg_s ctk_cfg;
            memset(&cfg, 0, sizeof(cfg));
            cfg.ctk_cfg = &ctk_cfg;
            cfg.ctk_cfg->coeff0 = ccProfile.CrossTalkCoeff.fCoeff[0];
            cfg.ctk_cfg->coeff1 = ccProfile.CrossTalkCoeff.fCoeff[1];
            cfg.ctk_cfg->coeff2 = ccProfile.CrossTalkCoeff.fCoeff[2];
            cfg.ctk_cfg->coeff3 = ccProfile.CrossTalkCoeff.fCoeff[3];
            cfg.ctk_cfg->coeff4 = ccProfile.CrossTalkCoeff.fCoeff[4];
            cfg.ctk_cfg->coeff5 = ccProfile.CrossTalkCoeff.fCoeff[5];
            cfg.ctk_cfg->coeff6 = ccProfile.CrossTalkCoeff.fCoeff[6];
            cfg.ctk_cfg->coeff7 = ccProfile.CrossTalkCoeff.fCoeff[7];
            cfg.ctk_cfg->coeff8 = ccProfile.CrossTalkCoeff.fCoeff[8];
            cfg.ctk_cfg->ct_offset_r = ccProfile.CrossTalkOffset.fCoeff[0];
            cfg.ctk_cfg->ct_offset_g = ccProfile.CrossTalkOffset.fCoeff[1];
            cfg.ctk_cfg->ct_offset_b = ccProfile.CrossTalkOffset.fCoeff[2];
            for(int i=0; i<9; i++){
                _results_for_tool.awb.CcMatrix.Coeff[i] = UtlFloatToFix_S0407(ccProfile.CrossTalkCoeff.fCoeff[i]);
             }
            _results_for_tool.awb.CcOffset.Red = UtlFloatToFix_S1200(ccProfile.CrossTalkOffset.fCoeff[0]);
            _results_for_tool.awb.CcOffset.Green = UtlFloatToFix_S1200(ccProfile.CrossTalkOffset.fCoeff[1]);
            _results_for_tool.awb.CcOffset.Blue = UtlFloatToFix_S1200(ccProfile.CrossTalkOffset.fCoeff[2]);
            cfg.updated_mask = HAL_ISP_CTK_MASK;
            cfg.enabled[HAL_ISP_CTK_ID] = HAL_ISP_ACTIVE_SETTING;
            _isp10_engine->configureISP(&cfg);
            _isp10_engine->setTuningToolAwbParams(NULL);
            #else
            for(int i=0; i<9; i++){
                _results_for_tool.awb.CcMatrix.Coeff[i] = UtlFloatToFix_S0407(ccProfile.CrossTalkCoeff.fCoeff[i]);
             }
            _results_for_tool.awb.CcOffset.Red = UtlFloatToFix_S1200(ccProfile.CrossTalkOffset.fCoeff[0]);
            _results_for_tool.awb.CcOffset.Green = UtlFloatToFix_S1200(ccProfile.CrossTalkOffset.fCoeff[1]);
            _results_for_tool.awb.CcOffset.Blue = UtlFloatToFix_S1200(ccProfile.CrossTalkOffset.fCoeff[2]);


            struct HAL_ISP_cfg_s cfg;
            memset(&cfg, 0, sizeof(cfg));
            cfg.updated_mask = HAL_ISP_CTK_MASK;
            cfg.enabled[HAL_ISP_CTK_ID] = HAL_ISP_ACTIVE_DEFAULT;
            _isp10_engine->configureISP(&cfg);
            _isp10_engine->setTuningToolAwbParams(NULL);
            #endif
            _ctk_en_for_tool = true;

        }
    }
    if(_ctk_en_for_tool){
    #if 1
        for(int i=0; i<9; i++){
           _ia_results.awb.CcMatrix.Coeff[i] = _results_for_tool.awb.CcMatrix.Coeff[i];
        }
        _ia_results.awb.CcOffset.Red = _results_for_tool.awb.CcOffset.Red;
        _ia_results.awb.CcOffset.Green = _results_for_tool.awb.CcOffset.Green;
        _ia_results.awb.CcOffset.Blue = _results_for_tool.awb.CcOffset.Blue;
    #else
        struct HAL_ISP_cfg_s cfg;
        struct HAL_ISP_ctk_cfg_s ctk_cfg;
        memset(&cfg, 0, sizeof(cfg));
        cfg.ctk_cfg = &ctk_cfg;
        cfg.ctk_cfg->coeff0 = _results_for_tool.awb.CcMatrix.Coeff[0];
        cfg.ctk_cfg->coeff1 = _results_for_tool.awb.CcMatrix.Coeff[1];
        cfg.ctk_cfg->coeff2 = _results_for_tool.awb.CcMatrix.Coeff[2];
        cfg.ctk_cfg->coeff3 = _results_for_tool.awb.CcMatrix.Coeff[3];
        cfg.ctk_cfg->coeff4 = _results_for_tool.awb.CcMatrix.Coeff[4];
        cfg.ctk_cfg->coeff5 = _results_for_tool.awb.CcMatrix.Coeff[5];
        cfg.ctk_cfg->coeff6 = _results_for_tool.awb.CcMatrix.Coeff[6];
        cfg.ctk_cfg->coeff7 = _results_for_tool.awb.CcMatrix.Coeff[7];
        cfg.ctk_cfg->coeff8 = _results_for_tool.awb.CcMatrix.Coeff[8];
        cfg.ctk_cfg->ct_offset_r = _results_for_tool.awb.CcOffset.Red;
        cfg.ctk_cfg->ct_offset_g = _results_for_tool.awb.CcOffset.Green;
        cfg.ctk_cfg->ct_offset_b = _results_for_tool.awb.CcOffset.Blue;
        cfg.updated_mask = HAL_ISP_CTK_MASK;
        cfg.enabled[HAL_ISP_CTK_ID] = HAL_ISP_ACTIVE_SETTING;
        _isp10_engine->configureISP(&cfg);
    #endif
    }
}

void RKiqCompositor::tuning_tool_set_awb()
{
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_awb_gain_cfg_s awb_gian;
    AwbInstanceConfig_t temp;

    if(_inputParams.ptr() && _inputParams->awbToolInputParams.updateFlag){
            AwbConfig_t awbParam;
            memset(&awbParam, 0, sizeof(awbParam));
            _inputParams->awbToolInputParams.updateFlag = false;
            if(_inputParams->awbToolInputParams.on){
                awbParam.awbTuning.forceGainEnable = BOOL_FALSE;
                awbParam.awbTuning.forceIlluEnable = BOOL_FALSE;
            }else{
                awbParam.awbTuning.forceGainEnable = BOOL_TRUE;
                if(_inputParams->awbToolInputParams.lock_ill)
                    awbParam.awbTuning.forceIlluEnable = BOOL_TRUE;
                else
                    awbParam.awbTuning.forceIlluEnable = BOOL_FALSE;
            }
            awbParam.awbTuning.forceGains.fBlue = _inputParams->awbToolInputParams.b_gain;
            awbParam.awbTuning.forceGains.fGreenB = _inputParams->awbToolInputParams.gb_gain;
            awbParam.awbTuning.forceGains.fGreenR = _inputParams->awbToolInputParams.gr_gain;
            awbParam.awbTuning.forceGains.fRed = _inputParams->awbToolInputParams.r_gain;
            strcpy(awbParam.awbTuning.ill_name,_inputParams->awbToolInputParams.ill_name);
            awbParam.awbTuning.forceGainSet = AWB_TUNING_ENABLE;
            _isp10_engine->setTuningToolAwbParams(&awbParam);
    }
}

void RKiqCompositor::tuning_tool_set_awb_wp()
{
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_awb_meas_cfg_s awb_meas;
    CamCalibAwb_V11_Global_t *pAwbGlobal=NULL;
    CamCalibDbHandle_t hCalib;
    char cur_resolution[15];
    AwbConfig_t awbParam;
    if(_inputParams.ptr() && _inputParams->awbWpInputParams.updateFlag){
        CAM_AwbVersion_t vName;
        _inputParams->awbWpInputParams.updateFlag = false;
        _isp10_engine->getCalibdbHandle(&hCalib);
        CamCalibDbGetAwb_VersionName(hCalib, &vName);
        if(vName != CAM_AWB_VERSION_11)
            return;
        memset(&awbParam, 0, sizeof(awbParam));
        awbParam.awbTuning.forceMeasuredFlag = BOOL_TRUE;
        awbParam.awbTuning.forceMeasuredMeans.NoWhitePixel = _inputParams->awbWpInputParams.cnt;
        awbParam.awbTuning.forceMeasuredMeans.MeanCr__R = _inputParams->awbWpInputParams.mean_cr;
        awbParam.awbTuning.forceMeasuredMeans.MeanCb__B = _inputParams->awbWpInputParams.mean_cb;
        awbParam.awbTuning.forceMeasuredMeans.MeanY__G = _inputParams->awbWpInputParams.mean_y;
        awbParam.awbTuning.forceMeasSet = AWB_TUNING_ENABLE;
        _isp10_engine->setTuningToolAwbParams(&awbParam);
        LOGD("awb_wp set enter");
        struct CamIA10_SensorModeData &sensor_mode = get_sensor_mode_data();
        sprintf(cur_resolution,"%dx%d",sensor_mode.sensor_output_width,sensor_mode.sensor_output_height);
        CamCalibDbGetAwb_V11_GlobalByResolution(hCalib, cur_resolution, &pAwbGlobal);
        if(pAwbGlobal){
            for (int i = 0; i < HAL_ISP_AWBFADE2PARM_LEN; i++) {
              pAwbGlobal->AwbFade2Parm.pFade[i] = _inputParams->awbWpInputParams.afFade[i];
              #if 1
                pAwbGlobal->AwbFade2Parm.pMaxCSum_br[i] = _inputParams->awbWpInputParams.afmaxCSum_br[i];
                pAwbGlobal->AwbFade2Parm.pMaxCSum_sr[i] = _inputParams->awbWpInputParams.afmaxCSum_sr[i];
                pAwbGlobal->AwbFade2Parm.pMinC_br[i] = _inputParams->awbWpInputParams.afminC_br[i];
                pAwbGlobal->AwbFade2Parm.pMaxY_br[i] = _inputParams->awbWpInputParams.afMaxY_br[i];
                pAwbGlobal->AwbFade2Parm.pMinY_br[i] = _inputParams->awbWpInputParams.afMinY_br[i];
                pAwbGlobal->AwbFade2Parm.pMinC_sr[i] = _inputParams->awbWpInputParams.afminC_sr[i];
                pAwbGlobal->AwbFade2Parm.pMaxY_sr[i] = _inputParams->awbWpInputParams.afMaxY_sr[i];
                pAwbGlobal->AwbFade2Parm.pMinY_sr[i] = _inputParams->awbWpInputParams.afMinY_sr[i];
              #else
                pAwbGlobal->AwbFade2Parm.pCbMinRegionMax[i] = _inputParams->awbWpInputParams.afCbMinRegionMax[i];
                pAwbGlobal->AwbFade2Parm.pCrMinRegionMax[i] = _inputParams->awbWpInputParams.afCrMinRegionMax[i];
                pAwbGlobal->AwbFade2Parm.pMaxCSumRegionMax[i] = _inputParams->awbWpInputParams.afMaxCSumRegionMax[i];
                pAwbGlobal->AwbFade2Parm.pCbMinRegionMin[i] = _inputParams->awbWpInputParams.afCbMinRegionMin[i];
                pAwbGlobal->AwbFade2Parm.pCrMinRegionMin[i] = _inputParams->awbWpInputParams.afCrMinRegionMin[i];
                pAwbGlobal->AwbFade2Parm.pMaxCSumRegionMin[i] = _inputParams->awbWpInputParams.afMaxCSumRegionMin[i];
                pAwbGlobal->AwbFade2Parm.pMinCRegionMax[i] = _inputParams->awbWpInputParams.afMinCRegionMax[i];
                pAwbGlobal->AwbFade2Parm.pMinCRegionMin[i] = _inputParams->awbWpInputParams.afMinCRegionMin[i];
                pAwbGlobal->AwbFade2Parm.pMaxYRegionMax[i] = _inputParams->awbWpInputParams.afMaxYRegionMax[i];
                pAwbGlobal->AwbFade2Parm.pMaxYRegionMin[i] = _inputParams->awbWpInputParams.afMaxYRegionMin[i];
                pAwbGlobal->AwbFade2Parm.pMinYMaxGRegionMax[i] = _inputParams->awbWpInputParams.afMinYMaxGRegionMax[i];
                pAwbGlobal->AwbFade2Parm.pMinYMaxGRegionMin[i] = _inputParams->awbWpInputParams.afMinYMaxGRegionMin[i];
              #endif
              pAwbGlobal->AwbFade2Parm.pRefCb[i] = _inputParams->awbWpInputParams.afRefCb[i];
              pAwbGlobal->AwbFade2Parm.pRefCr[i] = _inputParams->awbWpInputParams.afRefCr[i];
            }
            pAwbGlobal->fRgProjIndoorMin = _inputParams->awbWpInputParams.fRgProjIndoorMin;
            pAwbGlobal->fRgProjOutdoorMin = _inputParams->awbWpInputParams.fRgProjOutdoorMin;
            pAwbGlobal->fRgProjMax = _inputParams->awbWpInputParams.fRgProjMax;
            pAwbGlobal->fRgProjMaxSky = _inputParams->awbWpInputParams.fRgProjMaxSky;
            pAwbGlobal->fRgProjALimit = _inputParams->awbWpInputParams.fRgProjALimit;
            pAwbGlobal->fRgProjAWeight = _inputParams->awbWpInputParams.fRgProjAWeight;
            pAwbGlobal->fRgProjYellowLimitEnable = _inputParams->awbWpInputParams.fRgProjYellowLimitEnable;
            pAwbGlobal->fRgProjYellowLimit = _inputParams->awbWpInputParams.fRgProjYellowLimit;
            pAwbGlobal->fRgProjIllToCwfEnable = _inputParams->awbWpInputParams.fRgProjIllToCwfEnable;
            pAwbGlobal->fRgProjIllToCwf = _inputParams->awbWpInputParams.fRgProjIllToCwf;
            pAwbGlobal->fRgProjIllToCwfWeight = _inputParams->awbWpInputParams.fRgProjIllToCwfWeight;
            pAwbGlobal->fRegionSize = _inputParams->awbWpInputParams.fRegionSize;
            pAwbGlobal->fRegionSizeInc = _inputParams->awbWpInputParams.fRegionSizeInc;
            pAwbGlobal->fRegionSizeDec = _inputParams->awbWpInputParams.fRegionSizeDec;
        }
        #if 0
        memset(&cfg, 0, sizeof(cfg));
        cfg.updated_mask = HAL_ISP_AWB_MEAS_MASK;
        cfg.enabled[HAL_ISP_AWB_MEAS_ID] = HAL_ISP_ACTIVE_DEFAULT;
        _isp10_engine->configureISP(&cfg);
        #else
        _isp10_engine->setTuningToolAwbParams(NULL);
        #endif
        memset(&cfg, 0, sizeof(cfg));
        cfg.awb_cfg = &awb_meas;
        awb_meas.win.left_hoff = _inputParams->awbWpInputParams.win_h_offs;
        awb_meas.win.top_voff = _inputParams->awbWpInputParams.win_v_offs;
        awb_meas.win.right_width = _inputParams->awbWpInputParams.win_width;
        awb_meas.win.bottom_height = _inputParams->awbWpInputParams.win_height;
        awb_meas.mode = (enum HAL_ISP_AWB_MEASURING_MODE_e)_inputParams->awbWpInputParams.awb_mode;
        if(_inputParams->awbWpInputParams.awb_mode == 0){
            awb_meas.mode = HAL_ISP_AWB_MEASURING_MODE_RGB;
        }else if(_inputParams->awbWpInputParams.awb_mode == 1){
            awb_meas.mode = HAL_ISP_AWB_MEASURING_MODE_YCBCR;
        }else{
            awb_meas.mode = HAL_ISP_AWB_MEASURING_MODE_YCBCR;
        }
        cfg.updated_mask = HAL_ISP_AWB_MEAS_MASK;
        cfg.enabled[HAL_ISP_AWB_MEAS_ID] = HAL_ISP_ACTIVE_SETTING;
        _isp10_engine->configureISP(&cfg);
    }
}

void RKiqCompositor::tuning_tool_set_awb_curve()
{
    CamCalibAwb_V11_Global_t *pAwbGlobal=NULL;
    CamCalibDbHandle_t hCalib;
    char cur_resolution[20];

    if(_inputParams.ptr() && _inputParams->awbCurveInputParams.updateFlag){
        _inputParams->awbCurveInputParams.updateFlag = false;
        struct CamIA10_SensorModeData &sensor_mode = get_sensor_mode_data();
        sprintf(cur_resolution,"%dx%d",sensor_mode.sensor_output_width,sensor_mode.sensor_output_height);
        _isp10_engine->getCalibdbHandle(&hCalib);
        CamCalibDbGetAwb_V11_GlobalByResolution(hCalib, cur_resolution, &pAwbGlobal);
        if(pAwbGlobal){
            LOGD("set awb curve enter");
            pAwbGlobal->CenterLine.f_N0_Rg = _inputParams->awbCurveInputParams.f_N0_Rg;
            pAwbGlobal->CenterLine.f_N0_Bg = _inputParams->awbCurveInputParams.f_N0_Bg;
            pAwbGlobal->CenterLine.f_d = _inputParams->awbCurveInputParams.f_d;
            pAwbGlobal->KFactor.fCoeff[0] = _inputParams->awbCurveInputParams.Kfactor;
            for (int i = 0; i < pAwbGlobal->AwbClipParam.ArraySize1; i++) {
              pAwbGlobal->AwbClipParam.pRg1[i] = _inputParams->awbCurveInputParams.afRg1[i];
              pAwbGlobal->AwbClipParam.pMaxDist1[i] = _inputParams->awbCurveInputParams.afMaxDist1[i];
            }
            for (int i = 0; i < pAwbGlobal->AwbClipParam.ArraySize2; i++) {
              pAwbGlobal->AwbClipParam.pRg2[i] = _inputParams->awbCurveInputParams.afRg2[i];
              pAwbGlobal->AwbClipParam.pMaxDist2[i] = _inputParams->awbCurveInputParams.afMaxDist2[i];
            }
            for (int i = 0; i < pAwbGlobal->AwbGlobalFadeParm.ArraySize1; i++) {
              pAwbGlobal->AwbGlobalFadeParm.pGlobalFade1[i] = _inputParams->awbCurveInputParams.afGlobalFade1[i];
              pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance1[i] = _inputParams->awbCurveInputParams.afGlobalGainDistance1[i];
            }
            for (int i = 0; i < pAwbGlobal->AwbGlobalFadeParm.ArraySize2; i++) {
              pAwbGlobal->AwbGlobalFadeParm.pGlobalFade2[i] = _inputParams->awbCurveInputParams.afGlobalFade2[i];
              pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance2[i] = _inputParams->awbCurveInputParams.afGlobalGainDistance2[i];
            }
        }
        struct HAL_ISP_cfg_s cfg;
        memset(&cfg, 0, sizeof(cfg));
        cfg.updated_mask = HAL_ISP_AWB_MEAS_MASK;
        cfg.enabled[HAL_ISP_AWB_MEAS_ID] = HAL_ISP_ACTIVE_DEFAULT;
        _isp10_engine->configureISP(&cfg);
        _isp10_engine->setTuningToolAwbParams(NULL);
    }
}

void RKiqCompositor::tuning_tool_set_awb_refgain()
{
    CamCalibDbHandle_t hCalib;
    CamIlluminationName_t illuname;
    CamAwb_V11_IlluProfile_t *pIllumination = NULL;
    if(_inputParams.ptr() && _inputParams->awbRefGainInputParams.updateFlag){
        CAM_AwbVersion_t vName;
        _inputParams->awbRefGainInputParams.updateFlag = false;
        _isp10_engine->getCalibdbHandle(&hCalib);
        CamCalibDbGetAwb_VersionName(hCalib, &vName);
        if(vName != CAM_AWB_VERSION_11)
            return;

        memcpy(illuname, _inputParams->awbRefGainInputParams.ill_name,sizeof(illuname));
        CamCalibDbGetAwb_V11_IlluminationByName(hCalib, illuname, &pIllumination);
        if(pIllumination){
            pIllumination->referenceWBgain.fCoeff[0] = _inputParams->awbRefGainInputParams.refRGain;
            pIllumination->referenceWBgain.fCoeff[1] = _inputParams->awbRefGainInputParams.refGrGain;
            pIllumination->referenceWBgain.fCoeff[2] = _inputParams->awbRefGainInputParams.refGbGain;
            pIllumination->referenceWBgain.fCoeff[3] = _inputParams->awbRefGainInputParams.refBGain;
            _isp10_engine->setTuningToolAwbParams(NULL);
        }
    }

}

void RKiqCompositor::tuning_tool_set_goc()
{
    CamCalibGocProfile_t *pGocProfile;
    CamGOCProfileName_t goc_name;
    CamCalibDbHandle_t hCalib;

    if(_inputParams.ptr() && _inputParams->gocInputParams.updateFlag){
        _inputParams->gocInputParams.updateFlag = false;
        if(_inputParams->gocInputParams.on){
            memcpy(goc_name, _inputParams->gocInputParams.scene_name,sizeof(goc_name));
            _isp10_engine->getCalibdbHandle(&hCalib);
            CamCalibDbGetGocProfileByName(hCalib, goc_name, &pGocProfile);
            pGocProfile->def_cfg_mode = _inputParams->gocInputParams.cfg_mode;
            memcpy(pGocProfile->GammaY, _inputParams->gocInputParams.gamma_y, 34*2);
            struct HAL_ISP_cfg_s cfg;
            memset(&cfg, 0, sizeof(cfg));
            cfg.updated_mask = HAL_ISP_GOC_MASK;
            cfg.enabled[HAL_ISP_GOC_ID] = HAL_ISP_ACTIVE_DEFAULT;
            _isp10_engine->configureISP(&cfg);
        }else{
            struct HAL_ISP_cfg_s cfg;
            struct HAL_ISP_goc_cfg_s goc_cfg;
            memset(&cfg, 0, sizeof(cfg));
            cfg.goc_cfg = &goc_cfg;
            cfg.goc_cfg->used_cnt = 34;
            cfg.updated_mask = HAL_ISP_GOC_MASK;
            cfg.enabled[HAL_ISP_GOC_ID] = HAL_ISP_ACTIVE_FALSE;
            _isp10_engine->configureISP(&cfg);
        }
    }
}

void RKiqCompositor::tuning_tool_set_cproc()
{
    if(_inputParams.ptr() && _inputParams->cprocInputParams.updateFlag){
        _inputParams->cprocInputParams.updateFlag = false;
        if(_inputParams->cprocInputParams.on){
            struct HAL_ISP_cfg_s cfg;
            struct HAL_ISP_cproc_cfg_s isp_cproc_cfg;
            memset(&cfg, 0, sizeof(cfg));
            cfg.cproc_cfg = &isp_cproc_cfg;
            isp_cproc_cfg.use_case = (enum USE_CASE)_inputParams->cprocInputParams.mode;
            isp_cproc_cfg.cproc.brightness = _inputParams->cprocInputParams.cproc_brightness;
            isp_cproc_cfg.cproc.contrast = _inputParams->cprocInputParams.cproc_contrast;
            isp_cproc_cfg.cproc.hue = _inputParams->cprocInputParams.cproc_hue;
            isp_cproc_cfg.cproc.saturation = _inputParams->cprocInputParams.cproc_saturation;
            isp_cproc_cfg.range = HAL_ISP_COLOR_RANGE_OUT_FULL_RANGE;
            cfg.updated_mask = HAL_ISP_CPROC_MASK;
            cfg.enabled[HAL_ISP_CPROC_ID] = HAL_ISP_ACTIVE_SETTING;
            _isp10_engine->configureISP(&cfg);
        }else{
            struct HAL_ISP_cfg_s cfg;
            memset(&cfg, 0, sizeof(cfg));
            cfg.updated_mask = HAL_ISP_CPROC_MASK;
            cfg.enabled[HAL_ISP_CPROC_ID] = HAL_ISP_ACTIVE_FALSE;
            _isp10_engine->configureISP(&cfg);
        }
    }
}

void RKiqCompositor::tuning_tool_set_dpf()
{
    CamDpfProfile_t *pDpfProfile=NULL;
    CamCalibDbHandle_t hCalib;
    CamDpfProfileName_t dpf_name;
    if(_inputParams.ptr() && _inputParams->adpfInputParams.updateFlag){
        _inputParams->adpfInputParams.updateFlag = false;
        if(_inputParams->adpfInputParams.dpf_enable)
        {
            memcpy(dpf_name, _inputParams->adpfInputParams.dpf_name,sizeof(dpf_name));
            _isp10_engine->getCalibdbHandle(&hCalib);
            CamCalibDbGetDpfProfileByName(hCalib, strupr(dpf_name), &pDpfProfile);
            if(pDpfProfile){
                pDpfProfile->ADPFEnable = _inputParams->adpfInputParams.dpf_enable;
                pDpfProfile->nll_segmentation = _inputParams->adpfInputParams.nll_segment;
                memcpy(pDpfProfile->nll_coeff.uCoeff,_inputParams->adpfInputParams.nll_coeff, 34);
                pDpfProfile->SigmaGreen = _inputParams->adpfInputParams.sigma_green;
                pDpfProfile->SigmaRedBlue = _inputParams->adpfInputParams.sigma_redblue;
                pDpfProfile->fGradient = _inputParams->adpfInputParams.gradient;
                pDpfProfile->fOffset = _inputParams->adpfInputParams.offset;
                pDpfProfile->NfGains.fCoeff[0] = _inputParams->adpfInputParams.fRed;
                pDpfProfile->NfGains.fCoeff[1] = _inputParams->adpfInputParams.fGreenR;
                pDpfProfile->NfGains.fCoeff[2] = _inputParams->adpfInputParams.fGreenB;
                pDpfProfile->NfGains.fCoeff[3] = _inputParams->adpfInputParams.fBlue;

                struct HAL_ISP_cfg_s cfg;
                struct HAL_ISP_dpf_cfg_s dpf_cfg;
                memset(&cfg, 0, sizeof(cfg));
                cfg.dpf_cfg = &dpf_cfg;
                cfg.updated_mask = HAL_ISP_DPF_MASK;
                cfg.enabled[HAL_ISP_DPF_ID] = HAL_ISP_ACTIVE_SETTING;
                _isp10_engine->configureISP(&cfg);
                _isp10_engine->setTuningToolAdpfParams();
            }
            _dpf_en_for_tool = false;
        }
        else{
            _dpf_en_for_tool = true;
        }
    }
    if(_dpf_en_for_tool){
        struct HAL_ISP_cfg_s cfg;
        memset(&cfg, 0, sizeof(cfg));
        cfg.updated_mask = HAL_ISP_DPF_MASK;
        cfg.enabled[HAL_ISP_DPF_ID] = HAL_ISP_ACTIVE_FALSE;
        _isp10_engine->configureISP(&cfg);
    }
}

void RKiqCompositor::tuning_tool_set_flt()
{
    CamDpfProfile_t *pDpfProfile=NULL;
    CamFilterProfile_t *pFilterProfile=NULL;
    CamCalibDbHandle_t hCalib;
    CamFilterProfileName_t filt_name[]={"NORMAL","NIGHT"};
    CamDpfProfileName_t dpf_name;
    if(_inputParams.ptr() && _inputParams->fltInputParams.updateFlag){
        _inputParams->fltInputParams.updateFlag = false;
        if(_inputParams->fltInputParams.filter_enable){
        memcpy(dpf_name, _inputParams->fltInputParams.filter_name,sizeof(dpf_name));
        _isp10_engine->getCalibdbHandle(&hCalib);
        CamCalibDbGetDpfProfileByName(hCalib, strupr(dpf_name), &pDpfProfile);
        if(pDpfProfile){
            if(_inputParams->fltInputParams.scene_mode==0){
                CamCalibDbGetFilterProfileByName(hCalib, pDpfProfile, filt_name[0], &pFilterProfile);
            }else{
                CamCalibDbGetFilterProfileByName(hCalib, pDpfProfile, filt_name[1], &pFilterProfile);
            }

            if(pFilterProfile){
                pFilterProfile->FilterEnable = _inputParams->fltInputParams.filter_enable;
                for(int i=0; i<5; i++)
                {
                    pFilterProfile->DenoiseLevelCurve.pSensorGain[i] = (float)_inputParams->fltInputParams.denoise_gain[i];
                    pFilterProfile->DenoiseLevelCurve.pDlevel[i] = (CamerIcIspFltDeNoiseLevel_t)(_inputParams->fltInputParams.denoise_level[i]+1);
                }
                for(int i=0; i<5; i++)
                {
                    pFilterProfile->SharpeningLevelCurve.pSensorGain[i] = (float)_inputParams->fltInputParams.sharp_gain[i];
                    pFilterProfile->SharpeningLevelCurve.pSlevel[i] = (CamerIcIspFltSharpeningLevel_t)(_inputParams->fltInputParams.sharp_level[i]+1);
                }
                for(int i=0; i<pFilterProfile->FiltLevelRegConf.ArraySize; i++){
                    if (pFilterProfile->FiltLevelRegConf.p_FiltLevel[i] == _inputParams->fltInputParams.level){
                        pFilterProfile->FiltLevelRegConf.FiltLevelRegConfEnable = _inputParams->fltInputParams.level_conf_enable;
                        pFilterProfile->FiltLevelRegConf.p_grn_stage1[i] = _inputParams->fltInputParams.level_conf.grn_stage1;
                        pFilterProfile->FiltLevelRegConf.p_chr_h_mode[i] = _inputParams->fltInputParams.level_conf.chr_h_mode;
                        pFilterProfile->FiltLevelRegConf.p_chr_v_mode[i] = _inputParams->fltInputParams.level_conf.chr_v_mode;
                        pFilterProfile->FiltLevelRegConf.p_thresh_bl0[i] = _inputParams->fltInputParams.level_conf.thresh_bl0;
                        pFilterProfile->FiltLevelRegConf.p_thresh_bl1[i] = _inputParams->fltInputParams.level_conf.thresh_bl1;
                        pFilterProfile->FiltLevelRegConf.p_thresh_sh0[i] = _inputParams->fltInputParams.level_conf.thresh_sh0;
                        pFilterProfile->FiltLevelRegConf.p_thresh_sh1[i] = _inputParams->fltInputParams.level_conf.thresh_sh1;
                        pFilterProfile->FiltLevelRegConf.p_fac_sh0[i] = _inputParams->fltInputParams.level_conf.fac_sh0;
                        pFilterProfile->FiltLevelRegConf.p_fac_sh1[i] = _inputParams->fltInputParams.level_conf.fac_sh1;
                        pFilterProfile->FiltLevelRegConf.p_fac_mid[i] = _inputParams->fltInputParams.level_conf.fac_mid;
                        pFilterProfile->FiltLevelRegConf.p_fac_bl0[i] = _inputParams->fltInputParams.level_conf.fac_bl0;
                        pFilterProfile->FiltLevelRegConf.p_fac_bl1[i] = _inputParams->fltInputParams.level_conf.fac_bl1;

                        break;
                    }
                }
                #if 0
                struct HAL_ISP_cfg_s cfg;
                memset(&cfg, 0, sizeof(cfg));
                cfg.updated_mask = HAL_ISP_FLT_MASK;
                cfg.enabled[HAL_ISP_FLT_ID] = HAL_ISP_ACTIVE_DEFAULT;
                _isp10_engine->configureISP(&cfg);
                #endif
                _isp10_engine->setTuningToolAdpfParams();
                }
            }
            _flt_en_for_tool = false;
        }else{
            _flt_en_for_tool = true;
        }
    }
    if(_flt_en_for_tool){
        struct HAL_ISP_cfg_s cfg;
        memset(&cfg, 0, sizeof(cfg));
        cfg.updated_mask = HAL_ISP_FLT_MASK;
        cfg.enabled[HAL_ISP_FLT_ID] = HAL_ISP_ACTIVE_FALSE;
        _isp10_engine->configureISP(&cfg);
    }
}

void RKiqCompositor::tuning_tool_restart_engine()
{
    if(_inputParams.ptr() && _inputParams->restartInputParams.updateFlag){
        _inputParams->restartInputParams.updateFlag = false;
        if(_inputParams->restartInputParams.on){
            _awb_handler->_analyzer->restart();
            _skip_frame = true;
        }
    }
}

void RKiqCompositor::tuning_tool_process(struct CamIA10_Results &ia10_results)
{
    tuning_tool_set_bls();
    tuning_tool_set_lsc();
    tuning_tool_set_ccm(ia10_results.awb);
    tuning_tool_set_awb();
    tuning_tool_set_awb_wp();
    tuning_tool_set_awb_curve();
    tuning_tool_set_awb_refgain();
    tuning_tool_set_goc();
    tuning_tool_set_cproc();
    tuning_tool_set_dpf();
    tuning_tool_set_flt();
    tuning_tool_restart_engine();
}

XCamReturn RKiqCompositor::integrate (X3aResultList &results, bool first)
{
    SmartPtr<X3aResult> isp_results;
    struct rkisp_parameters isp_3a_result;

    if (!_isp10_engine)
        XCAM_LOG_ERROR ("ISP control device is null");

    xcam_mem_clear(isp_3a_result);

    //_isp10_engine->runIA(&_ia_dcfg, &_ia_stat, &_ia_results);
    _isp10_engine->getIAResult(&_ia_results);
    tuning_tool_process(_ia_results);
    if(_skip_frame){
        _skip_frame = false;
        return XCAM_RETURN_NO_ERROR;
    }
    if (!_isp10_engine->runISPManual(&_ia_results, BOOL_TRUE)) {
        XCAM_LOG_ERROR("%s:run ISP manual failed!", __func__);
    }

    if (_ae_handler && _awb_handler && _inputParams.ptr()) {
        if (_all_stats_meas_types ==
            (CAMIA10_AEC_MASK | CAMIA10_HST_MASK | CAMIA10_AWB_MEAS_MASK | CAMIA10_AFC_MASK)
            || first) {
            LOGD("%s:%d, complete all 3A stats analysis, report results",
                 __FUNCTION__, __LINE__);
            _ae_handler->processAeMetaResults(_ia_results.aec, results);
            _awb_handler->processAwbMetaResults(_ia_results.awb, results);
            _af_handler->processAfMetaResults(_ia_results.af, results);
            _common_handler->processToneMapsMetaResults(_ia_results.goc, results);
            _common_handler->processMiscMetaResults(_ia_results, results, first);
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
    isp_3a_result.awb_algo_results.DomIlluIdx =
        _ia_results.awb.DomIlluIdx;
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

    // update flash settings
    rkisp_flash_setting_t* flash_set = &isp_3a_result.flash_settings;
    CamIA10_flash_setting_t* flash_result = &_ia_results.flash;
    isp_3a_result.uc = _ia_results.uc;
    // if flash sync is needed, main flash should be triggered when
    // STILLCAP_SYNC_CMD_SYNCSTART is received
    if (_common_handler->_stillcap_sync_state ==
        AiqCommonHandler::STILLCAP_SYNC_STATE_WAITING_START &&
        flash_result->flash_mode == HAL_FLASH_MAIN) {
        *flash_set = _flash_old_setting;
    } else {
        flash_set->uc = _ia_results.uc;
        switch (flash_result->flash_mode) {
        case HAL_FLASH_OFF :
            flash_set->flash_mode = RKISP_FLASH_MODE_OFF;
            break;
        case HAL_FLASH_TORCH:
            flash_set->flash_mode = RKISP_FLASH_MODE_TORCH;
            break;
        case HAL_FLASH_PRE:
            flash_set->flash_mode = RKISP_FLASH_MODE_FLASH_PRE;
            break;
        case HAL_FLASH_MAIN:
            flash_set->flash_mode = RKISP_FLASH_MODE_FLASH_MAIN;
            break;
        case HAL_FLASH_ON:
            flash_set->flash_mode = RKISP_FLASH_MODE_FLASH;
            break;
        default:
            LOGE("not support flash mode %d", flash_result->flash_mode);
        }

        flash_set->strobe = flash_result->strobe;
        flash_set->timeout_ms = flash_result->flash_timeout_ms;
        for (int i = 0; i < CAMIA10_FLASH_NUM_MAX; i ++)
            flash_set->power[i] = flash_result->flash_power[i];
    }

    _flash_old_setting = *flash_set;

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
