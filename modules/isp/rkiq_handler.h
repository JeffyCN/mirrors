/*
 * aiq_handler.h - AIQ handler
 *
 *  Copyright (c) 2014-2015  Intel Corporation
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

#ifndef XCAM_AIQ_HANDLER_H
#define XCAM_AIQ_HANDLER_H

#include "xcam_utils.h"
#include "handler_interface.h"
#include "x3a_statistics_queue.h"
#include "ia_types.h"
#include "ia_aiq_types.h"
#include "ia_cmc_parser.h"
#include "ia_mkn_encoder.h"
#include "ia_aiq.h"
#include "ia_coordinate.h"
#include <dynamic_algorithms_libs_loader.h>
#include "x3a_analyzer_rkiq.h"
#include "rkaiq.h"
#include "awb_state_machine.h"
#include "af_state_machine.h"
#include "ae_state_machine.h"
#include "x3a_meta_result.h"

#include <isp10_engine.h>

typedef struct ia_isp_t ia_isp;

namespace XCam {

class RKiqCompositor;
struct IspInputParameters;


class IaIspAdaptor {
public:
    explicit IaIspAdaptor()
        : _handle (NULL)
    {}
    virtual ~IaIspAdaptor() {}

    virtual bool init (
        const ia_binary_data *cpf,
        unsigned int max_width,
        unsigned int max_height,
        ia_cmc_t *cmc,
        ia_mkn *mkn) = 0;
    virtual bool convert_statistics (
        void *statistics,
        ia_aiq_rgbs_grid **out_rgbs_grid,
        ia_aiq_af_grid **out_af_grid) = 0;
    virtual bool run (
        const IspInputParameters *isp_input_params,
        ia_binary_data *output_data) = 0;

private:
    XCAM_DEAD_COPY (IaIspAdaptor);

protected:
    ia_isp *_handle;
};

class AiqAeHandler
    : public AeHandler
{
    friend class RKiqCompositor;
private:
    struct AiqAeResult {
        ia_aiq_ae_results                 ae_result;
        ia_aiq_ae_exposure_result         ae_exp_ret;
        ia_aiq_exposure_parameters        aiq_exp_param;
        ia_aiq_exposure_sensor_parameters sensor_exp_param;
        ia_aiq_hist_weight_grid           weight_grid;
        ia_aiq_flash_parameters           flash_param;

        AiqAeResult();
        void copy (ia_aiq_ae_results *result);

        XCAM_DEAD_COPY (AiqAeResult);
    };

public:
    explicit AiqAeHandler (X3aAnalyzerRKiq *analyzer, SmartPtr<RKiqCompositor> &aiq_compositor);
    ~AiqAeHandler () {}

    bool is_started () const {
        return _started;
    }

    bool set_description (struct rkisp_sensor_mode_data *sensor_mode_data);

    AecResult_t *get_result () {
        return &_result;
    }

    X3aAnalyzerRKiq* get3aAnalyzer() { return _analyzer; };
    XCamReturn processAeMetaResults(AecResult_t aec_results, X3aResultList &output);

    //virtual functions from AnalyzerHandler
    virtual XCamReturn analyze (X3aResultList &output, bool first = false);

    // virtual functions from AeHandler
    virtual XCamFlickerMode get_flicker_mode ();
    virtual int64_t get_current_exposure_time ();
    virtual float get_current_exposure_time_us ();
    virtual double get_current_analog_gain ();
    virtual double get_max_analog_gain ();

    XCamReturn set_RGBS_weight_grid (ia_aiq_rgbs_grid **out_rgbs_grid);
    XCamReturn set_hist_weight_grid (ia_aiq_hist_weight_grid **out_weight_grid);
    XCamReturn dump_hist_weight_grid (const ia_aiq_hist_weight_grid *weight_grid);
    XCamReturn dump_RGBS_grid (const ia_aiq_rgbs_grid *rgbs_grid);

private:
    bool ensure_ia_parameters ();
    bool ensure_ae_mode ();
    bool ensure_ae_metering_mode ();
    bool ensure_ae_priority_mode ();
    bool ensure_ae_flicker_mode ();
    bool ensure_ae_manual ();
    bool ensure_ae_ev_shift ();

    void adjust_ae_speed (
        ia_aiq_exposure_sensor_parameters &cur_res,
        ia_aiq_exposure_parameters &cur_aiq_exp,
        const ia_aiq_exposure_sensor_parameters &last_res, double ae_speed);
    void adjust_ae_limitation (ia_aiq_exposure_sensor_parameters &cur_res,
                               ia_aiq_exposure_parameters &cur_aiq_exp);
    bool manual_control_result (
        ia_aiq_exposure_sensor_parameters &cur_res,
        ia_aiq_exposure_parameters &cur_aiq_exp,
        const ia_aiq_exposure_sensor_parameters &last_res);

    SmartPtr<X3aResult> pop_result ();

    static void convert_xcam_window_to_ia (const XCam3AWindow &window, ia_rectangle &ia_window);

private:
    XCAM_DEAD_COPY (AiqAeHandler);

protected:
    SmartPtr<RKiqCompositor>           _aiq_compositor;
    /* AIQ */
    ia_rectangle                      _ia_ae_window;
    ia_aiq_exposure_sensor_descriptor _sensor_descriptor;
    ia_aiq_ae_manual_limits           _manual_limits;

    XCamAeParam                       _input;

    /* result */
    AecResult_t                       _result;
    rk_aiq_ae_results                 _rkaiq_result;
    uint32_t                          _calculate_period;
    bool                              _started;
