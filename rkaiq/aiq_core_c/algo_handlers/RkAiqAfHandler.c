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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "aiq_base.h"
#include "RkAiqAfHandler.h"
#include "RkAiqAeHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"
#include "af/rk_aiq_uapi_af_int.h"

static void _handlerAf_deinit(AiqAlgoHandler_t* pHdl) {
    AiqAlgoHandler_deinit(pHdl);
}

static void _handlerAf_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);

    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoConfigAf)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcAf)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcResAf)));

    pHdl->mResultType = RESULT_TYPE_AF_PARAM;
    //pHdl->mResultSize = sizeof(ae_param_t);

    AiqAlgoHandlerAf_t* pAfHdl = (AiqAlgoHandlerAf_t*)pHdl;
    aiqMutex_init(&pAfHdl->mAeStableMutex);
    pAfHdl->mAfMeasResSyncFalg = -1;
    pAfHdl->mAfFocusResSyncFalg = -1;

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerAf_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "ae handle prepare failed");

    RkAiqAlgoConfigAf* af_config_int = (RkAiqAlgoConfigAf*)pAlgoHandler->mConfig;
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    af_config_int->af_mode    = 6;
    af_config_int->win_h_offs = 0;
    af_config_int->win_v_offs = 0;
    af_config_int->win_h_size = 0;
    af_config_int->win_v_size = 0;
    af_config_int->lens_des   = sharedCom->snsDes.lens_des;
    // for otp
    af_config_int->otp_af = sharedCom->snsDes.otp_af;
    af_config_int->otp_pdaf = sharedCom->snsDes.otp_pdaf;

    if ((af_config_int->com.u.prepare.sns_op_width != 0) &&
            (af_config_int->com.u.prepare.sns_op_height != 0)) {
        aiqMutex_lock(&pAlgoHandler->mCfgMutex);
        RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
        ret                       = des->prepare(pAlgoHandler->mConfig);
        aiqMutex_unlock(&pAlgoHandler->mCfgMutex);
        RKAIQCORE_CHECK_RET(ret, "ae algo prepare failed");
    } else {
        LOGI_AF("input sns_op_width %d or sns_op_height %d is zero, bypass!",
                af_config_int->com.u.prepare.sns_op_width,
                af_config_int->com.u.prepare.sns_op_height);
    }

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static bool getValueFromFile(const char* path, int* pos) {
    const char* delim = " ";
    char buffer[16]   = {0};
    int fp;

    fp = open(path, O_RDONLY | O_SYNC);
    if (fp != -1) {
        if (read(fp, buffer, sizeof(buffer)) <= 0) {
            LOGE_AF("read %s failed!", path);
            goto OUT;
        } else {
            char* p = NULL;

            p = strtok(buffer, delim);
            if (p != NULL) {
                *pos = atoi(p);
            }
        }
        close(fp);
        return true;
    }

OUT:
    return false;
}

static XCamReturn _handlerAf_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandlerAf_t* pAfHdl = (AiqAlgoHandlerAf_t*)pAlgoHandler;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcAf* af_proc_int = (RkAiqAlgoProcAf*)pAlgoHandler->mProcInParam;
    RkAiqAlgoProcResAf* af_proc_res_int = (RkAiqAlgoProcResAf*)pAlgoHandler->mProcOutParam;

    aiq_params_base_t* pBase = shared->fullParams->pParamsArray[RESULT_TYPE_AF_PARAM];
    if (pBase)
        af_proc_res_int->afStats_cfg = (rk_aiq_isp_af_params_t*)pBase->_data;
    else
        af_proc_res_int->afStats_cfg = NULL;

    aiq_params_base_t* pFocusBase = shared->fullParams->pParamsArray[RESULT_TYPE_FOCUS_PARAM];
    if (pFocusBase)
        af_proc_res_int->af_focus_param = (rk_aiq_focus_params_t*)pFocusBase->_data;
    else
        af_proc_res_int->af_focus_param = NULL; 

    if ((!shared->afStatsBuf || !shared->afStatsBuf->bValid) && !sharedCom->init) {
        LOGW("fid:%d no af stats, ignore!", shared->frameId);
        return XCAM_RETURN_BYPASS;
    }

    if (shared->afStatsBuf) {
        aiq_af_stats_wrapper_t* pAfStatsWrap = (aiq_af_stats_wrapper_t*)shared->afStatsBuf->_data;
        if (pAfStatsWrap) {
            af_proc_int->afStats_stats = &pAfStatsWrap->afStats_stats;
            af_proc_int->stat_motor = &pAfStatsWrap->stat_motor;
            af_proc_int->aecExpInfo = &pAfStatsWrap->aecExpInfo;
        } else {
            af_proc_int->afStats_stats = NULL;
            af_proc_int->stat_motor = NULL;
            af_proc_int->aecExpInfo = NULL;
        }
    }

    if (shared->pdafStatsBuf)
        af_proc_int->xcam_pdaf_stats = (rk_aiq_isp_pdaf_stats_t*)shared->pdafStatsBuf->_data;
    else
        af_proc_int->xcam_pdaf_stats = NULL;

