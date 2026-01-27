/*
 *  Copyright (c) 2024 Rockchip Corporation
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

#ifndef _RK_AIQ_MANAGER_C_H_
#define _RK_AIQ_MANAGER_C_H_

#include "hwi_c/aiq_CamHwBase.h"
#include "aiq_core_c/aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"
#include "RkAibnrManager.h"
#include "RknnManager.h"
#include "RkAiynrManager.h"

RKAIQ_BEGIN_DECLARE

typedef enum aiq_state_e {
    AIQ_STATE_INVALID,
    AIQ_STATE_INITED,
    AIQ_STATE_PREPARED,
    AIQ_STATE_STARTED,
    AIQ_STATE_STOPED,
} aiq_state_t;

typedef struct AiqManager_s {
    uint32_t sensor_output_width;
    uint32_t sensor_output_height;
    // post aiisp status
    bool ainr_status;
    AiqCamHwBase_t* mCamHw;
    AiqCore_t* mRkAiqAnalyzer;
    rk_aiq_error_cb mErrCb;
    rk_aiq_metas_cb mMetasCb;
    rk_aiq_hwevt_cb mHwEvtCb;
    rk_aiq_aiispCtx_t mAiispCtx;
    void* mHwEvtCbCtx;
    AiqAnalyzerCb_t mAnalyzeCb;
    AiqHwResListener_t mHwResCb;
    const char* mSnsEntName;
    rk_aiq_tb_info_t mTbInfo;
    CamCalibDbV2Context_t* mCalibDbV2;
    CamCalibDbV2Context_t* tuningCalib;
    bool mNeedFreeCalib;
    rk_aiq_working_mode_t mWorkingMode;
    rk_aiq_working_mode_t mOldWkModeForGray;
    bool mWkSwitching;
    uint32_t mWidth;
    uint32_t mHeight;
    int _state;
    bool mCurMirror;
    bool mCurFlip;
    int mCamPhyId;
#ifdef RKAIQ_ENABLE_CAMGROUP
    AiqCamGroupManager_t* mCamGroupCoreManager;
#endif
    bool mIsMain;
    int mTBStatsCnt;
    uint32_t mLastAweekId;
    GlobalParamsManager_t mGlobalParamsManager;
    /* aiq_params_base_t* */
    AiqList_t* mParamsList;
#if RKAIQ_HAVE_AIBNR
    AibnrManager_t mAibnrManager;
#endif
    RknnManager_t mRknnManager;

    bool mLastAibnrEn;

#if RKAIQ_HAVE_AIYNR
    AiynrManager_t mAiynrManager;
    bool mLastAiynrEn;
#endif

    rk_aiq_isp_hdr_mode_t mHdrMergeMode;
    rk_aiq_sensor_hdr_line_mode_t mCisHdrMode;

#if RKAIQ_HAVE_DUMPSYS
    int (*dump_mods)(void* self, st_string* result, int argc, void* argv[]);
    int (*dump_raw)(void* self, st_string* result, int argc, void* argv[]);
#endif
} AiqManager_t;

XCamReturn AiqManager_init(AiqManager_t* pAiqManager, const char* sns_ent_name, rk_aiq_error_cb err_cb, rk_aiq_metas_cb metas_cb);
XCamReturn AiqManager_deinit(AiqManager_t* pAiqManager);
XCamReturn AiqManager_prepare(AiqManager_t* pAiqManager, uint32_t width, uint32_t height, rk_aiq_working_mode_t mode);
XCamReturn AiqManager_start(AiqManager_t* pAiqManager);
XCamReturn AiqManager_stop(AiqManager_t* pAiqManager, bool keep_ext_hw_st);

#define AiqManager_setHwEvtCb(pAiqManager, hwevt_cb, evt_cb_ctx) \
        pAiqManager->mHwEvtCbCtx = evt_cb_ctx; \
        pAiqManager->mHwEvtCb = hwevt_cb

void AiqManager_setAiqCalibDb(AiqManager_t* pAiqManager, const CamCalibDbV2Context_t* calibDb);

#define AiqManager_unsetTuningCalibDb(pAiqManager, isNeedFreeCalib) \
    pAiqManager->mNeedFreeCalib = isNeedFreeCalib

#define AiqManager_setAiispCb(pAiqManager, aiispCtx) \
    pAiqManager->mAiispCtx = aiispCtx

#define AiqManager_setTbInfo(pAiqManager, tbinfo) \
    pAiqManager->mTbInfo = *tbinfo

#define AiqManager_setCamPhyId(pAiqManager, phyId) \
    pAiqManager->mCamPhyId = phyId

#define AiqManager_getCamPhyId(pAiqManager) \
    pAiqManager->mCamPhyId 

XCamReturn AiqManager_updateCalibDb(AiqManager_t* pAiqManager, const CamCalibDbV2Context_t* newCalibDb);
XCamReturn AiqManager_syncSofEvt(AiqManager_t* pAiqManager, AiqHwEvt_t* hwres);
#define AiqManager_rkAiqCalcDone(pAiqManager, results) \
    AiqManager_applyAnalyzerResult(pAiqManager, results, false)

XCamReturn AiqManager_setModuleCtl(AiqManager_t* pAiqManager, rk_aiq_module_id_t mId, bool mod_en);
XCamReturn AiqManager_getModuleCtl(AiqManager_t* pAiqManager, rk_aiq_module_id_t mId, bool* mod_en);
XCamReturn AiqManager_enqueueRawBuffer(AiqManager_t* pAiqManager, void *rawdata, bool sync);
XCamReturn AiqManager_enqueueRawFile(AiqManager_t* pAiqManager, const char *path);
XCamReturn AiqManager_registRawdataCb(AiqManager_t* pAiqManager, void (*callback)(void *));
XCamReturn AiqManager_rawdataPrepare(AiqManager_t* pAiqManager, rk_aiq_raw_prop_t prop);
XCamReturn AiqManager_setMirrorFlip(AiqManager_t* pAiqManager, bool mirror, bool flip, int skip_frm_cnt);
XCamReturn AiqManager_getMirrorFlip(AiqManager_t* pAiqManager, bool* mirror, bool* flip);
void AiqManager_setMulCamConc(AiqManager_t* pAiqManager, bool cc);

#define AiqManager_getCurrentCalibDBV2(pAiqManager) \
    pAiqManager->mCalibDbV2

XCamReturn AiqManager_calibTuning(AiqManager_t* pAiqManager, CamCalibDbV2Context_t* aiqCalib,
                       TuningCalib* change_list);
XCamReturn AiqManager_setVicapStreamMode(AiqManager_t* pAiqManager, int on, bool isSingleMode);
#ifdef RKAIQ_ENABLE_CAMGROUP

#define AiqManager_setCamGroupManager(pAiqManager, cam_group_manager, isMain) \
        (pAiqManager)->mCamGroupCoreManager = cam_group_manager; (pAiqManager)->mIsMain = isMain

#endif

#define AiqManager_getAiqState(pAiqManager) \
        pAiqManager->_state

#define AiqManager_getWorkingMode(pAiqManager) \
        pAiqManager->mWorkingMode

XCamReturn AiqManager_applyAnalyzerResult(AiqManager_t* pAiqManager, AiqFullParams_t* results, bool ignoreIsUpdate);
void AiqManager_pushImuData(AiqManager_t* pAiqManager, AiqImuData_t *data);

RKAIQ_END_DECLARE

#endif