private:
    AiqInputParams                    _latestInputParams;
    X3aAnalyzerRKiq*                  _analyzer;
    /**
     * To be handled by the AE state machine
     */
    SmartPtr<RkAEStateMachine>     mAeState;      /**< AE state machine */

    SmartPtr<X3aResult> mLastestAeresult;

};

class AiqAwbHandler
    : public AwbHandler
{
    friend class RKiqCompositor;
public:
    explicit AiqAwbHandler (X3aAnalyzerRKiq *analyzer, SmartPtr<RKiqCompositor> &aiq_compositor);
    ~AiqAwbHandler () {}

    virtual XCamReturn analyze (X3aResultList &output, bool first = false);
    XCamReturn processAwbMetaResults(CamIA10_AWB_Result_t awb_results, X3aResultList &output);

    // virtual functions from AwbHandler
    virtual uint32_t get_current_estimate_cct ();

    CamIA10_AWB_Result_t *get_result () {
        return &_result;
    }
    bool is_started () const {
        return _started;
    }

private:
    bool ensure_ia_parameters ();
    bool ensure_awb_mode ();
    void adjust_speed (const ia_aiq_awb_results &last_ret);

    XCAM_DEAD_COPY (AiqAwbHandler);

protected:
    SmartPtr<RKiqCompositor>     _aiq_compositor;
    /*aiq*/
    XCamAwbParam                _input;
    ia_aiq_awb_manual_cct_range _cct_range;

    X3aAnalyzerRKiq*                  _analyzer;
    /**
     * To be handled by the AWB state machine
     */
    /**< AWB state machine */
    SmartPtr<RkAWBStateMachine>    mAwbState;

    CamIA10_AWB_Result_t                 _result;
    rk_aiq_awb_results                 _rkaiq_result;
    CamIA10_AWB_Result_t          _history_result;
    bool                        _started;
};

class AiqAfHandler
    : public AfHandler
{
public:
    explicit AiqAfHandler (SmartPtr<RKiqCompositor> &aiq_compositor);
    ~AiqAfHandler () {}

    XCamReturn processAfMetaResults(XCam3aResultFocus af_results, X3aResultList &output);
    virtual XCamReturn analyze (X3aResultList &output, bool first = false);

private:
    XCAM_DEAD_COPY (AiqAfHandler);

protected:
    rk_aiq_af_results                 _rkaiq_result;
    SmartPtr<RkAFStateMachine>      mAfState;
    SmartPtr<RKiqCompositor>        _aiq_compositor;
};

class AiqCommonHandler
    : public CommonHandler
{
    friend class RKiqCompositor;
public:
    explicit AiqCommonHandler (SmartPtr<RKiqCompositor> &aiq_compositor);
    ~AiqCommonHandler ();

    virtual XCamReturn analyze (X3aResultList &output, bool first = false);
    ia_aiq_gbce_results *get_gbce_result () {
        return _gbce_result;
    }
    XCamColorEffect get_color_effect() {
        return _params.color_effect;
    }
    XCamReturn processToneMapsMetaResults(CamerIcIspGocConfig_t goc, X3aResultList &output);
    XCamReturn processMiscMetaResults(X3aResultList &output);

private:
    XCamReturn initTonemaps();
    XCamReturn fillTonemapCurve(CamerIcIspGocConfig_t goc, AiqInputParams* inputParams, CameraMetadata* metadata);
    XCAM_DEAD_COPY (AiqCommonHandler);
    // for tonemaps result
    uint32_t mMaxCurvePoints; /*!< Cache for max curve points for tonemap */
    float   *mRGammaLut;      /*!< [(P_IN, P_OUT), (P_IN, P_OUT), ..] */
    float   *mGGammaLut;      /*!< [(P_IN, P_OUT), (P_IN, P_OUT), ..] */
    float   *mBGammaLut;      /*!< [(P_IN, P_OUT), (P_IN, P_OUT), ..] */

protected:
    SmartPtr<RKiqCompositor>     _aiq_compositor;
    ia_aiq_gbce_results        *_gbce_result;
};

