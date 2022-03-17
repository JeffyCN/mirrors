/*
 * RkAiqHandleInt.h
 *
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

#ifndef _RK_AIQ_HANDLE_INT_H_
#define _RK_AIQ_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "rk_aiq_algo_types_int.h"
#include "ae/rk_aiq_uapi_ae_int.h"
#include "awb/rk_aiq_uapi_awb_int.h"
#include "awb/rk_aiq_uapiv2_awb_int.h"
#include "adebayer/rk_aiq_uapi_adebayer_int.h"
#include "amerge/rk_aiq_uapi_amerge_int.h"
#include "atmo/rk_aiq_uapi_atmo_int.h"
#include "adrc/rk_aiq_uapi_adrc_int.h"
#include "alsc/rk_aiq_uapi_alsc_int.h"
#include "accm/rk_aiq_uapi_accm_int.h"
#include "a3dlut/rk_aiq_uapi_a3dlut_int.h"
#include "xcam_mutex.h"
#include "adehaze/rk_aiq_uapi_adehaze_int.h"
#include "agamma/rk_aiq_uapi_agamma_int.h"
#include "adegamma/rk_aiq_uapi_adegamma_int.h"
#include "ablc/rk_aiq_uapi_ablc_int.h"
#include "adpcc/rk_aiq_uapi_adpcc_int.h"
#include "anr/rk_aiq_uapi_anr_int.h"
#include "asharp/rk_aiq_uapi_asharp_int.h"
#include "agic/rk_aiq_uapi_agic_int.h"
#include "afec/rk_aiq_uapi_afec_int.h"
#include "af/rk_aiq_uapi_af_int.h"
#include "asd/rk_aiq_uapi_asd_int.h"
#include "aldch/rk_aiq_uapi_aldch_int.h"
#include "acp/rk_aiq_uapi_acp_int.h"
#include "aie/rk_aiq_uapi_aie_int.h"
#include "aeis/rk_aiq_uapi_aeis_int.h"
#include "arawnr/rk_aiq_uapi_abayernr_int_v1.h"
#include "aynr/rk_aiq_uapi_aynr_int_v1.h"
#include "auvnr/rk_aiq_uapi_auvnr_int_v1.h"
#include "amfnr/rk_aiq_uapi_amfnr_int_v1.h"
#include "again/rk_aiq_uapi_again_int.h"

#include "RkAiqSharedDataManager.h"
#include "rk_aiq_api_private.h"

namespace RkCam {

class RkAiqCore;

class RkAiqHandleIntCom : virtual public RkAiqHandle
{
public:
    explicit RkAiqHandleIntCom(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , updateAtt(false)
        , mUpdateCond(false) {};
    virtual ~RkAiqHandleIntCom() {};
protected:
    XCamReturn configInparamsCom(RkAiqAlgoCom* com, int type);
    void waitSignal();
    void sendSignal();
    XCam::Mutex mCfgMutex;
    bool updateAtt;
    XCam::Cond mUpdateCond;
};

#define RKAIQHANDLEINT(algo) \
    class RkAiq##algo##HandleInt: \
            virtual public RkAiq##algo##Handle, \
            virtual public RkAiqHandleIntCom { \
    public: \
        explicit RkAiq##algo##HandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore) \
                    : RkAiqHandle(des, aiqCore) \
                    , RkAiq##algo##Handle(des, aiqCore) \
                    , RkAiqHandleIntCom(des, aiqCore) {}; \
        virtual ~RkAiq##algo##HandleInt() { RkAiq##algo##Handle::deInit(); }; \
        virtual XCamReturn prepare(); \
        virtual XCamReturn preProcess(); \
        virtual XCamReturn processing(); \
        virtual XCamReturn postProcess(); \
    protected: \
        virtual void init(); \
        virtual void deInit() { RkAiq##algo##Handle::deInit(); }; \
    }

// define
//RKAIQHANDLEINT(Ae);
//RKAIQHANDLEINT(Awb);
//RKAIQHANDLEINT(Af);
//RKAIQHANDLEINT(Ahdr);
//RKAIQHANDLEINT(Anr);
//RKAIQHANDLEINT(Alsc);
//RKAIQHANDLEINT(Asharp);
//RKAIQHANDLEINT(Adhaz);
//RKAIQHANDLEINT(Asd);
//RKAIQHANDLEINT(Acp);
//RKAIQHANDLEINT(A3dlut);
//RKAIQHANDLEINT(Ablc);
//RKAIQHANDLEINT(Accm);
RKAIQHANDLEINT(Acgc);
//RKAIQHANDLEINT(Adebayer);
//RKAIQHANDLEINT(Adpcc);
// RKAIQHANDLEINT(Afec);
//RKAIQHANDLEINT(Agamma);
//RKAIQHANDLEINT(Agic);
//RKAIQHANDLEINT(Aie);
// RKAIQHANDLEINT(Aldch);
RKAIQHANDLEINT(Ar2y);
RKAIQHANDLEINT(Awdr);
RKAIQHANDLEINT(Aorb);
//RKAIQHANDLEINT(Aeis);
// ae
class RkAiqAeHandleInt:
    virtual public RkAiqAeHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAeHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAeHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore)
        , mPreResShared(nullptr)
        , mProcResShared(nullptr) {};
    virtual ~RkAiqAeHandleInt() {
        RkAiqAeHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();

    // TODO: calibv1
    XCamReturn setExpSwAttr(Uapi_ExpSwAttr_t ExpSwAttr);
    XCamReturn getExpSwAttr(Uapi_ExpSwAttr_t* pExpSwAttr);
    XCamReturn setLinExpAttr(Uapi_LinExpAttr_t LinExpAttr);
    XCamReturn getLinExpAttr(Uapi_LinExpAttr_t* pLinExpAttr);
    XCamReturn setHdrExpAttr(Uapi_HdrExpAttr_t HdrExpAttr);
    XCamReturn getHdrExpAttr (Uapi_HdrExpAttr_t* pHdrExpAttr);

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

    XCamReturn setIrisAttr(Uapi_IrisAttrV2_t IrisAttr);
    XCamReturn getIrisAttr (Uapi_IrisAttrV2_t* pIrisAttr);
    XCamReturn setSyncTestAttr(Uapi_AecSyncTest_t SyncTestAttr);
    XCamReturn getSyncTestAttr (Uapi_AecSyncTest_t* pSyncTestAttr);
    XCamReturn queryExpInfo(Uapi_ExpQueryInfo_t* pExpQueryInfo);
    XCamReturn setExpWinAttr(Uapi_ExpWin_t ExpWinAttr);
    XCamReturn getExpWinAttr(Uapi_ExpWin_t* pExpWinAttr);

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAeHandle::deInit();
    };
private:

    // TODO: calibv1
    Uapi_ExpSwAttr_t  mCurExpSwAttr;
    Uapi_ExpSwAttr_t  mNewExpSwAttr;
    Uapi_LinExpAttr_t mCurLinExpAttr;
    Uapi_LinExpAttr_t mNewLinExpAttr;
    Uapi_HdrExpAttr_t mCurHdrExpAttr;
    Uapi_HdrExpAttr_t mNewHdrExpAttr;

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
    Uapi_IrisAttrV2_t     mCurIrisAttr;
    Uapi_IrisAttrV2_t     mNewIrisAttr;
    Uapi_AecSyncTest_t    mCurAecSyncTestAttr;
    Uapi_AecSyncTest_t    mNewAecSyncTestAttr;
    Uapi_ExpWin_t         mCurExpWinAttr;
    Uapi_ExpWin_t         mNewExpWinAttr;

    bool updateExpSwAttr = false;
    bool updateLinExpAttr = false;
    bool updateHdrExpAttr = false;

    bool updateExpSwAttrV2 = false;
    bool updateLinExpAttrV2 = false;
    bool updateHdrExpAttrV2 = false;

    bool updateLinAeRouteAttr = false;
    bool updateHdrAeRouteAttr = false;
    bool updateIrisAttr   = false;
    bool updateSyncTestAttr = false;
    bool updateExpWinAttr = false;

    uint16_t updateAttr = 0;

    SmartPtr<RkAiqAlgoPreResAeIntShared> mPreResShared;
    SmartPtr<RkAiqAlgoProcResAeIntShared> mProcResShared;
};

// awb
class RkAiqAwbHandleInt:
    virtual public RkAiqAwbHandle,
    virtual public RkAiqHandleIntCom {
    friend class RkAiqAwbV21HandleInt;
public:
    explicit RkAiqAwbHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAwbHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore)
        , mProcResShared(nullptr) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_wb_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_wb_attrib_t));
        memset(&mCurWbV20Attr, 0, sizeof(mCurWbV20Attr));
        mCurWbOpModeAttr = RK_AIQ_WB_MODE_MAX;
        memset(&mCurWbMwbAttr, 0, sizeof(mCurWbMwbAttr));
        memset(&mCurWbAwbAttr, 0, sizeof(mCurWbAwbAttr));
        memset(&mCurWbAwbWbGainAdjustAttr, 0, sizeof(mCurWbAwbWbGainAdjustAttr));
        memset(&mCurWbAwbWbGainOffsetAttr, 0, sizeof(mCurWbAwbWbGainOffsetAttr));
        memset(&mCurWbAwbMultiWindowAttr, 0, sizeof(mCurWbAwbMultiWindowAttr));
        memset(&mNewWbV20Attr, 0, sizeof(mNewWbV20Attr));
        mNewWbOpModeAttr = RK_AIQ_WB_MODE_MAX;
        memset(&mNewWbMwbAttr, 0, sizeof(mNewWbMwbAttr));
        memset(&mNewWbAwbAttr, 0, sizeof(mNewWbAwbAttr));
        memset(&mNewWbAwbWbGainAdjustAttr, 0, sizeof(mNewWbAwbWbGainAdjustAttr));
        memset(&mNewWbAwbWbGainOffsetAttr, 0, sizeof(mNewWbAwbWbGainOffsetAttr));
        memset(&mNewWbAwbMultiWindowAttr, 0, sizeof(mNewWbAwbMultiWindowAttr));
        updateWbV20Attr = false;
        updateWbOpModeAttr = false;
        updateWbMwbAttr = false;
        updateWbAwbAttr = false;
        updateWbAwbWbGainAdjustAttr = false;
        updateWbAwbWbGainOffsetAttr = false;
        updateWbAwbMultiWindowAttr = false;
    };
    virtual ~RkAiqAwbHandleInt() {
        RkAiqAwbHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_wb_attrib_t att);
    XCamReturn getAttrib(rk_aiq_wb_attrib_t *att);
    XCamReturn getCct(rk_aiq_wb_cct_t *cct);
    XCamReturn queryWBInfo(rk_aiq_wb_querry_info_t *wb_querry_info );
    XCamReturn lock();
    XCamReturn unlock();
    XCamReturn setWbV20Attrib(rk_aiq_uapiV2_wbV20_attrib_t att);
    XCamReturn getWbV20Attrib(rk_aiq_uapiV2_wbV20_attrib_t *att);
    XCamReturn setWbOpModeAttrib(rk_aiq_wb_op_mode_t att);
    XCamReturn getWbOpModeAttrib(rk_aiq_wb_op_mode_t *att);
    XCamReturn setMwbAttrib(rk_aiq_wb_mwb_attrib_t att);
    XCamReturn getMwbAttrib(rk_aiq_wb_mwb_attrib_t *att);
    XCamReturn setAwbV20Attrib(rk_aiq_uapiV2_wbV20_awb_attrib_t att);
    XCamReturn getAwbV20Attrib(rk_aiq_uapiV2_wbV20_awb_attrib_t *att);
    XCamReturn setWbAwbWbGainAdjustAttrib(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t att);
    XCamReturn getWbAwbWbGainAdjustAttrib(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t *att);
    XCamReturn setWbAwbWbGainOffsetAttrib(CalibDbV2_Awb_gain_offset_cfg_t att);
    XCamReturn getWbAwbWbGainOffsetAttrib(CalibDbV2_Awb_gain_offset_cfg_t *att);
    XCamReturn setWbAwbMultiWindowAttrib(CalibDbV2_Awb_Mul_Win_t att);
    XCamReturn getWbAwbMultiWindowAttrib(CalibDbV2_Awb_Mul_Win_t *att);

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAwbHandle::deInit();
    };
private:
    // TODO
    rk_aiq_wb_attrib_t mCurAtt;
    rk_aiq_wb_attrib_t mNewAtt;
    //v2
    rk_aiq_uapiV2_wbV20_attrib_t mCurWbV20Attr;//v21 todo
    rk_aiq_uapiV2_wbV20_attrib_t mNewWbV20Attr;
    rk_aiq_wb_op_mode_t  mCurWbOpModeAttr;
    rk_aiq_wb_op_mode_t  mNewWbOpModeAttr;
    rk_aiq_wb_mwb_attrib_t  mCurWbMwbAttr;
    rk_aiq_wb_mwb_attrib_t  mNewWbMwbAttr;
    rk_aiq_uapiV2_wbV20_awb_attrib_t  mCurWbAwbAttr;
    rk_aiq_uapiV2_wbV20_awb_attrib_t  mNewWbAwbAttr;
    rk_aiq_uapiV2_wb_awb_wbGainAdjust_t mCurWbAwbWbGainAdjustAttr;
    rk_aiq_uapiV2_wb_awb_wbGainAdjust_t mNewWbAwbWbGainAdjustAttr;
    CalibDbV2_Awb_gain_offset_cfg_t mCurWbAwbWbGainOffsetAttr;
    CalibDbV2_Awb_gain_offset_cfg_t mNewWbAwbWbGainOffsetAttr;
    CalibDbV2_Awb_Mul_Win_t mCurWbAwbMultiWindowAttr;
    CalibDbV2_Awb_Mul_Win_t mNewWbAwbMultiWindowAttr;
    bool updateWbV20Attr;
    bool updateWbOpModeAttr;
    bool updateWbMwbAttr;
    bool updateWbAwbAttr;
    bool updateWbAwbWbGainAdjustAttr;
    bool updateWbAwbWbGainOffsetAttr;
    bool updateWbAwbMultiWindowAttr;
    SmartPtr<RkAiqAlgoProcResAwbIntShared> mProcResShared;
};

// af
class RkAiqAfHandleInt:
    virtual public RkAiqAfHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAfHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAfHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore)
        , mProcResShared(nullptr) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_af_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_af_attrib_t));
    };
    virtual ~RkAiqAfHandleInt() {
        RkAiqAfHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_af_attrib_t *att);
    XCamReturn getAttrib(rk_aiq_af_attrib_t *att);
    XCamReturn lock();
    XCamReturn unlock();
    XCamReturn Oneshot();
    XCamReturn ManualTriger();
    XCamReturn Tracking();
    XCamReturn setZoomPos(int zoom_pos);
    XCamReturn GetSearchPath(rk_aiq_af_sec_path_t* path);
    XCamReturn GetSearchResult(rk_aiq_af_result_t* result);

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAfHandle::deInit();
    };
private:
    // TODO
    rk_aiq_af_attrib_t mCurAtt;
    rk_aiq_af_attrib_t mNewAtt;
    bool isUpdateAttDone;

    SmartPtr<RkAiqAlgoProcResAfIntShared> mProcResShared;
};

class RkAiqAdebayerHandleInt:
    virtual public RkAiqAdebayerHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAdebayerHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAdebayerHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {};
    virtual ~RkAiqAdebayerHandleInt() {
        RkAiqAdebayerHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(adebayer_attrib_t att);
    XCamReturn getAttrib(adebayer_attrib_t *att);
protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAdebayerHandle::deInit();
    };
private:
    adebayer_attrib_t mCurAtt;
    adebayer_attrib_t mNewAtt;
};

// amerge
class RkAiqAmergeHandleInt:
    virtual public RkAiqAmergeHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAmergeHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAmergeHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {}
    virtual ~RkAiqAmergeHandleInt() {
        RkAiqAmergeHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    XCamReturn setAttrib(amerge_attrib_t att);
    XCamReturn getAttrib(amerge_attrib_t* att);
protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAmergeHandle::deInit();
    };
private:
    amerge_attrib_t mCurAtt;
    amerge_attrib_t mNewAtt;
};

// atmo
class RkAiqAtmoHandleInt:
    virtual public RkAiqAtmoHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAtmoHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAtmoHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {}
    virtual ~RkAiqAtmoHandleInt() {
        RkAiqAtmoHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    XCamReturn setAttrib(atmo_attrib_t att);
    XCamReturn getAttrib(atmo_attrib_t* att);
protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAtmoHandle::deInit();
    };
private:
    atmo_attrib_t mCurAtt;
    atmo_attrib_t mNewAtt;
};

class RkAiqAgicHandleInt:
    virtual public RkAiqAgicHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAgicHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAgicHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {};
    virtual ~RkAiqAgicHandleInt() {
        RkAiqAgicHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    XCamReturn setAttrib(agic_attrib_t att);
    XCamReturn getAttrib(agic_attrib_t *att);
protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAgicHandle::deInit();
    };
private:
    agic_attrib_t mCurAtt;
    agic_attrib_t mNewAtt;
};

// adehaze
class RkAiqAdhazHandleInt:
    virtual public RkAiqAdhazHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAdhazHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAdhazHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {};
    virtual ~RkAiqAdhazHandleInt() {
        RkAiqAdhazHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    // TODO add algo specific methords, this is a sample
    XCamReturn setSwAttrib(adehaze_sw_s att);
    XCamReturn getSwAttrib(adehaze_sw_s *att);

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAdhazHandle::deInit();
    };
private:
    // TODO
    adehaze_sw_V2_t mCurAtt;
    adehaze_sw_V2_t mNewAtt;
};


// agamma
class RkAiqAgammaHandleInt:
    virtual public RkAiqAgammaHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAgammaHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAgammaHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_gamma_attrib_V2_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_gamma_attrib_V2_t));
    };
    virtual ~RkAiqAgammaHandleInt() {
        RkAiqAgammaHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_gamma_attrib_V2_t att);
    XCamReturn getAttrib(rk_aiq_gamma_attrib_V2_t *att);
    //XCamReturn queryLscInfo(rk_aiq_lsc_querry_info_t *lsc_querry_info );

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAgammaHandle::deInit();
    };
private:
    // TODO
    rk_aiq_gamma_attrib_V2_t mCurAtt;
    rk_aiq_gamma_attrib_V2_t mNewAtt;
};

// adegamma
class RkAiqAdegammaHandleInt:
    virtual public RkAiqAdegammaHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAdegammaHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAdegammaHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_degamma_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_degamma_attrib_t));
    };
    virtual ~RkAiqAdegammaHandleInt() {
        RkAiqAdegammaHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_degamma_attrib_t att);
    XCamReturn getAttrib(rk_aiq_degamma_attrib_t *att);
    //XCamReturn queryLscInfo(rk_aiq_lsc_querry_info_t *lsc_querry_info );

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAdegammaHandle::deInit();
    };
private:
    // TODO
    rk_aiq_degamma_attrib_t mCurAtt;
    rk_aiq_degamma_attrib_t mNewAtt;
};

// alsc
class RkAiqAlscHandleInt:
    virtual public RkAiqAlscHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAlscHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAlscHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_lsc_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_lsc_attrib_t));
    };
    virtual ~RkAiqAlscHandleInt() {
        RkAiqAlscHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_lsc_attrib_t att);
    XCamReturn getAttrib(rk_aiq_lsc_attrib_t *att);
    XCamReturn queryLscInfo(rk_aiq_lsc_querry_info_t *lsc_querry_info );

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAlscHandle::deInit();
    };
private:
    // TODO
    rk_aiq_lsc_attrib_t mCurAtt;
    rk_aiq_lsc_attrib_t mNewAtt;
};

// accm
class RkAiqAccmHandleInt:
    virtual public RkAiqAccmHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAccmHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAccmHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_ccm_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_ccm_attrib_t));
    };
    virtual ~RkAiqAccmHandleInt() {
        RkAiqAccmHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_ccm_attrib_t att);
    XCamReturn getAttrib(rk_aiq_ccm_attrib_t *att);
    XCamReturn queryCcmInfo(rk_aiq_ccm_querry_info_t *ccm_querry_info );

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAccmHandle::deInit();
    };
private:
    // TODO
    rk_aiq_ccm_attrib_t mCurAtt;
    rk_aiq_ccm_attrib_t mNewAtt;
};

// a3dlut
class RkAiqA3dlutHandleInt:
    virtual public RkAiqA3dlutHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqA3dlutHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqA3dlutHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_lut3d_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_lut3d_attrib_t));
    };
    virtual ~RkAiqA3dlutHandleInt() {
        RkAiqA3dlutHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_lut3d_attrib_t att);
    XCamReturn getAttrib(rk_aiq_lut3d_attrib_t *att);
    XCamReturn query3dlutInfo(rk_aiq_lut3d_querry_info_t *lut3d_querry_info );

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqA3dlutHandle::deInit();
    };
private:
    // TODO
    rk_aiq_lut3d_attrib_t mCurAtt;
    rk_aiq_lut3d_attrib_t mNewAtt;
};




class RkAiqAblcHandleInt:
    virtual public RkAiqAblcHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAblcHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAblcHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_blc_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_blc_attrib_t));
    };
    virtual ~RkAiqAblcHandleInt() {
        RkAiqAblcHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_blc_attrib_t *att);
    XCamReturn getAttrib(rk_aiq_blc_attrib_t *att);

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAblcHandle::deInit();
    };
private:
    // TODO
    rk_aiq_blc_attrib_t mCurAtt;
    rk_aiq_blc_attrib_t mNewAtt;
};




// adpcc
class RkAiqAdpccHandleInt:
    virtual public RkAiqAdpccHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAdpccHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAdpccHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_dpcc_attrib_V20_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_dpcc_attrib_V20_t));
    };
    virtual ~RkAiqAdpccHandleInt() {
        RkAiqAdpccHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_dpcc_attrib_V20_t *att);
    XCamReturn getAttrib(rk_aiq_dpcc_attrib_V20_t *att);

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAdpccHandle::deInit();
    };
private:
    // TODO
    rk_aiq_dpcc_attrib_V20_t mCurAtt;
    rk_aiq_dpcc_attrib_V20_t mNewAtt;
};


// anr
class RkAiqAnrHandleInt:
    virtual public RkAiqAnrHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAnrHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAnrHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_nr_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_nr_attrib_t));
    };
    virtual ~RkAiqAnrHandleInt() {
        RkAiqAnrHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_nr_attrib_t *att);
    XCamReturn getAttrib(rk_aiq_nr_attrib_t *att);
    XCamReturn setLumaSFStrength(float fPercent);
    XCamReturn setLumaTFStrength(float fPercent);
    XCamReturn getLumaSFStrength(float *pPercent);
    XCamReturn getLumaTFStrength(float *pPercent);
    XCamReturn setChromaSFStrength(float fPercent);
    XCamReturn setChromaTFStrength(float fPercent);
    XCamReturn getChromaSFStrength(float *pPercent);
    XCamReturn getChromaTFStrength(float *pPercent);
    XCamReturn setRawnrSFStrength(float fPercent);
    XCamReturn getRawnrSFStrength(float *pPercent);
    XCamReturn setIQPara(rk_aiq_nr_IQPara_t *pPara);
    XCamReturn getIQPara(rk_aiq_nr_IQPara_t *pPara);
protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAnrHandle::deInit();
    };
private:
    // TODO
    rk_aiq_nr_attrib_t mCurAtt;
    rk_aiq_nr_attrib_t mNewAtt;
    rk_aiq_nr_IQPara_t mCurIQpara;
    rk_aiq_nr_IQPara_t mNewIQpara;
    bool UpdateIQpara = false;
};


// anr
class RkAiqAsharpHandleInt:
    virtual public RkAiqAsharpHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAsharpHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAsharpHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_sharp_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_sharp_attrib_t));
    };
    virtual ~RkAiqAsharpHandleInt() {
        RkAiqAsharpHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_sharp_attrib_t *att);
    XCamReturn getAttrib(rk_aiq_sharp_attrib_t *att);
    XCamReturn setStrength(float fPercent);
    XCamReturn getStrength(float *pPercent);
    XCamReturn setIQPara(rk_aiq_sharp_IQpara_t *para);
    XCamReturn getIQPara(rk_aiq_sharp_IQpara_t *para);

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAsharpHandle::deInit();
    };
private:
    // TODO
    rk_aiq_sharp_attrib_t mCurAtt;
    rk_aiq_sharp_attrib_t mNewAtt;
    rk_aiq_sharp_IQpara_t mCurIQPara;
    rk_aiq_sharp_IQpara_t mNewIQPara;
    bool updateIQpara = false;

};

// afec
class RkAiqAfecHandleInt:
    virtual public RkAiqAfecHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAfecHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAfecHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_fec_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_fec_attrib_t));
        mCurAtt.en = 0xff;
    };
    virtual ~RkAiqAfecHandleInt() {
        RkAiqAfecHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();

    XCamReturn setAttrib(rk_aiq_fec_attrib_t att);
    XCamReturn getAttrib(rk_aiq_fec_attrib_t *att);

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAfecHandle::deInit();
    };
private:
    rk_aiq_fec_attrib_t mCurAtt;
    rk_aiq_fec_attrib_t mNewAtt;
};

class RkAiqAsdHandleInt:
    virtual public RkAiqAsdHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAsdHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAsdHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {};
    virtual ~RkAiqAsdHandleInt() {
        RkAiqAsdHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(asd_attrib_t att);
    XCamReturn getAttrib(asd_attrib_t *att);
protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAsdHandle::deInit();
    };
private:
    asd_attrib_t mCurAtt;
    asd_attrib_t mNewAtt;
};

// aldch
class RkAiqAldchHandleInt:
    virtual public RkAiqAldchHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAldchHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAldchHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_ldch_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_ldch_attrib_t));
    };
    virtual ~RkAiqAldchHandleInt() {
        RkAiqAldchHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();

    XCamReturn setAttrib(rk_aiq_ldch_attrib_t att);
    XCamReturn getAttrib(rk_aiq_ldch_attrib_t *att);

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAldchHandle::deInit();
    };
private:
    rk_aiq_ldch_attrib_t mCurAtt;
    rk_aiq_ldch_attrib_t mNewAtt;
};

class RkAiqAcpHandleInt:
    virtual public RkAiqAcpHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAcpHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAcpHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {};
    virtual ~RkAiqAcpHandleInt() {
        RkAiqAcpHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    XCamReturn setAttrib(acp_attrib_t att);
    XCamReturn getAttrib(acp_attrib_t *att);
protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAcpHandle::deInit();
    };
private:
    acp_attrib_t mCurAtt;
    acp_attrib_t mNewAtt;
};

class RkAiqAdrcHandleInt:
    virtual public RkAiqAdrcHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAdrcHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAdrcHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {};
    virtual ~RkAiqAdrcHandleInt() {
        RkAiqAdrcHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    XCamReturn setAttrib(drc_attrib_t att);
    XCamReturn getAttrib(drc_attrib_t *att);
protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAdrcHandle::deInit();
    };
private:
    drc_attrib_t mCurAtt;
    drc_attrib_t mNewAtt;
};
class RkAiqAieHandleInt:
    virtual public RkAiqAieHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAieHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAieHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {};
    virtual ~RkAiqAieHandleInt() {
        RkAiqAieHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    XCamReturn setAttrib(aie_attrib_t att);
    XCamReturn getAttrib(aie_attrib_t *att);
protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAieHandle::deInit();
    };
private:
    aie_attrib_t mCurAtt;
    aie_attrib_t mNewAtt;
};

// aeis
class RkAiqAeisHandleInt:
    virtual public RkAiqAeisHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAeisHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAeisHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_eis_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_eis_attrib_t));
        mCurAtt.en = 0xff;
    };
    virtual ~RkAiqAeisHandleInt() {
        RkAiqAeisHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();

    XCamReturn setAttrib(rk_aiq_eis_attrib_t att);
    XCamReturn getAttrib(rk_aiq_eis_attrib_t *att);

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAeisHandle::deInit();
    };
private:
    rk_aiq_eis_attrib_t mCurAtt;
    rk_aiq_eis_attrib_t mNewAtt;
};

//amd
class RkAiqAmdHandleInt:
    virtual public RkAiqAmdHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAmdHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAmdHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore)
        , mProcResShared(nullptr) {};
    virtual ~RkAiqAmdHandleInt() {
        RkAiqAmdHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAmdHandle::deInit();
    };
private:
    SmartPtr<RkAiqAlgoProcResAmdIntShared> mProcResShared;
};





// aynr v1
class RkAiqArawnrHandleInt:
    virtual public RkAiqArawnrHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqArawnrHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqArawnrHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_bayernr_attrib_v1_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_bayernr_attrib_v1_t));
    };
    virtual ~RkAiqArawnrHandleInt() {
        RkAiqArawnrHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_bayernr_attrib_v1_t *att);
    XCamReturn getAttrib(rk_aiq_bayernr_attrib_v1_t *att);
    XCamReturn setStrength(float fPercent);
    XCamReturn getStrength(float *pPercent);
    XCamReturn setIQPara(rk_aiq_bayernr_IQPara_V1_t *pPara);
    XCamReturn getIQPara(rk_aiq_bayernr_IQPara_V1_t *pPara);
    XCamReturn setJsonPara(rk_aiq_bayernr_JsonPara_V1_t *para);
    XCamReturn getJsonPara(rk_aiq_bayernr_JsonPara_V1_t *para);
protected:
    virtual void init();
    virtual void deInit() {
        RkAiqArawnrHandle::deInit();
    };
private:
    // TODO
    rk_aiq_bayernr_attrib_v1_t mCurAtt;
    rk_aiq_bayernr_attrib_v1_t mNewAtt;
    rk_aiq_bayernr_IQPara_V1_t mCurIQPara;
    rk_aiq_bayernr_IQPara_V1_t mNewIQPara;
    rk_aiq_bayernr_JsonPara_V1_t mCurJsonPara;
    rk_aiq_bayernr_JsonPara_V1_t mNewJsonPara;
    bool updateIQpara = false;
    bool updateJsonpara = false;
};


// aynr v1
class RkAiqAynrHandleInt:
    virtual public RkAiqAynrHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAynrHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAynrHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_ynr_attrib_v1_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_ynr_attrib_v1_t));
    };
    virtual ~RkAiqAynrHandleInt() {
        RkAiqAynrHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_ynr_attrib_v1_t *att);
    XCamReturn getAttrib(rk_aiq_ynr_attrib_v1_t *att);
    XCamReturn setStrength(float fPercent);
    XCamReturn getStrength(float *pPercent);
    XCamReturn setIQPara(rk_aiq_ynr_IQPara_V1_t *pPara);
    XCamReturn getIQPara(rk_aiq_ynr_IQPara_V1_t *pPara);
    XCamReturn setJsonPara(rk_aiq_ynr_JsonPara_V1_t *para);
    XCamReturn getJsonPara(rk_aiq_ynr_JsonPara_V1_t *para);
protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAynrHandle::deInit();
    };
private:
    // TODO
    rk_aiq_ynr_attrib_v1_t mCurAtt;
    rk_aiq_ynr_attrib_v1_t mNewAtt;
    rk_aiq_ynr_IQPara_V1_t mCurIQPara;
    rk_aiq_ynr_IQPara_V1_t mNewIQPara;
    rk_aiq_ynr_JsonPara_V1_t mCurJsonPara;
    rk_aiq_ynr_JsonPara_V1_t mNewJsonPara;
    bool updateIQpara = false;
    bool updateJsonpara = false;
};






// auvnr v1
class RkAiqAcnrHandleInt:
    virtual public RkAiqAcnrHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAcnrHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAcnrHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_uvnr_attrib_v1_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_uvnr_attrib_v1_t));
    };
    virtual ~RkAiqAcnrHandleInt() {
        RkAiqAcnrHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_uvnr_attrib_v1_t *att);
    XCamReturn getAttrib(rk_aiq_uvnr_attrib_v1_t *att);
    XCamReturn setStrength(float fPercent);
    XCamReturn getStrength(float *pPercent);
    XCamReturn setIQPara(rk_aiq_uvnr_IQPara_v1_t *pPara);
    XCamReturn getIQPara(rk_aiq_uvnr_IQPara_v1_t *pPara);
    XCamReturn setJsonPara(rk_aiq_uvnr_JsonPara_v1_t *para);
    XCamReturn getJsonPara(rk_aiq_uvnr_JsonPara_v1_t *para);
protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAcnrHandle::deInit();
    };
private:
    // TODO
    rk_aiq_uvnr_attrib_v1_t mCurAtt;
    rk_aiq_uvnr_attrib_v1_t mNewAtt;
    rk_aiq_uvnr_IQPara_v1_t mCurIQPara;
    rk_aiq_uvnr_IQPara_v1_t mNewIQPara;
    rk_aiq_uvnr_JsonPara_v1_t mCurJsonPara;
    rk_aiq_uvnr_JsonPara_v1_t mNewJsonPara;
    bool updateIQpara = false;
    bool updateJsonpara = false;
};

class RkAiqAmfnrHandleInt:
    virtual public RkAiqAmfnrHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAmfnrHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAmfnrHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_mfnr_attrib_v1_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_mfnr_attrib_v1_t));
        memset(&mCurIQPara, 0, sizeof(rk_aiq_mfnr_IQPara_V1_t));
        memset(&mNewIQPara, 0, sizeof(rk_aiq_mfnr_IQPara_V1_t));
        memset(&mCurJsonPara, 0, sizeof(rk_aiq_mfnr_JsonPara_V1_t));
        memset(&mNewJsonPara, 0, sizeof(rk_aiq_mfnr_JsonPara_V1_t));
    };
    virtual ~RkAiqAmfnrHandleInt() {
        RkAiqAmfnrHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    XCamReturn setAttrib(rk_aiq_mfnr_attrib_v1_t *att);
    XCamReturn getAttrib(rk_aiq_mfnr_attrib_v1_t *att);
    XCamReturn setLumaStrength(float fPercent);
    XCamReturn getLumaStrength(float *pPercent);
    XCamReturn setChromaStrength(float fPercent);
    XCamReturn getChromaStrength(float *pPercent);
    XCamReturn setIQPara(rk_aiq_mfnr_IQPara_V1_t *pPara);
    XCamReturn getIQPara(rk_aiq_mfnr_IQPara_V1_t *pPara);
    XCamReturn setJsonPara(rk_aiq_mfnr_JsonPara_V1_t *para);
    XCamReturn getJsonPara(rk_aiq_mfnr_JsonPara_V1_t *para);
protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAmfnrHandle::deInit();
    };
private:
    rk_aiq_mfnr_attrib_v1_t mCurAtt;
    rk_aiq_mfnr_attrib_v1_t mNewAtt;
    rk_aiq_mfnr_IQPara_V1_t mCurIQPara;
    rk_aiq_mfnr_IQPara_V1_t mNewIQPara;
    rk_aiq_mfnr_JsonPara_V1_t mCurJsonPara;
    rk_aiq_mfnr_JsonPara_V1_t mNewJsonPara;
    bool updateIQpara = false;
    bool updateJsonpara = false;
};


// again v1
class RkAiqAgainHandleInt:
    virtual public RkAiqAgainHandle,
    virtual public RkAiqHandleIntCom {
public:
    explicit RkAiqAgainHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore)
        , RkAiqAgainHandle(des, aiqCore)
        , RkAiqHandleIntCom(des, aiqCore) {
    };
    virtual ~RkAiqAgainHandleInt() {
        RkAiqAgainHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqAgainHandle::deInit();
    };
private:


};

}; //namespace RkCam

#endif
