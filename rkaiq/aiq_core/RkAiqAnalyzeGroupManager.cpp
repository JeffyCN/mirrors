/*
 * Copyright (c) 2019-2021 Rockchip Eletronics Co., Ltd.
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
#include "RkAiqAnalyzeGroupManager.h"

#include <algorithm>
#include <functional>
#include <sstream>

#include "smart_buffer_priv.h"

#define ANALYZER_SUBM (0x1)

namespace RkCam {

RkAiqAnalyzerGroup::RkAiqAnalyzerGroup(RkAiqCore* aiqCore, enum rk_aiq_core_analyze_type_e type,
                                       const uint64_t flag, const RkAiqGrpConditions_t* grpConds,
                                       const bool singleThrd)
    : mAiqCore(aiqCore), mGroupType(type), mDepsFlag(flag) {
    mUserSetDelayCnts = INT8_MAX;
    mAwakenId         = (uint32_t)-1;
    if (grpConds)
        mGrpConds = *grpConds;
    if (!singleThrd) {
        std::stringstream ss;
        ss << "g-" << std::hex << mGroupType << std::hex << ":" << mDepsFlag;
        mRkAiqGroupMsgHdlTh = new RkAiqAnalyzeGroupMsgHdlThread(ss.str().c_str(), this);
    } else {
        mRkAiqGroupMsgHdlTh = nullptr;
    }
}

XCamReturn RkAiqAnalyzerGroup::start() {
    if (mRkAiqGroupMsgHdlTh.ptr()) {
        mRkAiqGroupMsgHdlTh->triger_start();
        mRkAiqGroupMsgHdlTh->start();
    }

    return XCAM_RETURN_NO_ERROR;
}

void RkAiqAnalyzerGroup::msgReduction(std::map<uint32_t, GroupMessage>& msgMap) {
    // TODO(Cody): Workaround for current implementation
    // Should let message thread handle the reduction
    if (!mGroupMsgMap.empty()) {
        const auto originalSize = mGroupMsgMap.size();
        const int numToErase    = originalSize - 2;
        if (numToErase > 0) {
            int32_t unreadyFlag = mDepsFlag & ~mGroupMsgMap.begin()->second.msg_flags;
            // print missing params
            std::string missing_conds;
            uint32_t i = 0;
            while (unreadyFlag) {
                if (unreadyFlag % 2) {
                    missing_conds.append(MessageType2Str[i]);
                    missing_conds.append(",");
                }
                unreadyFlag >>= 1;
                i++;
            }
            LOGW_ANALYZER_SUBM(ANALYZER_SUBM,
                    "camId: %d, group(%s): id[%d] map size is %d, erase %d, element, missing conditions: %s",
                    mAiqCore->mAlogsComSharedParams.mCamPhyId,
                    AnalyzerGroupType2Str[mGroupType], mGroupMsgMap.begin()->first,
                    originalSize, numToErase,
                    missing_conds.c_str());

            auto eraseIter = mGroupMsgMap.begin();
            std::advance(eraseIter, numToErase);
            mGroupMsgMap.erase(mGroupMsgMap.begin(), eraseIter);
        }
    }
}

bool RkAiqAnalyzerGroup::pushMsg(RkAiqCoreVdBufMsg& msg) {
    //msgReduction(mGroupMsgMap);
    if (mRkAiqGroupMsgHdlTh.ptr()) {
        mRkAiqGroupMsgHdlTh->push_msg(msg);
    }
    return true;
}

int8_t RkAiqAnalyzerGroup::getMsgDelayCnt(XCamMessageType &msg_id) {
    uint32_t i = 0;
    int8_t delayCnt = 0;
    for (i = 0; i < mGrpConds.size; i++) {
        if (mGrpConds.conds[i].cond == msg_id) {
            delayCnt = mGrpConds.conds[i].delay;
            if (delayCnt != 0 && mUserSetDelayCnts != INT8_MAX)
                delayCnt = mUserSetDelayCnts;
           return delayCnt;
        }
    }

    if (i == mGrpConds.size)
        LOGE_ANALYZER_SUBM(ANALYZER_SUBM, "don't match msgId(0x%x) in mGrpConds", msg_id);

    return 0;
}

void RkAiqAnalyzerGroup::setDelayCnts(int8_t delayCnts) {
    mUserSetDelayCnts = delayCnts;
}

void RkAiqAnalyzerGroup::setVicapScaleFlag(bool mode) {
    mVicapScaleStart = mode;
}

void RkAiqAnalyzerGroup::awakenClean(uint32_t sequence) {
    stop();
    mAwakenId = sequence;
    start();
}

bool RkAiqAnalyzerGroup::msgHandle(RkAiqCoreVdBufMsg* msg) {
    if (!((1ULL << msg->msg_id) & mDepsFlag)) {
        return true;
    }

    XCamMessageType  msg_id = msg->msg_id;
    uint32_t delayCnt = getMsgDelayCnt(msg_id);
    if (msg->frame_id == mAwakenId)
        delayCnt = 0;
    uint32_t userId = msg->frame_id + delayCnt;
    GroupMessage& msgWrapper = mGroupMsgMap[userId];

    msgWrapper.msg_flags |= 1ULL << msg->msg_id;
    //msgWrapper.msgList.push_back(msg);
    int& msg_cnts = msgWrapper.msg_cnts;
    if (msg_cnts >= MAX_MESSAGES) {
        LOGE_ANALYZER_SUBM(ANALYZER_SUBM,
            "camId: %d, group(%s): id[%d] push msg(%s), msg_cnts: %d overflow",
             mAiqCore->mAlogsComSharedParams.mCamPhyId, AnalyzerGroupType2Str[mGroupType], msg->frame_id,
             MessageType2Str[msg->msg_id], msg_cnts);
        for (int i = 0; i < msg_cnts; i++) {
            LOGE_ANALYZER_SUBM(ANALYZER_SUBM, "%d: fid:%d, msg:%s",
                               i, msgWrapper.msgList[i].frame_id, MessageType2Str[msgWrapper.msgList[i].msg_id]);
        }

        return true;
    }

    msgWrapper.msgList[msg_cnts++] = *msg;

    LOGD_ANALYZER_SUBM(ANALYZER_SUBM,
        "camId: %d, group(%s): id[%d] push msg(%s), msg delayCnt(%d), map size is %d",
         mAiqCore->mAlogsComSharedParams.mCamPhyId,
         AnalyzerGroupType2Str[mGroupType], msg->frame_id,
         MessageType2Str[msg->msg_id], delayCnt, mGroupMsgMap.size());

    uint64_t msg_flags = msgWrapper.msg_flags;
    if (!(msg_flags ^ mDepsFlag)) {
        std::array<RkAiqCoreVdBufMsg, MAX_MESSAGES>& msgList = msgWrapper.msgList;
        mHandler(msgList, msg_cnts, userId, getType());
#if 0
        for (auto it = mGroupMsgMap.begin(); it != mGroupMsgMap.end();) {
            if ((*it).first <= userId) {
                it = mGroupMsgMap.erase(it);
            } else {
                break;//it++;
            }
        }
#else
        std::map<uint32_t, GroupMessage>::iterator itup = mGroupMsgMap.upper_bound(userId);
        mGroupMsgMap.erase(mGroupMsgMap.begin(), itup);
#endif
        LOGD_ANALYZER("%s, group %s erase frame(%d) msg map\n", __FUNCTION__, AnalyzerGroupType2Str[mGroupType], userId);
    } else {
        msgReduction(mGroupMsgMap);
        return true;
    }

    return true;
}

XCamReturn RkAiqAnalyzerGroup::stop() {
    if (mRkAiqGroupMsgHdlTh.ptr()) {
        mRkAiqGroupMsgHdlTh->triger_stop();
        mRkAiqGroupMsgHdlTh->stop();
    }
    mGroupMsgMap.clear();

    return XCAM_RETURN_NO_ERROR;
}

void RkAiqAnalyzerGroup::setDepsFlagAndClearMap(uint64_t new_deps)
{
    mDepsFlag = new_deps;
    if (mGroupMsgMap.size()) {
        mGroupMsgMap.clear();
    }
}

bool RkAiqAnalyzeGroupMsgHdlThread::loop() {
    ENTER_ANALYZER_FUNCTION();

    const static int32_t timeout = -1;
    bool res = false;

    //XCAM_STATIC_FPS_CALCULATION(GROUPMSGTH, 100);
    RkAiqCoreVdBufMsg msg;
    XCamReturn ret = mMsgsQueue.pop(msg, timeout);
    if (ret) {
        LOGW_ANALYZER_SUBM(ANALYZER_SUBM, "RkAiqAnalyzeGroupMsgHdlThread got empty msg, stop thread");
        return false;
    }

    for (auto& grp : mHandlerGroups) {
        handleCalibUpdate(grp);
    }

    for (auto& grp : mHandlerGroups) {
        res = grp->msgHandle(&msg);
    }

    EXIT_ANALYZER_FUNCTION();

    return res;
}

XCamReturn RkAiqAnalyzeGroupMsgHdlThread::handleCalibUpdate(RkAiqAnalyzerGroup* grp)
{
  ENTER_ANALYZER_FUNCTION();
  XCamReturn ret = XCAM_RETURN_NO_ERROR;

  if (grp && grp->getAiqCore()) {
    grp->getAiqCore()->updateCalib(grp->getType());
  }

  EXIT_ANALYZER_FUNCTION();
  return ret;
}

RkAiqAnalyzeGroupManager::RkAiqAnalyzeGroupManager(RkAiqCore* aiqCore, bool single_thread)
    : mAiqCore(aiqCore), mSingleThreadMode(single_thread), mMsgThrd(nullptr) {}

uint64_t RkAiqAnalyzeGroupManager::getGrpDeps(rk_aiq_core_analyze_type_e group) {
    auto res = std::find_if(std::begin(mGroupMap), std::end(mGroupMap),
                            [&group](const std::pair<uint64_t, SmartPtr<RkAiqAnalyzerGroup>>& grp) {
                                return group == grp.second->getType();
                            });
    if (res != std::end(mGroupMap)) {
        return res->second->getDepsFlag();
    }

    return 0;
}

XCamReturn RkAiqAnalyzeGroupManager::setGrpDeps(rk_aiq_core_analyze_type_e group,
                                                uint64_t new_deps) {
    auto res = std::find_if(std::begin(mGroupMap), std::end(mGroupMap),
                            [&group](const std::pair<uint64_t, SmartPtr<RkAiqAnalyzerGroup>>& grp) {
                                return group == grp.second->getType();
                            });
    if (res != std::end(mGroupMap)) {
        uint64_t old_deps = res->second->getDepsFlag();

        if (old_deps == new_deps)
            return XCAM_RETURN_NO_ERROR;

        res->second->setDepsFlag(new_deps);
        return XCAM_RETURN_NO_ERROR;
    }

    return XCAM_RETURN_ERROR_PARAM;
}

XCamReturn RkAiqAnalyzeGroupManager::firstAnalyze() {

    RkAiqCore::RkAiqAlgosGroupShared_t* shared = nullptr;
    for (auto& it : mGroupMap) {
        uint64_t grpMask = mAiqCore->grpId2GrpMask(it.second->getType());
        mAiqCore->getGroupSharedParams(grpMask, shared);
        XCAM_ASSERT(shared != nullptr);
        mAiqCore->groupAnalyze(it.second->getType(), shared);
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAnalyzeGroupManager::start() {
    if (mSingleThreadMode) {
        mMsgThrd->triger_start();
        mMsgThrd->start();
    } else {
        for (auto& it : mGroupMap) {
            it.second->start();
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAnalyzeGroupManager::stop() {
    if (mSingleThreadMode) {
        mMsgThrd->triger_stop();
        mMsgThrd->stop();
    }

    for (auto& it : mGroupMap) {
        it.second->stop();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAnalyzeGroupManager::groupMessageHandler(std::array<RkAiqCoreVdBufMsg, MAX_MESSAGES>& msgs,
                                                         int msg_cnts, uint32_t id, uint64_t grpId) {
    uint64_t grpMask                           = mAiqCore->grpId2GrpMask(grpId);
    RkAiqCore::RkAiqAlgosGroupShared_t* shared = nullptr;
    mAiqCore->getGroupSharedParams(grpMask, shared);
    XCAM_ASSERT(shared != nullptr);
    RkAiqCoreVdBufMsg* vdBufMsg = NULL;
    shared->frameId = id;
    RkAiqSofInfoWrapperProxy* sofInfoMsg = nullptr;
    for (int i = 0; i < msg_cnts; i++) {
        vdBufMsg = &msgs[i];
        if (!vdBufMsg || !vdBufMsg->msg.ptr()) continue;
        switch (vdBufMsg->msg_id) {
            case XCAM_MESSAGE_SOF_INFO_OK:
                sofInfoMsg =
                    vdBufMsg->msg.get_cast_ptr<RkAiqSofInfoWrapperProxy>();
                shared->curExp = sofInfoMsg->data()->curExp->data()->aecExpInfo;
                shared->preExp = sofInfoMsg->data()->preExp->data()->aecExpInfo;
                shared->nxtExp = sofInfoMsg->data()->nxtExp->data()->aecExpInfo;
                shared->sof = sofInfoMsg->data()->sof;
                shared->iso = sofInfoMsg->data()->iso;
                break;
            case XCAM_MESSAGE_ISP_STATS_OK:
                shared->ispStats = convert_to_XCamVideoBuffer(vdBufMsg->msg);
                break;
            case XCAM_MESSAGE_AEC_STATS_OK:
                {
                    RkAiqAecStatsProxy* aecStats = vdBufMsg->msg.get_cast_ptr<RkAiqAecStatsProxy>();
                    if (aecStats)
                        shared->aecStatsBuf = aecStats->data().ptr();
                }
                break;
            case XCAM_MESSAGE_AWB_STATS_OK:
                {
                    RkAiqAwbStatsProxy* awbStats = vdBufMsg->msg.get_cast_ptr<RkAiqAwbStatsProxy>();
                    if (awbStats)
                        shared->awbStatsBuf = awbStats->data().ptr();
                }
                break;
            case XCAM_MESSAGE_AF_STATS_OK:
                {
                    RkAiqAfStatsProxy* afStats = vdBufMsg->msg.get_cast_ptr<RkAiqAfStatsProxy>();
                    if (afStats)
                        shared->afStatsBuf = afStats->data().ptr();
                }
                break;
            case XCAM_MESSAGE_ISP_POLL_SP_OK:
                shared->sp = convert_to_XCamVideoBuffer(vdBufMsg->msg);
                break;
            case XCAM_MESSAGE_ISP_GAIN_OK:
                shared->ispGain = convert_to_XCamVideoBuffer(vdBufMsg->msg);
                break;
            case XCAM_MESSAGE_ISP_POLL_TX_OK:
                shared->tx = convert_to_XCamVideoBuffer(vdBufMsg->msg);
                break;
            case XCAM_MESSAGE_ISPP_GAIN_KG_OK:
                shared->kgGain = convert_to_XCamVideoBuffer(vdBufMsg->msg);
                break;
            case XCAM_MESSAGE_ISPP_GAIN_WR_OK:
                shared->wrGain = convert_to_XCamVideoBuffer(vdBufMsg->msg);
                break;
            case XCAM_MESSAGE_ORB_STATS_OK:
                shared->orbStats = convert_to_XCamVideoBuffer(vdBufMsg->msg);
                break;
            case XCAM_MESSAGE_NR_IMG_OK:
                shared->nrImg = convert_to_XCamVideoBuffer(vdBufMsg->msg);
                break;
            case XCAM_MESSAGE_PDAF_STATS_OK:
                {
                    RkAiqPdafStatsProxy* pdafStats = vdBufMsg->msg.get_cast_ptr<RkAiqPdafStatsProxy>();
                    if (pdafStats)
                        shared->pdafStatsBuf = pdafStats->data().ptr();
                }
                break;
            case XCAM_MESSAGE_AE_PRE_RES_OK:
                shared->res_comb.ae_pre_res = convert_to_XCamVideoBuffer(vdBufMsg->msg);
                break;
            case XCAM_MESSAGE_AE_PROC_RES_OK:
                shared->res_comb.ae_proc_res = convert_to_XCamVideoBuffer(vdBufMsg->msg);
                break;
            case XCAM_MESSAGE_AWB_PROC_RES_OK:
                shared->res_comb.awb_proc_res = convert_to_XCamVideoBuffer(vdBufMsg->msg);
                break;
            case XCAM_MESSAGE_AMD_PROC_RES_OK:
                // xCamAmdProcRes = convert_to_XCamVideoBuffer(vdBufMsg->msg);
                break;
            case XCAM_MESSAGE_BLC_PROC_RES_OK:
                shared->res_comb.ablc_proc_res =
                    ((RkAiqAlgoProcResAblc*)vdBufMsg->msg->map())->ablc_proc_res;
                LOGD_ANALYZER_SUBM(
                    ANALYZER_SUBM, "camId: %d, group: %s: id: %d, blc_r: %d, blc1_r: %d",
                    mAiqCore->mAlogsComSharedParams.mCamPhyId, AnalyzerGroupType2Str[grpId],
                    vdBufMsg->msg->get_sequence(), shared->res_comb.ablc_proc_res->blc_r,
                    shared->res_comb.ablc_proc_res->blc1_r);
                break;
            case XCAM_MESSAGE_BLC_V32_PROC_RES_OK:
                shared->res_comb.ablcV32_proc_res =
                    ((RkAiqAlgoProcResAblcV32*)vdBufMsg->msg->map())->ablcV32_proc_res;
                LOGD_ANALYZER_SUBM(
                    ANALYZER_SUBM,
                    "camId: %d, group: %s: id: %d, ob_offset: %d, ob_predgain: %f",
                    mAiqCore->mAlogsComSharedParams.mCamPhyId, AnalyzerGroupType2Str[grpId],
                    vdBufMsg->msg->get_sequence(),
                    shared->res_comb.ablcV32_proc_res->isp_ob_offset,
                    shared->res_comb.ablcV32_proc_res->isp_ob_predgain);
                break;
            case XCAM_MESSAGE_YNR_V3_PROC_RES_OK:
                shared->res_comb.aynrV3_proc_res =
                    &((RkAiqAlgoProcResAynrV3*)vdBufMsg->msg->map())->stAynrProcResult;
                LOGD_ANALYZER_SUBM(ANALYZER_SUBM,
                                   "camId: %d, group: %s: id: %d, sigma: %f %f %f %f %f %f %f "
                                   "%f %f %f %f %f %f %f %f %f %f ",
                                   mAiqCore->mAlogsComSharedParams.mCamPhyId,
                                   AnalyzerGroupType2Str[grpId],
                                   vdBufMsg->msg->get_sequence(),
                                   shared->res_comb.aynrV3_proc_res->stSelect->sigma[0],
                                   shared->res_comb.aynrV3_proc_res->stSelect->sigma[1],
                                   shared->res_comb.aynrV3_proc_res->stSelect->sigma[2],
                                   shared->res_comb.aynrV3_proc_res->stSelect->sigma[3],
                                   shared->res_comb.aynrV3_proc_res->stSelect->sigma[4],
                                   shared->res_comb.aynrV3_proc_res->stSelect->sigma[5],
                                   shared->res_comb.aynrV3_proc_res->stSelect->sigma[6],
                                   shared->res_comb.aynrV3_proc_res->stSelect->sigma[7],
                                   shared->res_comb.aynrV3_proc_res->stSelect->sigma[8],
                                   shared->res_comb.aynrV3_proc_res->stSelect->sigma[9],
                                   shared->res_comb.aynrV3_proc_res->stSelect->sigma[10],
                                   shared->res_comb.aynrV3_proc_res->stSelect->sigma[11],
                                   shared->res_comb.aynrV3_proc_res->stSelect->sigma[12],
                                   shared->res_comb.aynrV3_proc_res->stSelect->sigma[13],
                                   shared->res_comb.aynrV3_proc_res->stSelect->sigma[14],
                                   shared->res_comb.aynrV3_proc_res->stSelect->sigma[15],
                                   shared->res_comb.aynrV3_proc_res->stSelect->sigma[16]);
                break;
            case XCAM_MESSAGE_YNR_V22_PROC_RES_OK:
                shared->res_comb.aynrV22_proc_res =
                    &((RkAiqAlgoProcResAynrV22*)vdBufMsg->msg->map())->stAynrProcResult;
                LOGD_ANALYZER_SUBM(ANALYZER_SUBM,
                                   "camId: %d, group: %s: id: %d, sigma: %f %f %f %f %f %f %f "
                                   "%f %f %f %f %f %f %f %f %f %f ",
                                   mAiqCore->mAlogsComSharedParams.mCamPhyId,
                                   AnalyzerGroupType2Str[grpId],
                                   vdBufMsg->msg->get_sequence(),
                                   shared->res_comb.aynrV22_proc_res->stSelect->sigma[0],
                                   shared->res_comb.aynrV22_proc_res->stSelect->sigma[1],
                                   shared->res_comb.aynrV22_proc_res->stSelect->sigma[2],
                                   shared->res_comb.aynrV22_proc_res->stSelect->sigma[3],
                                   shared->res_comb.aynrV22_proc_res->stSelect->sigma[4],
                                   shared->res_comb.aynrV22_proc_res->stSelect->sigma[5],
                                   shared->res_comb.aynrV22_proc_res->stSelect->sigma[6],
                                   shared->res_comb.aynrV22_proc_res->stSelect->sigma[7],
                                   shared->res_comb.aynrV22_proc_res->stSelect->sigma[8],
                                   shared->res_comb.aynrV22_proc_res->stSelect->sigma[9],
                                   shared->res_comb.aynrV22_proc_res->stSelect->sigma[10],
                                   shared->res_comb.aynrV22_proc_res->stSelect->sigma[11],
                                   shared->res_comb.aynrV22_proc_res->stSelect->sigma[12],
                                   shared->res_comb.aynrV22_proc_res->stSelect->sigma[13],
                                   shared->res_comb.aynrV22_proc_res->stSelect->sigma[14],
                                   shared->res_comb.aynrV22_proc_res->stSelect->sigma[15],
                                   shared->res_comb.aynrV22_proc_res->stSelect->sigma[16]);
                break;
            case XCAM_MESSAGE_YNR_V24_PROC_RES_OK:
                shared->res_comb.aynrV24_proc_res =
                    &((RkAiqAlgoProcResAynrV24*)vdBufMsg->msg->map())->stAynrProcResult;
                LOGD_ANALYZER_SUBM(ANALYZER_SUBM,
                                   "camId: %d, group: %s: id: %d, sigma: %f %f %f %f %f %f %f "
                                   "%f %f %f %f %f %f %f %f %f %f ",
                                   mAiqCore->mAlogsComSharedParams.mCamPhyId,
                                   AnalyzerGroupType2Str[grpId],
                                   vdBufMsg->msg->get_sequence(),
                                   shared->res_comb.aynrV24_proc_res->stSelect->sigma[0],
                                   shared->res_comb.aynrV24_proc_res->stSelect->sigma[1],
                                   shared->res_comb.aynrV24_proc_res->stSelect->sigma[2],
                                   shared->res_comb.aynrV24_proc_res->stSelect->sigma[3],
                                   shared->res_comb.aynrV24_proc_res->stSelect->sigma[4],
                                   shared->res_comb.aynrV24_proc_res->stSelect->sigma[5],
                                   shared->res_comb.aynrV24_proc_res->stSelect->sigma[6],
                                   shared->res_comb.aynrV24_proc_res->stSelect->sigma[7],
                                   shared->res_comb.aynrV24_proc_res->stSelect->sigma[8],
                                   shared->res_comb.aynrV24_proc_res->stSelect->sigma[9],
                                   shared->res_comb.aynrV24_proc_res->stSelect->sigma[10],
                                   shared->res_comb.aynrV24_proc_res->stSelect->sigma[11],
                                   shared->res_comb.aynrV24_proc_res->stSelect->sigma[12],
                                   shared->res_comb.aynrV24_proc_res->stSelect->sigma[13],
                                   shared->res_comb.aynrV24_proc_res->stSelect->sigma[14],
                                   shared->res_comb.aynrV24_proc_res->stSelect->sigma[15],
                                   shared->res_comb.aynrV24_proc_res->stSelect->sigma[16]);
                break;
#if USE_NEWSTRUCT
            case XCAM_MESSAGE_YNR_PROC_RES_OK:
                shared->res_comb.ynr_proc_res =
                    ((RkAiqAlgoProcResYnr*)vdBufMsg->msg->map())->ynrRes;
                /*
                LOGD_ANALYZER_SUBM(ANALYZER_SUBM,
                                   "camId: %d, group: %s: id: %d, sigma: %d %d %d %d %d %d %d "
                                   "%d %d %d %d %d %d %d %d %d %d ",
                                   mAiqCore->mAlogsComSharedParams.mCamPhyId,
                                   AnalyzerGroupType2Str[grpId],
                                   vdBufMsg->msg->get_sequence(),
                                   shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[0],
                                   shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[1],
                                   shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[2],
                                   shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[3],
                                   shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[4],
                                   shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[5],
                                   shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[6],
                                   shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[7],
                                   shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[8],
                                   shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[9],
                                   shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[10],
                                   shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[11],
                                   shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[12],
                                   shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[13],
                                   shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[14],
                                   shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[15],
                                   shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[16]);
                */
                break;
