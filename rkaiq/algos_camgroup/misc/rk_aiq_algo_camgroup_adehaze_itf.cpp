/*
 * rk_aiq_algo_camgroup_adhaz_itf.c
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

#include "RkAiqCalibDbTypes.h"
#include "algos/adehaze/rk_aiq_algo_adehaze_itf.h"
#include "rk_aiq_algo_camgroup_types.h"
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
#include "RkAiqCalibDbTypes.h"
#include "xcam_log.h"

RKAIQ_BEGIN_DECLARE

static XCamReturn create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret                          = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t* pAdehazeGrpHandle      = NULL;
    AlgoCtxInstanceCfgCamGroup* instanc_int = (AlgoCtxInstanceCfgCamGroup*)cfg;

    ret = AdehazeInit(&pAdehazeGrpHandle, (CamCalibDbV2Context_t*)(instanc_int->s_calibv2));

    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ADEHAZE("%s Adehaze Init failed: %d", __FUNCTION__, ret);
        return (XCAM_RETURN_ERROR_FAILED);
    }

    *context = (RkAiqAlgoContext*)(pAdehazeGrpHandle);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

static XCamReturn destroy_context(RkAiqAlgoContext* context) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret                     = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t* pAdehazeGrpHandle = (AdehazeHandle_t*)context;

    ret = AdehazeRelease(pAdehazeGrpHandle);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

static XCamReturn prepare(RkAiqAlgoCom* params) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoCamGroupPrepare* config      = (RkAiqAlgoCamGroupPrepare*)params;
    AdehazeHandle_t* pAdehazeGrpHandle    = (AdehazeHandle_t*)params->ctx;
    const CamCalibDbV2Context_t* pCalibDb = config->s_calibv2;

    pAdehazeGrpHandle->working_mode = config->gcom.com.u.prepare.working_mode;
    pAdehazeGrpHandle->width        = config->gcom.com.u.prepare.sns_op_width;
    pAdehazeGrpHandle->height       = config->gcom.com.u.prepare.sns_op_height;

    if (pAdehazeGrpHandle->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAdehazeGrpHandle->FrameNumber = LINEAR_NUM;
    else if (pAdehazeGrpHandle->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             pAdehazeGrpHandle->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAdehazeGrpHandle->FrameNumber = HDR_2X_NUM;
    else
        pAdehazeGrpHandle->FrameNumber = HDR_3X_NUM;

    if (!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        LOGD_ADEHAZE("%s: Adehaze Reload Para!\n", __FUNCTION__);
#if RKAIQ_HAVE_DEHAZE_V10
        CalibDbV2_dehaze_v10_t* calibv2_adehaze_calib_V10 =
            (CalibDbV2_dehaze_v10_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adehaze_calib));
        if (calibv2_adehaze_calib_V10)
            memcpy(&pAdehazeGrpHandle->CalibV10, calibv2_adehaze_calib_V10,
                   sizeof(CalibDbV2_dehaze_v10_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V11
        CalibDbV2_dehaze_v11_t* calibv2_adehaze_calib_V11 =
            (CalibDbV2_dehaze_v11_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adehaze_calib));
        if (calibv2_adehaze_calib_V11)
            memcpy(&pAdehazeGrpHandle->AdehazeAtrrV11.stAuto, calibv2_adehaze_calib_V11,
                   sizeof(CalibDbV2_dehaze_v11_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V11_DUO
        CalibDbV2_dehaze_v11_t* calibv2_adehaze_calib_V11_duo =
            (CalibDbV2_dehaze_v11_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adehaze_calib));
        if (calibv2_adehaze_calib_V11_duo)
            memcpy(&pAdehazeGrpHandle->AdehazeAtrrV11duo.stAuto, calibv2_adehaze_calib_V11_duo,
                   sizeof(CalibDbV2_dehaze_v11_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V12
        CalibDbV2_dehaze_v12_t* calibv2_adehaze_calib_V12 =
            (CalibDbV2_dehaze_v12_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adehaze_calib));
        if (calibv2_adehaze_calib_V12)
            memcpy(&pAdehazeGrpHandle->AdehazeAtrrV12.stAuto, calibv2_adehaze_calib_V12,
                   sizeof(CalibDbV2_dehaze_v12_t));
#endif
        pAdehazeGrpHandle->ifReCalcStAuto = true;
    } else if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGERES) {
        pAdehazeGrpHandle->isCapture = true;
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

static XCamReturn processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret                            = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t* pAdehazeGrpHandle        = (AdehazeHandle_t*)inparams->ctx;
    RkAiqAlgoCamGroupProcIn* pGrpProcPara     = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* pGrpProcResPara = (RkAiqAlgoCamGroupProcOut*)outparams;
    pAdehazeGrpHandle->FrameID                = inparams->frame_id;

    LOGD_ADEHAZE("/*************************Adehaze Group Start******************/ \n");

    AdehazeGetCurrDataGroup(pAdehazeGrpHandle, pGrpProcPara->camgroupParmasArray[0]);
    AdehazeByPassProcessing(pAdehazeGrpHandle);

    bool Enable = DehazeEnableSetting(pAdehazeGrpHandle);

    if (Enable) {
    // process
    if (!(pAdehazeGrpHandle->byPassProc)) ret = AdehazeProcess(pAdehazeGrpHandle);
    } else {
        LOGD_ADEHAZE("Group Dehaze Enable is OFF, Bypass Dehaze !!! \n");
    }

    LOGD_ADEHAZE("/*************************Adehaze Group Over******************/ \n");

    // proc res
    pAdehazeGrpHandle->ProcRes.enable = pAdehazeGrpHandle->ProcRes.enable;
    pAdehazeGrpHandle->ProcRes.update = !(pAdehazeGrpHandle->byPassProc);
