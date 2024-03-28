/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
 */
#ifndef _RK_AIQ_CAMGROUP_HANDLE_INT_V3_H_
#define _RK_AIQ_CAMGROUP_HANDLE_INT_V3_H_

#include "RkAiqCamgroupHandle.h"
#include "a3dlut/rk_aiq_uapi_a3dlut_int.h"
#include "abayer2dnr2/rk_aiq_uapi_abayer2dnr_int_v2.h"
#include "abayer2dnrV23/rk_aiq_uapi_abayer2dnr_int_v23.h"
#include "abayertnr2/rk_aiq_uapi_abayertnr_int_v2.h"
#include "abayertnrV23/rk_aiq_uapi_abayertnr_int_v23.h"
#include "abayertnrV30/rk_aiq_uapi_abayertnr_int_v30.h"
#include "ablc/rk_aiq_uapi_ablc_int.h"
#include "ablcV32/rk_aiq_uapi_ablc_int_v32.h"
#include "accm/rk_aiq_uapi_accm_int.h"
#include "acnr2/rk_aiq_uapi_acnr_int_v2.h"
#include "acnrV30/rk_aiq_uapi_acnr_int_v30.h"
#include "acnrV31/rk_aiq_uapi_acnr_int_v31.h"
#include "adebayer/rk_aiq_uapi_adebayer_int.h"
#include "adehaze/rk_aiq_uapi_adehaze_int.h"
#include "adpcc/rk_aiq_uapi_adpcc_int.h"
#include "adrc/rk_aiq_uapi_adrc_int.h"
#include "ae/rk_aiq_uapi_ae_int.h"
#include "agamma/rk_aiq_uapi_agamma_int.h"
#include "aldch/rk_aiq_uapi_aldch_int.h"
#include "algo_camgroup_handlers/RkAiqCamGroupAgicHandle.h"
#include "algos/aldch/rk_aiq_uapi_aldch_v21_int.h"
#include "alsc/rk_aiq_uapi_alsc_int.h"
#include "amerge/rk_aiq_uapi_amerge_int.h"
#include "argbir/rk_aiq_uapi_argbir_int.h"
#include "asharp4/rk_aiq_uapi_asharp_int_v4.h"
#include "asharpV33/rk_aiq_uapi_asharp_int_v33.h"
#include "asharpV34/rk_aiq_uapi_asharp_int_v34.h"
#include "awb/rk_aiq_uapiv2_camgroup_awb_int.h"
#include "aynr3/rk_aiq_uapi_aynr_int_v3.h"
#include "aynrV22/rk_aiq_uapi_aynr_int_v22.h"
#include "ayuvmeV1/rk_aiq_uapi_ayuvme_int_v1.h"

using namespace XCam;

namespace RkCam {
// ae
class RkAiqCamGroupAeHandleInt:
    public RkAiqCamgroupHandle {

public:
    explicit RkAiqCamGroupAeHandleInt(RkAiqAlgoDesComm* des, RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        memset(&mCurExpSwAttrV2, 0, sizeof(Uapi_ExpSwAttrV2_t));
        memset(&mNewExpSwAttrV2, 0, sizeof(Uapi_ExpSwAttrV2_t));
        memset(&mCurLinExpAttrV2, 0, sizeof(Uapi_LinExpAttrV2_t));
        memset(&mNewLinExpAttrV2, 0, sizeof(Uapi_LinExpAttrV2_t));
        memset(&mCurHdrExpAttrV2, 0, sizeof(Uapi_HdrExpAttrV2_t));
        memset(&mNewHdrExpAttrV2, 0, sizeof(Uapi_HdrExpAttrV2_t));

        memset(&mCurLinAeRouteAttr, 0, sizeof(Uapi_LinAeRouteAttr_t));
        memset(&mNewLinAeRouteAttr, 0, sizeof(Uapi_LinAeRouteAttr_t));
        memset(&mCurHdrAeRouteAttr, 0, sizeof(Uapi_HdrAeRouteAttr_t));
        memset(&mNewHdrAeRouteAttr, 0, sizeof(Uapi_HdrAeRouteAttr_t));
        memset(&mCurAecSyncTestAttr, 0, sizeof(Uapi_AecSyncTest_t));
        memset(&mNewAecSyncTestAttr, 0, sizeof(Uapi_AecSyncTest_t));

        updateExpSwAttrV2 = false;
        updateLinExpAttrV2 = false;
        updateHdrExpAttrV2 = false;
        updateLinAeRouteAttr = false;
        updateHdrAeRouteAttr = false;
        updateSyncTestAttr = false;

    };
    virtual ~RkAiqCamGroupAeHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);

    // TODO: calibv2
    XCamReturn setExpSwAttr(Uapi_ExpSwAttrV2_t ExpSwAttr);
    XCamReturn getExpSwAttr(Uapi_ExpSwAttrV2_t* pExpSwAttr);
    XCamReturn setLinExpAttr(Uapi_LinExpAttrV2_t LinExpAttr);
    XCamReturn getLinExpAttr(Uapi_LinExpAttrV2_t* pLinExpAttr);
    XCamReturn setHdrExpAttr(Uapi_HdrExpAttrV2_t HdrExpAttr);
    XCamReturn getHdrExpAttr (Uapi_HdrExpAttrV2_t* pHdrExpAttr);

    XCamReturn setLinAeRouteAttr(Uapi_LinAeRouteAttr_t LinAeRouteAttr);
    XCamReturn getLinAeRouteAttr(Uapi_LinAeRouteAttr_t* pLinAeRouteAttr);
    XCamReturn setHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t HdrAeRouteAttr);
    XCamReturn getHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr);

    XCamReturn setSyncTestAttr(Uapi_AecSyncTest_t SyncTestAttr);
    XCamReturn getSyncTestAttr(Uapi_AecSyncTest_t* pSyncTestAttr);
    XCamReturn queryExpInfo(Uapi_ExpQueryInfo_t* pExpQueryInfo);
    XCamReturn setAecStatsCfg(Uapi_AecStatsCfg_t AecStatsCfg);
    XCamReturn getAecStatsCfg(Uapi_AecStatsCfg_t* pAecStatsCfg);

protected:

private:

    // TODO: calibv2
    Uapi_ExpSwAttrV2_t  mCurExpSwAttrV2;
    Uapi_ExpSwAttrV2_t  mNewExpSwAttrV2;
    Uapi_LinExpAttrV2_t mCurLinExpAttrV2;
    Uapi_LinExpAttrV2_t mNewLinExpAttrV2;
    Uapi_HdrExpAttrV2_t mCurHdrExpAttrV2;
    Uapi_HdrExpAttrV2_t mNewHdrExpAttrV2;

    Uapi_LinAeRouteAttr_t mCurLinAeRouteAttr;
    Uapi_LinAeRouteAttr_t mNewLinAeRouteAttr;
    Uapi_HdrAeRouteAttr_t mCurHdrAeRouteAttr;
    Uapi_HdrAeRouteAttr_t mNewHdrAeRouteAttr;
    Uapi_AecSyncTest_t    mCurAecSyncTestAttr;
    Uapi_AecSyncTest_t    mNewAecSyncTestAttr;
    Uapi_AecStatsCfg_t    mCurAecStatsCfg;
    Uapi_AecStatsCfg_t    mNewAecStatsCfg;

    mutable std::atomic<bool> updateExpSwAttrV2;
    mutable std::atomic<bool> updateLinExpAttrV2;
    mutable std::atomic<bool> updateHdrExpAttrV2;

    mutable std::atomic<bool> updateLinAeRouteAttr;
    mutable std::atomic<bool> updateHdrAeRouteAttr;
    mutable std::atomic<bool> updateSyncTestAttr;
    mutable std::atomic<bool> updateAecStatsCfg;
};

