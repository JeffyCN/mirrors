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

#include "RkAiqManager_c.h"

#include "hwi_c/aiq_fake_camhw.h"
#include "hwi_c/isp33/aiq_CamHwIsp33.h"
#include "hwi_c/isp35/aiq_CamHwIsp35.h"
#include "hwi_c/isp39/aiq_CamHwIsp39.h"
#include "iq_parser_v2/cis_head.h"

#define RKAIQMNG_CHECK_RET(ret, format, ...) \
    if (ret) { \
        LOGE(format, ##__VA_ARGS__); \
        return ret; \
    }

extern sensor_info_share_t g_rk1608_share_inf;

#if RKAIQ_HAVE_DUMPSYS
static int __dump_mods(void* self, st_string* result, int argc, void* argv[]);
static int __dump_raw(void* self, st_string* result, int argc, void* argv[]);
#endif

static void printSenceCisPara(AiqManager_t* pAiqManager) {
    calibdb_cis_reg_setting_t* cis_reg = (calibdb_cis_reg_setting_t*)(CALIBDBV2_GET_MODULE_PTR(
        pAiqManager->mCalibDbV2, CisRegSetting));
    if (cis_reg) {
        LOGD_ANALYZER("=== CIS Register Configuration ===");
        LOGD_ANALYZER("  Setting ID : 0x%04x", cis_reg->hw_cisCfg_setting_id);
        LOGD_ANALYZER("  DS Mode    : %d", cis_reg->hw_cisCfg_ds_mode);
        LOGD_ANALYZER("  RegCtrl Num: %d", cis_reg->regCtrl_num);

        for (int i = 0; i < cis_reg->regCtrl_num; i++) {
            LOGD_ANALYZER("    RegCtrl[%2d] -> Addr: 0x%04x, Val: 0x%04x", i,
                          cis_reg->regCtrl[i].regAddr, cis_reg->regCtrl[i].regVal);
        }
        LOGD_ANALYZER("===================================\n");
    }

    calibdb_cis_blc_t* cis_blc =
        (calibdb_cis_blc_t*)(CALIBDBV2_GET_MODULE_PTR(pAiqManager->mCalibDbV2, CisBlc));
    if (cis_blc) {
        LOGD_ANALYZER("=== CIS Black Level Correction (BLC) ===");
        LOGD_ANALYZER("  Enable               : %d", cis_blc->en);
        LOGD_ANALYZER("  BlkLevel             : %d", cis_blc->sta.hw_cisCfg_blkLevel_val);
        LOGD_ANALYZER("  BkDg Switch Enable   : %d", cis_blc->sta.bkDg.hw_cisCfg_bkdgSw_en);
        LOGD_ANALYZER("  DgBk → BkDg Threshold: %d", cis_blc->sta.bkDg.hw_cisCfg_dgBk2BkDg_thred);
        LOGD_ANALYZER("  BkDg → DgBk Threshold: %d", cis_blc->sta.bkDg.hw_cisCfg_bkDg2DgBk_thred);
        LOGD_ANALYZER("========================================\n");
    }

    calibdb_cis_hdr_t* cis_hdr =
        (calibdb_cis_hdr_t*)(CALIBDBV2_GET_MODULE_PTR(pAiqManager->mCalibDbV2, CisHdr));
    if (cis_hdr) {
        LOGD_ANALYZER("=== CIS HDR Configuration ===");
        LOGD_ANALYZER("  HDR MGE Mode     : %d", cis_hdr->sta.hw_cisCfg_hdrMge_mode);
        LOGD_ANALYZER("  CIS HDR Mode     : %d", cis_hdr->sta.hw_cisCfg_cisHdr_mode);
        LOGD_ANALYZER("  RegCtrl Count    : %d", cis_hdr->sta.regCtrl_num);

        for (int i = 0; i < cis_hdr->sta.regCtrl_num; i++) {
            LOGD_ANALYZER("    RegCtrl[%2d] -> Addr: 0x%04x, Val: 0x%04x", i,
                          cis_hdr->sta.regCtrl[i].regAddr, cis_hdr->sta.regCtrl[i].regVal);
        }
        LOGD_ANALYZER("=============================\n");
    }

    calibdb_cis_qbc_rmsc_t* cis_qbcrmsc =
        (calibdb_cis_qbc_rmsc_t*)(CALIBDBV2_GET_MODULE_PTR(pAiqManager->mCalibDbV2, CisQbcRmsc));
    if (cis_qbcrmsc) {
        LOGD_ANALYZER("=== CIS QBC RMSC Configuration ===");
        LOGD_ANALYZER("  Enable        : %d", cis_qbcrmsc->en);
        LOGD_ANALYZER("  RegCtrl Num   : %d", cis_qbcrmsc->sta.regCtrl_num);

        for (int i = 0; i < cis_qbcrmsc->sta.regCtrl_num; i++) {
            LOGD_ANALYZER("    RegCtrl[%2d] -> Addr: 0x%04x, Val: 0x%04x", i,
                          cis_qbcrmsc->sta.regCtrl[i].regAddr, cis_qbcrmsc->sta.regCtrl[i].regVal);
        }
        LOGD_ANALYZER("====================================\n");
    }

    calibdb_cis_cmps_out_t* cis_cmpsout =
        (calibdb_cis_cmps_out_t*)(CALIBDBV2_GET_MODULE_PTR(pAiqManager->mCalibDbV2, CisCmpsOut));
    if (cis_cmpsout) {
        LOGD_ANALYZER("=== CIS CMPS_OUT Configuration ===");
        LOGD_ANALYZER("  Enable      : %d", cis_cmpsout->en);
        LOGD_ANALYZER("  CMPS Mode   : %d", cis_cmpsout->sta.hw_cisCfgCmps_mode);
        LOGD_ANALYZER("===================================\n");
    }
}

static XCamReturn hwResCb(void* pCtx, AiqHwEvt_t* hwres)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqManager_t* pAiqManager = (AiqManager_t*)pCtx;
    if (hwres->type == ISP_POLL_3A_STATS) {
		AiqHwStatsEvt_t* pStatsEvt = (AiqHwStatsEvt_t*)hwres;
        uint32_t seq = hwres->frame_id;
#if defined(ISP_HW_V21)
typedef struct rkisp_isp21_stat_buffer rkisp_isp_stat_buffer;
#elif defined(ISP_HW_V30)
typedef struct rkisp3x_isp_stat_buffer rkisp_isp_stat_buffer;
#elif defined(ISP_HW_V32)
typedef struct rkisp32_isp_stat_buffer rkisp_isp_stat_buffer;
#elif defined(ISP_HW_V32_LITE)
typedef struct rkisp32_lite_stat_buffer rkisp_isp_stat_buffer;
#elif defined(ISP_HW_V39)
typedef struct rkisp39_stat_buffer rkisp_isp_stat_buffer;
#elif defined(ISP_HW_V33)
typedef struct rkisp33_stat_buffer rkisp_isp_stat_buffer;
#elif defined(ISP_HW_V35)
typedef struct rkisp35_stat_buffer rkisp_isp_stat_buffer;
#else
#error "wrong isp hw version !"
typedef rkisp_isp2x_stat_buffer rkisp_isp_stat_buffer;
#endif

        rkisp_isp_stat_buffer* stats =
            (rkisp_isp_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)hwres->vb));
        if (stats == NULL) {
            LOGE("fail to get stats ,ignore\n");
            return XCAM_RETURN_BYPASS;
        }

        if (stats->meas_type & ISP32_STAT_RTT_FST) {
            if (pAiqManager->mTBStatsCnt && pAiqManager->mLastAweekId == seq) {
                seq++;
                hwres->frame_id = seq;
                stats->frame_id = seq;
                if (g_mIsMultiIspMode) {
                    uint32_t bufLen = AiqV4l2Buffer_getV4lBufLength((AiqV4l2Buffer_t*)hwres->vb);
                    rkisp_isp_stat_buffer* right_stats = (rkisp_isp_stat_buffer*)((char*)stats + bufLen / 2);
                    right_stats->frame_id = seq;
                }
                AiqV4l2Buffer_setSequence((AiqV4l2Buffer_t*)hwres->vb, seq);
            }
            AiqCore_awakenClean(pAiqManager->mRkAiqAnalyzer, seq);
			//TODO
            //ret = AiqCamHw_setFastAeExp(pAiqManager->mmCamHw, seq);
            pAiqManager->mCamHw->mAweekId = seq;
            pAiqManager->mLastAweekId = seq;

            // push sof msg
            struct timespec tp;
            clock_gettime(CLOCK_MONOTONIC_RAW, &tp);

			Aiqisp20Evt_t hw_evt;
			AiqCamHw_make_ispHwEvt(pAiqManager->mCamHw,
					&hw_evt, seq, V4L2_EVENT_FRAME_SYNC,
					tp.tv_sec * 1000 * 1000 * 1000 + tp.tv_nsec);
			AiqCore_pushEvts(pAiqManager->mRkAiqAnalyzer, (AiqHwEvt_t*)&hw_evt);

            LOGI_ANALYZER("stats meas is special, buf frame id %d", seq);
        } else if (seq == pAiqManager->mLastAweekId) {
            return ret;
        } else if (pAiqManager->mTbInfo.is_fastboot && pAiqManager->mTBStatsCnt && seq > 1) {
            pAiqManager->mTBStatsCnt = 0;
            LOGK_ANALYZER("<TB> stats id %d, not the first run aiq", seq);
        }

        ret = AiqCore_pushStats(pAiqManager->mRkAiqAnalyzer, hwres);

    } else if (hwres->type == ISP_POLL_PARAMS) {
        rk_aiq_err_msg_t msg;
        msg.err_code = XCAM_RETURN_BYPASS;
        if (pAiqManager->mTbInfo.is_fastboot && pAiqManager->mTBStatsCnt) {
            if (pAiqManager->mErrCb && pAiqManager->mTBStatsCnt == 1) {
                (*pAiqManager->mErrCb)(&msg);
            }
            pAiqManager->mTBStatsCnt--;
        }

        if (pAiqManager->mHwEvtCb) {
            rk_aiq_hwevt_t hwevt;

            memset(&hwevt, 0, sizeof(hwevt));
            hwevt.cam_id = AiqCamHw_getCamPhyId(pAiqManager->mCamHw);
            hwevt.aiq_status = RK_AIQ_STATUS_PREAIQ_DONE;
            hwevt.ctx = pAiqManager->mHwEvtCbCtx;

            (*pAiqManager->mHwEvtCb)(&hwevt);
        }

        if (GlobalParamsManager_isFullManualMode(&pAiqManager->mGlobalParamsManager))
            AiqManager_applyAnalyzerResult(pAiqManager,
					GlobalParamsManager_getFullManParamsProxy(&pAiqManager->mGlobalParamsManager),
					false);
    } else if (hwres->type == ISPP_POLL_NR_STATS) {
        ret = AiqCore_pushStats(pAiqManager->mRkAiqAnalyzer, hwres);
    } else if (hwres->type == ISP_POLL_SOF) {
        if (pAiqManager->mTbInfo.is_fastboot && pAiqManager->mTBStatsCnt) {
            return ret;
        }
		AiqCamHw_notify_sof(pAiqManager->mCamHw, hwres);

		Aiqisp20Evt_t hw_evt;
		AiqCamHw_make_ispHwEvt(pAiqManager->mCamHw,
				&hw_evt, hwres->frame_id, V4L2_EVENT_FRAME_SYNC,
				hwres->mTimestamp);
		AiqCore_pushEvts(pAiqManager->mRkAiqAnalyzer, (AiqHwEvt_t*)&hw_evt);

        if (hwres->frame_id % 100 == 0)
            xcam_get_runtime_log_level();

        // TODO: moved to aiq core ?
        if (pAiqManager->mMetasCb) {
            rk_aiq_metas_t metas;
            memset(&metas, 0, sizeof(metas));
            metas.frame_id = hwres->frame_id ;
            metas.cam_id = AiqCamHw_getCamPhyId(pAiqManager->mCamHw);
            metas.sensor_name = pAiqManager->mSnsEntName;
            (*pAiqManager->mMetasCb)(&metas);
        }
    }
    else if (hwres->type == ISP_POLL_AIISP) {
#if RKAIQ_HAVE_AIBNR
        AiqHwAinnEvt_t *ainnEvt = (AiqHwAinnEvt_t *)hwres;
        AibnrManager_hdlEvent(&pAiqManager->mAibnrManager, ainnEvt);
#endif
#if RKAIQ_HAVE_AIYNR
        AiqHwAinnEvt_t *ainnEvt1 = (AiqHwAinnEvt_t *)hwres;
        AiynrManager_hdlEvent(&pAiqManager->mAiynrManager, ainnEvt1);
#endif
#if !defined(RKAIQ_HAVE_AIBNR) && !defined(RKAIQ_HAVE_AIYNR)
        AiqHwAiispEvt_t* aiisp_data = (AiqHwAiispEvt_t *)hwres;
        if (pAiqManager->mAiispCtx.mAiispEvtcb) {
            rk_aiq_aiisp_t aiisp_evt;
            memset(&aiisp_evt, 0, sizeof(aiisp_evt));
            aiisp_evt.wr_linecnt = pAiqManager->mCamHw->mAiisp_cfg.wr_linecnt;
            aiisp_evt.rd_linecnt = pAiqManager->mCamHw->mAiisp_cfg.rd_linecnt;
            aiisp_evt.height = aiisp_data->_height;
            aiisp_evt.sequence = aiisp_data->_base.frame_id;
            aiisp_evt.timestamp = aiisp_data->_base.mTimestamp;
            aiisp_evt.bay3dbuf = &pAiqManager->mCamHw->_bnrDrvBuf._bay3dbuf;
            aiisp_evt.iir_address = pAiqManager->mCamHw->_bnrDrvBuf.iir_address[aiisp_data->iir_index];
            aiisp_evt.gain_address = pAiqManager->mCamHw->_bnrDrvBuf.gain_address[aiisp_data->gain_index];
            aiisp_evt.aiisp_address = pAiqManager->mCamHw->_bnrDrvBuf.aiisp_address[aiisp_data->aiisp_index];
            aiisp_evt.iir_index = aiisp_data->iir_index;
            aiisp_evt.gain_index = aiisp_data->gain_index;
            aiisp_evt.aiisp_index = aiisp_data->aiisp_index;
            LOGD_ANALYZER("aiisp params: wr_linecnt %d rd_linecnt %d _height %d _frameid %d bay3dbuf.iir.buf_cnt  %d bay3dbuf.iir_size %d",
                          aiisp_evt.wr_linecnt, aiisp_evt.rd_linecnt, aiisp_evt.height, aiisp_evt.sequence, aiisp_evt.bay3dbuf->iir.buf_cnt,
                          aiisp_evt.bay3dbuf->iir.buf_size);
            (*pAiqManager->mAiispCtx.mAiispEvtcb)(&aiisp_evt, pAiqManager->mAiispCtx.ctx);
        }
        else {
            LOGE_ANALYZER("mAiispEvtcb is NULL");
        }
#endif
    }
