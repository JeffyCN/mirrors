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
#include "ae/rk_aiq_uapi_ae_int.h"
#include "awb/rk_aiq_uapiv2_camgroup_awb_int.h"
#include "accm/rk_aiq_uapi_accm_int.h"
#include "a3dlut/rk_aiq_uapi_a3dlut_int.h"
#include "agamma/rk_aiq_uapi_agamma_int.h"
#include "amerge/rk_aiq_uapi_amerge_int.h"
#include "adrc/rk_aiq_uapi_adrc_int.h"
#include "adehaze/rk_aiq_uapi_adehaze_int.h"
#include "algo_camgroup_handlers/RkAiqCamGroupAgicHandle.h"
#include "aynr3/rk_aiq_uapi_aynr_int_v3.h"
#include "acnr2/rk_aiq_uapi_acnr_int_v2.h"
#include "abayer2dnr2/rk_aiq_uapi_abayer2dnr_int_v2.h"
#include "ablc/rk_aiq_uapi_ablc_int.h"
#include "aldch/rk_aiq_uapi_aldch_int.h"
#include "adebayer/rk_aiq_uapi_adebayer_int.h"
#include "alsc/rk_aiq_uapi_alsc_int.h"
#include "adpcc/rk_aiq_uapi_adpcc_int.h"

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
    XCamReturn getSyncTestAttr (Uapi_AecSyncTest_t* pSyncTestAttr);
    XCamReturn queryExpInfo(Uapi_ExpQueryInfo_t* pExpQueryInfo);

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

    mutable std::atomic<bool> updateExpSwAttrV2;
    mutable std::atomic<bool> updateLinExpAttrV2;
    mutable std::atomic<bool> updateHdrExpAttrV2;

    mutable std::atomic<bool> updateLinAeRouteAttr;
    mutable std::atomic<bool> updateHdrAeRouteAttr;
    mutable std::atomic<bool> updateSyncTestAttr;
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
};

// accm
class RkAiqCamGroupAccmHandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAccmHandleInt(RkAiqAlgoDesComm* des,
                                        RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_ccm_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_ccm_attrib_t));
    };
    virtual ~RkAiqCamGroupAccmHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_ccm_attrib_t att);
    XCamReturn getAttrib(rk_aiq_ccm_attrib_t *att);
    XCamReturn queryCcmInfo(rk_aiq_ccm_querry_info_t* ccm_querry_info);

protected:

private:
    // TODO
    rk_aiq_ccm_attrib_t mCurAtt;
    rk_aiq_ccm_attrib_t mNewAtt;
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
    XCamReturn setAttrib(rk_aiq_lut3d_attrib_t att);
    XCamReturn getAttrib(rk_aiq_lut3d_attrib_t *att);
    XCamReturn query3dlutInfo(rk_aiq_lut3d_querry_info_t* lut3d_querry_info);

protected:

private:
    // TODO
    rk_aiq_lut3d_attrib_t mCurAtt;
    rk_aiq_lut3d_attrib_t mNewAtt;
};

// agamma
class RkAiqCamGroupAgammaHandleInt:
    virtual public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAgammaHandleInt(RkAiqAlgoDesComm* des,
                                          RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_gamma_attr_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_gamma_attr_t));
    };
    virtual ~RkAiqCamGroupAgammaHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_gamma_attr_t att);
    XCamReturn getAttrib(rk_aiq_gamma_attr_t *att);

protected:

private:
    // TODO
    rk_aiq_gamma_attr_t mCurAtt;
    rk_aiq_gamma_attr_t mNewAtt;
};

// amerge
class RkAiqCamGroupAmergeHandleInt:
    virtual public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAmergeHandleInt(RkAiqAlgoDesComm* des,
                                          RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        memset(&mCurAtt, 0, sizeof(mergeAttr_t));
        memset(&mNewAtt, 0, sizeof(mergeAttr_t));
    };
    virtual ~RkAiqCamGroupAmergeHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(mergeAttr_t att);
    XCamReturn getAttrib(mergeAttr_t *att);

protected:

private:
    // TODO
    mergeAttr_t mCurAtt;
    mergeAttr_t mNewAtt;
};

// adrc
class RkAiqCamGroupAdrcHandleInt:
    virtual public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAdrcHandleInt(RkAiqAlgoDesComm* des,
                                        RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        memset(&mCurAtt, 0, sizeof(drcAttr_t));
        memset(&mNewAtt, 0, sizeof(drcAttr_t));
    };
    virtual ~RkAiqCamGroupAdrcHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(drcAttr_t att);
    XCamReturn getAttrib(drcAttr_t *att);

protected:

private:
    // TODO
    drcAttr_t mCurAtt;
    drcAttr_t mNewAtt;
};

// adehaze
class RkAiqCamGroupAdhazHandleInt:
    virtual public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAdhazHandleInt(RkAiqAlgoDesComm* des,
                                         RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        memset(&mCurAtt, 0, sizeof(adehaze_sw_V2_t));
        memset(&mNewAtt, 0, sizeof(adehaze_sw_V2_t));
    };
    virtual ~RkAiqCamGroupAdhazHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttrib(adehaze_sw_V2_t att);
    XCamReturn getAttrib(adehaze_sw_V2_t *att);