#endif
            case XCAM_MESSAGE_ADEHAZE_STATS_OK:
                {
                    RkAiqAdehazeStatsProxy* dehazeStats = vdBufMsg->msg.get_cast_ptr<RkAiqAdehazeStatsProxy>();
                    if (dehazeStats)
                        shared->adehazeStatsBuf = dehazeStats->data().ptr();
                }
                break;
            case XCAM_MESSAGE_AGAIN_STATS_OK:
                {
                    RkAiqAgainStatsProxy* againStats = vdBufMsg->msg.get_cast_ptr<RkAiqAgainStatsProxy>();
                    if (againStats)
                        shared->againStatsBuf = againStats->data().ptr();
                }
                break;
            case XCAM_MESSAGE_VICAP_POLL_SCL_OK:
            {
                RkAiqVicapRawBufInfo_t *buf_info = (RkAiqVicapRawBufInfo_t *)vdBufMsg->msg->map();
                shared->scaleRawInfo.bpp = buf_info->bpp;
                if (buf_info->flags == RK_AIQ_VICAP_SCALE_HDR_MODE_NORMAL) {
                    shared->scaleRawInfo.raw_s = convert_to_XCamVideoBuffer(buf_info->raw_s);
                } else if (buf_info->flags == RK_AIQ_VICAP_SCALE_HDR_MODE_2_HDR) {
                    shared->scaleRawInfo.raw_l = convert_to_XCamVideoBuffer(buf_info->raw_l);
                    shared->scaleRawInfo.raw_s = convert_to_XCamVideoBuffer(buf_info->raw_s);
                } else if (buf_info->flags == RK_AIQ_VICAP_SCALE_HDR_MODE_3_HDR) {
                    shared->scaleRawInfo.raw_l = convert_to_XCamVideoBuffer(buf_info->raw_l);
                    shared->scaleRawInfo.raw_s = convert_to_XCamVideoBuffer(buf_info->raw_s);
                    shared->scaleRawInfo.raw_m = convert_to_XCamVideoBuffer(buf_info->raw_m);
                }
                break;
            }
            default:
                break;
        }
    }
    //msgs.clear();

    mAiqCore->groupAnalyze(grpId, shared);
    //msgs.clear();

    if (shared->aecStatsBuf) {
        shared->aecStatsBuf = nullptr;
    }
    if (shared->awbStatsBuf) {
        shared->awbStatsBuf = nullptr;
    }
    if (shared->afStatsBuf) {
        shared->afStatsBuf = nullptr;
    }
    if (shared->ispStats) {
        shared->ispStats->unref(shared->ispStats);
        shared->ispStats = nullptr;
    }
    if (shared->tx) {
        shared->tx->unref(shared->tx);
        shared->tx = nullptr;
    }
    if (shared->sp) {
        shared->sp->unref(shared->sp);
        shared->sp = nullptr;
    }
    if (shared->ispGain) {
        shared->ispGain->unref(shared->ispGain);
        shared->ispGain = nullptr;
    }
    if (shared->kgGain) {
        shared->kgGain->unref(shared->kgGain);
        shared->kgGain = nullptr;
    }
    if (shared->wrGain) {
        shared->wrGain->unref(shared->wrGain);
        shared->wrGain = nullptr;
    }
    if (shared->orbStats) {
        shared->orbStats->unref(shared->orbStats);
        shared->orbStats = nullptr;
    }
    if (shared->nrImg) {
        shared->nrImg->unref(shared->nrImg);
        shared->nrImg = nullptr;
    }
    if (shared->pdafStatsBuf) {
        shared->pdafStatsBuf = nullptr;
    }
    if (shared->res_comb.ae_pre_res) {
        shared->res_comb.ae_pre_res->unref(shared->res_comb.ae_pre_res);
        shared->res_comb.ae_pre_res = nullptr;
    }
    if (shared->res_comb.ae_proc_res) {
        shared->res_comb.ae_proc_res->unref(shared->res_comb.ae_proc_res);
        shared->res_comb.ae_proc_res = nullptr;
    }
    if (shared->res_comb.awb_proc_res) {
        shared->res_comb.awb_proc_res->unref(shared->res_comb.awb_proc_res);
        shared->res_comb.awb_proc_res = nullptr;
    }
    if (shared->adehazeStatsBuf) {
        shared->adehazeStatsBuf = nullptr;
    }
    if (shared->scaleRawInfo.raw_l) {
        shared->scaleRawInfo.raw_l->unref(shared->scaleRawInfo.raw_l);
        shared->scaleRawInfo.raw_l = nullptr;
    }
    if (shared->scaleRawInfo.raw_m) {
        shared->scaleRawInfo.raw_m->unref(shared->scaleRawInfo.raw_m);
        shared->scaleRawInfo.raw_m = nullptr;
    }
    if (shared->scaleRawInfo.raw_s) {
        shared->scaleRawInfo.raw_s->unref(shared->scaleRawInfo.raw_s);
        shared->scaleRawInfo.raw_s = nullptr;
    }
    if (shared->againStatsBuf) {
        shared->againStatsBuf = nullptr;
    }

    return XCAM_RETURN_NO_ERROR;
}

