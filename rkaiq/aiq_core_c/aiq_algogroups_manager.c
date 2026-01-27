/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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

#include "aiq_algogroups_manager.h"
#include "aiq_core_c/aiq_core.h"
#include "aiq_core_c/aiq_algo_handler.h"
#if RKAIQ_HAVE_DUMPSYS
#include "info/aiq_groupAnalyzerInfo.h"
#endif

#define GROUP_MSG_CNT_MAX     5
#define MSGHDL_MSGQ_MAX       100
#define ANALYZER_SUBM (0x1)

static void clearAiqCoreMsg(AiqCoreMsg_t* vdBufMsg, RkAiqAlgosGroupShared_t* shared) {
    switch (vdBufMsg->msg_id) {
        case XCAM_MESSAGE_SOF_INFO_OK: {
            AlgoRstShared_t* pRst        = *(AlgoRstShared_t**)(vdBufMsg->buf);
            AiqSofInfoWrapper_t* sofInfo = (AiqSofInfoWrapper_t*)pRst->_data;
            AIQ_REF_BASE_UNREF(&sofInfo->curExp->_base._ref_base);
            AIQ_REF_BASE_UNREF(&sofInfo->preExp->_base._ref_base);
            AIQ_REF_BASE_UNREF(&sofInfo->nxtExp->_base._ref_base);
            AIQ_REF_BASE_UNREF(&pRst->_ref_base);
        } break;
        case XCAM_MESSAGE_AEC_STATS_OK: {
			if (shared) shared->aecStatsBuf = NULL;
            aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
            if (pRst) {
                AIQ_REF_BASE_UNREF(&pRst->_ref_base);
            }
        } break;
        case XCAM_MESSAGE_AWB_STATS_OK: {
			if (shared) shared->awbStatsBuf = NULL;
            aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
            if (pRst) {
                AIQ_REF_BASE_UNREF(&pRst->_ref_base);
            }
        } break;
        case XCAM_MESSAGE_AF_STATS_OK: {
			if (shared) shared->awbStatsBuf = NULL;
            aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
            if (pRst) {
                AIQ_REF_BASE_UNREF(&pRst->_ref_base);
            }
        } break;
        case XCAM_MESSAGE_PDAF_STATS_OK: {
			if (shared) shared->pdafStatsBuf = NULL;
            aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
            if (pRst) {
                AIQ_REF_BASE_UNREF(&pRst->_ref_base);
            }
        } break;
        case XCAM_MESSAGE_ADEHAZE_STATS_OK: {
			if (shared) shared->adehazeStatsBuf = NULL;
            aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
            if (pRst) {
                AIQ_REF_BASE_UNREF(&pRst->_ref_base);
            }
        } break;
        case XCAM_MESSAGE_AGAIN_STATS_OK: {
			if (shared) shared->againStatsBuf = NULL;
            aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
            if (pRst) {
                AIQ_REF_BASE_UNREF(&pRst->_ref_base);
            }
        } break;
        case XCAM_MESSAGE_AE_PRE_RES_OK: {
			if (shared) shared->res_comb.ae_pre_res_c = NULL;
            aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
            if (pRst) {
                AIQ_REF_BASE_UNREF(&pRst->_ref_base);
            }
        } break;
        case XCAM_MESSAGE_AE_PROC_RES_OK: {
			if (shared) shared->res_comb.ae_proc_res_c = NULL;
            aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
            if (pRst) {
                AIQ_REF_BASE_UNREF(&pRst->_ref_base);
            }
        } break;
        case XCAM_MESSAGE_AWB_PROC_RES_OK: {
			if (shared) shared->res_comb.awb_proc_res_c = NULL;
            aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
            if (pRst) {
                AIQ_REF_BASE_UNREF(&pRst->_ref_base);
            }
        } break;
        case XCAM_MESSAGE_BLC_PROC_RES_OK: {
			if (shared) shared->res_comb.blc_proc_res = NULL;
            aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
            if (pRst) {
                AIQ_REF_BASE_UNREF(&pRst->_ref_base);
            }
        } break;
        case XCAM_MESSAGE_YNR_PROC_RES_OK: {
			if (shared) shared->res_comb.ynr_proc_res = NULL;
            aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
            if (pRst) {
                AIQ_REF_BASE_UNREF(&pRst->_ref_base);
            }
        } break;
        case XCAM_MESSAGE_ISP_POLL_SP_OK: {
			if (shared) shared->sp = NULL;
            XCamVideoBuffer* buf = *(XCamVideoBuffer**)(vdBufMsg->buf);
            if (buf) AiqVideoBuffer_unref(buf);
		} break;
        case XCAM_MESSAGE_ISP_GAIN_OK: {
			if (shared) shared->ispGain = NULL;
            XCamVideoBuffer* buf = *(XCamVideoBuffer**)(vdBufMsg->buf);
            if (buf) AiqVideoBuffer_unref(buf);
	    } break;
        case XCAM_MESSAGE_ISP_POLL_TX_OK: {
			if (shared) shared->tx = NULL;
            XCamVideoBuffer* buf = *(XCamVideoBuffer**)(vdBufMsg->buf);
            if (buf) AiqVideoBuffer_unref(buf);
        } break;
        case XCAM_MESSAGE_VICAP_POLL_SCL_OK: {
            rk_aiq_scale_raw_info_t* buf_info = (rk_aiq_scale_raw_info_t*)(vdBufMsg->buf);
            if (buf_info->raw_s) {
                AiqVideoBuffer_unref(buf_info->raw_s);
				if (shared) shared->scaleRawInfo.raw_s = NULL;
            }
            if (buf_info->raw_l) {
                AiqVideoBuffer_unref(buf_info->raw_l);
				if (shared) shared->scaleRawInfo.raw_l = NULL;
            }
            if (buf_info->raw_m) {
                AiqVideoBuffer_unref(buf_info->raw_m);
				if (shared) shared->scaleRawInfo.raw_m = NULL;
            }
            break;
        }
        default:
            break;
    }
}

