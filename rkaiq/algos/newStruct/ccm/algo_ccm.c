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
#include "ccm_types_prvt.h"
#include "xcam_log.h"

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
//#include "RkAiqHandle.h"

#include "interpolation.h"
#include "c_base/aiq_base.h"

#if RKAIQ_HAVE_DUMPSYS
#include "include/algo_ccm_info.h"
#include "rk_info_utils.h"
#endif

// RKAIQ_BEGIN_DECLARE

static int illu_estm_once(accm_param_illuLink_t *illuLinks, uint8_t illuLink_len, float awbGain[2]) {
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
        accm_param_illuLink_t *pIlluCase = &illuLinks[i];
        float refRG = pIlluCase->sw_ccmC_wbGainR_val;
        float refBG = pIlluCase->sw_ccmC_wbGainB_val;
        dist = sqrt((nRG - refRG) * (nRG -  refRG) + (nBG -  refBG) * (nBG -  refBG));
        if(dist < minDist) {
            minDist = dist;
            ret = i;
        }
    }
    LOGD_ACCM("%s: ret %d, minDist %f, %s", __func__, ret, minDist, illuLinks[ret].sw_ccmC_illu_name);
    return ret;
}

static float get_sat_by_gain(float idx[4], float val[4], float gain) {
    float fSaturation = 0.0;
    interpolation_f(idx, val, 4, gain, &fSaturation);
    return fSaturation;
}

static int get_all_mesh_by_name(CcmContext_t *pCcmCtx, char *name) {
    int cnt = 0;
    accm_ccmCalib_t* calibdb = &pCcmCtx->ccm_attrib->calibdb;
    uint8_t *mesh_all = pCcmCtx->illu_mesh_all;
    int table_len = calibdb->sw_ccmC_matrixAll_len;

    int i;
    for (i=0; i<table_len; i++) {
        accm_matrixAll_t *pTable = &calibdb->matrixAll[i];
        if (strncmp(name, pTable->sw_ccmC_illu_name, ACCM_ILLUM_NAME_LEN) == 0) {
            LOGI_ACCM("%s: pTable name %s, sw_ccmC_ccmSat_val %f i %d",
                    __func__, pTable->sw_ccmC_illu_name, pTable->sw_ccmC_ccmSat_val, i);
            mesh_all[cnt] = i;
            cnt ++;
        }
    }

    return cnt;
}

static int get_mesh_by_sat(CcmContext_t *pCcmCtx, float fSaturation, int mesh_out[2])
{
    int i;
    accm_ccmCalib_t* calibdb = &pCcmCtx->ccm_attrib->calibdb;
    uint8_t *mesh_all = pCcmCtx->illu_mesh_all;
    uint8_t mesh_len = pCcmCtx->illu_mesh_len;

    int upper_id = -1, lower_id = -1;

    float upper_diff = 99999;
    float lower_diff = 99999;

    for (i=0; i<mesh_len; i++) {
        uint8_t id = mesh_all[i];
        accm_matrixAll_t *pTable = &calibdb->matrixAll[id];
        float table_sat = pTable->sw_ccmC_ccmSat_val;

        float diff = table_sat - fSaturation;
        LOGV_ACCM("pTable name %s, table_sat %f, fSaturation %f, diff %f\n",
                pTable->sw_ccmC_illu_name, table_sat, fSaturation, diff);

        if (diff >= 0.0) {
            if (upper_diff > diff) {
                upper_diff = diff;
                upper_id = id;
            }
        }

        if (diff <= 0.0) {
            diff = -1.0 * diff;
            if (lower_diff > diff) {
                lower_diff = diff;
                lower_id = id;
            }
        }
    }

    if (upper_id == -1 && lower_id == -1) {
        return 0;
    }

    if (upper_id == -1) {
        mesh_out [0] = lower_id;
        mesh_out [1] = lower_id;
        return 1;
    }

    if (lower_id == -1) {
        mesh_out [0] = upper_id;
        mesh_out [1] = upper_id;
        return 1;
    }

    mesh_out [0] = lower_id;
    mesh_out [1] = upper_id;

    if (lower_id == upper_id) {
        return 1;
    } else {
        return 2;
    }

    return 0;
}

