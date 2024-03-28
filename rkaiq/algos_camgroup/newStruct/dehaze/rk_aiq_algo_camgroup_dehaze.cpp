/*
 * rk_aiq_algo_camgroup_dehaze.c
 *
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

#include "rk_aiq_algo_camgroup_types.h"
#include "newStruct/dehaze/dehaze_types_prvt.h"

typedef DehazeContext_t DehazeGroupContext_t;

static XCamReturn groupDehazeCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ADEHAZE("%s enter \n", __FUNCTION__ );
    DehazeGroupContext_t *ctx = (DehazeGroupContext_t *)malloc(sizeof(DehazeGroupContext_t));

    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;
    void *pCalibdbV2 = (void *)cfgInt->s_calibv2;
    dehaze_api_attrib_t *attr = (dehaze_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibdbV2, dehaze));

    *context = (RkAiqAlgoContext*)ctx;
    LOGD_ADEHAZE("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;

}

static XCamReturn groupDehazeDestroyCtx(RkAiqAlgoContext *context)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ADEHAZE("%s enter \n", __FUNCTION__ );

    DehazeGroupContext_t* pDehazeGroupCtx = (DehazeGroupContext_t*)context;
    free(pDehazeGroupCtx);

    LOGD_ADEHAZE("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupDehazePrepare(RkAiqAlgoCom* params)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ADEHAZE("%s enter \n", __FUNCTION__ );

    DehazeGroupContext_t *pDehazeGroupCtx = (DehazeGroupContext_t *)params->ctx;
    void *pCalibdbV2 = (void*)(params->u.prepare.calibv2 );
    dehaze_api_attrib_t *attr = (dehaze_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibdbV2, dehaze));

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pDehazeGroupCtx->dehaze_attrib = attr;
            return XCAM_RETURN_NO_ERROR;
        }
    }
    pDehazeGroupCtx->width             = params->u.prepare.sns_op_width;
    pDehazeGroupCtx->height            = params->u.prepare.sns_op_height;
    pDehazeGroupCtx->dehaze_attrib = attr;
    pDehazeGroupCtx->isReCal_ = true;

    LOGD_ADEHAZE("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupDehazeProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ADEHAZE("%s enter", __FUNCTION__ );

    DehazeGroupContext_t* pDehazeGroupCtx = (DehazeGroupContext_t *)inparams->ctx;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;

    //group empty
    if(procParaGroup == nullptr || procParaGroup->camgroupParmasArray == nullptr) {
        LOGE_ADEHAZE("procParaGroup or camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    // skip group algo if in MANUAL mode
    if (pDehazeGroupCtx->dehaze_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        return XCAM_RETURN_NO_ERROR;
    }

    int iso = pDehazeGroupCtx->iso;
    float blc_ob_predgain = procParaGroup->stAblcV32_proc_res.isp_ob_predgain;
    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pCurExp = &scam_3a_res->aec._effAecExpInfo;
        if((rk_aiq_working_mode_t)procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            iso =  blc_ob_predgain * scam_3a_res->hdrIso;
        } else {
            iso = scam_3a_res->hdrIso;
        }
    }

    // get ynr res
    for (int i = 0; i < YNR_ISO_CURVE_POINT_NUM; i++) {
#if RKAIQ_HAVE_DEHAZE_V12
        pDehazeGroupCtx->YnrProcRes_sigma[i] = scam_3a_res->ynr->result.dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[i];
#endif
#if RKAIQ_HAVE_DEHAZE_V14
        pDehazeGroupCtx->YnrProcRes_sigma[i] = scam_3a_res->aynr_sigma._aynr_sigma_v24[i];
#endif
    }
#if RKAIQ_HAVE_DEHAZE_V12
    // dehaze group dehaze not ready for now
    dehaze_stats_v12_t dehazeStats;
    memset(&dehazeStats, 0x0, sizeof(dehazeStats));
#endif

    if (procParaGroup->attribUpdated) {
        LOGI("%s attribUpdated", __func__);
        pDehazeGroupCtx->isReCal_ = true;
    }

    int delta_iso = abs(iso - pDehazeGroupCtx->iso);
    if (delta_iso > pDehazeGroupCtx->dehaze_attrib->stAuto.sta.sw_dehaze_byPass_thred)
        pDehazeGroupCtx->isReCal_ = true;

    rk_aiq_isp_dehaze_params_t *dehaze_param = procResParaGroup->camgroupParmasArray[0]->dehaze;

    if (pDehazeGroupCtx->isReCal_) {
#if RKAIQ_HAVE_DEHAZE_V12
        DehazeSelectParam(pDehazeGroupCtx, &dehaze_param->result, iso, false, dehaze_stats_v12);
#endif
#if RKAIQ_HAVE_DEHAZE_V14
        DehazeSelectParam(pDehazeGroupCtx, &dehaze_param->result, iso);
#endif
        dehaze_param->is_update = true;
        dehaze_param->en = pDehazeGroupCtx->dehaze_attrib->en;
        dehaze_param->bypass = pDehazeGroupCtx->dehaze_attrib->bypass;
        LOGD_ADEHAZE("group delta_iso:%d, iso %d, dehaze en:%d, bypass:%d", delta_iso, iso, dehaze_param->en, dehaze_param->bypass);

        // copy cam0's result to other cams
        for (int i = 1; i < procResParaGroup->arraySize; i++) {
            procResParaGroup->camgroupParmasArray[i]->dehaze->result = dehaze_param->result;
            procResParaGroup->camgroupParmasArray[i]->dehaze->is_update = true;
            procResParaGroup->camgroupParmasArray[i]->dehaze->en = dehaze_param->en;
            procResParaGroup->camgroupParmasArray[i]->dehaze->bypass = dehaze_param->bypass;
        }
    } else {
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            procResParaGroup->camgroupParmasArray[i]->dehaze->is_update = false;
        }
    }

    pDehazeGroupCtx->iso = iso;
    pDehazeGroupCtx->isReCal_ = false;

    LOGD_ADEHAZE("%s exit\n", __FUNCTION__);
    return ret;
}

#define RKISP_ALGO_CAMGROUP_DEHAZE_VERSION     "v0.0.1"
#define RKISP_ALGO_CAMGROUP_DEHAZE_VENDOR      "Rockchip"
#define RKISP_ALGO_CAMGROUP_DEHAZE_DESCRIPTION "Rockchip Dehaze camgroup algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupDehaze = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_DEHAZE_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_DEHAZE_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_DEHAZE_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ADHAZ,
        .id      = 0,
        .create_context  = groupDehazeCreateCtx,
        .destroy_context = groupDehazeDestroyCtx,
    },
    .prepare = groupDehazePrepare,
    .pre_process = NULL,
    .processing = groupDehazeProcessing,
    .post_process = NULL,
};
