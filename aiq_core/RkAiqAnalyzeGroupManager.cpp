/*
 * rkisp_aiq_core.h
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

#include "RkAiqAnalyzeGroupManager.h"

#include <algorithm>
#include <functional>
#include <sstream>

#include "smart_buffer_priv.h"

#define ANALYZER_SUBM (0x1)

namespace RkCam {

RkAiqAnalyzerGroup::RkAiqAnalyzerGroup(RkAiqCore* aiqCore, enum rk_aiq_core_analyze_type_e type,
                                       const uint64_t flag, const RkAiqGrpConditions_t* grpConds)
    : mAiqCore(aiqCore), mGroupType(type), mDepsFlag(flag) {
    if (grpConds)
        mGrpConds = *grpConds;
    std::stringstream ss;
    ss << "g-" << std::hex << mGroupType << std::hex << ":" << mDepsFlag;
    mRkAiqGroupMsgHdlTh = new RkAiqAnalyzeGroupMsgHdlThread(ss.str().c_str(), this);
}

XCamReturn RkAiqAnalyzerGroup::start() {
    mRkAiqGroupMsgHdlTh->triger_start();
    mRkAiqGroupMsgHdlTh->start();

    return XCAM_RETURN_NO_ERROR;
}

void RkAiqAnalyzerGroup::msgReduction(std::map<uint32_t, GroupMessage>& msgMap) {
    // TODO(Cody): Workaround for current implementation
    // Should let message thread handle the reduction
    if (mGroupMsgMap.size() > 0) {
        const auto originalSize = mGroupMsgMap.size();
        const int numToErase    = originalSize - 5;
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
            LOGE_ANALYZER_SUBM(ANALYZER_SUBM,
                    "group(%s): id[%d] map size is %d, erase %d, element, missing conditions: %s",
                    AnalyzerGroupType2Str[mGroupType], mGroupMsgMap.begin()->first,
                    originalSize, numToErase,
                    missing_conds.c_str());

            auto eraseIter = mGroupMsgMap.begin();
            std::advance(eraseIter, numToErase);
            mGroupMsgMap.erase(mGroupMsgMap.begin(), eraseIter);
        }
    }
}

bool RkAiqAnalyzerGroup::pushMsg(const SmartPtr<XCamMessage>& msg) {
    //msgReduction(mGroupMsgMap);
    mRkAiqGroupMsgHdlTh->push_msg(msg);
    return true;
}

int8_t RkAiqAnalyzerGroup::getMsgDelayCnt(XCamMessageType &msg_id) {
    uint32_t i = 0, delayCnt = 0;
    for (i = 0; i < mGrpConds.size; i++) {
        if (mGrpConds.conds[i].cond == msg_id)
           return mGrpConds.conds[i].delay;
    }

    if (i == mGrpConds.size)
        LOGE_ANALYZER_SUBM(ANALYZER_SUBM, "don't match msgId(0x%x) in mGrpConds", msg_id);

    return 0;
}

XCamReturn RkAiqAnalyzerGroup::msgHandle(const SmartPtr<XCamMessage>& msg) {
    if (!msg.ptr()) {
        LOGW_ANALYZER_SUBM(ANALYZER_SUBM, "msg is nullptr!");
        return XCAM_RETURN_ERROR_PARAM;
    }

    uint32_t delayCnt = getMsgDelayCnt(msg->msg_id);
    uint32_t userId = msg->frame_id + delayCnt;
    GroupMessage& msgWrapper = mGroupMsgMap[userId];
    msgWrapper.msg_flags |= 1 << msg->msg_id;
    msgWrapper.msgList.push_back(msg);
    LOGD_ANALYZER_SUBM(ANALYZER_SUBM,
        "group(%s): id[%d] push msg(%s), msg delayCnt(%d), map size is %d\n",
         AnalyzerGroupType2Str[mGroupType], msg->frame_id,
         MessageType2Str[msg->msg_id], delayCnt, mGroupMsgMap.size());

    uint64_t msg_flags = msgWrapper.msg_flags;
    if (!(msg_flags ^ mDepsFlag)) {
        for (auto it = mGroupMsgMap.begin(); it != mGroupMsgMap.end();) {
            if ((*it).first < userId) {
                it = mGroupMsgMap.erase(it);
            } else {
                break;//it++;
            }
        }
        std::list<SmartPtr<XCamMessage>>& msgList = msgWrapper.msgList;
        mHandler(msgList, userId);
        mGroupMsgMap.erase(userId);
        LOGD_ANALYZER("%s, erase frame(%d) msg map\n", __FUNCTION__, msg->frame_id);
    } else {
        msgReduction(mGroupMsgMap);
        return XCAM_RETURN_BYPASS;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAnalyzerGroup::stop() {
    mRkAiqGroupMsgHdlTh->triger_stop();
    mRkAiqGroupMsgHdlTh->stop();
    mGroupMsgMap.clear();

    return XCAM_RETURN_NO_ERROR;
}

bool RkAiqAnalyzeGroupMsgHdlThread::loop() {
    ENTER_ANALYZER_FUNCTION();

    const static int32_t timeout = -1;

    //XCAM_STATIC_FPS_CALCULATION(GROUPMSGTH, 100);
    SmartPtr<XCamMessage> msg = mMsgsQueue.pop(timeout);
    if (!msg.ptr()) {
        LOGW_ANALYZER_SUBM(ANALYZER_SUBM, "RkAiqAnalyzeGroupMsgHdlThread got empty msg, stop thread");
        return false;
    }

    XCamReturn ret = mRkAiqAnalyzerGroup->msgHandle(msg);
    if (ret == XCAM_RETURN_NO_ERROR || ret == XCAM_RETURN_ERROR_TIMEOUT || XCAM_RETURN_BYPASS)
        return true;

    EXIT_ANALYZER_FUNCTION();

    return false;
}

RkAiqAnalyzeGroupManager::RkAiqAnalyzeGroupManager(RkAiqCore* aiqCore) : mAiqCore(aiqCore) {}

XCamReturn RkAiqAnalyzeGroupManager::start() {
    for (auto& it : mGroupMap) {
        it.second->start();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAnalyzeGroupManager::stop() {
    for (auto& it : mGroupMap) {
        it.second->stop();
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAnalyzeGroupManager::aeGroupMessageHandler(
    std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id) {
    XCamVideoBuffer* aecStatsBuf         = NULL;
    list<SmartPtr<XCamMessage>>* msgList = &msgs;
    list<SmartPtr<XCamMessage>>::iterator listIt;
    //XCAM_STATIC_FPS_CALCULATION(AEHANDLER, 100);
    for (listIt = msgList->begin(); listIt != msgList->end();) {
        if ((*listIt)->msg_id == XCAM_MESSAGE_AEC_STATS_OK) {
            SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr())
                aecStatsBuf = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        }
        listIt = msgList->erase(listIt);
    }

    // 4. start executing algorithm module flow
    if (aecStatsBuf) {
        mAiqCore->aeGroupAnalyze(id, aecStatsBuf);
        aecStatsBuf->unref(aecStatsBuf);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAnalyzeGroupManager::awbGroupMessageHandler(
    std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id) {
    // 3. determine if all the required messages have been received
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    XCamVideoBuffer* awbStatsBuf               = nullptr;
    XCamVideoBuffer* aePreRes                  = nullptr;
    list<SmartPtr<XCamMessage>>* msgList       = &msgs;
    list<SmartPtr<XCamMessage>>::iterator listIt;
    //XCAM_STATIC_FPS_CALCULATION(AWBHANDLER, 100);
    for (listIt = msgList->begin(); listIt != msgList->end();) {
        if ((*listIt)->msg_id == XCAM_MESSAGE_AE_PRE_RES_OK) {
            SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg;
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr())
                aePreRes = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AWB_STATS_OK) {
            SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr())
                awbStatsBuf = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        }

        listIt = msgList->erase(listIt);
    }

    // 4. start executing algorithm module flow
    if (aePreRes != nullptr && awbStatsBuf != nullptr)
        mAiqCore->awbGroupAnalyze(id, aePreRes, awbStatsBuf);
    else
        ret = XCAM_RETURN_BYPASS;

    if (aePreRes)
        aePreRes->unref(aePreRes);
    if (awbStatsBuf)
        awbStatsBuf->unref(awbStatsBuf);

    return ret;
}

XCamReturn RkAiqAnalyzeGroupManager::measGroupMessageHandler(
    std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id) {
    // 3. determine if all the required messages have been received
    SmartPtr<RkAiqCoreIspStatsMsg> IspStatsMsg = nullptr;
    XCamVideoBuffer* aePreRes                 = NULL;
    list<SmartPtr<XCamMessage>>* msgList       = &msgs;
    list<SmartPtr<XCamMessage>>::iterator listIt;
    //XCAM_STATIC_FPS_CALCULATION(MEASHANDLER, 100);
    for (listIt = msgList->begin(); listIt != msgList->end();) {
        if ((*listIt)->msg_id == XCAM_MESSAGE_ISP_STATS_OK) {
            IspStatsMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreIspStatsMsg>();
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AE_PRE_RES_OK) {
            SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg;
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr()) {
                aePreRes = convert_to_XCamVideoBuffer(vdBufMsg->msg);
            }
        }

        listIt = msgList->erase(listIt);
    }

    // 4. start executing algorithm module flow
    if (IspStatsMsg.ptr())
        mAiqCore->measGroupAnalyze(id, IspStatsMsg, aePreRes);

    if (aePreRes)
        aePreRes->unref(aePreRes);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAnalyzeGroupManager::otherGroupMessageHandler(
    std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id) {
    //XCAM_STATIC_FPS_CALCULATION(OTHERHANDLER, 100);
    SmartPtr<RkAiqCoreExpMsg> sofInfoMsg = msgs.front().dynamic_cast_ptr<RkAiqCoreExpMsg>();
    msgs.pop_front();

    // 1. clear the excess elements
    // 2. add msg to group msg map
    // 3. determine if all the required messages have been received
    // 4. start executing algorithm module flow
    mAiqCore->otherGroupAnalye(id, sofInfoMsg);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAnalyzeGroupManager::amdGroupMessageHandler(
    std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id) {
    //XCAM_STATIC_FPS_CALCULATION(AMDHANDLER, 100);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    // 3. determine if all the required messages have been received
    XCamVideoBuffer* sp                  = NULL;
    XCamVideoBuffer* ispGain             = NULL;
    SmartPtr<RkAiqCoreExpMsg> sofInfoMsg = nullptr;

    list<SmartPtr<XCamMessage>>* msgList = &msgs;
    list<SmartPtr<XCamMessage>>::iterator listIt;
    for (listIt = msgList->begin(); listIt != msgList->end();) {
        SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg;
        if ((*listIt)->msg_id == XCAM_MESSAGE_SOF_INFO_OK) {
            sofInfoMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreExpMsg>();
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_ISP_POLL_SP_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr()) sp = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_ISP_GAIN_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr()) ispGain = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        }

        listIt = msgList->erase(listIt);
    }

    // 4. start executing algorithm module flow
    if (sofInfoMsg.ptr() && sp != NULL && ispGain != NULL)
        mAiqCore->amdGroupAnalyze(id, sofInfoMsg, sp, ispGain);
    else
        ret = XCAM_RETURN_BYPASS;
    if (sp) sp->unref(sp);
    if (ispGain) ispGain->unref(ispGain);

    return ret;
}

XCamReturn RkAiqAnalyzeGroupManager::thumbnailsGroupMessageHandler(
    std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id) {
    //XCAM_STATIC_FPS_CALCULATION(THUMBHANDLER, 100);
    rkaiq_image_source_t thumbnailsSrc;
    SmartPtr<XCamMessage> msg            = msgs.front();
    SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg = msg.dynamic_cast_ptr<RkAiqCoreVdBufMsg>();

    if (!vdBufMsg.ptr()) return XCAM_RETURN_BYPASS;

    if (msg->msg_id == XCAM_MESSAGE_ISP_POLL_SP_OK) {
        XCamVideoBuffer* sp = NULL;
        sp                  = convert_to_XCamVideoBuffer(vdBufMsg->msg);

        thumbnailsSrc.frame_id     = msg->frame_id;
        thumbnailsSrc.src_type     = RKISP20_STREAM_SP;
        thumbnailsSrc.image_source = sp;
    } else if (msg->msg_id == XCAM_MESSAGE_NR_IMG_OK) {
        XCamVideoBuffer* nrImg = NULL;
        nrImg                  = convert_to_XCamVideoBuffer(vdBufMsg->msg);

        thumbnailsSrc.frame_id     = msg->frame_id;
        thumbnailsSrc.src_type     = RKISP20_STREAM_NR;
        thumbnailsSrc.image_source = nrImg;
    }

    mAiqCore->thumbnailsGroupAnalyze(thumbnailsSrc);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAnalyzeGroupManager::lscGroupMessageHandler(
    std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id) {
    //XCAM_STATIC_FPS_CALCULATION(LSCHANDLER, 100);
    // 3. determine if all the required messages have been received
    XCamVideoBuffer* tx                  = NULL;
    XCamVideoBuffer* awbProcRes          = NULL;
    XCamVideoBuffer* aeProcRes           = NULL;
    SmartPtr<RkAiqCoreExpMsg> sofInfoMsg = nullptr;

    list<SmartPtr<XCamMessage>>* msgList = &msgs;
    list<SmartPtr<XCamMessage>>::iterator listIt;
    for (listIt = msgList->begin(); listIt != msgList->end();) {
        SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg;
        if ((*listIt)->msg_id == XCAM_MESSAGE_AWB_PROC_RES_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr()) awbProcRes = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_ISP_POLL_TX_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr()) {
                tx = convert_to_XCamVideoBuffer(vdBufMsg->msg);
            }
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_SOF_INFO_OK) {
            sofInfoMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreExpMsg>();
        }
#if 1
        else if ((*listIt)->msg_id == XCAM_MESSAGE_AE_PROC_RES_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr()) {
                aeProcRes = convert_to_XCamVideoBuffer(vdBufMsg->msg);
                RkAiqAlgoProcResAeInt* ae_proc_res =
                    (RkAiqAlgoProcResAeInt*)aeProcRes->map(aeProcRes);
                aeProcRes->unref(aeProcRes);
            }
        }
#endif
        listIt = msgList->erase(listIt);
    }

    // 4. start executing algorithm module flow
    if (awbProcRes != NULL && tx != NULL && sofInfoMsg.ptr())
        mAiqCore->lscGroupAnalyze(id, sofInfoMsg, awbProcRes, tx);
    else
        return XCAM_RETURN_BYPASS;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAnalyzeGroupManager::amfnrGroupMessageHandler(
    std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id) {
    //XCAM_STATIC_FPS_CALCULATION(FMNRHANDLER, 100);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    SmartPtr<RkAiqCoreExpMsg> sofInfoMsg = nullptr;
    XCamVideoBuffer* ispGain             = nullptr;
    XCamVideoBuffer* kgGain              = nullptr;
    XCamVideoBuffer* xCamAmdProcRes      = nullptr;
    RkAiqAlgoProcResAmdInt* amdProcRes   = nullptr;

    list<SmartPtr<XCamMessage>>* msgList = &msgs;
    list<SmartPtr<XCamMessage>>::iterator listIt;
    for (listIt = msgList->begin(); listIt != msgList->end();) {
        SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg;
        if ((*listIt)->msg_id == XCAM_MESSAGE_SOF_INFO_OK) {
            sofInfoMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreExpMsg>();
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_ISP_GAIN_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr()) {
                ispGain = convert_to_XCamVideoBuffer(vdBufMsg->msg);
            }
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_ISPP_GAIN_KG_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr()) {
                kgGain = convert_to_XCamVideoBuffer(vdBufMsg->msg);
            }
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AMD_PROC_RES_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr()) {
                xCamAmdProcRes = convert_to_XCamVideoBuffer(vdBufMsg->msg);
                if (xCamAmdProcRes)
                    amdProcRes = (RkAiqAlgoProcResAmdInt*)xCamAmdProcRes->map(xCamAmdProcRes);
            }
        }

        listIt = msgList->erase(listIt);
    }

    // 4. start executing algorithm module flow
    if (sofInfoMsg.ptr() /* && ispGain != nullptr */ &&
        kgGain != nullptr /* && amdProcRes != nullptr */)
        mAiqCore->mfnrGroupAnalyze(id, sofInfoMsg, ispGain, kgGain,
                                   amdProcRes->amd_proc_res_com.amd_proc_res);
    else
        ret = XCAM_RETURN_BYPASS;

    if (ispGain) ispGain->unref(ispGain);
    if (kgGain) kgGain->unref(kgGain);
    if (xCamAmdProcRes) xCamAmdProcRes->unref(xCamAmdProcRes);

    return ret;
}

