/*
 * rk_aiq_algo_camgroup_dpcc.c
 *
 *  Copyright (c) 2023 Rockchip Corporation
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
#include "newStruct/dpc/dpc_types_prvt.h"

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)
typedef DpcContext_t DpcGroupContext_t;

static XCamReturn groupDpccCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ADPCC("%s enter \n", __FUNCTION__ );
    DpcGroupContext_t *ctx = (DpcGroupContext_t *)malloc(sizeof(DpcGroupContext_t));

    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;
    void *pCalibdbV2 = (void *)cfgInt->s_calibv2;
    dpc_api_attrib_t *attr = (dpc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibdbV2, dpc));

    *context = (RkAiqAlgoContext*)ctx;
    LOGD_ADPCC("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;

}

static XCamReturn groupDpccDestroyCtx(RkAiqAlgoContext *context)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ADPCC("%s enter \n", __FUNCTION__ );

    DpcGroupContext_t* pDpcGroupCtx = (DpcGroupContext_t*)context;
    free(pDpcGroupCtx);

    LOGD_ADPCC("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupDpccPrepare(RkAiqAlgoCom* params)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ADPCC("%s enter \n", __FUNCTION__ );

    DpcGroupContext_t *pDpcGroupCtx = (DpcGroupContext_t *)params->ctx;
    void *pCalibdbV2 = (void*)(params->u.prepare.calibv2 );
    dpc_api_attrib_t *attr = (dpc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibdbV2, dpc));

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pDpcGroupCtx->dpc_attrib = attr;
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pDpcGroupCtx->dpc_attrib = attr;
    pDpcGroupCtx->isReCal_ = true;

    LOGD_ADPCC("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupDpccProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ADPCC("%s enter", __FUNCTION__ );

    DpcGroupContext_t* pDpcGroupCtx = (DpcGroupContext_t *)inparams->ctx;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;

    //group empty
    if(procParaGroup == nullptr || procParaGroup->camgroupParmasArray == nullptr) {
        LOGD_ADPCC("procParaGroup or camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    // skip group algo if in MANUAL mode
    if (pDpcGroupCtx->dpc_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        return XCAM_RETURN_NO_ERROR;
    }

    int iso = pDpcGroupCtx->iso;
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
        pDpcGroupCtx->isReCal_ = true;
    }
    int delta_iso = abs(iso - pDpcGroupCtx->iso);
    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO)
        pDpcGroupCtx->isReCal_ = true;

    rk_aiq_isp_dpcc_params_t *dpc_param = procResParaGroup->camgroupParmasArray[0]->dpc;

    if (pDpcGroupCtx->isReCal_) {
        DpcSelectParam(pDpcGroupCtx, &dpc_param->result, iso);

        dpc_param->is_update = true;
        dpc_param->en = pDpcGroupCtx->dpc_attrib->en;
        dpc_param->bypass = pDpcGroupCtx->dpc_attrib->bypass;
        LOGD_ADPCC("group dpc en:%d, bypass:%d", dpc_param->en, dpc_param->bypass);

        // copy cam0's result to other cams
        for (int i = 1; i < procResParaGroup->arraySize; i++) {
            procResParaGroup->camgroupParmasArray[i]->dpc->result = dpc_param->result;
            procResParaGroup->camgroupParmasArray[i]->dpc->is_update = true;
            procResParaGroup->camgroupParmasArray[i]->dpc->en = dpc_param->en;
            procResParaGroup->camgroupParmasArray[i]->dpc->bypass = dpc_param->bypass;
        }
    } else {
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            procResParaGroup->camgroupParmasArray[i]->dpc->is_update = false;
        }
    }

    pDpcGroupCtx->iso = iso;
    pDpcGroupCtx->isReCal_ = false;

    LOGD_ADPCC("%s exit\n", __FUNCTION__);
    return ret;
}

#define RKISP_ALGO_CAMGROUP_DPCC_VERSION     "v0.0.1"
#define RKISP_ALGO_CAMGROUP_DPCC_VENDOR      "Rockchip"
#define RKISP_ALGO_CAMGROUP_DPCC_DESCRIPTION "Rockchip Dpcc camgroup algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupDpcc = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_DPCC_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_DPCC_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_DPCC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ADPCC,
        .id      = 0,
        .create_context  = groupDpccCreateCtx,
        .destroy_context = groupDpccDestroyCtx,
    },
    .prepare = groupDpccPrepare,
    .pre_process = NULL,
    .processing = groupDpccProcessing,
    .post_process = NULL,
};
