/*
 * Copyright (c) 2021-2022 Rockchip Eletronics Co., Ltd.
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
#include "RkAiqCamGroupManager.h"

#include "RkAiqManager.h"
#include "RkAiqCamGroupHandleInt.h"
#include "aiq_core/RkAiqCore.h"
#include "algos_camgroup/ae/rk_aiq_algo_camgroup_ae_itf.h"
#include "algos_camgroup/awb/rk_aiq_algo_camgroup_awb_itf.h"
#include "algos_camgroup/misc/rk_aiq_algo_camgroup_misc_itf.h"
#include "algos_camgroup/aynr/rk_aiq_algo_camgroup_aynr_itf.h"
#include "algos_camgroup/acnr/rk_aiq_algo_camgroup_acnr_itf.h"
#include "algos_camgroup/atnr/rk_aiq_algo_camgroup_atnr_itf.h"
#include "algos_camgroup/abayernr/rk_aiq_algo_camgroup_abayernr_itf.h"
#include "algos_camgroup/again/rk_aiq_algo_camgroup_again_itf.h"
#include "algos_camgroup/asharp/rk_aiq_algo_camgroup_asharp_itf.h"
#include "smart_buffer_priv.h"

namespace RkCam {

const static struct RkAiqAlgoDesCommExt g_camgroup_algos[] = {
    { &g_RkIspAlgoDescCamgroupAe.common, RK_AIQ_CORE_ANALYZE_AE, 0, 2, 0},
    { &g_RkIspAlgoDescCamgroupAblc.common, RK_AIQ_CORE_ANALYZE_AWB, 0, 0, 0},
    { &g_RkIspAlgoDescCamgroupAwb.common, RK_AIQ_CORE_ANALYZE_AWB, 1, 1, 0},
    { &g_RkIspAlgoDescCamgroupAlsc.common, RK_AIQ_CORE_ANALYZE_AWB, 0, 0, 0},
    { &g_RkIspAlgoDescCamgroupAccm.common, RK_AIQ_CORE_ANALYZE_AWB, 0, 0, 0},
    { &g_RkIspAlgoDescCamgroupA3dlut.common, RK_AIQ_CORE_ANALYZE_AWB, 0, 0, 0},
    /* dpcc group algo is not mandatory now */
    //{ &g_RkIspAlgoDescCamgroupAdpcc.common, RK_AIQ_CORE_ANALYZE_AWB, 0, 0, 0},
    { & g_RkIspAlgoDescCamgroupAdhaz.common, RK_AIQ_CORE_ANALYZE_GRP0, 0, 1, 0},
    /* gamma group algo is not mandatory now */
    //{ &g_RkIspAlgoDescamgroupAgamma.common, RK_AIQ_CORE_ANALYZE_GRP0, 0, 0, 0 },
    { & g_RkIspAlgoDescCamgroupAdrc.common, RK_AIQ_CORE_ANALYZE_GRP0, 0, 1, 0},
    //{ &g_RkIspAlgoDescCamgroupAmerge.common, RK_AIQ_CORE_ANALYZE_GRP0, 0, 0, 0},
    { &g_RkIspAlgoDescCamgroupAynr.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0},
    { &g_RkIspAlgoDescCamgroupAcnr.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0},
#if defined(ISP_HW_V30)
    { &g_RkIspAlgoDescCamgroupAbayertnr.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0},
#endif
    { &g_RkIspAlgoDescCamgroupAbayernr.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0},
    { &g_RkIspAlgoDescCamgroupAsharp.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0},
#if defined(ISP_HW_V30)
    { &g_RkIspAlgoDescCamgroupAgain.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0},
#endif
    { NULL, RK_AIQ_CORE_ANALYZE_ALL, 0, 0 },
};

bool
RkAiqCamGroupReprocTh::loop ()
{
    const static int32_t timeout = -1;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<rk_aiq_groupcam_result_wrapper_t> camGroupRes_wrapper = mMsgQueue.pop (timeout);

    if (!camGroupRes_wrapper.ptr()) {
        return true;
    }

    rk_aiq_groupcam_result_t* camGroupRes = camGroupRes_wrapper->_gc_result;
    if (camGroupRes->_ready) {
        ret = mCamGroupManager->reProcess(camGroupRes);
        if (ret < 0) {
            LOGW_CAMGROUP("reprocess error, ignore!");
        } else
            mCamGroupManager->relayToHwi(camGroupRes);
    }
    // delete the processed result
    mCamGroupManager->putGroupCamResult(camGroupRes); // paired with sendFrame
    mCamGroupManager->clearGroupCamResult(camGroupRes->_frameId);

    return true;
}

bool
RkAiqCamGroupReprocTh::sendFrame(rk_aiq_groupcam_result_t* gc_result)
{
    {
        SmartLock locker (mCamGroupManager->mCamGroupResMutex);
        gc_result->_refCnt++;
    }
    mMsgQueue.push(new rk_aiq_groupcam_result_wrapper_t(gc_result));
    return true;
};

RkAiqCamGroupManager::RkAiqCamGroupManager()
{
    ENTER_CAMGROUP_FUNCTION();
    mCamGroupReprocTh = new RkAiqCamGroupReprocTh(this);
    mRequiredMsgsMask = (1ULL << XCAM_MESSAGE_AWB_STATS_OK) | (1ULL << XCAM_MESSAGE_AWB_PROC_RES_OK) | (1ULL << XCAM_MESSAGE_SOF_INFO_OK) |
                        (1ULL << XCAM_MESSAGE_AEC_STATS_OK) | (1ULL << XCAM_MESSAGE_AE_PRE_RES_OK) | (1ULL << XCAM_MESSAGE_AE_PROC_RES_OK);

    mGroupAlgosDesArray = g_camgroup_algos;
    mState = CAMGROUP_MANAGER_INVALID;
    mRequiredCamsResMask = 0;
    mRequiredAlgoResMask = 0;
    mInit = false;
    mCamgroupCalib = NULL;
    mVicapReadyMask = 0;
    mClearedSofId = 0;
    mClearedResultId = 0;
    mGroupCtx = NULL;

    EXIT_CAMGROUP_FUNCTION();
}

RkAiqCamGroupManager::~RkAiqCamGroupManager()
{
    ENTER_CAMGROUP_FUNCTION();
    EXIT_CAMGROUP_FUNCTION();
}

rk_aiq_groupcam_result_t*
RkAiqCamGroupManager::getGroupCamResult(uint32_t frameId, bool query_ready)
{
    SmartLock locker (mCamGroupResMutex);
    rk_aiq_groupcam_result_t* camGroupRes = NULL;

    if (mCamGroupResMap.find(frameId) != mCamGroupResMap.end()) {
        camGroupRes = mCamGroupResMap[frameId];
        if (!query_ready && camGroupRes->_ready)
            return NULL;
        camGroupRes->_refCnt++;
        LOG1_CAMGROUP("camgroup res of frame: %u exists", frameId);
    } else {
        if (!query_ready)
            return NULL;

        if (mCamGroupResMap.size() > 8) {
            LOGE_CAMGROUP("camgroup result map overflow:%d, first_id: %u",
                          mCamGroupResMap.size(), mCamGroupResMap.begin()->first);
            clearGroupCamResult_Locked(frameId - 4);
        }
        if (frameId < mClearedResultId) {
            LOGW_CAMGROUP("disorder frameId(%d) < mClearedResultId(%d)", frameId, mClearedResultId);
            return NULL;
        }
        camGroupRes = new rk_aiq_groupcam_result_t();
        if (!camGroupRes) {
            LOGE_CAMGROUP("malloc camGroup Res failed !");
            return NULL;
        }
        camGroupRes->reset();
        camGroupRes->_frameId = frameId;
        camGroupRes->_refCnt++;
        mCamGroupResMap[frameId] = camGroupRes;
        LOGD_CAMGROUP("malloc camgroup res for frame: %u success", frameId);
    }

    return camGroupRes;
}

