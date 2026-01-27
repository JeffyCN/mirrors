/*
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

#include "algo_types_priv.h"
#include "3dlut_types_prvt.h"
#include "xcam_log.h"

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

#include "interpolation.h"
#include "c_base/aiq_base.h"

#if RKAIQ_HAVE_DUMPSYS
#include "include/algo_3dlut_info.h"
#include "rk_info_utils.h"
#endif

static int illu_estm_once(alut3d_param_illuLink_t *illuLinks, uint8_t illuLink_len, float awbGain[2]) {
    int ret = -1;
    uint8_t case_id = 0;
    if (illuLink_len == 0)
        return -1;

    float minDist = 9999999;
    float dist;
    float nRG = awbGain[0]; //current R/G gain
    float nBG = awbGain[1]; //current B/G gain

    int i;
    for (i=0; i<illuLink_len; i++) {
        alut3d_param_illuLink_t *pIlluCase = &illuLinks[i];
        float refRG = pIlluCase->sw_lut3dC_wbGainR_val;
        float refBG = pIlluCase->sw_lut3dC_wbGainB_val;
        dist = sqrt((nRG - refRG) * (nRG -  refRG) + (nBG -  refBG) * (nBG -  refBG));
        if(dist < minDist) {
            minDist = dist;
            ret = i;
        }
    }
    LOGD_A3DLUT("%s: ret %d, minDist %f, %s", __func__, ret, minDist, illuLinks[ret].sw_lut3dC_illu_name);
    return ret;
}

static float get_alpha_by_gain(float idx[4], float val[4], float gain) {
    float alpha = 0.0;
    interpolation_f(idx, val, 4, gain, &alpha);
    return alpha;
}

static int get_mesh_by_name(Lut3dContext_t* pLut3dCtx, char *name) {
    alut3d_lut3dCalib_t* calibdb = &pLut3dCtx->lut3d_attrib->calibdb;
    int table_len = calibdb->sw_lut3dC_tblAll_len;

    int i;
    for (i=0; i<table_len; i++) {
        alut3d_tableAll_t *pTable = &calibdb->tableAll[i];
        if (strncmp(name, pTable->sw_lut3dC_illu_name, ALUT3D_ILLUM_NAME_LEN) == 0) {
            return i;
        }
    }

    return -1;
}

static void interpolate_lut0_alpha(float alpha, lut3d_meshGain_t *in, lut3d_meshGain_t *out) {
    uint32_t f1 = (uint32_t)(alpha * 128.0f);
    uint32_t f2 = 128 - f1;
    uint32_t lut0R, lut0G, lut0B;

    // initial lut 0
    uint16_t lut_idx        = 0;
    for (uint16_t i = 0; i < LUT3D_LUT_GRID_NUM; i++) {
        for (uint16_t j = 0; j < LUT3D_LUT_GRID_NUM; j++) {
            for (uint16_t k = 0; k < LUT3D_LUT_GRID_NUM; k++) {
                lut_idx = i * LUT3D_LUT_GRID_NUM * LUT3D_LUT_GRID_NUM + j * LUT3D_LUT_GRID_NUM + k;

                lut0R = (k << 7) - (k >> 3);
                lut0G = (j << 9) - (j >> 3);
                lut0B = (i << 7) - (i >> 3);

                out->hw_lut3dC_lutR_val[lut_idx] =
                    (uint16_t)((f1 * (uint32_t)in->hw_lut3dC_lutR_val[lut_idx] + f2 * (uint32_t)lut0R)>>7);
                out->hw_lut3dC_lutG_val[lut_idx] =
                    (uint16_t)((f1 * (uint32_t)in->hw_lut3dC_lutG_val[lut_idx] + f2 * (uint32_t)lut0G)>>7);
                out->hw_lut3dC_lutB_val[lut_idx] =
                    (uint16_t)((f1 * (uint32_t)in->hw_lut3dC_lutB_val[lut_idx] + f2 * (uint32_t)lut0B)>>7);
            }
        }
    }

}

static void Damping(Lut3dContext_t* pLut3dCtx, float damp)
{
    uint32_t f1 = (uint32_t)(damp * 128.0f);
    uint32_t f2 = 128 - f1;
    uint32_t lutSum[3] = {0};

    lut3d_meshGain_t *pDamped = &pLut3dCtx->damped_matrix;
    lut3d_meshGain_t *pUndamped = &pLut3dCtx->undamped_matrix;

    /* calc. damped lut */
    for (int i = 0; i < LUT3D_LUT_WSIZE; i++) {
        pDamped->hw_lut3dC_lutR_val[i] =
            (uint16_t)((f1 * (uint32_t) pDamped->hw_lut3dC_lutR_val[i] + f2 * (uint32_t) pUndamped->hw_lut3dC_lutR_val[i])>>7);
        pDamped->hw_lut3dC_lutG_val[i] =
            (uint16_t)((f1 * (uint32_t) pDamped->hw_lut3dC_lutG_val[i] + f2 * (uint32_t) pUndamped->hw_lut3dC_lutG_val[i])>>7);
        pDamped->hw_lut3dC_lutB_val[i] =
            (uint16_t)((f1 * (uint32_t) pDamped->hw_lut3dC_lutB_val[i] + f2 * (uint32_t) pUndamped->hw_lut3dC_lutB_val[i])>>7);

        lutSum[0] += pDamped->hw_lut3dC_lutR_val[i];
        lutSum[1] += pDamped->hw_lut3dC_lutG_val[i];
        lutSum[2] += pDamped->hw_lut3dC_lutB_val[i];
    }

    pLut3dCtx->damp_converged = !((lutSum[0] - pLut3dCtx->pre_lutSum[0]) > 0 ||
                                  (lutSum[1] - pLut3dCtx->pre_lutSum[1]) > 0 ||
                                  (lutSum[2] - pLut3dCtx->pre_lutSum[2]) > 0);

    LOGD_A3DLUT("3DLUT: Damping %f, lutsum [%u %u %u], pre: [%u %u %u] converged %d", damp,
            lutSum[0], lutSum[1], lutSum[2],
            pLut3dCtx->pre_lutSum[0], pLut3dCtx->pre_lutSum[1], pLut3dCtx->pre_lutSum[2],
            pLut3dCtx->damp_converged);

    pLut3dCtx->pre_lutSum[0] = lutSum[0];
    pLut3dCtx->pre_lutSum[1] = lutSum[1];
    pLut3dCtx->pre_lutSum[2] = lutSum[2];
}

