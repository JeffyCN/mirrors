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
#include "lsc_types_prvt.h"
#include "xcam_log.h"

#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

#include "interpolation.h"
#include "c_base/aiq_base.h"

#if RKAIQ_HAVE_DUMPSYS
#include "include/algo_lsc_info.h"
#include "rk_info_utils.h"
#endif

//RKAIQ_BEGIN_DECLARE


static int illu_estm_once(alsc_param_dyn_t *pdyn, float awbGain[2]) {
    int ret = -1;
    uint8_t case_id = 0;
    alsc_param_illuLink_t *illuLinks = pdyn->illuLink;
    uint8_t illuLink_len = pdyn->sw_lscT_illuLink_len;

    if (illuLink_len == 0)
        return -1;

    float minDist = 9999999;
    float dist;
    float nRG = awbGain[0]; //current R/G gain
    float nBG = awbGain[1]; //current B/G gain

    int i;
    for (i=0; i<illuLink_len; i++) {
        alsc_param_illuLink_t *pIlluCase = &illuLinks[i];
        float refRG = pIlluCase->sw_lscC_wbGainR_val;
        float refBG = pIlluCase->sw_lscC_wbGainB_val;
        dist = sqrt((nRG - refRG) * (nRG -  refRG) + (nBG -  refBG) * (nBG -  refBG));
        if(dist < minDist) {
            minDist = dist;
            ret = i;
        }
    }
    return ret;
}

float get_vign_by_gain(float idx[4], float val[4], float gain) {
    float fVignetting = 0.0;
    interpolation_f(idx, val, 4, gain, &fVignetting);
    return fVignetting;
}

static int get_all_mesh_by_name(LscContext_t *pLscCtx, char *name) {
    int cnt = 0;
    alsc_lscCalib_t* calibdb = &pLscCtx->lsc_attrib->calibdb;
    uint8_t *mesh_all = pLscCtx->illu_mesh_all;
    int table_len = pLscCtx->lsc_tableAll_use_len;

    int i;
    for (i=0; i<table_len; i++) {
        alsc_tableAll_t *pTable = &calibdb->tableAll[i];
        if (strncmp(name, pTable->sw_lscC_illu_name, ALSC_ILLUM_NAME_LEN) == 0) {
            LOGI_ALSC("%s: pTable name %s, sw_lscC_vignetting_val %f i %d\n",
                    __func__, pTable->sw_lscC_illu_name, pTable->sw_lscC_vignetting_val, i);
            mesh_all[cnt] = i;
            cnt ++;
        }
    }

    return cnt;
}