class RKiqCompositor {
public:
    explicit RKiqCompositor ();
    ~RKiqCompositor ();

    void set_ae_handler (SmartPtr<AiqAeHandler> &handler);
    void set_awb_handler (SmartPtr<AiqAwbHandler> &handler);
    void set_af_handler (SmartPtr<AiqAfHandler> &handler);
    void set_common_handler (SmartPtr<AiqCommonHandler> &handler);

    void set_frame_use (ia_aiq_frame_use value) {
        _frame_use = value;
    }
    void set_size (uint32_t width, uint32_t height) {
        _width = width;
        _height = height;
    }
    void get_size (uint32_t &out_width, uint32_t &out_height) const {
        out_width = _width;
        out_height = _height;
    }
    void set_framerate (double framerate) {
        _framerate = framerate;
    }
    double get_framerate () {
        return _framerate;
    }
    bool open (ia_binary_data &cpf);
    void close ();

    bool init_dynamic_config ();
    bool set_sensor_mode_data (struct isp_supplemental_sensor_mode_data *sensor_mode);
    struct CamIA10_SensorModeData &get_sensor_mode_data() { return _ia_stat.sensor_mode; };
    bool set_3a_stats (SmartPtr<X3aIspStatistics> &stats);
    struct CamIA10_Stats& get_3a_ia10_stats () { return _ia_stat; };
    struct cifisp_stat_buffer& get_3a_isp_stats () { return _isp_stats; };
    bool set_vcm_time (struct rk_cam_vcm_tim *vcm_tim);
    bool set_frame_softime (int64_t sof_tim);
    bool set_effect_ispparams (struct rkisp_parameters& isp_params);

    ia_aiq  * get_handle () {
        return _ia_handle;
    }
    ia_aiq_frame_use get_frame_use () const {
        return _frame_use;
    }

    XCamReturn integrate (  X3aResultList &results);

    SmartPtr<X3aResult> generate_3a_configs (struct rkisp_parameters *parameters);
    void convert_window_to_ia (const XCam3AWindow &window, ia_rectangle &ia_window);
    XCamReturn convert_color_effect (IspInputParameters &isp_input);

    double get_ae_ev_shift_unlock () {
        return _ae_handler->get_ev_shift_unlock();
    }
    void set_isp_ctrl_device(Isp10Engine* dev);
    SmartPtr<AiqInputParams> getAiqInputParams() { return _inputParams; };
    void setAiqInputParams(SmartPtr<AiqInputParams> inputParams) { _inputParams  = inputParams; };

private:
    XCamReturn apply_gamma_table (struct rkisp_parameters *isp_param);
    XCamReturn apply_night_mode (struct rkisp_parameters *isp_param);
    XCamReturn limit_nr_levels (struct rkisp_parameters *isp_param);
    double calculate_value_by_factor (double factor, double min, double mid, double max);

    XCAM_DEAD_COPY (RKiqCompositor);

public:
    Isp10Engine* _isp10_engine;

private:
    SmartPtr<X3aHandlerManager> _handle_manager;
    XCamAEDescription* _ae_desc;
    XCamAWBDescription* _awb_desc;
    XCamAFDescription* _af_desc;

    SmartPtr<AiqInputParams> _inputParams;

    SmartPtr<IaIspAdaptor>     _adaptor;
    //can't be SmartPtr because AiqAeHandler hold an RKiqCompositor SmartPtr ref,
    //if this point changes to SmartPtr, it will cause memory leak
    AiqAeHandler*     _ae_handler;
    AiqAwbHandler*    _awb_handler;
    AiqAfHandler*     _af_handler;
    AiqCommonHandler* _common_handler;
    ia_aiq                    *_ia_handle;
    ia_mkn                    *_ia_mkn;
    ia_aiq_pa_results         *_pa_result;
#ifdef HAVE_AIQ_2_7
    ia_aiq_sa_results         *_sa_result;
#endif
    ia_aiq_frame_use           _frame_use;
    ia_aiq_frame_params        _frame_params;

    struct cifisp_stat_buffer _isp_stats;
    struct CamIA10_Stats _ia_stat = {0};
    struct CamIA10_DyCfg _ia_dcfg;
    struct CamIA10_Results _ia_results = {0};
    struct CamIsp10ConfigSet _isp_cfg = {0};

    /*grids*/
    ;

    uint32_t                   _width;
    uint32_t                   _height;
    double                     _framerate;
    uint32_t                   _all_stats_meas_types;
};

};

#endif //XCAM_AIQ_HANDLER_H
