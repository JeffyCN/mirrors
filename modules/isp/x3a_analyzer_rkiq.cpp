/*
 * x3a_analyzer_aiq.h - 3a analyzer from AIQ
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

#include "x3a_analyzer_rkiq.h"
#include "rkiq_handler.h"
#include "isp_controller.h"
#include "ia_types.h"
#include "isp_ctrl.h"

namespace XCam {

X3aAnalyzerRKiq::X3aAnalyzerRKiq (SmartPtr<IspController> &isp, const char *cpf_path)
    : X3aAnalyzerRKiq(NULL, isp, cpf_path)
{
}

X3aAnalyzerRKiq::X3aAnalyzerRKiq (RkispDeviceManager* device_manager, SmartPtr<IspController> &isp, const char *cpf_path)
    : X3aAnalyzer ("X3aAnalyzerRKiq")
    , _device_manager (device_manager)
    , _isp (isp)
    , _isp_ctrl_dev (NULL)
    , _sensor_data_ready (false)
    , _cpf_path (NULL)
{
    if (cpf_path)
        _cpf_path = strndup (cpf_path, XCAM_MAX_STR_SIZE);

    _rkiq_compositor = new RKiqCompositor ();
    XCAM_ASSERT (_rkiq_compositor.ptr());
    xcam_mem_clear (_sensor_mode_data);

    XCAM_LOG_DEBUG ("X3aAnalyzerRKiq constructed");
}

X3aAnalyzerRKiq::X3aAnalyzerRKiq (struct isp_supplemental_sensor_mode_data &sensor_data, const char *cpf_path)
    : X3aAnalyzer ("X3aAnalyzerRKiq")
    , _sensor_mode_data (sensor_data)
    , _sensor_data_ready (true)
    , _cpf_path (NULL)
{
    if (cpf_path)
        _cpf_path = strndup (cpf_path, XCAM_MAX_STR_SIZE);

    _rkiq_compositor = new RKiqCompositor ();
    XCAM_ASSERT (_rkiq_compositor.ptr());

    XCAM_LOG_DEBUG ("X3aAnalyzerRKiq constructed");
}

X3aAnalyzerRKiq::~X3aAnalyzerRKiq()
{
    if (_cpf_path)
        xcam_free (_cpf_path);

    if (_isp_ctrl_dev) {
        delete _isp_ctrl_dev;
        _isp_ctrl_dev = NULL;
    }
    XCAM_LOG_DEBUG ("~X3aAnalyzerRKiq destructed");
}

SmartPtr<AeHandler>
X3aAnalyzerRKiq::create_ae_handler ()
{
    SmartPtr<AiqAeHandler> ae_handler = new AiqAeHandler (this, _rkiq_compositor);
    _rkiq_compositor->set_ae_handler (ae_handler);
    return ae_handler;
}

SmartPtr<AwbHandler>
X3aAnalyzerRKiq::create_awb_handler ()
{
    SmartPtr<AiqAwbHandler> awb_handler = new AiqAwbHandler (this, _rkiq_compositor);
    _rkiq_compositor->set_awb_handler (awb_handler);
    return awb_handler;
}

SmartPtr<AfHandler>
X3aAnalyzerRKiq::create_af_handler ()
{

    SmartPtr<AiqAfHandler> af_handler = new AiqAfHandler (_rkiq_compositor);
    _rkiq_compositor->set_af_handler (af_handler);
    return af_handler;
}

SmartPtr<CommonHandler>
X3aAnalyzerRKiq::create_common_handler ()
{
    SmartPtr<AiqCommonHandler> common_handler = new AiqCommonHandler (_rkiq_compositor);
    _rkiq_compositor->set_common_handler (common_handler);
    return common_handler;
}

XCamReturn
X3aAnalyzerRKiq::internal_init (uint32_t width, uint32_t height, double framerate)
{
    XCamReturn ret;
    struct isp_supplemental_sensor_mode_data sensor_mode_data;
    _isp_ctrl_dev = new Isp10Engine();

    _rkiq_compositor->set_isp_ctrl_device(_isp_ctrl_dev);
    if (!_device) {
        XCAM_LOG_WARNING("no capture device.\n");
    }

    XCAM_LOG_INFO ("ready get sensor mode succc.");
    ret = _isp->get_sensor_mode_data (sensor_mode_data);
    XCAM_FAIL_RETURN (WARNING, ret == XCAM_RETURN_NO_ERROR, ret, "get sensor mode data failed");

    XCAM_LOG_INFO ("init get sensor mode succc.");

    configIsp(_isp_ctrl_dev, &sensor_mode_data);

    XCAM_LOG_INFO ("config rkisp info sensorw-h=[%d-%d], ispoutw-hw-h=[%d-%d], vts: %d, llp: %d, pclk: %d",
                        sensor_mode_data.sensor_output_width,
                        sensor_mode_data.sensor_output_height,
                        sensor_mode_data.isp_output_width,
                        sensor_mode_data.isp_output_height,
                        sensor_mode_data.frame_length_lines,
                        sensor_mode_data.line_length_pck,
                        sensor_mode_data.vt_pix_clk_freq_hz);

    //init _isp_ctrl_dev
    if (_device) {
        _isp_ctrl_dev->setISPDeviceFd(_isp_stats_device->get_fd());
    }

    _isp_ctrl_dev->init(_cpf_path, _device_manager->get_sensor_entity_name(),
                        _device_manager->get_isp_ver(),
                        /*_device->get_fd()*/0);

    _rkiq_compositor->set_size (width, height);
    _rkiq_compositor->set_framerate (framerate);
    _rkiq_compositor->init_dynamic_config();

    return XCAM_RETURN_NO_ERROR;

}