static int mesh_interpolation(CcmContext_t *pCcmCtx, float fSaturation, int mesh_out[2])
{
    accm_ccmCalib_t* calibdb = &pCcmCtx->ccm_attrib->calibdb;
    uint8_t mesh_id1 = mesh_out[0];
    uint8_t mesh_id2 = mesh_out[1];

    accm_matrixAll_t *pTable1 = &calibdb->matrixAll[mesh_id1];
    accm_matrixAll_t *pTable2 = &calibdb->matrixAll[mesh_id2];

    ccm_matrix_t *pMatrixA = &pTable1->ccMatrix;
    ccm_matrix_t *pMatrixB = &pTable2->ccMatrix;

    float fSatA = pTable1->sw_ccmC_ccmSat_val;
    float fSatB = pTable2->sw_ccmC_ccmSat_val;

    if (fSatA == fSatB) {
        memcpy(&pCcmCtx->undamped_matrix, &pTable1->ccMatrix, sizeof(ccm_matrix_t));
        return 0;
    }

    float f1 = (fSatB - fSaturation) / (fSatB - fSatA);
    float f2 = 1.0f - f1;

    LOGD_ACCM("select:%s :%f  and %s :%f", pTable1->sw_ccmC_illu_name, f1, pTable2->sw_ccmC_illu_name, f2);

    int i;
    for ( i = 0; i < 9; i++) {
        pCcmCtx->undamped_matrix.hw_ccmC_matrix_coeff[i] = f1 * pMatrixA->hw_ccmC_matrix_coeff[i] + f2 * pMatrixB->hw_ccmC_matrix_coeff[i];
    }

    for ( i = 0; i < 3; i++) {
        pCcmCtx->undamped_matrix.hw_ccmC_matrix_offset[i] = f1 * pMatrixA->hw_ccmC_matrix_offset[i] + f2 * pMatrixB->hw_ccmC_matrix_offset[i];
    }

    return 0;
}