void
RkAiqCamGroupManager::clearGroupCamResult_Locked(uint32_t frameId)
{
    if (frameId == (uint32_t)(-1)) {
        // clear all
        LOGD_CAMGROUP("clear all camgroup res");
        for (auto it : mCamGroupResMap) {
            (it.second)->reset();
            delete it.second;
        }
        mCamGroupResMap.clear();
    } else {
        rk_aiq_groupcam_result_t* camGroupRes = NULL;

        std::map<uint32_t, rk_aiq_groupcam_result_t*>::iterator iter;
        for (iter = mCamGroupResMap.begin(); iter != mCamGroupResMap.end();) {
            if (iter->first <= frameId) {
                camGroupRes = iter->second;
                if (camGroupRes->_refCnt > 0) {
                    if (iter->first < mClearedResultId) {
                        LOGW("impossible, id:%u < mClearedResultId:%u, refCnt: %u",
                             iter->first, mClearedResultId, camGroupRes->_refCnt);
                    }
                    iter++;
                    continue;
                }
                LOGD_CAMGROUP("clear camgroup res of frame: %u, ready: %d", iter->first, camGroupRes->_ready);
                camGroupRes->reset();
                delete camGroupRes;
                if (iter->first > mClearedResultId)
                    mClearedResultId = iter->first;
                iter = mCamGroupResMap.erase(iter);
            } else {
                iter++;
            }
        }
    }
}

void
RkAiqCamGroupManager::clearGroupCamResult(uint32_t frameId) {

    SmartLock locker (mCamGroupResMutex);
    clearGroupCamResult_Locked(frameId);
}

void
RkAiqCamGroupManager::putGroupCamResult(rk_aiq_groupcam_result_t* gc_res)
{
    SmartLock locker (mCamGroupResMutex);
    if (gc_res && gc_res->_refCnt != 0)
        gc_res->_refCnt--;
}

rk_aiq_groupcam_sofsync_t*
RkAiqCamGroupManager::getGroupCamSofsync(uint32_t frameId, bool query_ready)
{
    SmartLock locker (mSofMutex);
    rk_aiq_groupcam_sofsync_t* camGroupSofsync = NULL;

    if (mCamGroupSofsyncMap.find(frameId) != mCamGroupSofsyncMap.end()) {
        camGroupSofsync = mCamGroupSofsyncMap[frameId];
        if (!query_ready && (camGroupSofsync->_validCamSofSyncBits == mRequiredCamsResMask))
            return NULL;
        camGroupSofsync->_refCnt++;
        LOG1_CAMGROUP("camgroup sofSync of frame: %u exists", frameId);
    } else {
        if (!query_ready)
            return NULL;

        // if overflow, clear some ones
        if (mCamGroupSofsyncMap.size() > 16) {
            LOGW_CAMGROUP("camgroup sofSync overflow:%d, first_id: %d",
                          mCamGroupSofsyncMap.size(), mCamGroupSofsyncMap.begin()->first);
            clearGroupCamSofsync_Locked(frameId - 8);
        }

        if (frameId < mClearedSofId) {
            LOGE_CAMGROUP("disorder frameId(%u) < mClearedSofId(%u)", frameId, mClearedSofId);
            return NULL;
        }

        camGroupSofsync = new rk_aiq_groupcam_sofsync_t();
        if (!camGroupSofsync ) {
            LOGE_CAMGROUP("malloc camGroup sofSync failed !");
            return NULL;
        }

        camGroupSofsync->reset();
        camGroupSofsync->_refCnt++;
        mCamGroupSofsyncMap[frameId] = camGroupSofsync;
        LOGD_CAMGROUP("malloc camgroup sofSync for frame: %u success", frameId);
    }

    return camGroupSofsync;
}

void
RkAiqCamGroupManager::putGroupCamSofsync(rk_aiq_groupcam_sofsync_t* syncSof)
{
    SmartLock locker (mSofMutex);
    if (syncSof && syncSof->_refCnt != 0)
        syncSof->_refCnt--;
}

void
RkAiqCamGroupManager::clearGroupCamSofsync_Locked(uint32_t frameId) {
    if (frameId == (uint32_t)(-1)) {
        // clear all
        LOGD_CAMGROUP("clear all camgroup sofSync res");
        for (auto it : mCamGroupSofsyncMap) {
            (it.second)->reset();
            delete it.second;
        }
        mCamGroupSofsyncMap.clear();
    } else {
        rk_aiq_groupcam_sofsync_t* camGroupSofsync = NULL;

        std::map<uint32_t, rk_aiq_groupcam_sofsync_t*>::iterator iter;
        for (iter = mCamGroupSofsyncMap.begin(); iter != mCamGroupSofsyncMap.end();) {
            if (iter->first <= frameId) {
                camGroupSofsync = iter->second;
                if (camGroupSofsync->_refCnt > 0) {
                    iter++;
                    continue;
                }
                LOGD_CAMGROUP("clear camgroup sofSync of frame: %u, ready: %d",
                              iter->first,
                              camGroupSofsync->_validCamSofSyncBits == mRequiredCamsResMask);
                camGroupSofsync->reset();
                delete camGroupSofsync;
                mClearedSofId = frameId;
                iter = mCamGroupSofsyncMap.erase(iter);
            } else {
                iter++;
            }
        }
        if (mCamGroupSofsyncMap.size() > 100)
            LOGE_CAMGROUP("mCamGroupSofsyncMap size > 100 !!!");
    }
}
void
RkAiqCamGroupManager::clearGroupCamSofsync(uint32_t frameId) {

    SmartLock locker (mSofMutex);
    clearGroupCamSofsync_Locked(frameId);
}