XCamReturn RkAiqAnalyzeGroupManager::aynrGroupMessageHandler(
    std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id) {
    //XCAM_STATIC_FPS_CALCULATION(YNRHANDLER, 100);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    SmartPtr<RkAiqCoreExpMsg> sofInfoMsg = nullptr;
    XCamVideoBuffer* ispGain             = nullptr;
    XCamVideoBuffer* wrGain              = nullptr;
    XCamVideoBuffer* xCamAmdProcRes      = nullptr;
    RkAiqAlgoProcResAmdInt* amdProcRes   = nullptr;
    list<SmartPtr<XCamMessage>>* msgList = &msgs;
    list<SmartPtr<XCamMessage>>::iterator listIt;
    for (listIt = msgList->begin(); listIt != msgList->end();) {
        SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg;
        if ((*listIt)->msg_id == XCAM_MESSAGE_SOF_INFO_OK) {
            sofInfoMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreExpMsg>();
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_ISP_GAIN_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr()) {
                ispGain = convert_to_XCamVideoBuffer(vdBufMsg->msg);
            }
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_ISPP_GAIN_WR_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr()) {
                wrGain = convert_to_XCamVideoBuffer(vdBufMsg->msg);
            }
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AMD_PROC_RES_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr()) {
                xCamAmdProcRes = convert_to_XCamVideoBuffer(vdBufMsg->msg);
                if (xCamAmdProcRes)
                    amdProcRes = (RkAiqAlgoProcResAmdInt*)xCamAmdProcRes->map(xCamAmdProcRes);
            }
        }

        listIt = msgList->erase(listIt);
    }

    // 4. start executing algorithm module flow
    if (sofInfoMsg.ptr() &&
        /* ispGain != nullptr && */ wrGain != nullptr /* && amdProcRes != nullptr */)
        mAiqCore->ynrGroupAnalyze(id, sofInfoMsg, ispGain, wrGain,
                                  amdProcRes->amd_proc_res_com.amd_proc_res);
    else
        ret = XCAM_RETURN_BYPASS;

    if (ispGain) ispGain->unref(ispGain);
    if (wrGain) wrGain->unref(wrGain);
    if (xCamAmdProcRes) xCamAmdProcRes->unref(xCamAmdProcRes);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAnalyzeGroupManager::grp0MessageHandler(
    std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id) {
    //XCAM_STATIC_FPS_CALCULATION(GRP0HANDLER, 100);
    // 3. determine if all the required messages have been received
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    grp0AnalyzerInParams_t inParams = {
        .sofInfoMsg             = nullptr,
        .aecStatsBuf            = nullptr,
        .awbStatsBuf            = nullptr,
        .afStatsBuf             = nullptr,
        .aePreRes               = nullptr,
        .aeProcRes              = nullptr,
    };

    list<SmartPtr<XCamMessage>>* msgList = &msgs;
    list<SmartPtr<XCamMessage>>::iterator listIt;
    for (listIt = msgList->begin(); listIt != msgList->end();) {
        SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg;
        if ((*listIt)->msg_id == XCAM_MESSAGE_SOF_INFO_OK) {
            inParams.sofInfoMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreExpMsg>();
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AE_PRE_RES_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr())
                inParams.aePreRes = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AE_PROC_RES_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr())
                inParams.aeProcRes = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AEC_STATS_OK) {
            SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr())
                inParams.aecStatsBuf = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AWB_STATS_OK) {
            SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr())
                inParams.awbStatsBuf = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AF_STATS_OK) {
            SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr())
                inParams.afStatsBuf = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        }

        listIt = msgList->erase(listIt);
    }

    inParams.id = id;

    // 4. start executing algorithm module flow
    if (inParams.sofInfoMsg.ptr() && inParams.aePreRes && inParams.aeProcRes && \
        inParams.aecStatsBuf && inParams.awbStatsBuf)
        mAiqCore->grp0Analyze(inParams);
    else
        ret = XCAM_RETURN_BYPASS;

    if (inParams.aePreRes)
        inParams.aePreRes->unref(inParams.aePreRes);
    if (inParams.aeProcRes)
        inParams.aeProcRes->unref(inParams.aeProcRes);
    if (inParams.aecStatsBuf)
        inParams.aecStatsBuf->unref(inParams.aecStatsBuf);
    if (inParams.awbStatsBuf)
        inParams.awbStatsBuf->unref(inParams.awbStatsBuf);
    if (inParams.afStatsBuf)
        inParams.afStatsBuf->unref(inParams.afStatsBuf);

    return ret;
}