//#define ZOOM_MOVE_DEBUG
#ifdef ZOOM_MOVE_DEBUG
    int zoom_index = 0;

    if (getValueFromFile("/data/.zoom_pos", &zoom_index) == true) {
        if (pAfHdl->mLastZoomIndex != zoom_index) {
            AiqAlgoHandlerAf_setZoomIndex(pAfHdl, zoom_index);
            AiqAlgoHandlerAf_endZoomChg(pAfHdl);
            pAfHdl->mLastZoomIndex = zoom_index;
        }
    }
#endif

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret < 0) {
        LOGE_ANALYZER("af handle processing failed ret %d", ret);
        return ret;
    } else if (ret == XCAM_RETURN_BYPASS) {
        LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
        return ret;
    }

    aiqMutex_lock(&pAfHdl->mAeStableMutex);
    af_proc_int->ae_stable = pAfHdl->mAeStable;
    aiqMutex_unlock(&pAfHdl->mAeStableMutex);

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    if (des->processing) {
        aiqMutex_lock(&pAlgoHandler->mCfgMutex);
        ret = des->processing(pAlgoHandler->mProcInParam, (RkAiqAlgoResCom*)(pAlgoHandler->mProcOutParam));
        aiqMutex_unlock(&pAlgoHandler->mCfgMutex);
        if (ret < 0) {
            LOGE_ANALYZER("af algo processing failed ret %d", ret);
            return ret;
        } else if (ret == XCAM_RETURN_BYPASS) {
            LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
            return ret;
        }
    }

    if (pAfHdl->isUpdateZoomPosDone) {
        pAfHdl->isUpdateZoomPosDone = false;
        //sendSignal();
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

static XCamReturn _handlerAf_genIspResult(AiqAlgoHandler_t* pAlgoHandler, AiqFullParams_t* params,
                                           AiqFullParams_t* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    AiqAlgoHandlerAf_t* pAfHdl = (AiqAlgoHandlerAf_t*)pAlgoHandler;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResAf* af_com                = (RkAiqAlgoProcResAf*)pAlgoHandler->mProcOutParam;

    aiq_params_base_t* pAfBase = params->pParamsArray[RESULT_TYPE_AF_PARAM];
    if (!pAfBase) {
        LOGW_AF("no af params buf !");
        return XCAM_RETURN_BYPASS;
    }

    aiq_params_base_t* pFocusBase = params->pParamsArray[RESULT_TYPE_FOCUS_PARAM];
    if (!pFocusBase) {
        LOGW_AF("no af focus buf !");
        return XCAM_RETURN_BYPASS;
    }

    if (sharedCom->init) {
        pAfBase->frame_id = 0;
        pFocusBase->frame_id      = 0;
    } else {
        pAfBase->frame_id = shared->frameId;
        pFocusBase->frame_id      = shared->frameId;
    }

    AiqAlgoHandlerAe_t* ae_hdl = 
        (AiqAlgoHandlerAe_t*)pAlgoHandler->mAiqCore->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];
    if (ae_hdl) {
        if (af_com->lockae_en)
            AiqAlgoHandlerAe_setLockAeForAf(ae_hdl, af_com->lockae);
        else
            AiqAlgoHandlerAe_setLockAeForAf(ae_hdl, false);
    }

    if (af_com->af_cfg_update) {
        pAfHdl->mAfMeasResSyncFalg = shared->frameId;
        pAfBase->sync_flag = pAfHdl->mAfMeasResSyncFalg;
        cur_params->pParamsArray[RESULT_TYPE_AF_PARAM] = params->pParamsArray[RESULT_TYPE_AF_PARAM];
        pAfBase->is_update = true;
        af_com->af_cfg_update = false;
        LOGD_AF("[%d] af meas params from algo", pAfHdl->mAfMeasResSyncFalg);
    } else if (pAfHdl->mAfMeasResSyncFalg != pAfBase->sync_flag) {
        pAfBase->sync_flag = pAfHdl->mAfMeasResSyncFalg;
        if (cur_params->pParamsArray[RESULT_TYPE_AF_PARAM]) {
            pAfBase->is_update = true;
            *(rk_aiq_isp_af_params_t*)(pAfBase->_data) =
                *(rk_aiq_isp_af_params_t*)(cur_params->pParamsArray[RESULT_TYPE_AF_PARAM]->_data);
        } else {
            LOGE_AF("no af latest params !");
            pAfBase->is_update = false;
        }
        LOGD_AF("[%d] af from latest [%d]", shared->frameId, pAfHdl->mAfMeasResSyncFalg);
    } else {
        // do nothing, result in buf needn't update
        pAfBase->is_update = false;
        //LOGD_AF("[%d] af params needn't update", shared->frameId);
    }

    if (af_com->af_focus_update) {
        pAfHdl->mAfFocusResSyncFalg = shared->frameId;
        pFocusBase->sync_flag = pAfHdl->mAfFocusResSyncFalg ;
        cur_params->pParamsArray[RESULT_TYPE_FOCUS_PARAM] = params->pParamsArray[RESULT_TYPE_FOCUS_PARAM];
        pFocusBase->is_update = true;
        af_com->af_cfg_update = false;
        LOGD_AF("[%d] focus params from algo", pAfHdl->mAfFocusResSyncFalg);
    } else if (pAfHdl->mAfFocusResSyncFalg != pFocusBase->sync_flag) {
        pFocusBase->sync_flag = pAfHdl->mAfFocusResSyncFalg;
        // copy from latest result
        if (cur_params->pParamsArray[RESULT_TYPE_FOCUS_PARAM]) {
            pFocusBase->is_update = false;
            *(rk_aiq_focus_params_t*)(pFocusBase->_data) =
                *(rk_aiq_focus_params_t*)(cur_params->pParamsArray[RESULT_TYPE_FOCUS_PARAM]->_data);
        } else {
            LOGE_AF("no focus latest params !");
            pFocusBase->is_update = false;
        }
        LOGD_AF("[%d] focus params from latest [%d]", shared->frameId, pAfHdl->mAfFocusResSyncFalg);
    } else {
        pFocusBase->is_update = false;
        // do nothing, result in buf needn't update
        //LOGD_AF("[%d] focus params needn't update", shared->frameId);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerAf_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandlerAf_t));
    if (!pHdl)
        return NULL;
    AiqAlgoHandler_constructor(pHdl, des, aiqCore);
    pHdl->processing   = _handlerAf_processing;
    pHdl->genIspResult = _handlerAf_genIspResult;
    pHdl->prepare      = _handlerAf_prepare;
    pHdl->init         = _handlerAf_init;
    pHdl->deinit       = _handlerAf_deinit;
    return pHdl;
}
#ifdef RKAIQ_HAVE_AF
XCamReturn AiqAlgoHandlerAf_setCalib(AiqAlgoHandlerAf_t* pAfHdl, void* calib)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    ret = rk_aiq_uapi_af_SetCalib(pHdl->mAlgoCtx, calib);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_getCalib(AiqAlgoHandlerAf_t* pAfHdl, void* calib)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    ret = rk_aiq_uapi_af_GetCalib(pHdl->mAlgoCtx, calib);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_setAttrib(AiqAlgoHandlerAf_t* pAfHdl, rk_aiq_af_attrib_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapi_af_SetAttrib(pHdl->mAlgoCtx, *att, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_getAttrib(AiqAlgoHandlerAf_t* pAfHdl, rk_aiq_af_attrib_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    rk_aiq_uapi_af_GetAttrib(pHdl->mAlgoCtx, att);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_lock(AiqAlgoHandlerAf_t* pAfHdl)
{
    ENTER_ANALYZER_FUNCTION();
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosComShared_t* sharedCom = &pHdl->mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        aiqMutex_lock(&pHdl->mCfgMutex);
        rk_aiq_uapi_af_Lock(pHdl->mAlgoCtx);
        aiqMutex_unlock(&pHdl->mCfgMutex);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_unlock(AiqAlgoHandlerAf_t* pAfHdl)
{
    ENTER_ANALYZER_FUNCTION();
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosComShared_t* sharedCom = &pHdl->mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        aiqMutex_lock(&pHdl->mCfgMutex);
        rk_aiq_uapi_af_Unlock(pHdl->mAlgoCtx);
        aiqMutex_unlock(&pHdl->mCfgMutex);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_Oneshot(AiqAlgoHandlerAf_t* pAfHdl)
{
    ENTER_ANALYZER_FUNCTION();
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosComShared_t* sharedCom = &pHdl->mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        aiqMutex_lock(&pHdl->mCfgMutex);
        rk_aiq_uapi_af_Oneshot(pHdl->mAlgoCtx);
        aiqMutex_unlock(&pHdl->mCfgMutex);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_ManualTriger(AiqAlgoHandlerAf_t* pAfHdl)
{
    ENTER_ANALYZER_FUNCTION();
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosComShared_t* sharedCom = &pHdl->mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        aiqMutex_lock(&pHdl->mCfgMutex);
        rk_aiq_uapi_af_ManualTriger(pHdl->mAlgoCtx);
        aiqMutex_unlock(&pHdl->mCfgMutex);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_Tracking(AiqAlgoHandlerAf_t* pAfHdl)
{
    ENTER_ANALYZER_FUNCTION();
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosComShared_t* sharedCom = &pHdl->mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        aiqMutex_lock(&pHdl->mCfgMutex);
        rk_aiq_uapi_af_Tracking(pHdl->mAlgoCtx);
        aiqMutex_unlock(&pHdl->mCfgMutex);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_setZoomIndex(AiqAlgoHandlerAf_t* pAfHdl, int index)
{
    ENTER_ANALYZER_FUNCTION();
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosComShared_t* sharedCom = &pHdl->mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        aiqMutex_lock(&pHdl->mCfgMutex);
        rk_aiq_uapi_af_setZoomIndex(pHdl->mAlgoCtx, index);
        pAfHdl->isUpdateZoomPosDone = true;
        // TODO: wait signal
        aiqMutex_unlock(&pHdl->mCfgMutex);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_getZoomIndex(AiqAlgoHandlerAf_t* pAfHdl, int* index)
{
    ENTER_ANALYZER_FUNCTION();
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosComShared_t* sharedCom = &pHdl->mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        aiqMutex_lock(&pHdl->mCfgMutex);
        rk_aiq_uapi_af_getZoomIndex(pHdl->mAlgoCtx, index);
        aiqMutex_unlock(&pHdl->mCfgMutex);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_endZoomChg(AiqAlgoHandlerAf_t* pAfHdl)
{
    ENTER_ANALYZER_FUNCTION();
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosComShared_t* sharedCom = &pHdl->mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        aiqMutex_lock(&pHdl->mCfgMutex);
        rk_aiq_uapi_af_endZoomChg(pHdl->mAlgoCtx);
        aiqMutex_unlock(&pHdl->mCfgMutex);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_startZoomCalib(AiqAlgoHandlerAf_t* pAfHdl)
{
    ENTER_ANALYZER_FUNCTION();
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosComShared_t* sharedCom = &pHdl->mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        aiqMutex_lock(&pHdl->mCfgMutex);
        rk_aiq_uapi_af_startZoomCalib(pHdl->mAlgoCtx);
        pAfHdl->isUpdateZoomPosDone = true;
        //waitSignal();
        aiqMutex_unlock(&pHdl->mCfgMutex);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_resetZoom(AiqAlgoHandlerAf_t* pAfHdl)
{
    ENTER_ANALYZER_FUNCTION();
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosComShared_t* sharedCom = &pHdl->mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        aiqMutex_lock(&pHdl->mCfgMutex);
        rk_aiq_uapi_af_resetZoom(pHdl->mAlgoCtx);
        pAfHdl->isUpdateZoomPosDone = true;
        //waitSignal();
        aiqMutex_unlock(&pHdl->mCfgMutex);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_GetSearchPath(AiqAlgoHandlerAf_t* pAfHdl, rk_aiq_af_sec_path_t* path)
{
    ENTER_ANALYZER_FUNCTION();
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosComShared_t* sharedCom = &pHdl->mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        rk_aiq_uapi_af_getSearchPath(pHdl->mAlgoCtx, path);
    } else {
        path->stat = RK_AIQ_AF_SEARCH_END;
        path->search_num = 0;
        LOGD_AF("no lens, return search end");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_GetSearchResult(AiqAlgoHandlerAf_t* pAfHdl, rk_aiq_af_result_t* result)
{
    ENTER_ANALYZER_FUNCTION();
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosComShared_t* sharedCom = &pHdl->mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        rk_aiq_uapi_af_getSearchResult(pHdl->mAlgoCtx, result);
    } else {
        result->stat = RK_AIQ_AF_SEARCH_END;
        result->final_pos = 0;
        LOGD_AF("no lens, return search end");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_GetFocusRange(AiqAlgoHandlerAf_t* pAfHdl, rk_aiq_af_focusrange* range)
{
    ENTER_ANALYZER_FUNCTION();
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosComShared_t* sharedCom = &pHdl->mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        rk_aiq_uapi_af_getFocusRange(pHdl->mAlgoCtx, range);
    } else {
        range->min_pos = 0;
        range->max_pos = 64;
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_GetZoomRange(AiqAlgoHandlerAf_t* pAfHdl, rk_aiq_af_zoomrange* range)
{
    ENTER_ANALYZER_FUNCTION();
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAfHdl;

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosComShared_t* sharedCom = &pHdl->mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        rk_aiq_uapi_af_getZoomRange(pHdl->mAlgoCtx, range);
    } else {
        range->min_pos = 0;
        range->max_pos = 64;
        range->min_fl = 1;
        range->max_fl = 1;
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAf_setAeStable(AiqAlgoHandlerAf_t* pAfHdl, bool ae_stable)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pAfHdl->mAeStableMutex);
    pAfHdl->mAeStable = ae_stable;
    aiqMutex_unlock(&pAfHdl->mAeStableMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
