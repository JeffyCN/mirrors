/*
 * rk_aiq_algo_camgroup_ynr.cpp
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
#include "newStruct/ynr/include/ynr_algo_api.h"

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)

typedef struct {
    ynr_api_attrib_t* ynr_attrib;
    int iso;
    bool isReCal_;
} YnrGroupContext_t;

static XCamReturn groupYnrCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ANR("%s enter \n", __FUNCTION__ );
    YnrGroupContext_t *ctx = (YnrGroupContext_t *)malloc(sizeof(YnrGroupContext_t));

    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;
    void *pCalibdbV2 = (void *)cfgInt->s_calibv2;
    ynr_api_attrib_t *attr = (ynr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibdbV2, ynr));

    *context = (RkAiqAlgoContext*)ctx;
    LOGD_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;

}

static XCamReturn groupYnrDestroyCtx(RkAiqAlgoContext *context)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ANR("%s enter \n", __FUNCTION__ );

    YnrGroupContext_t* pYnrGroupCtx = (YnrGroupContext_t*)context;
    free(pYnrGroupCtx);

    LOGD_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupYnrPrepare(RkAiqAlgoCom* params)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ANR("%s enter \n", __FUNCTION__ );

    YnrGroupContext_t *pYnrGroupCtx = (YnrGroupContext_t *)params->ctx;
    void *pCalibdbV2 = (void*)(params->u.prepare.calibv2 );
    ynr_api_attrib_t *attr = (ynr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibdbV2, ynr));

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pYnrGroupCtx->ynr_attrib = attr;
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pYnrGroupCtx->ynr_attrib = attr;
    pYnrGroupCtx->isReCal_ = true;

    LOGD_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupYnrProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ANR("%s enter", __FUNCTION__ );

    YnrGroupContext_t* pYnrGroupCtx = (YnrGroupContext_t *)inparams->ctx;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;

    //group empty
    if(procParaGroup == nullptr || procParaGroup->camgroupParmasArray == nullptr) {
        LOGE_ANR("procParaGroup or camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    // skip group algo if in MANUAL mode
    if (pYnrGroupCtx->ynr_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        return XCAM_RETURN_NO_ERROR;
    }

    int iso = pYnrGroupCtx->iso;
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
        pYnrGroupCtx->isReCal_ = true;
    }

    int delta_iso = abs(iso - pYnrGroupCtx->iso);
    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO)
        pYnrGroupCtx->isReCal_ = true;

    rk_aiq_isp_ynr_params_t *ynr_param = procResParaGroup->camgroupParmasArray[0]->ynr;

    if (pYnrGroupCtx->isReCal_) {
        YnrSelectParam(&pYnrGroupCtx->ynr_attrib->stAuto, &ynr_param->result, iso);

        ynr_param->is_update = true;
        ynr_param->en = pYnrGroupCtx->ynr_attrib->en;
        ynr_param->bypass = pYnrGroupCtx->ynr_attrib->bypass;
        LOGD_ANR("group delta_iso:%d, iso %d, ynr en:%d, bypass:%d", delta_iso, iso, ynr_param->en, ynr_param->bypass);

        // copy cam0's result to other cams
        for (int i = 1; i < procResParaGroup->arraySize; i++) {
            procResParaGroup->camgroupParmasArray[i]->ynr->result = ynr_param->result;
            procResParaGroup->camgroupParmasArray[i]->ynr->is_update = true;
            procResParaGroup->camgroupParmasArray[i]->ynr->en = ynr_param->en;
            procResParaGroup->camgroupParmasArray[i]->ynr->bypass = ynr_param->bypass;
        }
    } else {
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            procResParaGroup->camgroupParmasArray[i]->ynr->is_update = false;
        }
    }

    pYnrGroupCtx->iso = iso;
    pYnrGroupCtx->isReCal_ = false;

    LOGD_ANR("%s exit\n", __FUNCTION__);
    return ret;
}

#define RKISP_ALGO_CAMGROUP_YNR_VERSION     "v0.0.1"
#define RKISP_ALGO_CAMGROUP_YNR_VENDOR      "Rockchip"
#define RKISP_ALGO_CAMGROUP_YNR_DESCRIPTION "Rockchip Ynr camgroup algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupYnr = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_YNR_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_YNR_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_YNR_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AYNR,
        .id      = 0,
        .create_context  = groupYnrCreateCtx,
        .destroy_context = groupYnrDestroyCtx,
    },
    .prepare = groupYnrPrepare,
    .pre_process = NULL,
    .processing = groupYnrProcessing,
    .post_process = NULL,
};