void Saturationadjust(float fScale, float flevel1, float *pccMatrixA)
{
    float  Matrix_tmp[9];
    if (fScale < DIVMIN) {
        if(fabs((flevel1-50))>DIVMIN){
            LOGW_ACCM("fScale is  %f, so saturation adjust bypass\n", fScale);
        }
     } else {
        flevel1 = (flevel1 - 50) / 50 + 1;
        LOGV_ACCM("Satura: %f \n", flevel1);
        if (pccMatrixA == NULL) {
            LOGE_ACCM("%s: pointer pccMatrixA is NULL", __FUNCTION__);
            return;
        }
        memcpy(&Matrix_tmp, pccMatrixA, sizeof(Matrix_tmp));
        float *pccMatrixB;
        pccMatrixB = Matrix_tmp;
        /* ************************************
        *  M_A =  (M0 - E) * fscale + E
        *  M_B = rgb2ycbcr(M_A)
        *  M_B' = ycbcr2rgb[ sat_matrix * M_B ]
        *  M_A' = (M_B' - E) / fscale + E
        *  return (M_A')
        * ***********************************/
        if ( (pccMatrixA != NULL) && (pccMatrixB != NULL) )
        {
            for(int i =0; i < 9; i++)
            {
                if (i == 0 || i == 4 || i == 8){
                  pccMatrixA[i] = (pccMatrixA[i] - 1)*fScale+1;
                }
                else{
                  pccMatrixA[i] = pccMatrixA[i]*fScale;
                }
            }
            pccMatrixB[0] = 0.299 * pccMatrixA[0] + 0.587 * pccMatrixA[3] + 0.114 * pccMatrixA[6];
            pccMatrixB[1] = 0.299 * pccMatrixA[1] + 0.587 * pccMatrixA[4] + 0.114 * pccMatrixA[7];
            pccMatrixB[2] = 0.299 * pccMatrixA[2] + 0.587 * pccMatrixA[5] + 0.114 * pccMatrixA[8];
            pccMatrixB[3] = -0.1687 * pccMatrixA[0] - 0.3313 * pccMatrixA[3] + 0.5 * pccMatrixA[6];
            pccMatrixB[4] = -0.1687 * pccMatrixA[1] - 0.3313 * pccMatrixA[4] + 0.5 * pccMatrixA[7];
            pccMatrixB[5] = -0.1687 * pccMatrixA[2] - 0.3313 * pccMatrixA[5] + 0.5 * pccMatrixA[8];
            pccMatrixB[6] = 0.5 * pccMatrixA[0]  - 0.4187 * pccMatrixA[3] - 0.0813 * pccMatrixA[6];
            pccMatrixB[7] = 0.5 * pccMatrixA[1]  - 0.4187 * pccMatrixA[4] - 0.0813 * pccMatrixA[7];
            pccMatrixB[8] = 0.5 * pccMatrixA[2]  - 0.4187 * pccMatrixA[5] - 0.0813 * pccMatrixA[8];

            for(int i = 3; i < 9; i++)
            {
                 pccMatrixB[i] = flevel1 * pccMatrixB[i];
             }
            pccMatrixA[0] = 1 * pccMatrixB[0] + 0 * pccMatrixB[3] + 1.402 * pccMatrixB[6];
            pccMatrixA[1] = 1 * pccMatrixB[1] + 0 * pccMatrixB[4] + 1.402 * pccMatrixB[7];
            pccMatrixA[2] = 1 * pccMatrixB[2] + 0 * pccMatrixB[5] + 1.402 * pccMatrixB[8];
            pccMatrixA[3] = 1 * pccMatrixB[0] - 0.34414 * pccMatrixB[3]  - 0.71414 * pccMatrixB[6];
            pccMatrixA[4] = 1 * pccMatrixB[1] - 0.34414 * pccMatrixB[4]  - 0.71414 * pccMatrixB[7];
            pccMatrixA[5] = 1 * pccMatrixB[2] - 0.34414 * pccMatrixB[5]  - 0.71414 * pccMatrixB[8];
            pccMatrixA[6] = 1 * pccMatrixB[0]  + 1.772 * pccMatrixB[3] + 0 * pccMatrixB[6];
            pccMatrixA[7] = 1 * pccMatrixB[1]  + 1.772 * pccMatrixB[4] + 0 * pccMatrixB[7];
            pccMatrixA[8] = 1 * pccMatrixB[2]  + 1.772 * pccMatrixB[5] + 0 * pccMatrixB[8];


            for(int i =0; i < 9; i++)
            {
                if (i == 0 || i == 4 || i == 8){
                  pccMatrixA[i] = (pccMatrixA[i] - 1)/fScale+1;
                }
                else{
                  pccMatrixA[i] = pccMatrixA[i]/fScale;
                }
            }
        }

      }
}

static void Damping(CcmContext_t* pCcmCtx, float damp)
{
    bool flag = false;
    int32_t i;
    float f = (1.0f - damp);

    float *pMatrixUndamped = pCcmCtx->undamped_matrix.hw_ccmC_matrix_coeff;
    float *pMatrixDamped = pCcmCtx->damped_matrix.hw_ccmC_matrix_coeff;
    float *pOffsetUndamped = pCcmCtx->undamped_matrix.hw_ccmC_matrix_offset;
    float *pOffsetDamped = pCcmCtx->damped_matrix.hw_ccmC_matrix_offset;

    /* calc. damped cc matrix */
    for( i = 0; i < 9; i++ )
    {
        pMatrixDamped[i] = (damp * pMatrixDamped[i]) + (f *  pMatrixUndamped[i]);
        if (!flag) flag = (fabs(pMatrixDamped[i] - pMatrixUndamped[i]) > DIVMIN);
    }

    /* calc. damped cc offsets */
    for( i = 0; i < 3; i++ )
    {
        pOffsetDamped[i] = (damp * pOffsetDamped[i]) + (f *  pOffsetUndamped[i]);
        if (!flag) flag = (fabs(pOffsetDamped[i] - pOffsetUndamped[i]) > DIVMIN);
    }

    pCcmCtx->damp_converged = !flag;
}

