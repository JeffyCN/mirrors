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

#include "RkAiqCamGroupManager_c.h"
#include "RkAiqManager_c.h"
#include "aiq_core_c/aiq_core.h"
#include "aiq_core_c/AiqCoreConfig.h"
#include "aiq_core_c/aiq_algo_handler.h"
#include "aiq_algo_camgroup_handler.h"

#define MSG_REPROC_NUM_MAX 8
#define PENDING_RESULTS_MAX 5

static XCamReturn syncSingleCamResultWithMaster(AiqCamGroupManager_t* pCamGrpMan, rk_aiq_groupcam_result_t* gc_res);
static XCamReturn AiqCamGroupManager_reProcess(AiqCamGroupManager_t* pCamGrpMan, rk_aiq_groupcam_result_t* gc_res);
static void AiqCamGroupManager_relayToHwi(AiqCamGroupManager_t* pCamGrpMan, rk_aiq_groupcam_result_t* gc_res);
static void AiqCamGroupManager_clearGroupCamResult(AiqCamGroupManager_t* pCamGrpMan, uint32_t frameId);
static void AiqCamGroupManager_putGroupCamResult(AiqCamGroupManager_t* pCamGrpMan, rk_aiq_groupcam_result_t* gc_res);
static void calcHdrIso(AiqCamGroupManager_t* pCamGrpMan, RKAiqAecExpInfo_t* pCurExp, rk_aiq_singlecam_3a_result_t *singleCam3aRes);
static void clearGroupCamResult_Locked(AiqCamGroupManager_t* pCamGrpMan, uint32_t frameId);

static void clearSingleCamResult(rk_aiq_singlecam_result_t* pSres)
{
	if (pSres->_mAwbStats) {
        AIQ_REF_BASE_UNREF(&pSres->_mAwbStats->_ref_base);
		pSres->_mAwbStats = NULL;
	}
	if (pSres->_mAeStats) {
        AIQ_REF_BASE_UNREF(&pSres->_mAeStats->_ref_base);
		pSres->_mAeStats = NULL;
	}
	if (pSres->_fullIspParam) {
        AIQ_REF_BASE_UNREF(&pSres->_fullIspParam->_base._ref_base);
		pSres->_fullIspParam = NULL;
	}

	AlgoRstShared_t* stats_buf = pSres->_3aResults.aec._aePreRes_c;
	if (stats_buf) {
        AIQ_REF_BASE_UNREF(&stats_buf->_ref_base);
		pSres->_3aResults.aec._aePreRes_c = NULL;
	}
	aiq_memset(&pSres->_3aResults, 0, sizeof(pSres->_3aResults));
}

static void clearSingleCamStatus(rk_aiq_singlecam_result_status_t* pSglStatus)
{
	pSglStatus->_validAlgoResBits = 0;
	pSglStatus->_validCoreMsgsBits = 0;
	clearSingleCamResult(&pSglStatus->_singleCamResults);
}

static void clearGroupcamResults(rk_aiq_groupcam_result_t* pGrpRes)
{
	pGrpRes->_validCamResBits = 0;
	pGrpRes->_ready = false;
	pGrpRes->_frameId = -1;
	pGrpRes->_refCnt = 0;
	for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++)
		clearSingleCamStatus(&pGrpRes->_singleCamResultsStatus[i]);
}

static bool grpMsgHdlTh_func(void* user_data) {
    ENTER_ANALYZER_FUNCTION();

    AiqCamGroupReprocTh_t* pReprocTh = (AiqCamGroupReprocTh_t*)user_data;

    aiqMutex_lock(&pReprocTh->_mutex);
    while (!pReprocTh->bQuit && aiqList_size(pReprocTh->mMsgsQueue) <= 0) {
        aiqCond_wait(&pReprocTh->_cond, &pReprocTh->_mutex);
    }
    if (pReprocTh->bQuit) {
        aiqMutex_unlock(&pReprocTh->_mutex);
        LOGD_CAMGROUP("quit reprocMsgTh !");
        return false;
    }
    aiqMutex_unlock(&pReprocTh->_mutex);

    rk_aiq_groupcam_result_t* msg;
    int ret = aiqList_get(pReprocTh->mMsgsQueue, &msg);
    if (ret) {
        // ignore error
        return true;
    }
    // XCAM_STATIC_FPS_CALCULATION(GROUPMSGTH, 100);
    rk_aiq_groupcam_result_t* camGroupRes = msg;
    if (camGroupRes->_ready) {
        ret = syncSingleCamResultWithMaster(pReprocTh->pCamGrpMan, camGroupRes);
        if (ret < 0) {
            LOGW_CAMGROUP("Failed to sync single cam result with master");
        }

        ret = AiqCamGroupManager_rePrepare(pReprocTh->pCamGrpMan);
        ret = AiqCamGroupManager_reProcess(pReprocTh->pCamGrpMan, camGroupRes);
        if (ret < 0) {
            LOGW_CAMGROUP("reprocess error, ignore!");
        } else
            AiqCamGroupManager_relayToHwi(pReprocTh->pCamGrpMan, camGroupRes);
    }
    // delete the processed result
    AiqCamGroupManager_putGroupCamResult(pReprocTh->pCamGrpMan, camGroupRes); // paired with sendFrame
    AiqCamGroupManager_clearGroupCamResult(pReprocTh->pCamGrpMan, camGroupRes->_frameId);

    EXIT_ANALYZER_FUNCTION();

    return true;
}

XCamReturn AiqCamGroupReprocTh_init(AiqCamGroupReprocTh_t* pReprocTh, char* name,
                                            AiqCamGroupManager_t* pCamGroupManager)
{
    ENTER_CAMGROUP_FUNCTION();
	pReprocTh->pCamGrpMan = pCamGroupManager;
    aiqMutex_init(&pReprocTh->_mutex);
    aiqCond_init(&pReprocTh->_cond);

    AiqListConfig_t msgqCfg;
    msgqCfg._name      = "reProcMsgQ";
    msgqCfg._item_nums = MSG_REPROC_NUM_MAX;
    msgqCfg._item_size = sizeof(rk_aiq_groupcam_result_t*);
    pReprocTh->mMsgsQueue = aiqList_init(&msgqCfg);
    if (!pReprocTh->mMsgsQueue) {
        LOGE_CAMGROUP("init %s error", msgqCfg._name);
        goto fail;
    }

    pReprocTh->_base = aiqThread_init(name, grpMsgHdlTh_func, pReprocTh);
    if (!pReprocTh->_base) goto fail;

    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;

fail:
    return XCAM_RETURN_ERROR_FAILED;
}

void AiqCamGroupReprocTh_deinit(AiqCamGroupReprocTh_t* pReprocTh)
{
    ENTER_CAMGROUP_FUNCTION();
	if (pReprocTh->mMsgsQueue) {
		aiqList_deinit(pReprocTh->mMsgsQueue);
		pReprocTh->mMsgsQueue = NULL;
	}

	if (pReprocTh->_base) {
		aiqThread_deinit(pReprocTh->_base);
		pReprocTh->_base = NULL;
	}

    aiqMutex_deInit(&pReprocTh->_mutex);
    aiqCond_deInit(&pReprocTh->_cond);
    EXIT_CAMGROUP_FUNCTION();
}

void AiqCamGroupReprocTh_start(AiqCamGroupReprocTh_t* pReprocTh)
{
    ENTER_CAMGROUP_FUNCTION();
    pReprocTh->bQuit = false;
    if (!aiqThread_start(pReprocTh->_base)) {
        LOGE_CAMGROUP("%s failed", __func__);
        return;
    }
    EXIT_CAMGROUP_FUNCTION();
}

void AiqCamGroupReprocTh_stop(AiqCamGroupReprocTh_t* pReprocTh)
{
    ENTER_CAMGROUP_FUNCTION();
    aiqMutex_lock(&pReprocTh->_mutex);
    pReprocTh->bQuit = true;
    aiqMutex_unlock(&pReprocTh->_mutex);
    aiqCond_broadcast(&pReprocTh->_cond);
    aiqThread_stop(pReprocTh->_base);
    AiqListItem_t* pItem = NULL;
    bool rm              = false;
    AIQ_LIST_FOREACH(pReprocTh->mMsgsQueue, pItem, rm) {
        rk_aiq_groupcam_result_t* pMsg = *((rk_aiq_groupcam_result_t**)pItem->_pData);
        clearGroupcamResults(pMsg);
        rm    = true;
        pItem = aiqList_erase_item_locked(pReprocTh->mMsgsQueue, pItem);
    }
    EXIT_CAMGROUP_FUNCTION();
}

bool AiqCamGroupReprocTh_sendFrame(AiqCamGroupReprocTh_t* pReprocTh, rk_aiq_groupcam_result_t* gc_result)
{
    if (!pReprocTh->_base->_started) return false;
    aiqMutex_lock(&pReprocTh->pCamGrpMan->mCamGroupResMutex);
	gc_result->_refCnt++;
    int ret = aiqList_push(pReprocTh->mMsgsQueue, &gc_result);
    if (ret) {
        LOGE_CAMGROUP("push msg failed", __func__);
		gc_result->_refCnt--;
		aiqMutex_unlock(&pReprocTh->pCamGrpMan->mCamGroupResMutex);
        return false;
	}
    aiqMutex_unlock(&pReprocTh->pCamGrpMan->mCamGroupResMutex);

    aiqCond_broadcast(&pReprocTh->_cond);

    return true;
}