#if RKAIQ_HAVE_AIBNR
    else if (hwres->type == ISP_POLL_AIBNR_DONE) {
        if (AibnrManager_isNeedRknn(&pAiqManager->mAibnrManager))
            RknnManager_hdlEvent(&pAiqManager->mRknnManager, (AiqHwAinnEvt_t *)hwres);
        else
            AibnrManager_hdlEvent(&pAiqManager->mAibnrManager, (AiqHwAinnEvt_t *)hwres);
    }
    else if (hwres->type == ISP_POLL_RKNN_DONE) {
        AibnrManager_hdlEvent(&pAiqManager->mAibnrManager, (AiqHwAinnEvt_t *)hwres);
    }
#endif
#if RKAIQ_HAVE_AIYNR
    else if (hwres->type == ISP_POLL_AIYNR_DONE) {
        AiynrManager_hdlEvent(&pAiqManager->mAiynrManager, (AiqHwAinnEvt_t *)hwres);
    }
#endif
    else if (hwres->type == ISP_POLL_TX) {
        ret = AiqCore_pushStats(pAiqManager->mRkAiqAnalyzer, hwres);
    } else if (hwres->type == ISP_POLL_SP) {
        LOGD_ANALYZER("ISP_IMG");
        ret = AiqCore_pushStats(pAiqManager->mRkAiqAnalyzer, hwres);
    } else if (hwres->type == ISP_NR_IMG) {
        ret = AiqCore_pushStats(pAiqManager->mRkAiqAnalyzer, hwres);
    } else if (hwres->type == ISP_GAIN) {
        LOGD_ANALYZER("ISP_GAIN");
        ret = AiqCore_pushStats(pAiqManager->mRkAiqAnalyzer, hwres);
#if RKAIQ_HAVE_PDAF
    } else if (hwres->type == ISP_POLL_PDAF_STATS) {
        LOGD_ANALYZER("ISP_POLL_PDAF_STATS");
        ret = AiqCore_pushStats(pAiqManager->mRkAiqAnalyzer, hwres);
#endif
    } else if (hwres->type == VICAP_STREAM_ON_EVT) {
        LOGD_ANALYZER("VICAP_STREAM_ON_EVT ... ");
        if (pAiqManager->mHwEvtCb) {
            rk_aiq_hwevt_t hwevt;
            memset(&hwevt, 0, sizeof(hwevt));
            hwevt.cam_id = AiqCamHw_getCamPhyId(pAiqManager->mCamHw);
#ifdef RKAIQ_ENABLE_CAMGROUP
            if (pAiqManager->mCamGroupCoreManager) {
                AiqCamGroupManager_setVicapReady(pAiqManager->mCamGroupCoreManager, &hwevt);
                if (AiqCamGroupManager_isAllVicapReady(pAiqManager->mCamGroupCoreManager))
                    hwevt.aiq_status = RK_AIQ_STATUS_VICAP_READY;
                else
                    hwevt.aiq_status = 0;
            } else
                hwevt.aiq_status = RK_AIQ_STATUS_VICAP_READY;
#else
            hwevt.aiq_status = RK_AIQ_STATUS_VICAP_READY;
#endif
            hwevt.ctx = pAiqManager->mHwEvtCbCtx;
            (*pAiqManager->mHwEvtCb)(&hwevt);
        }
    } else if (hwres->type == VICAP_RESET_EVT) {
        LOGD_ANALYZER(" VICAP_RESET_EVT... ");
        if (pAiqManager->mHwEvtCb) {
            rk_aiq_hwevt_t hwevt;

            memset(&hwevt, 0, sizeof(hwevt));
            hwevt.cam_id = AiqCamHw_getCamPhyId(pAiqManager->mCamHw);
            hwevt.aiq_status = RK_AIQ_STATUS_VICAP_RESET;
            hwevt.ctx = pAiqManager->mHwEvtCbCtx;

            LOGE_ANALYZER("cam: %d, VICAP_RESET_EVT...", hwevt.cam_id);
            (*pAiqManager->mHwEvtCb)(&hwevt);
        }
    } else if (hwres->type == VICAP_WITH_RK1608_RESET_EVT) {
        LOGD_ANALYZER(" VICAP_WITH_RK1608_RESET_EVT... ");
        if (pAiqManager->mHwEvtCb && pAiqManager->mCamHw) {
            rk_aiq_hwevt_t hwevt;

            memset(&hwevt, 0, sizeof(hwevt));
            for(int id = 0; id < 8; id++)
                hwevt.multi_cam.multi_cam_id[id] = -1;

            int i = 0;
            for(int camPhyId = 0; camPhyId < CAM_INDEX_FOR_1608; camPhyId++) {
                if (g_rk1608_share_inf.raw_proc_unit[camPhyId]) {
                    hwevt.multi_cam.multi_cam_id[i++] = camPhyId;
                }
            }
            hwevt.multi_cam.cam_count = i;
            hwevt.cam_id = -1;
            hwevt.aiq_status = RK_AIQ_STATUS_VICAP_WITH_MULTI_CAM_RESET;
            hwevt.ctx = pAiqManager->mHwEvtCbCtx;

            for (i = 0; i < 8; i++) {
                LOGV_ANALYZER("multi_cam_id[%d]: %d \n", i, hwevt.multi_cam.multi_cam_id[i]);
            }

            (*pAiqManager->mHwEvtCb)(&hwevt);
        }
    } else if (hwres->type == VICAP_POLL_SCL) {
        ret = AiqCore_pushStats(pAiqManager->mRkAiqAnalyzer, hwres);
    }

