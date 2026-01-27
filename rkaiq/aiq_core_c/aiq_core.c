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

#include "aiq_core.h"
#include "aiq_core_c/aiq_algo_handler.h"
#include "xcore_c/aiq_v4l2_buffer.h"
#include "hwi_c/aiq_CamHwBase.h"
#include "hwi_c/aiq_sensorHw.h"
#include "algos/af/rk_aiq_types_af_algo_int.h"
#if RKAIQ_HAVE_DUMPSYS
#include "dumpcam_server/third-party/argparse/argparse.h"
#include "info/aiq_coreBufMgrInfo.h"
#include "info/aiq_coreInfo.h"
#include "info/aiq_coreIspModsInfo.h"
#include "info/aiq_coreIspParamsInfo.h"
#include "rk_info_utils.h"
#include "st_string.h"
#endif

#define EPSINON 0.0000001

#define grpId2GrpMask(grpId) \
    ((grpId) == RK_AIQ_CORE_ANALYZE_ALL ? (uint64_t)(grpId) : (1ULL << (grpId)))

static XCamReturn fixAiqParamsIsp(AiqCore_t* pAiqCore, AiqFullParams_t* aiqParams);
#if RKAIQ_HAVE_DUMPSYS
static void AiqCore_initNotifier(AiqCore_t* pAiqCore);
static int AiqCore_dump(void* self, st_string* result, int argc, void* argv[]);
static int AiqCore_algosDump(void* self, st_string* result, int argc, void* argv[]);
#endif

bool AiqCore_isGroupAlgo(AiqCore_t* pAiqCore, int algoType) {
#ifdef RKAIQ_ENABLE_CAMGROUP
    return (pAiqCore->mCamGroupCoreManager &&
            pAiqCore->mCamGroupCoreManager->mDefAlgoHandleMap[algoType] ? true : false);
#else
    return false;
#endif
}

static bool isNewStructAlgo(int algoType) {
    bool ret = false;
#if USE_NEWSTRUCT
    switch (algoType) {
    case RK_AIQ_ALGO_TYPE_ADEBAYER:
    case RK_AIQ_ALGO_TYPE_AMFNR:
        ret = true;
        break;
    }
#endif
    return ret;
}

#define AIQCORE_POOL_TYPE_STATS  0
#define AIQCORE_POOL_TYPE_PARAMS 1
#define AIQCORE_POOL_TYPE_SHARED 2

AiqPool_t* AiqCore_createPool(const char* name, int num, int itemSize, int type) {
    AiqPoolConfig_t poolCfg;
    AiqPoolItem_t* pItem = NULL;
    int i                = 0;
    AiqPool_t* ret       = NULL;
    poolCfg._name        = name;
    poolCfg._item_nums   = num;
    poolCfg._item_size   = itemSize;
    ret                  = aiqPool_init(&poolCfg);
    if (!ret) {
        LOGE_ANALYZER("init %s error", poolCfg._name);
        return NULL;
    }
    AIQ_POOL_FOREACH(ret, pItem) {
        if (type == AIQCORE_POOL_TYPE_STATS) {
            aiq_stats_base_t* pBase = (aiq_stats_base_t*)(pItem->_pData);
            AIQ_REF_BASE_INIT(&pBase->_ref_base, pItem, aiqPoolItem_ref, aiqPoolItem_unref);
            SET_HEAD_DATA_PTR(pBase);
            LOG1("pool:%s, %p", poolCfg._name, pBase->_data);
        } else if (type == AIQCORE_POOL_TYPE_PARAMS) {
            aiq_params_base_t* pBase = (aiq_params_base_t*)(pItem->_pData);
            AIQ_PARAMS_BASE_INIT(pBase);
            AIQ_REF_BASE_INIT(&pBase->_ref_base, pItem, aiqPoolItem_ref, aiqPoolItem_unref);
            SET_HEAD_DATA_PTR(pBase);
            LOG1("pool:%s, %p", poolCfg._name, pBase->_data);
        } else if (type == AIQCORE_POOL_TYPE_SHARED) {
            AlgoRstShared_t* pBase = (AlgoRstShared_t*)(pItem->_pData);
            AIQ_REF_BASE_INIT(&pBase->_ref_base, pItem, aiqPoolItem_ref, aiqPoolItem_unref);
            SET_HEAD_DATA_PTR(pBase);
            LOG1("pool:%s, %p", poolCfg._name, pBase->_data);
        }
    }
    return ret;
}

