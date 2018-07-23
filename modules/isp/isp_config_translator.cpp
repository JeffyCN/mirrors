/*
 * isp_config_translator.cpp - isp config translator
 *
 *  Copyright (c) 2014-2015 Intel Corporation
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
 */

#include "isp_config_translator.h"
#include <math.h>

namespace XCam {

static uint32_t
_get_max_bits (double value)
{
    uint32_t max_int = 0;
    uint32_t interger_bits = 0;

    max_int = (uint32_t)value;
    while (max_int) {
        ++interger_bits;
        max_int = (max_int >> 1);
    }
    return interger_bits;
}

IspConfigTranslator::IspConfigTranslator (SmartPtr<SensorDescriptor> &sensor)
    : _sensor (sensor)
{
    XCAM_ASSERT (_sensor.ptr());
}

IspConfigTranslator::~IspConfigTranslator ()
{
}

XCamReturn
IspConfigTranslator::translate_all (
    const XCam3aResultAll &from,
    struct rkisp_parameters &to)
{
    to.active_configs = from.active_configs;
    to.dpcc_config = from.dpcc_config;
    to.bls_config = from.bls_config;
    to.sdg_config = from.sdg_config;
    to.hst_config = from.hst_config;
    to.lsc_config = from.lsc_config;
    to.awb_gain_config = from.awb_gain_config;
    to.awb_meas_config = from.awb_meas_config;
    to.flt_config = from.flt_config;
    to.bdm_config = from.bdm_config;
    to.ctk_config = from.ctk_config;
    to.goc_config = from.goc_config;
    to.cproc_config = from.cproc_config;
    to.aec_config = from.aec_config;
    to.afc_config = from.afc_config;
    to.ie_config = from.ie_config;
    to.dpf_config = from.dpf_config;
    to.dpf_strength_config = from.dpf_strength_config;
    to.aec_config = from.aec_config;
    to.flt_denoise_level= from.flt_denoise_level;
    to.flt_sharp_level= from.flt_sharp_level;

    for (int i=0; i < HAL_ISP_MODULE_MAX_ID_ID + 1; i++) {
        to.enabled[i] = from.enabled[i];
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
IspConfigTranslator::translate_white_balance (
    const XCam3aResultWhiteBalance &from,
    struct rkisp_wb_config &to)
{
    uint32_t interger_bits = 0;
    double multiplier = 0.0;
    double max_gain = XCAM_MAX (from.b_gain, from.r_gain);
    max_gain = XCAM_MAX (max_gain, from.gr_gain);
    max_gain = XCAM_MAX (max_gain, from.gb_gain);

    interger_bits = _get_max_bits (max_gain);
    multiplier = (double)(1 << (16 - interger_bits));
    to.integer_bits = interger_bits;
    to.gr = (uint32_t)(from.gr_gain * multiplier + 0.5);
    to.r = (uint32_t)(from.r_gain * multiplier + 0.5);
    to.b = (uint32_t)(from.b_gain * multiplier + 0.5);
    to.gb = (uint32_t)(from.gb_gain * multiplier + 0.5);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
IspConfigTranslator::translate_black_level (
    const XCam3aResultBlackLevel &from, struct rkisp_ob_config &to)
{
    double multiplier = (double)(1 << 16);

    to.mode = rkisp_ob_mode_fixed;
    to.level_gr = (uint32_t)(from.gr_level * multiplier + 0.5);
    to.level_r = (uint32_t)(from.r_level * multiplier + 0.5);
    to.level_b = (uint32_t)(from.b_level * multiplier + 0.5);
    to.level_gb = (uint32_t)(from.gb_level * multiplier + 0.5);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
IspConfigTranslator::translate_color_matrix (
    const XCam3aResultColorMatrix &from, struct rkisp_cc_config &to)
{
    double max_value = 0.0;
    uint32_t interger_bits = 0;
    double multiplier = 0.0;
    bool have_minus = false;
    uint32_t i = 0;

    for (i = 0; i < XCAM_COLOR_MATRIX_SIZE; ++i) {
        if (fabs(from.matrix [i]) > max_value)
            max_value = fabs(from.matrix [i]);
        if (from.matrix [i] < 0)
            have_minus = true;
    }
    interger_bits = _get_max_bits (max_value);
    if (have_minus)
        ++interger_bits;

    XCAM_ASSERT (interger_bits < 13);
    to.fraction_bits = 13 - interger_bits;
    multiplier = (double)(1 << (13 - interger_bits));
    for (i = 0; i < XCAM_COLOR_MATRIX_SIZE; ++i) {
        to.matrix[i] = (int32_t)(from.matrix [i] * multiplier);
    }
    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
IspConfigTranslator::translate_exposure (
    const XCam3aResultExposure &from,
    struct rkisp_exposure &to)
{
    uint32_t coarse_time = 0, fine_time = 0;
    int32_t analog_code = 0, digital_code = 0;
    if (!_sensor->is_ready ()) {
        XCAM_LOG_WARNING ("translate exposure failed since sensor not ready");
        return XCAM_RETURN_ERROR_SENSOR;
    }
    
    if (!_sensor->exposure_time_to_integration (from.exposure_time, coarse_time, fine_time)) {
        XCAM_LOG_WARNING ("translate exposure time failed");
        return XCAM_RETURN_ERROR_SENSOR;
    }

    to.coarse_integration_time = coarse_time;
    to.fine_integration_time = fine_time;

    if (!_sensor->exposure_gain_to_code (from.analog_gain, from.digital_gain, analog_code, digital_code)) {
        XCAM_LOG_WARNING ("translate exposure gain failed");
        return XCAM_RETURN_ERROR_SENSOR;
    }
    to.analog_gain = analog_code;
    to.digital_gain = digital_code;
    return XCAM_RETURN_NO_ERROR;
}

};
