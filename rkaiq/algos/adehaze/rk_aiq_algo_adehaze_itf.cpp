/*
 * rk_aiq_algo_adhaz_itf.c
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
/* for rockchip v2.0.0*/

#include "rk_aiq_algo_adehaze_itf.h"

#include "RkAiqCalibDbTypes.h"
#if RKAIQ_HAVE_DEHAZE_V10
#include "adehaze/rk_aiq_adehaze_algo_v10.h"
#endif
#if RKAIQ_HAVE_DEHAZE_V11
#include "adehaze/rk_aiq_adehaze_algo_v11.h"
#endif
#if RKAIQ_HAVE_DEHAZE_V11_DUO
#include "adehaze/rk_aiq_adehaze_algo_v11_duo.h"
#endif
#if RKAIQ_HAVE_DEHAZE_V12
#include "adehaze/rk_aiq_adehaze_algo_v12.h"
#endif
#if RKAIQ_HAVE_DEHAZE_V14
#include "adehaze/rk_aiq_adehaze_algo_v14.h"
#endif
#include "RkAiqCalibDbTypes.h"
#include "rk_aiq_algo_types.h"
#include "xcam_log.h"

RKAIQ_BEGIN_DECLARE

static XCamReturn create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret                  = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t* pAdehazeHandle = NULL;

    ret = AdehazeInit(&pAdehazeHandle, (CamCalibDbV2Context_t*)(cfg->calibv2));

    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ADEHAZE("%s Adehaze Init failed: %d", __FUNCTION__, ret);
        return (XCAM_RETURN_ERROR_FAILED);
    }

    *context = (RkAiqAlgoContext*)(pAdehazeHandle);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

static XCamReturn destroy_context(RkAiqAlgoContext* context) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret                  = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t* pAdehazeHandle = (AdehazeHandle_t*)context;

    ret = AdehazeRelease(pAdehazeHandle);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