#if defined(RKAIQ_HAVE_THUMBNAILS)
XCamReturn RkAiqAnalyzeGroupManager::thumbnailsGroupMessageHandler(
    std::array<RkAiqCoreVdBufMsg, MAX_MESSAGES>& msgs, int msg_cnts, uint32_t id, uint64_t grpId) {
    //XCAM_STATIC_FPS_CALCULATION(THUMBHANDLER, 100);
    rkaiq_image_source_t thumbnailsSrc;
    RkAiqCoreVdBufMsg* vdBufMsg = &msgs.front();

    if (!vdBufMsg) return XCAM_RETURN_BYPASS;

    if (vdBufMsg->msg_id == XCAM_MESSAGE_ISP_POLL_SP_OK) {
        XCamVideoBuffer* sp = NULL;
        sp                  = convert_to_XCamVideoBuffer(vdBufMsg->msg);

        thumbnailsSrc.frame_id     = vdBufMsg->frame_id;
        thumbnailsSrc.src_type     = RKISP20_STREAM_SP;
        thumbnailsSrc.image_source = sp;
    } else if (vdBufMsg->msg_id == XCAM_MESSAGE_NR_IMG_OK) {
        XCamVideoBuffer* nrImg = NULL;
        nrImg                  = convert_to_XCamVideoBuffer(vdBufMsg->msg);

        thumbnailsSrc.frame_id     = vdBufMsg->frame_id;
        thumbnailsSrc.src_type     = RKISP20_STREAM_NR;
        thumbnailsSrc.image_source = nrImg;
    }

    mAiqCore->thumbnailsGroupAnalyze(thumbnailsSrc);

    return XCAM_RETURN_NO_ERROR;
}
#endif