#if RKAIQ_HAVE_DEHAZE_V10
    pAdehazeGrpHandle->ProcRes.enable = true;
    pAdehazeGrpHandle->ProcRes.update = !(pAdehazeGrpHandle->byPassProc);
    if (pAdehazeGrpHandle->ProcRes.update) {
        for (int i = 0; i < pGrpProcResPara->arraySize; i++) {
            pGrpProcResPara->camgroupParmasArray[i]->_adehazeConfig->enable =
                pAdehazeGrpHandle->ProcRes.enable;
            pGrpProcResPara->camgroupParmasArray[i]->_adehazeConfig->update =
                pAdehazeGrpHandle->ProcRes.update;
            memcpy(&pGrpProcResPara->camgroupParmasArray[i]->_adehazeConfig->ProcResV10,
                   &pAdehazeGrpHandle->ProcRes.ProcResV10, sizeof(AdehazeV10ProcResult_t));
        }
    }
#endif
#if RKAIQ_HAVE_DEHAZE_V11
    if (pAdehazeGrpHandle->ProcRes.update) {
        for (int i = 0; i < pGrpProcResPara->arraySize; i++) {
            pGrpProcResPara->camgroupParmasArray[i]->_adehazeConfig->enable =
                pAdehazeGrpHandle->ProcRes.enable;
            pGrpProcResPara->camgroupParmasArray[i]->_adehazeConfig->update =
                pAdehazeGrpHandle->ProcRes.update;
            memcpy(&pGrpProcResPara->camgroupParmasArray[i]->_adehazeConfig->ProcResV11,
                   &pAdehazeGrpHandle->ProcRes.ProcResV11, sizeof(AdehazeV11ProcResult_t));
        }
    }
#endif
#if RKAIQ_HAVE_DEHAZE_V11_DUO
    if (pAdehazeGrpHandle->ProcRes.update) {
        for (int i = 0; i < pGrpProcResPara->arraySize; i++) {
            pGrpProcResPara->camgroupParmasArray[i]->_adehazeConfig->enable =
                pAdehazeGrpHandle->ProcRes.enable;
            pGrpProcResPara->camgroupParmasArray[i]->_adehazeConfig->update =
                pAdehazeGrpHandle->ProcRes.update;
            memcpy(&pGrpProcResPara->camgroupParmasArray[i]->_adehazeConfig->ProcResV11duo,
                   &pAdehazeGrpHandle->ProcRes.ProcResV11duo, sizeof(AdehazeV11duoProcResult_t));
        }
    }
#endif
#if RKAIQ_HAVE_DEHAZE_V12
    if (pAdehazeGrpHandle->ProcRes.update) {
        for (int i = 0; i < pGrpProcResPara->arraySize; i++) {
            pGrpProcResPara->camgroupParmasArray[i]->_adehazeConfig->enable =
                pAdehazeGrpHandle->ProcRes.enable;
            pGrpProcResPara->camgroupParmasArray[i]->_adehazeConfig->update =
                pAdehazeGrpHandle->ProcRes.update;
            memcpy(&pGrpProcResPara->camgroupParmasArray[i]->_adehazeConfig->ProcResV12,
                   &pAdehazeGrpHandle->ProcRes.ProcResV12, sizeof(AdehazeV12ProcResult_t));
        }
    }
#endif

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAdhaz = {
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