void
RkAiqCamGroupManager::setSingleCamStatusReady(rk_aiq_singlecam_result_status_t* status, rk_aiq_groupcam_result_t* gc_result)
{
    bool gc_result_ready = false;
    {
        SmartLock locker (mCamGroupResMutex);
        if (((status->_validCoreMsgsBits & mRequiredMsgsMask) == mRequiredMsgsMask) &&
                ((status->_validAlgoResBits & mRequiredAlgoResMask) == mRequiredAlgoResMask)) {
            status->_ready = true;
            LOGD_CAMGROUP("camgroup single cam res ready, camId:%d, frameId:%u",
                          status->_singleCamResults._3aResults._camId,
                          status->_singleCamResults._3aResults._frameId);
            gc_result->_validCamResBits |= 1 << status->_singleCamResults._3aResults._camId;
            if (!gc_result->_ready && (gc_result->_validCamResBits == mRequiredCamsResMask)) {
                gc_result->_ready = true;
                gc_result_ready = true;
                LOGD_CAMGROUP("camgroup all cam res ready, frameId:%u",
                              status->_singleCamResults._3aResults._frameId);
            }
        }
    }

    if (gc_result_ready) {
        // force to ready
        uint32_t lastFrameId = status->_singleCamResults._3aResults._frameId - 1;
        rk_aiq_groupcam_result_t* last_gc_result = getGroupCamResult(lastFrameId, false);
        if (last_gc_result && (mState == CAMGROUP_MANAGER_STARTED)) {
            last_gc_result->_ready = true;
            last_gc_result->_validCamResBits = mRequiredCamsResMask;
            rk_aiq_singlecam_result_status_t* last_scam_status = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                if ((last_gc_result->_validCamResBits >> i) & 1) {
                    last_scam_status = &last_gc_result->_singleCamResultsStatus[i];
                    last_scam_status->_validAlgoResBits = mRequiredMsgsMask;
                    last_scam_status->_validCoreMsgsBits = mRequiredAlgoResMask;
                }
            }
            LOGW_CAMGROUP("camgroup res frameId disorder, unready frameId:%u < cur ready frame %u",
                          lastFrameId, status->_singleCamResults._3aResults._frameId);
            mCamGroupReprocTh->sendFrame(last_gc_result);
        }
        putGroupCamResult(last_gc_result);
        // init params is reprocessed in func prepare
        if (mState == CAMGROUP_MANAGER_STARTED) {
            LOGD_CAMGROUP("send frameId:%u ",gc_result->_frameId);
            mCamGroupReprocTh->sendFrame(gc_result);
        }
    } else {
        SmartLock locker (mCamGroupResMutex);
        if (status->_singleCamResults._fullIspParam.ptr()) {
            RkAiqFullParams* scam_aiqParams = status->_singleCamResults._fullIspParam->data().ptr();
            LOG1_CAMGROUP("scam_aiqParams %p ", scam_aiqParams);
        }
        LOG1_CAMGROUP("camgroup result status: validCams:0x%x(req:0x%x), camId:%d, frameId:%u, "
                      "validAlgoResBits:0x%" PRIx64 "(req:0x%" PRIx64 "), validMsgBits:0x%" PRIx64 "(req:0x%" PRIx64 ")",
                      gc_result->_validCamResBits, mRequiredCamsResMask,
                      status->_singleCamResults._3aResults._camId,
                      status->_singleCamResults._3aResults._frameId,
                      status->_validAlgoResBits, mRequiredAlgoResMask,
                      status->_validCoreMsgsBits, mRequiredMsgsMask);
    }
}

void
RkAiqCamGroupManager::processAiqCoreMsgs(RkAiqCore* src, SmartPtr<XCamMessage> &msg)
{
    int camId = src->getCamPhyId();
    uint32_t frameId = msg->frame_id;

    // check if the msg is required firstly
    if (!(mRequiredMsgsMask & (1ULL << msg->msg_id))) {
        LOG1_CAMGROUP("camgroup: not required core msg :%s of frameId: %d, ignore",
                      MessageType2Str[msg->msg_id], msg->frame_id);
        return;
    }

    switch (msg->msg_id) {
    case XCAM_MESSAGE_AWB_STATS_OK :
    case XCAM_MESSAGE_AEC_STATS_OK :
        // TODO: should get from RkAiqAnalyzerGroup::getMsgDelayCnt
        frameId += 2;
    default:
        break;
    }

    rk_aiq_groupcam_result_t* camGroupRes = getGroupCamResult(frameId);
    if (!camGroupRes) {
        LOGW_CAMGROUP("camgroup: get cam result faild for msg_id:%d, camId: %d, msg_id:%d, frame: %d", msg->msg_id, camId, frameId);
        return;
    }
    rk_aiq_singlecam_result_status_t* singleCamStatus = &camGroupRes->_singleCamResultsStatus[camId];
    rk_aiq_singlecam_result_t* singleCamRes = &singleCamStatus->_singleCamResults;

    SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg;

    switch (msg->msg_id) {
    case XCAM_MESSAGE_AWB_STATS_OK :
        vdBufMsg = msg.dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
        if (vdBufMsg.ptr())
            singleCamRes->_3aResults.awb._awbStats = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        break;
    case XCAM_MESSAGE_AWB_PROC_RES_OK :
        vdBufMsg = msg.dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
        if (vdBufMsg.ptr())
            singleCamRes->_3aResults.awb._awbProcRes = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        break;
    case XCAM_MESSAGE_AEC_STATS_OK :
        vdBufMsg = msg.dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
        if (vdBufMsg.ptr())
            singleCamRes->_3aResults.aec._aecStats = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        break;
    case XCAM_MESSAGE_AE_PRE_RES_OK :
        vdBufMsg = msg.dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
        if (vdBufMsg.ptr())
            singleCamRes->_3aResults.aec._aePreRes = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        break;
    case XCAM_MESSAGE_AE_PROC_RES_OK :
        vdBufMsg = msg.dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
        if (vdBufMsg.ptr())
            singleCamRes->_3aResults.aec._aeProcRes = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        break;
    case XCAM_MESSAGE_SOF_INFO_OK :
        vdBufMsg = msg.dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
        if (vdBufMsg.ptr()) {
            auto sofInfoMsg = vdBufMsg->msg.dynamic_cast_ptr<RkAiqSofInfoWrapperProxy>();
            singleCamRes->_3aResults.aec._effAecExpInfo =
                sofInfoMsg->data()->curExp->data()->aecExpInfo;
            singleCamRes->_3aResults.aec._bEffAecExpValid = true;
        }
        break;
    default:
        break;
    }

    // check if all requirements are satisfied, if so,
    // notify the reprocess procedure
    {
        SmartLock locker (mCamGroupResMutex);
        singleCamStatus->_validCoreMsgsBits |= ((uint64_t)1) << msg->msg_id;
        singleCamRes->_3aResults._camId = camId;
        singleCamRes->_3aResults._frameId = frameId;
    }
    LOGD_CAMGROUP("camgroup: got required core msg :%s of camId:%d, frameId: %u, ",
                  MessageType2Str[msg->msg_id], camId, frameId);
    setSingleCamStatusReady(singleCamStatus, camGroupRes);
    putGroupCamResult(camGroupRes);
}