void RkAiqAnalyzeGroupManager::parseAlgoGroup(const struct RkAiqAlgoDesCommExt* algoDes) {
    uint64_t disAlgosMask = mAiqCore->getInitDisAlgosMask();
    if (mSingleThreadMode) {
        mMsgThrd = new RkAiqAnalyzeGroupMsgHdlThread("GrpMsgThrd", nullptr);
        XCAM_ASSERT(mMsgThrd.ptr() != nullptr);
    }
    for (size_t i = 0; algoDes[i].des != NULL; i++) {
        int algo_type = algoDes[i].des->type;
        if ((1ULL << algo_type) & disAlgosMask)
            continue;

        if (algoDes[i].grpConds.size > MAX_MESSAGES)
            LOGE("group conds > %d, should increase MAX_MESSAGES", MAX_MESSAGES);

        uint64_t deps_flag = 0;
        for (size_t j = 0; j < algoDes[i].grpConds.size; j++)
            deps_flag |= 1ULL << algoDes[i].grpConds.conds[j].cond;
        rk_aiq_core_analyze_type_e group = algoDes[i].group;
        if (mAiqCore->getCurAlgoTypeHandle(algo_type)) {
            mGroupAlgoListMap[group].push_back(*mAiqCore->getCurAlgoTypeHandle(algo_type));
            mGroupAlgoListMap[RK_AIQ_CORE_ANALYZE_ALL].push_back(*mAiqCore->getCurAlgoTypeHandle(algo_type));
        }
        if (mGroupMap.count(group)) {
            continue;
        }
        mGroupMap[group] = new RkAiqAnalyzerGroup(mAiqCore, group, deps_flag,
                                                      &algoDes[i].grpConds, mSingleThreadMode);
        if (mSingleThreadMode) mMsgThrd->add_group(mGroupMap[group].ptr());
#if defined(RKAIQ_HAVE_THUMBNAILS)
        if (group == RK_AIQ_CORE_ANALYZE_THUMBNAILS) {
            mGroupMap[group]->setConcreteHandler(
                std::bind(&RkAiqAnalyzeGroupManager::thumbnailsGroupMessageHandler, this,
                          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        } else {
            mGroupMap[group]->setConcreteHandler(
                std::bind(&RkAiqAnalyzeGroupManager::groupMessageHandler, this,
                          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        }
#else
        mGroupMap[group]->setConcreteHandler(
            std::bind(&RkAiqAnalyzeGroupManager::groupMessageHandler, this, std::placeholders::_1,
                      std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
#endif
        LOGD_ANALYZER_SUBM(ANALYZER_SUBM, "Created group %" PRIx64 " for dep flags %" PRIx64"", (uint64_t)group, deps_flag);
    }
}

XCamReturn RkAiqAnalyzeGroupManager::handleMessage(RkAiqCoreVdBufMsg& msg) {
    //XCAM_STATIC_FPS_CALCULATION(HANDLEMSG, 100);
    if (mSingleThreadMode) {
        mMsgThrd->push_msg(msg);
        LOGD_ANALYZER_SUBM(ANALYZER_SUBM, "camId: %d, Handle message(%s) id[%d]",
                           mAiqCore->mAlogsComSharedParams.mCamPhyId,
                           MessageType2Str[msg.msg_id],
                           msg.frame_id);
    } else {
        for (auto& it : mGroupMap) {
            if ((it.second->getDepsFlag() & (1ULL << msg.msg_id)) != 0) {
                LOGD_ANALYZER_SUBM(ANALYZER_SUBM,
                    "camId: %d, Handle message(%s) id[%d] on group(%s), flags %" PRIx64 "",
                    mAiqCore->mAlogsComSharedParams.mCamPhyId,
                    MessageType2Str[msg.msg_id], msg.frame_id,
                    AnalyzerGroupType2Str[it.second->getType()], it.second->getDepsFlag());

                it.second->pushMsg(msg);
            }
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

void RkAiqAnalyzeGroupManager::setDelayCnts(int delayCnts) {
    for (auto& it : mGroupMap) {
        it.second->setDelayCnts(delayCnts);
    }
}

}  // namespace RkCam