static XCamReturn AiqCamGroupManager_reProcess(AiqCamGroupManager_t* pCamGrpMan, rk_aiq_groupcam_result_t* gc_res)
{
    ENTER_CAMGROUP_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int arraySize = aiqMap_size(pCamGrpMan->mBindAiqsMap);
    if (arraySize == 0) {
        LOGW_CAMGROUP("no group cam, bypass");
        return XCAM_RETURN_NO_ERROR;
    }

    // assume all single cam runs same algos
	AiqMapItem_t* pItem = aiqMap_begin(pCamGrpMan->mBindAiqsMap);
    AiqManager_t* aiqManager = *(AiqManager_t**)pItem->_pData;
    AiqCore_t* aiqCore = aiqManager->mRkAiqAnalyzer;

    LOGD_CAMGROUP("camgroup: set reprocess params ... ");

    rk_aiq_singlecam_3a_result_t* camgroupParmasArray[arraySize];

    memset(camgroupParmasArray, 0, sizeof(camgroupParmasArray));

    int i = 0, vaild_cam_ind = 0;
    rk_aiq_singlecam_3a_result_t* scam_3a_res = NULL;
    rk_aiq_singlecam_result_t* scam_res = NULL;
    AiqFullParams_t* aiqParams = NULL;

#define RET_FAILED()                                              \
    do {                                                          \
        LOGE_CAMGROUP("re-process param failed at %d", __LINE__); \
        return XCAM_RETURN_ERROR_PARAM;                           \
    } while (0)

    for (i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
        if ((gc_res->_validCamResBits >> i) & 1) {
            scam_res = &gc_res->_singleCamResultsStatus[i]._singleCamResults;
            scam_3a_res = &scam_res->_3aResults;
            if (!scam_res->_fullIspParam)
                return XCAM_RETURN_ERROR_FAILED;
            aiqParams = scam_res->_fullIspParam;
            // fill 3a params
            if ((aiqParams->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM])) {
				AiqAecExpInfoWrapper_t* exp_param =
					(AiqAecExpInfoWrapper_t*)(aiqParams->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM]->_data);
                scam_3a_res->aec.exp_tbl = exp_param->ae_proc_res_rk.exp_set_tbl;
                scam_3a_res->aec.exp_tbl_size = &exp_param->ae_proc_res_rk.exp_set_cnt;
                scam_3a_res->aec.exp_i2c_params = &exp_param->exp_i2c_params;
                if (pCamGrpMan->mInit) {
                    scam_3a_res->aec._effAecExpInfo =
                        exp_param->ae_proc_res_rk.exp_set_tbl[0];
                    scam_3a_res->aec._bEffAecExpValid = true;
                    calcHdrIso(pCamGrpMan, &scam_3a_res->aec._effAecExpInfo, scam_3a_res);
                }
            } else {
                LOGW_CAMGROUP("camId:%d, framId:%u, exp is null", i, gc_res->_frameId);
                // frame 1,2 exp may be null now
                //if (gc_res->_frameId == 1)
                RET_FAILED();
            }
#if USE_NEWSTRUCT
            if (!aiqParams->pParamsArray[RESULT_TYPE_AESTATS_PARAM])
                RET_FAILED();
            scam_3a_res->aec._aeNewCfg =
				(rk_aiq_isp_ae_stats_cfg_t*)(aiqParams->pParamsArray[RESULT_TYPE_AESTATS_PARAM]->_data);
#endif

#if RKAIQ_HAVE_AWB_V39
            if (!aiqParams->pParamsArray[RESULT_TYPE_AWB_PARAM])
                RET_FAILED();
            scam_3a_res->awb._awbCfgV39 =
				(rk_aiq_isp_awb_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_AWB_PARAM]->_data);
#endif
#if USE_NEWSTRUCT && defined(RKAIQ_HAVE_DEHAZE)
            if (!aiqParams->pParamsArray[RESULT_TYPE_DEHAZE_PARAM])
                RET_FAILED();
            scam_3a_res->dehaze =
				(rk_aiq_isp_dehaze_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_DEHAZE_PARAM]->_data);
#endif
#if USE_NEWSTRUCT
            if (!aiqParams->pParamsArray[RESULT_TYPE_HISTEQ_PARAM])
                RET_FAILED();
            scam_3a_res->histeq =
				(rk_aiq_isp_histeq_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_HISTEQ_PARAM]->_data);
#endif
#if USE_NEWSTRUCT && defined(RKAIQ_HAVE_ENHANCE)
            if (!aiqParams->pParamsArray[RESULT_TYPE_ENH_PARAM])
                RET_FAILED();
            scam_3a_res->enh =
				(rk_aiq_isp_enh_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_ENH_PARAM]->_data);
#endif
#if USE_NEWSTRUCT
            if (!aiqParams->pParamsArray[RESULT_TYPE_MERGE_PARAM])
                RET_FAILED();
            scam_3a_res->merge =
				(rk_aiq_isp_merge_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_MERGE_PARAM]->_data);
#endif
#if USE_NEWSTRUCT
            if (!aiqParams->pParamsArray[RESULT_TYPE_AGAMMA_PARAM])
                RET_FAILED();
            scam_3a_res->gamma =
				(rk_aiq_isp_gamma_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_AGAMMA_PARAM]->_data);
#endif
#if USE_NEWSTRUCT
            if (!aiqParams->pParamsArray[RESULT_TYPE_DRC_PARAM])
                RET_FAILED();
            scam_3a_res->drc =
				(rk_aiq_isp_drc_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_DRC_PARAM]->_data);
#endif
#if RKAIQ_HAVE_RGBIR_REMOSAIC
#if USE_NEWSTRUCT
            if (!aiqParams->pParamsArray[RESULT_TYPE_RGBIR_PARAM])
                RET_FAILED();
            scam_3a_res->rgbir =
				(rk_aiq_isp_rgbir_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_RGBIR_PARAM]->_data);
#endif
#endif
#if RKAIQ_HAVE_AWB_V32 || RKAIQ_HAVE_AWB_V32LT||RKAIQ_HAVE_AWB_V39
            if (!aiqParams->pParamsArray[RESULT_TYPE_AWBGAIN_PARAM])
                RET_FAILED();
            scam_3a_res->awb._awbGainV32Params =
				(rk_aiq_isp_awb_gain_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_AWBGAIN_PARAM]->_data);
#endif

#if USE_NEWSTRUCT
            if (!aiqParams->pParamsArray[RESULT_TYPE_LSC_PARAM])
                RET_FAILED();
            scam_3a_res->lsc =
				(rk_aiq_isp_lsc_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_LSC_PARAM]->_data);
#endif

#if USE_NEWSTRUCT
            if (!aiqParams->pParamsArray[RESULT_TYPE_DPCC_PARAM])
                RET_FAILED();
            scam_3a_res->dpc =
				(rk_aiq_isp_dpcc_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_DPCC_PARAM]->_data);
#endif

#if USE_NEWSTRUCT
            if (!aiqParams->pParamsArray[RESULT_TYPE_CCM_PARAM])
                RET_FAILED();
            scam_3a_res->ccm =
				(rk_aiq_isp_ccm_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_CCM_PARAM]->_data);
#endif
#if RKAIQ_HAVE_3DLUT
#if USE_NEWSTRUCT
            if (!aiqParams->pParamsArray[RESULT_TYPE_LUT3D_PARAM])
                RET_FAILED();
            scam_3a_res->lut3d =
				(rk_aiq_isp_lut3d_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_LUT3D_PARAM]->_data);
#endif
#if RKAIQ_HAVE_HSV
            if (!aiqParams->pParamsArray[RESULT_TYPE_HSV_PARAM])
                RET_FAILED();
            scam_3a_res->hsv =
				(rk_aiq_isp_hsv_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_HSV_PARAM]->_data);
#endif
#endif
#if USE_NEWSTRUCT
            if (!aiqParams->pParamsArray[RESULT_TYPE_BLC_PARAM])
                RET_FAILED();
            scam_3a_res->ablc.blc =
				(rk_aiq_isp_blc_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_BLC_PARAM]->_data);
#endif

#if USE_NEWSTRUCT
            if (!aiqParams->pParamsArray[RESULT_TYPE_YNR_PARAM])
                RET_FAILED();
            scam_3a_res->ynr =
				(rk_aiq_isp_ynr_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_YNR_PARAM]->_data);
#endif

#if USE_NEWSTRUCT
            if (!aiqParams->pParamsArray[RESULT_TYPE_UVNR_PARAM])
                RET_FAILED();
            scam_3a_res->cnr =
				(rk_aiq_isp_cnr_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_UVNR_PARAM]->_data);
#endif

#if USE_NEWSTRUCT
            if (!aiqParams->pParamsArray[RESULT_TYPE_SHARPEN_PARAM])
                RET_FAILED();
            scam_3a_res->sharp =
				(rk_aiq_isp_sharp_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_SHARPEN_PARAM]->_data);
#endif
#if defined(RKAIQ_HAVE_SHARP_V40)  || defined(RKAIQ_HAVE_SHARP_V41)
            if (!aiqParams->pParamsArray[RESULT_TYPE_TEXEST_PARAM])
                RET_FAILED();
            scam_3a_res->texEst =
				(rk_aiq_isp_texEst_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_TEXEST_PARAM]->_data);
#endif

#if USE_NEWSTRUCT
            if (!aiqParams->pParamsArray[RESULT_TYPE_TNR_PARAM])
                RET_FAILED();
            scam_3a_res->btnr =
				(rk_aiq_isp_btnr_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_TNR_PARAM]->_data);
#endif

#if RKAIQ_HAVE_YUVME_V1
            if (!aiqParams->pParamsArray[RESULT_TYPE_MOTION_PARAM])
                RET_FAILED();
			// TODO
            //scam_3a_res->ayuvme._ayuvme_procRes_v1 =
				//(rk_aiq_isp_yme_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_MOTION_PARAM]->_data);
#endif

#if RKAIQ_HAVE_GAIN
            if (!aiqParams->pParamsArray[RESULT_TYPE_GAIN_PARAM])
                RET_FAILED();
            scam_3a_res->gain =
				(rk_aiq_isp_gain_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_GAIN_PARAM]->_data);
#endif

#if RKAIQ_HAVE_AIBNR
            if (!aiqParams->pParamsArray[RESULT_TYPE_AIBNR_PARAM])
                RET_FAILED();
            scam_3a_res->isp_aibnr_params =
                (rk_aiq_isp_aibnr_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_AIBNR_PARAM]->_data);
#endif

#if RKAIQ_HAVE_AIRMS
            if (!aiqParams->pParamsArray[RESULT_TYPE_AIRMS_PARAM])
                RET_FAILED();
            scam_3a_res->isp_airms_params =
                (rk_aiq_isp_airms_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_AIRMS_PARAM]->_data);
#endif

#if RKAIQ_HAVE_AIYNR
            if (!aiqParams->pParamsArray[RESULT_TYPE_AIYNR_PARAM])
                RET_FAILED();
            scam_3a_res->isp_aiynr_params =
                (rk_aiq_isp_aiynr_params_t*)(aiqParams->pParamsArray[RESULT_TYPE_AIYNR_PARAM]->_data);