static XCamReturn
CcmSelectParam(CcmContext_t *pCcmCtx, ccm_param_t* out, int iso)
{
    LOG1_ACCM("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pCcmCtx == NULL) {
        LOGE_ACCM("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (out == NULL) {
        LOGE_ACCM("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    ccm_api_attrib_t* tunning = &pCcmCtx->ccm_attrib->tunning;
    accm_param_isoLink_t* isoLink = tunning->stAuto.dyn.isoLink;

    pre_interp(iso, pCcmCtx->iso_list, 13, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;

    pCcmCtx->fScale = interpolation_f32(
        isoLink[ilow].sw_ccmT_glbCcm_scale, isoLink[ihigh].sw_ccmT_glbCcm_scale, ratio);

    out->dyn.ccmAlpha_yFac.hw_ccmT_facMax_minThred = isoLink[inear].ccmAlpha_yFac.hw_ccmT_facMax_minThred;
    out->dyn.ccmAlpha_yFac.hw_ccmT_facMax_maxThred = isoLink[inear].ccmAlpha_yFac.hw_ccmT_facMax_maxThred;
    for (i=0; i<18; i++) {
        out->dyn.ccmAlpha_yFac.hw_ccmT_loY2Alpha_fac0[i] =
            isoLink[inear].ccmAlpha_yFac.hw_ccmT_loY2Alpha_fac0[i] * pCcmCtx->fScale;
    }
    for (i=0; i<18; i++) {
        out->dyn.ccmAlpha_yFac.hw_ccmT_hiY2Alpha_fac0[i] =
            isoLink[inear].ccmAlpha_yFac.hw_ccmT_hiY2Alpha_fac0[i] * pCcmCtx->fScale;
    }

    out->dyn.ccmAlpha_satFac.hw_ccmT_satIdx_maxLimit = interpolation_u8(
        isoLink[ilow].ccmAlpha_satFac.hw_ccmT_satIdx_maxLimit, isoLink[ihigh].ccmAlpha_satFac.hw_ccmT_satIdx_maxLimit, uratio);
    out->dyn.ccmAlpha_satFac.hw_ccmT_facMax_thred = interpolation_u8(
        isoLink[ilow].ccmAlpha_satFac.hw_ccmT_facMax_thred, isoLink[ihigh].ccmAlpha_satFac.hw_ccmT_facMax_thred, uratio);
    out->dyn.ccmAlpha_satFac.hw_ccmT_satIdx_scale = interpolation_f32(
        isoLink[ilow].ccmAlpha_satFac.hw_ccmT_satIdx_scale, isoLink[ihigh].ccmAlpha_satFac.hw_ccmT_satIdx_scale, ratio);
    for (i = 0; i < 17; i++) {
        out->dyn.ccmAlpha_satFac.hw_ccmT_sat2Alpha_fac1[i] = interpolation_u16(
            isoLink[ilow].ccmAlpha_satFac.hw_ccmT_sat2Alpha_fac1[i], isoLink[ihigh].ccmAlpha_satFac.hw_ccmT_sat2Alpha_fac1[i], uratio);
    }
    out->dyn.enhance.hw_ccmT_enhance_en = interpolation_u8(
        isoLink[ilow].enhance.hw_ccmT_enhance_en, isoLink[ihigh].enhance.hw_ccmT_enhance_en, uratio);
    out->dyn.enhance.hw_ccmT_enhanceRat_maxLimit = interpolation_f32(
        isoLink[ilow].enhance.hw_ccmT_enhanceRat_maxLimit, isoLink[ihigh].enhance.hw_ccmT_enhanceRat_maxLimit, ratio);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn Accm_prepare(RkAiqAlgoCom* params)
{
    CcmContext_t* pCcmCtx = (CcmContext_t *)params->ctx;
    pCcmCtx->ccm_attrib =
        (ccm_calib_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, ccm));
    pCcmCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;

    pCcmCtx->fScale = 0.0;
    pCcmCtx->damp_converged = false;
    pCcmCtx->is_calib_update = true;
    pCcmCtx->isReCal_ = true;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn Accm_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, illu_estm_info_t *swinfo)
{

    CcmContext_t* pCcmCtx = (CcmContext_t *)inparams->ctx;
    ccm_api_attrib_t* tunning = &pCcmCtx->ccm_attrib->tunning;
    accm_ccmCalib_t* calibdb = &pCcmCtx->ccm_attrib->calibdb;
    accm_param_dyn_t *pdyn = &tunning->stAuto.dyn;

    int iso = inparams->u.proc.iso;

    bool need_recal = pCcmCtx->isReCal_;

    bool isReCal_ = inparams->u.proc.is_attrib_update || inparams->u.proc.init;

    if (pCcmCtx->ccm_selector.frame_num == 0 || inparams->u.proc.is_attrib_update) {
        for (int i = 0; i < pdyn->sw_ccmT_illuLink_len; i++) {
            selector_add_source(&pCcmCtx->ccm_selector, i,
                                pdyn->illuLink[i].sw_ccmC_illu_name,
                                pdyn->illuLink[i].sw_ccmC_wbGainR_val,
                                pdyn->illuLink[i].sw_ccmC_wbGainB_val);
        }
        pCcmCtx->ccm_selector.source_count = MIN(pdyn->sw_ccmT_illuLink_len, ILLUM_MAX_NUM);
    }

    if (isReCal_) {
        need_recal = true;
    }
    LOGD_ACCM("awbGain= (%f, %f)", swinfo->awbGain[0], swinfo->awbGain[1]);
    int illu_idx = -1;

    selector_process_frame(&pCcmCtx->ccm_selector, swinfo->awbGain, &illu_idx);

    accm_param_illuLink_t *pIlluCase = &pdyn->illuLink[illu_idx];

    if (illu_idx != pCcmCtx->pre_illu_idx || pCcmCtx->is_calib_update) {
        if (illu_idx != pCcmCtx->pre_illu_idx) {
            pCcmCtx->pre_illu_idx  = illu_idx;
            need_recal = true;
        }
        pCcmCtx->illu_mesh_len =
            get_all_mesh_by_name(pCcmCtx, pIlluCase->sw_ccmC_illu_name);
    }

    float sat = get_sat_by_gain(pIlluCase->gain2SatCurve.sw_ccmT_isoIdx_val, pIlluCase->gain2SatCurve.sw_ccmT_glbSat_val, swinfo->sensorGain);
    if (sat != pCcmCtx->pre_saturation) {
        pCcmCtx->pre_saturation  = sat;
        need_recal = true;
    }

    if (need_recal || pCcmCtx->is_calib_update) {
        pCcmCtx->is_calib_update = false;
        int mesh_out[2] = {-1, -1};
        int ret = get_mesh_by_sat(pCcmCtx, sat, mesh_out);
        if (ret == 0) {
            LOGE_ACCM("no valid mesh!!");
        } else if (ret == 1) {
            uint8_t id = mesh_out[0];
            accm_matrixAll_t *pTable = &calibdb->matrixAll[id];
            LOGD_ACCM("copy mesh id %d", id);
            memcpy(&pCcmCtx->undamped_matrix, &pTable->ccMatrix, sizeof(ccm_matrix_t));
        } else {
            mesh_interpolation(pCcmCtx, sat, mesh_out);
        }
        need_recal = true;
    }

    // TODO
    bool damp_en = tunning->stAuto.sta.sw_ccmT_damp_en;
    if (damp_en) {
        if (need_recal || !pCcmCtx->damp_converged) {
            need_recal = true;
        }
    }
    int delta_iso = abs(iso - pCcmCtx->pre_iso);
    //if(delta_iso > 1 || pCcmCtx->isReCal_) {
    if(delta_iso > 1 || need_recal) {
        CcmSelectParam(pCcmCtx, outparams->algoRes, iso);
        pCcmCtx->pre_iso = iso;
        need_recal = true;
    }

    if (fabs(pCcmCtx->pre_scale - pCcmCtx->fScale) > 0.01) {
        Saturationadjust(pCcmCtx->fScale, 50, pCcmCtx->undamped_matrix.hw_ccmC_matrix_coeff);
        pCcmCtx->pre_scale = pCcmCtx->fScale;
        need_recal = true;
    }

    // bool damp_en = tunning->stAuto.sta.sw_ccmT_damp_en;
    if (damp_en) {
        if (need_recal || !pCcmCtx->damp_converged) {
            float dampCoef = swinfo->awbIIRDampCoef;
            Damping(pCcmCtx, dampCoef);
            need_recal = true;
        }
    }

    outparams->cfg_update = false;
    if (need_recal) {
        ccm_param_t* ccmRes = outparams->algoRes;
        ccmRes->sta = tunning->stAuto.sta.ccmCfg;
        if (damp_en)
            ccmRes->dyn.ccMatrix = pCcmCtx->damped_matrix;
        else
            ccmRes->dyn.ccMatrix = pCcmCtx->undamped_matrix;

        outparams->cfg_update = true;
        outparams->en = tunning->en;
        outparams->bypass = tunning->bypass;
    } else {
        outparams->cfg_update = false;
    }
    pCcmCtx->isReCal_ = false;
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__ );

    CcmContext_t* ctx = aiq_mallocz(sizeof(CcmContext_t));
    *context = (RkAiqAlgoContext *)(ctx);
    ctx->pre_illu_idx = INVALID_ILLU_IDX;
    ctx->pre_saturation = 0.0;
    ctx->pre_scale = 0.0;
    ctx->pre_iso = 0;

    selector_init(&ctx->ccm_selector);

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__ );
    return result;

}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOG1_ACCM("%s: (enter)\n", __FUNCTION__ );

    CcmContext_t* pCcmCtx = (CcmContext_t*)context;
    aiq_free(pCcmCtx);

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__ );
    return result;

}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__ );
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    Accm_prepare(params);
    LOG1_ACCM("%s: (exit)\n", __FUNCTION__ );
    return result;
}