exit:
    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn AiqManager_applyAnalyzerResult(AiqManager_t* pAiqManager, AiqFullParams_t* results, bool ignoreIsUpdate)
{
    ENTER_XCORE_FUNCTION();
    //xcam_get_runtime_log_level();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqFullParams_t* aiqParams = results;

    if (!results) {
        LOGW_ANALYZER("empty aiq params results!");
        return ret;
    }

    if (aiqParams->_base.frame_id == pAiqManager->mLastAweekId)
        ignoreIsUpdate = true;

    if (aiqParams->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM]) {
		aiqList_push(pAiqManager->mParamsList, &aiqParams->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM]);
    }

    if (aiqParams->pParamsArray[RESULT_TYPE_IRIS_PARAM]) {
		aiqList_push(pAiqManager->mParamsList, &aiqParams->pParamsArray[RESULT_TYPE_IRIS_PARAM]);
    }

    if (aiqParams->pParamsArray[RESULT_TYPE_FOCUS_PARAM] &&
		(ignoreIsUpdate || aiqParams->pParamsArray[RESULT_TYPE_FOCUS_PARAM]->is_update)) {
        aiqParams->pParamsArray[RESULT_TYPE_FOCUS_PARAM]->is_update = false;
		aiqList_push(pAiqManager->mParamsList, &aiqParams->pParamsArray[RESULT_TYPE_FOCUS_PARAM]);
    }

