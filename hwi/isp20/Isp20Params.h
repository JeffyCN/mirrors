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

#ifndef _CAM_HW_ISP20_PARAMS_H_
#define _CAM_HW_ISP20_PARAMS_H_

#include "rk_aiq_pool.h"
#include "rkisp2-config.h"
#include "rkispp-config.h"

namespace RkCam {

#define ISP20PARAM_SUBM (0x2)

typedef struct AntiTmoFlicker_s {
    int preFrameNum;
    bool FirstChange;
    int FirstChangeNum;
    bool FirstChangeDone;
    int FirstChangeDoneNum;
} AntiTmoFlicker_t;


enum params_type {
    ISP_PARAMS,
    ISPP_PARAMS,
};

class IspParamsAssembler {
public:
    explicit IspParamsAssembler(const char* name);
    virtual ~IspParamsAssembler();
    void addReadyCondition(uint32_t cond);
    void rmReadyCondition(uint32_t cond);
    XCamReturn queue(cam3aResultList& results);
    XCamReturn queue(SmartPtr<cam3aResult>& result);
    XCamReturn deQueOne(cam3aResultList& results, uint32_t& frame_id);
    void forceReady(sint32_t frame_id);
    bool ready();
    void reset();
    XCamReturn start();
    void stop();
private:
    XCAM_DEAD_COPY(IspParamsAssembler);
    XCamReturn queue_locked(SmartPtr<cam3aResult>& result);
    void reset_locked();
    typedef struct {
        bool ready;
        uint64_t flags;
        cam3aResultList params;
    } params_t;
    // <frameId, result lists>
    std::map<sint32_t, params_t> mParamsMap;
    Mutex mParamsMutex;
    sint32_t mLatestReadyFrmId;
    uint64_t mReadyMask;
    uint32_t mReadyNums;
    std::string mName;
    // <result_type, maskId>
    std::map<uint32_t, uint64_t> mCondMaskMap;
    uint8_t mCondNum;
    static uint32_t MAX_PENDING_PARAMS;
    cam3aResultList mInitParamsList;
    bool started;
};

class Isp20Params {
public:
    explicit Isp20Params() : _last_pp_module_init_ens(0)
        , _force_isp_module_ens(0)
        , _force_ispp_module_ens(0)
        , _force_module_flags(0)
    {   AntiTmoFlicker.preFrameNum = 0;
        AntiTmoFlicker.FirstChange = false;
        AntiTmoFlicker.FirstChangeNum = 0;
        AntiTmoFlicker.FirstChangeDone = false;
        AntiTmoFlicker.FirstChangeDoneNum = 0;
        _working_mode = RK_AIQ_WORKING_MODE_ISP_HDR3;
    };
    virtual ~Isp20Params() {};
    virtual void setCamPhyId(int phyId) {
        _CamPhyId = phyId;
    }

    virtual XCamReturn checkIsp20Params(struct isp2x_isp_params_cfg& isp_cfg);
    void set_working_mode(int mode);
    void setModuleStatus(rk_aiq_module_id_t mId, bool en);
    void getModuleStatus(rk_aiq_module_id_t mId, bool& en);
    void hdrtmoGetLumaInfo(rk_aiq_luma_params_t * Next, rk_aiq_luma_params_t *Cur,
                           s32 frameNum, s32 PixelNumBlock, float blc, float *luma);
    void hdrtmoGetAeInfo(RKAiqAecExpInfo_t* Next, RKAiqAecExpInfo_t* Cur, s32 frameNum, float* expo);
    s32 hdrtmoPredictK(float* luma, float* expo, s32 frameNum, PredictKPara_t *TmoPara);
    bool hdrtmoSceneStable(sint32_t frameId, int IIRMAX, int IIR, int SetWeight, s32 frameNum, float *LumaDeviation, float StableThr);
#if 0
    void forceOverwriteAiqIsppCfg(struct rkispp_params_cfg& pp_cfg,
                                  SmartPtr<RkAiqIspParamsProxy> aiq_meas_results,
                                  SmartPtr<RkAiqIspParamsProxy> aiq_other_results);
    void forceOverwriteAiqIspCfg(struct isp2x_isp_params_cfg& isp_cfg,
                                 SmartPtr<RkAiqIspParamsProxy> aiq_results,
                                 SmartPtr<RkAiqIspParamsProxy> aiq_other_results);
#endif
    template<typename T>
    XCamReturn merge_results(cam3aResultList &results, T &isp_cfg);
    XCamReturn get_tnr_cfg_params(cam3aResultList &results, struct rkispp_params_tnrcfg &tnr_cfg);
    //XCamReturn get_nr_cfg_params(cam3aResultList &results, struct rkispp_params_nrcfg &nr_cfg);
    XCamReturn get_fec_cfg_params(cam3aResultList &results, struct rkispp_params_feccfg &fec_cfg);
    virtual XCamReturn merge_isp_results(cam3aResultList &results, void* isp_cfg);
protected:
    XCAM_DEAD_COPY(Isp20Params);