XCamReturn A3dlut_prepare(RkAiqAlgoCom* params)
{
    Lut3dContext_t* pLut3dCtx = (Lut3dContext_t *)params->ctx;
    lut3d_calib_attrib_t* pcalib =
        (lut3d_calib_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, lut3d));

    pLut3dCtx->lut3d_attrib = pcalib;

    pLut3dCtx->pre_illu_idx = INVALID_ILLU_IDX;
    pLut3dCtx->pre_lutSum[0] = 0;
    pLut3dCtx->pre_lutSum[1] = 0;
    pLut3dCtx->pre_lutSum[2] = 0;
    pLut3dCtx->is_calib_update = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn A3dlut_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, illu_estm_info_t *swinfo)
{
    Lut3dContext_t* pLut3dCtx = (Lut3dContext_t *)inparams->ctx;
    lut3d_api_attrib_t* tunning = &pLut3dCtx->lut3d_attrib->tunning;
    alut3d_lut3dCalib_t* calibdb = &pLut3dCtx->lut3d_attrib->calibdb;
    alut3d_param_dyn_t *pdyn = &tunning->stAuto.dyn;

    bool need_recal = false;
    if (inparams->u.proc.is_attrib_update || inparams->u.proc.init) {
        need_recal = true;
    }

    //(1) estimate idx
    int illu_idx = illu_estm_once(pdyn->illuLink, pdyn->sw_lut3dT_illuLink_len, swinfo->awbGain);
    if (illu_idx < 0) {
        LOGE_A3DLUT("illu_estm_once failed!");
        return XCAM_RETURN_ERROR_PARAM;
    }
    alut3d_param_illuLink_t *pIlluCase = &pdyn->illuLink[illu_idx];

    if (illu_idx != pLut3dCtx->pre_illu_idx || pLut3dCtx->is_calib_update) {
        if (illu_idx != pLut3dCtx->pre_illu_idx) {
            pLut3dCtx->pre_illu_idx  = illu_idx;
            need_recal = true;

            LOGD_A3DLUT("%s: 3DLUT illu change --> %s",
                        __func__, pdyn->illuLink[illu_idx].sw_lut3dC_illu_name);
        }

        int mesh_idx = get_mesh_by_name(pLut3dCtx, pIlluCase->sw_lut3dC_illu_name);
        if (mesh_idx  < 0) {
            LOGE_A3DLUT("get_mesh_by_name failed!");
            return XCAM_RETURN_ERROR_PARAM;
        }

        alut3d_tableAll_t *pTable = &calibdb->tableAll[mesh_idx];
        pLut3dCtx->calib_matrix = &pTable->meshGain;
    }

    //(2) interpolate alpha
    float alpha = get_alpha_by_gain(
            pIlluCase->gain2StrgCurve.sw_lut3dT_isoIdx_val, pIlluCase->gain2StrgCurve.sw_lut3dT_alpha_val, swinfo->sensorGain);

    if (fabs(alpha - pLut3dCtx->pre_alpha) > DIVMIN) {
        LOGD_A3DLUT("%s: 3DLUT alpha changed %f --> %f",
                __func__, pLut3dCtx->pre_alpha, alpha);

        pLut3dCtx->pre_alpha  = alpha;
        need_recal = true;
    }

    //(3) lut = alpha*lutfile + (1-alpha)*lut0
    if (need_recal || pLut3dCtx->is_calib_update) {
        LOGD_A3DLUT("%s: 3DLUT interpolate_lut0_alpha", __func__);
        interpolate_lut0_alpha(alpha, pLut3dCtx->calib_matrix, &pLut3dCtx->undamped_matrix);
        pLut3dCtx->is_calib_update = false;
        need_recal = true;
    }

    //(4) damp
    bool damp_en = tunning->stAuto.sta.sw_lut3dT_damp_en;
    if (damp_en) {
        if (need_recal || !pLut3dCtx->damp_converged) {
            float dampCoef = swinfo->awbIIRDampCoef;
            Damping(pLut3dCtx, dampCoef);
            need_recal = true;
        }
    }

    outparams->cfg_update = false;
    if (need_recal) {
        lut3d_param_t* lut3dRes = outparams->algoRes;
        if (damp_en)
            lut3dRes->dyn.meshGain = pLut3dCtx->damped_matrix;
        else
            lut3dRes->dyn.meshGain = pLut3dCtx->undamped_matrix;

        outparams->cfg_update = true;
        outparams->en = tunning->en;
        outparams->bypass = tunning->bypass;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;
    LOG1_A3DLUT("%s: (enter)\n", __FUNCTION__ );

    Lut3dContext_t* ctx = aiq_mallocz(sizeof(Lut3dContext_t));
    memset(ctx, 0, sizeof(Lut3dContext_t));

    *context = (RkAiqAlgoContext *)(ctx);

    LOG1_A3DLUT("%s: (exit)\n", __FUNCTION__ );
    return result;

}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOG1_A3DLUT("%s: (enter)\n", __FUNCTION__ );

    Lut3dContext_t* pLut3dCtx = (Lut3dContext_t*)context;
    aiq_free(pLut3dCtx);

    LOG1_A3DLUT("%s: (exit)\n", __FUNCTION__ );
    return result;

}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_A3DLUT("%s: (enter)\n", __FUNCTION__ );
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    A3dlut_prepare(params);

    LOG1_A3DLUT("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    LOG1_A3DLUT("%s: (enter)\n", __FUNCTION__ );

    RkAiqAlgoProcLut3d* proc_param = (RkAiqAlgoProcLut3d*)inparams;
    illu_estm_info_t *swinfo = &proc_param->illu_info;

    A3dlut_processing(inparams, outparams, swinfo);

    LOG1_A3DLUT("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_DUMPSYS
static int dump(const RkAiqAlgoCom* config, st_string* result)
{
    // lut3d_dump_mod_param(config, result);
    // lut3d_dump_mod_attr(config, result);
    lut3d_dump_mod_status(config, result);

    return 0;
}
#endif

XCamReturn
algo_lut3d_queryalut3dStatus
(
    RkAiqAlgoContext* ctx,
    alut3d_status_t* status
)
{
    if(ctx == NULL || status == NULL) {
        LOGE_A3DLUT("have no alut3d status info !");
        return XCAM_RETURN_ERROR_PARAM;
    }

    Lut3dContext_t* pLut3dCtx = (Lut3dContext_t*)ctx;
    lut3d_param_auto_t* stAuto = &pLut3dCtx->lut3d_attrib->tunning.stAuto;

    strncpy(status->sw_lut3dC_illuUsed_name, 
            stAuto->dyn.illuLink[pLut3dCtx->pre_illu_idx].sw_lut3dC_illu_name, 
            ALUT3D_ILLUM_NAME_LEN - 1);
    
    status->sw_lut3dT_alpha_val = pLut3dCtx->pre_alpha;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_lut3d_SetCalib
(
    RkAiqAlgoContext* ctx,
    alut3d_lut3dCalib_t* calib
) {
    if(ctx == NULL || calib == NULL) {
        LOGE_A3DLUT("%s: null pointer\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    Lut3dContext_t* pLut3dCtx = (Lut3dContext_t*)ctx;
    lut3d_api_attrib_t* lut3d_attrib = &pLut3dCtx->lut3d_attrib->tunning;
    alut3d_lut3dCalib_t* alut3d_calib = &pLut3dCtx->lut3d_attrib->calibdb;

    if (lut3d_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_A3DLUT("not auto mode: %d", lut3d_attrib->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    memcpy(alut3d_calib, calib, sizeof(alut3d_lut3dCalib_t));
    pLut3dCtx->is_calib_update = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_lut3d_GetCalib
(
    RkAiqAlgoContext* ctx,
    alut3d_lut3dCalib_t* calib
)
{
    if(ctx == NULL || calib == NULL) {
        LOGE_A3DLUT("%s: null pointer\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    Lut3dContext_t* pLut3dCtx = (Lut3dContext_t*)ctx;
    alut3d_lut3dCalib_t* alut3d_calib = &pLut3dCtx->lut3d_attrib->calibdb;

    memcpy(calib, alut3d_calib, sizeof(alut3d_lut3dCalib_t));

    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_ALGO_3DLUT_VERSION     "v0.0.1"
#define RKISP_ALGO_3DLUT_VENDOR      "Rockchip"
#define RKISP_ALGO_3DLUT_DESCRIPTION "Rockchip 3dlut algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescLut3d = {
    .common = {
        .version = RKISP_ALGO_3DLUT_VERSION,
        .vendor  = RKISP_ALGO_3DLUT_VENDOR,
        .description = RKISP_ALGO_3DLUT_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_A3DLUT,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
#if RKAIQ_HAVE_DUMPSYS
    .dump = dump,
#endif
};