protected:

private:
    // TODO
    adehaze_sw_V2_t mCurAtt;
    adehaze_sw_V2_t mNewAtt;
};


// aynr
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

protected:

private:

    rk_aiq_ynr_attrib_v3_t mCurAtt;
    rk_aiq_ynr_attrib_v3_t mNewAtt;
    rk_aiq_ynr_strength_v3_t mCurStrength;
    rk_aiq_ynr_strength_v3_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
};

// acnr v2
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
protected:

private:
    // TODO
    rk_aiq_cnr_attrib_v2_t mCurAtt;
    rk_aiq_cnr_attrib_v2_t mNewAtt;
    rk_aiq_cnr_strength_v2_t mCurStrength;
    rk_aiq_cnr_strength_v2_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
};


// aynr v2
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
protected:

private:
    // TODO
    rk_aiq_bayer2dnr_attrib_v2_t mCurAtt;
    rk_aiq_bayer2dnr_attrib_v2_t mNewAtt;
    rk_aiq_bayer2dnr_strength_v2_t mCurStrength;
    rk_aiq_bayer2dnr_strength_v2_t mNewStrength;
    mutable std::atomic<bool>  updateStrength;
};


// ablc
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
    XCamReturn setAttrib(rk_aiq_blc_attrib_t *att);
    XCamReturn getAttrib(rk_aiq_blc_attrib_t *att);
protected:

private:

    rk_aiq_blc_attrib_t mCurAtt;
    rk_aiq_blc_attrib_t mNewAtt;
};

// asharp
class RkAiqCamGroupAsharpV4HandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAsharpV4HandleInt(RkAiqAlgoDesComm* des,
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
    virtual ~RkAiqCamGroupAsharpV4HandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_sharp_attrib_v4_t *att);
    XCamReturn getAttrib(rk_aiq_sharp_attrib_v4_t *att);
    XCamReturn setStrength(rk_aiq_sharp_strength_v4_t *pStrength);
    XCamReturn getStrength(rk_aiq_sharp_strength_v4_t *pStrength);

protected:

private:

    rk_aiq_sharp_attrib_v4_t mCurAtt;
    rk_aiq_sharp_attrib_v4_t mNewAtt;
    rk_aiq_sharp_strength_v4_t mCurStrength;
    rk_aiq_sharp_strength_v4_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
};

class RkAiqCamGroupAbayertnrV2HandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAbayertnrV2HandleInt(RkAiqAlgoDesComm* des,
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
    virtual ~RkAiqCamGroupAbayertnrV2HandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_bayertnr_attrib_v2_t *att);
    XCamReturn getAttrib(rk_aiq_bayertnr_attrib_v2_t *att);
    XCamReturn setStrength(rk_aiq_bayertnr_strength_v2_t *pStrength);
    XCamReturn getStrength(rk_aiq_bayertnr_strength_v2_t *pStrength);

protected:

private:

    rk_aiq_bayertnr_attrib_v2_t mCurAtt;
    rk_aiq_bayertnr_attrib_v2_t mNewAtt;
    rk_aiq_bayertnr_strength_v2_t mCurStrength;
    rk_aiq_bayertnr_strength_v2_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
};

// aldch
class RkAiqCamGroupAldchHandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAldchHandleInt(RkAiqAlgoDesComm* des,
                                         RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_ldch_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_ldch_attrib_t));
    };
    virtual ~RkAiqCamGroupAldchHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_ldch_attrib_t att);
    XCamReturn getAttrib(rk_aiq_ldch_attrib_t *att);

protected:

private:
    // TODO
    rk_aiq_ldch_attrib_t mCurAtt;
    rk_aiq_ldch_attrib_t mNewAtt;
};

// adebayer
class RkAiqCamGroupAdebayerHandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAdebayerHandleInt(RkAiqAlgoDesComm* des,
                                            RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        memset(&mCurAtt, 0, sizeof(adebayer_attrib_t));
        memset(&mNewAtt, 0, sizeof(adebayer_attrib_t));
    };
    virtual ~RkAiqCamGroupAdebayerHandleInt() {
        RkAiqCamgroupHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(adebayer_attrib_t att);
    XCamReturn getAttrib(adebayer_attrib_t *att);

protected:

private:
    // TODO
    adebayer_attrib_t mCurAtt;
    adebayer_attrib_t mNewAtt;
};

// lsc
class RkAiqCamGroupAlscHandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAlscHandleInt(RkAiqAlgoDesComm* des,
                                        RkAiqCamGroupManager* camGroupMg)
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

// dpcc
class RkAiqCamGroupAdpccHandleInt:
    public RkAiqCamgroupHandle {
public:
    explicit RkAiqCamGroupAdpccHandleInt(RkAiqAlgoDesComm* des,
                                         RkAiqCamGroupManager* camGroupMg)
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
    XCamReturn setAttrib(rk_aiq_gain_attrib_v2_t *att);
    XCamReturn getAttrib(rk_aiq_gain_attrib_v2_t *att);
protected:

private:
    // TODO
    rk_aiq_gain_attrib_v2_t mCurAtt;
    rk_aiq_gain_attrib_v2_t mNewAtt;

};


};

#endif