#endif

            camgroupParmasArray[vaild_cam_ind++] = scam_3a_res;
        }
    }

    if (vaild_cam_ind == 0) {
        LOGE_CAMGROUP("vaild_cam_ind == 0!");
        return XCAM_RETURN_NO_ERROR;
    }
    if (vaild_cam_ind != arraySize) {
        LOGW_CAMGROUP("wrong num of valid cam res:%d,exp:%d",
                      vaild_cam_ind, arraySize);
    }

    uint32_t frameId = camgroupParmasArray[0]->_frameId;
    LOGD_CAMGROUP("camgroup: frameId:%u reprocessing ... ", frameId);

	for (int i = 0; i < RK_AIQ_ALGO_TYPE_MAX; i++) {
        AiqAlgoCamGroupHandler_t* curHdl = pCamGrpMan->mDefAlgoHandleMap[i];
        while (curHdl) {
            if (AiqAlgoCamGroupHandler_getEnable(curHdl)) {
                ret = curHdl->processing(curHdl, camgroupParmasArray);
                if (ret < 0) {
                    LOGW_CAMGROUP("algoHdl %d processing failed", AiqAlgoCamGroupHandler_getAlgoType(curHdl));
                }
            }
            curHdl = AiqAlgoCamGroupHandler_getNextHdl(curHdl);
        }
    }

    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static rk_aiq_groupcam_result_t* getGroupCamResult(AiqCamGroupManager_t* pCamGrpMan, uint32_t frameId, bool query_ready)
{
    aiqMutex_lock(&pCamGrpMan->mCamGroupResMutex);
    rk_aiq_groupcam_result_t* camGroupRes = NULL;

    AiqMapItem_t* pItem = aiqMap_get(pCamGrpMan->mCamGroupResMap, (void*)(uintptr_t)frameId);
    if (pItem) {
        camGroupRes = *(rk_aiq_groupcam_result_t**)pItem->_pData ;
        if (!query_ready && camGroupRes->_ready) {
			aiqMutex_unlock(&pCamGrpMan->mCamGroupResMutex);
            return NULL;
		}
        camGroupRes->_refCnt++;
        LOG1_CAMGROUP("camgroup res of frame: %u exists", frameId);
    } else {
        if (!query_ready) {
			aiqMutex_unlock(&pCamGrpMan->mCamGroupResMutex);
            return NULL;
		}
		int size = aiqMap_size(pCamGrpMan->mCamGroupResMap);
        if (size > PENDING_RESULTS_MAX) {
			AiqMapItem_t* pItem = aiqMap_begin(pCamGrpMan->mCamGroupResMap);
            LOGE_CAMGROUP("camgroup result map overflow:%d, first_id: %u",
                          size, (uint32_t)(long)pItem->_key);
            if (frameId >= 2)
                clearGroupCamResult_Locked(pCamGrpMan, frameId - 2);
        }
        if (frameId < pCamGrpMan->mClearedResultId) {
			aiqMutex_unlock(&pCamGrpMan->mCamGroupResMutex);
            LOGW_CAMGROUP("disorder frameId(%d) < mClearedResultId(%d)", frameId, pCamGrpMan->mClearedResultId);
            return NULL;
        }
        camGroupRes = aiq_mallocz(sizeof(rk_aiq_groupcam_result_t));
        if (!camGroupRes) {
            LOGE_CAMGROUP("malloc camGroup Res failed !");
			aiqMutex_unlock(&pCamGrpMan->mCamGroupResMutex);
            return NULL;
        }
		clearGroupcamResults(camGroupRes);
        camGroupRes->_frameId = frameId;
        camGroupRes->_refCnt++;
		pItem = aiqMap_insert(pCamGrpMan->mCamGroupResMap, (void*)(uintptr_t)frameId, &camGroupRes);
		if (!pItem) {
			aiq_free(camGroupRes);
			aiqMutex_unlock(&pCamGrpMan->mCamGroupResMutex);
            return NULL;
		}
        LOGD_CAMGROUP("malloc camgroup res for frame: %u success", frameId);
    }

    aiqMutex_unlock(&pCamGrpMan->mCamGroupResMutex);

    return camGroupRes;
}

static void setSingleCamStatusReady(AiqCamGroupManager_t* pCamGrpMan, rk_aiq_singlecam_result_status_t* status, rk_aiq_groupcam_result_t* gc_result)
{
    bool gc_result_ready = false;
    {
		aiqMutex_lock(&pCamGrpMan->mCamGroupResMutex);
        if (((status->_validCoreMsgsBits & pCamGrpMan->mRequiredMsgsMask) == pCamGrpMan->mRequiredMsgsMask) &&
                ((status->_validAlgoResBits & pCamGrpMan->mRequiredAlgoResMask) == pCamGrpMan->mRequiredAlgoResMask)) {
            status->_ready = true;
            LOGD_CAMGROUP("camgroup single cam res ready, camId:%d, frameId:%u",
                          status->_singleCamResults._3aResults._camId,
                          status->_singleCamResults._3aResults._frameId);
            gc_result->_validCamResBits |= 1 << status->_singleCamResults._3aResults._camId;
            if (!gc_result->_ready && (gc_result->_validCamResBits == pCamGrpMan->mRequiredCamsResMask)) {
                gc_result->_ready = true;
                gc_result_ready = true;
                LOGD_CAMGROUP("camgroup all cam res ready, frameId:%u",
                              status->_singleCamResults._3aResults._frameId);
            }
        }
		aiqMutex_unlock(&pCamGrpMan->mCamGroupResMutex);
    }

    if (gc_result_ready) {
        // force to ready
        uint32_t lastFrameId = status->_singleCamResults._3aResults._frameId - 1;
        rk_aiq_groupcam_result_t* last_gc_result = getGroupCamResult(pCamGrpMan, lastFrameId, false);
        if (last_gc_result && (pCamGrpMan->mState == CAMGROUP_MANAGER_STARTED)) {
            last_gc_result->_ready = true;
            last_gc_result->_validCamResBits = pCamGrpMan->mRequiredCamsResMask;
            rk_aiq_singlecam_result_status_t* last_scam_status = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                if ((last_gc_result->_validCamResBits >> i) & 1) {
                    last_scam_status = &last_gc_result->_singleCamResultsStatus[i];
                    last_scam_status->_validAlgoResBits = pCamGrpMan->mRequiredMsgsMask;
                    last_scam_status->_validCoreMsgsBits = pCamGrpMan->mRequiredAlgoResMask;
                }
            }
            LOGW_CAMGROUP("camgroup res frameId disorder, unready frameId:%u < cur ready frame %u",
                          lastFrameId, status->_singleCamResults._3aResults._frameId);
            AiqCamGroupReprocTh_sendFrame(&pCamGrpMan->mCamGroupReprocTh, last_gc_result);
        }
        AiqCamGroupManager_putGroupCamResult(pCamGrpMan, last_gc_result);
        // init params is reprocessed in func prepare
        if (pCamGrpMan->mState == CAMGROUP_MANAGER_STARTED) {
            LOGD_CAMGROUP("send frameId:%u ", gc_result->_frameId);
            AiqCamGroupReprocTh_sendFrame(&pCamGrpMan->mCamGroupReprocTh, gc_result);
        }
    } else {
		aiqMutex_lock(&pCamGrpMan->mCamGroupResMutex);
        if (status->_singleCamResults._fullIspParam) {
            AiqFullParams_t* scam_aiqParams = status->_singleCamResults._fullIspParam;
            LOG1_CAMGROUP("scam_aiqParams %p ", scam_aiqParams);
        }
        LOG1_CAMGROUP("camgroup result status: validCams:0x%x(req:0x%x), camId:%d, frameId:%u, "
                      "validAlgoResBits:0x%" PRIx64 "(req:0x%" PRIx64 "), validMsgBits:0x%" PRIx64 "(req:0x%" PRIx64 ")",
                      gc_result->_validCamResBits, pCamGrpMan->mRequiredCamsResMask,
                      status->_singleCamResults._3aResults._camId,
                      status->_singleCamResults._3aResults._frameId,
                      status->_validAlgoResBits, pCamGrpMan->mRequiredAlgoResMask,
                      status->_validCoreMsgsBits, pCamGrpMan->mRequiredMsgsMask);
		aiqMutex_unlock(&pCamGrpMan->mCamGroupResMutex);
    }
}

