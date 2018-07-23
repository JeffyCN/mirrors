/*
 * isp_image_processor.cpp - isp image processor
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

#include "isp_image_processor.h"
#include "x3a_isp_config.h"
#include "isp_controller.h"
#include "isp_config_translator.h"

namespace XCam {

IspImageProcessor::IspImageProcessor (SmartPtr<IspController> &controller, bool sync)
    : ImageProcessor ("IspImageProcessor")
    , _sync(sync)
    , _controller (controller)
    , _3a_config (new X3aIspConfig)
{
    _sensor = new SensorDescriptor;
    _translator = new IspConfigTranslator (_sensor);
    XCAM_LOG_DEBUG ("IspImageProcessor construction");
}

IspImageProcessor::~IspImageProcessor ()
{
    XCAM_LOG_DEBUG ("~IspImageProcessor destruction");
}

XCamReturn
IspImageProcessor::process_buffer(SmartPtr<VideoBuffer> &input, SmartPtr<VideoBuffer> &output)
{
    output = input;
    return XCAM_RETURN_NO_ERROR;
}

bool
IspImageProcessor::can_process_result (SmartPtr<X3aResult> &result)
{
    if (result.ptr() == NULL)
        return false;
    XCAM_LOG_DEBUG ("can_process_result type: %d", result->get_type());

    switch (result->get_type()) {
    case X3aIspConfig::IspAllParameters:
    case X3aIspConfig::IspExposureParameters:
    case X3aIspConfig::IspFocusParameters:
    case XCAM_3A_RESULT_ALL:
    case XCAM_3A_RESULT_WHITE_BALANCE:
    case XCAM_3A_RESULT_EXPOSURE:
    case XCAM_3A_RESULT_BLACK_LEVEL:
    case XCAM_3A_RESULT_YUV2RGB_MATRIX:
    case XCAM_3A_RESULT_RGB2YUV_MATRIX:
    case XCAM_3A_RESULT_R_GAMMA:
    case XCAM_3A_RESULT_G_GAMMA:
    case XCAM_3A_RESULT_B_GAMMA:
    case XCAM_3A_RESULT_MACC:
    case XCAM_3A_RESULT_BAYER_NOISE_REDUCTION:
        return true;
    default:
        return false;
    }

    return false;
}

XCamReturn
IspImageProcessor::apply_3a_results (X3aResultList &results)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    XCAM_LOG_DEBUG ("apply_3a_results start");

    if (results.empty())
        return XCAM_RETURN_ERROR_PARAM;

    // activate sensor to make translator work
    if (!_sensor->is_ready()) {
        struct isp_supplemental_sensor_mode_data sensor_data;
        xcam_mem_clear (sensor_data);
        if (_controller->get_sensor_mode_data(sensor_data) != XCAM_RETURN_NO_ERROR) {
            XCAM_LOG_WARNING ("ispimageprocessor initiliaze sensor failed");
        } else
            _sensor->set_sensor_data (sensor_data);
        XCAM_ASSERT (_sensor->is_ready());
    }

    if ((ret = merge_results (results)) != XCAM_RETURN_NO_ERROR) {
        XCAM_LOG_WARNING ("merge 3a result to isp config failed");
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    if ((ret = apply_exposure_result (results)) != XCAM_RETURN_NO_ERROR) {
        XCAM_LOG_WARNING ("set 3a exposure to sensor failed");
    }

    if ((ret = apply_focus_result (results)) != XCAM_RETURN_NO_ERROR) {
        XCAM_LOG_WARNING ("set 3a focus to vcm failed");
    }

    // check _3a_config
    XCAM_ASSERT (_3a_config.ptr());
    XCAM_ASSERT (_controller.ptr());
    ret = _controller->set_3a_config (_3a_config.ptr());
    if (ret != XCAM_RETURN_NO_ERROR) {
        XCAM_LOG_WARNING ("set 3a config to isp failed");
    }

    /*
     * needn't clear old config, if there is new config, so that we can
     * use the old config.
     */
    /* _3a_config->clear (); */

    XCAM_LOG_DEBUG ("apply_3a_results done");

    return ret;
}

XCamReturn
IspImageProcessor::apply_3a_result (SmartPtr<X3aResult> &result)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    X3aResultList results;
    results.push_back (result);
    ret = apply_3a_results (results);
    return ret;
}

XCamReturn
IspImageProcessor::merge_results (X3aResultList &results)
{
    if (results.empty())
        return XCAM_RETURN_ERROR_PARAM;

    for (X3aResultList::iterator iter = results.begin ();
            iter != results.end ();)
    {
        SmartPtr<X3aResult> &x3a_result = *iter;
        if (_3a_config->attach (x3a_result, _translator.ptr())) {
            x3a_result->set_done (true);
            results.erase (iter++);
        } else
            ++iter;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
IspImageProcessor::apply_exposure_result (X3aResultList &results)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    for (X3aResultList::iterator iter = results.begin ();
            iter != results.end ();)
    {
        XCAM_LOG_DEBUG ("apply_exposure_result type: %x", (*iter)->get_type());
        if ((*iter)->get_type() == X3aIspConfig::IspExposureParameters) {
            SmartPtr<X3aIspExposureResult> res = (*iter).dynamic_cast_ptr<X3aIspExposureResult> ();

            if (!res.ptr ()) {
                XCAM_LOG_WARNING ("isp 3a exposure result is null");
            } else {
                _controller->push_3a_exposure (res.ptr ());
                if (!_sync) {
                    if((ret = _controller->set_3a_exposure (res.ptr ())) != XCAM_RETURN_NO_ERROR) {
                        XCAM_LOG_WARNING ("set 3a exposure to sensor failed");
                    }
                }
                res->set_done (true);
            }

            results.erase (iter++);
        } else if ((*iter)->get_type() == XCAM_3A_RESULT_EXPOSURE) {
            SmartPtr<X3aExposureResult> res = (*iter).dynamic_cast_ptr<X3aExposureResult> ();
            struct rkisp_exposure isp_exposure;
            xcam_mem_clear (isp_exposure);
            XCAM_ASSERT (res.ptr ());
            ret = _translator->translate_exposure (res->get_standard_result (), isp_exposure);
            if (ret != XCAM_RETURN_NO_ERROR) {
                XCAM_LOG_WARNING ("translate 3a exposure to sensor failed");
            }

            _controller->push_3a_exposure (isp_exposure);
            if (!_sync) {
                if ((ret = _controller->set_3a_exposure (isp_exposure)) != XCAM_RETURN_NO_ERROR) {
                    XCAM_LOG_WARNING ("set 3a exposure to sensor failed");
                }
            }

            res->set_done (true);
            results.erase (iter++);
        } else
            ++iter;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
IspImageProcessor::apply_focus_result (X3aResultList &results)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    for (X3aResultList::iterator iter = results.begin ();
            iter != results.end ();)
    {
        XCAM_LOG_DEBUG ("apply_focus_result type: %d", (*iter)->get_type());
        if ((*iter)->get_type() == X3aIspConfig::IspFocusParameters) {
            SmartPtr<X3aIspFocusResult> res = (*iter).dynamic_cast_ptr<X3aIspFocusResult> ();
            if (!res.ptr ()) {
                XCAM_LOG_WARNING ("isp 3a exposure result is null");
            } else {
                _controller->set_3a_focus(res.ptr());
                res->set_done (true);
            }
            results.erase (iter++);
        } else
            ++iter;
    }
    return XCAM_RETURN_NO_ERROR;
}

};