XCamReturn RkAiqAnalyzeGroupManager::grp1MessageHandler(
    std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id) {
    //XCAM_STATIC_FPS_CALCULATION(GRP1HANDLER, 100);
    // 3. determine if all the required messages have been received
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    grp1AnalyzerInParams_t inParams = {
        .sofInfoMsg             = nullptr,
        .aecStatsBuf            = nullptr,
        .awbStatsBuf            = nullptr,
        .afStatsBuf             = nullptr,
        .aePreRes               = nullptr,
        .awbProcRes             = nullptr,
    };

    list<SmartPtr<XCamMessage>>* msgList = &msgs;
    list<SmartPtr<XCamMessage>>::iterator listIt;
    for (listIt = msgList->begin(); listIt != msgList->end();) {
        SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg;
        if ((*listIt)->msg_id == XCAM_MESSAGE_SOF_INFO_OK) {
            inParams.sofInfoMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreExpMsg>();
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AE_PRE_RES_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr())
                inParams.aePreRes = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AWB_PROC_RES_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr())
                inParams.awbProcRes = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AEC_STATS_OK) {
            SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr())
                inParams.aecStatsBuf = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AWB_STATS_OK) {
            SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr())
                inParams.awbStatsBuf = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AF_STATS_OK) {
            SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr())
                inParams.afStatsBuf = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        }

        listIt = msgList->erase(listIt);
    }

    inParams.id = id;

    // 4. start executing algorithm module flow
    if (inParams.sofInfoMsg.ptr() && inParams.awbProcRes)
        mAiqCore->grp1Analyze(inParams);
    else
        ret = XCAM_RETURN_BYPASS;

    if (inParams.aePreRes)
        inParams.aePreRes->unref(inParams.aePreRes);
    if (inParams.awbProcRes)
        inParams.awbProcRes->unref(inParams.awbProcRes);
    if (inParams.aecStatsBuf)
        inParams.aecStatsBuf->unref(inParams.aecStatsBuf);
    if (inParams.awbStatsBuf)
        inParams.awbStatsBuf->unref(inParams.awbStatsBuf);
    if (inParams.afStatsBuf)
        inParams.afStatsBuf->unref(inParams.afStatsBuf);

    return ret;
}