static void deepCpyAiqCoreMsg(AiqCoreMsg_t* vdBufMsg) {
    switch (vdBufMsg->msg_id) {
        case XCAM_MESSAGE_SOF_INFO_OK: {
            AlgoRstShared_t* pRst        = *(AlgoRstShared_t**)(vdBufMsg->buf);
            AiqSofInfoWrapper_t* sofInfo = (AiqSofInfoWrapper_t*)pRst->_data;
            AIQ_REF_BASE_REF(&sofInfo->curExp->_base._ref_base);
            AIQ_REF_BASE_REF(&sofInfo->preExp->_base._ref_base);
            AIQ_REF_BASE_REF(&sofInfo->nxtExp->_base._ref_base);
            AIQ_REF_BASE_REF(&pRst->_ref_base);
        } break;
        case XCAM_MESSAGE_AEC_STATS_OK:
        case XCAM_MESSAGE_AWB_STATS_OK:
        case XCAM_MESSAGE_AF_STATS_OK:
        case XCAM_MESSAGE_PDAF_STATS_OK:
        case XCAM_MESSAGE_ADEHAZE_STATS_OK:
        case XCAM_MESSAGE_AGAIN_STATS_OK:
        case XCAM_MESSAGE_AE_PRE_RES_OK:
        case XCAM_MESSAGE_AE_PROC_RES_OK:
        case XCAM_MESSAGE_AWB_PROC_RES_OK:
        case XCAM_MESSAGE_BLC_PROC_RES_OK:
        case XCAM_MESSAGE_YNR_PROC_RES_OK: {
            aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
            if (pRst) {
                AIQ_REF_BASE_REF(&pRst->_ref_base);
            }
        } break;
        case XCAM_MESSAGE_ISP_POLL_SP_OK:
        case XCAM_MESSAGE_ISP_GAIN_OK:
        case XCAM_MESSAGE_ISP_POLL_TX_OK: {
            XCamVideoBuffer* buf = *(XCamVideoBuffer**)(vdBufMsg->buf);
            if (buf) AiqVideoBuffer_ref(buf);
        } break;
        case XCAM_MESSAGE_VICAP_POLL_SCL_OK: {
            rk_aiq_scale_raw_info_t* buf_info = (rk_aiq_scale_raw_info_t*)(vdBufMsg->buf);
            if (buf_info->raw_s) {
                AiqVideoBuffer_ref(buf_info->raw_s);
            }
            if (buf_info->raw_l) {
                AiqVideoBuffer_ref(buf_info->raw_l);
            }
            if (buf_info->raw_m) {
                AiqVideoBuffer_ref(buf_info->raw_m);
            }
            break;
        }
        default:
            break;
    }
}

static void msgReduction(AiqAnalyzerGroup_t* pGroup, AiqMap_t* msgMap) {
    if (aiqMap_size(msgMap) > 0) {
        const int originalSize = aiqMap_size(msgMap);
        int numToErase    = originalSize - pGroup->mGrpMsgOverflowCnt;
        if (numToErase > 0) {
            AiqMapItem_t* pItem  = aiqMap_begin(msgMap);
            GroupMessage_t* pMsg = (GroupMessage_t*)(pItem->_pData);
            int32_t unreadyFlag  = pGroup->mDepsFlag & ~pMsg->msg_flags;
            // print missing params
            char missing_conds[512] = "\0";
            uint32_t i              = 0;
            while (unreadyFlag) {
                if (unreadyFlag % 2) {
                    strcat(missing_conds, MessageType2Str[i]);
                    strcat(missing_conds, ",");
                }
                unreadyFlag >>= 1;
                i++;
            }
            LOGW_ANALYZER_SUBM(ANALYZER_SUBM,
                               "camId: %d, group(%s): id[%d] map size is %d, erase %d, element, "
                               "missing conditions: %s",
                               pGroup->mAiqCore->mAlogsComSharedParams.mCamPhyId,
                               AnalyzerGroupType2Str[pGroup->mGroupType],
                               (uint32_t)(long)pItem->_key, originalSize, numToErase,
                               missing_conds);

            bool rm = false;
            AIQ_MAP_FOREACH(msgMap, pItem, rm) {
                pMsg = (GroupMessage_t*)(pItem->_pData);
                for (int i = 0; i < pMsg->msg_cnts; i++) {
                    clearAiqCoreMsg(&pMsg->msgList[i], pGroup->mAiqCore->mAlogsGroupSharedParamsMap[pGroup->mGroupType]);
                }
                pMsg->msg_cnts = 0;
                pItem          = aiqMap_erase_locked(msgMap, pItem->_key);
                rm             = true;
                if (--numToErase <= 0) break;
            }
#if RKAIQ_HAVE_DUMPSYS
            pGroup->mMsgReduceCnt++;
#endif
        }
    }
}

bool AiqAnalyzerGroup_pushMsg(AiqAnalyzerGroup_t* pGroup, AiqCoreMsg_t* msg) {
    if (pGroup->mRkAiqGroupMsgHdlTh) {
        AiqAnalyzeGroupMsgHdlThread_push_msg(pGroup->mRkAiqGroupMsgHdlTh, msg);
    }
    return true;
}

static int8_t getMsgDelayCnt(AiqAnalyzerGroup_t* pGroup, enum XCamMessageType msg_id) {
    uint32_t i      = 0;
    int8_t delayCnt = 0;
    for (i = 0; i < pGroup->mGrpConds.size; i++) {
        if (pGroup->mGrpConds.conds[i].cond == msg_id) {
            delayCnt = pGroup->mGrpConds.conds[i].delay;
            if (delayCnt != 0 && pGroup->mUserSetDelayCnts != INT8_MAX)
                delayCnt = pGroup->mUserSetDelayCnts;
            return delayCnt;
        }
    }

    if (i == pGroup->mGrpConds.size)
        LOGE_ANALYZER_SUBM(ANALYZER_SUBM, "don't match msgId(0x%x) in mGrpConds", msg_id);

    return 0;
}