void
RkAiqCamGroupManager::RelayAiqCoreResults(RkAiqCore* src, SmartPtr<RkAiqFullParamsProxy> &results)
{
    // only support v3x now
    int camId = src->getCamPhyId();
    uint32_t frame_id = -1;

    if (!CHECK_ISP_HW_V3X() && !CHECK_ISP_HW_V21()) {
        LOGE_CAMGROUP("only support isp 3x and 21 now");
        return;
    }

    RkAiqFullParams* aiqParams = results->data().ptr();

#define SET_TO_CAMGROUP(lc, BC) \
    if (aiqParams->m##lc##Params.ptr()) { \
        frame_id = aiqParams->m##lc##Params->data()->frame_id; \
        rk_aiq_groupcam_result_t* camGroupRes = getGroupCamResult(frame_id); \
        if (!camGroupRes) { \
            LOGW_CAMGROUP("camgroup: get cam result faild for type:%s, camId: %d, frame: %d", #BC, camId, frame_id); \
            return; \
        } \
        rk_aiq_singlecam_result_status_t* singleCamStatus = \
            &camGroupRes->_singleCamResultsStatus[camId]; \
        rk_aiq_singlecam_result_t* singleCamRes = &singleCamStatus->_singleCamResults; \
        /* compose single aiq params to one */ \
        { \
            SmartLock locker (mCamGroupResMutex); \
            if (!singleCamRes->_fullIspParam.ptr()) { \
                singleCamRes->_fullIspParam = results; \
                LOGD_CAMGROUP("init scam_aiqParams_proxy : %p for camId:%d, frameId: %u",\
                              singleCamRes->_fullIspParam.ptr(), camId, frame_id); \
            } \
            RkAiqFullParams* scam_aiqParams = singleCamRes->_fullIspParam->data().ptr();\
            if (scam_aiqParams != aiqParams) \
                scam_aiqParams->m##lc##Params = aiqParams->m##lc##Params; \
            singleCamStatus->_validAlgoResBits |= ((uint64_t)1) << RESULT_TYPE_##BC##_PARAM; \
            singleCamRes->_3aResults._camId = camId; \
            singleCamRes->_3aResults._frameId = frame_id; \
        } \
        LOG1_CAMGROUP("%s: relay results: camId:%d, frameId:%u, type:%s", \
                      __FUNCTION__, camId, frame_id, #BC); \
        setSingleCamStatusReady(singleCamStatus, camGroupRes); \
        putGroupCamResult(camGroupRes);\
    } \

    SET_TO_CAMGROUP(Exposure, EXPOSURE);
    SET_TO_CAMGROUP(Focus, FOCUS);
    SET_TO_CAMGROUP(Aec, AEC);
    SET_TO_CAMGROUP(Hist, HIST);
    SET_TO_CAMGROUP(AwbGain, AWBGAIN);
    SET_TO_CAMGROUP(Dpcc, DPCC);
    SET_TO_CAMGROUP(Ccm, CCM);
    SET_TO_CAMGROUP(Lsc, LSC);
    SET_TO_CAMGROUP(Debayer, DEBAYER);
    SET_TO_CAMGROUP(Ldch, LDCH);
    SET_TO_CAMGROUP(Lut3d, LUT3D);
    SET_TO_CAMGROUP(Adegamma, ADEGAMMA);
    SET_TO_CAMGROUP(Wdr, WDR);
    SET_TO_CAMGROUP(Csm, CSM);
    SET_TO_CAMGROUP(Cgc, CGC);
    SET_TO_CAMGROUP(Conv422, CONV422);
    SET_TO_CAMGROUP(Yuvconv, YUVCONV);
    SET_TO_CAMGROUP(Cp, CP);
    SET_TO_CAMGROUP(Ie, IE);
    SET_TO_CAMGROUP(Cpsl, CPSL);
    SET_TO_CAMGROUP(Motion, MOTION);
    // TODO: special for fec ?
    SET_TO_CAMGROUP(Fec, FEC);
    // ispv21 and ispv3x common
    SET_TO_CAMGROUP(BlcV21, BLC);
    SET_TO_CAMGROUP(Gic, GIC);
    SET_TO_CAMGROUP(Dehaze, DEHAZE);
    SET_TO_CAMGROUP(Drc, DRC);
    SET_TO_CAMGROUP(Agamma, AGAMMA);
    SET_TO_CAMGROUP(Merge, MERGE);

    if (CHECK_ISP_HW_V21()) {
        // ispv21
        SET_TO_CAMGROUP(AwbV21, AWB);
        SET_TO_CAMGROUP(BaynrV21, RAWNR);
        SET_TO_CAMGROUP(YnrV21, YNR);
        SET_TO_CAMGROUP(CnrV21, UVNR);
        SET_TO_CAMGROUP(SharpenV21, SHARPEN);
        SET_TO_CAMGROUP(Af, AF);
        SET_TO_CAMGROUP(Gain, GAIN);
    } else {
        // ispv3x
        SET_TO_CAMGROUP(AwbV3x, AWB);
        SET_TO_CAMGROUP(AfV3x, AF);
        SET_TO_CAMGROUP(BaynrV3x, RAWNR);
        SET_TO_CAMGROUP(YnrV3x, YNR);
        SET_TO_CAMGROUP(CnrV3x, UVNR);
        SET_TO_CAMGROUP(SharpenV3x, SHARPEN);
        SET_TO_CAMGROUP(CacV3x, CAC);
        SET_TO_CAMGROUP(GainV3x, GAIN);
        SET_TO_CAMGROUP(TnrV3x, TNR);
    }
}

XCamReturn
RkAiqCamGroupManager::sofSync(RkAiqManager* aiqManager, SmartPtr<VideoBuffer>& sof_evt)
{
    int camId = aiqManager->getCamPhyId();
    uint32_t frameId = sof_evt->get_sequence();

    if (mState != CAMGROUP_MANAGER_STARTED) {
        LOGE_CAMGROUP("wrong state %d, ignore sofSync event \n", mState);
        return XCAM_RETURN_NO_ERROR;
    }

    LOGD_CAMGROUP("sofSync event camId: %d, frameId: %u ...\n", camId, frameId);

    rk_aiq_groupcam_sofsync_t* camGroupSofsync = getGroupCamSofsync(frameId);
    if (!camGroupSofsync) {
        LOGE_CAMGROUP("camgroup: get sofSync failed for camId: %d, frame: %u, igore", camId, frameId);
        return XCAM_RETURN_NO_ERROR;
    }
    camGroupSofsync->_singleCamSofEvt[camId] = sof_evt;

    bool sync_done = false;
    {
        SmartLock locker (mSofMutex);
        camGroupSofsync->_validCamSofSyncBits |= ((uint8_t)1) << camId;
        if (camGroupSofsync->_validCamSofSyncBits == mRequiredCamsResMask)
            sync_done = true;
    }

    if (sync_done) {
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            if ((camGroupSofsync->_validCamSofSyncBits >> i) & 1) {
                mBindAiqsMap[i]->syncSofEvt(camGroupSofsync->_singleCamSofEvt[i]);
            }
        }
        putGroupCamSofsync(camGroupSofsync);
        clearGroupCamSofsync(frameId);
    } else
        putGroupCamSofsync(camGroupSofsync);

    LOGD_CAMGROUP("sofSync event camId: %d, frameId: %u done\n", camId, frameId);

    return XCAM_RETURN_NO_ERROR;
}

SmartPtr<RkAiqCamgroupHandle>
RkAiqCamGroupManager::newAlgoHandle(RkAiqAlgoDesComm* algo, int hw_ver)
{
#define NEW_ALGO_HANDLE(lc, BC) \
    if (algo->type == RK_AIQ_ALGO_TYPE_##BC) { \
        if (hw_ver == 0) \
            return new RkAiqCamGroup##lc##HandleInt(algo, this); \
    }\

    NEW_ALGO_HANDLE(Ae, AE);
    NEW_ALGO_HANDLE(Awb, AWB);
    NEW_ALGO_HANDLE(Accm, ACCM);
    NEW_ALGO_HANDLE(A3dlut, A3DLUT);
    NEW_ALGO_HANDLE(Agamma, AGAMMA);
    NEW_ALGO_HANDLE(Amerge, AMERGE);
    NEW_ALGO_HANDLE(Adrc, ADRC);
    NEW_ALGO_HANDLE(Adhaz, ADHAZ);
    NEW_ALGO_HANDLE(Agic, AGIC);
    NEW_ALGO_HANDLE(AynrV3, AYNR);
    NEW_ALGO_HANDLE(AcnrV2, ACNR);
    NEW_ALGO_HANDLE(Abayer2dnrV2, ARAWNR);
    NEW_ALGO_HANDLE(Ablc, ABLC);
    NEW_ALGO_HANDLE(AsharpV4, ASHARP);
    NEW_ALGO_HANDLE(AbayertnrV2, AMFNR);
    NEW_ALGO_HANDLE(Alsc, ALSC);
    NEW_ALGO_HANDLE(Adpcc, ADPCC);
    NEW_ALGO_HANDLE(AgainV2, AGAIN);
    /* TODO: new the handle of other algo modules */

    return new RkAiqCamgroupHandle(algo, this);
}

SmartPtr<RkAiqCamgroupHandle>
RkAiqCamGroupManager::getDefAlgoTypeHandle(int algo_type)
{
    // get defalut algo handle(id == 0)
    if (mDefAlgoHandleMap.find(algo_type) != mDefAlgoHandleMap.end())
        return mDefAlgoHandleMap.at(algo_type);

    LOG1_CAMGROUP("can't find algo handle %d", algo_type);
    return NULL;
}

std::map<int, SmartPtr<RkAiqCamgroupHandle>>*
        RkAiqCamGroupManager::getAlgoTypeHandleMap(int algo_type)
{
    if (mAlgoHandleMaps.find(algo_type) != mAlgoHandleMaps.end())
        return &mAlgoHandleMaps.at(algo_type);

    LOG1_CAMGROUP("can't find algo map %d", algo_type);
    return NULL;
}

void
RkAiqCamGroupManager::addDefaultAlgos(const struct RkAiqAlgoDesCommExt* algoDes)
{
    if (mBindAiqsMap.empty()) {
        LOGD_CAMGROUP("no group cam, bypass");
        return ;
    }

    RkAiqManager* aiqManager = (mBindAiqsMap.begin())->second;
    RkAiqCore* aiqCore = aiqManager->mRkAiqAnalyzer.ptr();
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &aiqCore->mAlogsComSharedParams;

    mGroupAlgoCtxCfg.s_calibv2 = sharedCom->calibv2;
    mGroupAlgoCtxCfg.pCamgroupCalib = mCamgroupCalib;
    mGroupAlgoCtxCfg.cfg_com.isp_hw_version = aiqCore->mIspHwVer;
    mGroupAlgoCtxCfg.cfg_com.calibv2 =
        const_cast<CamCalibDbV2Context_t*>(sharedCom->calibv2);
    mGroupAlgoCtxCfg.cfg_com.isGroupMode = true;

    mGroupAlgoCtxCfg.camIdArrayLen = mBindAiqsMap.size();

    int i = 0;
    for (auto it : mBindAiqsMap)
        mGroupAlgoCtxCfg.camIdArray[i++] = it.first;

    for (int i = 0; algoDes[i].des != NULL; i++) {
        RkAiqAlgoDesComm* algo_des = algoDes[i].des;
        mGroupAlgoCtxCfg.cfg_com.module_hw_version = algoDes[i].module_hw_ver;

        SmartPtr<RkAiqCamgroupHandle> grpHandle = newAlgoHandle(algo_des, 0);
        mDefAlgoHandleList.push_back(grpHandle);
        mDefAlgoHandleMap[algo_des->type] = grpHandle;

        std::map<int, SmartPtr<RkAiqCamgroupHandle>> hdlMaps;
        hdlMaps[0] = grpHandle;
        mAlgoHandleMaps[algo_des->type] = hdlMaps;

        LOGD_CAMGROUP("camgroup: add algo: %d", algo_des->type);
    }
}

XCamReturn
RkAiqCamGroupManager::setCamgroupCalib(CamCalibDbCamgroup_t* camgroup_calib)
{
    ENTER_CAMGROUP_FUNCTION();
    if (mState == CAMGROUP_MANAGER_STARTED ||
            mState == CAMGROUP_MANAGER_INVALID) {
        LOGE_CAMGROUP("wrong state %d\n", mState);
        return XCAM_RETURN_ERROR_FAILED;
    }
    // update groupcalib
    mCamgroupCalib = camgroup_calib;
    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCamGroupManager::init()
{
    ENTER_CAMGROUP_FUNCTION();
    addDefaultAlgos(mGroupAlgosDesArray);

    if (mState != CAMGROUP_MANAGER_BINDED) {
        LOGE_CAMGROUP("wrong state %d\n", mState);
        return XCAM_RETURN_ERROR_FAILED;
    }
    mState = CAMGROUP_MANAGER_INITED;
    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCamGroupManager::deInit()
{
    ENTER_CAMGROUP_FUNCTION();
    if (mState != CAMGROUP_MANAGER_UNBINDED) {
        LOGE_CAMGROUP("wrong state %d\n", mState);
        return XCAM_RETURN_ERROR_FAILED;
    }

    mDefAlgoHandleList.clear();
    mDefAlgoHandleMap.clear();
    mAlgoHandleMaps.clear();

    mState = CAMGROUP_MANAGER_INVALID;
    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCamGroupManager::start()
{
    ENTER_CAMGROUP_FUNCTION();
    if (mState != CAMGROUP_MANAGER_PREPARED) {
        LOGE_CAMGROUP("wrong state %d\n", mState);
        return XCAM_RETURN_ERROR_FAILED;
    }

    mCamGroupReprocTh->triger_start();
    mCamGroupReprocTh->start();

    mState = CAMGROUP_MANAGER_STARTED;
    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCamGroupManager::stop()
{
    ENTER_CAMGROUP_FUNCTION();
    if (mState == CAMGROUP_MANAGER_INVALID) {
        LOGE_CAMGROUP("wrong state %d\n", mState);
        return XCAM_RETURN_ERROR_FAILED;
    }

    mCamGroupReprocTh->triger_stop();
    mCamGroupReprocTh->stop();
    clearGroupCamResult(-1);
    clearGroupCamSofsync(-1);
    mVicapReadyMask = 0;
    mClearedSofId = 0;
    mClearedResultId = 0;

    if (mState == CAMGROUP_MANAGER_STARTED) {
        mState = CAMGROUP_MANAGER_PREPARED;
    }
    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCamGroupManager::prepare()
{
    ENTER_CAMGROUP_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (mState != CAMGROUP_MANAGER_INITED &&
            mState != CAMGROUP_MANAGER_BINDED) {
        LOGE_CAMGROUP("wrong state %d\n", mState);
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (mBindAiqsMap.empty()) {
        LOGD_CAMGROUP("no group cam, bypass");
        return XCAM_RETURN_NO_ERROR;
    }

    // assume all single cam runs same algos
    RkAiqManager* aiqManager = (mBindAiqsMap.begin())->second;
    RkAiqCore* aiqCore = aiqManager->mRkAiqAnalyzer.ptr();

    // reprocess initial params

    // TODO: should deal with the case of eanbled algos changed dynamically
    mRequiredAlgoResMask = aiqCore->mAllReqAlgoResMask;

    rk_aiq_groupcam_result_t* camGroupRes = getGroupCamResult(0);

    LOGD_CAMGROUP("camgroup: prepare: relay init params ...");
    for (auto it : mBindAiqsMap) {
        RkAiqManager* sAiqManager = it.second;
        RkAiqCore* sAiqCore = sAiqManager->mRkAiqAnalyzer.ptr();
        // initial params has no stats
        camGroupRes->_singleCamResultsStatus[it.first]._validCoreMsgsBits = mRequiredMsgsMask;
        RelayAiqCoreResults(sAiqCore, sAiqCore->mAiqCurParams);
    }

    LOGD_CAMGROUP("camgroup: prepare: prepare algos ...");

    for (auto algoHdl : mDefAlgoHandleList) {
        RkAiqCamgroupHandle* curHdl = algoHdl.ptr();
        while (curHdl) {
            if (curHdl->getEnable()) {
                /* update user initial params */
                ret = curHdl->updateConfig(true);
                RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d update initial user params failed", curHdl->getAlgoType());
                ret = curHdl->prepare(aiqCore);
                RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d prepare failed", curHdl->getAlgoType());
            }
            curHdl = curHdl->getNextHdl();
        }
    }


    LOGD_CAMGROUP("camgroup: reprocess init params ...");

    mInit = true;

    ret = reProcess(camGroupRes);
    if (ret < 0) {
        goto failed;
    }
    mInit = false;

    LOGD_CAMGROUP("camgroup: send init params to hwi ...");
    relayToHwi(camGroupRes);

    LOGD_CAMGROUP("camgroup: clear init params ...");
    // delete the processed result
    putGroupCamResult(camGroupRes);
    clearGroupCamResult(0);

    LOGD_CAMGROUP("camgroup: prepare done");

    mState = CAMGROUP_MANAGER_PREPARED;
    return XCAM_RETURN_NO_ERROR;
    EXIT_CAMGROUP_FUNCTION();
failed:
    putGroupCamResult(camGroupRes);
    clearGroupCamResult(-1);
    return ret;
}

XCamReturn
RkAiqCamGroupManager::bind(RkAiqManager* ctx)
{
    ENTER_CAMGROUP_FUNCTION();

    if (mState != CAMGROUP_MANAGER_INVALID &&
            mState != CAMGROUP_MANAGER_BINDED) {
        LOGE_CAMGROUP("in error state %d", mState);
        return XCAM_RETURN_ERROR_FAILED;
    }

    int camId = ctx->getCamPhyId();
    std::map<uint8_t, RkAiqManager*>::iterator it =
        mBindAiqsMap.find(camId);

    LOGD_CAMGROUP("camgroup: bind camId: %d to group", camId);

    if (it != mBindAiqsMap.end()) {
        return XCAM_RETURN_NO_ERROR;
    } else {
        mBindAiqsMap[camId] = ctx;
        mRequiredCamsResMask |= 1 << camId;
    }

    mState = CAMGROUP_MANAGER_BINDED;

    LOGD_CAMGROUP("camgroup: binded cams mask: 0x%x", mRequiredCamsResMask);

    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCamGroupManager::unbind(int camId)
{
    ENTER_CAMGROUP_FUNCTION();

    if (mState == CAMGROUP_MANAGER_STARTED) {
        LOGE_CAMGROUP("in error state %d", mState);
        return XCAM_RETURN_ERROR_FAILED;
    }

    LOGD_CAMGROUP("camgroup: unbind camId: %d from group", camId);

    std::map<uint8_t, RkAiqManager*>::iterator it =
        mBindAiqsMap.find(camId);

    if (it != mBindAiqsMap.end()) {
        mBindAiqsMap.erase(it);
        mRequiredCamsResMask &= ~(1 << camId);
        if (mBindAiqsMap.empty())
            mState = CAMGROUP_MANAGER_UNBINDED;
    } else {
        return XCAM_RETURN_NO_ERROR;
    }

    LOGD_CAMGROUP("camgroup: binded cams mask: 0x%x", mRequiredCamsResMask);

    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCamGroupManager::reProcess(rk_aiq_groupcam_result_t* gc_res)
{
    ENTER_CAMGROUP_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (mBindAiqsMap.empty()) {
        LOGW_CAMGROUP("no group cam, bypass");
        return XCAM_RETURN_NO_ERROR;
    }

    // assume all single cam runs same algos
    RkAiqManager* aiqManager = (mBindAiqsMap.begin())->second;
    RkAiqCore* aiqCore = aiqManager->mRkAiqAnalyzer.ptr();
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &aiqCore->mAlogsComSharedParams;

    LOGD_CAMGROUP("camgroup: set reprocess params ... ");

    int arraySize = mBindAiqsMap.size();
    rk_aiq_singlecam_3a_result_t* camgroupParmasArray[arraySize];

    memset(camgroupParmasArray, 0, sizeof(camgroupParmasArray));

    int i = 0, vaild_cam_ind = 0;
    rk_aiq_singlecam_3a_result_t* scam_3a_res = NULL;
    rk_aiq_singlecam_result_t* scam_res = NULL;
    RkAiqFullParams* aiqParams = NULL;

    for (i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
        if ((gc_res->_validCamResBits >> i) & 1) {
            scam_res = &gc_res->_singleCamResultsStatus[i]._singleCamResults;
            scam_3a_res = &scam_res->_3aResults;
            if (!scam_res->_fullIspParam.ptr())
                return XCAM_RETURN_ERROR_FAILED;
            aiqParams = scam_res->_fullIspParam->data().ptr();
            // fill 3a params
            if ((aiqParams->mExposureParams.ptr())) {
                scam_3a_res->aec.exp_tbl = aiqParams->mExposureParams->data()->exp_tbl;
                scam_3a_res->aec.exp_tbl_size = &aiqParams->mExposureParams->data()->exp_tbl_size;
            } else {
                LOGW_CAMGROUP("camId:%d, framId:%u, exp is null", i, gc_res->_frameId);
                // frame 1,2 exp may be null now
                //if (gc_res->_frameId == 1)
                return XCAM_RETURN_ERROR_PARAM;
            }

            if (!aiqParams->mAecParams.ptr())
                return XCAM_RETURN_ERROR_PARAM;
            scam_3a_res->aec._aeMeasParams = &aiqParams->mAecParams->data()->result;

            if (!aiqParams->mHistParams.ptr())
                return XCAM_RETURN_ERROR_PARAM;
            scam_3a_res->aec._aeHistMeasParams = &aiqParams->mHistParams->data()->result;

            if (CHECK_ISP_HW_V21()) {
                if (!aiqParams->mAwbV21Params.ptr())
                    return XCAM_RETURN_ERROR_PARAM;
                scam_3a_res->awb._awbCfgV201 = &aiqParams->mAwbV21Params->data()->result;
            } else {
                if (!aiqParams->mAwbV3xParams.ptr())
                    return XCAM_RETURN_ERROR_PARAM;
                scam_3a_res->awb._awbCfgV3x = &aiqParams->mAwbV3xParams->data()->result;
            }

            if (!aiqParams->mDehazeParams.ptr())
                return XCAM_RETURN_ERROR_PARAM;

            scam_3a_res->_adehazeConfig = &aiqParams->mDehazeParams->data()->result;

            if (!aiqParams->mMergeParams.ptr())
                return XCAM_RETURN_ERROR_PARAM;
            scam_3a_res->_amergeConfig = &aiqParams->mMergeParams->data()->result;

            if (!aiqParams->mAgammaParams.ptr())
                return XCAM_RETURN_ERROR_PARAM;
            scam_3a_res->_agammaConfig = &aiqParams->mAgammaParams->data()->result;

            if (!aiqParams->mDrcParams.ptr())
                return XCAM_RETURN_ERROR_PARAM;
            scam_3a_res->_adrcConfig = &aiqParams->mDrcParams->data()->result;

            if (!aiqParams->mAwbGainParams.ptr())
                return XCAM_RETURN_ERROR_PARAM;
            scam_3a_res->awb._awbGainParams = &aiqParams->mAwbGainParams->data()->result;

            if (!aiqParams->mLscParams.ptr())
                return XCAM_RETURN_ERROR_PARAM;
            scam_3a_res->_lscConfig = &aiqParams->mLscParams->data()->result;

            if (!aiqParams->mDpccParams.ptr())
                return XCAM_RETURN_ERROR_PARAM;
            scam_3a_res->_dpccConfig = &aiqParams->mDpccParams->data()->result;

            if (!aiqParams->mCcmParams.ptr())
                return XCAM_RETURN_ERROR_PARAM;
            scam_3a_res->_ccmCfg = &aiqParams->mCcmParams->data()->result;

            if (!aiqParams->mLut3dParams.ptr())
                return XCAM_RETURN_ERROR_PARAM;
            scam_3a_res->_lut3dCfg = &aiqParams->mLut3dParams->data()->result;

            if (!aiqParams->mBlcV21Params.ptr())
                return XCAM_RETURN_ERROR_PARAM;
            scam_3a_res->_blcConfig = &aiqParams->mBlcV21Params->data()->result;


            if (CHECK_ISP_HW_V21()) {
                if (!aiqParams->mYnrV21Params.ptr())
                    return XCAM_RETURN_ERROR_PARAM;
                scam_3a_res->aynr._aynr_procRes_v2 = &aiqParams->mYnrV21Params->data()->result;
            } else {
                if (!aiqParams->mYnrV3xParams.ptr())
                    return XCAM_RETURN_ERROR_PARAM;
                scam_3a_res->aynr._aynr_procRes_v3 = &aiqParams->mYnrV3xParams->data()->result;
            }

            if (CHECK_ISP_HW_V21()) {
                if (!aiqParams->mCnrV21Params.ptr())
                    return XCAM_RETURN_ERROR_PARAM;
                scam_3a_res->acnr._acnr_procRes_v1 = &aiqParams->mCnrV21Params->data()->result;
            } else {
                if (!aiqParams->mCnrV3xParams.ptr())
                    return XCAM_RETURN_ERROR_PARAM;
                scam_3a_res->acnr._acnr_procRes_v2 = &aiqParams->mCnrV3xParams->data()->result;
            }

            if (CHECK_ISP_HW_V21()) {
                if (!aiqParams->mSharpenV21Params.ptr())
                    return XCAM_RETURN_ERROR_PARAM;
                scam_3a_res->asharp._asharp_procRes_v3 = &aiqParams->mSharpenV21Params->data()->result;
            } else {
                if (!aiqParams->mSharpenV3xParams.ptr())
                    return XCAM_RETURN_ERROR_PARAM;
                scam_3a_res->asharp._asharp_procRes_v4 = &aiqParams->mSharpenV3xParams->data()->result;
            }


            if (CHECK_ISP_HW_V21()) {
                if (!aiqParams->mBaynrV21Params.ptr())
                    return XCAM_RETURN_ERROR_PARAM;
                scam_3a_res->abayernr._abayernr_procRes_v1 = &aiqParams->mBaynrV21Params->data()->result;
            } else {
                if (!aiqParams->mBaynrV3xParams.ptr())
                    return XCAM_RETURN_ERROR_PARAM;
                scam_3a_res->abayernr._abayer2dnr_procRes_v2 = &aiqParams->mBaynrV3xParams->data()->result;
            }

            if (CHECK_ISP_HW_V30()) {
                if (!aiqParams->mTnrV3xParams.ptr())
                    return XCAM_RETURN_ERROR_PARAM;
                scam_3a_res->abayertnr._abayertnr_procRes_v2 = &aiqParams->mTnrV3xParams->data()->result;
            }

            if (CHECK_ISP_HW_V30()) {
                if (!aiqParams->mGainV3xParams.ptr())
                    return XCAM_RETURN_ERROR_PARAM;
                scam_3a_res->again._again_procRes_v2 = &aiqParams->mGainV3xParams->data()->result;
            }
            camgroupParmasArray[vaild_cam_ind++] = scam_3a_res;
        }
    }

    if (vaild_cam_ind != arraySize) {
        LOGW_CAMGROUP("wrong num of valid cam res:%d,exp:%d",
                      vaild_cam_ind, arraySize);
    }

    uint32_t frameId = camgroupParmasArray[0]->_frameId;
    LOGD_CAMGROUP("camgroup: frameId:%u reprocessing ... ", frameId);

    for (auto algoHdl : mDefAlgoHandleList) {
        RkAiqCamgroupHandle* curHdl = algoHdl.ptr();
        while (curHdl) {
            if (curHdl->getEnable()) {
                /* update user initial params */
                ret = curHdl->updateConfig(true);
                RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d update initial user params failed", curHdl->getAlgoType());
                ret = curHdl->processing(camgroupParmasArray);
                LOGD_CAMGROUP("algoHdl %d processing success", curHdl->getAlgoType());
            }
            curHdl = curHdl->getNextHdl();
        }
    }


    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void
RkAiqCamGroupManager::relayToHwi(rk_aiq_groupcam_result_t* gc_res)
{
    rk_aiq_singlecam_result_t* singlecam_res = NULL;
    {
        int exp_tbl_size = -1;
        bool skip_apply_exp = false;
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            if ((gc_res->_validCamResBits >> i) & 1) {
                singlecam_res = &gc_res->_singleCamResultsStatus[i]._singleCamResults;
                if (singlecam_res->_fullIspParam->data()->mExposureParams.ptr()) {
                    int tmp_size = singlecam_res->_fullIspParam->data()->mExposureParams->data()->exp_tbl_size;
                    if (exp_tbl_size == -1)
                        exp_tbl_size = tmp_size;
                    else if (exp_tbl_size != tmp_size) {
                        skip_apply_exp = true;
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            // apply exposure directly
            if ((gc_res->_validCamResBits >> i) & 1) {
                singlecam_res = &gc_res->_singleCamResultsStatus[i]._singleCamResults;
                if (!skip_apply_exp) {
                    SmartPtr<RkAiqFullParams> fullParam = new RkAiqFullParams();
                    SmartPtr<RkAiqFullParamsProxy> fullParamProxy = new RkAiqFullParamsProxy(fullParam );
                    fullParamProxy->data()->mExposureParams = singlecam_res->_fullIspParam->data()->mExposureParams;
                    if (fullParamProxy->data()->mExposureParams.ptr()) {
                        LOGD_CAMGROUP("camgroup: camId:%d, frameId:%u, exp_tbl_size:%d",
                                      i, gc_res->_frameId, fullParamProxy->data()->mExposureParams->data()->exp_tbl_size);
                    }
                    mBindAiqsMap[i]->applyAnalyzerResult(fullParamProxy);
                }
                singlecam_res->_fullIspParam->data()->mExposureParams.release();
            }
        }
    }

    for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
        if ((gc_res->_validCamResBits >> i) & 1) {
            singlecam_res = &gc_res->_singleCamResultsStatus[i]._singleCamResults;
            if (mState == CAMGROUP_MANAGER_STARTED) {
                LOGD_CAMGROUP("camgroup: relay camId %d params to aiq manager !", i);
                mBindAiqsMap[i]->rkAiqCalcDone(singlecam_res->_fullIspParam);
            } else {
                LOGD_CAMGROUP("camgroup: apply camId %d params to hwi directly !", i);
                mBindAiqsMap[i]->applyAnalyzerResult(singlecam_res->_fullIspParam);
            }
        }
    }
}

XCamReturn
RkAiqCamGroupManager::addAlgo(RkAiqAlgoDesComm& algo)
{
    ENTER_ANALYZER_FUNCTION();

    std::map<int, SmartPtr<RkAiqCamgroupHandle>>* algo_map = getAlgoTypeHandleMap(algo.type);

    if (!algo_map) {
        LOGE_ANALYZER("do not support this algo type %d !", algo.type);
        return XCAM_RETURN_ERROR_FAILED;
    }
    // TODO, check if exist befor insert ?
    std::map<int, SmartPtr<RkAiqCamgroupHandle>>::reverse_iterator rit = algo_map->rbegin();

    algo.id = rit->first + 1;

    // add to map
    SmartPtr<RkAiqCamgroupHandle> new_hdl;
    if (algo.type == RK_AIQ_ALGO_TYPE_AE ||
            algo.type == RK_AIQ_ALGO_TYPE_AWB) {
        new_hdl = new RkAiqCamgroupHandle(&algo, this);
    } else {
        LOGE_ANALYZER("not supported custom algo type: %d ", algo.type);
        return XCAM_RETURN_ERROR_FAILED;
    }
    new_hdl->setEnable(false);
    rit->second->setNextHdl(new_hdl.ptr());
    new_hdl->setParentHdl((*algo_map)[0].ptr());

    (*algo_map)[algo.id] = new_hdl;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCamGroupManager::rmAlgo(int algoType, int id)
{
    ENTER_ANALYZER_FUNCTION();

    // can't remove default algos
    if (id == 0)
        return XCAM_RETURN_NO_ERROR;

    SmartPtr<RkAiqCamgroupHandle> def_algo_hdl = getDefAlgoTypeHandle(algoType);
    if (!def_algo_hdl.ptr()) {
        LOGE_ANALYZER("can't find current type %d algo", algoType);
        return XCAM_RETURN_ERROR_FAILED;
    }
    std::map<int, SmartPtr<RkAiqCamgroupHandle>>* algo_map = getAlgoTypeHandleMap(algoType);
    NULL_RETURN_RET(algo_map, XCAM_RETURN_ERROR_FAILED);
    std::map<int, SmartPtr<RkAiqCamgroupHandle>>::iterator it = algo_map->find(id);

    if (it == algo_map->end()) {
        LOGE_ANALYZER("can't find type id <%d, %d> algo", algoType, id);
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (mState == CAMGROUP_MANAGER_STARTED) {
        LOGE_ANALYZER("can't remove algo in running state");
        return XCAM_RETURN_ERROR_FAILED;
    }

    RkAiqCamgroupHandle* rmHdl = it->second.ptr();
    RkAiqCamgroupHandle* curHdl = def_algo_hdl.ptr();
    while (curHdl) {
        RkAiqCamgroupHandle* nextHdl = curHdl->getNextHdl();
        if (nextHdl == rmHdl) {
            curHdl->setNextHdl(nextHdl->getNextHdl());
            break;
        }
        curHdl = nextHdl;
    }

    algo_map->erase(it);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCamGroupManager::enableAlgo(int algoType, int id, bool enable)
{
    ENTER_ANALYZER_FUNCTION();

    if (mState == CAMGROUP_MANAGER_STARTED) {
        LOGE_ANALYZER("can't enable algo in running state");
        return XCAM_RETURN_ERROR_FAILED;
    }

    // get default algotype handle, id should be 0
    SmartPtr<RkAiqCamgroupHandle> def_algo_hdl = getDefAlgoTypeHandle(algoType);
    if (!def_algo_hdl.ptr()) {
        LOGE_ANALYZER("can't find current type %d algo", algoType);
        return XCAM_RETURN_ERROR_FAILED;
    }
    std::map<int, SmartPtr<RkAiqCamgroupHandle>>* algo_map = getAlgoTypeHandleMap(algoType);
    NULL_RETURN_RET(algo_map, XCAM_RETURN_ERROR_FAILED);
    std::map<int, SmartPtr<RkAiqCamgroupHandle>>::iterator it = algo_map->find(id);

    if (it == algo_map->end()) {
        LOGE_ANALYZER("can't find type id <%d, %d> algo", algoType, id);
        return XCAM_RETURN_ERROR_FAILED;
    }

    LOGI_ANALYZER("set algo type_id <%d,%d> to %d", algoType, id, enable);

    it->second->setEnable(enable);
    /* WARNING:
     * Be careful when use SmartPtr<RkAiqxxxHandle> = SmartPtr<RkAiqHandle>
     * if RkAiqxxxHandle is derived from multiple RkAiqHandle,
     * the ptr of RkAiqxxxHandle and RkAiqHandle IS NOT the same
     * (RkAiqHandle ptr = RkAiqxxxHandle ptr + offset), but seams like
     * SmartPtr do not deal with this correctly.
     */

    if (enable) {
        if (mState >= CAMGROUP_MANAGER_PREPARED) {
            RkAiqManager* aiqManager = (mBindAiqsMap.begin())->second;
            RkAiqCore* aiqCore = aiqManager->mRkAiqAnalyzer.ptr();
            RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &aiqCore->mAlogsComSharedParams;
            it->second->prepare(aiqCore);
        }
    }

    int enable_cnt = 0;
    RkAiqCamgroupHandle* curHdl = def_algo_hdl.ptr();

    while (curHdl) {
        if (curHdl->getEnable()) {
            enable_cnt++;
        }
        curHdl = curHdl->getNextHdl();
    }

    LOGI_ANALYZER("algo type %d enabled count :%d", algoType, enable_cnt);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

bool
RkAiqCamGroupManager::getAxlibStatus(int algoType, int id)
{
    std::map<int, SmartPtr<RkAiqCamgroupHandle>>* algo_map = getAlgoTypeHandleMap(algoType);
    NULL_RETURN_RET(algo_map, false);
    std::map<int, SmartPtr<RkAiqCamgroupHandle>>::iterator it = algo_map->find(id);

    if (it == algo_map->end()) {
        LOGE_ANALYZER("can't find type id <%d, %d> algo", algoType, id);
        return false;
    }

    LOGD_ANALYZER("algo type id <%d,%d> status %s", algoType, id,
                  it->second->getEnable() ? "enable" : "disable");

    return it->second->getEnable();
}

RkAiqAlgoContext*
RkAiqCamGroupManager::getEnabledAxlibCtx(const int algo_type)
{
    if (algo_type <= RK_AIQ_ALGO_TYPE_NONE ||
            algo_type >= RK_AIQ_ALGO_TYPE_MAX)
        return NULL;

    std::map<int, SmartPtr<RkAiqCamgroupHandle>>* algo_map = getAlgoTypeHandleMap(algo_type);
    std::map<int, SmartPtr<RkAiqCamgroupHandle>>::reverse_iterator rit = algo_map->rbegin();
    if (rit !=  algo_map->rend() && rit->second->getEnable())
        return rit->second->getAlgoCtx();
    else
        return NULL;
}

RkAiqAlgoContext*
RkAiqCamGroupManager::getAxlibCtx(const int algo_type, const int lib_id)
{
    if (algo_type <= RK_AIQ_ALGO_TYPE_NONE ||
            algo_type >= RK_AIQ_ALGO_TYPE_MAX)
        return NULL;

    std::map<int, SmartPtr<RkAiqCamgroupHandle>>* algo_map = getAlgoTypeHandleMap(algo_type);

    std::map<int, SmartPtr<RkAiqCamgroupHandle>>::iterator it = algo_map->find(lib_id);

    if (it != algo_map->end()) {
        return it->second->getAlgoCtx();
    }

    EXIT_ANALYZER_FUNCTION();

    return NULL;
}

RkAiqCamgroupHandle*
RkAiqCamGroupManager::getAiqCamgroupHandle(const int algo_type, const int lib_id)
{
    if (algo_type <= RK_AIQ_ALGO_TYPE_NONE ||
            algo_type >= RK_AIQ_ALGO_TYPE_MAX)
        return NULL;

    std::map<int, SmartPtr<RkAiqCamgroupHandle>>* algo_map = getAlgoTypeHandleMap(algo_type);

    if (!algo_map)
        return NULL;

    std::map<int, SmartPtr<RkAiqCamgroupHandle>>::iterator it = algo_map->find(0);

    if (it != algo_map->end()) {
        return it->second.ptr();
    }

    EXIT_ANALYZER_FUNCTION();

    return NULL;
}

void
RkAiqCamGroupManager::setVicapReady(rk_aiq_hwevt_t* hwevt) {
    SmartLock locker (mCamGroupApiSyncMutex);
    mVicapReadyMask |= 1 << hwevt->cam_id;
}

bool
RkAiqCamGroupManager::isAllVicapReady() {
    SmartLock locker (mCamGroupApiSyncMutex);
    return (mVicapReadyMask == mRequiredCamsResMask) ? true : false;
}

}; //namespace
