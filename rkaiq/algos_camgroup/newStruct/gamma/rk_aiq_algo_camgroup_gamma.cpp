/*
 * rk_aiq_algo_camgroup_gamma.c
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
#include "newStruct/gamma/include/gamma_algo_api.h"

typedef struct {
    gamma_api_attrib_t* gamma_attrib;
    bool isReCal_;
} GammaGroupContext_t;

static XCamReturn groupGammaCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_AGAMMA("%s enter \n", __FUNCTION__ );
    GammaGroupContext_t *ctx = (GammaGroupContext_t *)malloc(sizeof(GammaGroupContext_t));

    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;
    void *pCalibdbV2 = (void *)cfgInt->s_calibv2;
    gamma_api_attrib_t *attr = (gamma_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibdbV2, gamma));

    *context = (RkAiqAlgoContext*)ctx;
    LOGD_AGAMMA("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;

}

static XCamReturn groupGammaDestroyCtx(RkAiqAlgoContext *context)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_AGAMMA("%s enter \n", __FUNCTION__ );

    GammaGroupContext_t* pGammaGroupCtx = (GammaGroupContext_t*)context;
    free(pGammaGroupCtx);

    LOGD_AGAMMA("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupGammaPrepare(RkAiqAlgoCom* params)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_AGAMMA("%s enter \n", __FUNCTION__ );

    GammaGroupContext_t *pGammaGroupCtx = (GammaGroupContext_t *)params->ctx;
    void *pCalibdbV2 = (void*)(params->u.prepare.calibv2 );
    gamma_api_attrib_t *attr = (gamma_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibdbV2, gamma));

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pGammaGroupCtx->gamma_attrib = attr;
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pGammaGroupCtx->gamma_attrib = attr;
    pGammaGroupCtx->isReCal_ = true;

    LOGD_AGAMMA("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupGammaProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_AGAMMA("%s enter", __FUNCTION__ );

    GammaGroupContext_t* pGammaGroupCtx = (GammaGroupContext_t *)inparams->ctx;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;

    //group empty
    if(procParaGroup == nullptr || procParaGroup->camgroupParmasArray == nullptr) {
        LOGE_AGAMMA("procParaGroup or camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    // skip group algo if in MANUAL mode
    if (pGammaGroupCtx->gamma_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        return XCAM_RETURN_NO_ERROR;
    }

    if (procParaGroup->attribUpdated) {
        LOGI("%s attribUpdated", __func__);
        pGammaGroupCtx->isReCal_ = true;
    }

    rk_aiq_isp_gamma_params_t *gamma_param = procResParaGroup->camgroupParmasArray[0]->gamma;

    if (pGammaGroupCtx->isReCal_) {
        GammaSelectParam(&pGammaGroupCtx->gamma_attrib->stAuto, &gamma_param->result);

        gamma_param->is_update = true;
        gamma_param->en = pGammaGroupCtx->gamma_attrib->en;
        gamma_param->bypass = pGammaGroupCtx->gamma_attrib->bypass;
        LOGD_AGAMMA("group gamma en:%d, bypass:%d", gamma_param->en, gamma_param->bypass);

        // copy cam0's result to other cams
        for (int i = 1; i < procResParaGroup->arraySize; i++) {
            procResParaGroup->camgroupParmasArray[i]->gamma->result = gamma_param->result;
            procResParaGroup->camgroupParmasArray[i]->gamma->is_update = true;
            procResParaGroup->camgroupParmasArray[i]->gamma->en = gamma_param->en;
            procResParaGroup->camgroupParmasArray[i]->gamma->bypass = gamma_param->bypass;
        }
    } else {
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            procResParaGroup->camgroupParmasArray[i]->gamma->is_update = false;
        }
    }

    pGammaGroupCtx->isReCal_ = false;

    LOGD_AGAMMA("%s exit\n", __FUNCTION__);
    return ret;
}

#define RKISP_ALGO_CAMGROUP_GAMMA_VERSION     "v0.0.1"
#define RKISP_ALGO_CAMGROUP_GAMMA_VENDOR      "Rockchip"
#define RKISP_ALGO_CAMGROUP_GAMMA_DESCRIPTION "Rockchip Gamma camgroup algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupGamma = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_GAMMA_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_GAMMA_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_GAMMA_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AGAMMA,
        .id      = 0,
        .create_context  = groupGammaCreateCtx,
        .destroy_context = groupGammaDestroyCtx,
    },
    .prepare = groupGammaPrepare,
    .pre_process = NULL,
    .processing = groupGammaProcessing,
    .post_process = NULL,
};