static void _syncParams(AiqFullParams_t* src, AiqFullParams_t* dst)
{
    if (src && dst) {
#if ISP_HW_V39
        aiq_params_base_t* pYnrBase    = dst->pParamsArray[RESULT_TYPE_YNR_PARAM];
        aiq_params_base_t* pDhzeBase   = dst->pParamsArray[RESULT_TYPE_DEHAZE_PARAM];
        if (pYnrBase && pDhzeBase &&
            (pYnrBase->is_update && src->pParamsArray[RESULT_TYPE_DEHAZE_PARAM] && !pDhzeBase->is_update)) {
            pDhzeBase->is_update = true;
            pDhzeBase->en = src->pParamsArray[RESULT_TYPE_DEHAZE_PARAM]->en;
            pDhzeBase->bypass = src->pParamsArray[RESULT_TYPE_DEHAZE_PARAM]->bypass;
            memcpy(pDhzeBase->_data, src->pParamsArray[RESULT_TYPE_DEHAZE_PARAM]->_data,
                    sizeof(dehaze_param_t));
        }

        aiq_params_base_t* pHisteqBase = dst->pParamsArray[RESULT_TYPE_HISTEQ_PARAM];
        if (src->pParamsArray[RESULT_TYPE_DEHAZE_PARAM]) {
            if (pDhzeBase && pHisteqBase &&
                pDhzeBase->is_update ^ pHisteqBase->is_update) {
                if (!pDhzeBase->is_update) {
                    pDhzeBase->en = src->pParamsArray[RESULT_TYPE_DEHAZE_PARAM]->en;
                    pDhzeBase->bypass = src->pParamsArray[RESULT_TYPE_DEHAZE_PARAM]->bypass;
                    memcpy(pDhzeBase->_data, src->pParamsArray[RESULT_TYPE_DEHAZE_PARAM]->_data,
                            sizeof(dehaze_param_t));
                } else {
                    pHisteqBase->en = src->pParamsArray[RESULT_TYPE_HISTEQ_PARAM]->en;
                    pHisteqBase->bypass = src->pParamsArray[RESULT_TYPE_HISTEQ_PARAM]->bypass;
                    memcpy(pHisteqBase->_data, src->pParamsArray[RESULT_TYPE_HISTEQ_PARAM]->_data,
                            sizeof(histeq_param_t));
                }
                pDhzeBase->is_update   = true;
                pHisteqBase->is_update = true;
            }
        }

        if (pDhzeBase && pDhzeBase->is_update)
            src->pParamsArray[RESULT_TYPE_DEHAZE_PARAM] = pDhzeBase;
        if (pHisteqBase && pHisteqBase->is_update)
            src->pParamsArray[RESULT_TYPE_HISTEQ_PARAM] = pHisteqBase;
#endif
        aiq_params_base_t* pDrcBase  = dst->pParamsArray[RESULT_TYPE_DRC_PARAM];
        aiq_params_base_t* pBtnrBase = dst->pParamsArray[RESULT_TYPE_TNR_PARAM];
        aiq_params_base_t* pAwbBase  = dst->pParamsArray[RESULT_TYPE_AWB_PARAM];
        aiq_params_base_t* pBlcBase  = dst->pParamsArray[RESULT_TYPE_BLC_PARAM];
        if (pDrcBase && pDrcBase->is_update ) {
            if (pAwbBase && src->pParamsArray[RESULT_TYPE_AWB_PARAM] && !pAwbBase->is_update) {
                pAwbBase->is_update = true;
                pAwbBase->en = src->pParamsArray[RESULT_TYPE_AWB_PARAM]->en;
                pAwbBase->bypass = src->pParamsArray[RESULT_TYPE_AWB_PARAM]->bypass;
                memcpy(pAwbBase->_data, src->pParamsArray[RESULT_TYPE_AWB_PARAM]->_data,
                        sizeof(rk_aiq_isp_awb_params_t));
            }

            if (pBlcBase && src->pParamsArray[RESULT_TYPE_BLC_PARAM] && !pBlcBase->is_update) {
                pBlcBase->is_update = true;
                pBlcBase->en = src->pParamsArray[RESULT_TYPE_BLC_PARAM]->en;
                pBlcBase->bypass = src->pParamsArray[RESULT_TYPE_BLC_PARAM]->bypass;
                memcpy(pBlcBase->_data, src->pParamsArray[RESULT_TYPE_BLC_PARAM]->_data,
                        sizeof(blc_param_t));
            }

            if (pBtnrBase && src->pParamsArray[RESULT_TYPE_TNR_PARAM] && !pBtnrBase->is_update) {
                pBtnrBase->is_update = true;
                pBtnrBase->en = src->pParamsArray[RESULT_TYPE_TNR_PARAM]->en;
                pBtnrBase->bypass = src->pParamsArray[RESULT_TYPE_TNR_PARAM]->bypass;
                memcpy(pBtnrBase->_data, src->pParamsArray[RESULT_TYPE_TNR_PARAM]->_data,
                        sizeof(btnr_param_t));
            }
        }

        // TODO: TNR/SHARP need update HWI params for each frame now
        aiq_params_base_t* psharpBase = dst->pParamsArray[RESULT_TYPE_SHARPEN_PARAM];

        if (psharpBase && src->pParamsArray[RESULT_TYPE_SHARPEN_PARAM] && !psharpBase->is_update) {
            psharpBase->is_update = true;
            psharpBase->en = src->pParamsArray[RESULT_TYPE_SHARPEN_PARAM]->en;
            psharpBase->bypass = src->pParamsArray[RESULT_TYPE_SHARPEN_PARAM]->bypass;
            memcpy(psharpBase->_data, src->pParamsArray[RESULT_TYPE_SHARPEN_PARAM]->_data,
                    sizeof(sharp_param_t));
            src->pParamsArray[RESULT_TYPE_SHARPEN_PARAM] = psharpBase;
        }

        if (pBtnrBase && src->pParamsArray[RESULT_TYPE_TNR_PARAM] && !pBtnrBase->is_update) {
            pBtnrBase->is_update = true;
            pBtnrBase->en = src->pParamsArray[RESULT_TYPE_TNR_PARAM]->en;
            pBtnrBase->bypass = src->pParamsArray[RESULT_TYPE_TNR_PARAM]->bypass;
            memcpy(pBtnrBase->_data, src->pParamsArray[RESULT_TYPE_TNR_PARAM]->_data,
                    sizeof(btnr_param_t));
        }

        //TODO: update _pLatestFullParams, may re-entry now
#if ISP_HW_V39
        if (pDhzeBase && pDhzeBase->is_update)
            src->pParamsArray[RESULT_TYPE_DEHAZE_PARAM] = pDhzeBase;
        if (pHisteqBase && pHisteqBase->is_update)
            src->pParamsArray[RESULT_TYPE_HISTEQ_PARAM] = pHisteqBase;
#endif
        if (pAwbBase && pAwbBase->is_update)
            src->pParamsArray[RESULT_TYPE_AWB_PARAM] = pAwbBase;
        if (pBlcBase && pBlcBase->is_update)
            src->pParamsArray[RESULT_TYPE_BLC_PARAM] = pBlcBase;
        if (pBtnrBase && pBtnrBase->is_update)
            src->pParamsArray[RESULT_TYPE_TNR_PARAM] = pBtnrBase;
        if (psharpBase && psharpBase->is_update)
            src->pParamsArray[RESULT_TYPE_SHARPEN_PARAM] = psharpBase;
    }
}

static void _fixAiqParamsIsp(AiqCamGroupManager_t* pCamGrpMan, rk_aiq_groupcam_result_t* camGroupRes)
{
	AiqMapItem_t* pItem = NULL;
    AiqManager_t* aiqManager = NULL;
    AiqCore_t* aiqCore = NULL;
	bool rm             = false;
    bool isFirst = true;
    AiqFullParams_t* mainFullParams = NULL;

	AIQ_MAP_FOREACH(pCamGrpMan->mBindAiqsMap, pItem, rm) {
		aiqManager = *(AiqManager_t**)pItem->_pData;
		aiqCore = aiqManager->mRkAiqAnalyzer;
        int camId = AiqCore_getCamPhyId(aiqCore);
        rk_aiq_singlecam_result_status_t* singleCamStatus =
            &camGroupRes->_singleCamResultsStatus[camId];
        rk_aiq_singlecam_result_t* singleCamRes = &singleCamStatus->_singleCamResults;

        if (!mainFullParams)
            mainFullParams = singleCamRes->_fullIspParam;
        _syncParams(&pCamGrpMan->_pLatestFullParams, singleCamRes->_fullIspParam);
	}
}

static void AiqCamGroupManager_relayToHwi(AiqCamGroupManager_t* pCamGrpMan, rk_aiq_groupcam_result_t* gc_res)
{
    rk_aiq_singlecam_result_t* singlecam_res = NULL;
    {
        int exp_tbl_size = -1;
        bool skip_apply_exp = false;
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            if ((gc_res->_validCamResBits >> i) & 1) {
                singlecam_res = &gc_res->_singleCamResultsStatus[i]._singleCamResults;
				if ((singlecam_res->_fullIspParam->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM])) {
                    int tmp_size =
						((AiqAecExpInfoWrapper_t*)(singlecam_res->_fullIspParam->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM]->_data))->ae_proc_res_rk.exp_set_cnt;
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
					AiqFullParams_t tmpFull;
					memset(&tmpFull, 0, sizeof(tmpFull));
					tmpFull._base.frame_id =
						singlecam_res->_fullIspParam->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM]->frame_id;
					tmpFull.pParamsArray[RESULT_TYPE_EXPOSURE_PARAM] =
						singlecam_res->_fullIspParam->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM];
                    if (tmpFull.pParamsArray[RESULT_TYPE_EXPOSURE_PARAM]) {
                        LOGD_CAMGROUP("camgroup: camId:%d, frameId:%u, exp_tbl_size:%d",
                                      i, gc_res->_frameId,
									  ((AiqAecExpInfoWrapper_t*)((tmpFull.pParamsArray[RESULT_TYPE_EXPOSURE_PARAM])->_data)) \
									  ->ae_proc_res_rk.exp_set_cnt);
                    }
					AiqMapItem_t* pItem = aiqMap_get(pCamGrpMan->mBindAiqsMap, (void*)(uintptr_t)i);
					AiqManager_t* aiqManager = *(AiqManager_t**)pItem->_pData;
					AiqManager_applyAnalyzerResult(aiqManager, &tmpFull, false );
					((AiqAecExpInfoWrapper_t*)(singlecam_res->_fullIspParam->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM]->_data)) \
						->ae_proc_res_rk.exp_set_cnt = 0;
					((AiqAecExpInfoWrapper_t*)(singlecam_res->_fullIspParam->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM]->_data)) \
						->exp_i2c_params.nNumRegs = 0;
                }
            }
        }
    }

    _fixAiqParamsIsp(pCamGrpMan, gc_res);

    for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
        if ((gc_res->_validCamResBits >> i) & 1) {
            singlecam_res = &gc_res->_singleCamResultsStatus[i]._singleCamResults;
            if (singlecam_res->_fullIspParam) {
				AiqMapItem_t* pItem = aiqMap_get(pCamGrpMan->mBindAiqsMap, (void*)(uintptr_t)i);
				AiqManager_t* aiqManager = *(AiqManager_t**)pItem->_pData;
                if (pCamGrpMan->mState == CAMGROUP_MANAGER_STARTED) {
                    LOGD_CAMGROUP("camgroup: relay camId %d params to aiq manager %p para %p!", i,
                                  aiqManager, singlecam_res->_fullIspParam);
					AiqManager_rkAiqCalcDone(aiqManager, singlecam_res->_fullIspParam);
                } else {
                    LOGD_CAMGROUP("camgroup: apply camId %d params to hwi directly %p para %p!", i,
                                  aiqManager, singlecam_res->_fullIspParam);
					AiqManager_applyAnalyzerResult(aiqManager, singlecam_res->_fullIspParam, false );
                }
            }
        }
    }
}

static void clearCamgrpResMap(AiqCamGroupManager_t* pCamGrpMan)
{
    AiqMapItem_t* pItem = NULL;
    bool rm             = false;
    AIQ_MAP_FOREACH(pCamGrpMan->mCamGroupResMap, pItem, rm) {
        rk_aiq_groupcam_result_t* pMsg = *((rk_aiq_groupcam_result_t**)pItem->_pData);
		clearGroupcamResults(pMsg);
        pItem          = aiqMap_erase_locked(pCamGrpMan->mCamGroupResMap, pItem->_key);
        aiq_free(pMsg);
        rm             = true;
    }
}

static void clearGroupCamResult_Locked(AiqCamGroupManager_t* pCamGrpMan, uint32_t frameId)
{
    if (frameId == (uint32_t)(-1)) {
        // clear all
        LOGD_CAMGROUP("clear all camgroup res");
		clearCamgrpResMap(pCamGrpMan);
    } else {
        rk_aiq_groupcam_result_t* camGroupRes = NULL;
		AiqMapItem_t* pItem = NULL;
		bool rm             = false;
		AIQ_MAP_FOREACH(pCamGrpMan->mCamGroupResMap, pItem, rm) {
			rk_aiq_groupcam_result_t* camGroupRes = *((rk_aiq_groupcam_result_t**)pItem->_pData);
			uint32_t keyId = (uint32_t)(long)pItem->_key;
			if (keyId <= frameId) {
                if (camGroupRes->_refCnt > 0) {
                    if (keyId < pCamGrpMan->mClearedResultId) {
                        LOGW("impossible, id:%u < mClearedResultId:%u, refCnt: %u",
                             keyId, pCamGrpMan->mClearedResultId, camGroupRes->_refCnt);
                    }
                    continue;
                }
                LOGD_CAMGROUP("clear camgroup res of frame: %u, ready: %d", keyId, camGroupRes->_ready);
				clearGroupcamResults(camGroupRes);
                if (keyId > pCamGrpMan->mClearedResultId)
                    pCamGrpMan->mClearedResultId = keyId;
				pItem = aiqMap_erase_locked(pCamGrpMan->mCamGroupResMap, pItem->_key);
                aiq_free(camGroupRes);
				rm             = true;
			}
		}
    }
}