#define APPLY_ANALYZER_RESULT(lc, BC) \
    { \
        if (GlobalParamsManager_isFullManualMode(&pAiqManager->mGlobalParamsManager)) { \
            aiq_params_base_t* manual_params = \
                GlobalParamsManager_getAndClearPending2(&pAiqManager->mGlobalParamsManager, RESULT_TYPE_##BC##_PARAM); \
            if (manual_params) { \
                LOGD("new manual result type: %s", #BC);\
                manual_params->frame_id = aiqParams->_base.frame_id; \
				aiqList_push(pAiqManager->mParamsList, &manual_params); \
            } \
        } else if (aiqParams->pParamsArray[RESULT_TYPE_##BC##_PARAM] && \
				(ignoreIsUpdate || aiqParams->pParamsArray[RESULT_TYPE_##BC##_PARAM]->is_update)) { \
            aiqParams->pParamsArray[RESULT_TYPE_##BC##_PARAM]->is_update = false; \
			aiqList_push(pAiqManager->mParamsList, &aiqParams->pParamsArray[RESULT_TYPE_##BC##_PARAM]); \
            AIQ_REF_BASE_REF(&aiqParams->pParamsArray[RESULT_TYPE_##BC##_PARAM]->_ref_base); \
        } \
    } \

#if RKAIQ_HAVE_ASD
    APPLY_ANALYZER_RESULT(Cpsl, CPSL);
#endif

#if RKAIQ_HAVE_AE
#if USE_NEWSTRUCT
    APPLY_ANALYZER_RESULT(AESTATS, AESTATS);
#else
    APPLY_ANALYZER_RESULT(Aec, AEC);
    APPLY_ANALYZER_RESULT(Hist, HIST);
#endif
#endif
#if RKAIQ_HAVE_AWB
    APPLY_ANALYZER_RESULT(AwbGain, AWBGAIN);
    APPLY_ANALYZER_RESULT(Awb, AWB);// call AWBGAIN  before AWB

#endif

#if RKAIQ_HAVE_AF
    APPLY_ANALYZER_RESULT(Af, AF);
#endif
#if RKAIQ_HAVE_DPCC
    APPLY_ANALYZER_RESULT(Dpcc, DPCC);
#endif
#if RKAIQ_HAVE_MERGE
    APPLY_ANALYZER_RESULT(Merge, MERGE);
#endif
#if RKAIQ_HAVE_TMO
    APPLY_ANALYZER_RESULT(Tmo, TMO);
#endif
#if RKAIQ_HAVE_CCM
    APPLY_ANALYZER_RESULT(Ccm, CCM);
#endif
#if RKAIQ_HAVE_BLC
    APPLY_ANALYZER_RESULT(Blc, BLC);
#endif
#if RKAIQ_HAVE_ANR
    APPLY_ANALYZER_RESULT(Rawnr, RAWNR);
#endif
#if RKAIQ_HAVE_GIC
    APPLY_ANALYZER_RESULT(Gic, GIC);
#endif
#if RKAIQ_HAVE_DEBAYER
#if USE_NEWSTRUCT
    APPLY_ANALYZER_RESULT(Dm, DEBAYER);
#else
    APPLY_ANALYZER_RESULT(Debayer, DEBAYER);
#endif
#endif
#if RKAIQ_HAVE_LDCH
    APPLY_ANALYZER_RESULT(Ldch, LDCH);
#endif
#if RKAIQ_HAVE_3DLUT
    APPLY_ANALYZER_RESULT(Lut3d, LUT3D);
#endif
#if RKAIQ_HAVE_DEHAZE
    APPLY_ANALYZER_RESULT(Dehaze, DEHAZE);
    APPLY_ANALYZER_RESULT(Histeq, HISTEQ);
#endif
#if RKAIQ_HAVE_HISTEQ
    APPLY_ANALYZER_RESULT(Histeq, HISTEQ);
#endif
#if RKAIQ_HAVE_ENHANCE
    APPLY_ANALYZER_RESULT(Enh, ENH);
#endif
#if RKAIQ_HAVE_HSV
    APPLY_ANALYZER_RESULT(Hsv, HSV);
#endif
#if RKAIQ_HAVE_GAMMA
#if USE_NEWSTRUCT
    APPLY_ANALYZER_RESULT(Gamma, AGAMMA);
#else
    APPLY_ANALYZER_RESULT(Agamma, AGAMMA);
#endif
#endif
#if RKAIQ_HAVE_DEGAMMA
    APPLY_ANALYZER_RESULT(Adegamma, ADEGAMMA);
#endif
#if RKAIQ_HAVE_WDR
    APPLY_ANALYZER_RESULT(Wdr, WDR);
#endif
#if RKAIQ_HAVE_CSM
    APPLY_ANALYZER_RESULT(Csm, CSM);
#endif
#if RKAIQ_HAVE_CGC
    APPLY_ANALYZER_RESULT(Cgc, CGC);
#endif
    APPLY_ANALYZER_RESULT(Conv422, CONV422);
    APPLY_ANALYZER_RESULT(Yuvconv, YUVCONV);
#if RKAIQ_HAVE_GAIN
    APPLY_ANALYZER_RESULT(Gain, GAIN);
#endif
#if RKAIQ_HAVE_ACP
    APPLY_ANALYZER_RESULT(Cp, CP);
#endif
#if RKAIQ_HAVE_AIE
    APPLY_ANALYZER_RESULT(Ie, IE);
#endif
#if RKAIQ_HAVE_AMD
    APPLY_ANALYZER_RESULT(Motion, MOTION);
#endif
#if RKAIQ_HAVE_ANR
    APPLY_ANALYZER_RESULT(Tnr, TNR);
    APPLY_ANALYZER_RESULT(Ynr, YNR);
    APPLY_ANALYZER_RESULT(Uvnr, UVNR);
    APPLY_ANALYZER_RESULT(Sharpen, SHARPEN);
    APPLY_ANALYZER_RESULT(Edgeflt, EDGEFLT);
#endif
#if defined(RKAIQ_HAVE_SHARP_V40) || defined(RKAIQ_HAVE_SHARP_V41)
    APPLY_ANALYZER_RESULT(texEst, TEXEST);
#endif
#if RKAIQ_HAVE_FEC
    APPLY_ANALYZER_RESULT(Fec, FEC);
#endif
#if RKAIQ_HAVE_ORB
    APPLY_ANALYZER_RESULT(Orb, ORB);
#endif
    // ispv21
#if RKAIQ_HAVE_DRC
    APPLY_ANALYZER_RESULT(Drc, DRC);

#endif
#if RKAIQ_HAVE_YNR
    APPLY_ANALYZER_RESULT(Ynr, YNR);
#endif
#if RKAIQ_HAVE_CNR
    APPLY_ANALYZER_RESULT(Cnr, UVNR);
#endif
#if RKAIQ_HAVE_SHARP
#if USE_NEWSTRUCT
    APPLY_ANALYZER_RESULT(Sharp, SHARPEN);
#else
    APPLY_ANALYZER_RESULT(Sharpen, SHARPEN);
#endif
#endif
#if RKAIQ_HAVE_BAYERNR || RKAIQ_HAVE_BAYER2DNR
    APPLY_ANALYZER_RESULT(Baynr, RAWNR);
#endif
    // ispv3x
#if RKAIQ_HAVE_LSC
    APPLY_ANALYZER_RESULT(Lsc, LSC);
#endif
#if RKAIQ_HAVE_CAC
    APPLY_ANALYZER_RESULT(Cac, CAC);
#endif
#if RKAIQ_HAVE_BAYERTNR
#if USE_NEWSTRUCT
    APPLY_ANALYZER_RESULT(Btnr, TNR);
    APPLY_ANALYZER_RESULT(Btnr, TNR2);
#else
    APPLY_ANALYZER_RESULT(Tnr, TNR);
#endif
#endif
    // ispv32
#if RKAIQ_HAVE_AFD
    APPLY_ANALYZER_RESULT(Afd, AFD);
#endif
#if RKAIQ_HAVE_YUVME
    APPLY_ANALYZER_RESULT(Yuvme, MOTION);
#endif
#if RKAIQ_HAVE_RGBIR_REMOSAIC
    APPLY_ANALYZER_RESULT(Rgbir, RGBIR);
#endif
#if RKAIQ_HAVE_LDC
    APPLY_ANALYZER_RESULT(Ldc, LDC);
#endif
#if RKAIQ_HAVE_AIBNR
    APPLY_ANALYZER_RESULT(Aibnr, AIBNR);
#endif
#if RKAIQ_HAVE_AIYNR
    APPLY_ANALYZER_RESULT(Aiynr, AIYNR);
#endif

	ret = AiqCamHw_applyAnalyzerResultList(pAiqManager->mCamHw, pAiqManager->mParamsList);
	if (ret) {
        LOGE_ANALYZER("cid:%d, fid:%d apply to hw failed", aiqParams->_base.frame_id,
				AiqCamHw_getCamPhyId(pAiqManager->mCamHw));
	}

    AiqListItem_t* pItem = NULL;
    bool rm              = false;
    AIQ_LIST_FOREACH(pAiqManager->mParamsList, pItem, rm) {
        aiq_params_base_t* params = *(aiq_params_base_t**)(pItem->_pData);
        pItem = aiqList_erase_item_locked(pAiqManager->mParamsList, pItem);
        rm    = true;
        LOGE_ANALYZER("cid:%d, fid:%d apply %d failed", aiqParams->_base.frame_id,
				AiqCamHw_getCamPhyId(pAiqManager->mCamHw), params->type);
        AIQ_REF_BASE_UNREF(&params->_ref_base);
    }

    EXIT_XCORE_FUNCTION();

    return ret;
}

static XCamReturn
AiqManager_applyAnalyzerExpResult(AiqManager_t* pAiqManager, aiq_params_base_t* results)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!results) {
        return ret;
    }

	aiqList_push(pAiqManager->mParamsList, &results);

    ret = AiqCamHw_applyAnalyzerResultList(pAiqManager->mCamHw, pAiqManager->mParamsList);
    if (ret) {
        LOGE_ANALYZER("cid:%d, fid:%d apply exp param to hw failed", results->frame_id,
				AiqCamHw_getCamPhyId(pAiqManager->mCamHw));
	}

    return ret;

}

static void
rkAiqCalcDone(void* pCtx, AiqFullParams_t* results)
{
    ENTER_XCORE_FUNCTION();
    AiqManager_applyAnalyzerResult((AiqManager_t*)pCtx, results, false);
    EXIT_XCORE_FUNCTION();
}

static void
rkAiqCalcExpDone(void* pCtx, aiq_params_base_t* results)
{
    ENTER_XCORE_FUNCTION();
    AiqManager_applyAnalyzerExpResult((AiqManager_t*)pCtx, results);
    EXIT_XCORE_FUNCTION();
}

XCamReturn AiqManager_setMirrorFlip(AiqManager_t* pAiqManager, bool mirror, bool flip, int skip_frm_cnt)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_XCORE_FUNCTION();
    if (pAiqManager->_state == AIQ_STATE_INVALID) {
        LOGE_ANALYZER("wrong aiq state !");
        return XCAM_RETURN_ERROR_FAILED;
    }
    ret = AiqCamHw_setSensorFlip(pAiqManager->mCamHw, mirror, flip, skip_frm_cnt);
    if (ret == XCAM_RETURN_NO_ERROR) {
        // notify aiq sensor flip is changed
        AiqCore_setSensorFlip(pAiqManager->mRkAiqAnalyzer, mirror, flip);
        pAiqManager->mCurMirror = mirror;
        pAiqManager->mCurFlip = flip;
    } else {
        LOGW_ANALYZER("set mirror %d, flip %d error", mirror, flip);
    }
    return ret;
    EXIT_XCORE_FUNCTION();
}

XCamReturn AiqManager_getMirrorFlip(AiqManager_t* pAiqManager, bool* mirror, bool* flip)
{
    ENTER_XCORE_FUNCTION();
    if (pAiqManager->_state == AIQ_STATE_INVALID) {
        LOGE_ANALYZER("wrong aiq state !");
        return XCAM_RETURN_ERROR_FAILED;
    }

    *mirror = pAiqManager->mCurMirror;
    *flip = pAiqManager->mCurFlip;

    EXIT_XCORE_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}


static void setDefMirrorFlip(AiqManager_t* pAiqManager)
{
    /* set defalut mirror & flip from iq*/
    CalibDb_Sensor_ParaV2_t* sensor =
        (CalibDb_Sensor_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR(pAiqManager->mCalibDbV2, sensor_calib));

    bool def_mirr = sensor->CISFlip & 0x1 ? true : false;
    bool def_flip = sensor->CISFlip & 0x2 ? true : false;
    AiqManager_setMirrorFlip(pAiqManager, def_mirr, def_flip, 0);
}

XCamReturn AiqManager_init(AiqManager_t* pAiqManager, const char* sns_ent_name, rk_aiq_error_cb err_cb, rk_aiq_metas_cb metas_cb)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    pAiqManager->mErrCb = err_cb;
    pAiqManager->mMetasCb = metas_cb;
	pAiqManager->mSnsEntName = sns_ent_name;

    XCAM_ASSERT (pAiqManager->mSnsEntName);
    XCAM_ASSERT (pAiqManager->mRkAiqAnalyzer);
    XCAM_ASSERT (pAiqManager->mCamHw);
    XCAM_ASSERT (pAiqManager->mCalibDbV2);

	pAiqManager->mLastAweekId = -1;
	pAiqManager->mAnalyzeCb.pCtx = pAiqManager;
	pAiqManager->mAnalyzeCb.rkAiqCalcDone    = rkAiqCalcDone;
    pAiqManager->mAnalyzeCb.rkAiqCalcExpDone = rkAiqCalcExpDone;
	pAiqManager->mAnalyzeCb.rkAiqCalcFailed  = NULL;
	AiqCore_setAnalyzeResultCb(pAiqManager->mRkAiqAnalyzer, &pAiqManager->mAnalyzeCb);

    GlobalParamsManager_setManager(&pAiqManager->mGlobalParamsManager, pAiqManager);
    CamHW_setManager(pAiqManager->mCamHw, pAiqManager);
    //ret = GlobalParamsManager_init(&pAiqManager->mGlobalParamsManager, false, pAiqManager->mCalibDbV2);
	//AiqCore_setGlobalParamsManager(pAiqManager->mRkAiqAnalyzer, &pAiqManager->mGlobalParamsManager);

    ret |= AiqCore_init(pAiqManager->mRkAiqAnalyzer, pAiqManager->mSnsEntName, pAiqManager->mCalibDbV2);
    RKAIQMNG_CHECK_RET(ret, "analyzer init error %d !", ret);

	pAiqManager->mHwResCb._pCtx = pAiqManager;
	pAiqManager->mHwResCb.hwResCb = hwResCb;
	AiqCamHw_setHwResListener(pAiqManager->mCamHw, &pAiqManager->mHwResCb);
	if (pAiqManager->mCamHw->mIsFake) {
#ifdef RKAIQ_ENABLE_FAKECAM
		AiqCamHwFake_init((AiqCamHwFake_t*)pAiqManager->mCamHw, pAiqManager->mSnsEntName);
#endif
	} else {
#if defined(ISP_HW_V39)
        pAiqManager->mTBStatsCnt = 1;
        ret = AiqCamHwIsp39_init(pAiqManager->mCamHw, pAiqManager->mSnsEntName);
#elif defined(ISP_HW_V33)
        pAiqManager->mTBStatsCnt = 2;
        ret = AiqCamHwIsp33_init(pAiqManager->mCamHw, pAiqManager->mSnsEntName);
#elif defined(ISP_HW_V35)
        pAiqManager->mTBStatsCnt = 2;
        ret = AiqCamHwIsp35_init(pAiqManager->mCamHw, pAiqManager->mSnsEntName);
#else
		XCAM_ASSERT(0);
#endif
	}
    RKAIQMNG_CHECK_RET(ret, "camHw init error %d !", ret);
    ret = GlobalParamsManager_init(&pAiqManager->mGlobalParamsManager, false, pAiqManager->mCalibDbV2);
    AiqCore_setGlobalParamsManager(pAiqManager->mRkAiqAnalyzer, &pAiqManager->mGlobalParamsManager);
    pAiqManager->_state = AIQ_STATE_INITED;

    isp_drv_share_mem_ops_t *mem_ops = NULL;
    AiqCamHw_getShareMemOps(pAiqManager->mCamHw, &mem_ops);
    AiqCore_setShareMemOps(pAiqManager->mRkAiqAnalyzer, mem_ops);
    // set default mirror & flip
    setDefMirrorFlip(pAiqManager);

	AiqListConfig_t paramsListCfg;
	paramsListCfg._name      = "paramsList";
	paramsListCfg._item_nums = RESULT_TYPE_MAX_PARAM;
	paramsListCfg._item_size = sizeof(aiq_params_base_t*);
	pAiqManager->mParamsList = aiqList_init(&paramsListCfg);
	if (!pAiqManager->mParamsList)
		LOGE_ANALYZER("init %s error", paramsListCfg._name);

#if RKAIQ_HAVE_DUMPSYS
    pAiqManager->dump_mods = __dump_mods;
    pAiqManager->dump_raw = __dump_raw;
#endif

#if RKAIQ_HAVE_AIBNR
    AibnrManager_init(pAiqManager, &pAiqManager->mAibnrManager);
#endif
#if RKAIQ_HAVE_AIYNR
    AiynrManager_init(&pAiqManager->mAiynrManager);
#endif

    RknnManager_init(&pAiqManager->mRknnManager);

    return ret;
}

XCamReturn AiqManager_prepare(AiqManager_t* pAiqManager, uint32_t width, uint32_t height, rk_aiq_working_mode_t mode)
{
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_exposure_sensor_descriptor sensor_des;

    XCAM_ASSERT (pAiqManager->mCalibDbV2);
#ifdef RUNTIME_MODULE_DEBUG
#ifndef RK_SIMULATOR_HW
    get_dbg_force_disable_mods_env();
#endif
#endif

#if RKAIQ_HAVE_AIBNR
    bool mNewAibnrEn =
        GlobalParamsManager_get_SingleModuleEn(&pAiqManager->mGlobalParamsManager, RESULT_TYPE_AIBNR_PARAM);
    if (mNewAibnrEn != pAiqManager->mLastAibnrEn) {
        pAiqManager->mLastAibnrEn = mNewAibnrEn;
        int delayCnt = pAiqManager->mLastAibnrEn ? ISP_PARAMS_AIBNR_EFFECT_DELAY_CNT : ISP_PARAMS_EFFECT_DELAY_CNT;
        AiqCore_reinitForNewStatsDelay(pAiqManager->mRkAiqAnalyzer, delayCnt);
        AiqCamHw_setAibnrDelayCnt(pAiqManager->mCamHw, delayCnt);
    }
#endif

#if RKAIQ_HAVE_AIYNR
    bool mNewAiynrEn =
        GlobalParamsManager_get_SingleModuleEn(&pAiqManager->mGlobalParamsManager, RESULT_TYPE_AIYNR_PARAM);
    if (mNewAiynrEn != pAiqManager->mLastAiynrEn) {
        pAiqManager->mLastAiynrEn = mNewAiynrEn;
        int delayCnt = pAiqManager->mLastAiynrEn ? ISP_PARAMS_AIYNR_EFFECT_DELAY_CNT : ISP_PARAMS_EFFECT_DELAY_CNT;
        AiqCore_reinitForNewStatsDelay(pAiqManager->mRkAiqAnalyzer, delayCnt);
        AiqCamHw_setAiynrDelayCnt(pAiqManager->mCamHw, delayCnt);
    }
#endif

    CalibDb_Sensor_ParaV2_t* sensor_calib =
        (CalibDb_Sensor_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR(pAiqManager->mCalibDbV2, sensor_calib));

    int working_mode_hw = RK_AIQ_WORKING_MODE_NORMAL;
    if (mode == RK_AIQ_WORKING_MODE_NORMAL) {
        working_mode_hw = mode;
    } else {
        /**
         * 1. Check if scene_cis is enabled.
         * 2. Otherwise, use the old configuration method.
         */
        calibdb_cis_hdr_t* cis_hdr =
            (calibdb_cis_hdr_t*)(CALIBDBV2_GET_MODULE_PTR(pAiqManager->mCalibDbV2, CisHdr));
        if (cis_hdr) {
            if (mode == RK_AIQ_WORKING_MODE_ISP_HDR2) {
                if (RK_AIQ_HDR_IS_HDR2(cis_hdr->sta.hw_cisCfg_hdrMge_mode))
                    working_mode_hw = cis_hdr->sta.hw_cisCfg_hdrMge_mode;
                else
                    LOGE_ANALYZER("HDR mode mismatch: expected 0x%x/0x%x, got 0x%x from calibdb!",
                                  RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR, RK_AIQ_ISP_HDR_MODE_2_LINE_HDR,
                                  cis_hdr->sta.hw_cisCfg_hdrMge_mode);
            } else if (mode == RK_AIQ_WORKING_MODE_ISP_HDR3) {
                if (RK_AIQ_HDR_IS_HDR3(cis_hdr->sta.hw_cisCfg_hdrMge_mode))
                    working_mode_hw = cis_hdr->sta.hw_cisCfg_hdrMge_mode;
                else
                    LOGE_ANALYZER("HDR mode mismatch: expected 0x%x/0x%x, got 0x%x from calibdb!",
                                  RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR, RK_AIQ_ISP_HDR_MODE_3_LINE_HDR,
                                  cis_hdr->sta.hw_cisCfg_hdrMge_mode);
            } else {
                LOGE_ANALYZER("Not supported HDR mode 0x%x!", mode);
            }

            pAiqManager->mCisHdrMode = cis_hdr->sta.hw_cisCfg_cisHdr_mode;
        } else {
            if (mode == RK_AIQ_WORKING_MODE_ISP_HDR2) {
                working_mode_hw = RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR;
            } else if (mode == RK_AIQ_WORKING_MODE_ISP_HDR3) {
                working_mode_hw = RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR;
            } else {
                LOGE_ANALYZER("Not supported HDR mode 0x%x!", mode);
            }

            pAiqManager->mCisHdrMode = sensor_calib->CISHdrSet.line_mode;
        }

        pAiqManager->mHdrMergeMode = working_mode_hw;

        LOGK_ANALYZER("CisHdrMode: %d, working_mode_hw: 0x%x", pAiqManager->mCisHdrMode,
                      working_mode_hw);
    }

    printSenceCisPara(pAiqManager);

    AiqCamHw_setCalib(pAiqManager->mCamHw, pAiqManager->mCalibDbV2);

#if RKAIQ_HAVE_MEMC
    if (AiqCamHw_getMemcNeededDelay(pAiqManager->mCamHw)) {
        // MEMC delay 1 frame more than noraml mode
        int delayCnt = 3;
        AiqCore_reinitForNewStatsDelay(pAiqManager->mRkAiqAnalyzer, delayCnt);
        AiqCamHw_setAibnrDelayCnt(pAiqManager->mCamHw, delayCnt);
    }
#endif

#ifdef RKAIQ_ENABLE_CAMGROUP
    AiqCamHw_setGroupMode(pAiqManager->mCamHw, pAiqManager->mCamGroupCoreManager ? true : false, pAiqManager->mIsMain);
#endif

    if(mode != RK_AIQ_WORKING_MODE_NORMAL)
        ret = pAiqManager->mCamHw->prepare(pAiqManager->mCamHw, width, height, working_mode_hw,
                                           sensor_calib->CISExpUpdate.Hdr.time_update,
                                           sensor_calib->CISExpUpdate.Hdr.gain_update);
    else
        ret = pAiqManager->mCamHw->prepare(pAiqManager->mCamHw, width, height, working_mode_hw,
                                           sensor_calib->CISExpUpdate.Linear.time_update,
                                           sensor_calib->CISExpUpdate.Linear.gain_update);

    RKAIQMNG_CHECK_RET(ret, "camhw prepare error %d", ret);

    xcam_mem_clear(sensor_des);
    ret = AiqCamHw_getSensorModeData(pAiqManager->mCamHw, pAiqManager->mSnsEntName, &sensor_des);
    AiqCamHw_setSnsOtpInfo(pAiqManager->mCamHw, &sensor_des.otp_awb, sensor_des.otp_lsc, &pAiqManager->mRkAiqAnalyzer->mUserOtpInfo);
    pAiqManager->sensor_output_width = sensor_des.sensor_output_width;
    pAiqManager->sensor_output_height = sensor_des.sensor_output_height;
    int w, h, aligned_w, aligned_h;
    ret = AiqCamHw_get_sp_resolution(pAiqManager->mCamHw, &w, &h, &aligned_w, &aligned_h);
    ret = AiqCore_set_sp_resolution(pAiqManager->mRkAiqAnalyzer, &w, &h, &aligned_w, &aligned_h);
#if RKAIQ_HAVE_PDAF
    ret = AiqCore_set_pdaf_support(pAiqManager->mRkAiqAnalyzer,
			AiqCamHw_get_pdaf_support(pAiqManager->mCamHw));
    ret = AiqCore_set_pdaf_type(pAiqManager->mRkAiqAnalyzer,
			AiqCamHw_get_pdaf_type(pAiqManager->mCamHw));
#endif

    RKAIQMNG_CHECK_RET(ret, "getSensorModeData error %d", ret);
    AiqCore_notifyIspStreamMode(pAiqManager->mRkAiqAnalyzer,
		AiqCamHw_getIspStreamMode(pAiqManager->mCamHw));
    AiqCore_setTranslaterIspUniteMode(pAiqManager->mRkAiqAnalyzer,
        AiqCamHw_getIspUniteMode(pAiqManager->mCamHw));
    ret = AiqCore_prepare(pAiqManager->mRkAiqAnalyzer, &sensor_des, working_mode_hw);
    RKAIQMNG_CHECK_RET(ret, "analyzer prepare error %d", ret);
#if RKAIQ_HAVE_AIBNR
    AibnrManager_prepare(&pAiqManager->mAibnrManager,
                        pAiqManager->mCamHw,
                        pAiqManager->mCalibDbV2,
                        sensor_des.isp_acq_width,
                        sensor_des.isp_acq_height);
#endif
#if RKAIQ_HAVE_AIYNR
    pAiqManager->mAiynrManager.is_aiynr_enable = pAiqManager->mCamHw->_aiynr_en;
    AiynrManager_prepare(&pAiqManager->mAiynrManager,
                        pAiqManager->mCamHw,
                        pAiqManager->mCalibDbV2,
                        sensor_des.isp_acq_width,
                        sensor_des.isp_acq_height);
#endif
    RknnManager_prepare(&pAiqManager->mRknnManager,
                        pAiqManager->mCamHw,
                        pAiqManager->mCalibDbV2);

    AiqFullParams_t* initParams = AiqCore_getAiqFullParams(pAiqManager->mRkAiqAnalyzer);

#ifdef RKAIQ_ENABLE_CAMGROUP
    if (!pAiqManager->mCamGroupCoreManager) {
#endif
        ret = AiqManager_applyAnalyzerResult(pAiqManager, initParams, true);
        RKAIQMNG_CHECK_RET(ret, "set initial params error %d", ret);
#ifdef RKAIQ_ENABLE_CAMGROUP
    }
#endif

    pAiqManager->mWorkingMode = mode;
    pAiqManager->mOldWkModeForGray = RK_AIQ_WORKING_MODE_NORMAL;
    pAiqManager->mWidth = width;
    pAiqManager->mHeight = height;
    pAiqManager->_state = AIQ_STATE_PREPARED;

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn AiqManager_start(AiqManager_t* pAiqManager)
{
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // restart
    if (pAiqManager->_state == AIQ_STATE_STOPED) {
        AiqFullParams_t* initParams = AiqCore_getAiqFullParams(pAiqManager->mRkAiqAnalyzer);
        AiqManager_applyAnalyzerResult(pAiqManager, initParams, true);
    } else if (pAiqManager->_state == AIQ_STATE_STARTED) {
        return ret;
    }

    ret = AiqCore_start(pAiqManager->mRkAiqAnalyzer);
    RKAIQMNG_CHECK_RET(ret, "analyzer start error %d", ret);

    ret = AiqCamHw_start(pAiqManager->mCamHw);
    RKAIQMNG_CHECK_RET(ret, "camhw start error %d", ret);
#if RKAIQ_HAVE_AIBNR
    AibnrManager_start(&pAiqManager->mAibnrManager);
    RKAIQMNG_CHECK_RET(ret, "AibnrManager start error %d", ret);
#endif
#if RKAIQ_HAVE_AIYNR
    AiynrManager_start(&pAiqManager->mAiynrManager);
    RKAIQMNG_CHECK_RET(ret, "AiynrManager start error %d", ret);
#endif
    RknnManager_start(&pAiqManager->mRknnManager);
    RKAIQMNG_CHECK_RET(ret, "RknnManager start error %d", ret);

    pAiqManager->_state = AIQ_STATE_STARTED;

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn AiqManager_stop(AiqManager_t* pAiqManager, bool keep_ext_hw_st)
{
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pAiqManager->_state  == AIQ_STATE_STOPED) {
        return ret;
    }

    pAiqManager->_state = AIQ_STATE_STOPED;

    ret = AiqCore_stop(pAiqManager->mRkAiqAnalyzer);
    RKAIQMNG_CHECK_RET(ret, "analyzer stop error %d", ret);

    AiqCamHw_keepHwStAtStop(pAiqManager->mCamHw, keep_ext_hw_st);
    ret = AiqCamHw_stop(pAiqManager->mCamHw);
    RKAIQMNG_CHECK_RET(ret, "camhw stop error %d", ret);
#if RKAIQ_HAVE_AIBNR
    AibnrManager_stop(&pAiqManager->mAibnrManager);
    RKAIQMNG_CHECK_RET(ret, "AibnrManager stop error %d", ret);
#endif
#if RKAIQ_HAVE_AIYNR
    AiynrManager_stop(&pAiqManager->mAiynrManager);
    RKAIQMNG_CHECK_RET(ret, "AiynrManager stop error %d", ret);
#endif
    RknnManager_stop(&pAiqManager->mRknnManager);
    RKAIQMNG_CHECK_RET(ret, "RknnManager stop error %d", ret);

	AiqCore_clean(pAiqManager->mRkAiqAnalyzer);
	AiqCamHw_clean(pAiqManager->mCamHw);

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn AiqManager_deinit(AiqManager_t* pAiqManager)
{
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // stop first in prepared status, some resources and process were
    // done at prepare stage
    if (pAiqManager->_state == AIQ_STATE_PREPARED)
        AiqManager_stop(pAiqManager, false);

    ret = AiqCore_deinit(pAiqManager->mRkAiqAnalyzer);
    RKAIQMNG_CHECK_RET(ret, "analyzer deinit error %d", ret);

	if (pAiqManager->mCamHw->mIsFake) {
#ifdef RKAIQ_ENABLE_FAKECAM
		AiqCamHwFake_deinit((AiqCamHwFake_t*)pAiqManager->mCamHw);
#endif
	} else {
		AiqCamHwBase_deinit(pAiqManager->mCamHw);
	}
    if (pAiqManager->mCalibDbV2) {
        aiq_free(pAiqManager->mCalibDbV2);
        pAiqManager->mCalibDbV2 = NULL;
    }
    if (pAiqManager->tuningCalib) {
        CamCalibDbFreeCalibByJ2S(pAiqManager->tuningCalib);
        pAiqManager->mCalibDbV2 = NULL;
    }

	if (pAiqManager->mParamsList) {
		aiqList_deinit(pAiqManager->mParamsList);
		pAiqManager->mParamsList = NULL;
	}

    GlobalParamsManager_deinit(&pAiqManager->mGlobalParamsManager);
#if RKAIQ_HAVE_AIBNR
    AibnrManager_deinit(&pAiqManager->mAibnrManager);
#endif
#if RKAIQ_HAVE_AIYNR
    AiynrManager_deinit(&pAiqManager->mAiynrManager);
#endif
    RknnManager_deinit(&pAiqManager->mRknnManager);

    pAiqManager->_state = AIQ_STATE_INVALID;

    EXIT_XCORE_FUNCTION();

    return ret;
}

void AiqManager_setAiqCalibDb(AiqManager_t* pAiqManager, const CamCalibDbV2Context_t* calibDb)
{
    ENTER_XCORE_FUNCTION();
    XCAM_ASSERT (!pAiqManager->mCalibDbV2);
    pAiqManager->mCalibDbV2 = (CamCalibDbV2Context_t*)aiq_mallocz(sizeof(CamCalibDbV2Context_t));
    *pAiqManager->mCalibDbV2 = *calibDb;
    EXIT_XCORE_FUNCTION();
}

XCamReturn AiqManager_updateCalibDb(AiqManager_t* pAiqManager, const CamCalibDbV2Context_t* newCalibDb)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pAiqManager->mWorkingMode != RK_AIQ_WORKING_MODE_NORMAL) {
        calibdb_cis_hdr_t* cis_hdr =
            (calibdb_cis_hdr_t*)(CALIBDBV2_GET_MODULE_PTR(pAiqManager->mCalibDbV2, CisHdr));
        if (cis_hdr) {
            if (pAiqManager->mHdrMergeMode != cis_hdr->sta.hw_cisCfg_hdrMge_mode ||
                pAiqManager->mCisHdrMode != cis_hdr->sta.hw_cisCfg_cisHdr_mode) {
                if (pAiqManager->_state != AIQ_STATE_STOPED &&
                    pAiqManager->_state != AIQ_STATE_INITED) {
                    LOGE_ANALYZER("CisHdrMode changed from %d to %d, not support now",
                                  pAiqManager->mCisHdrMode, cis_hdr->sta.hw_cisCfg_hdrMge_mode);
                    XCAM_ASSERT(0);
                    return XCAM_RETURN_ERROR_FAILED;
                }
            }
        }
    }

    *pAiqManager->mCalibDbV2 = *(CamCalibDbV2Context_t*)newCalibDb;
    AiqCamHw_setCalib(pAiqManager->mCamHw, newCalibDb);
#if RKAIQ_HAVE_AIBNR
    AibnrManager_setCalib(&pAiqManager->mAibnrManager, pAiqManager->mCalibDbV2);
#endif
#if RKAIQ_HAVE_AIYNR
    AiynrManager_setCalib(&pAiqManager->mAiynrManager, pAiqManager->mCalibDbV2);
#endif

    ret = AiqCore_setCalib(pAiqManager->mRkAiqAnalyzer, pAiqManager->mCalibDbV2);

    if (!AiqCore_isRunningState(pAiqManager->mRkAiqAnalyzer)) {
        AiqCore_updateCalibDbBrutal(pAiqManager->mRkAiqAnalyzer, pAiqManager->mCalibDbV2);
    } else {
		TuningCalib update_list;
		update_list.calib = (CamCalibDbV2Context_t*)newCalibDb;
		strcpy(update_list.moduleNames[0], "colorAsGrey");
		strcpy(update_list.moduleNames[1], "ALL");
		update_list.moduleNamesSize = 2;
        AiqCore_calibTuning(pAiqManager->mRkAiqAnalyzer, pAiqManager->mCalibDbV2, &update_list);
    }

    EXIT_XCORE_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqManager_syncSofEvt(AiqManager_t* pAiqManager, AiqHwEvt_t* hwres)
{
    ENTER_XCORE_FUNCTION();

    if (hwres->type == ISP_POLL_SOF) {
        xcam_get_runtime_log_level();
		AiqCamHw_notify_sof(pAiqManager->mCamHw, hwres);

		Aiqisp20Evt_t hw_evt;
		AiqCamHw_make_ispHwEvt(pAiqManager->mCamHw,
				&hw_evt, hwres->frame_id, V4L2_EVENT_FRAME_SYNC,
				hwres->mTimestamp);
		AiqCore_pushEvts(pAiqManager->mRkAiqAnalyzer, (AiqHwEvt_t*)&hw_evt);

        // TODO: moved to aiq core ?
        if (pAiqManager->mMetasCb) {
            rk_aiq_metas_t metas;
            memset(&metas, 0, sizeof(metas));
            metas.frame_id = hwres->frame_id;
            metas.cam_id = AiqCamHw_getCamPhyId(pAiqManager->mCamHw);
            metas.sensor_name = pAiqManager->mSnsEntName;
            (*pAiqManager->mMetasCb)(&metas);
        }
    }

    EXIT_XCORE_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqManager_setModuleCtl(AiqManager_t* pAiqManager, rk_aiq_module_id_t mId, bool mod_en)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = AiqCamHw_setModuleCtl(pAiqManager->mCamHw, mId, mod_en);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn AiqManager_getModuleCtl(AiqManager_t* pAiqManager, rk_aiq_module_id_t mId, bool* mod_en)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = AiqCamHw_getModuleCtl(pAiqManager->mCamHw, mId, mod_en);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn AiqManager_rawdataPrepare(AiqManager_t* pAiqManager, rk_aiq_raw_prop_t prop)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef RKAIQ_ENABLE_FAKECAM
    ret = AiqCamHwFake_rawdataPrepare((AiqCamHwFake_t*)pAiqManager->mCamHw, prop);
#endif
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn AiqManager_enqueueRawBuffer(AiqManager_t* pAiqManager, void *rawdata, bool sync)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef RKAIQ_ENABLE_FAKECAM
    ret = AiqCamHwFake_enqueueRawBuffer((AiqCamHwFake_t*)pAiqManager->mCamHw, rawdata, sync);
#endif
    EXIT_XCORE_FUNCTION();
    return ret;

}

XCamReturn AiqManager_enqueueRawFile(AiqManager_t* pAiqManager, const char *path)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	// TODO
#ifdef RKAIQ_ENABLE_FAKECAM
    ret = AiqCamHwFake_enqueueRawFile((AiqCamHwFake_t*)pAiqManager->mCamHw, path);
#endif
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn AiqManager_registRawdataCb(AiqManager_t* pAiqManager, void (*callback)(void *))
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef RKAIQ_ENABLE_FAKECAM
    ret = AiqCamHwFake_registRawdataCb((AiqCamHwFake_t*)pAiqManager->mCamHw, callback);
#endif
    EXIT_XCORE_FUNCTION();
    return ret;
}

void AiqManager_setMulCamConc(AiqManager_t* pAiqManager, bool cc)
{
	AiqCamHw_setMulCamConc(pAiqManager->mCamHw, cc);
	AiqCore_setMulCamConc(pAiqManager->mRkAiqAnalyzer, cc);
}

XCamReturn AiqManager_calibTuning(AiqManager_t* pAiqManager, CamCalibDbV2Context_t* aiqCalib,
                       TuningCalib* change_list)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqCamHw_setCalib(pAiqManager->mCamHw, aiqCalib);
    bool need_check = true;
    if (change_list->moduleNamesSize == 1 &&
        (strstr(change_list->moduleNames[0], "ae") || strstr(change_list->moduleNames[0], "wb") || strstr(change_list->moduleNames[0], "af")))
        need_check = false;
    GlobalParamsManager_switchCalibDb(&pAiqManager->mGlobalParamsManager, aiqCalib, need_check);
#if RKAIQ_HAVE_AIBNR
    AibnrManager_setCalib(&pAiqManager->mAibnrManager, aiqCalib);
#endif
#if RKAIQ_HAVE_AIYNR
    AiynrManager_setCalib(&pAiqManager->mAiynrManager, aiqCalib);
#endif

    ret = AiqCore_setCalib(pAiqManager->mRkAiqAnalyzer, aiqCalib);

	AiqCore_calibTuning(pAiqManager->mRkAiqAnalyzer, aiqCalib, change_list);

    // Won't free calib witch from iqfiles
    *pAiqManager->mCalibDbV2 = *aiqCalib;
    if (pAiqManager->mNeedFreeCalib) {
        CamCalibDbFreeCalibByJ2S(pAiqManager->tuningCalib);
        pAiqManager->tuningCalib = aiqCalib;
    }
    else {
        pAiqManager->mNeedFreeCalib = true;
    }
    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn AiqManager_setVicapStreamMode(AiqManager_t* pAiqManager, int on, bool isSingleMode)
{
    AiqCore_setAovMode(pAiqManager->mRkAiqAnalyzer, !on);
    return AiqCamHw_setVicapStreamMode(pAiqManager->mCamHw, on, isSingleMode);
}

void AiqManager_pushImuData(AiqManager_t* pAiqManager, AiqImuData_t *data)
{
    AiqCamHw_pushImuData(pAiqManager->mCamHw, data);
}

#if RKAIQ_HAVE_DUMPSYS
static int __dump_mods(void* self, st_string* result, int argc, void* argv[]) {
    if (!self) return -1;

    AiqManager_t* mgr = (AiqManager_t*)self;
    char argvArray[256][256];
    char* extended_argv[256];
    int extended_argc = 0;

    extended_argv[0] = argvArray[0];
    extended_argc++;

    if (argc > 0) {
        char mod[32] = {0};
        xcam_to_lowercase((char*)argv[0], mod);

        snprintf(argvArray[extended_argc], sizeof(argvArray[extended_argc]), "--%s", mod);
        extended_argv[extended_argc] = argvArray[extended_argc];
        extended_argc++;
    }

    {
        AiqCore_t* analyzer = mgr->mRkAiqAnalyzer;

        if (analyzer && analyzer->dump_algos) {
            snprintf(argvArray[0], sizeof(argvArray[0]), "%s", "algo");
            extended_argv[0] = argvArray[0];

            analyzer->dump_algos(analyzer, result, extended_argc, (void**)extended_argv);
        }
    }

    {
        AiqCamHwBase_t* cam_hw = mgr->mCamHw;

        if (cam_hw && cam_hw->dump) {
            snprintf(argvArray[0], sizeof(argvArray[0]), "%s", "hwi");
            extended_argv[0] = argvArray[0];

            cam_hw->dump(cam_hw, result, extended_argc, (void**)extended_argv);
        }
    }

    return 0;
}

static int __dump_raw(void* self, st_string* result, int argc, void* argv[]) {
    int ret = -1;
    AiqManager_t* mgr = (AiqManager_t*)self;

#if RKAIQ_HAVE_AIBNR
    if (!strcmp(argv[1], "aibnr")) {
        int dump_num = atoi(argv[2]);

        ret = AibnrManager_dumpRaw(&mgr->mAibnrManager, dump_num);
        if (ret)
            aiq_string_printf(result, "set dump raw flag failed\n");
        else
            aiq_string_printf(result, "set dump raw flag success\n");

        aiq_string_printf(result, "\n");
    }
#endif

#if RKAIQ_HAVE_AIRMS
    if (!strcmp(argv[1], "airms")) {
        int dump_num = atoi(argv[2]);

        if (mgr->mCamHw->mAiRmsProcUnit)
            ret = AiqAiRmsStreamProcUnit_dumpRaw(mgr->mCamHw->mAiRmsProcUnit, dump_num);
        if (ret)
            aiq_string_printf(result, "set dump raw flag failed\n");
        else
            aiq_string_printf(result, "set dump raw flag success\n");

        aiq_string_printf(result, "\n");
    }
#endif

#if RKAIQ_HAVE_AIYNR
    if (!strcmp(argv[1], "aiynr")) {
        int dump_num = atoi(argv[2]);

        ret = AiynrManager_dumpRaw(&mgr->mAiynrManager, dump_num);
        if (ret)
            aiq_string_printf(result, "set dump raw flag failed\n");
        else
            aiq_string_printf(result, "set dump raw flag success\n");

        aiq_string_printf(result, "\n");
    }
#endif

    return ret;
}

#endif