static int get_mesh_by_vign(LscContext_t *pLscCtx, float fVignetting, int mesh_out[2])
{
    int i;
    alsc_lscCalib_t* calibdb = &pLscCtx->lsc_attrib->calibdb;
    uint8_t *mesh_all = pLscCtx->illu_mesh_all;
    uint8_t mesh_len = pLscCtx->illu_mesh_len;

    int upper_id = -1, lower_id = -1;

    float upper_diff = 99999;
    float lower_diff = 99999;

    for (i=0; i<mesh_len; i++) {
        uint8_t id = mesh_all[i];
        alsc_tableAll_t *pTable = &calibdb->tableAll[id];
        float table_vig = pTable->sw_lscC_vignetting_val;

        float diff = table_vig - fVignetting;
        LOGV_ALSC("pTable name %s, table_vig %f, fVignetting %f, diff %f\n",
                pTable->sw_lscC_illu_name, table_vig, fVignetting, diff);

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

int mesh_interpolation(LscContext_t *pLscCtx, float fVignetting, int mesh_out[2])
{
    alsc_lscCalib_t* calibdb = &pLscCtx->lsc_attrib->calibdb;
    uint8_t mesh_id1 = mesh_out[0];
    uint8_t mesh_id2 = mesh_out[1];

    alsc_tableAll_t *pTable1 = &calibdb->tableAll[mesh_id1];
    alsc_tableAll_t *pTable2 = &calibdb->tableAll[mesh_id2];

    float fVigA = pTable1->sw_lscC_vignetting_val;
    float fVigB = pTable2->sw_lscC_vignetting_val;

    if (fVigA == fVigB) {
        memcpy(&pLscCtx->undamped_matrix, &pTable1->meshGain, sizeof(lsc_meshGain_t));
        return 0;
    }

    float f1 = (fVigB - fVignetting) / (fVigB - fVigA);
    float f2 = 1.0f - f1;

    LOGI_ALSC("select:%s :%f  and %s :%f", pTable1->sw_lscC_illu_name, f1, pTable2->sw_lscC_illu_name, f2);

    /* left shift 16 */
    uint32_t f1_ = (uint32_t)(f1 * 65536.0f);
    uint32_t f2_ = (uint32_t)(f2 * 65536.0f);

    int16_t i;

    uint32_t red;
    uint32_t greenr;
    uint32_t greenb;
    uint32_t blue;

    for (i = 0; i < (LSC_LSCTABLE_SIZE); i++) {
        red     = (f1_ * (uint32_t)pTable1->meshGain.hw_lscC_gainR_val[i])
                  + (f2_ * (uint32_t)pTable2->meshGain.hw_lscC_gainR_val[i]);

        greenr  = (f1_ * (uint32_t)pTable1->meshGain.hw_lscC_gainGr_val[i])
                  + (f2_ * (uint32_t)pTable2->meshGain.hw_lscC_gainGr_val[i]);

        greenb  = (f1_ * (uint32_t)pTable1->meshGain.hw_lscC_gainGb_val[i])
                  + (f2_ * (uint32_t)pTable2->meshGain.hw_lscC_gainGb_val[i]);

        blue    = (f1_ * (uint32_t)pTable1->meshGain.hw_lscC_gainB_val[i])
                  + (f2_ * (uint32_t)pTable2->meshGain.hw_lscC_gainB_val[i]);

        /* with round up (add 65536/2 <=> 0.5) before right shift */
        pLscCtx->undamped_matrix.hw_lscC_gainR_val[i]    = (uint16_t)((red + (65536 >> 1)) >> 16);
        pLscCtx->undamped_matrix.hw_lscC_gainGr_val[i] = (uint16_t)((greenr + (65536 >> 1)) >> 16);
        pLscCtx->undamped_matrix.hw_lscC_gainGb_val[i] = (uint16_t)((greenb + (65536 >> 1)) >> 16);
        pLscCtx->undamped_matrix.hw_lscC_gainB_val[i]   = (uint16_t)((blue + (65536 >> 1)) >> 16);
    }

    return 0;
}

static void Damping(LscContext_t* pLscCtx, float damp)
{
    lsc_meshGain_t *pMatrixUndamped = &pLscCtx->undamped_matrix;
    lsc_meshGain_t *pMatrixDamped = &pLscCtx->damped_matrix;

    /* left shift 16 */
    uint32_t f1_ = (uint32_t)(damp * 65536.0f);
    uint32_t f2_ = (uint32_t)(65536U - f1_);

    int16_t i;

    uint32_t red;
    uint32_t greenr;
    uint32_t greenb;
    uint32_t blue;
    float dis=0;
    float dis_th=0.015;
    bool speedDamp = true;
    dis = (float)(pMatrixDamped->hw_lscC_gainR_val[0]-
        pMatrixUndamped->hw_lscC_gainR_val[0])/
        (pMatrixDamped->hw_lscC_gainR_val[0]+
        pMatrixUndamped->hw_lscC_gainR_val[0]);
    speedDamp = fabs(dis)<dis_th;
    dis = (float)(pMatrixDamped->hw_lscC_gainB_val[LSC_DATA_TBL_SIZE-1]-
        pMatrixUndamped->hw_lscC_gainB_val[LSC_DATA_TBL_SIZE-1])/
        (pMatrixDamped->hw_lscC_gainB_val[LSC_DATA_TBL_SIZE-1]+
        pMatrixUndamped->hw_lscC_gainB_val[LSC_DATA_TBL_SIZE-1]);
    speedDamp &= fabs(dis)<dis_th;
    if(speedDamp){
        damp = 0;
        f1_ = 0;
        f2_ = 65536;
    }

    for (i = 0; i < (LSC_DATA_TBL_SIZE); i++) {
        red     = (f1_ * (uint32_t)pMatrixDamped->hw_lscC_gainR_val[i])
                + (f2_ * (uint32_t)pMatrixUndamped->hw_lscC_gainR_val[i]);

        greenr  = (f1_ * (uint32_t)pMatrixDamped->hw_lscC_gainGr_val[i])
                + (f2_ * (uint32_t)pMatrixUndamped->hw_lscC_gainGr_val[i]);

        greenb  = (f1_ * (uint32_t)pMatrixDamped->hw_lscC_gainGb_val[i])
                + (f2_ * (uint32_t)pMatrixUndamped->hw_lscC_gainGb_val[i]);

        blue    = (f1_ * (uint32_t)pMatrixDamped->hw_lscC_gainB_val[i])
                + (f2_ * (uint32_t)pMatrixUndamped->hw_lscC_gainB_val[i]);

        /* with round up (add 65536/2 <=> 0.5) before right shift */
        pMatrixDamped->hw_lscC_gainR_val[i]  = (uint16_t)((red    + (65536 >> 1)) >> 16);
        pMatrixDamped->hw_lscC_gainGr_val[i] = (uint16_t)((greenr + (65536 >> 1)) >> 16);
        pMatrixDamped->hw_lscC_gainGb_val[i] = (uint16_t)((greenb + (65536 >> 1)) >> 16);
        pMatrixDamped->hw_lscC_gainB_val[i]  = (uint16_t)((blue   + (65536 >> 1)) >> 16);
    }

    pLscCtx->damp_converged = speedDamp;
}

XCamReturn Alsc_prepare(RkAiqAlgoCom* params)
{
    LscContext_t* pLscCtx = (LscContext_t*)params->ctx;

    /*
    sprintf(pLscCtx->cur_res.name, "%dx%d", para->com.u.prepare.sns_op_width,
            para->com.u.prepare.sns_op_height );
    pLscCtx->cur_res.width = para->com.u.prepare.sns_op_width;
    pLscCtx->cur_res.height = para->com.u.prepare.sns_op_height;

    pLscCtx->lscSwInfo.prepare_type = params->u.prepare.conf_type;
    */

    pLscCtx->lsc_attrib =
        (lsc_calib_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, lsc));

    pLscCtx->pre_illu_idx = INVALID_ILLU_IDX;
    pLscCtx->pre_vignetting = 0.0;
    pLscCtx->isReCal_ = true;
    pLscCtx->is_calib_update = true;
    pLscCtx->lsc_tableAll_use_len = pLscCtx->lsc_attrib->calibdb.tableAll_len;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn Alsc_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, illu_estm_info_t *swinfo)
{
    LscContext_t* pLscCtx = (LscContext_t *)inparams->ctx;
    lsc_api_attrib_t* tunning = &pLscCtx->lsc_attrib->tunning;
    alsc_lscCalib_t* calibdb = &pLscCtx->lsc_attrib->calibdb;

    bool need_recal = pLscCtx->isReCal_;

    if (inparams->u.proc.is_attrib_update || inparams->u.proc.init) {
        need_recal = true;
    }
    LOGD_ALSC("awbGain= (%f, %f)", swinfo->awbGain[0], swinfo->awbGain[1]);
    int illu_idx = illu_estm_once(&tunning->stAuto.dyn, swinfo->awbGain);
    if (illu_idx < 0) {
        LOGE_ALSC("illu_estm_once failed!");
        return XCAM_RETURN_ERROR_PARAM;
    }

    alsc_param_illuLink_t *pIlluCase = &tunning->stAuto.dyn.illuLink[illu_idx];

    if (illu_idx != pLscCtx->pre_illu_idx || pLscCtx->is_calib_update) {
        if (illu_idx != pLscCtx->pre_illu_idx) {
            pLscCtx->pre_illu_idx  = illu_idx;
            need_recal = true;
        }

        pLscCtx->illu_mesh_len =
            get_all_mesh_by_name(pLscCtx, pIlluCase->sw_lscC_illu_name);
    }

    float vig = get_vign_by_gain(pIlluCase->gain2VigCurve.sw_lscT_isoIdx_val, pIlluCase->gain2VigCurve.sw_lscT_vignetting_val, swinfo->sensorGain);
    if (vig != pLscCtx->pre_vignetting) {
        pLscCtx->pre_vignetting  = vig;
        need_recal = true;
    }

    if (need_recal || pLscCtx->is_calib_update) {
        pLscCtx->is_calib_update = false;
        int mesh_out[2] = {-1, -1};
        int ret = get_mesh_by_vign(pLscCtx, vig, mesh_out);
        if (ret == 0) {
            LOGE_ALSC("no valid mesh!!");
        } else if (ret == 1) {
            uint8_t id = mesh_out[0];
            alsc_tableAll_t *pTable = &calibdb->tableAll[id];
            LOGV_ALSC("copy mesh id %d\n", id);
            memcpy(&pLscCtx->undamped_matrix, &pTable->meshGain, sizeof(lsc_meshGain_t));
        } else {
            mesh_interpolation(pLscCtx, vig, mesh_out);
        }
        need_recal = true;
    }

    bool damp_en = tunning->stAuto.sta.sw_lscT_damp_en;
    if (damp_en) {
        if (need_recal || !pLscCtx->damp_converged) {
            float dampCoef = swinfo->awbIIRDampCoef;
            Damping(pLscCtx, dampCoef);
            need_recal = true;
        }
    }
    outparams->cfg_update = false;

    if (need_recal) {
        lsc_param_t* lscRes = outparams->algoRes;
        lscRes->sta = tunning ->stAuto.sta.lscCfg;
        if (damp_en)
            lscRes->dyn.meshGain = pLscCtx->damped_matrix;
        else
            lscRes->dyn.meshGain  = pLscCtx->undamped_matrix;

        outparams->cfg_update = true;
        outparams->en = tunning ->en;
        outparams->bypass = tunning ->bypass;
        pLscCtx->isReCal_ = false;
        LOGD_ALSC("lsc en:%d, bypass:%d", outparams->en, outparams->bypass);
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LscContext_t *ctx = aiq_mallocz(sizeof(LscContext_t));
    *context = (RkAiqAlgoContext*)ctx;

    LOGV_ALSC("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext* context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LscContext_t* pLscCtx = (LscContext_t*)context;
    aiq_free(pLscCtx);
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    Alsc_prepare(params);
    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    LOGV_ALSC("%s: Lsc (enter)\n", __FUNCTION__ );

    RkAiqAlgoProcLsc* proc_param = (RkAiqAlgoProcLsc*)inparams;
    illu_estm_info_t *swinfo = &proc_param->illu_info;


    // if (inparams->u.proc.is_bw_sensor) {
    //     lsc_attrib->en = false;
    //     outparams->cfg_update = init ? true : false;
    //     return XCAM_RETURN_NO_ERROR;
    // }

    Alsc_processing(inparams, outparams, swinfo);


    LOGV_ALSC("%s: Lsc (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_DUMPSYS
static int dump(const RkAiqAlgoCom* config, st_string* result)
{
    // lsc_dump_mod_param(config, result);
    lsc_dump_mod_attr(config, result);
    lsc_dump_mod_status(config, result);

    return 0;
}
#endif

XCamReturn algo_lsc_queryalscStatus
(
    RkAiqAlgoContext* ctx,
    alsc_status_t* status
)
{
    if(ctx == NULL || status == NULL) {
        LOGE_ALSC("have no alsc status info !");
        return XCAM_RETURN_ERROR_PARAM;
    }

    LscContext_t* pLscCtx = (LscContext_t*)ctx;
    lsc_param_auto_t* stAuto = &pLscCtx->lsc_attrib->tunning.stAuto;

    strncpy(status->sw_lscC_illuUsed_name,
            stAuto->dyn.illuLink[pLscCtx->pre_illu_idx].sw_lscC_illu_name,
            ALSC_ILLUM_NAME_LEN - 1);

    status->sw_lscC_vignetting_val = pLscCtx->pre_vignetting;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_lsc_SetCalib(RkAiqAlgoContext* ctx, alsc_lscCalib_t *calib) {
    if(ctx == NULL || calib == NULL) {
        LOGE_ALSC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    LscContext_t* pLscCtx = (LscContext_t*)ctx;
    lsc_api_attrib_t* lsc_attrib = &pLscCtx->lsc_attrib->tunning;
    alsc_lscCalib_t* alsc_calib = &pLscCtx->lsc_attrib->calibdb;

    if (lsc_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ALSC("not auto mode: %d", lsc_attrib->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (calib->tableAll_len <= alsc_calib->tableAll_len) {
        pLscCtx->lsc_tableAll_use_len = calib->tableAll_len;
    } else {
        pLscCtx->lsc_tableAll_use_len = alsc_calib->tableAll_len;
        LOGE_ALSC("lsc calib tableAll max len is %d, calib len %d more than max, maybe cause heap corruption",
            alsc_calib->tableAll_len, calib->tableAll_len);
    }
    pLscCtx->is_calib_update = true;

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
algo_lsc_GetCalib(RkAiqAlgoContext* ctx, alsc_lscCalib_t* calib)
{
    if(ctx == NULL || calib == NULL) {
        LOGE_ALSC("%s: null pointer\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    LscContext_t* pLscCtx = (LscContext_t*)ctx;
    alsc_lscCalib_t* alsc_calib = &pLscCtx->lsc_attrib->calibdb;

    memcpy(calib, alsc_calib, sizeof(alsc_lscCalib_t));
    calib->tableAll_len = pLscCtx->lsc_tableAll_use_len;

    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_ALGO_LSC_VERSION     "v0.0.1"
#define RKISP_ALGO_LSC_VENDOR      "Rockchip"
#define RKISP_ALGO_LSC_DESCRIPTION "Rockchip lsc algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescLsc = {
    .common = {
        .version = RKISP_ALGO_LSC_VERSION,
        .vendor  = RKISP_ALGO_LSC_VENDOR,
        .description = RKISP_ALGO_LSC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ALSC,
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

//RKAIQ_END_DECLARE