static void AiqCamGroupManager_clearGroupCamResult(AiqCamGroupManager_t* pCamGrpMan, uint32_t frameId)
{
    aiqMutex_lock(&pCamGrpMan->mCamGroupResMutex);
    clearGroupCamResult_Locked(pCamGrpMan, frameId);
    aiqMutex_unlock(&pCamGrpMan->mCamGroupResMutex);
}

static void AiqCamGroupManager_putGroupCamResult(AiqCamGroupManager_t* pCamGrpMan, rk_aiq_groupcam_result_t* gc_res)
{
    aiqMutex_lock(&pCamGrpMan->mCamGroupResMutex);
    if (gc_res && gc_res->_refCnt != 0)
        gc_res->_refCnt--;
    aiqMutex_unlock(&pCamGrpMan->mCamGroupResMutex);
}

static void addDefaultAlgos(AiqCamGroupManager_t* pCamGrpMan, const struct RkAiqAlgoDesCommExt* algoDes)
{
    if (aiqMap_size(pCamGrpMan->mBindAiqsMap) == 0) {
        LOGD_CAMGROUP("no group cam, bypass");
        return ;
    }

	AiqMapItem_t* pItem = aiqMap_begin(pCamGrpMan->mBindAiqsMap);
    AiqManager_t* aiqManager = *(AiqManager_t**)pItem->_pData;
    AiqCore_t* aiqCore = aiqManager->mRkAiqAnalyzer;
    RkAiqAlgosComShared_t* sharedCom = &aiqCore->mAlogsComSharedParams;

    pCamGrpMan->mGroupAlgoCtxCfg.s_calibv2 = sharedCom->calibv2;
    pCamGrpMan->mGroupAlgoCtxCfg.pCamgroupCalib = pCamGrpMan->mCamgroupCalib;
    pCamGrpMan->mGroupAlgoCtxCfg.cfg_com.isp_hw_version = aiqCore->mIspHwVer;
    pCamGrpMan->mGroupAlgoCtxCfg.cfg_com.calibv2 =
        (CamCalibDbV2Context_t*)(sharedCom->calibv2);
    pCamGrpMan->mGroupAlgoCtxCfg.cfg_com.isGroupMode = true;

    pCamGrpMan->mGroupAlgoCtxCfg.camIdArrayLen = aiqMap_size(pCamGrpMan->mBindAiqsMap);

    int i = 0;
	bool rm             = false;
	AIQ_MAP_FOREACH(pCamGrpMan->mBindAiqsMap, pItem, rm) {
        pCamGrpMan->mGroupAlgoCtxCfg.camIdArray[i++] = (uint32_t)(long)pItem->_key;
	}

    for (i = 0; algoDes[i].des != NULL; i++) {
        AiqAlgoHandler_t* algo_map = aiqCore->mAlgoHandleMaps[algoDes[i].des->type];
        if (!algo_map) {
            LOGE_CAMGROUP("Adding group algo %s without single algo is not supported yet!",
                          AlgoTypeToString(algoDes[i].des->type));
            continue;
        }
        pCamGrpMan->mGroupAlgoCtxCfg.cfg_com.module_hw_version = algoDes[i].module_hw_ver;
        pCamGrpMan->mGroupAlgoCtxCfg.pSingleAlgoCtx = AiqAlgoHandler_getAlgoCtx(algo_map);

        AiqAlgoCamGroupHandler_t* grpHandle =
            createAlgoCamGroupHandlerInstance(algoDes[i].des->type, algoDes[i].des, pCamGrpMan, aiqCore);
		AiqAlgoCamGroupHandler_setAlgoHandle(grpHandle, algo_map);

        pCamGrpMan->mDefAlgoHandleMap[algoDes[i].des->type] = grpHandle;

        LOGD_CAMGROUP("camgroup: add algo: %d", algoDes[i].des->type);
    }
}

static XCamReturn syncSingleCamResultWithMaster(AiqCamGroupManager_t* pCamGrpMan, rk_aiq_groupcam_result_t* gc_res)
{
	// TODO
    return XCAM_RETURN_NO_ERROR;
}

static void calcHdrIso(AiqCamGroupManager_t* pCamGrpMan, RKAiqAecExpInfo_t* pCurExp, rk_aiq_singlecam_3a_result_t *singleCam3aRes)
{
	AiqMapItem_t* pItem = aiqMap_begin(pCamGrpMan->mBindAiqsMap);
    AiqManager_t* aiqManager = *(AiqManager_t**)pItem->_pData;
    AiqCore_t* aiqCore = aiqManager->mRkAiqAnalyzer;
    RkAiqAlgosComShared_t* sharedCom = &aiqCore->mAlogsComSharedParams;

    float arAGain = 1.0;
    float arDGain = 1.0;
    float arTime = 1.0;

    int working_mode = sharedCom->working_mode;
    if (working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        arAGain = pCurExp->LinearExp.exp_real_params.analog_gain;
        arDGain = pCurExp->LinearExp.exp_real_params.digital_gain;
        arTime = pCurExp->LinearExp.exp_real_params.integration_time;
        if (arAGain < 1.0) {
            arAGain = 1.0;
        }
        if (arDGain < 1.0) {
            arDGain = 1.0;
        }

        singleCam3aRes->hdrIso = arAGain * arDGain * 50;
    } else {
        int i = 0;
        if(working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
                || working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR)
            i = 1;
        else
            i = 2;

        arAGain = pCurExp->HdrExp[i].exp_real_params.analog_gain;
        arDGain = pCurExp->HdrExp[i].exp_real_params.digital_gain;
        arTime = pCurExp->HdrExp[i].exp_real_params.integration_time;

        if (arAGain < 1.0) {
            arAGain = 1.0;
        }
        if (arDGain < 1.0) {
            arDGain = 1.0;
        }

        singleCam3aRes->hdrIso = arAGain * arDGain * 50;
    }
}
    // folowing called by single RkAiq