static XCamReturn prepare(RkAiqAlgoCom* params) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoConfigAdhaz* config          = (RkAiqAlgoConfigAdhaz*)params;
    AdehazeHandle_t* pAdehazeHandle       = (AdehazeHandle_t*)params->ctx;
    const CamCalibDbV2Context_t* pCalibDb = config->com.u.prepare.calibv2;

    pAdehazeHandle->working_mode      = config->com.u.prepare.working_mode;
    pAdehazeHandle->is_multi_isp_mode = config->is_multi_isp_mode;
    pAdehazeHandle->width             = config->com.u.prepare.sns_op_width;
    pAdehazeHandle->height            = config->com.u.prepare.sns_op_height;

    if (pAdehazeHandle->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAdehazeHandle->FrameNumber = LINEAR_NUM;
    else if (pAdehazeHandle->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             pAdehazeHandle->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAdehazeHandle->FrameNumber = HDR_2X_NUM;
    else
        pAdehazeHandle->FrameNumber = HDR_3X_NUM;

    if (!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        LOGD_ADEHAZE("%s: Adehaze Reload Para!\n", __FUNCTION__);
#if RKAIQ_HAVE_DEHAZE_V10
        CalibDbV2_dehaze_v10_t* calibv2_adehaze_calib_V10 =
            (CalibDbV2_dehaze_v10_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adehaze_calib));
        if (calibv2_adehaze_calib_V10) {
            memcpy(&pAdehazeHandle->CalibV10, calibv2_adehaze_calib_V10,
                   sizeof(CalibDbV2_dehaze_v10_t));
            // memcpy(&pAdehazeHandle->AdehazeAtrrV10., calibv2_adehaze_calib_V10,
            // sizeof(CalibDbV2_dehaze_v10_t));
        }
#endif
#if RKAIQ_HAVE_DEHAZE_V11
        CalibDbV2_dehaze_v11_t* calibv2_adehaze_calib_V11 =
            (CalibDbV2_dehaze_v11_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adehaze_calib));
        if (calibv2_adehaze_calib_V11)
            memcpy(&pAdehazeHandle->AdehazeAtrrV11.stAuto, calibv2_adehaze_calib_V11,
                   sizeof(CalibDbV2_dehaze_v11_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V11_DUO
        CalibDbV2_dehaze_v11_t* calibv2_adehaze_calib_V11_duo =
            (CalibDbV2_dehaze_v11_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adehaze_calib));
        if (calibv2_adehaze_calib_V11_duo)
            memcpy(&pAdehazeHandle->AdehazeAtrrV11duo.stAuto, calibv2_adehaze_calib_V11_duo,
                   sizeof(CalibDbV2_dehaze_v11_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V12
        CalibDbV2_dehaze_v12_t* calibv2_adehaze_calib_V12 =
            (CalibDbV2_dehaze_v12_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adehaze_calib));
        if (calibv2_adehaze_calib_V12)
            memcpy(&pAdehazeHandle->AdehazeAtrrV12.stAuto, calibv2_adehaze_calib_V12,
                   sizeof(CalibDbV2_dehaze_v12_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V14
        CalibDbV2_dehaze_v14_t* calibv2_adehaze_calib_V14 =
            (CalibDbV2_dehaze_v14_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adehaze_calib));
        if (calibv2_adehaze_calib_V14)
            memcpy(&pAdehazeHandle->AdehazeAtrrV14.stAuto, calibv2_adehaze_calib_V14,
                   sizeof(CalibDbV2_dehaze_v14_t));
#endif
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR)
            return XCAM_RETURN_NO_ERROR;
        pAdehazeHandle->ifReCalcStAuto = true;
    } else if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGERES) {
        pAdehazeHandle->isCapture = true;
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

static XCamReturn processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret                  = XCAM_RETURN_NO_ERROR;
    bool dehaze_bypass_processing   = true;
    AdehazeHandle_t* pAdehazeHandle = (AdehazeHandle_t*)inparams->ctx;
    RkAiqAlgoProcAdhaz* pProcPara   = (RkAiqAlgoProcAdhaz*)inparams;
    RkAiqAlgoProcResAdhaz* pProcRes = (RkAiqAlgoProcResAdhaz*)outparams;
    pAdehazeHandle->FrameID         = inparams->frame_id;

    LOGD_ADEHAZE("/*************************Adehaze Start******************/ \n");

    AdehazeGetCurrData(pAdehazeHandle, pProcPara);
    dehaze_bypass_processing = AdehazeByPassProcessing(pAdehazeHandle);

    if (DehazeEnableSetting(pAdehazeHandle, pProcRes->AdehzeProcRes)) {
        // process
        if (!dehaze_bypass_processing) {
#if RKAIQ_HAVE_DEHAZE_V10
            ret = AdehazeProcess(pAdehazeHandle, pProcPara->dehaze_stats_v10, pProcRes->AdehzeProcRes);
#endif
#if RKAIQ_HAVE_DEHAZE_V11
            ret = AdehazeProcess(pAdehazeHandle, pProcPara->dehaze_stats_v11, pProcRes->AdehzeProcRes);
#endif
#if RKAIQ_HAVE_DEHAZE_V11_DUO
            ret = AdehazeProcess(pAdehazeHandle, pProcPara->dehaze_stats_v11_duo, pProcRes->AdehzeProcRes);
#endif
#if RKAIQ_HAVE_DEHAZE_V12
            ret = AdehazeProcess(pAdehazeHandle, pProcPara->dehaze_stats_v12, pProcPara->stats_true,
                                 pProcRes->AdehzeProcRes);
#endif
#if RKAIQ_HAVE_DEHAZE_V14
            ret = AdehazeProcess(pAdehazeHandle, pProcPara->dehaze_stats_v14, pProcPara->stats_true,
                                 pProcRes->AdehzeProcRes);
#endif
        }
    } else {
        LOGD_ADEHAZE("Dehaze Enable is OFF, Bypass Dehaze !!! \n");
    }

    LOGD_ADEHAZE("/*************************Adehaze over******************/ \n");

    // proc res
    outparams->cfg_update = !dehaze_bypass_processing || inparams->u.proc.init;
    if (pAdehazeHandle->ifReCalcStManual) pAdehazeHandle->ifReCalcStManual = false;
    if (pAdehazeHandle->ifReCalcStAuto) pAdehazeHandle->ifReCalcStAuto = false;

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

static XCamReturn post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAdhaz = {
    .common =
    {
        .version         = RKISP_ALGO_ADHAZ_VERSION,
        .vendor          = RKISP_ALGO_ADHAZ_VENDOR,
        .description     = RKISP_ALGO_ADHAZ_DESCRIPTION,
        .type            = RK_AIQ_ALGO_TYPE_ADHAZ,
        .id              = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare      = prepare,
    .pre_process  = NULL,
    .processing   = processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