// awb
class RkAiqCamGroupAwbHandleInt:
    public RkAiqCamgroupHandle {

public:
    explicit RkAiqCamGroupAwbHandleInt(RkAiqAlgoDesComm* des,
                                       RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        memset(&mCurWbV21Attr, 0, sizeof(mCurWbV21Attr));
        memset(&mCurWbOpModeAttr, 0, sizeof(mCurWbOpModeAttr));
        mCurWbOpModeAttr.mode = RK_AIQ_WB_MODE_MAX;
        memset(&mCurWbMwbAttr, 0, sizeof(mCurWbMwbAttr));
        memset(&mCurWbAwbAttr, 0, sizeof(mCurWbAwbAttr));
        memset(&mCurWbAwbWbGainAdjustAttr, 0, sizeof(mCurWbAwbWbGainAdjustAttr));
        memset(&mCurWbAwbWbGainOffsetAttr, 0, sizeof(mCurWbAwbWbGainOffsetAttr));
        memset(&mCurWbAwbMultiWindowAttr, 0, sizeof(mCurWbAwbMultiWindowAttr));
        memset(&mNewWbV21Attr, 0, sizeof(mNewWbV21Attr));
        memset(&mNewWbOpModeAttr, 0, sizeof(mNewWbOpModeAttr));
        mNewWbOpModeAttr.mode = RK_AIQ_WB_MODE_MAX;
        memset(&mNewWbMwbAttr, 0, sizeof(mNewWbMwbAttr));
        memset(&mNewWbAwbAttr, 0, sizeof(mNewWbAwbAttr));
        memset(&mNewWbAwbWbGainAdjustAttr, 0, sizeof(mNewWbAwbWbGainAdjustAttr));
        memset(&mNewWbAwbWbGainOffsetAttr, 0, sizeof(mNewWbAwbWbGainOffsetAttr));
        memset(&mNewWbAwbMultiWindowAttr, 0, sizeof(mNewWbAwbMultiWindowAttr));
        updateWbV21Attr = false;
        updateWbOpModeAttr = false;
        updateWbMwbAttr = false;
        updateWbAwbAttr = false;
        updateWbAwbWbGainAdjustAttr = false;
        updateWbAwbWbGainOffsetAttr = false;
        updateWbAwbMultiWindowAttr = false;
        // isp32
        memset(&mCurWbV32Attr, 0, sizeof(rk_aiq_uapiV2_wbV32_attrib_t));
        memset(&mNewWbV32Attr, 0, sizeof(rk_aiq_uapiV2_wbV32_attrib_t));
        memset(&mCurWbV32AwbMultiWindowAttr, 0, sizeof(rk_aiq_uapiV2_wbV32_awb_mulWindow_t));
        memset(&mNewWbV32AwbMultiWindowAttr, 0, sizeof(rk_aiq_uapiV2_wbV32_awb_mulWindow_t));
        updateWbV32Attr = false;
        updateWbV32AwbMultiWindowAttr = false;
    };
    virtual ~RkAiqCamGroupAwbHandleInt() {
        // free wbGainAdjust.lutAll from rk_aiq_uapiV2_awb_GetAwbGainAdjust in rk_aiq_uapiv2_awb_int.cpp
        for(int i = 0; i < mNewWbV21Attr.stAuto.wbGainAdjust.lutAll_len; i++) {
            if(mNewWbV21Attr.stAuto.wbGainAdjust.lutAll[i].cri_lut_out) {
                free(mNewWbV21Attr.stAuto.wbGainAdjust.lutAll[i].cri_lut_out);
                mNewWbV21Attr.stAuto.wbGainAdjust.lutAll[i].cri_lut_out = NULL;
            }
            if(mNewWbV21Attr.stAuto.wbGainAdjust.lutAll[i].ct_lut_out) {
                free(mNewWbV21Attr.stAuto.wbGainAdjust.lutAll[i].ct_lut_out);
                mNewWbV21Attr.stAuto.wbGainAdjust.lutAll[i].ct_lut_out = NULL;
            }
        }
        if (mNewWbV21Attr.stAuto.wbGainAdjust.lutAll) {
            free(mNewWbV21Attr.stAuto.wbGainAdjust.lutAll);
            mNewWbV21Attr.stAuto.wbGainAdjust.lutAll = NULL;
        }

        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);

    XCamReturn setWbV21Attrib(rk_aiq_uapiV2_wbV21_attrib_t att);
    XCamReturn getWbV21Attrib(rk_aiq_uapiV2_wbV21_attrib_t *att);
    XCamReturn getCct(rk_aiq_wb_cct_t *cct);
    XCamReturn queryWBInfo(rk_aiq_wb_querry_info_t *wb_querry_info );
    XCamReturn lock();
    XCamReturn unlock();
    XCamReturn setWbOpModeAttrib(rk_aiq_uapiV2_wb_opMode_t att);
    XCamReturn getWbOpModeAttrib(rk_aiq_uapiV2_wb_opMode_t *att);
    XCamReturn setMwbAttrib(rk_aiq_wb_mwb_attrib_t att);
    XCamReturn getMwbAttrib(rk_aiq_wb_mwb_attrib_t *att);
    XCamReturn setWbAwbWbGainAdjustAttrib(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t att);
    XCamReturn getWbAwbWbGainAdjustAttrib(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t *att);
    XCamReturn setWbAwbWbGainOffsetAttrib(rk_aiq_uapiV2_wb_awb_wbGainOffset_t att);
    XCamReturn getWbAwbWbGainOffsetAttrib(rk_aiq_uapiV2_wb_awb_wbGainOffset_t *att);
    XCamReturn setWbAwbMultiWindowAttrib(rk_aiq_uapiV2_wb_awb_mulWindow_t att);
    XCamReturn getWbAwbMultiWindowAttrib(rk_aiq_uapiV2_wb_awb_mulWindow_t *att);
    XCamReturn getAlgoStat(rk_tool_awb_stat_res_full_t *awb_stat_algo);
    XCamReturn getStrategyResult(rk_tool_awb_strategy_result_t *awb_strategy_result);
    //isp32
    XCamReturn setWbV32Attrib(rk_aiq_uapiV2_wbV32_attrib_t att);
    XCamReturn getWbV32Attrib(rk_aiq_uapiV2_wbV32_attrib_t* att);
    XCamReturn setWbV32AwbMultiWindowAttrib(rk_aiq_uapiV2_wbV32_awb_mulWindow_t att);
    XCamReturn getWbV32AwbMultiWindowAttrib(rk_aiq_uapiV2_wbV32_awb_mulWindow_t* att);
    XCamReturn setWbV32IQAutoExtPara(const rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t* att);
    XCamReturn getWbV32IQAutoExtPara(rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t* att);
    XCamReturn setWbV32IQAutoPara(const rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t* att);
    XCamReturn getWbV32IQAutoPara(rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t* att);
protected:

private:
    // TODO
    rk_aiq_uapiV2_wbV21_attrib_t mCurWbV21Attr;
    rk_aiq_uapiV2_wbV21_attrib_t mNewWbV21Attr;
    rk_aiq_uapiV2_wb_opMode_t  mCurWbOpModeAttr;
    rk_aiq_uapiV2_wb_opMode_t  mNewWbOpModeAttr;
    rk_aiq_wb_mwb_attrib_t  mCurWbMwbAttr;
    rk_aiq_wb_mwb_attrib_t  mNewWbMwbAttr;
    rk_aiq_uapiV2_wbV20_awb_attrib_t  mCurWbAwbAttr;
    rk_aiq_uapiV2_wbV20_awb_attrib_t  mNewWbAwbAttr;
    rk_aiq_uapiV2_wb_awb_wbGainAdjust_t mCurWbAwbWbGainAdjustAttr;
    rk_aiq_uapiV2_wb_awb_wbGainAdjust_t mNewWbAwbWbGainAdjustAttr;
    rk_aiq_uapiV2_wb_awb_wbGainOffset_t mCurWbAwbWbGainOffsetAttr;
    rk_aiq_uapiV2_wb_awb_wbGainOffset_t mNewWbAwbWbGainOffsetAttr;
    rk_aiq_uapiV2_wb_awb_mulWindow_t mCurWbAwbMultiWindowAttr;
    rk_aiq_uapiV2_wb_awb_mulWindow_t mNewWbAwbMultiWindowAttr;
    mutable std::atomic<bool> updateWbV21Attr;
    mutable std::atomic<bool> updateWbOpModeAttr;
    mutable std::atomic<bool> updateWbMwbAttr;
    mutable std::atomic<bool> updateWbAwbAttr;
    mutable std::atomic<bool> updateWbAwbWbGainAdjustAttr;
    mutable std::atomic<bool> updateWbAwbWbGainOffsetAttr;
    mutable std::atomic<bool> updateWbAwbMultiWindowAttr;

    // isp32
    rk_aiq_uapiV2_wbV32_attrib_t mCurWbV32Attr;
    rk_aiq_uapiV2_wbV32_attrib_t mNewWbV32Attr;
    mutable std::atomic<bool> updateWbV32Attr;
    rk_aiq_uapiV2_wbV32_awb_mulWindow_t mCurWbV32AwbMultiWindowAttr;
    rk_aiq_uapiV2_wbV32_awb_mulWindow_t mNewWbV32AwbMultiWindowAttr;
    mutable std::atomic<bool> updateWbV32AwbMultiWindowAttr;
};

