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

#include "rk_aiq_algo_camgroup_types.h"
#include "newStruct/sharp/include/sharp_algo_api.h"

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)

typedef struct {
    sharp_api_attrib_t* sharp_attrib;
    int iso;
    bool isReCal_;
} SharpGroupContext_t;

static XCamReturn groupSharpCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ASHARP("%s enter \n", __FUNCTION__ );
    SharpGroupContext_t *ctx = (SharpGroupContext_t *)malloc(sizeof(SharpGroupContext_t));

    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;
    void *pCalibdbV2 = (void *)cfgInt->s_calibv2;
    sharp_api_attrib_t *attr = (sharp_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibdbV2, sharp));

    *context = (RkAiqAlgoContext*)ctx;
    LOGD_ASHARP("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;

}

static XCamReturn groupSharpDestroyCtx(RkAiqAlgoContext *context)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ASHARP("%s enter \n", __FUNCTION__ );

    SharpGroupContext_t* pSharpGroupCtx = (SharpGroupContext_t*)context;
    free(pSharpGroupCtx);

    LOGD_ASHARP("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupSharpPrepare(RkAiqAlgoCom* params)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ASHARP("%s enter \n", __FUNCTION__ );

    SharpGroupContext_t *pSharpGroupCtx = (SharpGroupContext_t *)params->ctx;
    void *pCalibdbV2 = (void*)(params->u.prepare.calibv2 );
    sharp_api_attrib_t *attr = (sharp_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibdbV2, sharp));

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pSharpGroupCtx->sharp_attrib = attr;
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pSharpGroupCtx->sharp_attrib = attr;
    pSharpGroupCtx->isReCal_ = true;

    LOGD_ASHARP("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupSharpProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ASHARP("%s enter", __FUNCTION__ );

    SharpGroupContext_t* pSharpGroupCtx = (SharpGroupContext_t *)inparams->ctx;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;

    //group empty
    if(procParaGroup == nullptr || procParaGroup->camgroupParmasArray == nullptr) {
        LOGE_ASHARP("procParaGroup or camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    // skip group algo if in MANUAL mode
    if (pSharpGroupCtx->sharp_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        return XCAM_RETURN_NO_ERROR;
    }

    int iso = pSharpGroupCtx->iso;
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

    if (procParaGroup->attribUpdated) {
        LOGI("%s attribUpdated", __func__);
        pSharpGroupCtx->isReCal_ = true;
    }

    int delta_iso = abs(iso - pSharpGroupCtx->iso);
    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO)
        pSharpGroupCtx->isReCal_ = true;

    rk_aiq_isp_sharp_params_t *sharp_param = procResParaGroup->camgroupParmasArray[0]->sharp;

    if (pSharpGroupCtx->isReCal_) {
        SharpSelectParam(&pSharpGroupCtx->sharp_attrib->stAuto, &sharp_param->result, iso);

        sharp_param->is_update = true;
        sharp_param->en = pSharpGroupCtx->sharp_attrib->en;
        sharp_param->bypass = pSharpGroupCtx->sharp_attrib->bypass;
        LOGD_ASHARP("group delta_iso:%d, iso %d, sharp en:%d, bypass:%d", delta_iso, iso, sharp_param->en, sharp_param->bypass);

        // copy cam0's result to other cams
        for (int i = 1; i < procResParaGroup->arraySize; i++) {
            procResParaGroup->camgroupParmasArray[i]->sharp->result = sharp_param->result;
            procResParaGroup->camgroupParmasArray[i]->sharp->is_update = true;
            procResParaGroup->camgroupParmasArray[i]->sharp->en = sharp_param->en;
            procResParaGroup->camgroupParmasArray[i]->sharp->bypass = sharp_param->bypass;
        }
    } else {
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            procResParaGroup->camgroupParmasArray[i]->sharp->is_update = false;
        }
    }

    pSharpGroupCtx->iso = iso;
    pSharpGroupCtx->isReCal_ = false;

    LOGD_ASHARP("%s exit\n", __FUNCTION__);
    return ret;
}

#define RKISP_ALGO_CAMGROUP_SHARP_VERSION     "v0.0.1"
#define RKISP_ALGO_CAMGROUP_SHARP_VENDOR      "Rockchip"
#define RKISP_ALGO_CAMGROUP_SHARP_DESCRIPTION "Rockchip Sharp camgroup algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupSharp = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_SHARP_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_SHARP_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_SHARP_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ASHARP,
        .id      = 0,
        .create_context  = groupSharpCreateCtx,
        .destroy_context = groupSharpDestroyCtx,
    },
    .prepare = groupSharpPrepare,
    .pre_process = NULL,
    .processing = groupSharpProcessing,
    .post_process = NULL,
};