XCamReturn RkAiqAnalyzeGroupManager::afMessageHandler(
    std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id) {
    //XCAM_STATIC_FPS_CALCULATION(AFHANDLER, 100);
    // 3. determine if all the required messages have been received
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    afAnalyzerInParams_t inParams = {
        .expInfo                  = nullptr,
        .aecStatsBuf              = nullptr,
        .afStatsBuf               = nullptr,
        .aePreRes                 = nullptr,
        .aeProcRes                = nullptr,
    };

    list<SmartPtr<XCamMessage>>* msgList = &msgs;
    list<SmartPtr<XCamMessage>>::iterator listIt;
    for (listIt = msgList->begin(); listIt != msgList->end();) {
        SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg;
        if ((*listIt)->msg_id == XCAM_MESSAGE_SOF_INFO_OK) {
            inParams.expInfo = (*listIt).dynamic_cast_ptr<RkAiqCoreExpMsg>();
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AE_PRE_RES_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr())
                inParams.aePreRes = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AE_PROC_RES_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr())
                inParams.aeProcRes = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AEC_STATS_OK) {
            SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr())
                inParams.aecStatsBuf = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_AF_STATS_OK) {
            SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr())
                inParams.afStatsBuf = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        }

        listIt = msgList->erase(listIt);
    }

    inParams.id = id;

    // 4. start executing algorithm module flow
    if (inParams.expInfo.ptr() && inParams.aecStatsBuf && inParams.afStatsBuf && \
        inParams.aePreRes && inParams.aeProcRes)
        mAiqCore->afAnalyze(inParams);
    else
        ret = XCAM_RETURN_BYPASS;

    if (inParams.aePreRes)
        inParams.aePreRes->unref(inParams.aePreRes);
    if (inParams.aeProcRes)
        inParams.aeProcRes->unref(inParams.aeProcRes);
    if (inParams.aecStatsBuf)
        inParams.aecStatsBuf->unref(inParams.aecStatsBuf);
    if (inParams.afStatsBuf)
        inParams.afStatsBuf->unref(inParams.afStatsBuf);

    return ret;
}