XCamReturn
X3aAnalyzerRKiq::internal_deinit ()
{
    if (_rkiq_compositor.ptr ()) {
        XCAM_LOG_INFO ("release RKiqCompositor.");
        _rkiq_compositor->close ();
        _rkiq_compositor.release();
    }

    if (_isp_ctrl_dev) {
        _isp_ctrl_dev->deInit();
        delete _isp_ctrl_dev;
        _isp_ctrl_dev = NULL;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
X3aAnalyzerRKiq::configure_3a ()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    X3aResultList first_results;

    // always true for reconfig 3a
    if (/* !_sensor_data_ready */true) {
        struct isp_supplemental_sensor_mode_data sensor_mode_data;
        xcam_mem_clear (sensor_mode_data);
        XCAM_ASSERT (_isp.ptr());

        ret = _isp->get_sensor_mode_data (sensor_mode_data);
        XCAM_FAIL_RETURN (WARNING, ret == XCAM_RETURN_NO_ERROR, ret, "get sensor mode data failed");
        _sensor_mode_data = sensor_mode_data;
        _sensor_data_ready = true;
    }

    /* update the initial settings from user */
    _rkiq_compositor->setAiqInputParams(this->getAiqInputParams());
    if (!_rkiq_compositor->set_sensor_mode_data (&_sensor_mode_data)) {
        XCAM_LOG_WARNING ("AIQ configure 3a failed");
        return XCAM_RETURN_ERROR_AIQ;
    }

    XCAM_LOG_INFO("X3aAnalyzerRKiq got sensor mode data, coarse_time_min:%u, "
                    "coarse_time_max_margin:%u, "
                    "fine_time_min:%u, fine_time_max_margin:%u, "
                    "frame_length_lines:%u, line_length_pck:%u, "
                    "vt_pix_clk_freq_mhz:%u, "
                    "crop_horizontal_start:%u, crop_vertical_start:%u, "
                    "crop_horizontal_end:%u, crop_vertical_end:%u, "
                    "output_width:%u, output_height:%u, "
                    "sensor_output_width:%d, sensor_output_height:%d, "
                    "binning_factor_x:%u, binning_factor_y:%u",
                    _sensor_mode_data.coarse_integration_time_min,
                    _sensor_mode_data.coarse_integration_time_max_margin,
                    _sensor_mode_data.fine_integration_time_min,
                    _sensor_mode_data.fine_integration_time_max_margin,
                    _sensor_mode_data.frame_length_lines,
                    _sensor_mode_data.line_length_pck,
                    _sensor_mode_data.vt_pix_clk_freq_hz,
                    _sensor_mode_data.crop_horizontal_start,
                    _sensor_mode_data.crop_vertical_start,
                    _sensor_mode_data.crop_horizontal_end,
                    _sensor_mode_data.crop_vertical_end,
                    _sensor_mode_data.isp_output_width,
                    _sensor_mode_data.isp_output_height,
                    _sensor_mode_data.sensor_output_width,
                    _sensor_mode_data.sensor_output_height,
                    (uint32_t)_sensor_mode_data.binning_factor_x,
                    (uint32_t)_sensor_mode_data.binning_factor_y);

    // initialize ae and awb
    get_ae_handler ()->analyze (first_results, true);
    get_awb_handler ()->analyze (first_results, true);
    get_af_handler ()->analyze (first_results, true);

    if (!first_results.empty()) {
        ret = _rkiq_compositor->integrate (first_results);
        XCAM_FAIL_RETURN (WARNING, ret == XCAM_RETURN_NO_ERROR, ret, "AIQ configure_3a failed on integrate results");
        #if 1
        for (X3aResultList::iterator iter = first_results.begin (); iter != first_results.end (); ++iter) {
            SmartPtr<X3aResult> &isp_result = *iter;

            isp_result->set_first_params (true);

            XCAM_LOG_INFO ("first result type: %d", isp_result->get_type());
            if (isp_result->get_type() == X3aIspConfig::IspAllParameters) {
                SmartPtr<X3aAtomIspParametersResult> isp_3a_all =
                    isp_result.dynamic_cast_ptr<X3aAtomIspParametersResult> ();
                XCAM_ASSERT (isp_3a_all.ptr ());
                const struct rkisp_parameters &rkisp_params = isp_3a_all->get_isp_config ();
                XCAM_LOG_INFO("rkiq active_configs: %x", rkisp_params.active_configs);
            }

        }
        #endif
        notify_calculation_done (first_results);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
X3aAnalyzerRKiq::analyze (const SmartPtr<VideoBuffer> &buffer)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = X3aAnalyzer::analyze(buffer);

    return ret;
}

XCamReturn
X3aAnalyzerRKiq::pre_3a_analyze (SmartPtr<X3aStats> &stats)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<X3aIspStatistics> xcam_isp_stats = stats.dynamic_cast_ptr<X3aIspStatistics> ();
    XCAM_ASSERT (xcam_isp_stats.ptr ());
    struct isp_supplemental_sensor_mode_data sensor_mode_data;
    struct rk_cam_vcm_tim vcm_tim;
    int64_t sof_tim;
    struct rkisp_parameters isp_params;

    xcam_mem_clear (sensor_mode_data);
    xcam_mem_clear (isp_params);
    XCAM_ASSERT (_isp.ptr());

    struct cifisp_stat_buffer* stats_3a = (struct cifisp_stat_buffer*)xcam_isp_stats->get_isp_stats();

    ret = _isp->get_isp_parameter (isp_params, stats_3a->frame_id);
    XCAM_FAIL_RETURN (WARNING, ret == XCAM_RETURN_NO_ERROR, ret,
                      "get effecting id %d isp params failed", stats_3a->frame_id);

    ret = _isp->get_sensor_mode_data (sensor_mode_data, stats_3a->frame_id);

    XCAM_FAIL_RETURN (WARNING, ret == XCAM_RETURN_NO_ERROR, ret, "get sensor mode data failed");
    _sensor_mode_data = sensor_mode_data;

    //ensure that the inputParams is the same one in the awb and ae anzlyer
    //function
    _rkiq_compositor->setAiqInputParams(this->getAiqInputParams());
    LOGD("@%s : reqId %d", __FUNCTION__, this->getAiqInputParams().ptr() ? this->getAiqInputParams()->reqId : -1);

    ret = _isp->get_frame_softime (sof_tim);
    XCAM_FAIL_RETURN (WARNING, ret == XCAM_RETURN_NO_ERROR, ret, "get sof time failed");
    ret = _isp->get_vcm_time (&vcm_tim);
    XCAM_FAIL_RETURN (WARNING, ret == XCAM_RETURN_NO_ERROR, ret, "get vcm time failed");

    if (!_rkiq_compositor->set_sensor_mode_data (&_sensor_mode_data)) {
        XCAM_LOG_WARNING ("AIQ configure 3a failed");
        return XCAM_RETURN_ERROR_AIQ;
    }

    if (!_rkiq_compositor->set_frame_softime (sof_tim)) {
        XCAM_LOG_WARNING ("AIQ set vcm time failed");
        return XCAM_RETURN_ERROR_AIQ;
    }

    if (!_rkiq_compositor->set_vcm_time (&vcm_tim)) {
        XCAM_LOG_WARNING ("AIQ set vcm time failed");
        return XCAM_RETURN_ERROR_AIQ;
    }

    if (!_rkiq_compositor->set_effect_ispparams (isp_params)) {
        XCAM_LOG_WARNING ("AIQ set effect isp params failed");
        return XCAM_RETURN_ERROR_AIQ;
    }

    if (!_rkiq_compositor->set_3a_stats (xcam_isp_stats)) {
        XCAM_LOG_WARNING ("Aiq compositor set 3a stats failed");
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    return ret;
}

XCamReturn
X3aAnalyzerRKiq::post_3a_analyze (X3aResultList &results)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = _rkiq_compositor->integrate (results);
    XCAM_FAIL_RETURN (WARNING, ret == XCAM_RETURN_NO_ERROR, ret, "AIQ integrate 3A results failed");

    _rkiq_compositor->setAiqInputParams(NULL);
    return XCAM_RETURN_NO_ERROR;
}

};