// accm
class RkAiqCamGroupAccmHandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAccmHandleInt(RkAiqAlgoDesComm* des,
                                        RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
#if RKAIQ_HAVE_CCM_V1
        memset(&mCurAtt, 0, sizeof(rk_aiq_ccm_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_ccm_attrib_t));
        memset(&mCurCalibAtt, 0, sizeof(rk_aiq_ccm_calib_attrib_t));
        memset(&mNewCalibAtt, 0, sizeof(rk_aiq_ccm_calib_attrib_t));
#endif
#if RKAIQ_HAVE_CCM_V2
        memset(&mCurAttV2, 0, sizeof(rk_aiq_ccm_v2_attrib_t));
        memset(&mNewAttV2, 0, sizeof(rk_aiq_ccm_v2_attrib_t));
        memset(&mCurCalibAttV2, 0, sizeof(rk_aiq_ccm_v2_calib_attrib_t));
        memset(&mNewCalibAttV2, 0, sizeof(rk_aiq_ccm_v2_calib_attrib_t));
#endif
#if RKAIQ_HAVE_CCM_V3
        memset(&mCurAttV3, 0, sizeof(rk_aiq_ccm_v3_attrib_t));
        memset(&mNewAttV3, 0, sizeof(rk_aiq_ccm_v3_attrib_t));
        memset(&mCurCalibAttV3, 0, sizeof(rk_aiq_ccm_v3_calib_attrib_t));
        memset(&mNewCalibAttV3, 0, sizeof(rk_aiq_ccm_v3_calib_attrib_t));
#endif
    };
    virtual ~RkAiqCamGroupAccmHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
#if RKAIQ_HAVE_CCM_V1
    XCamReturn setAttrib(const rk_aiq_ccm_attrib_t* att);
    XCamReturn getAttrib(rk_aiq_ccm_attrib_t *att);
    XCamReturn setIqParam(const rk_aiq_ccm_calib_attrib_t* att);
    XCamReturn getIqParam(rk_aiq_ccm_calib_attrib_t* att);
#endif
#if RKAIQ_HAVE_CCM_V2
    XCamReturn setAttribV2(const rk_aiq_ccm_v2_attrib_t* att);
    XCamReturn getAttribV2(rk_aiq_ccm_v2_attrib_t *att);
    XCamReturn setIqParamV2(const rk_aiq_ccm_v2_calib_attrib_t* att);
    XCamReturn getIqParamV2(rk_aiq_ccm_v2_calib_attrib_t* att);
#endif
#if RKAIQ_HAVE_CCM_V3
    XCamReturn setAttribV3(const rk_aiq_ccm_v3_attrib_t* att);
    XCamReturn getAttribV3(rk_aiq_ccm_v3_attrib_t *att);
    XCamReturn setIqParamV3(const rk_aiq_ccm_v3_calib_attrib_t* att);
    XCamReturn getIqParamV3(rk_aiq_ccm_v3_calib_attrib_t* att);
#endif
    XCamReturn queryCcmInfo(rk_aiq_ccm_querry_info_t* ccm_querry_info);

protected:

private:
    // TODO
#if RKAIQ_HAVE_CCM_V1
    rk_aiq_ccm_attrib_t          mCurAtt;
    rk_aiq_ccm_calib_attrib_t    mCurCalibAtt;
    rk_aiq_ccm_attrib_t          mNewAtt;
    rk_aiq_ccm_calib_attrib_t    mNewCalibAtt;
#endif
#if RKAIQ_HAVE_CCM_V2
    rk_aiq_ccm_v2_attrib_t       mCurAttV2;
    rk_aiq_ccm_v2_calib_attrib_t mCurCalibAttV2;
    rk_aiq_ccm_v2_attrib_t       mNewAttV2;
    rk_aiq_ccm_v2_calib_attrib_t mNewCalibAttV2;
#endif
#if RKAIQ_HAVE_CCM_V3
    rk_aiq_ccm_v3_attrib_t mCurAttV3;
    rk_aiq_ccm_v3_calib_attrib_t mCurCalibAttV3;
    rk_aiq_ccm_v3_attrib_t mNewAttV3;
    rk_aiq_ccm_v3_calib_attrib_t mNewCalibAttV3;
#endif
    mutable std::atomic<bool>    updateCalibAttr;
};

// a3dlut
class RkAiqCamGroupA3dlutHandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupA3dlutHandleInt(RkAiqAlgoDesComm* des,
                                          RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_lut3d_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_lut3d_attrib_t));
    };
    virtual ~RkAiqCamGroupA3dlutHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(const rk_aiq_lut3d_attrib_t* att);
    XCamReturn getAttrib(rk_aiq_lut3d_attrib_t *att);
    XCamReturn query3dlutInfo(rk_aiq_lut3d_querry_info_t* lut3d_querry_info);

protected:

private:
    // TODO
    rk_aiq_lut3d_attrib_t mCurAtt;
    rk_aiq_lut3d_attrib_t mNewAtt;
};

#if (USE_NEWSTRUCT == 0)
// agamma
class RkAiqCamGroupAgammaHandleInt:
    virtual public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAgammaHandleInt(RkAiqAlgoDesComm* des,
                                          RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
#if RKAIQ_HAVE_GAMMA_V10
        memset(&mCurAttV10, 0, sizeof(rk_aiq_gamma_v10_attr_t));
        memset(&mNewAttV10, 0, sizeof(rk_aiq_gamma_v10_attr_t));
#endif
#if RKAIQ_HAVE_GAMMA_V11
        memset(&mCurAttV11, 0, sizeof(rk_aiq_gamma_v11_attr_t));
        memset(&mNewAttV11, 0, sizeof(rk_aiq_gamma_v11_attr_t));
#endif
    };
    virtual ~RkAiqCamGroupAgammaHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
#if RKAIQ_HAVE_GAMMA_V10
    XCamReturn setAttribV10(const rk_aiq_gamma_v10_attr_t* att);
    XCamReturn getAttribV10(rk_aiq_gamma_v10_attr_t* att);