static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__ );


    RkAiqAlgoProcCcm* proc_param = (RkAiqAlgoProcCcm*)inparams;
    illu_estm_info_t *swinfo = &proc_param->illu_info;

    Accm_processing(inparams, outparams, swinfo);

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_DUMPSYS
static int dump(const RkAiqAlgoCom* config, st_string* result)
{
    // ccm_dump_mod_param(config, result);
    // ccm_dump_mod_attr(config, result);
    ccm_dump_mod_status(config, result);

    return 0;
}
#endif

XCamReturn
algo_ccm_queryaccmStatus
(
    RkAiqAlgoContext* ctx,
    accm_status_t* status
)
{
    if(ctx == NULL || status == NULL) {
        LOGE_ACCM("have no accm status info !");
        return XCAM_RETURN_ERROR_PARAM;
    }

    CcmContext_t* pCcmCtx = (CcmContext_t*)ctx;
    ccm_param_auto_t* stAuto = &pCcmCtx->ccm_attrib->tunning.stAuto;

    strncpy(status->sw_ccmC_illuUsed_name,
            stAuto->dyn.illuLink[pCcmCtx->pre_illu_idx].sw_ccmC_illu_name,
            ACCM_ILLUM_NAME_LEN - 1);

    status->sw_ccmC_ccmSat_val = pCcmCtx->pre_saturation;
    status->sw_ccmT_glbCcm_scale = pCcmCtx->pre_scale;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_ccm_SetCalib
(
    RkAiqAlgoContext* ctx,
    accm_ccmCalib_t* calib
) {
    if(ctx == NULL || calib == NULL) {
        LOGE_ACCM("%s: null pointer\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CcmContext_t* pCcmCtx = (CcmContext_t*)ctx;
    ccm_api_attrib_t* ccm_attrib = &pCcmCtx->ccm_attrib->tunning;
    accm_ccmCalib_t* accm_calib = &pCcmCtx->ccm_attrib->calibdb;

    if (ccm_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ACCM("not auto mode: %d", ccm_attrib->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    memcpy(accm_calib, calib, sizeof(accm_ccmCalib_t));
    pCcmCtx->is_calib_update = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_ccm_GetCalib
(
    RkAiqAlgoContext* ctx,
    accm_ccmCalib_t* calib
)
{
    if(ctx == NULL || calib == NULL) {
        LOGE_ACCM("%s: null pointer\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CcmContext_t* pCcmCtx = (CcmContext_t*)ctx;
    accm_ccmCalib_t* accm_calib = &pCcmCtx->ccm_attrib->calibdb;

    memcpy(calib, accm_calib, sizeof(accm_ccmCalib_t));

    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_ALGO_CCM_VERSION     "v0.0.1"
#define RKISP_ALGO_CCM_VENDOR      "Rockchip"
#define RKISP_ALGO_CCM_DESCRIPTION "Rockchip ccm algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCcm = {
    .common = {
        .version = RKISP_ALGO_CCM_VERSION,
        .vendor  = RKISP_ALGO_CCM_VENDOR,
        .description = RKISP_ALGO_CCM_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ACCM,
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

// RKAIQ_END_DECLARE