XCamReturn RkAiqAnalyzeGroupManager::eisGroupMessageHandler(
    std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id) {
    //XCAM_STATIC_FPS_CALCULATION(EISHANDLER, 100);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqCoreExpMsg> sofInfoMsg = nullptr;
    XCamVideoBuffer* orbStats = nullptr;
    XCamVideoBuffer* nrImg               = nullptr;

    list<SmartPtr<XCamMessage>>* msgList = &msgs;
    list<SmartPtr<XCamMessage>>::iterator listIt;
    for (listIt = msgList->begin(); listIt != msgList->end();) {
        SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg;
        if ((*listIt)->msg_id == XCAM_MESSAGE_SOF_INFO_OK) {
            sofInfoMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreExpMsg>();
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_ORB_STATS_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr()) orbStats = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        } else if ((*listIt)->msg_id == XCAM_MESSAGE_NR_IMG_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr()) nrImg = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        }

        listIt = msgList->erase(listIt);
    }

    if (sofInfoMsg.ptr() && orbStats)
        ret = mAiqCore->eisGroupAnalyze(id, sofInfoMsg, orbStats, nrImg);
    else
        ret = XCAM_RETURN_BYPASS;

    orbStats->unref(orbStats);
    nrImg->unref(nrImg);

    return ret;
}