#endif
#if RKAIQ_HAVE_GAMMA_V11
    XCamReturn setAttribV11(const rk_aiq_gamma_v11_attr_t* att);
    XCamReturn getAttribV11(rk_aiq_gamma_v11_attr_t* att);
#endif
protected:
private:
#if RKAIQ_HAVE_GAMMA_V10
    rk_aiq_gamma_v10_attr_t mCurAttV10;
    rk_aiq_gamma_v10_attr_t mNewAttV10;
#endif
#if RKAIQ_HAVE_GAMMA_V11
    rk_aiq_gamma_v11_attr_t mCurAttV11;
    rk_aiq_gamma_v11_attr_t mNewAttV11;
#endif
};
#endif

// amerge
class RkAiqCamGroupAmergeHandleInt:
    virtual public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAmergeHandleInt(RkAiqAlgoDesComm* des,
                                          RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
#if RKAIQ_HAVE_MERGE_V10
        memset(&mCurAttV10, 0, sizeof(mergeAttrV10_t));
        memset(&mNewAttV10, 0, sizeof(mergeAttrV10_t));
#endif
#if RKAIQ_HAVE_MERGE_V11
        memset(&mCurAttV11, 0, sizeof(mergeAttrV11_t));
        memset(&mNewAttV11, 0, sizeof(mergeAttrV11_t));
#endif
#if RKAIQ_HAVE_MERGE_V12
        memset(&mCurAttV12, 0, sizeof(mergeAttrV12_t));
        memset(&mNewAttV12, 0, sizeof(mergeAttrV12_t));
#endif
    };
    virtual ~RkAiqCamGroupAmergeHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
#if RKAIQ_HAVE_MERGE_V10
    XCamReturn setAttribV10(const mergeAttrV10_t* att);
    XCamReturn getAttribV10(mergeAttrV10_t* att);
#endif
#if RKAIQ_HAVE_MERGE_V11
    XCamReturn setAttribV11(const mergeAttrV11_t* att);
    XCamReturn getAttribV11(mergeAttrV11_t* att);
#endif
#if RKAIQ_HAVE_MERGE_V12
    XCamReturn setAttribV12(const mergeAttrV12_t* att);
    XCamReturn getAttribV12(mergeAttrV12_t* att);
#endif

protected:
private:
// TODO
#if RKAIQ_HAVE_MERGE_V10
    mergeAttrV10_t mCurAttV10;
    mergeAttrV10_t mNewAttV10;
#endif
#if RKAIQ_HAVE_MERGE_V11
    mergeAttrV11_t mCurAttV11;
    mergeAttrV11_t mNewAttV11;
#endif
#if RKAIQ_HAVE_MERGE_V12
    mergeAttrV12_t mCurAttV12;
    mergeAttrV12_t mNewAttV12;
#endif
};
#if (USE_NEWSTRUCT == 0)
// adrc
class RkAiqCamGroupAdrcHandleInt:
    virtual public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAdrcHandleInt(RkAiqAlgoDesComm* des,
                                        RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
#if RKAIQ_HAVE_DRC_V10
        memset(&mCurAttV10, 0, sizeof(drcAttrV10_t));
        memset(&mNewAttV10, 0, sizeof(drcAttrV10_t));
#endif
#if RKAIQ_HAVE_DRC_V11
        memset(&mCurAttV11, 0, sizeof(drcAttrV11_t));
        memset(&mNewAttV11, 0, sizeof(drcAttrV11_t));
#endif
#if RKAIQ_HAVE_DRC_V12
        memset(&mCurAttV12, 0, sizeof(drcAttrV12_t));
        memset(&mNewAttV12, 0, sizeof(drcAttrV12_t));
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
        memset(&mCurAttV12Lite, 0, sizeof(drcAttrV12Lite_t));
        memset(&mNewAttV12Lite, 0, sizeof(drcAttrV12Lite_t));
#endif
#if RKAIQ_HAVE_DRC_V20
        memset(&mCurAttV20, 0, sizeof(drcAttrV20_t));
        memset(&mNewAttV20, 0, sizeof(drcAttrV20_t));
#endif
    };
    virtual ~RkAiqCamGroupAdrcHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
#if RKAIQ_HAVE_DRC_V10
    XCamReturn setAttribV10(const drcAttrV10_t* att);
    XCamReturn getAttribV10(drcAttrV10_t* att);
#endif
#if RKAIQ_HAVE_DRC_V11
    XCamReturn setAttribV11(const drcAttrV11_t* att);
    XCamReturn getAttribV11(drcAttrV11_t* att);
#endif
#if RKAIQ_HAVE_DRC_V12
    XCamReturn setAttribV12(const drcAttrV12_t* att);
    XCamReturn getAttribV12(drcAttrV12_t* att);
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    XCamReturn setAttribV12Lite(const drcAttrV12Lite_t* att);
    XCamReturn getAttribV12Lite(drcAttrV12Lite_t* att);
#endif
#if RKAIQ_HAVE_DRC_V20
    XCamReturn setAttribV20(const drcAttrV20_t* att);
    XCamReturn getAttribV20(drcAttrV20_t* att);
#endif

 protected:
 private:
// TODO
#if RKAIQ_HAVE_DRC_V10
    drcAttrV10_t mCurAttV10;
    drcAttrV10_t mNewAttV10;
#endif
#if RKAIQ_HAVE_DRC_V11
    drcAttrV11_t mCurAttV11;
    drcAttrV11_t mNewAttV11;
#endif
#if RKAIQ_HAVE_DRC_V12
    drcAttrV12_t mCurAttV12;
    drcAttrV12_t mNewAttV12;
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    drcAttrV12Lite_t mCurAttV12Lite;
    drcAttrV12Lite_t mNewAttV12Lite;
#endif
#if RKAIQ_HAVE_DRC_V20
    drcAttrV20_t mCurAttV20;
    drcAttrV20_t mNewAttV20;
#endif
};
#endif
// argbir
class RkAiqCamGroupArgbirHandleInt : virtual public RkAiqCamgroupHandle {
 public:
    explicit RkAiqCamGroupArgbirHandleInt(RkAiqAlgoDesComm* des, RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
        memset(&mCurAttV10, 0, sizeof(RgbirAttrV10_t));
        memset(&mNewAttV10, 0, sizeof(RgbirAttrV10_t));
#endif
    };
    virtual ~RkAiqCamGroupArgbirHandleInt() { RkAiqCamgroupHandle::deInit(); };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
    XCamReturn setAttribV10(const RgbirAttrV10_t* att);
    XCamReturn getAttribV10(RgbirAttrV10_t* att);
#endif