void AiqCamGroupManager_processAiqCoreMsgs(AiqCamGroupManager_t* pCamGrpMan, AiqCore_t* src, AiqCoreMsg_t* msg)
{
    int camId = AiqCore_getCamPhyId(src);
    uint32_t frameId = msg->frame_id;
    // only support v3x now
    // check if the msg is required firstly
    if (!(pCamGrpMan->mRequiredMsgsMask & (1ULL << msg->msg_id))) {
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

    rk_aiq_groupcam_result_t* camGroupRes = getGroupCamResult(pCamGrpMan, frameId, true);
    if (!camGroupRes) {
        LOGW_CAMGROUP("camgroup: get cam result faild for msg_id:%d, camId: %d, msg_id:%d, frame: %d", msg->msg_id, camId, frameId);
        return;
    }
    rk_aiq_singlecam_result_status_t* singleCamStatus = &camGroupRes->_singleCamResultsStatus[camId];
    rk_aiq_singlecam_result_t* singleCamRes = &singleCamStatus->_singleCamResults;

    XCamVideoBuffer* buf = NULL;

    switch (msg->msg_id) {
    case XCAM_MESSAGE_AWB_STATS_OK:
		{
			singleCamRes->_mAwbStats = *(aiq_stats_base_t**)(msg->buf);
			AIQ_REF_BASE_REF(&singleCamRes->_mAwbStats->_ref_base);
			aiq_awb_stats_wrapper_t* wrap = (aiq_awb_stats_wrapper_t*)singleCamRes->_mAwbStats->_data;
			singleCamRes->_3aResults.awb.awb_stats_v39 =
				(awbStats_stats_priv_t*)(&wrap->awb_stats_v39);
		}
        break;
    case XCAM_MESSAGE_AEC_STATS_OK:
		{
			singleCamRes->_mAeStats = *(aiq_stats_base_t**)(msg->buf);
			AIQ_REF_BASE_REF(&singleCamRes->_mAeStats->_ref_base);
			aiq_ae_stats_wrapper_t* wrap = (aiq_ae_stats_wrapper_t*)singleCamRes->_mAeStats->_data;
			singleCamRes->_3aResults.aec.aec_stats_v25 =
				(RKAiqAecStatsV25_t*)(&wrap->aec_stats_v25);
		}
        break;
    case XCAM_MESSAGE_AE_PRE_RES_OK:
        singleCamRes->_3aResults.aec._aePreRes_c = *(AlgoRstShared_t**)(msg->buf);
		AIQ_REF_BASE_REF(&(singleCamRes->_3aResults.aec._aePreRes_c)->_ref_base);
        break;
    case XCAM_MESSAGE_SOF_INFO_OK: {
        AiqSofInfoWrapper_t* sofInfoMsg =
			(AiqSofInfoWrapper_t*)((*(AlgoRstShared_t**)(msg->buf))->_data);
        singleCamRes->_3aResults.aec._effAecExpInfo =
            sofInfoMsg->curExp->aecExpInfo;
        singleCamRes->_3aResults.aec._bEffAecExpValid = true;
        calcHdrIso(pCamGrpMan, &singleCamRes->_3aResults.aec._effAecExpInfo, &singleCamRes->_3aResults);
    }
    break;
    default:
        break;
    }

set_bits:
    // check if all requirements are satisfied, if so,
    // notify the reprocess procedure
    {
		aiqMutex_lock(&pCamGrpMan->mCamGroupResMutex);
        singleCamStatus->_validCoreMsgsBits |= ((uint64_t)1) << msg->msg_id;
        singleCamRes->_3aResults._camId = camId;
        singleCamRes->_3aResults._frameId = frameId;
		aiqMutex_unlock(&pCamGrpMan->mCamGroupResMutex);
    }
    LOGD_CAMGROUP("camgroup: got required core msg :%s of camId:%d, frameId: %u, ",
                  MessageType2Str[msg->msg_id], camId, frameId);
    setSingleCamStatusReady(pCamGrpMan, singleCamStatus, camGroupRes);
	AiqCamGroupManager_putGroupCamResult(pCamGrpMan, camGroupRes);
}

void AiqCamGroupManager_RelayAiqCoreResults(AiqCamGroupManager_t* pCamGrpMan, AiqCore_t* src, AiqFullParams_t* results)
{
    // only support v3x now
    int camId = AiqCore_getCamPhyId(src);
    uint32_t frame_id = -1;

    AiqFullParams_t* aiqParams = results;
	frame_id = aiqParams->_base.frame_id;

	rk_aiq_groupcam_result_t* camGroupRes = getGroupCamResult(pCamGrpMan, frame_id, true);
	if (!camGroupRes) {
		LOGW_CAMGROUP("cid:%d, fid:%d get cam result faild", camId, frame_id);
		return;
	}

	aiqMutex_lock(&pCamGrpMan->mCamGroupResMutex);
	rk_aiq_singlecam_result_status_t* singleCamStatus =
		&camGroupRes->_singleCamResultsStatus[camId];
	rk_aiq_singlecam_result_t* singleCamRes = &singleCamStatus->_singleCamResults;
	if (!singleCamRes->_fullIspParam) {
		singleCamRes->_fullIspParam = results;
		LOGD_CAMGROUP("init scam_aiqParams_proxy : %p for camId:%d, frameId: %u",
					  singleCamRes->_fullIspParam, camId, frame_id);
	}

	AIQ_REF_BASE_REF(&aiqParams->_base._ref_base);
	aiqMutex_unlock(&pCamGrpMan->mCamGroupResMutex);

#define SET_TO_CAMGROUP(lc, BC) \
    if (aiqParams->pParamsArray[RESULT_TYPE_##BC##_PARAM]) { \
        /* compose single aiq params to one */ \
        { \
			aiqMutex_lock(&pCamGrpMan->mCamGroupResMutex); \
            singleCamStatus->_validAlgoResBits |= ((uint64_t)1) << RESULT_TYPE_##BC##_PARAM; \
            singleCamRes->_3aResults._camId = camId; \
            singleCamRes->_3aResults._frameId = frame_id; \
			aiqMutex_unlock(&pCamGrpMan->mCamGroupResMutex); \
        } \
        LOG1_CAMGROUP("relay results: camId:%d, frameId:%u, type:%s, validAlgoResBits:0x%" PRIx64 " ", \
                      camId, frame_id, #BC, singleCamStatus->_validAlgoResBits ); \
		setSingleCamStatusReady(pCamGrpMan, singleCamStatus, camGroupRes); \
    } \

    SET_TO_CAMGROUP(Exposure, EXPOSURE);
    SET_TO_CAMGROUP(Focus, FOCUS);
    SET_TO_CAMGROUP(Aec, AEC);
    SET_TO_CAMGROUP(Hist, HIST);
    SET_TO_CAMGROUP(AwbGain, AWBGAIN);
    SET_TO_CAMGROUP(Dpcc, DPCC);
    SET_TO_CAMGROUP(Lsc, LSC);
    SET_TO_CAMGROUP(Lut3d, LUT3D);
    SET_TO_CAMGROUP(Hsv, HSV);
    SET_TO_CAMGROUP(Adegamma, ADEGAMMA);
    SET_TO_CAMGROUP(Wdr, WDR);
    SET_TO_CAMGROUP(Csm, CSM);
    SET_TO_CAMGROUP(Cgc, CGC);
    SET_TO_CAMGROUP(Conv422, CONV422);
    SET_TO_CAMGROUP(Yuvconv, YUVCONV);
    SET_TO_CAMGROUP(Cp, CP);
    SET_TO_CAMGROUP(Ie, IE);
    SET_TO_CAMGROUP(Cpsl, CPSL);
    // TODO: special for fec ?
    SET_TO_CAMGROUP(Fec, FEC);
    // ispv21 and ispv3x common
    SET_TO_CAMGROUP(Gic, GIC);
    SET_TO_CAMGROUP(Dehaze, DEHAZE);
    SET_TO_CAMGROUP(Histeq, HISTEQ);
#if RKAIQ_HAVE_ENHANCE
    SET_TO_CAMGROUP(Enh, ENH);
#endif
    SET_TO_CAMGROUP(Drc, DRC);
    SET_TO_CAMGROUP(Merge, MERGE);
#if RKAIQ_HAVE_RGBIR_REMOSAIC
    SET_TO_CAMGROUP(Rgbir, RGBIR);
#endif
    SET_TO_CAMGROUP(Blc, BLC);
    SET_TO_CAMGROUP(Awb, AWB);
    SET_TO_CAMGROUP(Baynr, RAWNR);
    SET_TO_CAMGROUP(Ynr, YNR);
    SET_TO_CAMGROUP(Cnr, UVNR);
    SET_TO_CAMGROUP(Af, AF);
    SET_TO_CAMGROUP(Gain, GAIN);
    SET_TO_CAMGROUP(Ccm, CCM);

#if RKAIQ_HAVE_CAC
    SET_TO_CAMGROUP(Cac, CAC);
#endif
#if RKAIQ_HAVE_LDC
    SET_TO_CAMGROUP(Ldc, LDC);
#endif
#if USE_NEWSTRUCT
    SET_TO_CAMGROUP(Sharp, SHARPEN);
#if defined(RKAIQ_HAVE_SHARP_V40)  || defined(RKAIQ_HAVE_SHARP_V41)
    SET_TO_CAMGROUP(Textest, TEXEST);
#endif
    SET_TO_CAMGROUP(Btnr, TNR);
    SET_TO_CAMGROUP(Dm, DEBAYER);
    SET_TO_CAMGROUP(Gamma, AGAMMA);
    SET_TO_CAMGROUP(Yme, MOTION);
    SET_TO_CAMGROUP(AeStats, AESTATS);
#endif
#if RKAIQ_HAVE_AIBNR
    SET_TO_CAMGROUP(Aibnr, AIBNR);
#endif
#if RKAIQ_HAVE_AIBNR
    SET_TO_CAMGROUP(Aiynr, AIYNR);
#endif
	AiqCamGroupManager_putGroupCamResult(pCamGrpMan, camGroupRes);
}

XCamReturn AiqCamGroupManager_setCamgroupCalib(AiqCamGroupManager_t* pCamGrpMan, CamCalibDbCamgroup_t* camgroup_calib)
{
    ENTER_CAMGROUP_FUNCTION();
    if (pCamGrpMan->mState == CAMGROUP_MANAGER_STARTED ||
            pCamGrpMan->mState == CAMGROUP_MANAGER_INVALID) {
        LOGE_CAMGROUP("wrong state %d\n", pCamGrpMan->mState);
        return XCAM_RETURN_ERROR_FAILED;
    }
    // update groupcalib
    pCamGrpMan->mCamgroupCalib = camgroup_calib;
    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCamGroupManager_construct(AiqCamGroupManager_t* pCamGrpMan)
{
    ENTER_CAMGROUP_FUNCTION();

    aiqMutex_init(&pCamGrpMan->mCamGroupResMutex);
    aiqMutex_init(&pCamGrpMan->mCamGroupApiSyncMutex);
    aiqMutex_init(&pCamGrpMan->_update_mutex);
	aiqCond_init(&pCamGrpMan->_update_done_cond);

	AiqMapConfig_t bindAiqMapCfg;
	bindAiqMapCfg._name          = "bindAiqMap";
	bindAiqMapCfg._key_type		= AIQ_MAP_KEY_TYPE_UINT32;
	bindAiqMapCfg._item_nums     = RK_AIQ_CAM_GROUP_MAX_CAMS;
	bindAiqMapCfg._item_size     = sizeof(AiqManager_t*);
	pCamGrpMan->mBindAiqsMap = aiqMap_init(&bindAiqMapCfg);
	if (!pCamGrpMan->mBindAiqsMap)
		LOGE_ANALYZER("init %s error", bindAiqMapCfg._name);

	AiqMapConfig_t grpResCfg;
	grpResCfg._name          = "grpResMap";
	grpResCfg._key_type		= AIQ_MAP_KEY_TYPE_UINT32;
	grpResCfg._item_nums     = RK_AIQ_CAM_GROUP_MAX_CAMS;
	grpResCfg._item_size     = sizeof(rk_aiq_groupcam_result_t*);
	pCamGrpMan->mCamGroupResMap = aiqMap_init(&grpResCfg);
	if (!pCamGrpMan->mCamGroupResMap)
		LOGE_ANALYZER("init %s error", grpResCfg._name);

    pCamGrpMan->mRequiredMsgsMask = 1ULL << XCAM_MESSAGE_SOF_INFO_OK;
#if RKAIQ_HAVE_AE_V1
    pCamGrpMan->mRequiredMsgsMask |= (1ULL << XCAM_MESSAGE_AEC_STATS_OK) | (1ULL << XCAM_MESSAGE_AE_PRE_RES_OK);
#endif
#if RKAIQ_HAVE_AWB
    pCamGrpMan->mRequiredMsgsMask |= (1ULL << XCAM_MESSAGE_AWB_STATS_OK);
#endif
#if RKAIQ_HAVE_BAYERTNR_V30 && !USE_NEWSTRUCT
    pCamGrpMan->mRequiredMsgsMask |= (1ULL << XCAM_MESSAGE_TNR_STATS_OK);
#endif

	AiqCamGroupReprocTh_init(&pCamGrpMan->mCamGroupReprocTh, "AiqCamGroupReprocTh", pCamGrpMan);

    pCamGrpMan->mGroupAlgosDesArray = g_camgroup_algos;
    pCamGrpMan->mState = CAMGROUP_MANAGER_INVALID;
    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

// called after single cam aiq init
XCamReturn AiqCamGroupManager_init(AiqCamGroupManager_t* pCamGrpMan)
{
    ENTER_CAMGROUP_FUNCTION();

    if (pCamGrpMan->mState != CAMGROUP_MANAGER_BINDED) {
        LOGE_CAMGROUP("wrong state %d\n", pCamGrpMan->mState);
        return XCAM_RETURN_ERROR_FAILED;
    }
	addDefaultAlgos(pCamGrpMan, pCamGrpMan->mGroupAlgosDesArray);
    pCamGrpMan->mState = CAMGROUP_MANAGER_INITED;
    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

// called only once
XCamReturn AiqCamGroupManager_deInit(AiqCamGroupManager_t* pCamGrpMan)
{
    ENTER_CAMGROUP_FUNCTION();

    if (pCamGrpMan->mState != CAMGROUP_MANAGER_UNBINDED) {
        LOGE_CAMGROUP("wrong state %d\n", pCamGrpMan->mState);
        return XCAM_RETURN_ERROR_FAILED;
    }

    pCamGrpMan->mState = CAMGROUP_MANAGER_INVALID;

	AiqCamGroupReprocTh_deinit(&pCamGrpMan->mCamGroupReprocTh);

	AiqCamGroupManager_clearGroupCamResult(pCamGrpMan, -1);

	for (int i = 0; i < RK_AIQ_ALGO_TYPE_MAX; i++) {
		if (pCamGrpMan->mDefAlgoHandleMap[i])
			destroyAlgoCamGroupHandler(pCamGrpMan->mDefAlgoHandleMap[i]);
	}

    aiqMutex_deInit(&pCamGrpMan->mCamGroupResMutex);
    aiqMutex_deInit(&pCamGrpMan->mCamGroupApiSyncMutex);
    aiqMutex_deInit(&pCamGrpMan->_update_mutex);
	aiqCond_deInit(&pCamGrpMan->_update_done_cond);
	if (pCamGrpMan->mBindAiqsMap)
		aiqMap_deinit(pCamGrpMan->mBindAiqsMap);
	if (pCamGrpMan->mCamGroupResMap)
		aiqMap_deinit(pCamGrpMan->mCamGroupResMap);

    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}
// start analyze thread
XCamReturn AiqCamGroupManager_start(AiqCamGroupManager_t* pCamGrpMan)
{
    ENTER_CAMGROUP_FUNCTION();
    if (pCamGrpMan->mState != CAMGROUP_MANAGER_PREPARED) {
        LOGE_CAMGROUP("wrong state %d\n", pCamGrpMan->mState);
        return XCAM_RETURN_ERROR_FAILED;
    }

	AiqCamGroupReprocTh_start(&pCamGrpMan->mCamGroupReprocTh);
    pCamGrpMan->mState = CAMGROUP_MANAGER_STARTED;
    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;

}
// stop analyze thread
XCamReturn AiqCamGroupManager_stop(AiqCamGroupManager_t* pCamGrpMan)
{
    ENTER_CAMGROUP_FUNCTION();
    if (pCamGrpMan->mState == CAMGROUP_MANAGER_INVALID) {
        LOGE_CAMGROUP("wrong state %d\n", pCamGrpMan->mState);
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (pCamGrpMan->mState == CAMGROUP_MANAGER_STARTED) {
        pCamGrpMan->mState = CAMGROUP_MANAGER_STOPED;
    }

	AiqCamGroupReprocTh_stop(&pCamGrpMan->mCamGroupReprocTh);
    AiqCamGroupManager_clearGroupCamResult(pCamGrpMan, -1);
    pCamGrpMan->mVicapReadyMask = 0;
    pCamGrpMan->mClearedResultId = 0;

    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}
// called before start(), get initial settings
XCamReturn AiqCamGroupManager_prepare(AiqCamGroupManager_t* pCamGrpMan)
{
    ENTER_CAMGROUP_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pCamGrpMan->mState != CAMGROUP_MANAGER_INITED &&
        pCamGrpMan->mState != CAMGROUP_MANAGER_BINDED &&
        pCamGrpMan->mState != CAMGROUP_MANAGER_STOPED) {
        LOGE_CAMGROUP("wrong state %d\n", pCamGrpMan->mState);
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (aiqMap_size(pCamGrpMan->mBindAiqsMap) == 0) {
        LOGD_CAMGROUP("no group cam, bypass");
        return XCAM_RETURN_NO_ERROR;
    }

    // assume all single cam runs same algos
	AiqMapItem_t* pItem = aiqMap_begin(pCamGrpMan->mBindAiqsMap);
    AiqManager_t* aiqManager = *(AiqManager_t**)pItem->_pData;
    AiqCore_t* aiqCore = aiqManager->mRkAiqAnalyzer;

    // reprocess initial params

    // TODO: should deal with the case of eanbled algos changed dynamically
    pCamGrpMan->mRequiredAlgoResMask = aiqCore->mAllReqAlgoResMask;

    rk_aiq_groupcam_result_t* camGroupRes = getGroupCamResult(pCamGrpMan, 0, true);

    LOGD_CAMGROUP("relay init params ...");
	bool rm             = false;
	AIQ_MAP_FOREACH(pCamGrpMan->mBindAiqsMap, pItem, rm) {
		aiqManager = *(AiqManager_t**)pItem->_pData;
		aiqCore = aiqManager->mRkAiqAnalyzer;
        camGroupRes->_singleCamResultsStatus[(uint32_t)(long)pItem->_key]._validCoreMsgsBits = pCamGrpMan->mRequiredMsgsMask;
        AiqCamGroupManager_RelayAiqCoreResults(pCamGrpMan, aiqCore, aiqCore->mAiqCurParams);
	}

    LOGD_CAMGROUP("prepare algos ...");

	for (int i = 0; i < RK_AIQ_ALGO_TYPE_MAX; i++) {
        AiqAlgoCamGroupHandler_t* curHdl = pCamGrpMan->mDefAlgoHandleMap[i];
        while (curHdl) {
            if (AiqAlgoCamGroupHandler_getEnable(curHdl)) {
                /* update user initial params */
                ret = curHdl->prepare(curHdl, aiqCore);
                RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d prepare failed",
						AiqAlgoCamGroupHandler_getAlgoType(curHdl));
            }
            curHdl = AiqAlgoCamGroupHandler_getNextHdl(curHdl);
        }
    }

    LOGD_CAMGROUP("reprocess init params ...");

    pCamGrpMan->mInit = true;

    ret = AiqCamGroupManager_reProcess(pCamGrpMan, camGroupRes);
    if (ret < 0) {
        LOGE_CAMGROUP("reProcess failed");
        goto failed;
    }
    pCamGrpMan->mInit = false;

    LOGD_CAMGROUP("send init params to hwi ...");
    AiqCamGroupManager_relayToHwi(pCamGrpMan, camGroupRes);

    LOGD_CAMGROUP("clear init params ...");
    // delete the processed result
	AiqCamGroupManager_putGroupCamResult(pCamGrpMan, camGroupRes);
    /* clear the aysnc results after stop */
    AiqCamGroupManager_clearGroupCamResult(pCamGrpMan, -1);

    LOGD_CAMGROUP("prepare done");

    pCamGrpMan->mState = CAMGROUP_MANAGER_PREPARED;
    return XCAM_RETURN_NO_ERROR;
    EXIT_CAMGROUP_FUNCTION();
failed:
	AiqCamGroupManager_putGroupCamResult(pCamGrpMan, camGroupRes);
    AiqCamGroupManager_clearGroupCamResult(pCamGrpMan, -1);
    return ret;
}

// if called, prepare should be re-called
XCamReturn AiqCamGroupManager_bind(AiqCamGroupManager_t* pCamGrpMan, AiqManager_t* ctx)
{
    ENTER_CAMGROUP_FUNCTION();

    if (pCamGrpMan->mState != CAMGROUP_MANAGER_INVALID &&
            pCamGrpMan->mState != CAMGROUP_MANAGER_BINDED) {
        LOGE_CAMGROUP("in error state %d", pCamGrpMan->mState);
        return XCAM_RETURN_ERROR_FAILED;
    }

    int camId = AiqManager_getCamPhyId(ctx);

    LOGD_CAMGROUP("camgroup: bind camId: %d %p to group", camId, ctx);

    AiqMapItem_t* pItem = aiqMap_get(pCamGrpMan->mBindAiqsMap, (void*)(uintptr_t)camId);

    if (pItem) {
        return XCAM_RETURN_NO_ERROR;
    } else {
		pItem = aiqMap_insert(pCamGrpMan->mBindAiqsMap, (void*)(uintptr_t)camId, &ctx);
		if (!pItem)
			return XCAM_RETURN_ERROR_FAILED;
        pCamGrpMan->mRequiredCamsResMask |= 1 << camId;
    }

    pCamGrpMan->mState = CAMGROUP_MANAGER_BINDED;

    LOGD_CAMGROUP("camgroup: binded cams mask: 0x%x", pCamGrpMan->mRequiredCamsResMask);

    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCamGroupManager_unbind(AiqCamGroupManager_t* pCamGrpMan, int camId)
{
    ENTER_CAMGROUP_FUNCTION();

    if (pCamGrpMan->mState == CAMGROUP_MANAGER_STARTED) {
        LOGE_CAMGROUP("in error state %d", pCamGrpMan->mState);
        return XCAM_RETURN_ERROR_FAILED;
    }

    LOGD_CAMGROUP("camgroup: unbind camId: %d from group", camId);


	aiqMap_erase(pCamGrpMan->mBindAiqsMap, (void*)(uintptr_t)camId);
	pCamGrpMan->mRequiredCamsResMask &= ~(1 << camId);
	if (aiqMap_size(pCamGrpMan->mBindAiqsMap) == 0)
		pCamGrpMan->mState = CAMGROUP_MANAGER_UNBINDED;

    LOGD_CAMGROUP("camgroup: binded cams mask: 0x%x", 	pCamGrpMan->mRequiredCamsResMask);

    EXIT_CAMGROUP_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCamGroupManager_addAlgo(AiqCamGroupManager_t* pCamGrpMan, RkAiqAlgoDesComm* algo)
{
	LOGE_CAMGROUP("Not implement !");
	return XCAM_RETURN_ERROR_FAILED;
}

XCamReturn AiqCamGroupManager_enableAlgo(AiqCamGroupManager_t* pCamGrpMan, int algoType, int id, bool enable)
{
    ENTER_ANALYZER_FUNCTION();
    // get default algotype handle, id should be 0
    AiqAlgoCamGroupHandler_t* first_algo_hdl  = pCamGrpMan->mDefAlgoHandleMap[algoType];
    AiqAlgoCamGroupHandler_t* target_algo_hdl = first_algo_hdl;
    if (!first_algo_hdl) {
        LOGE_ANALYZER("can't find current type %d algo", algoType);
        return XCAM_RETURN_ERROR_FAILED;
    }

    while (target_algo_hdl) {
        if (AiqAlgoCamGroupHandler_getAlgoId(target_algo_hdl) == id) break;
        target_algo_hdl = AiqAlgoCamGroupHandler_getNextHdl(target_algo_hdl);
    }

    if (!target_algo_hdl) {
        LOGE_ANALYZER("can't find type id <%d, %d> algo", algoType, id);
        return XCAM_RETURN_ERROR_FAILED;
    }

	AiqAlgoCamGroupHandler_setEnable(target_algo_hdl, enable);

    if (enable) {
        if (pCamGrpMan->mState >= CAMGROUP_MANAGER_PREPARED) {
			AiqMapItem_t* pItem = aiqMap_begin(pCamGrpMan->mBindAiqsMap);
			AiqManager_t* aiqManager = *(AiqManager_t**)pItem->_pData;
			AiqCore_t* aiqCore = aiqManager->mRkAiqAnalyzer;
			AiqAlgoCamGroupHandler_prepare(target_algo_hdl, aiqCore);
        }
    }

    int enable_cnt  = 0;
    target_algo_hdl = first_algo_hdl;
    while (target_algo_hdl) {
        if (AiqAlgoCamGroupHandler_getEnable(target_algo_hdl)) {
            enable_cnt++;
        }
        target_algo_hdl = AiqAlgoCamGroupHandler_getNextHdl(target_algo_hdl);
    }

    LOGI_ANALYZER("algo type %d enabled count :%d", algoType, enable_cnt);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCamGroupManager_rmAlgo(AiqCamGroupManager_t* pCamGrpMan, int algoType, int id)
{
	LOGE_CAMGROUP("Not implement !");
	return XCAM_RETURN_ERROR_FAILED;
}

bool AiqCamGroupManager_getAxlibStatus(AiqCamGroupManager_t* pCamGrpMan, int algoType, int id)
{
    AiqAlgoCamGroupHandler_t* pHdl = pCamGrpMan->mDefAlgoHandleMap[algoType];

    while (pHdl) {
        if (AiqAlgoCamGroupHandler_getAlgoId(pHdl) == id) break;
        pHdl = AiqAlgoCamGroupHandler_getNextHdl(pHdl);
    }

    if (!pHdl) {
        LOGW_ANALYZER("can't find type id <%d, %d> algo", algoType, id);
        return false;
    }

    return AiqAlgoCamGroupHandler_getEnable(pHdl);
}

RkAiqAlgoContext* AiqCamGroupManager_getEnabledAxlibCtx(AiqCamGroupManager_t* pCamGrpMan, const int algo_type)
{
    if (algo_type <= RK_AIQ_ALGO_TYPE_NONE ||
            algo_type >= RK_AIQ_ALGO_TYPE_MAX)
        return NULL;

    AiqAlgoCamGroupHandler_t *pHdl, *pNextHdl;
    pHdl = pNextHdl = pCamGrpMan->mDefAlgoHandleMap[algo_type];

    // get last
    while (pNextHdl) {
        pHdl     = pNextHdl;
        pNextHdl = AiqAlgoCamGroupHandler_getNextHdl(pHdl);
    }

    if (pHdl && AiqAlgoCamGroupHandler_getEnable(pHdl))
        return pHdl->mAlgoCtx;
    else
        return NULL;
}

RkAiqAlgoContext* AiqCamGroupManager_getAxlibCtx(AiqCamGroupManager_t* pCamGrpMan, const int algo_type, const int lib_id)
{
    AiqAlgoCamGroupHandler_t *pHdl = pCamGrpMan->mDefAlgoHandleMap[algo_type];

    while (pHdl) {
        if (AiqAlgoCamGroupHandler_getAlgoId(pHdl) == lib_id) break;
        pHdl = AiqAlgoCamGroupHandler_getNextHdl(pHdl);
    }

    if (!pHdl) {
        LOGE_ANALYZER("can't find type id <%d, %d> algo", algo_type, lib_id);
        return NULL;
    }

    return pHdl->mAlgoCtx;
}

AiqAlgoCamGroupHandler_t* AiqCamGroupManager_getAiqCamgroupHandle(AiqCamGroupManager_t* pCamGrpMan, const int algo_type, const int lib_id)
{
    if (algo_type <= RK_AIQ_ALGO_TYPE_NONE || algo_type >= RK_AIQ_ALGO_TYPE_MAX) return NULL;

	return pCamGrpMan->mDefAlgoHandleMap[algo_type];
}

XCamReturn AiqCamGroupManager_calibTuning(AiqCamGroupManager_t* pCamGrpMan, const CamCalibDbV2Context_t* aiqCalib)
{
    ENTER_ANALYZER_FUNCTION();
    if (!aiqCalib) {
        LOGE_ANALYZER("invalied tuning param\n");
        return XCAM_RETURN_ERROR_PARAM;
    }

	if (aiqMap_size(pCamGrpMan->mBindAiqsMap) == 0) {
        LOGD_CAMGROUP("no group cam, bypass");
        return XCAM_RETURN_NO_ERROR;
    }

    // assume all single cam runs same algos
	AiqMapItem_t* pItem = aiqMap_begin(pCamGrpMan->mBindAiqsMap);
	AiqManager_t* aiqManager = *(AiqManager_t**)pItem->_pData;
	AiqCore_t* aiqCore = aiqManager->mRkAiqAnalyzer;
    if (!aiqCore || !AiqCore_isRunningState(aiqCore)) {
        LOGE_ANALYZER("GroupCam not prepared, force update\n");
        pCamGrpMan->mCalibv2 = *aiqCalib;
        pCamGrpMan->mGroupAlgoCtxCfg.s_calibv2 = &pCamGrpMan->mCalibv2;
        pCamGrpMan->needReprepare = true;
    } else {
		aiqMutex_lock(&pCamGrpMan->_update_mutex);

        pCamGrpMan->mCalibv2 = *aiqCalib;
        pCamGrpMan->mGroupAlgoCtxCfg.s_calibv2 = &pCamGrpMan->mCalibv2;
        /* optimization the issue that first calibTuning don't take effect */
        aiqCore->mAlogsComSharedParams.conf_type = RK_AIQ_ALGO_CONFTYPE_UPDATECALIB;
        pCamGrpMan->needReprepare = true;

        while (pCamGrpMan->needReprepare == true) {
            aiqCond_timedWait(&pCamGrpMan->_update_done_cond, &pCamGrpMan->_update_mutex, 100000ULL);
        }
		aiqMutex_unlock(&pCamGrpMan->_update_mutex);
    }

    return XCAM_RETURN_NO_ERROR;

}

XCamReturn AiqCamGroupManager_updateCalibDb(AiqCamGroupManager_t* pCamGrpMan, const CamCalibDbV2Context_t* newCalibDb)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqCamGroupManager_calibTuning(pCamGrpMan, newCalibDb);

    EXIT_XCORE_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCamGroupManager_rePrepare(AiqCamGroupManager_t* pCamGrpMan)
{
    ENTER_CAMGROUP_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!pCamGrpMan->needReprepare) {
        return ret;
    }

    if (aiqMap_size(pCamGrpMan->mBindAiqsMap) == 0) {
        LOGD_CAMGROUP("no group cam, bypass");
        return XCAM_RETURN_NO_ERROR;
    }

    // assume all single cam runs same algos
	AiqMapItem_t* pItem = aiqMap_begin(pCamGrpMan->mBindAiqsMap);
    AiqManager_t* aiqManager = *(AiqManager_t**)pItem->_pData;
    AiqCore_t* aiqCore = aiqManager->mRkAiqAnalyzer;

	for (int i = 0; i < RK_AIQ_ALGO_TYPE_MAX; i++) {
        AiqAlgoCamGroupHandler_t* curHdl = pCamGrpMan->mDefAlgoHandleMap[i];
        while (curHdl) {
            if (AiqAlgoCamGroupHandler_getEnable(curHdl)) {
                ret = AiqAlgoCamGroupHandler_prepare(curHdl, aiqCore);
                RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d prepare failed",
						AiqAlgoCamGroupHandler_getAlgoType(curHdl));
            }
			curHdl = AiqAlgoCamGroupHandler_getNextHdl(curHdl);
        }
    }

    pCamGrpMan->needReprepare = false;
    aiqCond_broadcast(&pCamGrpMan->_update_done_cond);

    return ret;

}

void AiqCamGroupManager_setVicapReady(AiqCamGroupManager_t* pCamGrpMan, rk_aiq_hwevt_t* hwevt)
{
	aiqMutex_lock(&pCamGrpMan->mCamGroupApiSyncMutex);
    pCamGrpMan->mVicapReadyMask |= 1 << hwevt->cam_id;
	aiqMutex_unlock(&pCamGrpMan->mCamGroupApiSyncMutex);
}

bool AiqCamGroupManager_isAllVicapReady(AiqCamGroupManager_t* pCamGrpMan)
{
	aiqMutex_lock(&pCamGrpMan->mCamGroupApiSyncMutex);
	bool ret = (pCamGrpMan->mVicapReadyMask == pCamGrpMan->mRequiredCamsResMask) ? true : false;
	aiqMutex_unlock(&pCamGrpMan->mCamGroupApiSyncMutex);
    return ret;
}

XCamReturn AiqCamGroupManager_register3Aalgo(AiqCamGroupManager_t* pCamGrpMan, void* algoDes, void *cbs)
{
    int arraySize = aiqMap_size(pCamGrpMan->mBindAiqsMap);
	if (arraySize == 0) {
        LOGD_CAMGROUP("no group cam, bypass");
        return XCAM_RETURN_ERROR_FAILED;
    }
    struct RkAiqAlgoDesCommExt *algoDesArray = (struct RkAiqAlgoDesCommExt*)algoDes;

	AiqMapItem_t* pItem = aiqMap_begin(pCamGrpMan->mBindAiqsMap);
    AiqManager_t* aiqManager = *(AiqManager_t**)pItem->_pData;
    AiqCore_t* aiqCore = aiqManager->mRkAiqAnalyzer;
    RkAiqAlgosComShared_t* sharedCom = &aiqCore->mAlogsComSharedParams;

    if (pCamGrpMan->mState != CAMGROUP_MANAGER_STOPED &&
		pCamGrpMan->mState != CAMGROUP_MANAGER_INITED)
        return XCAM_RETURN_ERROR_ANALYZER;

    for (int i = 0; algoDesArray[i].des != NULL; i++) {
        RkAiqAlgoDesComm* algo_des = algoDesArray[i].des;
        pCamGrpMan->mGroupAlgoCtxCfg.cfg_com.module_hw_version = algoDesArray[i].module_hw_ver;
        pCamGrpMan->mGroupAlgoCtxCfg.cfg_com.cbs               = cbs;

        AiqAlgoHandler_t* algo_map = aiqCore->mAlgoHandleMaps[algo_des->type];
        if (!algo_map) {
            LOGE_CAMGROUP("Adding group algo %s without single algo is not supported yet!",
                          AlgoTypeToString(algo_des->type));
            continue;
        }
        pCamGrpMan->mGroupAlgoCtxCfg.pSingleAlgoCtx = AiqAlgoHandler_getAlgoCtx(algo_map);

        AiqAlgoCamGroupHandler_t* grpHandle =
            createAlgoCamGroupHandlerInstance(algo_des->type, algo_des, pCamGrpMan, aiqCore);
		AiqAlgoCamGroupHandler_setAlgoHandle(grpHandle, algo_map);
        pCamGrpMan->mDefAlgoHandleMap[algo_des->type] = grpHandle;

        pCamGrpMan->mGroupAlgoCtxCfg.cfg_com.cbs = NULL;
        LOGD_CAMGROUP("camgroup: add algo: %d", algo_des->type);
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCamGroupManager_unregister3Aalgo(AiqCamGroupManager_t* pCamGrpMan, int algoType)
{
    ENTER_ANALYZER_FUNCTION();

    if (pCamGrpMan->mState != CAMGROUP_MANAGER_STOPED &&
		pCamGrpMan->mState != CAMGROUP_MANAGER_INITED) {
        return XCAM_RETURN_ERROR_ANALYZER;
	}

	if (pCamGrpMan->mDefAlgoHandleMap[algoType]) {
		destroyAlgoCamGroupHandler(pCamGrpMan->mDefAlgoHandleMap[algoType]);
		pCamGrpMan->mDefAlgoHandleMap[algoType] = NULL;
	}

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}