XCamReturn RkAiqAnalyzeGroupManager::orbGroupMessageHandler(
    std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id) {
    //XCAM_STATIC_FPS_CALCULATION(ORBHANDLER, 100);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    XCamVideoBuffer* orbStats = nullptr;

    list<SmartPtr<XCamMessage>>* msgList = &msgs;
    list<SmartPtr<XCamMessage>>::iterator listIt;
    for (listIt = msgList->begin(); listIt != msgList->end();) {
        SmartPtr<RkAiqCoreVdBufMsg> vdBufMsg;
        if ((*listIt)->msg_id == XCAM_MESSAGE_ORB_STATS_OK) {
            vdBufMsg = (*listIt).dynamic_cast_ptr<RkAiqCoreVdBufMsg>();
            if (vdBufMsg.ptr()) orbStats = convert_to_XCamVideoBuffer(vdBufMsg->msg);
        }

        listIt = msgList->erase(listIt);
    }

    if (orbStats)
        ret = mAiqCore->orbGroupAnalyze(id, orbStats);
    else
        ret = XCAM_RETURN_BYPASS;

    orbStats->unref(orbStats);

    return ret;
}

void RkAiqAnalyzeGroupManager::parseAlgoGroup(const struct RkAiqAlgoDesCommExt* algoDes) {
    // clang-format off
    std::map<rk_aiq_core_analyze_type_e, MessageHandleWrapper> concreteHandlerMap = {
        { RK_AIQ_CORE_ANALYZE_MEAS,
          std::bind(&RkAiqAnalyzeGroupManager::measGroupMessageHandler, this, std::placeholders::_1, std::placeholders::_2)},
        { RK_AIQ_CORE_ANALYZE_OTHER,
          std::bind(&RkAiqAnalyzeGroupManager::otherGroupMessageHandler, this, std::placeholders::_1, std::placeholders::_2)},
        { RK_AIQ_CORE_ANALYZE_AMD,
          std::bind(&RkAiqAnalyzeGroupManager::amdGroupMessageHandler, this, std::placeholders::_1, std::placeholders::_2)},
        { RK_AIQ_CORE_ANALYZE_THUMBNAILS,
          std::bind(&RkAiqAnalyzeGroupManager::thumbnailsGroupMessageHandler, this, std::placeholders::_1, std::placeholders::_2)},
        { RK_AIQ_CORE_ANALYZE_LSC,
          std::bind(&RkAiqAnalyzeGroupManager::lscGroupMessageHandler, this, std::placeholders::_1, std::placeholders::_2)},
        { RK_AIQ_CORE_ANALYZE_AE,
          std::bind(&RkAiqAnalyzeGroupManager::aeGroupMessageHandler, this, std::placeholders::_1, std::placeholders::_2)},
        { RK_AIQ_CORE_ANALYZE_AMFNR,
          std::bind(&RkAiqAnalyzeGroupManager::amfnrGroupMessageHandler, this, std::placeholders::_1, std::placeholders::_2)},
        { RK_AIQ_CORE_ANALYZE_AYNR,
          std::bind(&RkAiqAnalyzeGroupManager::aynrGroupMessageHandler, this, std::placeholders::_1, std::placeholders::_2)},
        { RK_AIQ_CORE_ANALYZE_AWB,
          std::bind(&RkAiqAnalyzeGroupManager::awbGroupMessageHandler, this, std::placeholders::_1, std::placeholders::_2)},
        { RK_AIQ_CORE_ANALYZE_GRP0,
          std::bind(&RkAiqAnalyzeGroupManager::grp0MessageHandler, this, std::placeholders::_1, std::placeholders::_2)},
        { RK_AIQ_CORE_ANALYZE_GRP1,
          std::bind(&RkAiqAnalyzeGroupManager::grp1MessageHandler, this, std::placeholders::_1, std::placeholders::_2)},
        { RK_AIQ_CORE_ANALYZE_AF,
          std::bind(&RkAiqAnalyzeGroupManager::afMessageHandler, this, std::placeholders::_1, std::placeholders::_2)},
        { RK_AIQ_CORE_ANALYZE_EIS,
          std::bind(&RkAiqAnalyzeGroupManager::eisGroupMessageHandler, this, std::placeholders::_1, std::placeholders::_2)},
        { RK_AIQ_CORE_ANALYZE_ORB,
          std::bind(&RkAiqAnalyzeGroupManager::orbGroupMessageHandler, this, std::placeholders::_1, std::placeholders::_2)},
    };

    // clang-format on
    uint64_t enAlgosMask = mAiqCore->getCustomEnAlgosMask();
    for (size_t i = 0; algoDes[i].des != NULL; i++) {
        int algo_type = algoDes[i].des->type;
        if (!(1 << algo_type & enAlgosMask))
            continue;
        int deps_flag = 0;
        for (size_t j = 0; j < algoDes[i].grpConds.size; j++)
            deps_flag |= 1 << algoDes[i].grpConds.conds[j].cond;
        rk_aiq_core_analyze_type_e group = algoDes[i].group;
        mGroupAlgoListMap[group].push_back(algo_type);
        mGroupAlgoListMap[RK_AIQ_CORE_ANALYZE_ALL].push_back(algo_type);
        if (mGroupMap.count(deps_flag)) {
            continue;
        }
        mGroupMap[deps_flag] = new RkAiqAnalyzerGroup(mAiqCore, group, deps_flag, &algoDes[i].grpConds);
        mGroupMap[deps_flag]->setConcreteHandler(concreteHandlerMap[group]);
        LOGD_ANALYZER_SUBM(ANALYZER_SUBM, "Created group %" PRIx64 " for dep flags %" PRIx64"", deps_flag, group);
    }
}

XCamReturn RkAiqAnalyzeGroupManager::handleMessage(const SmartPtr<XCamMessage> &msg) {
    //XCAM_STATIC_FPS_CALCULATION(HANDLEMSG, 100);
    for (auto& it : mGroupMap) {
        if ((it.first & (1 << msg->msg_id)) != 0) {
            LOGD_ANALYZER_SUBM(ANALYZER_SUBM,
                "Handle message(%s) id[%d] on group(%s), flags %" PRIx64 "",
                MessageType2Str[msg->msg_id], msg->frame_id,
                AnalyzerGroupType2Str[it.second->getType()],
                it.second->getDepsFlag());

            it.second->pushMsg(msg);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

};  // namespace RkCam