 protected:
 private:
// TODO
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
    RgbirAttrV10_t mCurAttV10;
    RgbirAttrV10_t mNewAttV10;
#endif
};
#if (USE_NEWSTRUCT == 0)
// adehaze
class RkAiqCamGroupAdehazeHandleInt : virtual public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAdehazeHandleInt(RkAiqAlgoDesComm* des, RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
#if RKAIQ_HAVE_DEHAZE_V10
        memset(&mCurAttV10, 0, sizeof(adehaze_sw_v10_t));
        memset(&mNewAttV10, 0, sizeof(adehaze_sw_v10_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO
        memset(&mCurAttV11, 0, sizeof(adehaze_sw_v11_t));
        memset(&mNewAttV11, 0, sizeof(adehaze_sw_v11_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V12
        memset(&mCurAttV12, 0, sizeof(adehaze_sw_v12_t));
        memset(&mNewAttV12, 0, sizeof(adehaze_sw_v12_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V14
        memset(&mCurAttV14, 0, sizeof(adehaze_sw_v14_t));
        memset(&mNewAttV14, 0, sizeof(adehaze_sw_v14_t));
#endif
    };
    virtual ~RkAiqCamGroupAdehazeHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
#if RKAIQ_HAVE_DEHAZE_V10
    XCamReturn setAttribV10(const adehaze_sw_v10_t* att);
    XCamReturn getAttribV10(adehaze_sw_v10_t* att);
#endif
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO
    XCamReturn setAttribV11(const adehaze_sw_v11_t* att);
    XCamReturn getAttribV11(adehaze_sw_v11_t* att);
#endif
#if RKAIQ_HAVE_DEHAZE_V12
    XCamReturn setAttribV12(const adehaze_sw_v12_t* att);
    XCamReturn getAttribV12(adehaze_sw_v12_t* att);
#endif
#if RKAIQ_HAVE_DEHAZE_V14
    XCamReturn setAttribV14(const adehaze_sw_v14_t* att);
    XCamReturn getAttribV14(adehaze_sw_v14_t* att);
#endif

 protected:
 private:
// TODO
#if RKAIQ_HAVE_DEHAZE_V10
    adehaze_sw_v10_t mCurAttV10;
    adehaze_sw_v10_t mNewAttV10;
#endif
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO
    adehaze_sw_v11_t mCurAttV11;
    adehaze_sw_v11_t mNewAttV11;
#endif
#if RKAIQ_HAVE_DEHAZE_V12
    adehaze_sw_v12_t mCurAttV12;
    adehaze_sw_v12_t mNewAttV12;
#endif
#if RKAIQ_HAVE_DEHAZE_V14
    adehaze_sw_v14_t mCurAttV14;
    adehaze_sw_v14_t mNewAttV14;
#endif
};
#endif
// aynr
#if RKAIQ_HAVE_YNR_V3

class RkAiqCamGroupAynrV3HandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAynrV3HandleInt(RkAiqAlgoDesComm* des,
                                          RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateAtt = false;
        updateStrength = false;
        memset(&mCurStrength, 0x00, sizeof(mCurStrength));
        mCurStrength.percent = 1.0;
        memset(&mNewStrength, 0x00, sizeof(mNewStrength));
        mNewStrength.percent = 1.0;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));
    };
    virtual ~RkAiqCamGroupAynrV3HandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttrib(rk_aiq_ynr_attrib_v3_t *att);
    XCamReturn getAttrib(rk_aiq_ynr_attrib_v3_t *att);
    XCamReturn setStrength(rk_aiq_ynr_strength_v3_t *pStrength);
    XCamReturn getStrength(rk_aiq_ynr_strength_v3_t *pStrength);
    XCamReturn getInfo(rk_aiq_ynr_info_v3_t *pInfo);

protected:

private:

    rk_aiq_ynr_attrib_v3_t mCurAtt;
    rk_aiq_ynr_attrib_v3_t mNewAtt;
    rk_aiq_ynr_strength_v3_t mCurStrength;
    rk_aiq_ynr_strength_v3_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
};
#endif

// acnr v2
#if RKAIQ_HAVE_CNR_V2

class RkAiqCamGroupAcnrV2HandleInt:
    virtual public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAcnrV2HandleInt(RkAiqAlgoDesComm* des,
                                          RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateStrength = false;
        updateAtt = false;
        memset(&mCurStrength, 0x00, sizeof(mCurStrength));
        mCurStrength.percent = 1.0;
        memset(&mNewStrength, 0x00, sizeof(mNewStrength));
        mNewStrength.percent = 1.0;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));
    };
    virtual ~RkAiqCamGroupAcnrV2HandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttrib(rk_aiq_cnr_attrib_v2_t *att);
    XCamReturn getAttrib(rk_aiq_cnr_attrib_v2_t *att);
    XCamReturn setStrength(rk_aiq_cnr_strength_v2_t *pStrength);
    XCamReturn getStrength(rk_aiq_cnr_strength_v2_t *pStrength);
    XCamReturn getInfo(rk_aiq_cnr_info_v2_t *pInfo);
protected:

private:
    // TODO
    rk_aiq_cnr_attrib_v2_t mCurAtt;
    rk_aiq_cnr_attrib_v2_t mNewAtt;
    rk_aiq_cnr_strength_v2_t mCurStrength;
    rk_aiq_cnr_strength_v2_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
};
#endif

// aynr v2
#if RKAIQ_HAVE_BAYER2DNR_V2

class RkAiqCamGroupAbayer2dnrV2HandleInt:
    virtual public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAbayer2dnrV2HandleInt(RkAiqAlgoDesComm* des,
            RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateStrength = false;
        updateAtt = false;
        memset(&mCurStrength, 0x00, sizeof(mCurStrength));
        memset(&mNewStrength, 0x00, sizeof(mNewStrength));
        mCurStrength.percent = 1.0;
        mNewStrength.percent = 1.0;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));

    };
    virtual ~RkAiqCamGroupAbayer2dnrV2HandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttrib(rk_aiq_bayer2dnr_attrib_v2_t *att);
    XCamReturn getAttrib(rk_aiq_bayer2dnr_attrib_v2_t *att);
    XCamReturn setStrength(rk_aiq_bayer2dnr_strength_v2_t *pStrength);
    XCamReturn getStrength(rk_aiq_bayer2dnr_strength_v2_t *pStrength);
    XCamReturn getInfo(rk_aiq_bayer2dnr_info_v2_t *pInfo);
protected:

private:
    // TODO
    rk_aiq_bayer2dnr_attrib_v2_t mCurAtt;
    rk_aiq_bayer2dnr_attrib_v2_t mNewAtt;
    rk_aiq_bayer2dnr_strength_v2_t mCurStrength;
    rk_aiq_bayer2dnr_strength_v2_t mNewStrength;
    mutable std::atomic<bool>  updateStrength;
};
#endif

// ablc
#if RKAIQ_HAVE_BLC_V1

class RkAiqCamGroupAblcHandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAblcHandleInt(RkAiqAlgoDesComm* des,
                                        RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateAtt = false;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));

    };
    virtual ~RkAiqCamGroupAblcHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttrib(const rk_aiq_blc_attrib_t *att);
    XCamReturn getAttrib(rk_aiq_blc_attrib_t *att);
    XCamReturn getInfo(rk_aiq_ablc_info_t *pInfo);
protected:

private:

    rk_aiq_blc_attrib_t mCurAtt;
    rk_aiq_blc_attrib_t mNewAtt;
};
#endif

// asharp
#if RKAIQ_HAVE_SHARP_V4

class RkAiqCamGroupAsharpV4HandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAsharpV4HandleInt(RkAiqAlgoDesComm* des, RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateAtt      = false;
        updateStrength = false;
        memset(&mCurStrength, 0x00, sizeof(mCurStrength));
        mCurStrength.percent = 1.0;
        memset(&mNewStrength, 0x00, sizeof(mNewStrength));
        mNewStrength.percent = 1.0;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));
    };
    virtual ~RkAiqCamGroupAsharpV4HandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_sharp_attrib_v4_t *att);
    XCamReturn getAttrib(rk_aiq_sharp_attrib_v4_t *att);
    XCamReturn setStrength(rk_aiq_sharp_strength_v4_t *pStrength);
    XCamReturn getStrength(rk_aiq_sharp_strength_v4_t *pStrength);
    XCamReturn getInfo(rk_aiq_sharp_info_v4_t *pInfo);

protected:

private:

    rk_aiq_sharp_attrib_v4_t mCurAtt;
    rk_aiq_sharp_attrib_v4_t mNewAtt;
    rk_aiq_sharp_strength_v4_t mCurStrength;
    rk_aiq_sharp_strength_v4_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
};
#endif