static XCamReturn newAiqParamPool(AiqCore_t* pAiqCore, int type) {

    uint16_t DEFAULT_POOL_SIZE = pAiqCore->mDefaultDelayCnt + 1;

#define NEW_PARAMS_POOL(RESULT, PTYPE)                                                     \
    do {                                                                                  \
        if (!pAiqCore->mAiqParamsPoolArray[RESULT]) {                                     \
            pAiqCore->mAiqParamsPoolArray[RESULT] = AiqCore_createPool(                   \
                #RESULT, DEFAULT_POOL_SIZE, CALC_SIZE_WITH_HEAD(aiq_params_base_t, PTYPE), \
                AIQCORE_POOL_TYPE_PARAMS);                                                \
            if (!pAiqCore->mAiqParamsPoolArray[RESULT]) {                                 \
                LOGE_ANALYZER("init %s pool failed", #RESULT);                            \
                return XCAM_RETURN_ERROR_FAILED;                                          \
            }                                                                             \
        }                                                                                 \
    } while (0)

    switch (type) {
    case RK_AIQ_ALGO_TYPE_AE:
        if (!pAiqCore->mAiqAecStatsPool) {
            pAiqCore->mAiqAecStatsPool = AiqCore_createPool(
                                             "AeStats", DEFAULT_POOL_SIZE,
                                             CALC_SIZE_WITH_HEAD(aiq_stats_base_t, aiq_ae_stats_wrapper_t),
                                             AIQCORE_POOL_TYPE_STATS);
            if (!pAiqCore->mAiqAecStatsPool) {
                LOGE_ANALYZER("init aecstas pool failed");
                return XCAM_RETURN_ERROR_FAILED;
            }
        }

        if (!pAiqCore->mPreResAeSharedPool) {
            pAiqCore->mPreResAeSharedPool =
                AiqCore_createPool("AePreRes", DEFAULT_POOL_SIZE,
                                   CALC_SIZE_WITH_HEAD(AlgoRstShared_t, RkAiqAlgoPreResAe),
                                   AIQCORE_POOL_TYPE_SHARED);
            if (!pAiqCore->mPreResAeSharedPool) {
                LOGE_ANALYZER("creat AePreRes failed !");
                return XCAM_RETURN_ERROR_FAILED;
            }
        }
#if USE_NEWSTRUCT
        NEW_PARAMS_POOL(RESULT_TYPE_AESTATS_PARAM, rk_aiq_isp_ae_stats_cfg_t);
#else
        NEW_PARAMS_POOL(RESULT_TYPE_AEC_PARAM, rk_aiq_isp_aec_meas_t);
        NEW_PARAMS_POOL(RESULT_TYPE_HIST_PARAM, rk_aiq_isp_hist_meas_t);
#endif
        NEW_PARAMS_POOL(RESULT_TYPE_EXPOSURE_PARAM, AiqAecExpInfoWrapper_t);
        NEW_PARAMS_POOL(RESULT_TYPE_IRIS_PARAM, AiqIrisInfoWrapper_t);
        break;
    case RK_AIQ_ALGO_TYPE_AWB:
        if (!pAiqCore->mAiqAwbStatsPool) {
            pAiqCore->mAiqAwbStatsPool = AiqCore_createPool(
                                             "AwbStats", DEFAULT_POOL_SIZE,
                                             CALC_SIZE_WITH_HEAD(aiq_stats_base_t, aiq_awb_stats_wrapper_t),
                                             AIQCORE_POOL_TYPE_STATS);
            if (!pAiqCore->mAiqAwbStatsPool) {
                LOGE_ANALYZER("init AwbStats pool failed");
                return XCAM_RETURN_ERROR_FAILED;
            }
        }

        if (!pAiqCore->mProcResAwbSharedPool) {
            pAiqCore->mProcResAwbSharedPool = AiqCore_createPool(
                                                  "AwbProcRes", DEFAULT_POOL_SIZE,
                                                  CALC_SIZE_WITH_HEAD(AlgoRstShared_t, RkAiqAlgoProcResAwbShared_t),
                                                  AIQCORE_POOL_TYPE_SHARED);
            if (!pAiqCore->mProcResAwbSharedPool) {
                LOGE_ANALYZER("creat AwbProcRes failed !");
                return XCAM_RETURN_ERROR_FAILED;
            }
        }
        NEW_PARAMS_POOL(RESULT_TYPE_AWB_PARAM, rk_aiq_isp_awb_params_t);
        NEW_PARAMS_POOL(RESULT_TYPE_AWBGAIN_PARAM, rk_aiq_isp_awb_gain_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_AF:
        if (!pAiqCore->mAiqAfStatsPool) {
            pAiqCore->mAiqAfStatsPool = AiqCore_createPool(
                                            "AfStats", DEFAULT_POOL_SIZE,
                                            CALC_SIZE_WITH_HEAD(aiq_stats_base_t, aiq_af_stats_wrapper_t),
                                            AIQCORE_POOL_TYPE_STATS);
            if (!pAiqCore->mAiqAfStatsPool) {
                LOGE_ANALYZER("init AfStats pool failed");
                return XCAM_RETURN_ERROR_FAILED;
            }
        }

#if RKAIQ_HAVE_PDAF
        if (!pAiqCore->mAiqPdafStatsPool) {
            pAiqCore->mAiqPdafStatsPool = AiqCore_createPool(
                                              "pdafStats", DEFAULT_POOL_SIZE,
                                              CALC_SIZE_WITH_HEAD(aiq_stats_base_t, rk_aiq_isp_pdaf_stats_t),
                                              AIQCORE_POOL_TYPE_STATS);
            if (!pAiqCore->mAiqPdafStatsPool) {
                LOGE_ANALYZER("init pdaf pool failed");
                return XCAM_RETURN_ERROR_FAILED;
            }
        }
#endif
        NEW_PARAMS_POOL(RESULT_TYPE_FOCUS_PARAM, rk_aiq_focus_params_t);
        NEW_PARAMS_POOL(RESULT_TYPE_AF_PARAM, rk_aiq_isp_af_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_ADPCC:
        NEW_PARAMS_POOL(RESULT_TYPE_DPCC_PARAM, rk_aiq_isp_dpcc_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_AMERGE:
        NEW_PARAMS_POOL(RESULT_TYPE_MERGE_PARAM, rk_aiq_isp_merge_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_ACCM:
        NEW_PARAMS_POOL(RESULT_TYPE_CCM_PARAM, rk_aiq_isp_ccm_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_ALSC:
        NEW_PARAMS_POOL(RESULT_TYPE_LSC_PARAM, rk_aiq_isp_lsc_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_ABLC:
        NEW_PARAMS_POOL(RESULT_TYPE_BLC_PARAM, rk_aiq_isp_blc_params_t);
        if (!pAiqCore->mProcResBlcSharedPool) {
            pAiqCore->mProcResBlcSharedPool =
                AiqCore_createPool("BlcProcRes", DEFAULT_POOL_SIZE,
                                   CALC_SIZE_WITH_HEAD(AlgoRstShared_t, RkAiqAlgoProcResAblc),
                                   AIQCORE_POOL_TYPE_SHARED);
            if (!pAiqCore->mProcResBlcSharedPool) {
                LOGE_ANALYZER("creat BlcProcRes failed !");
                return XCAM_RETURN_ERROR_FAILED;
            }
        }
        break;
    case RK_AIQ_ALGO_TYPE_ARAWNR:
        //NEW_PARAMS_POOL(RESULT_TYPE_RAWNR_PARAM, rk_aiq_isp_baynr_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_AGIC:
        NEW_PARAMS_POOL(RESULT_TYPE_GIC_PARAM, rk_aiq_isp_gic_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_ADEBAYER:
#if USE_NEWSTRUCT
        NEW_PARAMS_POOL(RESULT_TYPE_DEBAYER_PARAM, rk_aiq_isp_dm_params_t);
#else
        NEW_PARAMS_POOL(RESULT_TYPE_DEBAYER_PARAM, rk_aiq_isp_debayer_params_t);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_A3DLUT:
#if RKAIQ_HAVE_3DLUT
        NEW_PARAMS_POOL(RESULT_TYPE_LUT3D_PARAM, rk_aiq_isp_lut3d_params_t);
#endif
        break;
#if RKAIQ_HAVE_DEHAZE
    case RK_AIQ_ALGO_TYPE_ADHAZ:
        if (!pAiqCore->mAiqAdehazeStatsPool) {
            pAiqCore->mAiqAdehazeStatsPool =
                AiqCore_createPool("DhazeStats", DEFAULT_POOL_SIZE,
                                   CALC_SIZE_WITH_HEAD(aiq_stats_base_t, rkisp_adehaze_stats_t),
                                   AIQCORE_POOL_TYPE_STATS);
            if (!pAiqCore->mAiqAdehazeStatsPool) {
                LOGE_ANALYZER("init DhazeStats pool failed");
                return XCAM_RETURN_ERROR_FAILED;
            }
        }
        NEW_PARAMS_POOL(RESULT_TYPE_DEHAZE_PARAM, rk_aiq_isp_dehaze_params_t);
        break;
#endif
    case RK_AIQ_ALGO_TYPE_AHISTEQ:
        NEW_PARAMS_POOL(RESULT_TYPE_HISTEQ_PARAM, rk_aiq_isp_histeq_params_t);
        break;
#if RKAIQ_HAVE_ENHANCE
    case RK_AIQ_ALGO_TYPE_AENH:
        NEW_PARAMS_POOL(RESULT_TYPE_ENH_PARAM, rk_aiq_isp_enh_params_t);
        break;
#endif
#if RKAIQ_HAVE_HSV
    case RK_AIQ_ALGO_TYPE_AHSV:
        NEW_PARAMS_POOL(RESULT_TYPE_HSV_PARAM, rk_aiq_isp_hsv_params_t);
        break;
#endif
    case RK_AIQ_ALGO_TYPE_AGAMMA:
#if USE_NEWSTRUCT
        NEW_PARAMS_POOL(RESULT_TYPE_AGAMMA_PARAM, rk_aiq_isp_gamma_params_t);
#else
        NEW_PARAMS_POOL(RESULT_TYPE_AGAMMA_PARAM, rk_aiq_isp_agamma_params_t);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_ADEGAMMA:
        NEW_PARAMS_POOL(RESULT_TYPE_ADEGAMMA_PARAM, rk_aiq_isp_adegamma_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_AWDR:
        NEW_PARAMS_POOL(RESULT_TYPE_WDR_PARAM, rk_aiq_isp_wdr_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_ACSM:
        NEW_PARAMS_POOL(RESULT_TYPE_CSM_PARAM, rk_aiq_isp_csm_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_ACGC:
        NEW_PARAMS_POOL(RESULT_TYPE_CGC_PARAM, rk_aiq_isp_cgc_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_AGAIN:
#if RK_GAIN_V2_ENABLE_GAIN2DDR && defined(ISP_HW_V32)
        if (pAiqCore->mAiqAgainStatsPool) {
            pAiqCore->mAiqAgainStatsPool =
                AiqCore_createPool("GainStats", DEFAULT_POOL_SIZE,
                                   CALC_SIZE_WITH_HEAD(aiq_stats_base_t, rk_aiq_again_stat_t),
                                   AIQCORE_POOL_TYPE_STATS);
            if (!pAiqCore->mAiqAgainStatsPool) {
                LOGE_ANALYZER("init GainStats pool failed");
                return XCAM_RETURN_ERROR_FAILED;
            }
        }
#endif
        NEW_PARAMS_POOL(RESULT_TYPE_GAIN_PARAM, rk_aiq_isp_gain_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_ACP:
        NEW_PARAMS_POOL(RESULT_TYPE_CP_PARAM, rk_aiq_isp_cp_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_AIE:
        NEW_PARAMS_POOL(RESULT_TYPE_IE_PARAM, rk_aiq_isp_ie_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_AMD:
#if RKAIQ_HAVE_AMD_V1
        NEW_PARAMS_POOL(RESULT_TYPE_MOTION_PARAM, rk_aiq_isp_md_params_t);
#elif RKAIQ_HAVE_YUVME_V1
        NEW_PARAMS_POOL(RESULT_TYPE_MOTION_PARAM, rk_aiq_isp_yme_params_t);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_AMFNR:
#if USE_NEWSTRUCT
        NEW_PARAMS_POOL(RESULT_TYPE_TNR_PARAM, rk_aiq_isp_btnr_params_t);
#else
        NEW_PARAMS_POOL(RESULT_TYPE_TNR_PARAM, rk_aiq_isp_tnr_params_t);

#if defined(RKAIQ_HAVE_BAYERTNR_V30)
        if (pAiqCore->mbay3dStatPool) {
            pAiqCore->mbay3dStatPool =
                AiqCore_createPool("bay3dstats", DEFAULT_POOL_SIZE,
                                   CALC_SIZE_WITH_HEAD(aiq_stats_base_t, rk_aiq_bay3d_stat_t),
                                   AIQCORE_POOL_TYPE_STATS);
            if (!pAiqCore->mbay3dStatPool) {
                LOGE_ANALYZER("init bay3dstats pool failed");
                return XCAM_RETURN_ERROR_FAILED;
            }
        }
        if (!pAiqCore->bay3dStatList) {
            AiqListConfig_t bay3dStatListCfg;
            bay3dStatListCfg._name      = "bay3dStat";
            bay3dStatListCfg._item_nums = 6;
            bay3dStatListCfg._item_size = sizeof(aiq_stats_base_t*);
            pAiqCore->bay3dStatList     = aiqList_init(&bay3dStatListCfg);
            if (!pAiqCore->bay3dStatList)
                LOGE_ANALYZER("init %s error", cid, bay3dStatListCfg._name);
        }
#endif
#endif
        break;
    case RK_AIQ_ALGO_TYPE_AYNR:
        NEW_PARAMS_POOL(RESULT_TYPE_YNR_PARAM, rk_aiq_isp_ynr_params_t);
        if (!pAiqCore->mProcResYnrSharedPool) {
            pAiqCore->mProcResYnrSharedPool =
                AiqCore_createPool("YnrProcRes", DEFAULT_POOL_SIZE,
                                   CALC_SIZE_WITH_HEAD(AlgoRstShared_t, RkAiqAlgoProcResYnr),
                                   AIQCORE_POOL_TYPE_SHARED);
            if (!pAiqCore->mProcResYnrSharedPool) {
                LOGE_ANALYZER("creat YnrProcRes failed !");
                return XCAM_RETURN_ERROR_FAILED;
            }
        }
        break;
    case RK_AIQ_ALGO_TYPE_ACNR:
        NEW_PARAMS_POOL(RESULT_TYPE_UVNR_PARAM, rk_aiq_isp_cnr_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_ASHARP:
#if USE_NEWSTRUCT
        NEW_PARAMS_POOL(RESULT_TYPE_SHARPEN_PARAM, rk_aiq_isp_sharp_params_t);
#else
        NEW_PARAMS_POOL(RESULT_TYPE_SHARPEN_PARAM, rk_aiq_isp_sharpen_params_t);
#endif

#if defined(RKAIQ_HAVE_SHARP_V40) || defined(RKAIQ_HAVE_SHARP_V41)
        NEW_PARAMS_POOL(RESULT_TYPE_TEXEST_PARAM, rk_aiq_isp_texEst_params_t);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_AFEC:
    case RK_AIQ_ALGO_TYPE_AEIS:
        NEW_PARAMS_POOL(RESULT_TYPE_FEC_PARAM, rk_aiq_isp_fec_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_ACAC:
#if RKAIQ_HAVE_CAC
        NEW_PARAMS_POOL(RESULT_TYPE_CAC_PARAM, rk_aiq_isp_cac_params_t);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_ADRC:
        NEW_PARAMS_POOL(RESULT_TYPE_DRC_PARAM, rk_aiq_isp_drc_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_AFD:
        NEW_PARAMS_POOL(RESULT_TYPE_AFD_PARAM, rk_aiq_isp_afd_params_t);
        break;
    case RK_AIQ_ALGO_TYPE_ARGBIR:
#if RKAIQ_HAVE_RGBIR_REMOSAIC
        NEW_PARAMS_POOL(RESULT_TYPE_RGBIR_PARAM, rk_aiq_isp_rgbir_params_t);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_ALDC:
#if RKAIQ_HAVE_LDC
        NEW_PARAMS_POOL(RESULT_TYPE_LDC_PARAM, rk_aiq_isp_ldc_params_t);
#endif
        break;
#if RKAIQ_HAVE_AIBNR
    case RK_AIQ_ALGO_TYPE_AIBNR:
        NEW_PARAMS_POOL(RESULT_TYPE_AIBNR_PARAM, rk_aiq_isp_aibnr_params_t);
        break;
#endif
#if RKAIQ_HAVE_AIRMS
    case RK_AIQ_ALGO_TYPE_AIRMS:
        NEW_PARAMS_POOL(RESULT_TYPE_AIRMS_PARAM, rk_aiq_isp_airms_params_t);
        break;
#endif
#if RKAIQ_HAVE_AIYNR
    case RK_AIQ_ALGO_TYPE_AIYNR:
        NEW_PARAMS_POOL(RESULT_TYPE_AIYNR_PARAM, rk_aiq_isp_aiynr_params_t);
        break;
#endif
    default:
        break;
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn newAllAiqParamPool(AiqCore_t* pAiqCore) {
    int i          = 0;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    while (i < RK_AIQ_ALGO_TYPE_MAX) {
        bool alloc_pool          = false;
        AiqAlgoHandler_t* curHdl = pAiqCore->mAlgoHandleMaps[i];
        while (curHdl) {
            if (AiqAlgoHandler_getEnable(curHdl)) {
                alloc_pool = true;
                break;
            }
            curHdl = AiqAlgoHandler_getNextHdl(curHdl);
        }
        if (alloc_pool) {
            ret |= newAiqParamPool(pAiqCore, i);
        }
        i++;
    }

    return ret;
}

static XCamReturn deleteAllAiqParamPool(AiqCore_t* pAiqCore) {

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pAiqCore->mAiqAecStatsPool) {
        aiqPool_deinit(pAiqCore->mAiqAecStatsPool);
        pAiqCore->mAiqAecStatsPool = NULL;
    }

    if (pAiqCore->mAiqAwbStatsPool) {
        aiqPool_deinit(pAiqCore->mAiqAwbStatsPool);
        pAiqCore->mAiqAwbStatsPool = NULL;
    }

    if (pAiqCore->mAiqAdehazeStatsPool) {
        aiqPool_deinit(pAiqCore->mAiqAdehazeStatsPool);
        pAiqCore->mAiqAdehazeStatsPool = NULL;
    }

    if (pAiqCore->mAiqAgainStatsPool) {
        aiqPool_deinit(pAiqCore->mAiqAgainStatsPool);
        pAiqCore->mAiqAgainStatsPool = NULL;
    }

    if (pAiqCore->mAiqAfStatsPool) {
        aiqPool_deinit(pAiqCore->mAiqAfStatsPool);
        pAiqCore->mAiqAfStatsPool = NULL;
    }

#if RKAIQ_HAVE_PDAF
    if (pAiqCore->mAiqPdafStatsPool) {
        aiqPool_deinit(pAiqCore->mAiqPdafStatsPool);
        pAiqCore->mAiqPdafStatsPool = NULL;
    }
#endif

    for (int i = 0; i < RESULT_TYPE_MAX_PARAM; i++) {
        if (pAiqCore->mAiqParamsPoolArray[i])
            aiqPool_deinit(pAiqCore->mAiqParamsPoolArray[i]);
        pAiqCore->mAiqParamsPoolArray[i] = NULL;
    }

    if (pAiqCore->bay3dStatList) {
        aiqList_deinit(pAiqCore->bay3dStatList);
        pAiqCore->bay3dStatList = NULL;
    }

    if (pAiqCore->mbay3dStatPool) {
        aiqPool_deinit(pAiqCore->mbay3dStatPool);
        pAiqCore->mbay3dStatPool = NULL;
    }

    if (pAiqCore->mPreResAeSharedPool) {
        aiqPool_deinit(pAiqCore->mPreResAeSharedPool);
        pAiqCore->mPreResAeSharedPool = NULL;
    }

    if (pAiqCore->mProcResAwbSharedPool) {
        aiqPool_deinit(pAiqCore->mProcResAwbSharedPool);
        pAiqCore->mProcResAwbSharedPool = NULL;
    }

    if (pAiqCore->mProcResBlcSharedPool) {
        aiqPool_deinit(pAiqCore->mProcResBlcSharedPool);
        pAiqCore->mProcResBlcSharedPool = NULL;
    }

    if (pAiqCore->mProcResYnrSharedPool) {
        aiqPool_deinit(pAiqCore->mProcResYnrSharedPool);
        pAiqCore->mProcResYnrSharedPool = NULL;
    }

    return ret;
}

AiqAlgoHandler_t* newAlgoHandle(AiqCore_t* pAiqCore, RkAiqAlgoDesComm* algo, int hw_ver,
                                int handle_ver) {
    return createAlgoHandlerInstance(algo->type, algo, pAiqCore);
}

XCamReturn AiqCore_addAlgo(AiqCore_t* pAiqCore, RkAiqAlgoDesComm* algo) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_t *pHdl, *pNextHdl;
    pHdl = pNextHdl = pAiqCore->mAlgoHandleMaps[algo->type];

    // get last
    while (pNextHdl) {
        pHdl     = pNextHdl;
        pNextHdl = AiqAlgoHandler_getNextHdl(pHdl);
    }

    if (!pHdl) {
        algo->id = 0;
    } else {
        algo->id = AiqAlgoHandler_getAlgoId(pHdl) + 1;
    }

    AiqAlgoHandler_t* new_hdl = NULL;
#if 0 // TODO, ZYC
    if (algo.type == RK_AIQ_ALGO_TYPE_AE) {
        new_hdl = new RkAiqCustomAeHandle(&algo, this);
        AiqAlgoHandler_setGroupId(new_hdl, RK_AIQ_CORE_ANALYZE_AE);
        AiqAlgoHandler_setGroupShared(new_hdl, mAlogsGroupSharedParamsMap[RK_AIQ_CORE_ANALYZE_AE]);
    }
#if RKAIQ_HAVE_AWB_V21 || RKAIQ_HAVE_AWB_V32
    else if (algo.type == RK_AIQ_ALGO_TYPE_AWB) {
        new_hdl = new RkAiqCustomAwbHandle(&algo, this);
        AiqAlgoHandler_setGroupId(new_hdl, RK_AIQ_CORE_ANALYZE_AWB);
        AiqAlgoHandler_setGroupShared(new_hdl, mAlogsGroupSharedParamsMap[RK_AIQ_CORE_ANALYZE_AWB]);
    }
#endif
    else {
        LOGE_ANALYZER("not supported custom algo type: %d ", algo.type);
        return XCAM_RETURN_ERROR_FAILED;
    }
#endif
    AiqAlgoHandler_setEnable(new_hdl, false);
    if (pHdl) {
        AiqAlgoHandler_setNextHdl(pHdl, new_hdl);
        AiqAlgoHandler_setParentHdl(new_hdl, pHdl);
    } else {
        pAiqCore->mAlgoHandleMaps[algo->type] = new_hdl;
    }

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCore_rmAlgo(AiqCore_t* pAiqCore, int algoType, int id) {
    ENTER_ANALYZER_FUNCTION();

    // can't remove default algos
    if (id == 0) return XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = pAiqCore->mAlgoHandleMaps[algoType];

    while (pHdl) {
        if (AiqAlgoHandler_getAlgoId(pHdl) == id) break;
        pHdl = AiqAlgoHandler_getNextHdl(pHdl);
    }

    if (!pHdl) {
        LOGE_ANALYZER("can't find type id <%d, %d> algo", algoType, id);
        return XCAM_RETURN_ERROR_FAILED;
    }

    aiqMutex_lock(&pAiqCore->mApiMutex);
    while (pAiqCore->mState == RK_AIQ_CORE_STATE_RUNNING && pAiqCore->mSafeEnableAlgo != true)
        aiqCond_wait(&pAiqCore->mApiMutexCond, &pAiqCore->mApiMutex);

    AiqAlgoHandler_t* parent = AiqAlgoHandler_getParent(pHdl);
    AiqAlgoHandler_t* next   = AiqAlgoHandler_getNextHdl(pHdl);

    if (parent)
        AiqAlgoHandler_setNextHdl(parent, next);
    else
        pAiqCore->mAlgoHandleMaps[algoType] = NULL;

    if (next) AiqAlgoHandler_setParentHdl(next, parent);

    AiqAlgoHandler_deinit(pHdl);
    aiq_free(pHdl);
    aiqMutex_unlock(&pAiqCore->mApiMutex);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

bool AiqCore_getAxlibStatus(AiqCore_t* pAiqCore, int algoType, int id) {
    AiqAlgoHandler_t* pHdl = pAiqCore->mAlgoHandleMaps[algoType];

    while (pHdl) {
        if (AiqAlgoHandler_getAlgoId(pHdl) == id) break;
        pHdl = AiqAlgoHandler_getNextHdl(pHdl);
    }

    if (!pHdl) {
        LOGD_ANALYZER("can't find type id <%d, %d> algo", algoType, id);
        return false;
    }

    return AiqAlgoHandler_getEnable(pHdl);
}

RkAiqAlgoContext* AiqCore_getEnabledAxlibCtx(AiqCore_t* pAiqCore, const int algo_type) {
    if (algo_type <= RK_AIQ_ALGO_TYPE_NONE || algo_type >= RK_AIQ_ALGO_TYPE_MAX) return NULL;

    AiqAlgoHandler_t *pHdl, *pNextHdl;
    pHdl = pNextHdl = pAiqCore->mAlgoHandleMaps[algo_type];

    // get last
    while (pNextHdl) {
        pHdl     = pNextHdl;
        pNextHdl = AiqAlgoHandler_getNextHdl(pHdl);
    }

    if (pHdl && AiqAlgoHandler_getEnable(pHdl))
        return pHdl->mAlgoCtx;
    else
        return NULL;
}

RkAiqAlgoContext* AiqCore_getAxlibCtx(AiqCore_t* pAiqCore, const int algo_type, const int lib_id) {
    AiqAlgoHandler_t* pHdl = pAiqCore->mAlgoHandleMaps[algo_type];

    while (pHdl) {
        if (AiqAlgoHandler_getAlgoId(pHdl) == lib_id) break;
        pHdl = AiqAlgoHandler_getNextHdl(pHdl);
    }

    if (!pHdl) {
        LOGE_ANALYZER("can't find type id <%d, %d> algo", algo_type, lib_id);
        return NULL;
    }

    return AiqAlgoHandler_getAlgoCtx(pHdl);
}

static uint64_t getReqAlgoResMask(AiqCore_t* pAiqCore, int algoType) {
    uint64_t tmp = 0;
    switch (algoType) {
    case RK_AIQ_ALGO_TYPE_AE:
        tmp |= 1ULL << RESULT_TYPE_EXPOSURE_PARAM;
#ifdef USE_NEWSTRUCT
        tmp |= 1ULL << RESULT_TYPE_AESTATS_PARAM;
#else
        tmp |= 1ULL << RESULT_TYPE_AEC_PARAM;
        tmp |= 1ULL << RESULT_TYPE_HIST_PARAM;
#endif
        break;
    case RK_AIQ_ALGO_TYPE_AWB:
        tmp |= 1ULL << RESULT_TYPE_AWB_PARAM;
        tmp |= 1ULL << RESULT_TYPE_AWBGAIN_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_AF:
        tmp |= 1ULL << RESULT_TYPE_AF_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_ADPCC:
        tmp |= 1ULL << RESULT_TYPE_DPCC_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_AMERGE:
        tmp |= 1ULL << RESULT_TYPE_MERGE_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_ACCM:
        tmp |= 1ULL << RESULT_TYPE_CCM_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_ALSC:
        tmp |= 1ULL << RESULT_TYPE_LSC_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_ABLC:
        tmp |= 1ULL << RESULT_TYPE_BLC_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_ARAWNR:
        tmp |= 1ULL << RESULT_TYPE_RAWNR_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_AGIC:
        tmp |= 1ULL << RESULT_TYPE_GIC_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_ADEBAYER:
        tmp |= 1ULL << RESULT_TYPE_DEBAYER_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_A3DLUT:
        tmp |= 1ULL << RESULT_TYPE_LUT3D_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_ADHAZ:
        tmp |= 1ULL << RESULT_TYPE_DEHAZE_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_AHISTEQ:
        tmp |= 1ULL << RESULT_TYPE_HISTEQ_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_AENH:
        tmp |= 1ULL << RESULT_TYPE_ENH_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_AHSV:
        tmp |= 1ULL << RESULT_TYPE_HSV_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_AGAMMA:
        tmp |= 1ULL << RESULT_TYPE_AGAMMA_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_ADEGAMMA:
        tmp |= 1ULL << RESULT_TYPE_ADEGAMMA_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_AWDR:
        tmp |= 1ULL << RESULT_TYPE_WDR_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_AGAIN:
        tmp |= 1ULL << RESULT_TYPE_GAIN_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_ACP:
        tmp |= 1ULL << RESULT_TYPE_CP_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_ACSM:
        tmp |= 1ULL << RESULT_TYPE_CSM_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_AIE:
        tmp |= 1ULL << RESULT_TYPE_IE_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_AMD:
        tmp |= 1ULL << RESULT_TYPE_MOTION_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_AMFNR:
        tmp |= 1ULL << RESULT_TYPE_TNR_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_AYNR:
        tmp |= 1ULL << RESULT_TYPE_YNR_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_ACNR:
        tmp |= 1ULL << RESULT_TYPE_UVNR_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_ASHARP:
        tmp |= 1ULL << RESULT_TYPE_SHARPEN_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_AFEC:
    case RK_AIQ_ALGO_TYPE_AEIS:
        tmp |= 1ULL << RESULT_TYPE_FEC_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_ADRC:
        tmp |= 1ULL << RESULT_TYPE_DRC_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_ACAC:
        tmp |= 1ULL << RESULT_TYPE_CAC_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_ACGC:
        tmp |= 1ULL << RESULT_TYPE_CGC_PARAM;
        break;
#if RKAIQ_HAVE_ASD_V10
    case RK_AIQ_ALGO_TYPE_ASD:
        tmp |= 1ULL << RESULT_TYPE_CPSL_PARAM;
        break;
#endif
    case RK_AIQ_ALGO_TYPE_ARGBIR:
        tmp |= 1ULL << RESULT_TYPE_RGBIR_PARAM;
        break;
    case RK_AIQ_ALGO_TYPE_ALDC:
        tmp |= 1ULL << RESULT_TYPE_LDC_PARAM;
        break;
#if RKAIQ_HAVE_AIBNR
    case RK_AIQ_ALGO_TYPE_AIBNR:
        tmp |= 1ULL << RESULT_TYPE_AIBNR_PARAM;
        break;
#endif
#if RKAIQ_HAVE_AIYNR
    case RK_AIQ_ALGO_TYPE_AIYNR:
        tmp |= 1ULL << RESULT_TYPE_AIYNR_PARAM;
        break;
#endif
#if 0
    case RK_AIQ_ALGO_TYPE_BAYERTNR2:
        tmp |= 1ULL << RESULT_TYPE_TNR2_PARAM;
        break;
#endif
    default:
        break;
    }

    return tmp;
}

static void setReqAlgoResMask(AiqCore_t* pAiqCore, int algoType, bool req) {
    uint64_t tmp = getReqAlgoResMask(pAiqCore, algoType);
    if (req) {
        pAiqCore->mAllReqAlgoResMask |= tmp;
    } else {
        pAiqCore->mAllReqAlgoResMask &= ~tmp;
    }

    LOGI_ANALYZER("mAllReqAlgoResMask : %llx", pAiqCore->mAllReqAlgoResMask);
}

XCamReturn AiqCore_enableAlgo(AiqCore_t* pAiqCore, int algoType, int id, bool enable) {
    ENTER_ANALYZER_FUNCTION();
    // get default algotype handle, id should be 0
    AiqAlgoHandler_t* first_algo_hdl  = pAiqCore->mAlgoHandleMaps[algoType];
    AiqAlgoHandler_t* target_algo_hdl = first_algo_hdl;
    if (!first_algo_hdl) {
        LOGE_ANALYZER("can't find current type %d algo", algoType);
        return XCAM_RETURN_ERROR_FAILED;
    }

    while (target_algo_hdl) {
        if (AiqAlgoHandler_getAlgoId(target_algo_hdl) == id) break;
        target_algo_hdl = AiqAlgoHandler_getNextHdl(target_algo_hdl);
    }

    if (!target_algo_hdl) {
        LOGE_ANALYZER("can't find type id <%d, %d> algo", algoType, id);
        return XCAM_RETURN_ERROR_FAILED;
    }

    aiqMutex_lock(&pAiqCore->mApiMutex);
    while (pAiqCore->mSafeEnableAlgo != true)
        aiqCond_wait(&pAiqCore->mApiMutexCond, &pAiqCore->mApiMutex);

    LOGI_ANALYZER("set algo type_id <%d,%d> to %d", algoType, id, enable);

    AiqAlgoHandler_setEnable(target_algo_hdl, enable);

    if (enable) {
        if (pAiqCore->mState >= RK_AIQ_CORE_STATE_PREPARED)
            target_algo_hdl->prepare(target_algo_hdl);
    }

    int enable_cnt  = 0;
    target_algo_hdl = first_algo_hdl;
    while (target_algo_hdl) {
        if (AiqAlgoHandler_getEnable(target_algo_hdl)) {
            enable_cnt++;
        }
        target_algo_hdl = AiqAlgoHandler_getNextHdl(target_algo_hdl);
    }

    setReqAlgoResMask(pAiqCore, algoType, enable_cnt > 0 ? true : false);

    target_algo_hdl = first_algo_hdl;

    while (target_algo_hdl) {
        AiqAlgoHandler_setMulRun(target_algo_hdl, enable_cnt > 1 ? true : false);
        target_algo_hdl = AiqAlgoHandler_getNextHdl(target_algo_hdl);
    }

    aiqMutex_unlock(&pAiqCore->mApiMutex);

    LOGI_ANALYZER("algo type %d enabled count :%d", algoType, enable_cnt);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

static void addDefaultAlgos(AiqCore_t* pAiqCore, const struct RkAiqAlgoDesCommExt* algoDes) {
    for (int i = 0; i < RK_AIQ_ALGO_TYPE_MAX; i++) {
        pAiqCore->mAlogsComSharedParams.ctxCfigs[i].calibv2 =
            (CamCalibDbV2Context_t*)(pAiqCore->mAlogsComSharedParams.calibv2);
        pAiqCore->mAlogsComSharedParams.ctxCfigs[i].isp_hw_version = pAiqCore->mIspHwVer;
    }

    for (size_t i = 0; algoDes[i].des != NULL; i++) {
        int algo_type = algoDes[i].des->type;
        // enable only the specified algorithm modules
        if ((1ULL << algo_type) & pAiqCore->mInitDisAlgosMask) continue;
        pAiqCore->mAlogsComSharedParams.ctxCfigs[algo_type].calibv2 =
            (CamCalibDbV2Context_t*)(pAiqCore->mAlogsComSharedParams.calibv2);
        pAiqCore->mAlogsComSharedParams.ctxCfigs[algo_type].module_hw_version =
            algoDes[i].module_hw_ver;
        if (!pAiqCore->mAlogsGroupSharedParamsMap[algoDes[i].group]) {
            int groupId = algoDes[i].group;
            pAiqCore->mAlogsGroupSharedParamsMap[groupId] =
                (RkAiqAlgosGroupShared_t*)aiq_mallocz(sizeof(RkAiqAlgosGroupShared_t));
            pAiqCore->mAlogsGroupSharedParamsMap[groupId]->groupId = algoDes[i].group;
            pAiqCore->mAlogsGroupSharedParamsMap[groupId]->frameId = 0;
        }
        pAiqCore->mAlgoHandleMaps[algo_type] =
            newAlgoHandle(pAiqCore, algoDes[i].des, algoDes[i].algo_ver, algoDes[i].handle_ver);
        if (!pAiqCore->mAlgoHandleMaps[algo_type]) {
            LOGE_ANALYZER("new algo_type %d handle failed", algo_type);
            continue;
        }
        AiqAlgoHandler_setGroupId(pAiqCore->mAlgoHandleMaps[algo_type], algoDes[i].group);
        AiqAlgoHandler_setGroupShared(pAiqCore->mAlgoHandleMaps[algo_type],
                                      pAiqCore->mAlogsGroupSharedParamsMap[algoDes[i].group]);
        AiqCore_enableAlgo(pAiqCore, algo_type, 0, true);
    }
}

static XCamReturn newAiqGroupAnayzer(AiqCore_t* pAiqCore, const struct RkAiqAlgoDesCommExt *algoDesArray) {
    AiqAnalyzeGroupManager_init(&pAiqCore->mRkAiqCoreGroupManager, pAiqCore,
                                pAiqCore->mIsSingleThread);
    AiqAnalyzeGroupManager_parseAlgoGroup(&pAiqCore->mRkAiqCoreGroupManager,
                                          algoDesArray);

    AiqAnalyzerGroup_t** groupMaps =
        AiqAnalyzeGroupManager_getGroups(&pAiqCore->mRkAiqCoreGroupManager);

    for (int i = 0; i < RK_AIQ_CORE_ANALYZE_MAX; i++) {
        if (groupMaps[i]) {
            pAiqCore->mFullParamReqGroupsMasks |= 1ULL << (uint64_t)(i);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_PDAF
static void newPdafStatsPool(AiqCore_t* pAiqCore) {
    if (!pAiqCore->mAiqPdafStatsPool) return;
    const CamCalibDbContext_t* aiqCalib = pAiqCore->mAlogsComSharedParams.calibv2;
    int pd_size                         = 0;

#if RKAIQ_HAVE_AF_V30
    CalibDbV2_AFV30_t* af_v30 =
        (CalibDbV2_AFV30_t*)(CALIBDBV2_GET_MODULE_PTR((void*)aiqCalib, af_v30));
    CalibDbV2_Af_Pdaf_t* pdaf = &af_v30->TuningPara.pdaf;

    pd_size = pdaf->pdMaxWidth * pdaf->pdMaxHeight * sizeof(short);
    if (pd_size > 0 && pdaf && pdaf->enable) {
        AiqPoolItem_t* pItem = NULL;
        AIQ_POOL_FOREACH(pAiqCore->mAiqPdafStatsPool, pItem) {
            rk_aiq_isp_pdaf_stats_t* pdaf_stats =
                (rk_aiq_isp_pdaf_stats_t*)(((aiq_stats_base_t*)pItem->_pData)->_data);
            pdaf_stats->pdWidth  = pdaf->pdMaxWidth;
            pdaf_stats->pdHeight = pdaf->pdMaxHeight;
            pdaf_stats->pdLData  = (unsigned short*)aiq_mallocz(pd_size);
            pdaf_stats->pdRData  = (unsigned short*)aiq_mallocz(pd_size);
        }
    } else {
        AiqPoolItem_t* pItem = NULL;
        AIQ_POOL_FOREACH(pAiqCore->mAiqPdafStatsPool, pItem) {
            rk_aiq_isp_pdaf_stats_t* pdaf_stats =
                (rk_aiq_isp_pdaf_stats_t*)(((aiq_stats_base_t*)pItem->_pData)->_data);
            pdaf_stats->pdLData = NULL;
            pdaf_stats->pdRData = NULL;
        }
    }
#endif

#if RKAIQ_HAVE_AF_V33
#if ISP_HW_V39
    CalibDbV2_AFV33_t* af_v33 =
        (CalibDbV2_AFV33_t*)(CALIBDBV2_GET_MODULE_PTR((void*)aiqCalib, af_v33));
    Af_Pdaf_t* pdaf = &af_v33->Pdaf;

    pd_size = pdaf->PdMaxWidth * pdaf->PdMaxHeight * sizeof(short);
    if (pd_size > 0 && pdaf && pdaf->Enable) {
        AiqPoolItem_t* pItem = NULL;
        AIQ_POOL_FOREACH(pAiqCore->mAiqPdafStatsPool, pItem) {
            rk_aiq_isp_pdaf_stats_t* pdaf_stats =
                (rk_aiq_isp_pdaf_stats_t*)(((aiq_stats_base_t*)pItem->_pData)->_data);
            pdaf_stats->pdWidth  = pdaf->PdMaxWidth;
            pdaf_stats->pdHeight = pdaf->PdMaxHeight;
            pdaf_stats->pdLData  = (unsigned short*)aiq_mallocz(pd_size);
            pdaf_stats->pdRData  = (unsigned short*)aiq_mallocz(pd_size);
        }
    } else {
        AiqPoolItem_t* pItem = NULL;
        AIQ_POOL_FOREACH(pAiqCore->mAiqPdafStatsPool, pItem) {
            rk_aiq_isp_pdaf_stats_t* pdaf_stats =
                (rk_aiq_isp_pdaf_stats_t*)(((aiq_stats_base_t*)pItem->_pData)->_data);
            pdaf_stats->pdLData = NULL;
            pdaf_stats->pdRData = NULL;
        }
    }
#else
    af_param_t* af_calib =
        (af_param_t*)(CALIBDBV2_GET_MODULE_PTR((void*)aiqCalib, af_calib));
    af_pdaf_t *pdaf = &af_calib->pdaf;

    pd_size = pdaf->sw_afT_pdMax_width * pdaf->sw_afT_pdMax_height * sizeof(short);
    if (pd_size > 0 && pdaf && pdaf->sw_afT_pdAf_en) {
        AiqPoolItem_t* pItem = NULL;
        AIQ_POOL_FOREACH(pAiqCore->mAiqPdafStatsPool, pItem) {
            rk_aiq_isp_pdaf_stats_t* pdaf_stats =
                (rk_aiq_isp_pdaf_stats_t*)(((aiq_stats_base_t*)pItem->_pData)->_data);
            pdaf_stats->pdWidth  = pdaf->sw_afT_pdMax_width;
            pdaf_stats->pdHeight = pdaf->sw_afT_pdMax_height;
            pdaf_stats->pdLData  = (unsigned short*)aiq_mallocz(pd_size);
            pdaf_stats->pdRData  = (unsigned short*)aiq_mallocz(pd_size);
        }
    } else {
        AiqPoolItem_t* pItem = NULL;
        AIQ_POOL_FOREACH(pAiqCore->mAiqPdafStatsPool, pItem) {
            rk_aiq_isp_pdaf_stats_t* pdaf_stats =
                (rk_aiq_isp_pdaf_stats_t*)(((aiq_stats_base_t*)pItem->_pData)->_data);
            pdaf_stats->pdLData = NULL;
            pdaf_stats->pdRData = NULL;
        }
    }
#endif
#endif
}

static void delPdafStatsPool(AiqCore_t* pAiqCore) {
    if (!pAiqCore->mAiqPdafStatsPool) return;
    rk_aiq_isp_pdaf_stats_t* pdaf_stats;

    AiqPoolItem_t* pItem = NULL;
    AIQ_POOL_FOREACH(pAiqCore->mAiqPdafStatsPool, pItem) {
        pdaf_stats = (rk_aiq_isp_pdaf_stats_t*)(((aiq_stats_base_t*)pItem->_pData)->_data);
        if (pdaf_stats->pdLData) {
            free(pdaf_stats->pdLData);
            pdaf_stats->pdLData = NULL;
        }
        if (pdaf_stats->pdRData) {
            free(pdaf_stats->pdRData);
            pdaf_stats->pdRData = NULL;
        }
    }

    for (int i = 0; i < 2; i++) {
        if (pAiqCore->mPdafStats[i]) {
            pdaf_stats = (rk_aiq_isp_pdaf_stats_t*)(pAiqCore->mPdafStats[i]->_data);
            if (pdaf_stats->pdLData) {
                free(pdaf_stats->pdLData);
                pdaf_stats->pdLData = NULL;
            }
            if (pdaf_stats->pdRData) {
                free(pdaf_stats->pdRData);
                pdaf_stats->pdRData = NULL;
            }
        }
    }
}
#endif

#ifdef RKAIQ_ENABLE_CAMGROUP
void AiqCore_setCamGroupManager(AiqCore_t* pAiqCore, AiqCamGroupManager_t* cam_group_manager) {
    pAiqCore->mCamGroupCoreManager = cam_group_manager;
    if (pAiqCore->mTranslator && cam_group_manager)
        AiqStatsTranslator_setGroupMode(pAiqCore->mTranslator, true);

    uint64_t removed_mask = 0xffffffffffffffff;
    if (pAiqCore->mCamGroupCoreManager != NULL) {
        if (/*AiqCore_isGroupAlgo(pAiqCore, RK_AIQ_ALGO_TYPE_AE)*/true) {
            removed_mask &=
                ~((1ULL << XCAM_MESSAGE_AE_PRE_RES_OK) | (1ULL << XCAM_MESSAGE_AE_PROC_RES_OK));
        }
        if (/*AiqCore_isGroupAlgo(pAiqCore, RK_AIQ_ALGO_TYPE_AWB)*/true) {
            removed_mask &= ~(1ULL << XCAM_MESSAGE_AWB_PROC_RES_OK);
        }
        if (/*AiqCore_isGroupAlgo(pAiqCore, RK_AIQ_ALGO_TYPE_AF)*/true) {
            removed_mask &= ~(1ULL << XCAM_MESSAGE_AF_PROC_RES_OK);
        }
        if (AiqCore_isGroupAlgo(pAiqCore, RK_AIQ_ALGO_TYPE_AYNR)) {
            removed_mask &= ~((1ULL << XCAM_MESSAGE_YNR_V3_PROC_RES_OK) |
                              (1ULL << XCAM_MESSAGE_YNR_V22_PROC_RES_OK) |
                              (1ULL << XCAM_MESSAGE_YNR_V24_PROC_RES_OK) |
                              (1ULL << XCAM_MESSAGE_YNR_PROC_RES_OK));
        }
        if (AiqCore_isGroupAlgo(pAiqCore, RK_AIQ_ALGO_TYPE_ABLC)) {
            removed_mask &= ~((1ULL << XCAM_MESSAGE_BLC_PROC_RES_OK) |
                              (1ULL << XCAM_MESSAGE_BLC_V32_PROC_RES_OK));
        }
        AiqAnalyzerGroup_t** groups =
            AiqAnalyzeGroupManager_getGroups(&pAiqCore->mRkAiqCoreGroupManager);
        for (int i = 0; i < RK_AIQ_CORE_ANALYZE_MAX; i++) {
            if (groups[i]) {
                uint64_t mask =
                    AiqAnalyzeGroupManager_getGrpDeps(&pAiqCore->mRkAiqCoreGroupManager, i);
                mask &= removed_mask;
                AiqAnalyzeGroupManager_setGrpDeps(&pAiqCore->mRkAiqCoreGroupManager, i, mask);
            }
        }
    }
}
#endif

XCamReturn AiqCore_init(AiqCore_t* pAiqCore, const char* sns_ent_name,
                        const CamCalibDbV2Context_t* aiqCalibv2) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pAiqCore->mState != RK_AIQ_CORE_STATE_INVALID) {
        LOGE_ANALYZER("wrong state %d\n", pAiqCore->mState);
        return XCAM_RETURN_ERROR_ANALYZER;
    }

    pAiqCore->mIsSingleThread = true;
    pAiqCore->mSafeEnableAlgo = true;
    aiqMutex_init(&pAiqCore->_update_mutex);
    aiqMutex_init(&pAiqCore->mIspStatsMutex);
    aiqMutex_init(&pAiqCore->mApiMutex);
    aiqMutex_init(&pAiqCore->_mFullParam_mutex);
    aiqMutex_init(&pAiqCore->mPdafStatsMutex);
    aiqCond_init(&pAiqCore->_update_done_cond);
    aiqCond_init(&pAiqCore->mIspStatsCond);
    aiqCond_init(&pAiqCore->mApiMutexCond);

    pAiqCore->mDefaultDelayCnt       = ISP_PARAMS_EFFECT_DELAY_CNT;
    uint16_t DEFAULT_POOL_SIZE       = pAiqCore->mDefaultDelayCnt + 1;

    {
        AiqPoolConfig_t poolCfg;
        int i                        = 0;
        AiqPool_t* ret               = NULL;
        poolCfg._name                = "fullParam";
        poolCfg._item_nums           = DEFAULT_POOL_SIZE + 1;
        poolCfg._item_size           = sizeof(AiqFullParams_t);
        pAiqCore->mAiqFullParamsPool = aiqPool_init(&poolCfg);
        if (!pAiqCore->mAiqFullParamsPool ) {
            LOGE_ANALYZER("init %s error", poolCfg._name);
            return XCAM_RETURN_ERROR_ANALYZER;
        }
        AiqPoolItem_t* pItem = NULL;
        AIQ_POOL_FOREACH(pAiqCore->mAiqFullParamsPool, pItem) {
            aiq_params_base_t* pBase = (aiq_params_base_t*)(pItem->_pData);
            AIQ_REF_BASE_INIT(&pBase->_ref_base, pItem, aiqPoolItem_ref, aiqPoolItem_unref);
        }
        pItem = aiqPool_getFree(pAiqCore->mAiqFullParamsPool);
        pAiqCore->mAiqCurParams = (AiqFullParams_t*)pItem->_pData;
    }

    {
        AiqMapConfig_t pendingParamsCfg;
        pendingParamsCfg._name          = "pendingParams";
        pendingParamsCfg._key_type      = AIQ_MAP_KEY_TYPE_UINT32;
        pendingParamsCfg._item_nums     = DEFAULT_POOL_SIZE;
        pendingParamsCfg._item_size     = sizeof(pending_params_t);
        pAiqCore->mFullParamsPendingMap = aiqMap_init(&pendingParamsCfg);
        if (!pAiqCore->mFullParamsPendingMap)
            LOGE_ANALYZER("cId:%d init %s error", pAiqCore->mAlogsComSharedParams.mCamPhyId,
                          pendingParamsCfg._name);
    }

    pAiqCore->mAlogsComSharedParams.calibv2 = aiqCalibv2;
    const CalibDb_AlgoSwitch_t* algoSwitch  = &aiqCalibv2->sys_cfg->algoSwitch;
    if (algoSwitch->enable && algoSwitch->disable_algos) {
        for (uint16_t i = 0; i < algoSwitch->disable_algos_len; i++)
            pAiqCore->mInitDisAlgosMask |= 1ULL << algoSwitch->disable_algos[i];
    }
    LOGI_ANALYZER("mInitDisAlgosMask: 0x%" PRIx64 "\n", pAiqCore->mInitDisAlgosMask);

    pAiqCore->mAlgosDesArray = g_default_3a_des;
    addDefaultAlgos(pAiqCore, pAiqCore->mAlgosDesArray);
    ret = newAllAiqParamPool(pAiqCore);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ANALYZER("init params pool failed");
        goto fail;
    }

    pAiqCore->mTranslator = (AiqStatsTranslator_t*)aiq_mallocz(sizeof(AiqStatsTranslator_t));
    if (!pAiqCore->mTranslator) goto fail;

    AiqStatsTranslator_setCamPhyId(pAiqCore->mTranslator,
                                   pAiqCore->mAlogsComSharedParams.mCamPhyId);
    AiqStatsTranslator_init(pAiqCore->mTranslator);

    newAiqGroupAnayzer(pAiqCore, pAiqCore->mAlgosDesArray);
#if RKAIQ_HAVE_PDAF
    newPdafStatsPool(pAiqCore);
#endif

    pAiqCore->mAiqSofInfoWrapperPool = AiqCore_createPool(
                                           "sofInfo", DEFAULT_POOL_SIZE, CALC_SIZE_WITH_HEAD(AlgoRstShared_t, AiqSofInfoWrapper_t),
                                           AIQCORE_POOL_TYPE_SHARED);
    if (!pAiqCore->mAiqSofInfoWrapperPool) {
        LOGE_ANALYZER("init sofInfo pool failed");
        goto fail;
    }

#if RKAIQ_HAVE_DUMPSYS
    {
        pAiqCore->dump_core  = AiqCore_dump;
        pAiqCore->dump_algos = AiqCore_algosDump;

        AiqCore_initNotifier(pAiqCore);
        pAiqCore->mForceDoneCnt = 0;
        pAiqCore->mForceDoneId  = 0;
        pAiqCore->mSofLoss      = 0;
        pAiqCore->mStatsLoss    = 0;

        xcam_mem_clear(pAiqCore->mNoFreeBufCnt);
        xcam_mem_clear(pAiqCore->mIsStatsAvail);
    }
#endif

    pAiqCore->mState = RK_AIQ_CORE_STATE_INITED;
    return XCAM_RETURN_NO_ERROR;
fail:
    AiqCore_deinit(pAiqCore);
    return XCAM_RETURN_ERROR_FAILED;

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn AiqCore_deinit(AiqCore_t* pAiqCore) {
    ENTER_ANALYZER_FUNCTION();

    if (pAiqCore->mState == RK_AIQ_CORE_STATE_STARTED ||
            pAiqCore->mState == RK_AIQ_CORE_STATE_RUNNING) {
        LOGE_ANALYZER("wrong state %d\n", pAiqCore->mState);
        return XCAM_RETURN_ERROR_ANALYZER;
    }

    AiqAnalyzeGroupManager_deinit(&pAiqCore->mRkAiqCoreGroupManager);
    if (pAiqCore->mTranslator) {
        AiqStatsTranslator_deinit(pAiqCore->mTranslator);
        aiq_free(pAiqCore->mTranslator);
        pAiqCore->mTranslator = NULL;
    }

    for (int i = 0; i < RK_AIQ_CORE_ANALYZE_MAX; i++) {
        if (pAiqCore->mAlogsGroupSharedParamsMap[i] != NULL) {
            aiq_free(pAiqCore->mAlogsGroupSharedParamsMap[i]);
            pAiqCore->mAlogsGroupSharedParamsMap[i] = NULL;
        }
    }

    for (int i = 0; i < RK_AIQ_ALGO_TYPE_MAX; i++) {
        if (pAiqCore->mAlgoHandleMaps[i]) destroyAlgoHandler(pAiqCore->mAlgoHandleMaps[i]);
        pAiqCore->mAlgoHandleMaps[i] = NULL;
    }

#if RKAIQ_HAVE_PDAF
    delPdafStatsPool(pAiqCore);
#endif
    deleteAllAiqParamPool(pAiqCore);

    if (pAiqCore->mAiqCurParams) {
        AIQ_REF_BASE_UNREF(&pAiqCore->mAiqCurParams->_base._ref_base);
        pAiqCore->mAiqCurParams = NULL;
    }

    if (pAiqCore->mAiqFullParamsPool) {
        aiqPool_deinit(pAiqCore->mAiqFullParamsPool);
        pAiqCore->mAiqFullParamsPool = NULL;
    }

    if (pAiqCore->mFullParamsPendingMap) {
        aiqMap_deinit(pAiqCore->mFullParamsPendingMap);
        pAiqCore->mFullParamsPendingMap = NULL;
    }

    if (pAiqCore->mUpdateCalibAlgosList) {
        aiqList_deinit(pAiqCore->mUpdateCalibAlgosList);
        pAiqCore->mUpdateCalibAlgosList = NULL;
    }

    if (pAiqCore->mAiqSofInfoWrapperPool) {
        aiqPool_deinit(pAiqCore->mAiqSofInfoWrapperPool);
        pAiqCore->mAiqSofInfoWrapperPool = NULL;
    }

    aiqMutex_deInit(&pAiqCore->_update_mutex);
    aiqMutex_deInit(&pAiqCore->mIspStatsMutex);
    aiqMutex_deInit(&pAiqCore->mApiMutex);
    aiqMutex_deInit(&pAiqCore->_mFullParam_mutex);
    aiqMutex_deInit(&pAiqCore->bay3dStatListMutex);
    aiqMutex_deInit(&pAiqCore->mPdafStatsMutex);
    aiqCond_deInit(&pAiqCore->_update_done_cond);
    aiqCond_deInit(&pAiqCore->mIspStatsCond);
    aiqCond_deInit(&pAiqCore->mApiMutexCond);

    pAiqCore->mState = RK_AIQ_CORE_STATE_INVALID;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCore_reinitForNewStatsDelay(AiqCore_t* pAiqCore, int delayCnt)
{
    if (pAiqCore->mDefaultDelayCnt == delayCnt) {
        return XCAM_RETURN_NO_ERROR;
    }

#if RKAIQ_HAVE_PDAF
    delPdafStatsPool(pAiqCore);
#endif
    deleteAllAiqParamPool(pAiqCore);

    if (pAiqCore->mAiqCurParams) {
        AIQ_REF_BASE_UNREF(&pAiqCore->mAiqCurParams->_base._ref_base);
        pAiqCore->mAiqCurParams = NULL;
    }

    if (pAiqCore->mAiqFullParamsPool) {
        aiqPool_deinit(pAiqCore->mAiqFullParamsPool);
        pAiqCore->mAiqFullParamsPool = NULL;
    }

    if (pAiqCore->mFullParamsPendingMap) {
        aiqMap_deinit(pAiqCore->mFullParamsPendingMap);
        pAiqCore->mFullParamsPendingMap = NULL;
    }

    if (pAiqCore->mAiqSofInfoWrapperPool) {
        aiqPool_deinit(pAiqCore->mAiqSofInfoWrapperPool);
        pAiqCore->mAiqSofInfoWrapperPool = NULL;
    }

    pAiqCore->mDefaultDelayCnt = delayCnt;
    uint16_t DEFAULT_POOL_SIZE = pAiqCore->mDefaultDelayCnt + 1;

    {
        AiqPoolConfig_t poolCfg;
        int i                        = 0;
        AiqPool_t* ret               = NULL;
        poolCfg._name                = "fullParam";
        poolCfg._item_nums           = DEFAULT_POOL_SIZE + 1;
        poolCfg._item_size           = sizeof(AiqFullParams_t);
        pAiqCore->mAiqFullParamsPool = aiqPool_init(&poolCfg);
        if (!pAiqCore->mAiqFullParamsPool ) {
            LOGE_ANALYZER("init %s error", poolCfg._name);
            return XCAM_RETURN_ERROR_ANALYZER;
        }
        AiqPoolItem_t* pItem = NULL;
        AIQ_POOL_FOREACH(pAiqCore->mAiqFullParamsPool, pItem) {
            aiq_params_base_t* pBase = (aiq_params_base_t*)(pItem->_pData);
            AIQ_REF_BASE_INIT(&pBase->_ref_base, pItem, aiqPoolItem_ref, aiqPoolItem_unref);
        }
        pItem = aiqPool_getFree(pAiqCore->mAiqFullParamsPool);
        pAiqCore->mAiqCurParams = (AiqFullParams_t*)pItem->_pData;
    }

    {
        AiqMapConfig_t pendingParamsCfg;
        pendingParamsCfg._name          = "pendingParams";
        pendingParamsCfg._key_type      = AIQ_MAP_KEY_TYPE_UINT32;
        pendingParamsCfg._item_nums     = DEFAULT_POOL_SIZE;
        pendingParamsCfg._item_size     = sizeof(pending_params_t);
        pAiqCore->mFullParamsPendingMap = aiqMap_init(&pendingParamsCfg);
        if (!pAiqCore->mFullParamsPendingMap)
            LOGE_ANALYZER("cId:%d init %s error", pAiqCore->mAlogsComSharedParams.mCamPhyId,
                          pendingParamsCfg._name);
    }
    newAllAiqParamPool(pAiqCore);
#if RKAIQ_HAVE_PDAF
    newPdafStatsPool(pAiqCore);
#endif

    pAiqCore->mAiqSofInfoWrapperPool = AiqCore_createPool(
                                           "sofInfo", DEFAULT_POOL_SIZE, CALC_SIZE_WITH_HEAD(AlgoRstShared_t, AiqSofInfoWrapper_t),
                                           AIQCORE_POOL_TYPE_SHARED);
    if (!pAiqCore->mAiqSofInfoWrapperPool) {
        LOGE_ANALYZER("init sofInfo pool failed");
        return XCAM_RETURN_ERROR_FAILED;
    }

    AiqAnalyzeGroupManager_resetDelayCnt(&pAiqCore->mRkAiqCoreGroupManager, delayCnt);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCore_start(AiqCore_t* pAiqCore) {
    ENTER_ANALYZER_FUNCTION();

    aiqMutex_lock(&pAiqCore->mApiMutex);

    if ((pAiqCore->mState != RK_AIQ_CORE_STATE_PREPARED) &&
            (pAiqCore->mState != RK_AIQ_CORE_STATE_STOPED)) {
        LOGE_ANALYZER("wrong state %d\n", pAiqCore->mState);
        aiqMutex_unlock(&pAiqCore->mApiMutex);
        return XCAM_RETURN_ERROR_ANALYZER;
    }
#if RKAIQ_HAVE_PDAF
    uint64_t deps = AiqAnalyzeGroupManager_getGrpDeps(&pAiqCore->mRkAiqCoreGroupManager,
                    RK_AIQ_CORE_ANALYZE_AF);
    if (pAiqCore->mPdafSupport) {
        deps |= 1LL << XCAM_MESSAGE_PDAF_STATS_OK;
    } else {
        deps &= ~(1LL << XCAM_MESSAGE_PDAF_STATS_OK);
    }
    AiqAnalyzeGroupManager_setGrpDeps(&pAiqCore->mRkAiqCoreGroupManager, RK_AIQ_CORE_ANALYZE_AF,
                                      deps);
#endif
    AiqAnalyzeGroupManager_start(&pAiqCore->mRkAiqCoreGroupManager);
    pAiqCore->mState = RK_AIQ_CORE_STATE_STARTED;

    EXIT_ANALYZER_FUNCTION();

    aiqMutex_unlock(&pAiqCore->mApiMutex);

    return XCAM_RETURN_NO_ERROR;
}

#if defined(RKAIQ_HAVE_BAYERTNR_V30)
static void ClearBay3dStatsList(AiqCore_t* pAiqCore) {
    aiqMutex_lock(&pAiqCore->bay3dStatListMutex);
    AiqListItem_t* pItem = NULL;
    bool rm              = false;
    AIQ_LIST_FOREACH(pAiqCore->bay3dStatList, pItem, rm) {
        aiq_stats_base_t* pStat = *(aiq_stats_base_t**)(pItem->_pData);
        AIQ_REF_BASE_UNREF(&pStat->_ref_base);
        pItem = aiqList_erase_item_locked(pAiqCore->bay3dStatList, pItem);
        rm    = true;
    }
    aiqMutex_unlock(&pAiqCore->bay3dStatListMutex);
}
#endif

static void clearFullParamsPendingMap(AiqCore_t* pAiqCore) {
    AiqMapItem_t* pItem;
    bool rm;
    AIQ_MAP_FOREACH(pAiqCore->mFullParamsPendingMap, pItem, rm) {
        pending_params_t* pPending = (pending_params_t*)(pItem->_pData);
        if (pPending->fullParams) AIQ_REF_BASE_UNREF(&pPending->fullParams->_base._ref_base);
        pItem = aiqMap_erase_locked(pAiqCore->mFullParamsPendingMap, pItem->_key);
        rm    = true;
    }
}

static void resetVicapScalbuf(RkAiqVicapRawBufInfo_t* pRawBufInfo) {
    if (!pRawBufInfo) return;

    pRawBufInfo->frame_id = -1;
    pRawBufInfo->ready    = false;

    if (pRawBufInfo->raw_s) {
        AiqVideoBuffer_unref(pRawBufInfo->raw_s);
        pRawBufInfo->raw_s = NULL;
    }
    if (pRawBufInfo->raw_m) {
        AiqVideoBuffer_unref(pRawBufInfo->raw_m);
        pRawBufInfo->raw_m = NULL;
    }
    if (pRawBufInfo->raw_l) {
        AiqVideoBuffer_unref(pRawBufInfo->raw_l);
        pRawBufInfo->raw_l = NULL;
    }
}

XCamReturn AiqCore_stop(AiqCore_t* pAiqCore) {
    ENTER_ANALYZER_FUNCTION();

    aiqMutex_lock(&pAiqCore->mApiMutex);

    if (pAiqCore->mState != RK_AIQ_CORE_STATE_STARTED &&
            pAiqCore->mState != RK_AIQ_CORE_STATE_RUNNING) {
        LOGW_ANALYZER("in state %d\n", pAiqCore->mState);
        aiqMutex_unlock(&pAiqCore->mApiMutex);
        return XCAM_RETURN_ERROR_FAILED;
    }

    aiqMutex_lock(&pAiqCore->mIspStatsMutex);
    pAiqCore->mState                                    = RK_AIQ_CORE_STATE_STOPED;
    AiqAnalyzeGroupManager_stop(&pAiqCore->mRkAiqCoreGroupManager);
    pAiqCore->mAlogsComSharedParams.conf_type = RK_AIQ_ALGO_CONFTYPE_INIT;
    aiqMutex_unlock(&pAiqCore->mIspStatsMutex);
    aiqCond_broadcast(&pAiqCore->mIspStatsCond);

    int cnts = 0;
    AiqAlgoHandler_t** pHdlList = NULL;
    pHdlList = AiqAnalyzeGroupManager_getGroupAlgoList(&pAiqCore->mRkAiqCoreGroupManager, RK_AIQ_CORE_ANALYZE_ALL, &cnts);

    for (int i = 0; i < cnts; i++) {
        AiqAlgoHandler_t* pHdl = pHdlList[i];
        bool got_buffer        = false;
        while (pHdl) {
            if (pHdl->mEnable) {
                if (pHdl->stop)
                    pHdl->stop(pHdl);
            }
            pHdl = AiqAlgoHandler_getNextHdl(pHdl);
        }
    }
    aiqMutex_unlock(&pAiqCore->mApiMutex);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

void AiqCore_clean(AiqCore_t* pAiqCore)
{
    pAiqCore->firstStatsReceived              = false;
    pAiqCore->mLastAnalyzedId                 = 0;
    pAiqCore->mSafeEnableAlgo = true;
    clearFullParamsPendingMap(pAiqCore);
    pAiqCore->mLatestParamsDoneId = 0;
    pAiqCore->mLatestEvtsId       = 0;
    pAiqCore->mLatestStatsId      = 0;
#if RKAIQ_HAVE_DUMPSYS
    pAiqCore->mForceDoneCnt = 0;
    pAiqCore->mForceDoneId  = 0;
    pAiqCore->mSofLoss      = 0;
    pAiqCore->mStatsLoss    = 0;

    xcam_mem_clear(pAiqCore->mNoFreeBufCnt);
    xcam_mem_clear(pAiqCore->mIsStatsAvail);
#endif

#if defined(RKAIQ_HAVE_BAYERTNR_V30)
#if (USE_NEWSTRUCT == 0)
    ClearBay3dStatsList(pAiqCore);
#endif
#endif
    if (pAiqCore->mVicapBufs) resetVicapScalbuf(pAiqCore->mVicapBufs);
    if (pAiqCore->mCurAeStats) {
        AIQ_REF_BASE_UNREF(&pAiqCore->mCurAeStats->_ref_base);
        pAiqCore->mCurAeStats = NULL;
    }
    if (pAiqCore->mCurAwbStats) {
        AIQ_REF_BASE_UNREF(&pAiqCore->mCurAwbStats->_ref_base);
        pAiqCore->mCurAwbStats = NULL;
    }
    if (pAiqCore->mCurAfStats) {
        AIQ_REF_BASE_UNREF(&pAiqCore->mCurAfStats->_ref_base);
        pAiqCore->mCurAfStats = NULL;
    }
    if (pAiqCore->mAfStats[0]) {
        AIQ_REF_BASE_UNREF(&pAiqCore->mAfStats[0]->_ref_base);
        pAiqCore->mAfStats[0] = NULL;
    }
    if (pAiqCore->mAfStats[1]) {
        AIQ_REF_BASE_UNREF(&pAiqCore->mAfStats[1]->_ref_base);
        pAiqCore->mAfStats[1] = NULL;
    }
    if (pAiqCore->mPdafStats[0]) {
        AIQ_REF_BASE_UNREF(&pAiqCore->mPdafStats[0]->_ref_base);
        pAiqCore->mPdafStats[0] = NULL;
    }
    if (pAiqCore->mPdafStats[1]) {
        AIQ_REF_BASE_UNREF(&pAiqCore->mPdafStats[1]->_ref_base);
        pAiqCore->mPdafStats[1] = NULL;
    }
    AiqAnalyzeGroupManager_clean(&pAiqCore->mRkAiqCoreGroupManager);
}

static XCamReturn freeGroupSharebuf(RkAiqAlgosGroupShared_t* shared) {
    if (shared) {
        if (shared->afStatsBuf) {
            AIQ_REF_BASE_UNREF(&shared->afStatsBuf->_ref_base);
            shared->afStatsBuf = NULL;
        }
        if (shared->tx) {
            shared->tx->unref(shared->tx);
            shared->tx = NULL;
        }
        if (shared->sp) {
            shared->sp->unref(shared->sp);
            shared->sp = NULL;
        }
        if (shared->ispGain) {
            shared->ispGain->unref(shared->ispGain);
            shared->ispGain = NULL;
        }
        if (shared->nrImg) {
            shared->nrImg->unref(shared->nrImg);
            shared->nrImg = NULL;
        }
        if (shared->pdafStatsBuf) {
            AIQ_REF_BASE_UNREF(&shared->pdafStatsBuf->_ref_base);
            shared->pdafStatsBuf = NULL;
        }
        if (shared->adehazeStatsBuf) {
            AIQ_REF_BASE_UNREF(&shared->adehazeStatsBuf->_ref_base);
            shared->adehazeStatsBuf = NULL;
        }
        if (shared->againStatsBuf) {
            AIQ_REF_BASE_UNREF(&shared->againStatsBuf->_ref_base);
            shared->againStatsBuf = NULL;
        }
        if (shared->res_comb.ae_pre_res) {
            shared->res_comb.ae_pre_res->unref(shared->res_comb.ae_pre_res);
            shared->res_comb.ae_pre_res = NULL;
        }
        if (shared->res_comb.ae_proc_res) {
            shared->res_comb.ae_proc_res->unref(shared->res_comb.ae_proc_res);
            shared->res_comb.ae_proc_res = NULL;
        }
        if (shared->res_comb.awb_proc_res) {
            shared->res_comb.awb_proc_res->unref(shared->res_comb.awb_proc_res);
            shared->res_comb.awb_proc_res = NULL;
        }

        return XCAM_RETURN_NO_ERROR;
    }
    return XCAM_RETURN_NO_ERROR;
}

static void clearAlgosComSharedParams(AiqCore_t* pAiqCore) {
    // TODO
}

static void clearAlgosGroupSharedParams(RkAiqAlgosGroupShared_t* pSahred) {
    // TODO
    freeGroupSharebuf(pSahred);
}

static XCamReturn getAiqParamsBuffer(AiqCore_t* pAiqCore, AiqFullParams_t* aiqParams, int type,
                                     uint32_t frame_id) {
#if RKAIQ_HAVE_DUMPSYS
#define INC_NO_FREE_CNT(type) pAiqCore->mNoFreeBufCnt.algosParams[type]++
#else
#define INC_NO_FREE_CNT(type)
#endif

#define NEW_PARAMS_BUFFER(BC)                                                             \
    do {                                                                                  \
        aiq_params_base_t* pBase = aiqParams->pParamsArray[RESULT_TYPE_##BC##_PARAM];     \
        if (!pBase) {                                                                     \
            AiqPoolItem_t* pItem =                                                        \
                aiqPool_getFree(pAiqCore->mAiqParamsPoolArray[RESULT_TYPE_##BC##_PARAM]); \
            if (pItem) {                                                                  \
                pBase            = (aiq_params_base_t*)pItem->_pData;                     \
                pBase->frame_id  = frame_id;                                              \
                pBase->is_update = false;                                                 \
                pBase->type      = RESULT_TYPE_##BC##_PARAM;                              \
                aiqParams->pParamsArray[RESULT_TYPE_##BC##_PARAM] = pBase;                \
            } else {                                                                      \
                LOGE_ANALYZER("no free %s buffer for Id: %d !", #BC, frame_id);           \
                INC_NO_FREE_CNT(RESULT_TYPE_##BC##_PARAM);                                \
                return XCAM_RETURN_ERROR_MEM;                                             \
            }                                                                             \
        } else {                                                                          \
            pBase->frame_id  = frame_id;                                                  \
            pBase->is_update = false;                                                     \
        }                                                                                 \
    } while (0)

    switch (type) {
    case RK_AIQ_ALGO_TYPE_AE:
        NEW_PARAMS_BUFFER(EXPOSURE);
        NEW_PARAMS_BUFFER(IRIS);
#if USE_NEWSTRUCT
        NEW_PARAMS_BUFFER(AESTATS);
#else
        NEW_PARAMS_BUFFER(AEC);
        NEW_PARAMS_BUFFER(HIST);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_AWB:
        NEW_PARAMS_BUFFER(AWB);
        NEW_PARAMS_BUFFER(AWBGAIN);
        break;
    case RK_AIQ_ALGO_TYPE_AF:
        NEW_PARAMS_BUFFER(AF);
        NEW_PARAMS_BUFFER(FOCUS);
        break;
    case RK_AIQ_ALGO_TYPE_ABLC:
        NEW_PARAMS_BUFFER(BLC);
        break;
    case RK_AIQ_ALGO_TYPE_ADPCC:
        NEW_PARAMS_BUFFER(DPCC);
        break;
    case RK_AIQ_ALGO_TYPE_AMERGE:
        NEW_PARAMS_BUFFER(MERGE);
        break;
    case RK_AIQ_ALGO_TYPE_ALSC:
        NEW_PARAMS_BUFFER(LSC);
        break;
    case RK_AIQ_ALGO_TYPE_AGIC:
        NEW_PARAMS_BUFFER(GIC);
        break;
    case RK_AIQ_ALGO_TYPE_ADEBAYER:
        NEW_PARAMS_BUFFER(DEBAYER);
        break;
    case RK_AIQ_ALGO_TYPE_ACCM:
        NEW_PARAMS_BUFFER(CCM);
        break;
    case RK_AIQ_ALGO_TYPE_AGAMMA:
        NEW_PARAMS_BUFFER(AGAMMA);
        break;
    case RK_AIQ_ALGO_TYPE_AWDR:
        NEW_PARAMS_BUFFER(WDR);
        break;
    case RK_AIQ_ALGO_TYPE_ADHAZ:
        NEW_PARAMS_BUFFER(DEHAZE);
        break;
    case RK_AIQ_ALGO_TYPE_AHISTEQ:
        NEW_PARAMS_BUFFER(HISTEQ);
        break;
    case RK_AIQ_ALGO_TYPE_AENH:
        NEW_PARAMS_BUFFER(ENH);
        break;
    case RK_AIQ_ALGO_TYPE_AHSV:
        NEW_PARAMS_BUFFER(HSV);
        break;
    case RK_AIQ_ALGO_TYPE_A3DLUT:
        NEW_PARAMS_BUFFER(LUT3D);
        break;
    case RK_AIQ_ALGO_TYPE_ACSM:
        NEW_PARAMS_BUFFER(CSM);
        break;
    case RK_AIQ_ALGO_TYPE_ACP:
        NEW_PARAMS_BUFFER(CP);
        break;
    case RK_AIQ_ALGO_TYPE_AIE:
        NEW_PARAMS_BUFFER(IE);
        break;
    case RK_AIQ_ALGO_TYPE_ACGC:
        NEW_PARAMS_BUFFER(CGC);
        break;
    case RK_AIQ_ALGO_TYPE_ADRC:
        NEW_PARAMS_BUFFER(DRC);
        break;
    case RK_AIQ_ALGO_TYPE_ADEGAMMA:
        NEW_PARAMS_BUFFER(ADEGAMMA);
        break;
    case RK_AIQ_ALGO_TYPE_ARAWNR:
        NEW_PARAMS_BUFFER(RAWNR);
        break;
    case RK_AIQ_ALGO_TYPE_AMFNR:
        NEW_PARAMS_BUFFER(TNR);
        break;
    case RK_AIQ_ALGO_TYPE_AYNR:
        NEW_PARAMS_BUFFER(YNR);
        break;
    case RK_AIQ_ALGO_TYPE_ACNR:
        NEW_PARAMS_BUFFER(UVNR);
        break;
    case RK_AIQ_ALGO_TYPE_ASHARP:
        NEW_PARAMS_BUFFER(SHARPEN);
#if defined(RKAIQ_HAVE_SHARP_V40) || defined(RKAIQ_HAVE_SHARP_V41)
        NEW_PARAMS_BUFFER(TEXEST);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_AFEC:
    case RK_AIQ_ALGO_TYPE_AEIS:
        NEW_PARAMS_BUFFER(FEC);
        break;
    case RK_AIQ_ALGO_TYPE_AMD:
        NEW_PARAMS_BUFFER(MOTION);
        break;
    case RK_AIQ_ALGO_TYPE_AGAIN:
#if RKAIQ_HAVE_GAIN
        NEW_PARAMS_BUFFER(GAIN);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_ACAC:
#if RKAIQ_HAVE_CAC
        NEW_PARAMS_BUFFER(CAC);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_AFD:
#if RKAIQ_HAVE_AFD
        NEW_PARAMS_BUFFER(AFD);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_ARGBIR:
#if RKAIQ_HAVE_RGBIR_REMOSAIC
        NEW_PARAMS_BUFFER(RGBIR);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_ALDC:
#if RKAIQ_HAVE_LDC
        NEW_PARAMS_BUFFER(LDC);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_AIBNR:
#if RKAIQ_HAVE_AIBNR
        NEW_PARAMS_BUFFER(AIBNR);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_AIRMS:
#if RKAIQ_HAVE_AIRMS
        NEW_PARAMS_BUFFER(AIRMS);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_AIYNR:
#if RKAIQ_HAVE_AIYNR
        NEW_PARAMS_BUFFER(AIYNR);
#endif
        break;
    default:
        break;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCore_post_message(AiqCore_t* pAiqCore, AiqCoreMsg_t* msg) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret            = AiqAnalyzeGroupManager_handleMessage(&pAiqCore->mRkAiqCoreGroupManager, msg);
#ifdef RKAIQ_ENABLE_CAMGROUP
    if (pAiqCore->mCamGroupCoreManager)
        AiqCamGroupManager_processAiqCoreMsgs(pAiqCore->mCamGroupCoreManager, pAiqCore, msg);
#endif
    return ret;
}

void AiqCore_clear_message(AiqCoreMsg_t* msg) {
    if (msg->msg_id == XCAM_MESSAGE_AE_PRE_RES_OK || msg->msg_id == XCAM_MESSAGE_AWB_PROC_RES_OK ||
            msg->msg_id == XCAM_MESSAGE_AWB_PROC_RES_OK ||
            msg->msg_id == XCAM_MESSAGE_YNR_V24_PROC_RES_OK) {
        AlgoRstShared_t* pShared = *(AlgoRstShared_t**)(msg->buf);
        if (!pShared) {
            AIQ_REF_BASE_UNREF(&pShared->_ref_base);
        }
    }
}

static void getDummyAlgoRes(AiqCore_t * pAiqCore, int type, uint32_t frame_id) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (type == RK_AIQ_ALGO_TYPE_AE) {
        AiqPoolItem_t* pItem = aiqPool_getFree(pAiqCore->mPreResAeSharedPool);
        if (!pItem) {
            LOGE_ANALYZER("fid:%d, get free AePreRes faild !", frame_id);
#if RKAIQ_HAVE_DUMPSYS
            pAiqCore->mNoFreeBufCnt.aePreRes++;
#endif
            return;
        }
        AlgoRstShared_t* pSahred = (AlgoRstShared_t*)pItem->_pData;
        pSahred->frame_id        = frame_id;
        AiqCoreMsg_t msg;
        msg.msg_id                    = XCAM_MESSAGE_AE_PRE_RES_OK;
        msg.frame_id                  = frame_id;
        *(AlgoRstShared_t**)(msg.buf) = pSahred;
        AiqCore_post_message(pAiqCore, &msg);
        AIQ_REF_BASE_UNREF(&pSahred->_ref_base);
    } else if (type == RK_AIQ_ALGO_TYPE_AWB) {
        AiqPoolItem_t* pItem = aiqPool_getFree(pAiqCore->mProcResAwbSharedPool);
        if (!pItem) {
            LOGE_ANALYZER("fid:%d, get free AwbProcRes faild !", frame_id);
#if RKAIQ_HAVE_DUMPSYS
            pAiqCore->mNoFreeBufCnt.awbProcRes++;
#endif
            return;
        }
        AlgoRstShared_t* pSahred = (AlgoRstShared_t*)pItem->_pData;
        pSahred->frame_id        = frame_id;
        AiqCoreMsg_t msg;
        msg.msg_id                    = XCAM_MESSAGE_AWB_PROC_RES_OK;
        msg.frame_id                  = frame_id;
        *(AlgoRstShared_t**)(msg.buf) = pSahred;
        ret                           = AiqCore_post_message(pAiqCore, &msg);
        AIQ_REF_BASE_UNREF(&pSahred->_ref_base);
    } else if (type == RK_AIQ_ALGO_TYPE_ABLC) {
        AiqPoolItem_t* pItem = aiqPool_getFree(pAiqCore->mProcResBlcSharedPool);
        if (!pItem) {
            LOGE_ANALYZER("fid:%d, get free BlcProcRes faild !", frame_id);
#if RKAIQ_HAVE_DUMPSYS
            pAiqCore->mNoFreeBufCnt.blcProcRes++;
#endif
            return;
        }
        AlgoRstShared_t* pSahred = (AlgoRstShared_t*)pItem->_pData;
        pSahred->frame_id        = frame_id;
        AiqCoreMsg_t msg;
        msg.msg_id                    = XCAM_MESSAGE_BLC_V32_PROC_RES_OK;
        msg.frame_id                  = frame_id;
        *(AlgoRstShared_t**)(msg.buf) = pSahred;
        ret                           = AiqCore_post_message(pAiqCore, &msg);
        AIQ_REF_BASE_UNREF(&pSahred->_ref_base);
    } else if (type == RK_AIQ_ALGO_TYPE_AYNR) {
        AiqPoolItem_t* pItem = aiqPool_getFree(pAiqCore->mProcResYnrSharedPool);
        if (!pItem) {
            LOGE_ANALYZER("fid:%d, get free YnrProcRes faild !", frame_id);
#if RKAIQ_HAVE_DUMPSYS
            pAiqCore->mNoFreeBufCnt.ynrProcRes++;
#endif
            return;
        }
        AlgoRstShared_t* pSahred = (AlgoRstShared_t*)pItem->_pData;
        pSahred->frame_id        = frame_id;
        AiqCoreMsg_t msg;
        msg.msg_id                    = XCAM_MESSAGE_YNR_V24_PROC_RES_OK;
        msg.frame_id                  = frame_id;
        *(AlgoRstShared_t**)(msg.buf) = pSahred;
        ret                           = AiqCore_post_message(pAiqCore, &msg);
        AIQ_REF_BASE_UNREF(&pSahred->_ref_base);
    }
}

static AiqFullParams_t* analyzeInternal(AiqCore_t * pAiqCore,
                                        enum rk_aiq_core_analyze_type_e grp_type) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqFullParams_t* pAiqFullParams = NULL;
    bool got_full_buf               = false;

    int cnts = 0;
    AiqAlgoHandler_t** pHdlList = NULL;
    pHdlList = AiqAnalyzeGroupManager_getGroupAlgoList(&pAiqCore->mRkAiqCoreGroupManager, grp_type, &cnts);

    for (int i = 0; i < cnts; i++) {
        AiqAlgoHandler_t* pHdl = pHdlList[i];
        bool got_buffer        = false;
        while (pHdl) {
            if (pHdl->mEnable) {
                int type = AiqAlgoHandler_getAlgoType(pHdl);
                if (!got_buffer) {
                    RkAiqAlgosGroupShared_t* shared =
                        pAiqCore->mAlogsGroupSharedParamsMap[pHdl->mGroupId];
                    uint32_t frame_id = -1;
                    if (shared) frame_id = shared->frameId;

                    if (!got_full_buf) {
                        aiqMutex_lock(&pAiqCore->_mFullParam_mutex);
                        if ((shared->frameId < pAiqCore->mLatestParamsDoneId ||
                                shared->frameId == pAiqCore->mLatestParamsDoneId) &&
                                pAiqCore->mLatestParamsDoneId != 0) {
                            LOGW_ANALYZER("[%u] <= [%u], skip grp_type 0x%x process !",
                                          shared->frameId, pAiqCore->mLatestParamsDoneId,
                                          grp_type);
                            aiqMutex_unlock(&pAiqCore->_mFullParam_mutex);
                            return NULL;
                        }
                        pending_params_t* pPendingParams = NULL;
                        AiqMapItem_t* pItem =
                            aiqMap_get(pAiqCore->mFullParamsPendingMap, (void*)(intptr_t)frame_id);
                        if (pItem) {
                            pPendingParams = (pending_params_t*)pItem->_pData;
                            pAiqFullParams = pPendingParams->fullParams;
                            LOGD_ANALYZER(
                                "[%d] pending params, algo_type: 0x%x, grp_type: 0x%x, "
                                "params_ptr: %p",
                                frame_id, type, grp_type, pAiqFullParams);
                        } else {
                            AiqPoolItem_t* pPoolItem =
                                aiqPool_getFree(pAiqCore->mAiqFullParamsPool);
                            if (!pPoolItem) {
                                LOGE_ANALYZER("cid:%d, fid:%d, no free aiq params buffer!",
                                              pAiqCore->mAlogsComSharedParams.mCamPhyId, frame_id);
                                // dump pending
                                bool rm = false;
                                AIQ_MAP_FOREACH(pAiqCore->mFullParamsPendingMap, pItem, rm) {
                                    pPendingParams = (pending_params_t*)pItem->_pData;
                                    LOGE_ANALYZER("pendings: fid:%d, grpmask:0x%lx", pItem->_key, pPendingParams->groupMasks);
                                }
                                aiqMutex_unlock(&pAiqCore->_mFullParam_mutex);
#if RKAIQ_HAVE_DUMPSYS
                                pAiqCore->mNoFreeBufCnt.fullParams++;
#endif
                                return NULL;
                            }
                            pAiqFullParams = (AiqFullParams_t*)pPoolItem->_pData;

                            LOGD_ANALYZER(
                                "[%d] new params, algo_type: 0x%x, grp_type: 0x%x, "
                                "params_ptr: %p",
                                frame_id, type, grp_type, pAiqFullParams);

                            pending_params_t temp;
                            aiq_memset(&temp, 0, sizeof(pending_params_t));
                            temp.fullParams = pAiqFullParams;
                            pItem = aiqMap_insert(pAiqCore->mFullParamsPendingMap, (void*)(intptr_t)frame_id,
                                                  &temp);
                            if (!pItem) {
                                LOGE_ANALYZER(
                                    "cid:%d, fid:%d, inert to pending map error\n",
                                    pAiqCore->mAlogsComSharedParams.mCamPhyId, frame_id);
                                AIQ_REF_BASE_UNREF(&pAiqFullParams->_base._ref_base);
                                aiqMutex_unlock(&pAiqCore->_mFullParam_mutex);
                                return NULL;
                            }
                            pPendingParams = (pending_params_t*)pItem->_pData;
                        }
                        pPendingParams->groupMasks |= 1ULL << grp_type;
                        if (pPendingParams->groupMasks ==
                                pAiqCore->mFullParamReqGroupsMasks)
                            pPendingParams->ready = true;
                        else
                            pPendingParams->ready = false;
                        got_full_buf = true;
                        aiqMutex_unlock(&pAiqCore->_mFullParam_mutex);
                    }
                    ret = getAiqParamsBuffer(pAiqCore, pAiqFullParams, type, frame_id);
                    if (ret) break;
                    shared->fullParams = pAiqFullParams;
                    got_buffer         = true;
                }
                if (true/*pAiqCore->mAlogsComSharedParams.init || !AiqCore_isGroupAlgo(pAiqCore, type)*/) {
                    if (pHdl->preProcess)
                        ret = pHdl->preProcess(pHdl);
                    if (ret) {
                        LOGW("cid[%d], preProc %d error", pAiqCore->mAlogsComSharedParams.mCamPhyId, type);
                        pHdl->genIspResult(pHdl, pAiqFullParams, pAiqCore->mAiqCurParams);
                        break;
                    }
                    if (pHdl->processing)
                        ret = pHdl->processing(pHdl);
                    if (ret) {
                        LOGW("cid[%d], Proc %d error", pAiqCore->mAlogsComSharedParams.mCamPhyId, type);
                        pHdl->genIspResult(pHdl, pAiqFullParams, pAiqCore->mAiqCurParams);
                        break;
                    }
                    if (pHdl->postProcess)
                        ret = pHdl->postProcess(pHdl);
                    pHdl->genIspResult(pHdl, pAiqFullParams, pAiqCore->mAiqCurParams);
                }
            }
            pHdl = AiqAlgoHandler_getNextHdl(pHdl);
        }
    }

    return pAiqFullParams;
}

XCamReturn AiqCore_prepare(AiqCore_t * pAiqCore,
                           const rk_aiq_exposure_sensor_descriptor* sensor_des, int mode) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    // check state
    if ((pAiqCore->mState == RK_AIQ_CORE_STATE_STARTED) ||
            (pAiqCore->mState == RK_AIQ_CORE_STATE_INVALID) ||
            (pAiqCore->mState == RK_AIQ_CORE_STATE_RUNNING)) {
        LOGW_ANALYZER("in state %d\n", pAiqCore->mState);
        return XCAM_RETURN_NO_ERROR;
    }

    bool res_changed =
        (pAiqCore->mAlogsComSharedParams.snsDes.isp_acq_width != 0) &&
        (sensor_des->isp_acq_width != pAiqCore->mAlogsComSharedParams.snsDes.isp_acq_width ||
         sensor_des->isp_acq_height != pAiqCore->mAlogsComSharedParams.snsDes.isp_acq_height);
    if (res_changed) {
        pAiqCore->mAlogsComSharedParams.conf_type |= RK_AIQ_ALGO_CONFTYPE_CHANGERES;
        LOGD_ANALYZER("resolution changed !");
    } else
        pAiqCore->mAlogsComSharedParams.conf_type &= ~RK_AIQ_ALGO_CONFTYPE_CHANGERES;

    if ((pAiqCore->mState == RK_AIQ_CORE_STATE_STOPED) ||
            (pAiqCore->mState == RK_AIQ_CORE_STATE_PREPARED)) {
        pAiqCore->mAlogsComSharedParams.conf_type |= RK_AIQ_ALGO_CONFTYPE_KEEPSTATUS;
        LOGD_ANALYZER("prepare from stopped, should keep algo status !");
    }
    if (mode == RK_AIQ_WORKING_MODE_NORMAL) {
        pAiqCore->mAlogsComSharedParams.hdr_mode = 0;
    } else if (RK_AIQ_HDR_IS_HDR2(mode)) {
        pAiqCore->mAlogsComSharedParams.hdr_mode = 1;
    } else if (RK_AIQ_HDR_IS_HDR3(mode)) {
        pAiqCore->mAlogsComSharedParams.hdr_mode = 2;
    }

    pAiqCore->mAlogsComSharedParams.snsDes = *sensor_des;
    pAiqCore->mAlogsComSharedParams.working_mode    = mode;
    pAiqCore->mAlogsComSharedParams.spWidth         = pAiqCore->mSpWidth;
    pAiqCore->mAlogsComSharedParams.spHeight        = pAiqCore->mSpHeight;
    pAiqCore->mAlogsComSharedParams.spAlignedWidth  = pAiqCore->mSpAlignedWidth;
    pAiqCore->mAlogsComSharedParams.spAlignedHeight = pAiqCore->mSpAlignedHeight;

    CalibDbV2_ColorAsGrey_t* colorAsGrey = (CalibDbV2_ColorAsGrey_t*)CALIBDBV2_GET_MODULE_PTR(
            (void*)(pAiqCore->mAlogsComSharedParams.calibv2), colorAsGrey);
    if ((pAiqCore->mAlogsComSharedParams.snsDes.sensor_pixelformat == V4L2_PIX_FMT_GREY) ||
            (pAiqCore->mAlogsComSharedParams.snsDes.sensor_pixelformat == V4L2_PIX_FMT_Y10) ||
            (pAiqCore->mAlogsComSharedParams.snsDes.sensor_pixelformat == V4L2_PIX_FMT_Y12)) {
        pAiqCore->mAlogsComSharedParams.is_bw_sensor = true;
        pAiqCore->mGrayMode                          = RK_AIQ_GRAY_MODE_ON;
        pAiqCore->mAlogsComSharedParams.gray_mode    = true;
    } else {
        pAiqCore->mAlogsComSharedParams.is_bw_sensor = false;
        if (colorAsGrey->param.enable) {
            pAiqCore->mAlogsComSharedParams.gray_mode = true;
            pAiqCore->mGrayMode                       = RK_AIQ_GRAY_MODE_ON;
        } else {
            pAiqCore->mGrayMode                       = RK_AIQ_GRAY_MODE_OFF;
            pAiqCore->mAlogsComSharedParams.gray_mode = false;
        }
    }

#if defined(RKAIQ_HAVE_MULTIISP)

    if (pAiqCore->mHwInfo.is_multi_isp_mode) {
        XCAM_ASSERT((sensor_des->isp_acq_width % 32 == 0));  // &&
        //(sensor_des->isp_acq_height % 16 == 0));
        uint32_t extended_pixel          = pAiqCore->mHwInfo.multi_isp_extended_pixel;
        AiqStatsTranslator_t* translator = pAiqCore->mTranslator;
        uint32_t size                    = sensor_des->isp_acq_width * sensor_des->isp_acq_height;
        int isp_unite_mode               = translator->mIspUniteMode;
        if (isp_unite_mode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
            AiqStatsTranslator_SetMultiIspMode(translator, true);
            AiqStatsTranslator_SetIspUniteMode(translator, RK_AIQ_ISP_UNITE_MODE_FOUR_GRID);
            Rectangle_t rect;
            rect.x = 0;
            rect.y = 0;
            rect.w = sensor_des->isp_acq_width;
            rect.h = sensor_des->isp_acq_height;
            AiqStatsTranslator_SetPicInfo(translator, &rect);
            rect.x = 0;
            rect.y = 0;
            rect.w = sensor_des->isp_acq_width / 2 + extended_pixel;
            rect.h = sensor_des->isp_acq_height / 2 + extended_pixel;
            AiqStatsTranslator_SetLeftIspRect(translator, &rect);
            rect.x = sensor_des->isp_acq_width / 2 - extended_pixel;
            rect.y = 0;
            rect.w = sensor_des->isp_acq_width / 2 + extended_pixel;
            rect.h = sensor_des->isp_acq_height / 2 + extended_pixel;
            AiqStatsTranslator_SetRightIspRect(translator, &rect);
            rect.x = 0;
            rect.y = sensor_des->isp_acq_height / 2 - extended_pixel;
            rect.w = sensor_des->isp_acq_width / 2 + extended_pixel;
            rect.h = sensor_des->isp_acq_height / 2 + extended_pixel;
            AiqStatsTranslator_SetBottomLeftIspRect(translator, &rect);
            rect.x = sensor_des->isp_acq_width / 2 - extended_pixel;
            rect.y = sensor_des->isp_acq_height / 2 - extended_pixel;
            rect.w = sensor_des->isp_acq_width / 2 + extended_pixel;
            rect.h = sensor_des->isp_acq_height / 2 + extended_pixel;
            AiqStatsTranslator_SetBottomRightIspRect(translator, &rect);
        } else if (isp_unite_mode == RK_AIQ_ISP_UNITE_MODE_TWO_GRID) {
            AiqStatsTranslator_SetMultiIspMode(translator, true);
            AiqStatsTranslator_SetIspUniteMode(translator, RK_AIQ_ISP_UNITE_MODE_TWO_GRID);
            Rectangle_t rect;
            rect.x = 0;
            rect.y = 0;
            rect.w = sensor_des->isp_acq_width;
            rect.h = sensor_des->isp_acq_height;
            AiqStatsTranslator_SetPicInfo(translator, &rect);
            rect.x = 0;
            rect.y = 0;
            rect.w = sensor_des->isp_acq_width / 2 + extended_pixel;
            rect.h = sensor_des->isp_acq_height;
            AiqStatsTranslator_SetLeftIspRect(translator, &rect);
            rect.x = sensor_des->isp_acq_width / 2 - extended_pixel;
            AiqStatsTranslator_SetRightIspRect(translator, &rect);
        } else {
            AiqStatsTranslator_SetMultiIspMode(translator, true);
            AiqStatsTranslator_SetIspUniteMode(translator, RK_AIQ_ISP_UNITE_MODE_NORMAL);
            Rectangle_t rect;
            rect.x = 0;
            rect.y = 0;
            rect.w = sensor_des->isp_acq_width;
            rect.h = sensor_des->isp_acq_height;
            AiqStatsTranslator_SetPicInfo(translator, &rect);
            AiqStatsTranslator_SetLeftIspRect(translator, &rect);
            AiqStatsTranslator_SetRightIspRect(translator, &rect);
        }
        Rectangle_t f = AiqStatsTranslator_GetPicInfo(translator);
        Rectangle_t l = AiqStatsTranslator_GetLeftIspRect(translator);
        Rectangle_t r = AiqStatsTranslator_GetRightIspRect(translator);
        LOGD_ANALYZER(
            "Set Multi-ISP mode Translator info :"
            " F: { %u, %u, %u, %u }"
            " L: { %u, %u, %u, %u }"
            " R: { %u, %u, %u, %u }",
            f.x, f.y, f.w, f.h, l.x, l.y, l.w, l.h, r.x, r.y, r.w, r.h);
        pAiqCore->mAlogsComSharedParams.is_multi_isp_mode         = pAiqCore->mHwInfo.is_multi_isp_mode;
        pAiqCore->mAlogsComSharedParams.multi_isp_extended_pixels = extended_pixel;
    }
#endif

    AiqStatsTranslator_setWorkingMode(pAiqCore->mTranslator, mode);

    for (int i = 0; i < RK_AIQ_ALGO_TYPE_MAX; i++) {
        AiqAlgoHandler_t* pHandle = pAiqCore->mAlgoHandleMaps[i];
        while (pHandle) {
            if (AiqAlgoHandler_getEnable(pHandle)) {
                AiqAlgoHandler_setReConfig(pHandle, true);
                pHandle->prepare(pHandle);
                RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d prepare failed",
                                       AiqAlgoHandler_getAlgoType(pHandle));
            }
            pHandle = AiqAlgoHandler_getNextHdl(pHandle);
        }
    }
    pAiqCore->mAlogsComSharedParams.init = true;
    // run algos without stats to generate
    // initial params
    for (int i = 0; i < RK_AIQ_CORE_ANALYZE_MAX; i++) {
        RkAiqAlgosGroupShared_t* shared = pAiqCore->mAlogsGroupSharedParamsMap[i];
        if (shared) {
            if (!(pAiqCore->mAlogsComSharedParams.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGERES)) {
                clearAlgosGroupSharedParams(shared);
            } else {
                shared->frameId = 0;
                shared->sof     = 0;
            }
        }
    }
    analyzeInternal(pAiqCore, RK_AIQ_CORE_ANALYZE_ALL);
    clearFullParamsPendingMap(pAiqCore);
    for (uint32_t i = 0; i < RK_AIQ_CORE_ANALYZE_MAX; i++) {
        if (pAiqCore->mAlogsGroupSharedParamsMap[i]) freeGroupSharebuf(pAiqCore->mAlogsGroupSharedParamsMap[i]);
    }

    pAiqCore->mAlogsComSharedParams.init = false;

    pAiqCore->mState = RK_AIQ_CORE_STATE_PREPARED;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn handleAecStats(AiqCore_t* pAiqCore, const aiq_VideoBuffer_t* buffer) {
    XCamReturn ret             = XCAM_RETURN_NO_ERROR;
    aiq_stats_base_t* aecStats = NULL;

    if (!pAiqCore->mAiqAecStatsPool) {
        return XCAM_RETURN_BYPASS;
    }

    AiqPoolItem_t* pItem = aiqPool_getFree(pAiqCore->mAiqAecStatsPool);
    if (!pItem) {
        LOGW_AEC("fid:%d no free aecStats buffer!", AiqVideoBuffer_getSequence(buffer));
#if RKAIQ_HAVE_DUMPSYS
        pAiqCore->mNoFreeBufCnt.aeStats++;
#endif
        return XCAM_RETURN_BYPASS;
    }
    aecStats = (aiq_stats_base_t*)pItem->_pData;
    ret      = pAiqCore->mTranslator->translateAecStats(pAiqCore->mTranslator, buffer, aecStats);
    if (ret < 0) {
        LOGE_AEC("translate aec stats failed!");
        AIQ_REF_BASE_UNREF(&aecStats->_ref_base);
        return XCAM_RETURN_BYPASS;
    }

    aiqMutex_lock(&pAiqCore->mIspStatsMutex);
    if (pAiqCore->mCurAeStats) AIQ_REF_BASE_UNREF(&pAiqCore->mCurAeStats->_ref_base);

    pAiqCore->mCurAeStats = aecStats;
#if RKAIQ_HAVE_DUMPSYS
    if (aecStats->bValid)
        pAiqCore->mIsStatsAvail.ae = true;
    else
        pAiqCore->mIsStatsAvail.ae = false;
#endif
    AIQ_REF_BASE_REF(&aecStats->_ref_base);
    aiqMutex_unlock(&pAiqCore->mIspStatsMutex);

    AiqCoreMsg_t msg;
    msg.msg_id                     = XCAM_MESSAGE_AEC_STATS_OK;
    msg.frame_id                   = AiqVideoBuffer_getSequence(buffer);
    *(aiq_stats_base_t**)(msg.buf) = aecStats;
    ret                            = AiqCore_post_message(pAiqCore, &msg);
    AIQ_REF_BASE_UNREF(&aecStats->_ref_base);

    return ret;
}

static XCamReturn handleAwbStats(AiqCore_t* pAiqCore, const aiq_VideoBuffer_t* buffer) {
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    aiq_stats_base_t* awbStat = NULL;

    if (!pAiqCore->mAiqAwbStatsPool) {
        return XCAM_RETURN_BYPASS;
    }

    AiqPoolItem_t* pItem = aiqPool_getFree(pAiqCore->mAiqAwbStatsPool);
    if (!pItem) {
        LOGW_AWB("no free awbStat buffer!");
#if RKAIQ_HAVE_DUMPSYS
        pAiqCore->mNoFreeBufCnt.awbStats++;
#endif
        return XCAM_RETURN_BYPASS;
    }
    awbStat = (aiq_stats_base_t*)pItem->_pData;
    ret     = pAiqCore->mTranslator->translateAwbStats(pAiqCore->mTranslator, buffer, awbStat);
    if (ret < 0) {
        LOGE_AWB("translate awb stats failed!");
        AIQ_REF_BASE_UNREF(&awbStat->_ref_base);
        return XCAM_RETURN_BYPASS;
    }

    aiqMutex_lock(&pAiqCore->mIspStatsMutex);
    if (pAiqCore->mCurAwbStats) AIQ_REF_BASE_UNREF(&pAiqCore->mCurAwbStats->_ref_base);

    pAiqCore->mCurAwbStats = awbStat;
#if RKAIQ_HAVE_DUMPSYS
    if (awbStat->bValid)
        pAiqCore->mIsStatsAvail.awb = true;
    else
        pAiqCore->mIsStatsAvail.awb = false;
#endif
    AIQ_REF_BASE_REF(&awbStat->_ref_base);
    aiqMutex_unlock(&pAiqCore->mIspStatsMutex);

    AiqCoreMsg_t msg;
    msg.msg_id                     = XCAM_MESSAGE_AWB_STATS_OK;
    msg.frame_id                   = AiqVideoBuffer_getSequence(buffer);
    *(aiq_stats_base_t**)(msg.buf) = awbStat;
    ret                            = AiqCore_post_message(pAiqCore, &msg);
    AIQ_REF_BASE_UNREF(&awbStat->_ref_base);

    return ret;
}

static void matchPdafAndAfStats(AiqCore_t* pAiqCore) {
    bool ispOnline;

    ispOnline = (pAiqCore->mPdafType == PDAF_SENSOR_TYPE3) ? true : pAiqCore->mIspOnline;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if (!pAiqCore->mAfStats[i] || !pAiqCore->mPdafStats[j]) continue;
            if (((ABS(pAiqCore->mAfStatsTime[i] - pAiqCore->mPdafStatsTime[j]) <
                    pAiqCore->mFrmInterval / 2LL) &&
                    ispOnline) ||
                    ((pAiqCore->mAfStatsTime[i] - pAiqCore->mPdafStatsTime[j] <
                      pAiqCore->mFrmInterval) &&
                     (pAiqCore->mAfStatsTime[i] >= pAiqCore->mPdafStatsTime[j]) &&
                     !ispOnline)) {
                LOGD_AF("Match: i %d, j %d, id %d, %d, time %lld, %lld, diff %lld", i,
                        j, pAiqCore->mAfStatsFrmId[i], pAiqCore->mPdafStatsFrmId[j],
                        pAiqCore->mAfStatsTime[i], pAiqCore->mPdafStatsTime[j],
                        pAiqCore->mAfStatsTime[i] - pAiqCore->mPdafStatsTime[j]);

                AiqCoreMsg_t msg;
                msg.msg_id                     = XCAM_MESSAGE_AF_STATS_OK;
                msg.frame_id                   = pAiqCore->mAfStatsFrmId[i];
                *(aiq_stats_base_t**)(msg.buf) = pAiqCore->mAfStats[i];
                AIQ_REF_BASE_REF(&pAiqCore->mAfStats[i]->_ref_base);
                AiqCore_post_message(pAiqCore, &msg);
                AIQ_REF_BASE_UNREF(&pAiqCore->mAfStats[i]->_ref_base);

                msg.msg_id                     = XCAM_MESSAGE_PDAF_STATS_OK;
                msg.frame_id                   = pAiqCore->mAfStatsFrmId[i];
                *(aiq_stats_base_t**)(msg.buf) = pAiqCore->mPdafStats[j];
                AIQ_REF_BASE_REF(&pAiqCore->mPdafStats[j]->_ref_base);
                AiqCore_post_message(pAiqCore, &msg);
                AIQ_REF_BASE_UNREF(&pAiqCore->mPdafStats[j]->_ref_base);

                AIQ_REF_BASE_UNREF(&pAiqCore->mAfStats[i]->_ref_base);
                pAiqCore->mAfStats[i]      = NULL;
                pAiqCore->mAfStatsFrmId[i] = (uint32_t) -1;
                if (i == 0) {
                    if (pAiqCore->mAfStats[1])
                        AIQ_REF_BASE_UNREF(&pAiqCore->mAfStats[1]->_ref_base);
                    pAiqCore->mAfStats[1]      = NULL;
                    pAiqCore->mAfStatsFrmId[1] = (uint32_t) -1;
                }
                AIQ_REF_BASE_UNREF(&pAiqCore->mPdafStats[j]->_ref_base);
                pAiqCore->mPdafStats[j]      = NULL;
                pAiqCore->mPdafStatsFrmId[j] = (uint32_t) -1;
                if (j == 0) {
                    if (pAiqCore->mPdafStats[1])
                        AIQ_REF_BASE_UNREF(&pAiqCore->mPdafStats[1]->_ref_base);
                    pAiqCore->mPdafStats[1]      = NULL;
                    pAiqCore->mPdafStatsFrmId[1] = (uint32_t) -1;
                }
            }
        }
    }
}

static XCamReturn handleAfStats(AiqCore_t* pAiqCore, const aiq_VideoBuffer_t* buffer) {
    XCamReturn ret           = XCAM_RETURN_NO_ERROR;
    aiq_stats_base_t* afStat = NULL;

    if (!pAiqCore->mAiqAfStatsPool) {
        return XCAM_RETURN_BYPASS;
    }

    AiqPoolItem_t* pItem = aiqPool_getFree(pAiqCore->mAiqAfStatsPool);
    if (!pItem) {
        LOGW_AF("no free afStat buffer!");
        return XCAM_RETURN_BYPASS;
    }
    afStat = (aiq_stats_base_t*)pItem->_pData;
    ret    = pAiqCore->mTranslator->translateAfStats(pAiqCore->mTranslator, buffer, afStat);
    if (ret < 0) {
        LOGE_AF("translate af stats failed!");
        AIQ_REF_BASE_UNREF(&afStat->_ref_base);
        return XCAM_RETURN_BYPASS;
    }

    aiqMutex_lock(&pAiqCore->mIspStatsMutex);
    if (pAiqCore->mCurAfStats) AIQ_REF_BASE_UNREF(&pAiqCore->mCurAfStats->_ref_base);

    pAiqCore->mCurAfStats = afStat;
#if RKAIQ_HAVE_DUMPSYS
    if (afStat->bValid)
        pAiqCore->mIsStatsAvail.af = true;
    else
        pAiqCore->mIsStatsAvail.af = false;
#endif
    AIQ_REF_BASE_REF(&afStat->_ref_base);
    aiqMutex_unlock(&pAiqCore->mIspStatsMutex);

    if (pAiqCore->mPdafSupport) {
        aiqMutex_lock(&pAiqCore->mPdafStatsMutex);
        if (pAiqCore->mAfStats[1]) AIQ_REF_BASE_UNREF(&pAiqCore->mAfStats[1]->_ref_base);
        pAiqCore->mAfStats[1]      = pAiqCore->mAfStats[0];
        pAiqCore->mAfStatsFrmId[1] = pAiqCore->mAfStatsFrmId[0];
        pAiqCore->mAfStatsTime[1]  = pAiqCore->mAfStatsTime[0];
        pAiqCore->mAfStats[0]      = afStat;
        AIQ_REF_BASE_REF(&afStat->_ref_base);

        pAiqCore->mAfStatsFrmId[0] = AiqVideoBuffer_getSequence(buffer);
        pAiqCore->mAfStatsTime[0]  = AiqVideoBuffer_getTimestamp(buffer);
        //LOGD_AF("%s: mAfStatsFrmId %d, %d, mPdafStatsFrmId %d, %d, mAfStatsTime %lld, %lld, mPdafStatsTime %lld, %lld, mFrmInterval %lld, mIspOnline %d",
        //    __func__, pAiqCore->mAfStatsFrmId[0], pAiqCore->mAfStatsFrmId[1], pAiqCore->mPdafStatsFrmId[0], pAiqCore->mPdafStatsFrmId[1],
        //    pAiqCore->mAfStatsTime[0], pAiqCore->mAfStatsTime[1], pAiqCore->mPdafStatsTime[0], pAiqCore->mPdafStatsTime[1], pAiqCore->mFrmInterval,
        //    pAiqCore->mIspOnline);
        matchPdafAndAfStats(pAiqCore);
        aiqMutex_unlock(&pAiqCore->mPdafStatsMutex);
    } else {
        AiqCoreMsg_t msg;
        msg.msg_id                     = XCAM_MESSAGE_AF_STATS_OK;
        msg.frame_id                   = AiqVideoBuffer_getSequence(buffer);
        *(aiq_stats_base_t**)(msg.buf) = afStat;
        AIQ_REF_BASE_REF(&afStat->_ref_base);
        ret = AiqCore_post_message(pAiqCore, &msg);
        AIQ_REF_BASE_UNREF(&afStat->_ref_base);
    }

    AIQ_REF_BASE_UNREF(&afStat->_ref_base);

    return ret;
}

#if RKAIQ_HAVE_PDAF
static XCamReturn handlePdafStats(AiqCore_t* pAiqCore, const rk_aiq_isp_pdaf_meas_t* pdaf_meas, const aiq_VideoBuffer_t* buffer) {
    XCamReturn ret              = XCAM_RETURN_NO_ERROR;
    aiq_stats_base_t* pdafStats = NULL;
    bool sns_mirror             = pAiqCore->mAlogsComSharedParams.sns_mirror;

    if (!pAiqCore->mAiqPdafStatsPool) {
        return XCAM_RETURN_BYPASS;
    }

    AiqPoolItem_t* pItem = aiqPool_getFree(pAiqCore->mAiqPdafStatsPool);
    if (!pItem) {
        LOGW_AF("no free pdafStat buffer!");
        return XCAM_RETURN_BYPASS;
    }
    pdafStats = (aiq_stats_base_t*)pItem->_pData;
    ret       = pAiqCore->mTranslator->translatePdafStats(pAiqCore->mTranslator, pdaf_meas, buffer, pdafStats,
                sns_mirror);
    if (ret < 0) {
        LOGE_AF("translate pdaf stats failed!");
        AIQ_REF_BASE_UNREF(&pdafStats->_ref_base);
        return XCAM_RETURN_BYPASS;
    }

    aiqMutex_lock(&pAiqCore->mPdafStatsMutex);
    if (pAiqCore->mPdafStats[1]) AIQ_REF_BASE_UNREF(&pAiqCore->mPdafStats[1]->_ref_base);
    pAiqCore->mPdafStats[1]      = pAiqCore->mPdafStats[0];
    pAiqCore->mPdafStatsFrmId[1] = pAiqCore->mPdafStatsFrmId[0];
    pAiqCore->mPdafStatsTime[1]  = pAiqCore->mPdafStatsTime[0];
    pAiqCore->mPdafStats[0]                = pdafStats;
    AIQ_REF_BASE_REF(&pdafStats->_ref_base);
    pAiqCore->mPdafStatsFrmId[0] = AiqVideoBuffer_getSequence(buffer);
    pAiqCore->mPdafStatsTime[0]  = AiqVideoBuffer_getTimestamp(buffer);
    //LOGD_AF("%s: mAfStatsFrmId %d, %d, mPdafStatsFrmId %d, %d, mAfStatsTime %lld, %lld, mPdafStatsTime %lld, %lld, mFrmInterval %lld, mIspOnline %d",
    //    __func__, pAiqCore->mAfStatsFrmId[0], pAiqCore->mAfStatsFrmId[1], pAiqCore->mPdafStatsFrmId[0], pAiqCore->mPdafStatsFrmId[1],
    //    pAiqCore->mAfStatsTime[0], pAiqCore->mAfStatsTime[1], pAiqCore->mPdafStatsTime[0], pAiqCore->mPdafStatsTime[1], pAiqCore->mFrmInterval,
    //    pAiqCore->mIspOnline);
    matchPdafAndAfStats(pAiqCore);
    aiqMutex_unlock(&pAiqCore->mPdafStatsMutex);
    AIQ_REF_BASE_UNREF(&pdafStats->_ref_base);
    return XCAM_RETURN_NO_ERROR;
}
#endif

static XCamReturn handleAdehazeStats(AiqCore_t* pAiqCore, const aiq_VideoBuffer_t* buffer) {
    XCamReturn ret               = XCAM_RETURN_NO_ERROR;
    aiq_stats_base_t* dehazeStat = NULL;

    if (!pAiqCore->mAiqAdehazeStatsPool) {
        return XCAM_RETURN_BYPASS;
    }

    AiqPoolItem_t* pItem = aiqPool_getFree(pAiqCore->mAiqAdehazeStatsPool);
    if (!pItem) {
        LOGW_ADEHAZE("no free dehazeStat buffer!");
#if RKAIQ_HAVE_DUMPSYS
        pAiqCore->mNoFreeBufCnt.dhazStats++;
#endif
        return XCAM_RETURN_BYPASS;
    }
    dehazeStat = (aiq_stats_base_t*)pItem->_pData;
    ret = pAiqCore->mTranslator->translateAdehazeStats(pAiqCore->mTranslator, buffer, dehazeStat);
    if (ret < 0) {
        LOGE_ADEHAZE("translate dehazeStat failed!");
        AIQ_REF_BASE_UNREF(&dehazeStat->_ref_base);
        return XCAM_RETURN_BYPASS;
    }

    AiqCoreMsg_t msg;
    msg.msg_id                     = XCAM_MESSAGE_ADEHAZE_STATS_OK;
    msg.frame_id                   = AiqVideoBuffer_getSequence(buffer);
    *(aiq_stats_base_t**)(msg.buf) = dehazeStat;
    ret                            = AiqCore_post_message(pAiqCore, &msg);
#if RKAIQ_HAVE_DUMPSYS
    if (dehazeStat->bValid)
        pAiqCore->mIsStatsAvail.dhaze = true;
    else
        pAiqCore->mIsStatsAvail.dhaze = false;
#endif
    AIQ_REF_BASE_UNREF(&dehazeStat->_ref_base);

    return ret;
}

#if RK_GAIN_V2_ENABLE_GAIN2DDR
static XCamReturn handleAgainStats(AiqCore_t* pAiqCore, const aiq_VideoBuffer_t* buffer) {
    XCamReturn ret             = XCAM_RETURN_NO_ERROR;
    aiq_stats_base_t* gainStat = NULL;

    if (!pAiqCore->mAiqAgainStatsPool) {
        return XCAM_RETURN_BYPASS;
    }

    AiqPoolItem_t* pItem = aiqPool_getFree(pAiqCore->mAiqAgainStatsPool);
    if (!pItem) {
        LOGW("no free gainStat buffer!");
#if RKAIQ_HAVE_DUMPSYS
        pAiqCore->mNoFreeBufCnt.gainStats++;
#endif
        return XCAM_RETURN_BYPASS;
    }
    gainStat = (aiq_stats_base_t*)pItem->_pData;
    if (pAiqCore->mTranslator->translateAgainStats) {
        ret = pAiqCore->mTranslator->translateAgainStats(pAiqCore->mTranslator, buffer, gainStat);
        if (ret < 0) {
            LOGE("translate gainStat failed!");
            AIQ_REF_BASE_UNREF(&gainStat->_ref_base);
            return XCAM_RETURN_BYPASS;
        }

        AiqCoreMsg_t msg;
        msg.msg_id                     = XCAM_MESSAGE_AGAIN_STATS_OK;
        msg.frame_id                   = AiqVideoBuffer_getSequence(buffer);
        *(aiq_stats_base_t**)(msg.buf) = gainStat;
        ret                            = AiqCore_post_message(pAiqCore, &msg);
#if RKAIQ_HAVE_DUMPSYS
        if (gainStat->bValid)
            pAiqCore->mIsStatsAvail.gain = true;
        else
            pAiqCore->mIsStatsAvail.gain = false;
#endif
        AIQ_REF_BASE_UNREF(&gainStat->_ref_base);
    }
    return ret;
}
#endif

static XCamReturn handleBay3dStats(AiqCore_t* pAiqCore, const aiq_VideoBuffer_t* buffer) {
    AiqAlgoHandler_t* pHdl = pAiqCore->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AMFNR];
    if (!pHdl || !pHdl->mEnable) {
        LOGW_ANALYZER("AMFNR not run !");
        return XCAM_RETURN_BYPASS;
    }

    RkAiqAlgosGroupShared_t* grpShared = pAiqCore->mAlogsGroupSharedParamsMap[pHdl->mGroupId];
    if (!grpShared) {
        LOGW_ANALYZER("Fail to get the shared of AMFNR!");
        return XCAM_RETURN_BYPASS;
    }
    XCamReturn ret = pAiqCore->mTranslator->translateBay3dStats(
                         pAiqCore->mTranslator, buffer, pAiqCore->bay3dStatList, pAiqCore->mbay3dStatPool,
                         &pAiqCore->bay3dStatListMutex);
    if (ret) {
        LOGE_ANALYZER("translate bay3d stats failed!");
#if RKAIQ_HAVE_DUMPSYS
        pAiqCore->mNoFreeBufCnt.btnrStats++;
#endif
        ret = XCAM_RETURN_BYPASS;
    }

#if RKAIQ_HAVE_DUMPSYS
    if (ret == XCAM_RETURN_NO_ERROR)
        pAiqCore->mIsStatsAvail.btnr = true;
    else
        pAiqCore->mIsStatsAvail.btnr = false;
#endif

    return ret;
}

static XCamReturn handleVicapScaleBufs(AiqCore_t* pAiqCore, aiq_VideoBuffer_t* buffer) {
    XCamReturn ret             = XCAM_RETURN_NO_ERROR;
    AiqV4l2Buffer_t* buf_proxy = (AiqV4l2Buffer_t*)buffer;
    int* reserved              = (int*)AiqV4l2Buffer_getReserved(buf_proxy);
    int raw_index              = reserved[0];
    int bpp                    = reserved[1];
    int mode                   = pAiqCore->mAlogsComSharedParams.working_mode;
    uint32_t frameId           = AiqV4l2Buffer_getSequence(buf_proxy);

    if (!pAiqCore->mVicapBufs) {
        pAiqCore->mVicapBufs = aiq_mallocz(sizeof(RkAiqVicapRawBufInfo_t));
        if (!pAiqCore->mVicapBufs) return XCAM_RETURN_ERROR_MEM;
        pAiqCore->mVicapBufs->frame_id = -1;
    }

    if (frameId > pAiqCore->mVicapBufs->frame_id && pAiqCore->mVicapBufs->flags != 0) {
        /* over run */
        LOGE_ANALYZER("frame id: %d buf flags: %#x scale raw buf unready and force to release\n",
                      pAiqCore->mVicapBufs->frame_id, pAiqCore->mVicapBufs->flags);
        resetVicapScalbuf(pAiqCore->mVicapBufs);
    }

    if (mode == RK_AIQ_WORKING_MODE_NORMAL ||
            RK_AIQ_HDR_IS_SENSOR_BUILTIN(mode)) {
        pAiqCore->mVicapBufs->frame_id = frameId;
        if (pAiqCore->mVicapBufs->raw_s) AiqVideoBuffer_unref(pAiqCore->mVicapBufs->raw_s);
        pAiqCore->mVicapBufs->raw_s    = buffer;
        AiqVideoBuffer_ref(buffer);
        pAiqCore->mVicapBufs->ready = true;
        pAiqCore->mVicapBufs->flags = pAiqCore->mVicapBufs->flags | (1 << raw_index);
    } else if (mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR || mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
        pAiqCore->mVicapBufs->frame_id = frameId;
        if (raw_index == 0) {
            pAiqCore->mVicapBufs->raw_l = buffer;
            if (pAiqCore->mVicapBufs->raw_l) AiqVideoBuffer_unref(pAiqCore->mVicapBufs->raw_l);
            AiqVideoBuffer_ref(buffer);
            pAiqCore->mVicapBufs->flags = pAiqCore->mVicapBufs->flags | (1 << raw_index);
        } else if (raw_index == 1) {
            if (pAiqCore->mVicapBufs->raw_s) AiqVideoBuffer_unref(pAiqCore->mVicapBufs->raw_s);
            pAiqCore->mVicapBufs->raw_s = buffer;
            AiqVideoBuffer_ref(buffer);
            pAiqCore->mVicapBufs->flags = pAiqCore->mVicapBufs->flags | (1 << raw_index);
        }
        if (!(pAiqCore->mVicapBufs->flags ^ RK_AIQ_VICAP_SCALE_HDR_MODE_2_HDR)) {
            pAiqCore->mVicapBufs->ready = true;
        }
    } else if (mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR || mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
        pAiqCore->mVicapBufs->frame_id = frameId;
        if (raw_index == 0) {
            if (pAiqCore->mVicapBufs->raw_l) AiqVideoBuffer_unref(pAiqCore->mVicapBufs->raw_l);
            pAiqCore->mVicapBufs->raw_l = buffer;
            AiqVideoBuffer_ref(buffer);
            pAiqCore->mVicapBufs->flags = pAiqCore->mVicapBufs->flags | (1 << raw_index);
        } else if (raw_index == 1) {
            if (pAiqCore->mVicapBufs->raw_m) AiqVideoBuffer_unref(pAiqCore->mVicapBufs->raw_m);
            pAiqCore->mVicapBufs->raw_m = buffer;
            AiqVideoBuffer_ref(buffer);
            pAiqCore->mVicapBufs->flags = pAiqCore->mVicapBufs->flags | (1 << raw_index);
        } else if (raw_index == 2) {
            if (pAiqCore->mVicapBufs->raw_s) AiqVideoBuffer_unref(pAiqCore->mVicapBufs->raw_s);
            pAiqCore->mVicapBufs->raw_s = buffer;
            AiqVideoBuffer_ref(buffer);
            pAiqCore->mVicapBufs->flags = pAiqCore->mVicapBufs->flags | (1 << raw_index);
        }
        if (!(pAiqCore->mVicapBufs->flags ^ RK_AIQ_VICAP_SCALE_HDR_MODE_3_HDR)) {
            pAiqCore->mVicapBufs->ready = true;
        }
    }
    if (pAiqCore->mVicapBufs->ready && pAiqCore->mVicapBufs->frame_id > 2) {
        AiqCoreMsg_t msg;
        msg.msg_id                    = XCAM_MESSAGE_VICAP_POLL_SCL_OK;
        msg.frame_id                  = pAiqCore->mVicapBufs->frame_id;
        rk_aiq_scale_raw_info_t* info = (rk_aiq_scale_raw_info_t*)(msg.buf);
        info->bpp                     = bpp;
        info->flags = pAiqCore->mVicapBufs->flags;

        info->raw_s = pAiqCore->mVicapBufs->raw_s;
        info->raw_l = pAiqCore->mVicapBufs->raw_l;
        info->raw_m = pAiqCore->mVicapBufs->raw_m;

        ret = AiqCore_post_message(pAiqCore, &msg);

        resetVicapScalbuf(pAiqCore->mVicapBufs);
    }
    return ret;
}

typedef enum AIQ_STATS_TYPE_e {
    AIQ_STATS_TYPE_AEC,
    AIQ_STATS_TYPE_AWB,
    AIQ_STATS_TYPE_AF,
    AIQ_STATS_TYPE_DHAZ,
    AIQ_STATS_TYPE_BAY3D,
} AIQ_STATS_TYPE_t;

static bool _check_stats_valid(aiq_VideoBuffer_t* buffer, AIQ_STATS_TYPE_t check_type)
{
#if defined(ISP_HW_V39)
    struct rkisp39_stat_buffer* stats =
        (struct rkisp39_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)buffer));
#elif defined(ISP_HW_V33)
    struct rkisp33_stat_buffer* stats =
        (struct rkisp33_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)buffer));
#elif defined(ISP_HW_V35)
    struct rkisp35_stat_buffer* stats =
        (struct rkisp35_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)buffer));
#else
#error "wrong plateform !"
#endif
    uint32_t meas_type = stats->meas_type;
    LOGD_ANALYZER("sequence(%d), meas_type:0x%x",
                  AiqVideoBuffer_getSequence(buffer), meas_type);
    if (AIQ_STATS_TYPE_AEC == check_type)
        return (!!(meas_type & ISP2X_STAT_RAWAE0) && !!(meas_type & ISP2X_STAT_RAWHST0));
    if (AIQ_STATS_TYPE_DHAZ == check_type)
        return !!(meas_type & ISP2X_STAT_DHAZ);
    if (AIQ_STATS_TYPE_AWB == check_type)
        return !!(meas_type & ISP2X_STAT_RAWAWB);
    if (AIQ_STATS_TYPE_AF == check_type)
        return !!(meas_type & ISP2X_STAT_RAWAF);
    if (AIQ_STATS_TYPE_BAY3D == check_type)
        return !!(meas_type & ISP39_STAT_BAY3D);
    return false;
}

static XCamReturn RkAiqCore_analyze(AiqCore_t * pAiqCore, AiqHwEvt_t * evt) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pAiqCore->mApiMutex);
    pAiqCore->mSafeEnableAlgo = false;
    aiqMutex_unlock(&pAiqCore->mApiMutex);

    if (!pAiqCore->firstStatsReceived && pAiqCore->mState != RK_AIQ_CORE_STATE_STOPED) {
        pAiqCore->firstStatsReceived = true;
        pAiqCore->mState             = RK_AIQ_CORE_STATE_RUNNING;
    }

    aiq_VideoBuffer_t* buffer  = (aiq_VideoBuffer_t*)evt->vb;
    AiqHwStatsEvt_t* pStatsEvt = (AiqHwStatsEvt_t*)evt;
    bool is_broadcast          = false;
    if (buffer->_buf_type == ISP_POLL_3A_STATS) {
        LOGD_ANALYZER("new stats: camId:%d, sequence(%d)",
                      pAiqCore->mAlogsComSharedParams.mCamPhyId,
                      AiqVideoBuffer_getSequence(buffer));
        pAiqCore->mTranslator->mCamHw       = pStatsEvt->mCamHw;
        pAiqCore->mTranslator->mSensorHw    = pStatsEvt->mSensorHw;
        pAiqCore->mTranslator->mFocusLensHw = pStatsEvt->mFocusLensHw;
        pAiqCore->mTranslator->mIrishw      = pStatsEvt->mIrishw;
        ret = AiqStatsTranslator_getParams(pAiqCore->mTranslator, buffer);
        if (ret) {
            AiqStatsTranslator_releaseParams(pAiqCore->mTranslator);
            return ret;
        }
        if (_check_stats_valid(buffer, AIQ_STATS_TYPE_AEC)) {
            LOGD_ANALYZER("new aec stats: camId:%d, sequence(%d), ts: %lld, buf:%p",
                          pAiqCore->mAlogsComSharedParams.mCamPhyId,
                          AiqVideoBuffer_getSequence(buffer), AiqVideoBuffer_getTimestamp(buffer), buffer);
            handleAecStats(pAiqCore, buffer);
        }
        if (_check_stats_valid(buffer, AIQ_STATS_TYPE_AWB)) {
            LOGD_ANALYZER("new awb stats: camId:%d, sequence(%d), ts: %lld, buf:%p",
                          pAiqCore->mAlogsComSharedParams.mCamPhyId,
                          AiqVideoBuffer_getSequence(buffer), AiqVideoBuffer_getTimestamp(buffer), buffer);
            handleAwbStats(pAiqCore, buffer);
            is_broadcast = true;
        }
        if (_check_stats_valid(buffer, AIQ_STATS_TYPE_AF))
            handleAfStats(pAiqCore, buffer);
        if (_check_stats_valid(buffer, AIQ_STATS_TYPE_DHAZ))
            handleAdehazeStats(pAiqCore, buffer);
#if RK_GAIN_V2_ENABLE_GAIN2DDR && defined(ISP_HW_V32)
        handleAgainStats(pAiqCore, buffer);
#endif
#if defined(RKAIQ_HAVE_BAYERTNR_V30)
#if (USE_NEWSTRUCT == 0)
        if (_check_stats_valid(buffer, AIQ_STATS_TYPE_BAY3D))
            handleBay3dStats(pAiqCore, buffer);
#endif
#endif
        if (is_broadcast) {
            if (AiqVideoBuffer_getSequence(buffer) != pAiqCore->mCurAeStats->frame_id) {
                LOGW_ANALYZER("AE stats id %d, AWB stats id %d, Ae stats maybe delay or pre",
                              AiqVideoBuffer_getSequence(buffer), pAiqCore->mCurAeStats->frame_id);
            }
            aiqCond_broadcast(&pAiqCore->mIspStatsCond);
        }
        AiqStatsTranslator_releaseParams(pAiqCore->mTranslator);
    } else {
        int type = -1;
        switch (buffer->_buf_type) {
        case ISP_POLL_SP:
            type = XCAM_MESSAGE_ISP_POLL_SP_OK;
            break;
        case ISP_GAIN:
            type = XCAM_MESSAGE_ISP_GAIN_OK;
            break;
        case ISPP_GAIN_KG:
            type = XCAM_MESSAGE_ISPP_GAIN_KG_OK;
            break;
        case ISPP_GAIN_WR:
            type = XCAM_MESSAGE_ISPP_GAIN_WR_OK;
            break;
        case ISP_NR_IMG:
            type = XCAM_MESSAGE_NR_IMG_OK;
            break;
        case ISP_POLL_TX: {
            type = XCAM_MESSAGE_ISP_POLL_TX_OK;
            break;
        }
        case VICAP_POLL_SCL:
            handleVicapScaleBufs(pAiqCore, buffer);
            break;
#if RKAIQ_HAVE_PDAF
        case ISP_POLL_PDAF_STATS: {
            AiqHwPdafEvt_t* pPdafEvt = (AiqHwPdafEvt_t*)evt;
            handlePdafStats(pAiqCore, &pPdafEvt->pdaf_meas, buffer);
            break;
        }
#endif
        case ISPP_POLL_TNR_STATS:
        case ISPP_POLL_FEC_PARAMS:
        case ISPP_POLL_TNR_PARAMS:
        case ISPP_POLL_NR_PARAMS:
        case ISP_POLL_LUMA:
        case ISP_POLL_PARAMS:
        case ISP_POLL_SOF:
        case ISP_POLL_RX:
        case ISP_POLL_ISPSTREAMSYNC:
        case VICAP_STREAM_ON_EVT:
            LOGW_ANALYZER("buffer type: 0x%x is not used!", buffer->_buf_type);
            /* assert(false); */
            break;
        default:
            LOGW_ANALYZER("don't know buffer type: 0x%x!", buffer->_buf_type);
            assert(false);
            break;
        }
        if (type != -1) {
            AiqCoreMsg_t msg;
            msg.msg_id                      = (enum XCamMessageType)type;
            msg.frame_id                    = AiqVideoBuffer_getSequence(buffer);
            *(aiq_VideoBuffer_t**)(msg.buf) = buffer;
            ret                             = AiqCore_post_message(pAiqCore, &msg);
        }
    }

    return ret;
}

XCamReturn AiqCore_pushStats(AiqCore_t * pAiqCore, AiqHwEvt_t * evt) {

    if (pAiqCore->mState == RK_AIQ_CORE_STATE_STOPED) {
        LOGW("in stopped stat, ignore stats !");
        return XCAM_RETURN_NO_ERROR;
    }

    aiq_VideoBuffer_t* buffer = evt->vb;
    if (buffer && buffer->_buf_type == ISP_POLL_3A_STATS) {
        // just check awb stats now, may add aec stats to check later
        if (_check_stats_valid(buffer, AIQ_STATS_TYPE_AWB)) {
            uint32_t seq     = AiqVideoBuffer_getSequence(buffer);
            int32_t delta    = seq - pAiqCore->mLatestEvtsId;
            int32_t interval = seq - pAiqCore->mLatestStatsId;

            if (interval == 1) {
                // do nothing
            } else if ((interval == 0 && (pAiqCore->mLatestStatsId != 0)) || (interval < 0)) {
                LOGE_ANALYZER("stats disorder, latest:%u, new:%u", pAiqCore->mLatestStatsId, seq);
                return XCAM_RETURN_NO_ERROR;
            } else if (interval > 1) {
                LOGW_ANALYZER("stats not continuous, latest:%u, new:%u", pAiqCore->mLatestStatsId, seq);
            }

#if RKAIQ_HAVE_DUMPSYS
            if (seq > pAiqCore->mLatestStatsId)
                pAiqCore->mStatsLoss += seq > 1 ? seq - pAiqCore->mLatestStatsId - 1 : 0;
#endif
            pAiqCore->mLatestStatsId = seq;
            if (delta > 3) {
                LOGW_ANALYZER("stats delta: %d, skip stats %u", delta, seq);
                return XCAM_RETURN_NO_ERROR;
            }
        }
    }
    XCamReturn ret = RkAiqCore_analyze(pAiqCore, evt);

    return ret;
}

static XCamReturn events_analyze(AiqCore_t* pAiqCore, const AiqHwEvt_t* evts) {
#define CLEAN_EXPS(EXP)                        \
    if (EXP) {                                 \
        AIQ_REF_BASE_UNREF(&(EXP)->_base._ref_base); \
        (EXP) = NULL;                          \
    }

    XCamReturn ret                      = XCAM_RETURN_NO_ERROR;
    int hdr_iso[3]                      = {0};
    aiq_sensor_exp_info_t* preExpParams = NULL;
    aiq_sensor_exp_info_t* curExpParams = NULL;
    aiq_sensor_exp_info_t* nxtExpParams = NULL;

    Aiqisp20Evt_t* isp20Evts = (Aiqisp20Evt_t*)evts;
    uint32_t sequence        = evts->frame_id;
    if (sequence == (uint32_t)(-1)) return ret;

    AiqSensorHw_t* pSnsHw = isp20Evts->mCamHw->_mSensorDev;
    uint32_t id = 0, maxId = 0;
    if (sequence > 0)
        id = pAiqCore->mLastAnalyzedId + 1 > sequence ? pAiqCore->mLastAnalyzedId + 1 : sequence;
    maxId = sequence == 0 ? 0 : sequence + isp20Evts->_expDelay - 1;

    LOGD_ANALYZER("camId:%d, sequence(%d), expDelay(%d), id(%d), maxId(%d)",
                  pAiqCore->mAlogsComSharedParams.mCamPhyId, evts->frame_id, isp20Evts->_expDelay,
                  id, maxId);

    while (id <= maxId) {
        AlgoRstShared_t* pShared = NULL;

        if (!pAiqCore->mAiqSofInfoWrapperPool) {
            ret = XCAM_RETURN_BYPASS;
            goto out;
        }

        AiqPoolItem_t* pItem = aiqPool_getFree(pAiqCore->mAiqSofInfoWrapperPool);
        if (!pItem) {
            LOGW("cid:%d, fid:%d no free sof buffer!", pAiqCore->mAlogsComSharedParams.mCamPhyId,
                 id);
#if RKAIQ_HAVE_DUMPSYS
            pAiqCore->mNoFreeBufCnt.sofInfo++;
#endif
            ret = XCAM_RETURN_BYPASS;
            goto out;
        }
        pShared = (AlgoRstShared_t*)pItem->_pData;

        CLEAN_EXPS(preExpParams);
        preExpParams = pSnsHw->getEffectiveExpParams(pSnsHw, id > 0 ? id - 1 : 0);
        if (!preExpParams) {
            LOGE_ANALYZER("id(%d) get pre exp failed!", id);
            AIQ_REF_BASE_UNREF(&pShared->_ref_base);
            ret = XCAM_RETURN_BYPASS;
            goto out;
        }

        CLEAN_EXPS(curExpParams);
        curExpParams = pSnsHw->getEffectiveExpParams(pSnsHw, id);
        if (!curExpParams) {
            LOGE_ANALYZER("id(%d) get cur exp failed!", id);
            AIQ_REF_BASE_UNREF(&pShared->_ref_base);
            ret = XCAM_RETURN_BYPASS;
            goto out;
        }

        CLEAN_EXPS(nxtExpParams);
        nxtExpParams = pSnsHw->getEffectiveExpParams(pSnsHw, id + 1);
        if (!nxtExpParams) {
            LOGE_ANALYZER("id(%d) get next exp failed!", id + 1);
            AIQ_REF_BASE_UNREF(&pShared->_ref_base);
            ret = XCAM_RETURN_BYPASS;
            goto out;
        }

        if (pAiqCore->mAlogsComSharedParams.hdr_mode == 0) {
            hdr_iso[0] = 50 * curExpParams->aecExpInfo.LinearExp.exp_real_params.analog_gain *
                         curExpParams->aecExpInfo.LinearExp.exp_real_params.digital_gain *
                         curExpParams->aecExpInfo.LinearExp.exp_real_params.isp_dgain;
        } else {
            for (int i = 0; i < 3; i++) {
                hdr_iso[i] = 50 * curExpParams->aecExpInfo.HdrExp[i].exp_real_params.analog_gain *
                             curExpParams->aecExpInfo.HdrExp[i].exp_real_params.digital_gain *
                             curExpParams->aecExpInfo.HdrExp[i].exp_real_params.isp_dgain;
            }
        }

        AiqSofInfoWrapper_t* sofInfo = (AiqSofInfoWrapper_t*)pShared->_data;
        sofInfo->sequence            = id;
        sofInfo->preExp              = preExpParams;
        sofInfo->curExp              = curExpParams;
        sofInfo->nxtExp              = nxtExpParams;
        sofInfo->sof                 = evts->mTimestamp;
        sofInfo->iso                 = hdr_iso[pAiqCore->mAlogsComSharedParams.hdr_mode];

        int64_t sofTime = evts->mTimestamp / 1000LL;
        if ((pAiqCore->mSofTime != 0LL) && (sofTime != pAiqCore->mSofTime))
            pAiqCore->mFrmInterval = sofTime - pAiqCore->mSofTime;
        pAiqCore->mSofTime = sofTime;

        AiqCoreMsg_t msg;
        msg.msg_id                    = XCAM_MESSAGE_SOF_INFO_OK;
        msg.frame_id                  = id;
        *(AlgoRstShared_t**)(msg.buf) = pShared;
        ret                           = AiqCore_post_message(pAiqCore, &msg);
        AIQ_REF_BASE_UNREF(&pShared->_ref_base);
        if (ret) {
            ret = XCAM_RETURN_BYPASS;
            goto out;
        }

        pAiqCore->mLastAnalyzedId = id;
        id++;

        LOGV_ANALYZER(
            ">>> Framenum=%d, id=%d, Cur sgain=%f,stime=%f,mgain=%f,mtime=%f,lgain=%f,ltime=%f",
            sequence, id, curExpParams->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
            curExpParams->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
            curExpParams->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
            curExpParams->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
            curExpParams->aecExpInfo.HdrExp[2].exp_real_params.analog_gain,
            curExpParams->aecExpInfo.HdrExp[2].exp_real_params.integration_time);
        LOGV_ANALYZER(
            ">>> Framenum=%d, id=%d, nxt sgain=%f,stime=%f,mgain=%f,mtime=%f,lgain=%f,ltime=%f",
            sequence, id, nxtExpParams->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
            nxtExpParams->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
            nxtExpParams->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
            nxtExpParams->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
            nxtExpParams->aecExpInfo.HdrExp[2].exp_real_params.analog_gain,
            nxtExpParams->aecExpInfo.HdrExp[2].exp_real_params.integration_time);

        LOGV_ANALYZER("analyze the id(%d), sequence(%d), mLastAnalyzedId(%d)", id, sequence,
                      pAiqCore->mLastAnalyzedId);
    }

out:
    CLEAN_EXPS(preExpParams);
    CLEAN_EXPS(curExpParams);
    CLEAN_EXPS(nxtExpParams);

    return ret;
}

XCamReturn AiqCore_pushEvts(AiqCore_t* pAiqCore, AiqHwEvt_t* evts) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pAiqCore->mState == RK_AIQ_CORE_STATE_STOPED) {
        LOGW("in stopped stat, ignore stats !");
        return XCAM_RETURN_NO_ERROR;
    }

    if (evts->type == V4L2_EVENT_FRAME_SYNC) {
        uint32_t seq     = evts->frame_id;
        int32_t delta    = seq - pAiqCore->mLatestStatsId;
        int32_t interval = seq - pAiqCore->mLatestEvtsId;

#if RKAIQ_HAVE_DUMPSYS
        pAiqCore->mSofLoss += seq ? interval - 1 : 0;
#endif

        if (interval == 1) {
            // do nothing
        } else if ((interval == 0 && (pAiqCore->mLatestEvtsId != 0)) || (interval < 0)) {
            LOGE_ANALYZER("sof disorder, latest:%u, new:%u", pAiqCore->mLatestEvtsId, seq);
            return XCAM_RETURN_NO_ERROR;
        } else if (interval > 1) {
            LOGW_ANALYZER("sof not continuous, latest:%u, new:%u", pAiqCore->mLatestEvtsId, seq);
        }

        pAiqCore->mLatestEvtsId = evts->frame_id;

        if (delta > (pAiqCore->mDefaultDelayCnt + 1)) {
            LOGW_ANALYZER("sof delta: %d, skip sof %u", delta, seq);
            return XCAM_RETURN_NO_ERROR;
        }
        ret = events_analyze(pAiqCore, evts);
    }

    return ret;
}

static void RkAiqCore_copyIspStats(AiqCore_t* pAiqCore, rk_aiq_isp_statistics_t* to) {
    aiq_ae_stats_wrapper_t* pAeStats   = NULL;
    aiq_awb_stats_wrapper_t* pAwbStats = NULL;
    aiq_af_stats_wrapper_t* pAfStats   = NULL;

    to->bValid_aec_stats = false;
    to->bValid_awb_stats = false;
#if RKAIQ_HAVE_AF_V33 || RKAIQ_ONLY_AF_STATS_V33
    to->bValid_af_stats  = false;
#endif
    if (pAiqCore->mCurAeStats) {
        pAeStats             = (aiq_ae_stats_wrapper_t*)pAiqCore->mCurAeStats->_data;
        to->aec_stats = pAeStats->aec_stats_v25;
        to->frame_id         = pAiqCore->mCurAeStats->frame_id;
        to->bValid_aec_stats = true;
    }

    if (pAiqCore->mCurAwbStats) {
        pAwbStats            = (aiq_awb_stats_wrapper_t*)pAiqCore->mCurAwbStats->_data;
        to->awb_stats        = pAwbStats->awb_stats_v39;
        to->frame_id         = pAiqCore->mCurAwbStats->frame_id;
        to->bValid_awb_stats = true;
    }

#if RKAIQ_HAVE_AF_V33 || RKAIQ_ONLY_AF_STATS_V33
    if (pAiqCore->mCurAfStats) {
        pAfStats            = (aiq_af_stats_wrapper_t*)pAiqCore->mCurAfStats->_data;
        to->bValid_af_stats = true;
        to->afStats_stats   = pAfStats->afStats_stats;
    }
#endif
}

/*
 * timeout: -1 next, 0 current, > 0 wait next until timeout
 */
XCamReturn AiqCore_get3AStats(AiqCore_t* pAiqCore, rk_aiq_isp_statistics_t* stats, int timeout_ms) {
    //if (pAiqCore->mState != RK_AIQ_CORE_STATE_RUNNING) return XCAM_RETURN_ERROR_FAILED;

    aiqMutex_lock(&pAiqCore->mIspStatsMutex);

    if (timeout_ms < 0) {
        // equal to -1
        aiqCond_wait(&pAiqCore->mIspStatsCond, &pAiqCore->mIspStatsMutex);
    } else if (timeout_ms > 0) {
        int ret =
            aiqCond_timedWait(&pAiqCore->mIspStatsCond, &pAiqCore->mIspStatsMutex, timeout_ms * 1000);
        if (ret == ETIMEDOUT) {
            aiqMutex_unlock(&pAiqCore->mIspStatsMutex);
            return XCAM_RETURN_ERROR_TIMEOUT;
        }
    } else {
        // get current
    }

    RkAiqCore_copyIspStats(pAiqCore, stats);
    aiqMutex_unlock(&pAiqCore->mIspStatsMutex);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCore_setHwInfos(AiqCore_t* pAiqCore, AiqHwInfo_t* hw_info) {
    ENTER_ANALYZER_FUNCTION();
    pAiqCore->mHwInfo = *hw_info;
    if (pAiqCore->mTranslator)
        AiqStatsTranslator_setModuleRot(pAiqCore->mTranslator, hw_info->module_rotation);
    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCore_setGrayMode(AiqCore_t* pAiqCore, rk_aiq_gray_mode_t mode) {
    LOGD_ANALYZER("%s: gray mode %d", __FUNCTION__, mode);

    if (pAiqCore->mAlogsComSharedParams.is_bw_sensor) {
        LOGE_ANALYZER("%s: not support for black&white sensor", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CalibDbV2_ColorAsGrey_t* colorAsGrey = (CalibDbV2_ColorAsGrey_t*)CALIBDBV2_GET_MODULE_PTR(
            (void*)(pAiqCore->mAlogsComSharedParams.calibv2), colorAsGrey);

    pAiqCore->mGrayMode = mode;
    if (mode == RK_AIQ_GRAY_MODE_OFF)
        pAiqCore->mAlogsComSharedParams.gray_mode = false;
    else if (mode == RK_AIQ_GRAY_MODE_ON)
        pAiqCore->mAlogsComSharedParams.gray_mode = true;
    else if (mode == RK_AIQ_GRAY_MODE_CPSL)
        ;  // do nothing
    else
        LOGE_ANALYZER("%s: gray mode %d error", __FUNCTION__, mode);

    return XCAM_RETURN_NO_ERROR;
}

rk_aiq_gray_mode_t AiqCore_getGrayMode(AiqCore_t* pAiqCore) {
    LOGD_ANALYZER("%s: gray mode %d", __FUNCTION__, pAiqCore->mGrayMode);
    return pAiqCore->mGrayMode;
}

void AiqCore_setSensorFlip(AiqCore_t* pAiqCore, bool mirror, bool flip) {
    pAiqCore->mAlogsComSharedParams.sns_mirror = mirror;
    pAiqCore->mAlogsComSharedParams.sns_flip   = flip;
}

XCamReturn AiqCore_setCalib(AiqCore_t* pAiqCore, const CamCalibDbV2Context_t* aiqCalib) {
    ENTER_ANALYZER_FUNCTION();

    pAiqCore->mAlogsComSharedParams.calibv2   = aiqCalib;
    pAiqCore->mAlogsComSharedParams.conf_type = RK_AIQ_ALGO_CONFTYPE_UPDATECALIB;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

struct iqModStrToAlgoMap_s {
    const char* name;
    int type;
};

static struct iqModStrToAlgoMap_s iqModuleStrToAlgoEnumMap[] = {
    {"ae_", RK_AIQ_ALGO_TYPE_AE},
    {"wb", RK_AIQ_ALGO_TYPE_AWB},
    {"ablc", RK_AIQ_ALGO_TYPE_ABLC},
    {"lut3d", RK_AIQ_ALGO_TYPE_A3DLUT},
    {"adegamma", RK_AIQ_ALGO_TYPE_ADEGAMMA},
    {"agic_", RK_AIQ_ALGO_TYPE_AGIC},
    {"debayer", RK_AIQ_ALGO_TYPE_ADEBAYER},
    {"demosaic", RK_AIQ_ALGO_TYPE_ADEBAYER},
    {"amerge", RK_AIQ_ALGO_TYPE_AMERGE},
    {"adrc", RK_AIQ_ALGO_TYPE_ADRC},
    {"drc", RK_AIQ_ALGO_TYPE_ADRC},
    {"agamma", RK_AIQ_ALGO_TYPE_AGAMMA},
    {"gamma", RK_AIQ_ALGO_TYPE_AGAMMA},
    {"adehaze", RK_AIQ_ALGO_TYPE_ADHAZ},
    {"dehaze", RK_AIQ_ALGO_TYPE_ADHAZ},
    {"histeq", RK_AIQ_ALGO_TYPE_AHISTEQ},
    {"enh", RK_AIQ_ALGO_TYPE_AENH},
    {"hsv", RK_AIQ_ALGO_TYPE_AHSV},
    {"adpcc", RK_AIQ_ALGO_TYPE_ADPCC},
    {"dpc", RK_AIQ_ALGO_TYPE_ADPCC},
    {"cproc", RK_AIQ_ALGO_TYPE_ACP},
    {"cp", RK_AIQ_ALGO_TYPE_ACP},
    {"ie", RK_AIQ_ALGO_TYPE_AIE},
    {"lsc", RK_AIQ_ALGO_TYPE_ALSC},
    {"bayer2dnr", RK_AIQ_ALGO_TYPE_ARAWNR},
    {"bayertnr", RK_AIQ_ALGO_TYPE_AMFNR},
    {"ynr", RK_AIQ_ALGO_TYPE_AYNR},
    {"cnr", RK_AIQ_ALGO_TYPE_ACNR},
    {"sharp", RK_AIQ_ALGO_TYPE_ASHARP},
    {"cac", RK_AIQ_ALGO_TYPE_ACAC},
    {"af_", RK_AIQ_ALGO_TYPE_AF},
    {"gain_", RK_AIQ_ALGO_TYPE_AGAIN},
    {"csm", RK_AIQ_ALGO_TYPE_ACSM},
    {"cgc", RK_AIQ_ALGO_TYPE_ACGC},
    {"ccm_", RK_AIQ_ALGO_TYPE_ACCM},
    {"yuvme_", RK_AIQ_ALGO_TYPE_AMD},
    {"argbir", RK_AIQ_ALGO_TYPE_ARGBIR},
    {"rgbir", RK_AIQ_ALGO_TYPE_ARGBIR},
    {"ldc", RK_AIQ_ALGO_TYPE_ALDC},
    {"aibnr", RK_AIQ_ALGO_TYPE_AIBNR},
    {"airms", RK_AIQ_ALGO_TYPE_AIRMS},
    {"aiynr", RK_AIQ_ALGO_TYPE_AIYNR},
};

static void mapModStrListToEnum(AiqCore_t* pAiqCore, TuningCalib* change_name_list) {
    if (!pAiqCore->mUpdateCalibAlgosList) {
        AiqListConfig_t algosListCfg;
        algosListCfg._name              = "updateAlgos";
        algosListCfg._item_nums         = RK_AIQ_ALGO_TYPE_MAX;
        algosListCfg._item_size         = sizeof(RkAiqAlgoType_t);
        pAiqCore->mUpdateCalibAlgosList = aiqList_init(&algosListCfg);
        if (!pAiqCore->mUpdateCalibAlgosList) {
            LOGE_ANALYZER("init %s error", algosListCfg._name);
            return;
        }
    }

    aiqList_reset(pAiqCore->mUpdateCalibAlgosList);
    int type;
    int algoNums         = sizeof(iqModuleStrToAlgoEnumMap) / sizeof(struct iqModStrToAlgoMap_s);

    for (int i = 0; i < change_name_list->moduleNamesSize; i++) {
        char* name = change_name_list->moduleNames[i];
        if (strstr(name, "sensor_calib")) {
            type = RK_AIQ_ALGO_TYPE_AE;
            aiqList_push(pAiqCore->mUpdateCalibAlgosList, &type);
        } else if (strstr(name, "ALL")) {
            for (int i = 0; i < algoNums; i++) {
                type = iqModuleStrToAlgoEnumMap[i].type;
                aiqList_push(pAiqCore->mUpdateCalibAlgosList, &type);
            }
        }

        for (int i = 0; i < algoNums; i++) {
            if (strstr(name, iqModuleStrToAlgoEnumMap[i].name)) {
                type = iqModuleStrToAlgoEnumMap[i].type;
                aiqList_push(pAiqCore->mUpdateCalibAlgosList, &type);
                LOGK_ANALYZER("updateCalib: %s", name);
                break;
            }
        }
    }
}

static XCamReturn notifyUpdate(AiqCore_t* pAiqCore, uint64_t mask) {
    aiqMutex_lock(&pAiqCore->_update_mutex);
    pAiqCore->groupUpdateMask |= mask;
    aiqMutex_unlock(&pAiqCore->_update_mutex);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn waitUpdateDone(AiqCore_t* pAiqCore) {
    aiqMutex_lock(&pAiqCore->_update_mutex);

    int times = 12;
    while (times-- > 0 && pAiqCore->groupUpdateMask != 0) {
        aiqCond_timedWait(&pAiqCore->_update_done_cond, &pAiqCore->_update_mutex, 10000ULL);
    }

    if (pAiqCore->groupUpdateMask != 0) {
        LOGW_ANALYZER("calib not updated completely !");
    }

    aiqMutex_unlock(&pAiqCore->_update_mutex);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _prepare(AiqCore_t* pAiqCore, enum rk_aiq_core_analyze_type_e type) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret2 = XCAM_RETURN_BYPASS;

    int cnts = 0;
    AiqAlgoHandler_t** pHdlList = NULL;
    pHdlList = AiqAnalyzeGroupManager_getGroupAlgoList(&pAiqCore->mRkAiqCoreGroupManager, type, &cnts);
    AiqAlgoHandler_t* pHdl = NULL;
    for (int i = 0; i < cnts; i++) {
        pHdl = pHdlList[i];
        if (pHdl) {
            int old           = aiqList_size(pAiqCore->mUpdateCalibAlgosList);
            AiqListItem_t* pItem = NULL;
            bool rm              = false;
            AIQ_LIST_FOREACH(pAiqCore->mUpdateCalibAlgosList, pItem, rm) {
                if (AiqAlgoHandler_getAlgoType(pHdl) == *(int*)(pItem->_pData)) {
                    pItem = aiqList_erase_item_locked(pAiqCore->mUpdateCalibAlgosList, pItem);
                    rm    = true;
                } else {
                    rm = false;
                }
            }

            if (aiqList_size(pAiqCore->mUpdateCalibAlgosList) == old)
                pAiqCore->mAlogsComSharedParams.conf_type |= RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR;

            while (pHdl) {
                if (AiqAlgoHandler_getEnable(pHdl)) {
                    ret2 = pHdl->prepare(pHdl);
                    RKAIQCORE_CHECK_BYPASS(ret2, "algoHdl %d processing failed",
                                           AiqAlgoHandler_getAlgoType(pHdl));
                }
                pHdl = AiqAlgoHandler_getNextHdl(pHdl);
            }

            pAiqCore->mAlogsComSharedParams.conf_type &= ~RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR;
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCore_updateCalib(AiqCore_t * pAiqCore, enum rk_aiq_core_analyze_type_e type) {
    aiqMutex_lock(&pAiqCore->_update_mutex);
    // check if group bit still set
    uint64_t need_update = pAiqCore->groupUpdateMask & grpId2GrpMask(type);
    if (!need_update) {
        aiqMutex_unlock(&pAiqCore->_update_mutex);
        return XCAM_RETURN_NO_ERROR;
    }

    _prepare(pAiqCore, type);
    // clear group bit after update
    pAiqCore->groupUpdateMask &= (~need_update);
    // notify update done
    aiqCond_broadcast(&pAiqCore->_update_done_cond);
    aiqMutex_unlock(&pAiqCore->_update_mutex);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCore_calibTuning(AiqCore_t* pAiqCore, const CamCalibDbV2Context_t* aiqCalib,
                               TuningCalib* change_name_list) {
    ENTER_ANALYZER_FUNCTION();

    if (!aiqCalib || !change_name_list) {
        LOGE_ANALYZER("invalied tuning param\n");
        return XCAM_RETURN_ERROR_PARAM;
    }

    // Fill new calib to the AlogsSharedParams
    pAiqCore->mAlogsComSharedParams.calibv2 = aiqCalib;
    LOGK_ANALYZER("new calib %p", aiqCalib);
    pAiqCore->mAlogsComSharedParams.conf_type = RK_AIQ_ALGO_CONFTYPE_UPDATECALIB;

    for (int i = 0; i < change_name_list->moduleNamesSize; i++) {
        char* name = change_name_list->moduleNames[i];
        if (strstr(name, "colorAsGrey")) {
            CalibDbV2_ColorAsGrey_t* colorAsGrey =
                (CalibDbV2_ColorAsGrey_t*)CALIBDBV2_GET_MODULE_PTR(
                    (void*)(pAiqCore->mAlogsComSharedParams.calibv2), colorAsGrey);
            if (colorAsGrey->param.enable) {
                pAiqCore->mGrayMode                       = RK_AIQ_GRAY_MODE_ON;
                pAiqCore->mAlogsComSharedParams.gray_mode = true;
            } else {
                pAiqCore->mGrayMode                       = RK_AIQ_GRAY_MODE_OFF;
                pAiqCore->mAlogsComSharedParams.gray_mode = false;
            }
        }
    }
    mapModStrListToEnum(pAiqCore, change_name_list);

    uint64_t grpMask = 0;
    AiqAnalyzerGroup_t** groups =
        AiqAnalyzeGroupManager_getGroups(&pAiqCore->mRkAiqCoreGroupManager);
    for (int i = 0; i < RK_AIQ_CORE_ANALYZE_MAX; i++) {
        if (groups[i]) {
            grpMask |= 1ULL << i;
        }
    }

    notifyUpdate(pAiqCore, grpMask);
    if (pAiqCore->mState != RK_AIQ_CORE_STATE_RUNNING || pAiqCore->mIsAovMode)
        AiqCore_updateCalib(pAiqCore, RK_AIQ_CORE_ANALYZE_ALL);
    else {
        waitUpdateDone(pAiqCore);
    }
    pAiqCore->mAlogsComSharedParams.conf_type &= ~RK_AIQ_ALGO_CONFTYPE_UPDATECALIB;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCore_setMemsSensorIntf(AiqCore_t* pAiqCore, const rk_aiq_mems_sensor_intf_t* intf) {
    if (pAiqCore->mState != RK_AIQ_CORE_STATE_INITED) {
        LOGE_ANALYZER("set MEMS sensor intf in wrong aiq state %d !", pAiqCore->mState);
        return XCAM_RETURN_ERROR_FAILED;
    }

    pAiqCore->mMemsSensorIntf = intf;

    return XCAM_RETURN_NO_ERROR;
}

const rk_aiq_mems_sensor_intf_t* AiqCore_getMemsSensorIntf(AiqCore_t* pAiqCore) {
    return pAiqCore->mMemsSensorIntf;
}

XCamReturn AiqCore_groupAnalyze(AiqCore_t * pAiqCore, uint64_t grpId,
                                const RkAiqAlgosGroupShared_t* shared) {
    AiqFullParams_t* fullParam = NULL;
    AiqFullParams_t* pAiqFullParams = NULL;

    pAiqFullParams = analyzeInternal(pAiqCore, (enum rk_aiq_core_analyze_type_e)grpId);
    if (grpId == RK_AIQ_CORE_ANALYZE_AF && pAiqFullParams &&
        pAiqFullParams->pParamsArray[RESULT_TYPE_FOCUS_PARAM] &&
        pAiqFullParams->pParamsArray[RESULT_TYPE_FOCUS_PARAM]->is_update) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        if (!pAiqCore->mCamGroupCoreManager && pAiqCore->mCb)
#else
        if (pAiqCore->mCb)
#endif
            pAiqCore->mCb->rkAiqCalcExpDone(pAiqCore->mCb->pCtx, pAiqFullParams->pParamsArray[RESULT_TYPE_FOCUS_PARAM]);
        pAiqFullParams->pParamsArray[RESULT_TYPE_FOCUS_PARAM]->is_update = false;
    }

    {
        aiqMutex_lock(&pAiqCore->_mFullParam_mutex);
        AiqMapItem_t* pItem = aiqMap_get(pAiqCore->mFullParamsPendingMap, (void*)(intptr_t)shared->frameId);
        pending_params_t* pParams = NULL;
        if (pItem) pParams = (pending_params_t*)(pItem->_pData);
        if (pParams && pParams->ready) {
            bool rm = false;
            AIQ_MAP_FOREACH(pAiqCore->mFullParamsPendingMap, pItem, rm) {
                if ((uint32_t)(long)pItem->_key <= shared->frameId) {
                    pParams = (pending_params_t*)(pItem->_pData);
                    if (fullParam) AIQ_REF_BASE_UNREF(&fullParam->_base._ref_base);
                    fullParam                 = pParams->fullParams;
                    fullParam->_base.frame_id = (uint32_t)(long)pItem->_key;
                    pItem = aiqMap_erase_locked(pAiqCore->mFullParamsPendingMap,
                                                (void*)(intptr_t)fullParam->_base.frame_id);
                    rm    = true;
                    pAiqCore->mLatestParamsDoneId = fullParam->_base.frame_id;
                    if (fullParam->_base.frame_id < shared->frameId) {
#ifdef RKAIQ_ENABLE_CAMGROUP
                        if (!pAiqCore->mCamGroupCoreManager && pAiqCore->mCb)
#else
                        if (pAiqCore->mCb)
#endif
                            pAiqCore->mCb->rkAiqCalcExpDone(pAiqCore->mCb->pCtx, fullParam->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM]);

                        LOGD_ANALYZER("[%d]:%p fullParam discard, shared->frame %d, just set exp param",
                                      fullParam->_base.frame_id, fullParam, shared->frameId);
                    }
                    LOGD_ANALYZER("[%d]:%p fullParams done !", fullParam->_base.frame_id,
                                  fullParam);
                } else
                    break;
            }
        } else {
            uint16_t FULLPARMAS_MAX_PENDING_SIZE = pAiqCore->mDefaultDelayCnt;
            uint16_t counts = aiqMap_size(pAiqCore->mFullParamsPendingMap);
            if (counts > FULLPARMAS_MAX_PENDING_SIZE) {
                pItem                     = aiqMap_begin(pAiqCore->mFullParamsPendingMap);
                pending_params_t* pParams = (pending_params_t*)(pItem->_pData);
                fullParam                 = pParams->fullParams;
                LOGW_ANALYZER("force [%d]:%p fullParams done ! new param id %d",
                              (uint32_t)(long)pItem->_key, fullParam, shared->frameId);
                fullParam->_base.frame_id = (uint32_t)(long)pItem->_key;
                for (int i = 0; i < RESULT_TYPE_MAX_PARAM; i++) {
                    if (fullParam->pParamsArray[i] &&
                            fullParam->pParamsArray[i]->frame_id != fullParam->_base.frame_id) {
                        fullParam->pParamsArray[i]->frame_id = fullParam->_base.frame_id;
                    }
                }
                aiqMap_erase_locked(pAiqCore->mFullParamsPendingMap, (void*)(intptr_t)fullParam->_base.frame_id);
                pAiqCore->mLatestParamsDoneId = fullParam->_base.frame_id;
#if RKAIQ_HAVE_DUMPSYS
                pAiqCore->mForceDoneCnt++;
                pAiqCore->mForceDoneId = pAiqCore->mLatestParamsDoneId;
#endif
            }
        }
        aiqMutex_unlock(&pAiqCore->_mFullParam_mutex);
    }

    if (fullParam) {
        LOG1_ANALYZER("cb [%d] fullParams done !", pAiqCore->mLatestParamsDoneId);
#ifdef RKAIQ_ENABLE_CAMGROUP
        if (!pAiqCore->mCamGroupCoreManager)
#endif
            fixAiqParamsIsp(pAiqCore, fullParam);
#ifdef RKAIQ_ENABLE_CAMGROUP
        if (pAiqCore->mCamGroupCoreManager) {
            if (!pAiqCore->mTbInfo.is_fastboot || shared->frameId != 0) {
                AiqCamGroupManager_RelayAiqCoreResults(pAiqCore->mCamGroupCoreManager, pAiqCore, fullParam);
            } else {
                if (pAiqCore->mCb) pAiqCore->mCb->rkAiqCalcDone(pAiqCore->mCb->pCtx, fullParam);
            }
        } else if (pAiqCore->mCb)
#else
        if (pAiqCore->mCb)
#endif
            pAiqCore->mCb->rkAiqCalcDone(pAiqCore->mCb->pCtx, fullParam);
        AIQ_REF_BASE_UNREF(&fullParam->_base._ref_base);
    }
exit:
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCore_set_sp_resolution(AiqCore_t* pAiqCore, int* width, int* height, int* aligned_w,
                                     int* aligned_h) {
    pAiqCore->mSpWidth         = *width;
    pAiqCore->mSpHeight        = *height;
    pAiqCore->mSpAlignedWidth  = *aligned_w;
    pAiqCore->mSpAlignedHeight = *aligned_h;
    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_PDAF
XCamReturn AiqCore_set_pdaf_support(AiqCore_t* pAiqCore, bool support) {
    pAiqCore->mPdafSupport = support;
    return XCAM_RETURN_NO_ERROR;
}

bool AiqCore_get_pdaf_support(AiqCore_t* pAiqCore) {
    return pAiqCore->mPdafSupport;
}

XCamReturn AiqCore_set_pdaf_type(AiqCore_t* pAiqCore, PdafSensorType_t type) {
    pAiqCore->mPdafType = type;
    return XCAM_RETURN_NO_ERROR;
}

PdafSensorType_t AiqCore_get_pdaf_type(AiqCore_t* pAiqCore) {
    return pAiqCore->mPdafType;
}
#endif

XCamReturn AiqCore_updateCalibDbBrutal(AiqCore_t* pAiqCore, CamCalibDbV2Context_t* aiqCalib) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // Fill new calib to the AlogsSharedParams
    pAiqCore->mAlogsComSharedParams.calibv2   = aiqCalib;
    pAiqCore->mAlogsComSharedParams.conf_type = RK_AIQ_ALGO_CONFTYPE_UPDATECALIB;

    CalibDbV2_ColorAsGrey_t* colorAsGrey = (CalibDbV2_ColorAsGrey_t*)CALIBDBV2_GET_MODULE_PTR(
            (void*)(pAiqCore->mAlogsComSharedParams.calibv2), colorAsGrey);
    if (colorAsGrey->param.enable) {
        pAiqCore->mGrayMode                       = RK_AIQ_GRAY_MODE_ON;
        pAiqCore->mAlogsComSharedParams.gray_mode = true;
    } else {
        pAiqCore->mGrayMode                       = RK_AIQ_GRAY_MODE_OFF;
        pAiqCore->mAlogsComSharedParams.gray_mode = false;
    }
    AiqCore_setGrayMode(pAiqCore, pAiqCore->mGrayMode);

    int cnts = 0;
    AiqAlgoHandler_t** pHdlList = NULL;
    pHdlList = AiqAnalyzeGroupManager_getGroupAlgoList(&pAiqCore->mRkAiqCoreGroupManager, RK_AIQ_CORE_ANALYZE_ALL, &cnts);
    for (int i = 0; i < cnts; i++) {
        AiqAlgoHandler_t* pHandle = pHdlList[i];
        while (pHandle) {
            if (AiqAlgoHandler_getEnable(pHandle)) {
                AiqAlgoHandler_setReConfig(pHandle, true);
                pHandle->prepare(pHandle);
                RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d prepare failed",
                                       AiqAlgoHandler_getAlgoType(pHandle));
            }
            pHandle = AiqAlgoHandler_getNextHdl(pHandle);
        }
    }

    pAiqCore->mAlogsComSharedParams.conf_type &= ~RK_AIQ_ALGO_CONFTYPE_UPDATECALIB;

    return XCAM_RETURN_NO_ERROR;
}

void AiqCore_setDelayCnts(AiqCore_t* pAiqCore, int8_t delayCnts) {
    AiqAnalyzeGroupManager_setDelayCnts(&pAiqCore->mRkAiqCoreGroupManager, delayCnts);
}

void AiqCore_setVicapScaleFlag(AiqCore_t* pAiqCore, bool mode) {
    AiqAnalyzerGroup_t** groups =
        AiqAnalyzeGroupManager_getGroups(&pAiqCore->mRkAiqCoreGroupManager);

    if (groups && groups[RK_AIQ_CORE_ANALYZE_AFD]) {
        uint64_t flag = groups[RK_AIQ_CORE_ANALYZE_AFD]->mDepsFlag;
        if (mode)
            flag |= (1ULL << XCAM_MESSAGE_VICAP_POLL_SCL_OK);
        else
            flag &= ~(1ULL << XCAM_MESSAGE_VICAP_POLL_SCL_OK);
        AiqAnalyzerGroup_setDepsFlagAndClearMap(groups[RK_AIQ_CORE_ANALYZE_AFD], flag);
        LOGD_ANALYZER("afd algo dep flag %llx\n", flag);
    }
}

void AiqCore_awakenClean(AiqCore_t* pAiqCore, uint32_t sequence) {
    AiqAnalyzeGroupManager_awakenClean(&pAiqCore->mRkAiqCoreGroupManager, sequence);
    aiqMutex_lock(&pAiqCore->_mFullParam_mutex);
    clearFullParamsPendingMap(pAiqCore);
    if (pAiqCore->mLastAnalyzedId) pAiqCore->mLastAnalyzedId = 0;
    if (pAiqCore->mLatestEvtsId) {
        pAiqCore->mLatestEvtsId = sequence - 1;
    }
    if (pAiqCore->mLatestStatsId) {
        pAiqCore->mLatestStatsId = sequence - 1;
    }
    if (pAiqCore->mLatestParamsDoneId) pAiqCore->mLatestParamsDoneId = 0;
    aiqMutex_unlock(&pAiqCore->_mFullParam_mutex);
}

XCamReturn AiqCore_setUserOtpInfo(AiqCore_t* pAiqCore, rk_aiq_user_otp_info_t otp_info) {
    if (pAiqCore->mState == RK_AIQ_CORE_STATE_PREPARED ||
            pAiqCore->mState == RK_AIQ_CORE_STATE_STARTED ||
            pAiqCore->mState == RK_AIQ_CORE_STATE_RUNNING) {
        LOGE_ANALYZER("wrong state %d, set otp before prepare\n", pAiqCore->mState);
        return XCAM_RETURN_ERROR_ANALYZER;
    }

    LOGD_ANALYZER("user awb otp: flag: %d, r:%d,b:%d,gr:%d,gb:%d, golden r:%d,b:%d,gr:%d,gb:%d\n",
                  otp_info.otp_awb.flag, otp_info.otp_awb.r_value, otp_info.otp_awb.b_value,
                  otp_info.otp_awb.gr_value, otp_info.otp_awb.gb_value,
                  otp_info.otp_awb.golden_r_value, otp_info.otp_awb.golden_b_value,
                  otp_info.otp_awb.golden_gr_value, otp_info.otp_awb.golden_gb_value);

    pAiqCore->mUserOtpInfo = otp_info;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCore_setAOVForAE(AiqCore_t* pAiqCore, bool en) {
    AiqAlgoHandler_t* pHdl = pAiqCore->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];
    if (pHdl) {
        //TODO: zyc
        //pHdl->setAOVForAE(pHdl, en);
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn fixAiqParamsIsp(AiqCore_t* pAiqCore, AiqFullParams_t* aiqParams) {
#if ISP_HW_V39
    aiq_params_base_t* pYnrBase    = aiqParams->pParamsArray[RESULT_TYPE_YNR_PARAM];
    aiq_params_base_t* pHisteqBase = aiqParams->pParamsArray[RESULT_TYPE_HISTEQ_PARAM];
    aiq_params_base_t* pDhzeBase   = aiqParams->pParamsArray[RESULT_TYPE_DEHAZE_PARAM];

    // update ynr cure
    if (pYnrBase && pDhzeBase && pYnrBase->is_update) {
        pDhzeBase->is_update = true;
    }

    // histeq and dehaze need update at the same time
    if (pDhzeBase && pHisteqBase && pDhzeBase->is_update ^ pHisteqBase->is_update) {
        pDhzeBase->is_update   = true;
        pHisteqBase->is_update = true;
    }
#endif

#if defined(ISP_HW_V35)
    aiq_params_base_t* pIeBase    = aiqParams->pParamsArray[RESULT_TYPE_IE_PARAM];
    aiq_params_base_t* pCnrBase   = aiqParams->pParamsArray[RESULT_TYPE_UVNR_PARAM];
    aiq_params_base_t* pEnhBase   = aiqParams->pParamsArray[RESULT_TYPE_ENH_PARAM];
    aiq_params_base_t* pYnrBase   = aiqParams->pParamsArray[RESULT_TYPE_YNR_PARAM];
    if (pIeBase && pIeBase->is_update) {
        if (pCnrBase && !pCnrBase->is_update) {
            pCnrBase->is_update = true;
        }
        if (pEnhBase && !pEnhBase->is_update) {
            pEnhBase->is_update = true;
        }
        if (pYnrBase && !pYnrBase->is_update) {
            pYnrBase->is_update = true;
        }
    }

#if 0
    aiq_params_base_t* pBtnr2Base = aiqParams->pParamsArray[RESULT_TYPE_TNR2_PARAM];
    if (pBtnr2Base && !pBtnr2Base->is_update) {
        pBtnr2Base->is_update = true;
    }
#endif
#endif

    aiq_params_base_t* pDrcBase  = aiqParams->pParamsArray[RESULT_TYPE_DRC_PARAM];
    aiq_params_base_t* pAwbBase  = aiqParams->pParamsArray[RESULT_TYPE_AWB_PARAM];
    aiq_params_base_t* pBlcBase  = aiqParams->pParamsArray[RESULT_TYPE_BLC_PARAM];
    aiq_params_base_t* pBtnrBase = aiqParams->pParamsArray[RESULT_TYPE_TNR_PARAM];
    aiq_params_base_t* pGainBase = aiqParams->pParamsArray[RESULT_TYPE_GAIN_PARAM];
    aiq_params_base_t* pCacBase = aiqParams->pParamsArray[RESULT_TYPE_CAC_PARAM];
    aiq_params_base_t* pwbgainBase = aiqParams->pParamsArray[RESULT_TYPE_AWBGAIN_PARAM];
    // drc update, maybe predgain update
    if (pDrcBase && pDrcBase->is_update ) {
        if (pAwbBase && !pAwbBase->is_update) {
            pAwbBase->is_update = true;
        }

        if (pBlcBase && !pBlcBase->is_update) {
            pBlcBase->is_update = true;
        }

        if (pBtnrBase && !pBtnrBase->is_update) {
            pBtnrBase->is_update = true;
        }

        if (pCacBase && !pCacBase->is_update) {
            pCacBase->is_update = true;
        }
    }
    if (pBlcBase && pBlcBase->is_update) {
        if (pAwbBase && !pAwbBase->is_update) {
            pAwbBase->is_update = true;
        }
        if (pwbgainBase && !pwbgainBase->is_update) {
            pwbgainBase->is_update = true;
        }
    }

    // TODO: TNR/SHARP need update HWI params for each frame now
    aiq_params_base_t* psharpBase = aiqParams->pParamsArray[RESULT_TYPE_SHARPEN_PARAM];
    aiq_params_base_t* pGicBase   = aiqParams->pParamsArray[RESULT_TYPE_GIC_PARAM];
    aiq_params_base_t* pCcmBase   = aiqParams->pParamsArray[RESULT_TYPE_CCM_PARAM];
    if (psharpBase && !psharpBase->is_update) {
        psharpBase->is_update = true;
    }

    if (pGicBase && !pGicBase->is_update) {
        pGicBase->is_update = true;
    }

    if (pBtnrBase && !pBtnrBase->is_update) {
        pBtnrBase->is_update = true;
    }

    if (pGainBase && !pGainBase->is_update) {
        pGainBase->is_update = true;
    }

    if (pBlcBase && pBlcBase->is_update) {
#if RKAIQ_HAVE_AIBNR
        aiq_params_base_t* pAibnrBase = aiqParams->pParamsArray[RESULT_TYPE_AIBNR_PARAM];
        if (pAibnrBase && !pAibnrBase->is_update) {
            pAibnrBase->is_update = true;
        }
#endif
    }

    if (pCcmBase && !pCcmBase->is_update) {
        pCcmBase->is_update = true;
    }

    return XCAM_RETURN_NO_ERROR;
}

void AiqCore_notifyIspStreamMode(AiqCore_t* pAiqCore, rk_isp_stream_mode_t mode)
{
    if (mode == RK_ISP_STREAM_MODE_ONLNIE) {
        pAiqCore->mIspOnline = true;
    } else if (mode == RK_ISP_STREAM_MODE_OFFLNIE) {
        pAiqCore->mIspOnline = false;
    } else {
        pAiqCore->mIspOnline = false;
    }
}

void AiqCore_setTbInfo(AiqCore_t* pAiqCore, rk_aiq_tb_info_t* info) {
    if (pAiqCore && info) memcpy(&pAiqCore->mTbInfo, info, sizeof(*info));
}

void AiqCore_setShareMemOps(AiqCore_t* pAiqCore, isp_drv_share_mem_ops_t* mem_ops) {
    if (pAiqCore) pAiqCore->mShareMemOps = mem_ops;
}

XCamReturn
AiqCore_register3Aalgo(AiqCore_t* pAiqCore, void* algoDes, void *cbs)
{
    if (pAiqCore->mState != RK_AIQ_CORE_STATE_INITED &&
            pAiqCore->mState != RK_AIQ_CORE_STATE_STOPED)
        return XCAM_RETURN_ERROR_ANALYZER;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct RkAiqAlgoDesCommExt *algoDesArray = (struct RkAiqAlgoDesCommExt*)algoDes;

    for (size_t i = 0; algoDesArray[i].des != NULL; i++) {
        int algo_type = algoDesArray[i].des->type;
        if (algo_type == algoDesArray->des->type)
            pAiqCore->mAlogsComSharedParams.ctxCfigs[algo_type].cbs = cbs;
    }

    addDefaultAlgos(pAiqCore, algoDesArray);
    ret = newAllAiqParamPool(pAiqCore);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ANALYZER("init params pool failed");
        return XCAM_RETURN_ERROR_ANALYZER;
    }
    newAiqGroupAnayzer(pAiqCore, algoDesArray);

    return ret;
}

XCamReturn
AiqCore_unregister3Aalgo(AiqCore_t* pAiqCore, int algoType)
{
    ENTER_ANALYZER_FUNCTION();

    if (pAiqCore->mState != RK_AIQ_CORE_STATE_INITED && pAiqCore->mState != RK_AIQ_CORE_STATE_STOPED) {
        return XCAM_RETURN_ERROR_ANALYZER;
    }

    if (algoType != RK_AIQ_ALGO_TYPE_AE && algoType != RK_AIQ_ALGO_TYPE_AWB) return XCAM_RETURN_ERROR_ANALYZER;

    AiqAnalyzeGroupManager_rmAlgoHandle(&pAiqCore->mRkAiqCoreGroupManager, algoType);

    if (pAiqCore->mAlgoHandleMaps[algoType]) destroyAlgoHandler(pAiqCore->mAlgoHandleMaps[algoType]);
    pAiqCore->mAlgoHandleMaps[algoType] = NULL;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCore_setTranslaterIspUniteMode(AiqCore_t* pAiqCore, RkAiqIspUniteMode mode) {

    if (pAiqCore->mTranslator)
        pAiqCore->mTranslator->mIspUniteMode = mode;

    return XCAM_RETURN_NO_ERROR;
}


#if RKAIQ_HAVE_DUMPSYS
static int __dump(void* self, st_string* result, int argc, void* argv[]) {
    core_dump_mod_param((AiqCore_t*)self, result);
    core_dump_mod_attr((AiqCore_t*)self, result);
    core_dump_mod_status((AiqCore_t*)self, result);
    core_dump_mod_3a_stats((AiqCore_t*)self, result);

    return 0;
}

static void AiqCore_initNotifier(AiqCore_t* pAiqCore) {
    aiq_notifier_init(&pAiqCore->notifier);

    {
        pAiqCore->sub_core.match_type     = AIQ_NOTIFIER_MATCH_CORE;
        pAiqCore->sub_core.name           = "CORE -> core";
        pAiqCore->sub_core.dump.dump_fn_t = __dump;
        pAiqCore->sub_core.dump.dumper    = pAiqCore;

        aiq_notifier_add_subscriber(&pAiqCore->notifier, &pAiqCore->sub_core);
    }

    {
        pAiqCore->sub_buf_mgr.match_type     = AIQ_NOTIFIER_MATCH_CORE_BUF_MGR;
        pAiqCore->sub_buf_mgr.name           = "CORE -> buf_manager";
        pAiqCore->sub_buf_mgr.dump.dump_fn_t = core_buf_mgr_dump;
        pAiqCore->sub_buf_mgr.dump.dumper    = pAiqCore;

        aiq_notifier_add_subscriber(&pAiqCore->notifier, &pAiqCore->sub_buf_mgr);
    }

    {
        pAiqCore->sub_grp_analyzer.match_type     = AIQ_NOTIFIER_MATCH_CORE_GRP_ANALYZER;
        pAiqCore->sub_grp_analyzer.name           = "CORE -> group_analyzer";
        pAiqCore->sub_grp_analyzer.dump.dump_fn_t = AiqAnalyzerGroup_dump;
        pAiqCore->sub_grp_analyzer.dump.dumper    = (void*)&pAiqCore->mRkAiqCoreGroupManager;

        aiq_notifier_add_subscriber(&pAiqCore->notifier, &pAiqCore->sub_grp_analyzer);
    }

    core_isp_mods_add_subscriber(pAiqCore);
}

static const char* const usages[] = {
    "./dumpcam core cmd [args]",
    NULL,
};

static int _gHelp = 0;
static int dbg_help_cb(struct argparse* self, const struct argparse_option* option) {
    _gHelp = 1;

    return 0;
}

static int AiqCore_dump(void* self, st_string* result, int argc, void* argv[]) {
    char buffer[MAX_LINE_LENGTH]                   = {0};
    int dump_args[AIQ_NOTIFIER_MATCH_CORE_ALL + 1] = {0};

    if (argc < 2) {
        dump_args[AIQ_NOTIFIER_MATCH_CORE_ALL] = 1;
    } else {
        struct argparse_option options[] = {
            OPT_GROUP("basic options:"),
            OPT_BOOLEAN('A', "all", &dump_args[AIQ_NOTIFIER_MATCH_CORE_ALL], "dump core all", NULL,
                        0, 0),
            OPT_BOOLEAN('b', "buf", &dump_args[AIQ_NOTIFIER_MATCH_CORE_BUF_MGR],
                        "dump core buffer manager info", NULL, 0, 0),
            OPT_BOOLEAN('c', "core", &dump_args[AIQ_NOTIFIER_MATCH_CORE], "dump core info", NULL, 0,
                        0),
            OPT_BOOLEAN('n', "analyzer", &dump_args[AIQ_NOTIFIER_MATCH_CORE_GRP_ANALYZER],
                        "dump group analyzer info", NULL, 0, 0),
            OPT_BOOLEAN('\0', "help", NULL, "show this help message and exit", dbg_help_cb, 0,
                        OPT_NONEG),
            OPT_END(),
        };

        struct argparse argparse;
        argparse_init(&argparse, options, usages, 0);
        argparse_describe(&argparse, "\nselect a test case to run.", "\nuse --help for details.");

        argc = argparse_parse(&argparse, argc, (const char**)argv);
        if (_gHelp || argc < 0) {
            _gHelp = 0;
            argparse_usage_string(&argparse, buffer);
            aiq_string_printf(result, buffer);
        }
    }

    if (dump_args[AIQ_NOTIFIER_MATCH_CORE_ALL]) {
        for (int32_t i = AIQ_NOTIFIER_MATCH_CORE; i <= AIQ_NOTIFIER_MATCH_CORE_ALL; i++)
            aiq_notifier_notify_dumpinfo(&((AiqCore_t*)self)->notifier, i, result, argc, argv);

        return true;
    }

    for (int32_t i = AIQ_NOTIFIER_MATCH_CORE; i <= AIQ_NOTIFIER_MATCH_CORE_ALL; i++) {
        if (dump_args[i])
            aiq_notifier_notify_dumpinfo(&((AiqCore_t*)self)->notifier, i, result, argc, argv);
    }

    return true;
}

static const char* const algos_usages[] = {
    "./dumpcam algo cmd [args]",
    NULL,
};

static int _gAlgosHelp = 0;
static int algos_dbg_help_cb(struct argparse* self, const struct argparse_option* option) {
    _gAlgosHelp = 1;

    return 0;
}
static int AiqCore_algosDump(void* self, st_string* result, int argc, void* argv[]) {
    char buffer[MAX_LINE_LENGTH * 4]               = {0};
    int dump_args[AIQ_NOTIFIER_MATCH_MODS_ALL + 1] = {0};

    if (argc < 2) {
        dump_args[AIQ_NOTIFIER_MATCH_MODS_ALL] = 1;
    } else {
        struct argparse_option options[] = {
            OPT_GROUP("isp modules options:"),
            OPT_BOOLEAN('A', "all", &dump_args[AIQ_NOTIFIER_MATCH_MODS_ALL], "dump all info", NULL,
                        0, 0),
            OPT_BOOLEAN('e', "ae", &dump_args[AIQ_NOTIFIER_MATCH_AEC], "dump ae module info", NULL,
                        0, 0),
            OPT_BOOLEAN('f', "hist", &dump_args[AIQ_NOTIFIER_MATCH_HIST], "dump hist module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('i', "awb", &dump_args[AIQ_NOTIFIER_MATCH_AWB], "dump awb module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('j', "wbgain", &dump_args[AIQ_NOTIFIER_MATCH_AWB],
                        "dump awb gain module info ", NULL, 0, 0),
            OPT_BOOLEAN('k', "af", &dump_args[AIQ_NOTIFIER_MATCH_AF], "dump af module info", NULL,
                        0, 0),
            OPT_BOOLEAN('m', "dpc", &dump_args[AIQ_NOTIFIER_MATCH_DPCC], "dump dpcc module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('n', "merge", &dump_args[AIQ_NOTIFIER_MATCH_MERGE],
                        "dump merge module info ", NULL, 0, 0),
            OPT_BOOLEAN('o', "ccm", &dump_args[AIQ_NOTIFIER_MATCH_CCM], "dump ccm module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('p', "lsc", &dump_args[AIQ_NOTIFIER_MATCH_LSC], "dump lsc module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('r', "blc", &dump_args[AIQ_NOTIFIER_MATCH_BLC], "dump blc module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('V', "rawnr", &dump_args[AIQ_NOTIFIER_MATCH_RAWNR],
                        "dump rawnr module info ", NULL, 0, 0),
            OPT_BOOLEAN('t', "gic", &dump_args[AIQ_NOTIFIER_MATCH_GIC], "dump gic module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('u', "debayer", &dump_args[AIQ_NOTIFIER_MATCH_DEBAYER],
                        "dump debayer module info ", NULL, 0, 0),
            OPT_BOOLEAN('v', "lut3d", &dump_args[AIQ_NOTIFIER_MATCH_LUT3D],
                        "dump lut3d module info ", NULL, 0, 0),
            OPT_BOOLEAN('w', "dehaz", &dump_args[AIQ_NOTIFIER_MATCH_DEHAZE],
                        "dump dehaze module info ", NULL, 0, 0),
            OPT_BOOLEAN('x', "gamma", &dump_args[AIQ_NOTIFIER_MATCH_AGAMMA],
                        "dump gamma module info ", NULL, 0, 0),
            OPT_BOOLEAN('y', "degamma", &dump_args[AIQ_NOTIFIER_MATCH_ADEGAMMA],
                        "dump degamma module info ", NULL, 0, 0),
            OPT_BOOLEAN('z', "csm", &dump_args[AIQ_NOTIFIER_MATCH_CSM], "dump csm module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('B', "cgc", &dump_args[AIQ_NOTIFIER_MATCH_CGC], "dump cgc module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('C', "gain", &dump_args[AIQ_NOTIFIER_MATCH_GAIN], "dump gain module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('D', "cp", &dump_args[AIQ_NOTIFIER_MATCH_CP], "dump cproc module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('E', "ie", &dump_args[AIQ_NOTIFIER_MATCH_IE], "dump ie module info", NULL,
                        0, 0),
            OPT_BOOLEAN('F', "tnr", &dump_args[AIQ_NOTIFIER_MATCH_TNR], "dump tnr module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('G', "ynr", &dump_args[AIQ_NOTIFIER_MATCH_YNR], "dump ynr module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('H', "cnr", &dump_args[AIQ_NOTIFIER_MATCH_CNR], "dump uvnr module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('I', "sharp", &dump_args[AIQ_NOTIFIER_MATCH_SHARPEN],
                        "dump sharp module info ", NULL, 0, 0),
            OPT_BOOLEAN('J', "drc", &dump_args[AIQ_NOTIFIER_MATCH_DRC], "dump drc module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('K', "cac", &dump_args[AIQ_NOTIFIER_MATCH_CAC], "dump cac module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('M', "afd", &dump_args[AIQ_NOTIFIER_MATCH_AFD], "dump afd module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('N', "rgbir", &dump_args[AIQ_NOTIFIER_MATCH_RGBIR],
                        "dump rgbir module info ", NULL, 0, 0),
            OPT_BOOLEAN('O', "ldc", &dump_args[AIQ_NOTIFIER_MATCH_LDC], "dump ldc module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('P', "aestats", &dump_args[AIQ_NOTIFIER_MATCH_AESTATS],
                        "dump ae stats module info ", NULL, 0, 0),
            OPT_BOOLEAN('R', "histeq", &dump_args[AIQ_NOTIFIER_MATCH_HISTEQ],
                        "dump histeq module info ", NULL, 0, 0),
            OPT_BOOLEAN('S', "enhaz", &dump_args[AIQ_NOTIFIER_MATCH_ENH],
                        "dump enhance module info", NULL, 0, 0),
            OPT_BOOLEAN('T', "texest", &dump_args[AIQ_NOTIFIER_MATCH_TEXEST],
                        "dump texest module info ", NULL, 0, 0),
            OPT_BOOLEAN('U', "hsv", &dump_args[AIQ_NOTIFIER_MATCH_HSV], "dump hsv module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('V', "aibnr", &dump_args[AIQ_NOTIFIER_MATCH_AIBNR], "dump aibnr module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('W', "aiynr", &dump_args[AIQ_NOTIFIER_MATCH_AIYNR], "dump aiynr module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('\0', "help", NULL, "show this help message and exit", dbg_help_cb, 0,
                        OPT_NONEG),
            OPT_END(),
        };

        struct argparse argparse;
        argparse_init(&argparse, options, usages, 0);
        argparse_describe(&argparse, "\nselect a test case to run.", "\nuse --help for details.");

        argc = argparse_parse(&argparse, argc, (const char**)argv);
        if (_gHelp || argc < 0) {
            _gHelp = 0;

            argparse_usage_string(&argparse, buffer);
            aiq_string_printf(result, buffer);
        }
    }

    if (dump_args[AIQ_NOTIFIER_MATCH_MODS_ALL]) {
        for (int32_t i = _MODS_NUM_OFFSET; i < AIQ_NOTIFIER_MATCH_MODS_ALL; i++)
            aiq_notifier_notify_dumpinfo(&((AiqCore_t*)self)->notifier, i, result, argc, argv);

        return true;
    }

    for (int32_t i = _MODS_NUM_OFFSET; i < AIQ_NOTIFIER_MATCH_MODS_ALL; i++) {
        if (dump_args[i])
            aiq_notifier_notify_dumpinfo(&((AiqCore_t*)self)->notifier, i, result, argc, argv);
    }

    return true;
}

#endif
