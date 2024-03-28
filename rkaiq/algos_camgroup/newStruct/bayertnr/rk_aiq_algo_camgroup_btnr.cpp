/*
 * rk_aiq_algo_camgroup_btnr.c
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
#include "newStruct/bayertnr/include/bayertnr_algo_api.h"

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)

typedef struct {
    btnr_api_attrib_t* btnr_attrib;
    int iso;
    bool isReCal_;
} BtnrGroupContext_t;

static XCamReturn groupBtnrCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ANR("%s enter \n", __FUNCTION__ );
    BtnrGroupContext_t *ctx = (BtnrGroupContext_t *)malloc(sizeof(BtnrGroupContext_t));

    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;
    void *pCalibdbV2 = (void *)cfgInt->s_calibv2;
    btnr_api_attrib_t *attr = (btnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibdbV2, bayertnr));

    *context = (RkAiqAlgoContext*)ctx;
    LOGD_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;

}

static XCamReturn groupBtnrDestroyCtx(RkAiqAlgoContext *context)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ANR("%s enter \n", __FUNCTION__ );

    BtnrGroupContext_t* pBtnrGroupCtx = (BtnrGroupContext_t*)context;
    free(pBtnrGroupCtx);

    LOGD_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupBtnrPrepare(RkAiqAlgoCom* params)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ANR("%s enter \n", __FUNCTION__ );

    BtnrGroupContext_t *pBtnrGroupCtx = (BtnrGroupContext_t *)params->ctx;
    void *pCalibdbV2 = (void*)(params->u.prepare.calibv2 );
    btnr_api_attrib_t *attr = (btnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibdbV2, bayertnr));

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pBtnrGroupCtx->btnr_attrib = attr;
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pBtnrGroupCtx->btnr_attrib = attr;
    pBtnrGroupCtx->isReCal_ = true;

    LOGD_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupBtnrProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ANR("%s enter", __FUNCTION__ );

    BtnrGroupContext_t* pBtnrGroupCtx = (BtnrGroupContext_t *)inparams->ctx;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;

    //group empty
    if(procParaGroup == nullptr || procParaGroup->camgroupParmasArray == nullptr) {
        LOGE_ANR("procParaGroup or camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    // skip group algo if in MANUAL mode
    if (pBtnrGroupCtx->btnr_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        return XCAM_RETURN_NO_ERROR;
    }

    int iso = pBtnrGroupCtx->iso;
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
        pBtnrGroupCtx->isReCal_ = true;
    }

    int delta_iso = abs(iso - pBtnrGroupCtx->iso);
    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO)
        pBtnrGroupCtx->isReCal_ = true;

    rk_aiq_isp_btnr_params_t *btnr_param = procResParaGroup->camgroupParmasArray[0]->btnr;

    if (pBtnrGroupCtx->isReCal_) {
        BtnrSelectParam(&pBtnrGroupCtx->btnr_attrib->stAuto, &btnr_param->result, iso);

        btnr_param->is_update = true;
        btnr_param->en = pBtnrGroupCtx->btnr_attrib->en;
        btnr_param->bypass = pBtnrGroupCtx->btnr_attrib->bypass;
        LOGD_ANR("group delta_iso:%d, iso %d, btnr en:%d, bypass:%d", delta_iso, iso, btnr_param->en, btnr_param->bypass);

        // copy cam0's result to other cams
        for (int i = 1; i < procResParaGroup->arraySize; i++) {
            procResParaGroup->camgroupParmasArray[i]->btnr->result = btnr_param->result;
            procResParaGroup->camgroupParmasArray[i]->btnr->is_update = true;
            procResParaGroup->camgroupParmasArray[i]->btnr->en = btnr_param->en;
            procResParaGroup->camgroupParmasArray[i]->btnr->bypass = btnr_param->bypass;
        }
    } else {
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            procResParaGroup->camgroupParmasArray[i]->btnr->is_update = false;
        }
    }

    pBtnrGroupCtx->iso = iso;
    pBtnrGroupCtx->isReCal_ = false;

    LOGD_ANR("%s exit\n", __FUNCTION__);
    return ret;
}

#define RKISP_ALGO_CAMGROUP_BTNR_VERSION     "v0.0.1"
#define RKISP_ALGO_CAMGROUP_BTNR_VENDOR      "Rockchip"
#define RKISP_ALGO_CAMGROUP_BTNR_DESCRIPTION "Rockchip Btnr camgroup algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupBtnr = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_BTNR_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_BTNR_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_BTNR_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AMFNR,
        .id      = 0,
        .create_context  = groupBtnrCreateCtx,
        .destroy_context = groupBtnrDestroyCtx,
    },
    .prepare = groupBtnrPrepare,
    .pre_process = NULL,
    .processing = groupBtnrProcessing,
    .post_process = NULL,
};