#if RKAIQ_HAVE_BAYERTNR_V2

class RkAiqCamGroupAbayertnrV2HandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAbayertnrV2HandleInt(RkAiqAlgoDesComm* des,
            RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateAtt      = false;
        updateStrength = false;
        memset(&mCurStrength, 0x00, sizeof(mCurStrength));
        mCurStrength.percent = 1.0;
        memset(&mNewStrength, 0x00, sizeof(mNewStrength));
        mNewStrength.percent = 1.0;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));
    };
    virtual ~RkAiqCamGroupAbayertnrV2HandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_bayertnr_attrib_v2_t *att);
    XCamReturn getAttrib(rk_aiq_bayertnr_attrib_v2_t *att);
    XCamReturn setStrength(rk_aiq_bayertnr_strength_v2_t *pStrength);
    XCamReturn getStrength(rk_aiq_bayertnr_strength_v2_t *pStrength);
    XCamReturn getInfo(rk_aiq_bayertnr_info_v2_t *pInfo);

protected:

private:

    rk_aiq_bayertnr_attrib_v2_t mCurAtt;
    rk_aiq_bayertnr_attrib_v2_t mNewAtt;
    rk_aiq_bayertnr_strength_v2_t mCurStrength;
    rk_aiq_bayertnr_strength_v2_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
};
#endif

// aldch
class RkAiqCamGroupAldchHandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAldchHandleInt(RkAiqAlgoDesComm* des, RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        memset(&mCurAtt, 0, sizeof(mCurAtt));
        memset(&mNewAtt, 0, sizeof(mNewAtt));
    };
    virtual ~RkAiqCamGroupAldchHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
#if (RKAIQ_HAVE_LDCH_V21)
    XCamReturn setAttrib(const rk_aiq_ldch_v21_attrib_t* att);
    XCamReturn getAttrib(rk_aiq_ldch_v21_attrib_t* att);
#else
    XCamReturn setAttrib(const rk_aiq_ldch_attrib_t* att);
    XCamReturn getAttrib(rk_aiq_ldch_attrib_t *att);
#endif

protected:

private:
    // TODO
#if (RKAIQ_HAVE_LDCH_V21)
    rk_aiq_ldch_v21_attrib_t mCurAtt;
    rk_aiq_ldch_v21_attrib_t mNewAtt;
#else
    rk_aiq_ldch_attrib_t mCurAtt;
    rk_aiq_ldch_attrib_t mNewAtt;
#endif
};

// adebayer
class RkAiqCamGroupAdebayerHandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAdebayerHandleInt(RkAiqAlgoDesComm* des, RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
#if RKAIQ_HAVE_DEBAYER_V1
        memset(&mCurAtt, 0, sizeof(mCurAtt));
        memset(&mNewAtt, 0, sizeof(mNewAtt));
#endif
#if RKAIQ_HAVE_DEBAYER_V2
        memset(&mCurAttV2, 0, sizeof(mCurAttV2));
        memset(&mNewAttV2, 0, sizeof(mNewAttV2));
#endif
#if RKAIQ_HAVE_DEBAYER_V2_LITE
        memset(&mCurAttV2Lite, 0, sizeof(mCurAttV2Lite));
        memset(&mNewAttV2Lite, 0, sizeof(mNewAttV2Lite));
#endif
#if RKAIQ_HAVE_DEBAYER_V3
        memset(&mCurAttV3, 0, sizeof(mCurAttV3));
        memset(&mNewAttV3, 0, sizeof(mNewAttV3));
#endif

    };
    virtual ~RkAiqCamGroupAdebayerHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
#if RKAIQ_HAVE_DEBAYER_V1
    XCamReturn setAttrib(adebayer_attrib_t att);
    XCamReturn getAttrib(adebayer_attrib_t *att);
#endif
#if RKAIQ_HAVE_DEBAYER_V2
    XCamReturn setAttribV2(adebayer_v2_attrib_t att);
    XCamReturn getAttribV2(adebayer_v2_attrib_t* att);
#endif
#if RKAIQ_HAVE_DEBAYER_V2_LITE
    XCamReturn setAttribV2(adebayer_v2lite_attrib_t att);
    XCamReturn getAttribV2(adebayer_v2lite_attrib_t* att);
#endif
#if RKAIQ_HAVE_DEBAYER_V3
    XCamReturn setAttribV3(adebayer_v3_attrib_t att);
    XCamReturn getAttribV3(adebayer_v3_attrib_t* att);
#endif

protected:

private:
    // TODO
#if RKAIQ_HAVE_DEBAYER_V1
    adebayer_attrib_t mCurAtt;
    adebayer_attrib_t mNewAtt;
#endif

#if RKAIQ_HAVE_DEBAYER_V2
    adebayer_v2_attrib_t mCurAttV2;
    adebayer_v2_attrib_t mNewAttV2;
#endif
#if RKAIQ_HAVE_DEBAYER_V2_LITE
    adebayer_v2lite_attrib_t mCurAttV2Lite;
    adebayer_v2lite_attrib_t mNewAttV2Lite;
#endif
#if RKAIQ_HAVE_DEBAYER_V3
    adebayer_v3_attrib_t mCurAttV3;
    adebayer_v3_attrib_t mNewAttV3;
#endif

};

// lsc
class RkAiqCamGroupAlscHandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAlscHandleInt(RkAiqAlgoDesComm* des, RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_lsc_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_lsc_attrib_t));
    };
    virtual ~RkAiqCamGroupAlscHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttrib(rk_aiq_lsc_attrib_t att);
    XCamReturn getAttrib(rk_aiq_lsc_attrib_t* att);
    XCamReturn queryLscInfo(rk_aiq_lsc_querry_info_t* lsc_querry_info);

protected:

private:
    // TODO
    rk_aiq_lsc_attrib_t mCurAtt;
    rk_aiq_lsc_attrib_t mNewAtt;
};
#if (USE_NEWSTRUCT == 0)
// dpcc
class RkAiqCamGroupAdpccHandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAdpccHandleInt(RkAiqAlgoDesComm* des, RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_dpcc_attrib_V20_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_dpcc_attrib_V20_t));
    };
    virtual ~RkAiqCamGroupAdpccHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttrib(rk_aiq_dpcc_attrib_V20_t att);
    XCamReturn getAttrib(rk_aiq_dpcc_attrib_V20_t* att);

protected:

private:
    rk_aiq_dpcc_attrib_V20_t mCurAtt;
    rk_aiq_dpcc_attrib_V20_t mNewAtt;
};
#endif
#if RKAIQ_HAVE_GAIN_V2

class RkAiqCamGroupAgainV2HandleInt:
    virtual public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAgainV2HandleInt(RkAiqAlgoDesComm* des,
                                           RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateAtt = false;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));

    };
    virtual ~RkAiqCamGroupAgainV2HandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttrib(const rk_aiq_gain_attrib_v2_t *att);
    XCamReturn getAttrib(rk_aiq_gain_attrib_v2_t *att);
    XCamReturn getInfo(rk_aiq_gain_info_v2_t *pInfo);
private:
    // TODO
    rk_aiq_gain_attrib_v2_t mCurAtt;
    rk_aiq_gain_attrib_v2_t mNewAtt;

};
#endif


// aynr
#if RKAIQ_HAVE_YNR_V22