void AiqAnalyzerGroup_setDelayCnts(AiqAnalyzerGroup_t* pGroup, int8_t delayCnts) {
    pGroup->mUserSetDelayCnts = delayCnts;
}

void AiqAnalyzerGroup_setVicapScaleFlag(AiqAnalyzerGroup_t* pGroup, bool mode) {
    pGroup->mVicapScaleStart = mode;
}

void AiqAnalyzerGroup_setDepsFlagAndClearMap(AiqAnalyzerGroup_t* pGroup, uint64_t new_deps) {
    pGroup->mDepsFlag = new_deps;

    AiqMapItem_t* pItem = NULL;
    bool rm             = false;
    AIQ_MAP_FOREACH(pGroup->mGroupMsgMap, pItem, rm) {
        GroupMessage_t* pMsg = (GroupMessage_t*)(pItem->_pData);
        for (int i = 0; i < pMsg->msg_cnts; i++) {
            clearAiqCoreMsg(&pMsg->msgList[i], pGroup->mAiqCore->mAlogsGroupSharedParamsMap[pGroup->mGroupType]);
        }
        pMsg->msg_cnts = 0;
        pItem          = aiqMap_erase_locked(pGroup->mGroupMsgMap, pItem->_key);
        rm             = true;
    }
}

XCamReturn AiqAnalyzerGroup_init(AiqAnalyzerGroup_t* pGroup, AiqCore_t* aiqCore,
                                 enum rk_aiq_core_analyze_type_e type, const uint64_t flag,
                                 const RkAiqGrpConditions_t* grpConds, const bool singleThrd) {
    ENTER_ANALYZER_FUNCTION();
    pGroup->mAiqCore           = aiqCore;
    pGroup->mGroupType         = type;
    pGroup->mDepsFlag          = flag;
    pGroup->mUserSetDelayCnts  = INT8_MAX;
    pGroup->mAwakenId          = (uint32_t)-1;
    pGroup->mGrpMsgOverflowCnt = GROUP_MSG_OVERFLOW_TH;
#if RKAIQ_HAVE_DUMPSYS
    pGroup->mMsgReduceCnt = 0;
#endif
    if (grpConds) {
        pGroup->mGrpConds.conds = (RkAiqGrpCondition_t*)aiq_mallocz(sizeof(RkAiqGrpCondition_t) * grpConds->size);
        memcpy(pGroup->mGrpConds.conds, grpConds->conds, sizeof(RkAiqGrpCondition_t) * grpConds->size);
        pGroup->mGrpConds.size = grpConds->size;
    }
    if (!singleThrd) {
        char name[64];
        sprintf(name, "g-%x:0x%" PRIx64, pGroup->mGroupType, pGroup->mDepsFlag);
        pGroup->mRkAiqGroupMsgHdlTh =
            (AiqAnalyzeGroupMsgHdlThread_t*)aiq_mallocz(sizeof(AiqAnalyzeGroupMsgHdlThread_t));
        AiqAnalyzeGroupMsgHdlThread_init(pGroup->mRkAiqGroupMsgHdlTh, name, pGroup);
    } else {
        pGroup->mRkAiqGroupMsgHdlTh = NULL;
    }

    AiqMapConfig_t grpMsgCfg;
    grpMsgCfg._name        = "grpMsg";
	grpMsgCfg._key_type	   = AIQ_MAP_KEY_TYPE_UINT32;
    grpMsgCfg._item_nums   = GROUP_MSG_CNT_MAX;
    grpMsgCfg._item_size   = sizeof(GroupMessage_t);
    pGroup->mGroupMsgMap = aiqMap_init(&grpMsgCfg);
    if (!pGroup->mGroupMsgMap) LOGE_ANALYZER("init %s error", grpMsgCfg._name);
    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

void AiqAnalyzerGroup_deinit(AiqAnalyzerGroup_t* pGroup) {
    ENTER_ANALYZER_FUNCTION();
    if (pGroup->mRkAiqGroupMsgHdlTh) {
        AiqAnalyzeGroupMsgHdlThread_deinit(pGroup->mRkAiqGroupMsgHdlTh);
        aiq_free(pGroup->mRkAiqGroupMsgHdlTh);
        pGroup->mRkAiqGroupMsgHdlTh = NULL;
    }
    if (pGroup->mGroupMsgMap) {
        aiqMap_deinit(pGroup->mGroupMsgMap);
        pGroup->mGroupMsgMap = NULL;
    }
    if (pGroup->mGrpConds.conds) {
        aiq_free(pGroup->mGrpConds.conds);
        pGroup->mGrpConds.conds = NULL;
    }
    EXIT_ANALYZER_FUNCTION();
}

XCamReturn AiqAnalyzerGroup_start(AiqAnalyzerGroup_t* pGroup) {
    ENTER_ANALYZER_FUNCTION();
    if (pGroup->mRkAiqGroupMsgHdlTh) AiqAnalyzeGroupMsgHdlThread_start(pGroup->mRkAiqGroupMsgHdlTh);
    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqAnalyzerGroup_stop(AiqAnalyzerGroup_t* pGroup) {
    ENTER_ANALYZER_FUNCTION();
    if (pGroup->mRkAiqGroupMsgHdlTh) AiqAnalyzeGroupMsgHdlThread_stop(pGroup->mRkAiqGroupMsgHdlTh);

#if RKAIQ_HAVE_DUMPSYS
    pGroup->mMsgReduceCnt = 0;
#endif

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

bool AiqAnalyzerGroup_msgHandle(AiqAnalyzerGroup_t* pGroup, AiqCoreMsg_t* msg) {
    if (!((1ULL << msg->msg_id) & pGroup->mDepsFlag)) {
        return true;
    }

    enum XCamMessageType msg_id = msg->msg_id;
    uint32_t delayCnt      = getMsgDelayCnt(pGroup, msg_id);
    if (msg->frame_id == pGroup->mAwakenId) delayCnt = 0;
    uint32_t userId = msg->frame_id + delayCnt;

    GroupMessage_t msgWrapper;
    aiq_memset(&msgWrapper, 0, sizeof(msgWrapper));
    GroupMessage_t* pMsgWrapper = NULL;
    bool bInsert              = false;
    AiqMapItem_t* pItem       = aiqMap_get(pGroup->mGroupMsgMap, (void*)(intptr_t)userId);

    if (pItem) {
        pMsgWrapper = (GroupMessage_t*)(pItem->_pData);
        bInsert     = false;
    } else {
        bInsert     = true;
        pMsgWrapper = &msgWrapper;
    }

    pMsgWrapper->msg_flags |= 1ULL << msg->msg_id;
    if (pMsgWrapper->msg_cnts >= MAX_MESSAGES) {
        LOGE_ANALYZER_SUBM(ANALYZER_SUBM,
                           "camId: %d, group(%s): id[%d] push msg(%s), msg_cnts: %d overflow",
                           pGroup->mAiqCore->mAlogsComSharedParams.mCamPhyId,
                           AnalyzerGroupType2Str[pGroup->mGroupType], msg->frame_id,
                           MessageType2Str[msg->msg_id], pMsgWrapper->msg_cnts);
        for (int i = 0; i < pMsgWrapper->msg_cnts; i++) {
            LOGE_ANALYZER_SUBM(ANALYZER_SUBM, "%d: fid:%d, msg:%s", i,
                               pMsgWrapper->msgList[i].frame_id,
                               MessageType2Str[msgWrapper.msgList[i].msg_id]);
        }

        return true;
    }

    pMsgWrapper->msgList[pMsgWrapper->msg_cnts++] = *msg;
	deepCpyAiqCoreMsg(msg);

	LOGD_ANALYZER_SUBM(
		ANALYZER_SUBM,
		"camId: %d, group(%s): id[%d] push msg(%s), msg delayCnt(%d), map size is %d",
		pGroup->mAiqCore->mAlogsComSharedParams.mCamPhyId,
		AnalyzerGroupType2Str[pGroup->mGroupType], msg->frame_id, MessageType2Str[msg->msg_id],
		delayCnt, aiqMap_size(pGroup->mGroupMsgMap));

    uint64_t msg_flags = pMsgWrapper->msg_flags;
    if (!(msg_flags ^ pGroup->mDepsFlag)) {
        pGroup->mHandler(pGroup->mAiqCore, &pMsgWrapper->msgList[0], pMsgWrapper->msg_cnts, userId,
                         pGroup->mGroupType);

        AiqMapItem_t* pItem = NULL;
        bool rm             = false;
        AIQ_MAP_FOREACH(pGroup->mGroupMsgMap, pItem, rm) {
            if ((uint32_t)(long)pItem->_key <= userId) {
                GroupMessage_t* pMsg = (GroupMessage_t*)(pItem->_pData);
                for (int i = 0; i < pMsg->msg_cnts; i++) {
                    clearAiqCoreMsg(&pMsg->msgList[i], pGroup->mAiqCore->mAlogsGroupSharedParamsMap[pGroup->mGroupType]);
                }
                pMsg->msg_cnts = 0;
                pItem          = aiqMap_erase_locked(pGroup->mGroupMsgMap, pItem->_key);
                rm             = true;
            } else
                break;
        }
		LOGD_ANALYZER("%s, group %s erase frame(%d) msg map\n", __FUNCTION__,
					AnalyzerGroupType2Str[pGroup->mGroupType], userId);
		if (bInsert) {
			for (int i = 0; i < pMsgWrapper->msg_cnts; i++) {
				clearAiqCoreMsg(&pMsgWrapper->msgList[i], pGroup->mAiqCore->mAlogsGroupSharedParamsMap[pGroup->mGroupType]);
			}

		}
    } else {
        if (bInsert) aiqMap_insert(pGroup->mGroupMsgMap, (void*)(intptr_t)userId, pMsgWrapper);
        msgReduction(pGroup, pGroup->mGroupMsgMap);
        return true;
    }

    return true;
}

static XCamReturn handleCalibUpdate(AiqAnalyzerGroup_t* grp) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (grp && grp->mAiqCore) {
        AiqCore_updateCalib(grp->mAiqCore, grp->mGroupType);
    }

    return ret;
}

static bool grpMsgHdlTh_func(void* user_data) {
    AiqAnalyzeGroupMsgHdlThread_t* pHdlTh = (AiqAnalyzeGroupMsgHdlThread_t*)user_data;

    aiqMutex_lock(&pHdlTh->_mutex);
    while (!pHdlTh->bQuit && aiqList_size(pHdlTh->mMsgsQueue) <= 0) {
        aiqCond_wait(&pHdlTh->_cond, &pHdlTh->_mutex);
    }
    if (pHdlTh->bQuit) {
        aiqMutex_unlock(&pHdlTh->_mutex);
        LOGW_ANALYZER_SUBM(ANALYZER_SUBM, "quit grpMsgTh !");
        return false;
    }
    aiqMutex_unlock(&pHdlTh->_mutex);

    AiqCoreMsg_t msg;
    int ret = aiqList_get(pHdlTh->mMsgsQueue, &msg);
    if (ret) {
        // ignore error
        return true;
    }
    // XCAM_STATIC_FPS_CALCULATION(GROUPMSGTH, 100);

    AiqListItem_t* pItem = NULL;
    bool rm              = false;
    AIQ_LIST_FOREACH(pHdlTh->mHandlerGroups, pItem, rm) {
        AiqAnalyzerGroup_t* pGrp = *(AiqAnalyzerGroup_t**)pItem->_pData;
        handleCalibUpdate(pGrp);
        AiqAnalyzerGroup_msgHandle(pGrp, &msg);
    }

	clearAiqCoreMsg(&msg, NULL);

    return true;
}

XCamReturn AiqAnalyzeGroupMsgHdlThread_init(AiqAnalyzeGroupMsgHdlThread_t* pHdlTh, char* name,
                                            AiqAnalyzerGroup_t* group) {
    aiqMutex_init(&pHdlTh->_mutex);
    aiqCond_init(&pHdlTh->_cond);
    AiqListConfig_t hdlGropuCfg;
    hdlGropuCfg._name      = "hdlGrp";
    hdlGropuCfg._item_nums = RK_AIQ_CORE_ANALYZE_MAX;
    hdlGropuCfg._item_size = sizeof(AiqAnalyzerGroup_t*);
    pHdlTh->mHandlerGroups = aiqList_init(&hdlGropuCfg);
    if (!pHdlTh->mHandlerGroups) {
        LOGE_ANALYZER("init %s error", hdlGropuCfg._name);
        goto fail;
    }

    if (group) aiqList_push(pHdlTh->mHandlerGroups, &group);

    AiqListConfig_t msgqCfg;
    msgqCfg._name      = "msgQ";
    msgqCfg._item_nums = MSGHDL_MSGQ_MAX;
    msgqCfg._item_size = sizeof(AiqCoreMsg_t);
    pHdlTh->mMsgsQueue = aiqList_init(&msgqCfg);
    if (!pHdlTh->mMsgsQueue) {
        LOGE_ANALYZER("init %s error", msgqCfg._name);
        goto fail;
    }

    pHdlTh->_base = aiqThread_init(name, grpMsgHdlTh_func, pHdlTh);
    if (!pHdlTh->_base) goto fail;

    return XCAM_RETURN_NO_ERROR;

fail:
    return XCAM_RETURN_ERROR_FAILED;
}

void AiqAnalyzeGroupMsgHdlThread_deinit(AiqAnalyzeGroupMsgHdlThread_t* pHdlTh) {
    ENTER_ANALYZER_FUNCTION();
    if (pHdlTh->mHandlerGroups) aiqList_deinit(pHdlTh->mHandlerGroups);
    if (pHdlTh->mMsgsQueue) aiqList_deinit(pHdlTh->mMsgsQueue);
    if (pHdlTh->_base) aiqThread_deinit(pHdlTh->_base);

    aiqMutex_deInit(&pHdlTh->_mutex);
    aiqCond_deInit(&pHdlTh->_cond);
    EXIT_ANALYZER_FUNCTION();
}

void AiqAnalyzeGroupMsgHdlThread_add_group(AiqAnalyzeGroupMsgHdlThread_t* pHdlTh,
                                           AiqAnalyzerGroup_t* group) {
    aiqList_push(pHdlTh->mHandlerGroups, &group);
}

void AiqAnalyzeGroupMsgHdlThread_start(AiqAnalyzeGroupMsgHdlThread_t* pHdlTh) {
    ENTER_ANALYZER_FUNCTION();
    pHdlTh->bQuit = false;
    if (!aiqThread_start(pHdlTh->_base)) {
        LOGE_ANALYZER("%s failed", __func__);
        return;
    }
    EXIT_ANALYZER_FUNCTION();
}

void AiqAnalyzeGroupMsgHdlThread_stop(AiqAnalyzeGroupMsgHdlThread_t* pHdlTh) {
    ENTER_ANALYZER_FUNCTION();
    aiqMutex_lock(&pHdlTh->_mutex);
    pHdlTh->bQuit = true;
    aiqMutex_unlock(&pHdlTh->_mutex);
    aiqCond_broadcast(&pHdlTh->_cond);
    aiqThread_stop(pHdlTh->_base);
    EXIT_ANALYZER_FUNCTION();
}

bool AiqAnalyzeGroupMsgHdlThread_push_msg(AiqAnalyzeGroupMsgHdlThread_t* pHdlTh,
                                          AiqCoreMsg_t* buffer) {
    if (!pHdlTh->_base->_started) return false;

    deepCpyAiqCoreMsg(buffer);

    int ret = aiqList_push(pHdlTh->mMsgsQueue, buffer);
    if (ret) {
        LOGE_ANALYZER("push msg failed", __func__);
        clearAiqCoreMsg(buffer, NULL);
        return false;
	}

    aiqMutex_lock(&pHdlTh->_mutex);
	aiqCond_broadcast(&pHdlTh->_cond);
    aiqMutex_unlock(&pHdlTh->_mutex);

    return true;
}

XCamReturn AiqAnalyzeGroupManager_init(AiqAnalyzeGroupManager_t* pGroupMan, AiqCore_t* aiqCore,
                                       bool single_thread) {
    ENTER_ANALYZER_FUNCTION();
    pGroupMan->mSingleThreadMode = single_thread;
    pGroupMan->mAiqCore          = aiqCore;
    pGroupMan->mDefaultDelayCnt  = ISP_PARAMS_EFFECT_DELAY_CNT;
    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void AiqAnalyzeGroupManager_deinit(AiqAnalyzeGroupManager_t* pGroupMan) {
    ENTER_ANALYZER_FUNCTION();
	if (pGroupMan->mMsgThrd) {
		AiqAnalyzeGroupMsgHdlThread_deinit(pGroupMan->mMsgThrd);
        aiq_free(pGroupMan->mMsgThrd);
        pGroupMan->mMsgThrd = NULL;
	}

    for (int i = 0; i < RK_AIQ_CORE_ANALYZE_MAX; i++) {
		if (pGroupMan->mGroupMap[i]) {
			AiqAnalyzerGroup_deinit(pGroupMan->mGroupMap[i]);
			aiq_free(pGroupMan->mGroupMap[i]);
			pGroupMan->mGroupMap[i] = NULL;
		}
    }
    EXIT_ANALYZER_FUNCTION();
}

XCamReturn AiqAnalyzeGroupManager_start(AiqAnalyzeGroupManager_t* pGroupMan)
{
    ENTER_ANALYZER_FUNCTION();
    if (pGroupMan->mSingleThreadMode) {
		AiqAnalyzeGroupMsgHdlThread_start(pGroupMan->mMsgThrd);
    } else {
		AiqAnalyzerGroup_t* pGrp = NULL;
        for (int i = 0; i < RK_AIQ_CORE_ANALYZE_MAX; i++) {
            pGrp = pGroupMan->mGroupMap[i];
            if (pGrp) {
				AiqAnalyzerGroup_start(pGrp);
			}
		}
    }
    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqAnalyzeGroupManager_stop(AiqAnalyzeGroupManager_t* pGroupMan)
{
    ENTER_ANALYZER_FUNCTION();
    if (pGroupMan->mSingleThreadMode) {
		AiqAnalyzeGroupMsgHdlThread_stop(pGroupMan->mMsgThrd);
    } else {
        AiqAnalyzerGroup_t* pGrp = NULL;
        for (int i = 0; i < RK_AIQ_CORE_ANALYZE_MAX; i++) {
            pGrp = pGroupMan->mGroupMap[i];
            if (pGrp)
                AiqAnalyzerGroup_stop(pGrp);
        }
    }
    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

void AiqAnalyzeGroupManager_clean(AiqAnalyzeGroupManager_t* pGroupMan)
{
    if (pGroupMan->mSingleThreadMode) {
		AiqAnalyzeGroupMsgHdlThread_t* pHdlTh = pGroupMan->mMsgThrd;
		AiqListItem_t* pItem = NULL;
		bool rm              = false;
		AIQ_LIST_FOREACH(pHdlTh->mMsgsQueue, pItem, rm) {
			AiqCoreMsg_t* pMsg = (AiqCoreMsg_t*)pItem->_pData;
			clearAiqCoreMsg(pMsg, NULL);
			rm    = true;
			pItem = aiqList_erase_item_locked(pHdlTh->mMsgsQueue, pItem);
		}
	}

	AiqAnalyzerGroup_t* pGrp = NULL;
	for (int i = 0; i < RK_AIQ_CORE_ANALYZE_MAX; i++) {
		pGrp = pGroupMan->mGroupMap[i];
		if (pGrp) {
			AiqMapItem_t* pItem = NULL;
			bool rm             = false;
			AIQ_MAP_FOREACH(pGrp->mGroupMsgMap, pItem, rm) {
				GroupMessage_t* pMsg = (GroupMessage_t*)(pItem->_pData);
				for (int i = 0; i < pMsg->msg_cnts; i++) {
					clearAiqCoreMsg(&pMsg->msgList[i], pGrp->mAiqCore->mAlogsGroupSharedParamsMap[pGrp->mGroupType]);
				}
				pMsg->msg_cnts = 0;
				pItem          = aiqMap_erase_locked(pGrp->mGroupMsgMap, pItem->_key);
				rm             = true;
			}
		}
	}
}

XCamReturn AiqAnalyzeGroupManager_handleMessage(AiqAnalyzeGroupManager_t* pGroupMan,
                                                AiqCoreMsg_t* msg) {
    bool ret = false;
    // XCAM_STATIC_FPS_CALCULATION(HANDLEMSG, 100);
    if (pGroupMan->mSingleThreadMode) {
        ret = AiqAnalyzeGroupMsgHdlThread_push_msg(pGroupMan->mMsgThrd, msg);
        LOGD_ANALYZER_SUBM(ANALYZER_SUBM, "camId: %d, Handle message(%s) id[%d]",
                           pGroupMan->mAiqCore->mAlogsComSharedParams.mCamPhyId,
                           MessageType2Str[msg->msg_id], msg->frame_id);
    } else {
        AiqAnalyzerGroup_t* pGrp = NULL;
        for (int i = 0; i < RK_AIQ_CORE_ANALYZE_MAX; i++) {
            pGrp = pGroupMan->mGroupMap[i];
            if (pGrp && (pGrp->mDepsFlag & (1ULL << msg->msg_id))) {
                LOGD_ANALYZER_SUBM(
                    ANALYZER_SUBM,
                    "camId: %d, Handle message(%s) id[%d] on group(%s), flags %" PRIx64 "",
                    pGroupMan->mAiqCore->mAlogsComSharedParams.mCamPhyId,
                    MessageType2Str[msg->msg_id], msg->frame_id, AnalyzerGroupType2Str[i],
                    pGrp->mDepsFlag);
                ret = AiqAnalyzerGroup_pushMsg(pGrp, msg);
            }
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn groupMessageHandler(AiqCore_t* aiqCore, AiqCoreMsg_t* msgs, int msg_cnts,
                                      uint32_t id, uint64_t grpId) {
    RkAiqAlgosGroupShared_t* shared = aiqCore->mAlogsGroupSharedParamsMap[grpId];
    XCAM_ASSERT(shared != NULL);
    AiqCoreMsg_t* vdBufMsg = NULL;
    shared->frameId        = id;
    for (int i = 0; i < msg_cnts; i++) {
        vdBufMsg = &msgs[i];
        switch (vdBufMsg->msg_id) {
            case XCAM_MESSAGE_SOF_INFO_OK: {
                AlgoRstShared_t* pRst        = *(AlgoRstShared_t**)(vdBufMsg->buf);
                AiqSofInfoWrapper_t* sofInfo = (AiqSofInfoWrapper_t*)pRst->_data;
                shared->curExp               = sofInfo->curExp->aecExpInfo;
                shared->preExp               = sofInfo->preExp->aecExpInfo;
                shared->nxtExp               = sofInfo->nxtExp->aecExpInfo;
                shared->sof                  = sofInfo->sof;
                shared->iso                  = sofInfo->iso;
            } break;
            case XCAM_MESSAGE_AEC_STATS_OK: {
                aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
                if (pRst) {
                    shared->aecStatsBuf = pRst;
                }
            } break;
            case XCAM_MESSAGE_AWB_STATS_OK: {
                aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
                if (pRst) {
                    shared->awbStatsBuf = pRst;
                }
            } break;
            case XCAM_MESSAGE_AF_STATS_OK: {
                aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
                if (pRst) {
                    shared->afStatsBuf = pRst;
                }
            } break;
            case XCAM_MESSAGE_ISP_POLL_SP_OK:
                shared->sp = *(XCamVideoBuffer**)(vdBufMsg->buf);
                break;
            case XCAM_MESSAGE_ISP_GAIN_OK:
                shared->ispGain = *(XCamVideoBuffer**)(vdBufMsg->buf);
                break;
            case XCAM_MESSAGE_ISP_POLL_TX_OK:
                shared->tx = *(XCamVideoBuffer**)(vdBufMsg->buf);
                break;
            case XCAM_MESSAGE_PDAF_STATS_OK: {
                aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
                if (pRst) {
                    shared->pdafStatsBuf = pRst;
                }
            } break;
            case XCAM_MESSAGE_AE_PRE_RES_OK: {
                AlgoRstShared_t* pRst = *(AlgoRstShared_t**)(vdBufMsg->buf);
                if (pRst) {
                    shared->res_comb.ae_pre_res_c = pRst;
                }
            } break;
            case XCAM_MESSAGE_AE_PROC_RES_OK: {
                AlgoRstShared_t* pRst = *(AlgoRstShared_t**)(vdBufMsg->buf);
                if (pRst) {
                    shared->res_comb.ae_proc_res_c = pRst;
                }
            } break;
            case XCAM_MESSAGE_AWB_PROC_RES_OK: {
                AlgoRstShared_t* pRst = *(AlgoRstShared_t**)(vdBufMsg->buf);
                if (pRst) {
                    shared->res_comb.awb_proc_res_c = pRst;
                }
            } break;
            case XCAM_MESSAGE_BLC_PROC_RES_OK: {
                AlgoRstShared_t* pRst = *(AlgoRstShared_t**)(vdBufMsg->buf);
                if (pRst) {
                    shared->res_comb.blc_proc_res = (blc_param_t*)pRst->_data;
                }
            } break;
            case XCAM_MESSAGE_YNR_PROC_RES_OK: {
                AlgoRstShared_t* pRst = *(AlgoRstShared_t**)(vdBufMsg->buf);
                if (pRst) {
                    shared->res_comb.ynr_proc_res = ((RkAiqAlgoProcResYnr*)pRst->_data)->ynrRes;
                }
            } break;
            case XCAM_MESSAGE_ADEHAZE_STATS_OK: {
                aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
                if (pRst) {
                    shared->adehazeStatsBuf = pRst;
                }
            } break;
            case XCAM_MESSAGE_AGAIN_STATS_OK: {
                aiq_stats_base_t* pRst = *(aiq_stats_base_t**)(vdBufMsg->buf);
                if (pRst) {
                    shared->againStatsBuf = pRst;
                }
            } break;
            case XCAM_MESSAGE_VICAP_POLL_SCL_OK: {
                rk_aiq_scale_raw_info_t* buf_info = (rk_aiq_scale_raw_info_t*)(vdBufMsg->buf);
                shared->scaleRawInfo.bpp          = buf_info->bpp;
                if (buf_info->flags == RK_AIQ_VICAP_SCALE_HDR_MODE_NORMAL) {
                    shared->scaleRawInfo.raw_s = buf_info->raw_s;
                } else if (buf_info->flags == RK_AIQ_VICAP_SCALE_HDR_MODE_2_HDR) {
                    shared->scaleRawInfo.raw_l = buf_info->raw_l;
                    shared->scaleRawInfo.raw_s = buf_info->raw_s;
                } else if (buf_info->flags == RK_AIQ_VICAP_SCALE_HDR_MODE_3_HDR) {
                    shared->scaleRawInfo.raw_l = buf_info->raw_l;
                    shared->scaleRawInfo.raw_s = buf_info->raw_s;
                    shared->scaleRawInfo.raw_m = buf_info->raw_m;
                }
                break;
            }
            default:
                break;
        }
    }

	AiqCore_groupAnalyze(aiqCore, grpId, shared);

    return XCAM_RETURN_NO_ERROR;
}

void AiqAnalyzeGroupManager_parseAlgoGroup(AiqAnalyzeGroupManager_t* pGroupMan,
                                           const struct RkAiqAlgoDesCommExt* algoDes) {
    uint64_t disAlgosMask = AiqCore_getInitDisAlgosMask(pGroupMan->mAiqCore);
    if (pGroupMan->mSingleThreadMode && !pGroupMan->mMsgThrd) {
        pGroupMan->mMsgThrd =
            (AiqAnalyzeGroupMsgHdlThread_t*)aiq_mallocz(sizeof(AiqAnalyzeGroupMsgHdlThread_t));
        AiqAnalyzeGroupMsgHdlThread_init(pGroupMan->mMsgThrd, "GrpMsgThrd", NULL);
    }

    for (size_t i = 0; algoDes[i].des != NULL; i++) {
        int algo_type = algoDes[i].des->type;
        if ((1ULL << algo_type) & disAlgosMask) continue;

        if (algoDes[i].grpConds.size > MAX_MESSAGES)
            LOGE("group conds > %d, should increase MAX_MESSAGES", MAX_MESSAGES);

        uint64_t deps_flag = 0;
        for (size_t j = 0; j < algoDes[i].grpConds.size; j++)
            deps_flag |= 1ULL << algoDes[i].grpConds.conds[j].cond;
        enum rk_aiq_core_analyze_type_e group = algoDes[i].group;
        if (pGroupMan->mAiqCore->mAlgoHandleMaps[algo_type]) {
            int cnts = pGroupMan->mGroupAlgoListCnts[group];
            pGroupMan->mGroupAlgoListMap[group][cnts] =
                pGroupMan->mAiqCore->mAlgoHandleMaps[algo_type];
            pGroupMan->mGroupAlgoListCnts[group]++;

            cnts = pGroupMan->mGroupAlgoListCnts[RK_AIQ_CORE_ANALYZE_MAX];
            pGroupMan->mGroupAlgoListMap[RK_AIQ_CORE_ANALYZE_MAX][algo_type] =
                pGroupMan->mAiqCore->mAlgoHandleMaps[algo_type];
            pGroupMan->mGroupAlgoListCnts[RK_AIQ_CORE_ANALYZE_MAX]++;
        }

        if (pGroupMan->mGroupMap[group]) continue;
        pGroupMan->mGroupMap[group] = (AiqAnalyzerGroup_t*)aiq_mallocz(sizeof(AiqAnalyzerGroup_t));
        AiqAnalyzerGroup_init(pGroupMan->mGroupMap[group], pGroupMan->mAiqCore, group, deps_flag,
                              &algoDes[i].grpConds, pGroupMan->mSingleThreadMode);
        if (pGroupMan->mSingleThreadMode)
            AiqAnalyzeGroupMsgHdlThread_add_group(pGroupMan->mMsgThrd, pGroupMan->mGroupMap[group]);
        pGroupMan->mGroupMap[group]->mHandler = groupMessageHandler;
        LOGD_ANALYZER_SUBM(ANALYZER_SUBM, "Created group %" PRIx64 " for dep flags %" PRIx64 "",
                           (uint64_t)group, deps_flag);
    }
}

void AiqAnalyzeGroupManager_setDelayCnts(AiqAnalyzeGroupManager_t* pGroupMan, int delayCnts) {
    for (int i = 0; i < RK_AIQ_CORE_ANALYZE_MAX; i++) {
        if (pGroupMan->mGroupMap[i])
            AiqAnalyzerGroup_setDelayCnts(pGroupMan->mGroupMap[i], delayCnts);
    }
}

XCamReturn AiqAnalyzeGroupManager_firstAnalyze(AiqAnalyzeGroupManager_t* pGroupMan) {
    for (int i = 0; i < RK_AIQ_CORE_ANALYZE_MAX; i++) {
        if (pGroupMan->mGroupMap[i]) {
            AiqCore_groupAnalyze(pGroupMan->mAiqCore, i, pGroupMan->mAiqCore->mAlogsGroupSharedParamsMap[i]);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

AiqAlgoHandler_t** AiqAnalyzeGroupManager_getGroupAlgoList(AiqAnalyzeGroupManager_t* pGroupMan,
                                                      enum rk_aiq_core_analyze_type_e group, int* lens) {
	enum rk_aiq_core_analyze_type_e dst_grp = 0;

	if (group == RK_AIQ_CORE_ANALYZE_ALL) {
		dst_grp = RK_AIQ_CORE_ANALYZE_MAX;
		*lens = RK_AIQ_ALGO_TYPE_MAX;
	} else {
		dst_grp = group;
		*lens = pGroupMan->mGroupAlgoListCnts[dst_grp];
	}

    return pGroupMan->mGroupAlgoListMap[dst_grp];
}

AiqAnalyzerGroup_t** AiqAnalyzeGroupManager_getGroups(AiqAnalyzeGroupManager_t* pGroupMan) {
    return pGroupMan->mGroupMap;
}

void AiqAnalyzeGroupManager_rmAlgoHandle(AiqAnalyzeGroupManager_t* pGroupMan, int algoType)
{
	AiqAlgoHandler_t* pHdl = NULL;
	for (int i = 0; i < RK_AIQ_CORE_ANALYZE_MAX + 1; i++) {
		for (int j = 0; j < RK_AIQ_ALGO_TYPE_MAX; j++) {
			pHdl = pGroupMan->mGroupAlgoListMap[i][j];
			if (pHdl && (AiqAlgoHandler_getAlgoType(pHdl) == algoType)) {
				pGroupMan->mGroupAlgoListMap[i][j] = NULL;
				pGroupMan->mGroupAlgoListCnts[i] -= 1;
			}
		}
	}
	//TODO: remove groups ?
}

void AiqAnalyzeGroupManager_awakenClean(AiqAnalyzeGroupManager_t* pGroupMan, uint32_t sequence)
{
	AiqAnalyzeGroupManager_stop(pGroupMan);
    AiqAnalyzeGroupManager_clean(pGroupMan);
    for (int i = 0; i < RK_AIQ_CORE_ANALYZE_MAX; i++) {
		if (pGroupMan->mGroupMap[i]) {
			AiqAnalyzerGroup_setWakenId(pGroupMan->mGroupMap[i], sequence);
		}
    }
	AiqAnalyzeGroupManager_start(pGroupMan);
}

XCamReturn AiqAnalyzeGroupManager_resetDelayCnt(AiqAnalyzeGroupManager_t* pGroupMan, int delayCnt)
{
	if (pGroupMan->mDefaultDelayCnt == delayCnt) {
        return XCAM_RETURN_NO_ERROR;
    }

    pGroupMan->mDefaultDelayCnt = delayCnt;
    for (int i = 0; i < RK_AIQ_CORE_ANALYZE_MAX; i++) {
        if (pGroupMan->mGroupMap[i]) {
            for (uint32_t j = 0; j < pGroupMan->mGroupMap[i]->mGrpConds.size; j++) {
                switch (pGroupMan->mGroupMap[i]->mGrpConds.conds[j].cond) {
                    case XCAM_MESSAGE_AEC_STATS_OK:
                    case XCAM_MESSAGE_AWB_STATS_OK:
                    case XCAM_MESSAGE_AF_STATS_OK:
                    case XCAM_MESSAGE_PDAF_STATS_OK:
                    case XCAM_MESSAGE_AGAIN_STATS_OK:
                    case XCAM_MESSAGE_ADEHAZE_STATS_OK:
                        pGroupMan->mGroupMap[i]->mGrpConds.conds[j].delay = delayCnt;
                        break;
                    default:
                        break;
                }
            }
            pGroupMan->mGroupMap[i]->mGrpMsgOverflowCnt = delayCnt;
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_DUMPSYS
int AiqAnalyzerGroup_dump(void* self, st_string* result, int argc, void* argv[]) {
    group_analyzer_dump_mod_param((AiqAnalyzeGroupManager_t*)self, result);
    group_analyzer_dump_attr((AiqAnalyzeGroupManager_t*)self, result);
    group_analyzer_dump_msg_hdl_status((AiqAnalyzeGroupManager_t*)self, result);
    group_analyzer_dump_msg_map_status1((AiqAnalyzeGroupManager_t*)self, result);
    group_analyzer_dump_msg_map_status2((AiqAnalyzeGroupManager_t*)self, result);

    return 0;
}
#endif