    template<class T>
    void convertAiqAeToIsp20Params(T& isp_cfg,
                                   const rk_aiq_isp_aec_meas_t& aec_meas);
    template<class T>
    void convertAiqHistToIsp20Params(T& isp_cfg,
                                     const rk_aiq_isp_hist_meas_t& hist_meas);
    template<class T>
    void convertAiqAwbToIsp20Params(T& isp_cfg,
                                    const rk_aiq_awb_stat_cfg_v200_t& awb_meas,
                                    bool awb_cfg_udpate);
    template<class T>
    void convertAiqAwbGainToIsp20Params(T& isp_cfg,
                                        const rk_aiq_wb_gain_t& awb_gain, const rk_aiq_isp_blc_t &blc,
                                        bool awb_gain_update);
    template<class T>
    void convertAiqMergeToIsp20Params(T& isp_cfg,
                                      const rk_aiq_isp_merge_t& amerge_data);
    template<class T>
    void convertAiqTmoToIsp20Params(T& isp_cfg,
                                    const rk_aiq_isp_tmo_t& atmo_data);
    template<class T>
    void convertAiqAdehazeToIsp20Params(T& isp_cfg,
                                        const rk_aiq_isp_dehaze_t& dhaze                     );
    template<class T>
    void convertAiqAgammaToIsp20Params(T& isp_cfg,
                                       const AgammaProcRes_t& gamma_out_cfg);
    template<class T>
    void convertAiqAdegammaToIsp20Params(T& isp_cfg,
                                         const AdegammaProcRes_t& degamma_cfg);

    template<class T>
    void convertAiqAdemosaicToIsp20Params(T& isp_cfg, rk_aiq_isp_debayer_t &demosaic);

    template<class T>
    void convertAiqLscToIsp20Params(T& isp_cfg,
                                    const rk_aiq_lsc_cfg_t& lsc);
    template<class T>
    void convertAiqBlcToIsp20Params(T& isp_cfg, rk_aiq_isp_blc_t &blc);

    template<class T>
    void convertAiqDpccToIsp20Params(T& isp_cfg, rk_aiq_isp_dpcc_t &dpcc);

    template<class T>
    void convertAiqCcmToIsp20Params(T& isp_cfg,
                                    const rk_aiq_ccm_cfg_t& ccm);
    template<class T>
    void convertAiqA3dlutToIsp20Params(T& isp_cfg,
                                       const rk_aiq_lut3d_cfg_t& lut3d_cfg);
    template<class T>
    void convertAiqCpToIsp20Params(T& isp_cfg,
                                   const rk_aiq_acp_params_t& lut3d_cfg);
    template<class T>
    void convertAiqIeToIsp20Params(T& isp_cfg,
                                   const rk_aiq_isp_ie_t& ie_cfg);
    template<class T>
    void convertAiqRawnrToIsp20Params(T& isp_cfg,
                                      rk_aiq_isp_rawnr_t& rawnr);
    template<typename T>
    void convertAiqTnrToIsp20Params(T& pp_cfg,
                                    rk_aiq_isp_tnr_t& tnr);
    template<typename T>
    void convertAiqUvnrToIsp20Params(T& pp_cfg,
                                     rk_aiq_isp_uvnr_t& uvnr);
    template<typename T>
    void convertAiqYnrToIsp20Params(T& pp_cfg,
                                    rk_aiq_isp_ynr_t& ynr);
    template<typename T>
    void convertAiqSharpenToIsp20Params(T& pp_cfg,
                                        rk_aiq_isp_sharpen_t& sharp, rk_aiq_isp_edgeflt_t& edgeflt);
    template<class T>
    void convertAiqAfToIsp20Params(T& isp_cfg,
                                   const rk_aiq_isp_af_meas_t& af_data, bool af_cfg_udpate);
    template<class T>
    void convertAiqGainToIsp20Params(T& isp_cfg,
                                     rk_aiq_isp_gain_t& gain);
    template<class T>
    void convertAiqAldchToIsp20Params(T& isp_cfg,
                                      const rk_aiq_isp_ldch_t& ldch_cfg);
    template<typename T>
    void convertAiqFecToIsp20Params(T& pp_cfg,
                                    rk_aiq_isp_fec_t& fec);
    template<class T>
    void convertAiqGicToIsp20Params(T& isp_cfg,
                                    const rk_aiq_isp_gic_t& gic_cfg);
    template<typename T>
    void convertAiqOrbToIsp20Params(T& pp_cfg,
                                    rk_aiq_isp_orb_t& orb);
    bool getModuleForceFlag(int module_id);
    void setModuleForceFlagInverse(int module_id);
    bool getModuleForceEn(int module_id);
    void updateIspModuleForceEns(u64 module_ens);
    void updateIsppModuleForceEns(u32 module_ens);
    uint32_t _last_pp_module_init_ens;
    u64 _force_isp_module_ens;
    u32 _force_ispp_module_ens;
    u64 _force_module_flags;
    int _working_mode;
    int _CamPhyId;
    AntiTmoFlicker_t AntiTmoFlicker;
    Mutex _mutex;

    virtual bool convert3aResultsToIspCfg(SmartPtr<cam3aResult> &result, void* isp_cfg_p);
    SmartPtr<cam3aResult> get_3a_result (cam3aResultList &results, int32_t type);
    // std::map<int, std::list<SmartPtr<cam3aResult>>> _cam3aConfig;
    SmartPtr<cam3aResult> mBlcResult;
};
};
#endif