class RkAiqCamGroupAynrV22HandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAynrV22HandleInt(RkAiqAlgoDesComm* des,
                                           RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateAtt = false;
        updateStrength = false;
        memset(&mCurStrength, 0x00, sizeof(mCurStrength));
        mCurStrength.percent = 1.0;
        memset(&mNewStrength, 0x00, sizeof(mNewStrength));
        mNewStrength.percent = 1.0;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));
    };
    virtual ~RkAiqCamGroupAynrV22HandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttrib(const rk_aiq_ynr_attrib_v22_t *att);
    XCamReturn getAttrib(rk_aiq_ynr_attrib_v22_t *att);
    XCamReturn setStrength(const rk_aiq_ynr_strength_v22_t *pStrength);
    XCamReturn getStrength(rk_aiq_ynr_strength_v22_t *pStrength);
    XCamReturn getInfo(rk_aiq_ynr_info_v22_t *pInfo);
protected:

private:

    rk_aiq_ynr_attrib_v22_t mCurAtt;
    rk_aiq_ynr_attrib_v22_t mNewAtt;
    rk_aiq_ynr_strength_v22_t mCurStrength;
    rk_aiq_ynr_strength_v22_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
};
#endif
#if RKAIQ_HAVE_YNR_V24

class RkAiqCamGroupAynrV24HandleInt : public RkAiqCamgroupHandle {
 public:
    explicit RkAiqCamGroupAynrV24HandleInt(RkAiqAlgoDesComm* des, RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateAtt      = false;
        updateStrength = false;
        memset(&mCurStrength, 0x00, sizeof(mCurStrength));
        mCurStrength.percent = 1.0;
        memset(&mNewStrength, 0x00, sizeof(mNewStrength));
        mNewStrength.percent = 1.0;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));
    };
    virtual ~RkAiqCamGroupAynrV24HandleInt() { RkAiqCamgroupHandle::deInit(); };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttrib(const rk_aiq_ynr_attrib_v24_t* att);
    XCamReturn getAttrib(rk_aiq_ynr_attrib_v24_t* att);
    XCamReturn setStrength(const rk_aiq_ynr_strength_v24_t* pStrength);
    XCamReturn getStrength(rk_aiq_ynr_strength_v24_t* pStrength);
    XCamReturn getInfo(rk_aiq_ynr_info_v24_t* pInfo);

 protected:
 private:
    rk_aiq_ynr_attrib_v24_t mCurAtt;
    rk_aiq_ynr_attrib_v24_t mNewAtt;
    rk_aiq_ynr_strength_v24_t mCurStrength;
    rk_aiq_ynr_strength_v24_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
};
#endif

// acnr v2
#if (RKAIQ_HAVE_CNR_V30 || RKAIQ_HAVE_CNR_V30_LITE)

class RkAiqCamGroupAcnrV30HandleInt:
    virtual public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAcnrV30HandleInt(RkAiqAlgoDesComm* des,
                                           RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateStrength = false;
        updateAtt = false;
        memset(&mCurStrength, 0x00, sizeof(mCurStrength));
        mCurStrength.percent = 1.0;
        memset(&mNewStrength, 0x00, sizeof(mNewStrength));
        mNewStrength.percent = 1.0;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));
    };
    virtual ~RkAiqCamGroupAcnrV30HandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttrib(const rk_aiq_cnr_attrib_v30_t *att);
    XCamReturn getAttrib(rk_aiq_cnr_attrib_v30_t *att);
    XCamReturn setStrength(const rk_aiq_cnr_strength_v30_t *pStrength);
    XCamReturn getStrength(rk_aiq_cnr_strength_v30_t *pStrength);
    XCamReturn getInfo(rk_aiq_cnr_info_v30_t *pInfo);
protected:

private:
    // TODO
    rk_aiq_cnr_attrib_v30_t mCurAtt;
    rk_aiq_cnr_attrib_v30_t mNewAtt;
    rk_aiq_cnr_strength_v30_t mCurStrength;
    rk_aiq_cnr_strength_v30_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
};
#endif


// aynr v2
#if RKAIQ_HAVE_BAYER2DNR_V23

class RkAiqCamGroupAbayer2dnrV23HandleInt:
    virtual public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAbayer2dnrV23HandleInt(RkAiqAlgoDesComm* des,
            RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateStrength = false;
        updateAtt = false;
        memset(&mCurStrength, 0x00, sizeof(mCurStrength));
        memset(&mNewStrength, 0x00, sizeof(mNewStrength));
        mCurStrength.percent = 1.0;
        mNewStrength.percent = 1.0;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));

    };
    virtual ~RkAiqCamGroupAbayer2dnrV23HandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttrib(const rk_aiq_bayer2dnr_attrib_v23_t *att);
    XCamReturn getAttrib(rk_aiq_bayer2dnr_attrib_v23_t *att);
    XCamReturn setStrength(const rk_aiq_bayer2dnr_strength_v23_t *pStrength);
    XCamReturn getStrength(rk_aiq_bayer2dnr_strength_v23_t *pStrength);
    XCamReturn getInfo(rk_aiq_bayer2dnr_info_v23_t *pInfo);
protected:

private:
    // TODO
    rk_aiq_bayer2dnr_attrib_v23_t mCurAtt;
    rk_aiq_bayer2dnr_attrib_v23_t mNewAtt;
    rk_aiq_bayer2dnr_strength_v23_t mCurStrength;
    rk_aiq_bayer2dnr_strength_v23_t mNewStrength;
    mutable std::atomic<bool>  updateStrength;
};
#endif


// ablc
#if RKAIQ_HAVE_BLC_V32 && !USE_NEWSTRUCT
class RkAiqCamGroupAblcV32HandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAblcV32HandleInt(RkAiqAlgoDesComm* des,
                                           RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateAtt = false;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));

    };
    virtual ~RkAiqCamGroupAblcV32HandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttrib(const rk_aiq_blc_attrib_V32_t *att);
    XCamReturn getAttrib(rk_aiq_blc_attrib_V32_t *att);
    XCamReturn getInfo(rk_aiq_blc_info_v32_t* pInfo);
protected:

private:

    rk_aiq_blc_attrib_V32_t mCurAtt;
    rk_aiq_blc_attrib_V32_t mNewAtt;
};
#endif

#if (RKAIQ_HAVE_BAYERTNR_V23 || RKAIQ_HAVE_BAYERTNR_V23_LITE)
#if (USE_NEWSTRUCT == 0)
class RkAiqCamGroupAbayertnrV23HandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAbayertnrV23HandleInt(RkAiqAlgoDesComm* des,
            RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateAtt      = false;
        updateStrength = false;
        updateAttLite  = false;
        memset(&mCurStrength, 0x00, sizeof(mCurStrength));
        mCurStrength.percent = 1.0;
        memset(&mNewStrength, 0x00, sizeof(mNewStrength));
        mNewStrength.percent = 1.0;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));
        memset(&mCurAttLite, 0x00, sizeof(mCurAttLite));
        memset(&mNewAttLite, 0x00, sizeof(mNewAttLite));
    };
    virtual ~RkAiqCamGroupAbayertnrV23HandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(const rk_aiq_bayertnr_attrib_v23_t *att);
    XCamReturn getAttrib(rk_aiq_bayertnr_attrib_v23_t *att);
    XCamReturn setAttribLite(const rk_aiq_bayertnr_attrib_v23L_t* att);
    XCamReturn getAttribLite(rk_aiq_bayertnr_attrib_v23L_t* att);
    XCamReturn setStrength(const rk_aiq_bayertnr_strength_v23_t *pStrength);
    XCamReturn getStrength(rk_aiq_bayertnr_strength_v23_t *pStrength);
    XCamReturn getInfo(rk_aiq_bayertnr_info_v23_t *pInfo);

protected:

private:

    rk_aiq_bayertnr_attrib_v23_t mCurAtt;
    rk_aiq_bayertnr_attrib_v23_t mNewAtt;
    rk_aiq_bayertnr_strength_v23_t mCurStrength;
    rk_aiq_bayertnr_strength_v23_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
    rk_aiq_bayertnr_attrib_v23L_t mCurAttLite;
    rk_aiq_bayertnr_attrib_v23L_t mNewAttLite;
    mutable std::atomic<bool> updateAttLite;
};
#endif
#endif

// asharp
#if (RKAIQ_HAVE_SHARP_V33 || RKAIQ_HAVE_SHARP_V33_LITE)

class RkAiqCamGroupAsharpV33HandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAsharpV33HandleInt(RkAiqAlgoDesComm* des, RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateAtt      = false;
        updateStrength = false;
        updateAttLite  = false;
        memset(&mCurStrength, 0x00, sizeof(mCurStrength));
        mCurStrength.percent = 1.0;
        memset(&mNewStrength, 0x00, sizeof(mNewStrength));
        mNewStrength.percent = 1.0;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));
        memset(&mCurAttLite, 0x00, sizeof(mCurAttLite));
        memset(&mNewAttLite, 0x00, sizeof(mNewAttLite));
    };
    virtual ~RkAiqCamGroupAsharpV33HandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(const rk_aiq_sharp_attrib_v33_t *att);
    XCamReturn getAttrib(rk_aiq_sharp_attrib_v33_t *att);
    XCamReturn setAttribLite(const rk_aiq_sharp_attrib_v33LT_t* att);
    XCamReturn getAttribLite(rk_aiq_sharp_attrib_v33LT_t* att);
    XCamReturn setStrength(const rk_aiq_sharp_strength_v33_t *pStrength);
    XCamReturn getStrength(rk_aiq_sharp_strength_v33_t *pStrength);
    XCamReturn getInfo(rk_aiq_sharp_info_v33_t *pStrength);

protected:

private:

    rk_aiq_sharp_attrib_v33_t mCurAtt;
    rk_aiq_sharp_attrib_v33_t mNewAtt;
    rk_aiq_sharp_strength_v33_t mCurStrength;
    rk_aiq_sharp_strength_v33_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
    rk_aiq_sharp_attrib_v33LT_t mCurAttLite;
    rk_aiq_sharp_attrib_v33LT_t mNewAttLite;
    mutable std::atomic<bool> updateAttLite;
};
#endif

#if (RKAIQ_HAVE_BAYERTNR_V30)

class RkAiqCamGroupAbayertnrV30HandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAbayertnrV30HandleInt(RkAiqAlgoDesComm* des,
            RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateAtt      = false;
        updateStrength = false;
        memset(&mCurStrength, 0x00, sizeof(mCurStrength));
        mCurStrength.percent = 1.0;
        memset(&mNewStrength, 0x00, sizeof(mNewStrength));
        mNewStrength.percent = 1.0;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));
    };
    virtual ~RkAiqCamGroupAbayertnrV30HandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(const rk_aiq_bayertnr_attrib_v30_t *att);
    XCamReturn getAttrib(rk_aiq_bayertnr_attrib_v30_t *att);
    XCamReturn setStrength(const rk_aiq_bayertnr_strength_v30_t *pStrength);
    XCamReturn getStrength(rk_aiq_bayertnr_strength_v30_t *pStrength);
    XCamReturn getInfo(rk_aiq_bayertnr_info_v30_t *pInfo);

protected:

private:

    rk_aiq_bayertnr_attrib_v30_t mCurAtt;
    rk_aiq_bayertnr_attrib_v30_t mNewAtt;
    rk_aiq_bayertnr_strength_v30_t mCurStrength;
    rk_aiq_bayertnr_strength_v30_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
};
#endif

// ayuvme
#if RKAIQ_HAVE_YUVME_V1

class RkAiqCamGroupAyuvmeV1HandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAyuvmeV1HandleInt(RkAiqAlgoDesComm* des,
                                            RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateAtt = false;
        updateStrength = false;
        memset(&mCurStrength, 0x00, sizeof(mCurStrength));
        mCurStrength.percent = 1.0;
        memset(&mNewStrength, 0x00, sizeof(mNewStrength));
        mNewStrength.percent = 1.0;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));
    };
    virtual ~RkAiqCamGroupAyuvmeV1HandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttrib(const rk_aiq_yuvme_attrib_v1_t *att);
    XCamReturn getAttrib(rk_aiq_yuvme_attrib_v1_t *att);
    XCamReturn setStrength(const rk_aiq_yuvme_strength_v1_t *pStrength);
    XCamReturn getStrength(rk_aiq_yuvme_strength_v1_t *pStrength);
    XCamReturn getInfo(rk_aiq_yuvme_info_v1_t *pInfo);
protected:

private:

    rk_aiq_yuvme_attrib_v1_t mCurAtt;
    rk_aiq_yuvme_attrib_v1_t mNewAtt;
    rk_aiq_yuvme_strength_v1_t mCurStrength;
    rk_aiq_yuvme_strength_v1_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
};
#endif

// acnr v31
#if (RKAIQ_HAVE_CNR_V31)

class RkAiqCamGroupAcnrV31HandleInt:
    virtual public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAcnrV31HandleInt(RkAiqAlgoDesComm* des,
                                           RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateStrength = false;
        updateAtt = false;
        memset(&mCurStrength, 0x00, sizeof(mCurStrength));
        mCurStrength.percent = 1.0;
        memset(&mNewStrength, 0x00, sizeof(mNewStrength));
        mNewStrength.percent = 1.0;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));
    };
    virtual ~RkAiqCamGroupAcnrV31HandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttrib(const rk_aiq_cnr_attrib_v31_t *att);
    XCamReturn getAttrib(rk_aiq_cnr_attrib_v31_t *att);
    XCamReturn setStrength(const rk_aiq_cnr_strength_v31_t *pStrength);
    XCamReturn getStrength(rk_aiq_cnr_strength_v31_t *pStrength);
    XCamReturn getInfo(rk_aiq_cnr_info_v31_t *pInfo);
protected:

private:
    // TODO
    rk_aiq_cnr_attrib_v31_t mCurAtt;
    rk_aiq_cnr_attrib_v31_t mNewAtt;
    rk_aiq_cnr_strength_v31_t mCurStrength;
    rk_aiq_cnr_strength_v31_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
};
#endif

// asharp
#if (RKAIQ_HAVE_SHARP_V34)

class RkAiqCamGroupAsharpV34HandleInt : public RkAiqCamgroupHandle {
 public:
    explicit RkAiqCamGroupAsharpV34HandleInt(RkAiqAlgoDesComm* des,
                                             RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateAtt      = false;
        updateStrength = false;
        memset(&mCurStrength, 0x00, sizeof(mCurStrength));
        mCurStrength.percent = 1.0;
        memset(&mNewStrength, 0x00, sizeof(mNewStrength));
        mNewStrength.percent = 1.0;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));
    };
    virtual ~RkAiqCamGroupAsharpV34HandleInt() { RkAiqCamgroupHandle::deInit(); };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(const rk_aiq_sharp_attrib_v34_t* att);
    XCamReturn getAttrib(rk_aiq_sharp_attrib_v34_t* att);
    XCamReturn setStrength(const rk_aiq_sharp_strength_v34_t* pStrength);
    XCamReturn getStrength(rk_aiq_sharp_strength_v34_t* pStrength);
    XCamReturn getInfo(rk_aiq_sharp_info_v34_t* pStrength);

 protected:
 private:
    rk_aiq_sharp_attrib_v34_t mCurAtt;
    rk_aiq_sharp_attrib_v34_t mNewAtt;
    rk_aiq_sharp_strength_v34_t mCurStrength;
    rk_aiq_sharp_strength_v34_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
};
#endif

}

#endif
