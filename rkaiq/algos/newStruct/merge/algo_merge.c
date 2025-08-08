/*
 * rk_aiq_algo_debayer_itf.c
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

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "algo_types_priv.h"
#include "merge_types_prvt.h"
#include "xcam_log.h"
 //#include "RkAiqHandle.h"
#include "c_base/aiq_base.h"
#include "interpolation.h"
#include "common/rk_aiq_types_priv_c.h"
// RKAIQ_BEGIN_DECLARE

XCamReturn MergeSelectParam(MergeContext_t* pMergeCtx, mge_param_t* out, int iso);
void MergeDamping(MergeContext_t* pMergeCtx, mge_param_t* out);

static XCamReturn create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg) {
    XCamReturn result                 = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t* pCalibDbV2 = cfg->calibv2;

    MergeContext_t* ctx = aiq_mallocz(sizeof(MergeContext_t));
    if (ctx == NULL) {
        LOGE_AMERGE("%s: create Merge context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }

    ctx->isReCal_       = true;
    ctx->prepare_params = NULL;
    ctx->merge_attrib   = (mge_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, mge));

    *context = (RkAiqAlgoContext*)ctx;
    LOGV_AMERGE("%s: (exit)\n", __FUNCTION__);
    return result;
}

static XCamReturn destroy_context(RkAiqAlgoContext* context) {
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    MergeContext_t* pMergeCtx = (MergeContext_t*)context;
    aiq_free(pMergeCtx);
    return result;
}

static XCamReturn prepare(RkAiqAlgoCom* params) {
    XCamReturn result         = XCAM_RETURN_NO_ERROR;
    MergeContext_t* pMergeCtx = (MergeContext_t*)params->ctx;

    if (params->u.prepare.working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        pMergeCtx->FrameNumber = LINEAR_NUM;
    else if (params->u.prepare.working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             params->u.prepare.working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        pMergeCtx->FrameNumber = HDR_2X_NUM;
    else {
        pMergeCtx->FrameNumber = HDR_3X_NUM;
    }
    if (pMergeCtx->FrameNumber == HDR_2X_NUM || pMergeCtx->FrameNumber == HDR_3X_NUM) {
        if (!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
            // just update calib ptr
            if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
                pMergeCtx->merge_attrib =
                    (mge_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, mge));
                return XCAM_RETURN_NO_ERROR;
            }
        } else if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGERES) {
            pMergeCtx->isCapture = true;
        }
    }
    pMergeCtx->merge_attrib =
        (mge_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, mge));
    pMergeCtx->prepare_params = &params->u.prepare;
    pMergeCtx->isReCal_       = true;

    return result;
}

static XCamReturn processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    MergeContext_t* pMergeCtx            = (MergeContext_t*)inparams->ctx;
    pMergeCtx->FrameID                   = inparams->frame_id;
    mge_api_attrib_t* merge_attrib       = pMergeCtx->merge_attrib;
    mge_param_t* pMergeProcResParams     = outparams->algoRes;
    RkAiqAlgoProcMerge* merge_proc_param = (RkAiqAlgoProcMerge*)inparams;

    LOG1_AMERGE("%s: (enter)\n", __FUNCTION__);
    if (pMergeCtx->FrameNumber == HDR_2X_NUM || pMergeCtx->FrameNumber == HDR_3X_NUM) {
        if (!merge_attrib) {
            LOGE_AMERGE("merge_attrib is NULL !");
            return XCAM_RETURN_ERROR_MEM;
        }

        if (merge_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
            LOGI_AMERGE("mode is %d, not auto mode, ignore", merge_attrib->opMode);
            return XCAM_RETURN_NO_ERROR;
        }

        if (inparams->u.proc.is_attrib_update) {
            pMergeCtx->isReCal_ = true;
        }

        if (pMergeCtx->isCapture) {
            LOGE_AMERGE("%s: It's capturing, using pre frame params\n", __func__);
            pMergeCtx->isCapture = false;
            return XCAM_RETURN_NO_ERROR;
        }
        AlgoRstShared_t* xCamAePreRes = merge_proc_param->com.u.proc.res_comb->ae_pre_res_c;
        RkAiqAlgoPreResAe* pAEPreRes  = NULL;
        if (xCamAePreRes) {
            pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->_data;
            pMergeCtx->NextCtrlData.ExpoData.EnvLv =
                    pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
            // Normalize the current envLv for AEC
            pMergeCtx->NextCtrlData.ExpoData.EnvLv =
                (pMergeCtx->NextCtrlData.ExpoData.EnvLv - MIN_ENV_LV) /
                (MAX_ENV_LV - MIN_ENV_LV);
            pMergeCtx->NextCtrlData.ExpoData.EnvLv =
                LIMIT_VALUE(pMergeCtx->NextCtrlData.ExpoData.EnvLv, ENVLVMAX, ENVLVMIN);
        }
        else {
            pMergeCtx->NextCtrlData.ExpoData.EnvLv = ENVLVMIN;
            LOGW_AMERGE("%s: ae Pre result is null!!!\n", __FUNCTION__);
        }

        // get LongFrmMode
        if (merge_proc_param->LongFrmMode != pMergeCtx->NextCtrlData.ExpoData.LongFrmMode) {
            pMergeCtx->isReCal_ = true;
        }
        pMergeCtx->NextCtrlData.ExpoData.LongFrmMode = merge_proc_param->LongFrmMode;
        // get motion coef
        pMergeCtx->NextCtrlData.MoveCoef = 0;
        // expo para process
        pMergeCtx->NextCtrlData.ExpoData.SGain =
            merge_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain *
            merge_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.digital_gain *
            merge_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.isp_dgain;
        pMergeCtx->NextCtrlData.ExpoData.MGain =
            merge_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
            merge_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain *
            merge_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.isp_dgain;

        pMergeCtx->NextCtrlData.ExpoData.SExpo =
            pMergeCtx->NextCtrlData.ExpoData.SGain *
            merge_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time;
        pMergeCtx->NextCtrlData.ExpoData.MExpo =
            pMergeCtx->NextCtrlData.ExpoData.MGain *
            merge_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time;
        if (pMergeCtx->NextCtrlData.ExpoData.SExpo < FLT_EPSILON) {
            pMergeCtx->NextCtrlData.ExpoData.SGain =
                merge_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.analog_gain *
                merge_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.digital_gain *
                merge_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.isp_dgain;
            pMergeCtx->NextCtrlData.ExpoData.MGain =
                merge_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                merge_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                merge_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain;

            pMergeCtx->NextCtrlData.ExpoData.SExpo =
                pMergeCtx->NextCtrlData.ExpoData.SGain *
                merge_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.integration_time;
            pMergeCtx->NextCtrlData.ExpoData.MExpo =
                pMergeCtx->NextCtrlData.ExpoData.MGain *
                merge_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.integration_time;
        }
        if (pMergeCtx->FrameNumber == HDR_2X_NUM) {
            pMergeCtx->NextCtrlData.ExpoData.LExpo = pMergeCtx->NextCtrlData.ExpoData.MExpo;
        } else if (pMergeCtx->FrameNumber == HDR_3X_NUM) {
            pMergeCtx->NextCtrlData.ExpoData.LExpo =
                merge_proc_param->com.u.proc.nxtExp->HdrExp[2].exp_real_params.analog_gain *
                merge_proc_param->com.u.proc.nxtExp->HdrExp[2].exp_real_params.digital_gain *
                merge_proc_param->com.u.proc.nxtExp->HdrExp[2].exp_real_params.isp_dgain *
                merge_proc_param->com.u.proc.nxtExp->HdrExp[2].exp_real_params.integration_time;
            if (pMergeCtx->NextCtrlData.ExpoData.SExpo < FLT_EPSILON) {
                pMergeCtx->NextCtrlData.ExpoData.LExpo =
                    merge_proc_param->com.u.proc.curExp->HdrExp[2].exp_real_params.analog_gain *
                    merge_proc_param->com.u.proc.curExp->HdrExp[2].exp_real_params.digital_gain *
                    merge_proc_param->com.u.proc.curExp->HdrExp[2].exp_real_params.isp_dgain *
                    merge_proc_param->com.u.proc.curExp->HdrExp[2].exp_real_params.integration_time;
            }
        }

        if (pMergeCtx->NextCtrlData.ExpoData.SExpo > FLT_EPSILON)
            pMergeCtx->NextCtrlData.ExpoData.RatioLS =
                pMergeCtx->NextCtrlData.ExpoData.LExpo / pMergeCtx->NextCtrlData.ExpoData.SExpo;
        else
            LOGE_AMERGE("%s(%d): Short frame for merge expo sync is ERROR!!!\n", __FUNCTION__,
                        __LINE__);
        if (pMergeCtx->NextCtrlData.ExpoData.MExpo > FLT_EPSILON)
            pMergeCtx->NextCtrlData.ExpoData.RatioLM =
                pMergeCtx->NextCtrlData.ExpoData.LExpo / pMergeCtx->NextCtrlData.ExpoData.MExpo;
        else
            LOGE_AMERGE("%s(%d): Middle frame for merge expo sync is ERROR!!!\n", __FUNCTION__,
                        __LINE__);
        // clip for long frame mode
        if (pMergeCtx->NextCtrlData.ExpoData.LongFrmMode) {
            pMergeCtx->NextCtrlData.ExpoData.RatioLS = LONG_FRAME_MODE_RATIO;
            pMergeCtx->NextCtrlData.ExpoData.RatioLM = LONG_FRAME_MODE_RATIO;
        }

        int iso               = inparams->u.proc.iso;
        bool init             = inparams->u.proc.init;
        int delta_iso         = abs(iso - pMergeCtx->iso);
        outparams->cfg_update = false;

#if 0
        if (inparams->u.proc.is_bw_sensor) {
            merge_attrib->en      = false;
            outparams->cfg_update = init ? true : false;
            return XCAM_RETURN_NO_ERROR;
        }
#endif
        if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO) {
            pMergeCtx->isReCal_ = true;
        }

        // get bypass_expo_process
        bool bypass_expo_process = true;
        if (pMergeCtx->NextCtrlData.ExpoData.RatioLS >= RATIO_DEFAULT &&
            pMergeCtx->NextCtrlData.ExpoData.RatioLM >= RATIO_DEFAULT) {
            if (pMergeCtx->FrameID <= INIT_CALC_PARAMS_NUM)
                bypass_expo_process = false;
            else if (!pMergeCtx->CurrData.CtrlData.ExpoData.LongFrmMode !=
                     !pMergeCtx->NextCtrlData.ExpoData.LongFrmMode)
                bypass_expo_process = false;
            else if ((pMergeCtx->CurrData.CtrlData.ExpoData.RatioLS -
                      pMergeCtx->NextCtrlData.ExpoData.RatioLS) > FLT_EPSILON ||
                     (pMergeCtx->CurrData.CtrlData.ExpoData.RatioLS -
                      pMergeCtx->NextCtrlData.ExpoData.RatioLS) < -FLT_EPSILON ||
                     (pMergeCtx->CurrData.CtrlData.ExpoData.RatioLM -
                      pMergeCtx->NextCtrlData.ExpoData.RatioLM) > FLT_EPSILON ||
                     (pMergeCtx->CurrData.CtrlData.ExpoData.RatioLM -
                      pMergeCtx->NextCtrlData.ExpoData.RatioLM) < -FLT_EPSILON ||
                     (pMergeCtx->CurrData.CtrlData.ExpoData.SGain -
                      pMergeCtx->NextCtrlData.ExpoData.SGain) > FLT_EPSILON ||
                     (pMergeCtx->CurrData.CtrlData.ExpoData.SGain -
                      pMergeCtx->NextCtrlData.ExpoData.SGain) < -FLT_EPSILON ||
                     (pMergeCtx->CurrData.CtrlData.ExpoData.MGain -
                      pMergeCtx->NextCtrlData.ExpoData.MGain) > FLT_EPSILON ||
                     (pMergeCtx->CurrData.CtrlData.ExpoData.MGain -
                      pMergeCtx->NextCtrlData.ExpoData.MGain) < -FLT_EPSILON)
                bypass_expo_process = false;
            else
                bypass_expo_process = true;
        } else {
            LOGE_AMERGE("%s(%d): AE RatioLS:%f RatioLM:%f for drc expo sync is under one!!!\n",
                        __FUNCTION__, __LINE__, pMergeCtx->NextCtrlData.ExpoData.RatioLS,
                        pMergeCtx->NextCtrlData.ExpoData.RatioLM);
            bypass_expo_process = true;
        }

        if (pMergeCtx->isReCal_ || !bypass_expo_process) {
            MergeSelectParam(pMergeCtx, pMergeProcResParams, iso);
            outparams->cfg_update = true;
            outparams->en         = merge_attrib->en;
            outparams->bypass     = merge_attrib->bypass;
            LOGI_AMERGE("merge en:%d, bypass:%d", outparams->en, outparams->bypass);
        }

        pMergeCtx->iso      = iso;
        pMergeCtx->isReCal_ = false;
    } else {
        // TODO: disable merge
        if (inparams->u.proc.init)
            outparams->cfg_update = true;
        else
            outparams->cfg_update = false;
        LOGD_AMERGE("%s FrameID:%d, It's in Linear Mode, Merge function bypass_tuning_process\n",
                    __FUNCTION__, pMergeCtx->FrameID);
    }
    LOG1_AMERGE("%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

const float default_envlv_list[] = {0,   0.005, 0.01, 0.05, 0.1, 0.15, 0.2,
                                       0.3, 0.4,   0.5,  0.6,  0.8, 1};
void pre_interp_envlv(float envlv, float* envlv_list, int num, int* lo, int* hi,
                         float* ratio) {
    int i;
    float envlv_lo, envlv_hi;
    if (envlv_list == NULL) {
        envlv_list = (float*)default_envlv_list;
        num           = 13;
    }
    if (envlv <= envlv_list[0]) {
        *lo    = 0;
        *hi    = 0;
        *ratio = 0.0f;
        return;
    }
    if (envlv >= envlv_list[num - 1]) {
        *lo    = num - 1;
        *hi    = num - 1;
        *ratio = 0.0f;
        return;
    }

    for (i = 1; i < num; i++) {
        if (envlv < envlv_list[i]) {
            *lo         = i - 1;
            *hi         = i;
            envlv_lo = envlv_list[*lo];
            envlv_hi = envlv_list[*hi];
            *ratio      = (float)(envlv - envlv_lo) / (envlv_hi - envlv_lo);

            return;
        }
    }
}

XCamReturn MergeSelectParam(MergeContext_t* pMergeCtx, mge_param_t* out, int iso) {
    LOGD_AMERGE("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if (pMergeCtx == NULL) {
        LOGE_AMERGE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    mge_param_auto_t* paut = &pMergeCtx->merge_attrib->stAuto;

    int i       = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0;
    float ratio = 0.0f;

    if (paut->sta.paraLinkCfg.sw_mgeT_paraLink_mode == amge_isoLink_mode) {
        pre_interp(iso, (uint32_t *)paut->sta.paraLinkCfg.sw_mgeT_isoLink_val, MERGE_LINK_NUM, &ilow, &ihigh,
                   &ratio);
    } else if (paut->sta.paraLinkCfg.sw_mgeT_paraLink_mode == amge_envLink_mode) {
        pre_interp_envlv(pMergeCtx->NextCtrlData.ExpoData.EnvLv,
                            paut->sta.paraLinkCfg.sw_mgeT_envLink_val, MERGE_LINK_NUM, &ilow,
                            &ihigh, &ratio);
    } else {
        LOGE_AMERGE("%s(%d): paraLink mode is ERROR!!!\n", __FUNCTION__, __LINE__);
    }

    out->sta.expRat                   = paut->sta.mgeCfg.expRat;
    out->dyn.sw_mgeT_baseFrm_mode     = paut->dyn[ilow].sw_mgeT_baseFrm_mode;
    out->dyn.sw_mgeT_baseHdrL_mode    = paut->dyn[ilow].sw_mgeT_baseHdrL_mode;
    out->dyn.oeWgt.sw_mgeT_oeLut_mode = paut->dyn[ilow].oeWgt.sw_mgeT_oeLut_mode;
    out->dyn.oeWgt.sw_mgeT_lutCreate_slope =
        interpolation_f32(paut->dyn[ilow].oeWgt.sw_mgeT_lutCreate_slope,
                          paut->dyn[ihigh].oeWgt.sw_mgeT_lutCreate_slope, ratio);
    out->dyn.oeWgt.sw_mgeT_lutCreate_offset =
        interpolation_f32(paut->dyn[ilow].oeWgt.sw_mgeT_lutCreate_offset,
                          paut->dyn[ihigh].oeWgt.sw_mgeT_lutCreate_offset, ratio);
    for (i = 0; i < MGE_OECURVE_LEN; i++) {
        out->dyn.oeWgt.sw_mgeT_luma2Wgt_val[i] =
            interpolation_f32(paut->dyn[ilow].oeWgt.sw_mgeT_luma2Wgt_val[i],
                              paut->dyn[ihigh].oeWgt.sw_mgeT_luma2Wgt_val[i], ratio);
    }
    if (out->dyn.sw_mgeT_baseFrm_mode == mge_baseHdrL_mode) {
        out->dyn.mdWgt_baseHdrL.sw_mgeT_mdLut_mode =
            paut->dyn[ilow].mdWgt_baseHdrL.sw_mgeT_mdLut_mode;
        out->dyn.mdWgt_baseHdrL.sw_mgeT_lumaLutCreate_slope =
            interpolation_f32(paut->dyn[ilow].mdWgt_baseHdrL.sw_mgeT_lumaLutCreate_slope,
                              paut->dyn[ihigh].mdWgt_baseHdrL.sw_mgeT_lumaLutCreate_slope, ratio);
        out->dyn.mdWgt_baseHdrL.hw_mgeT_lumaLutCreate_offset =
            interpolation_f32(paut->dyn[ilow].mdWgt_baseHdrL.hw_mgeT_lumaLutCreate_offset,
                              paut->dyn[ihigh].mdWgt_baseHdrL.hw_mgeT_lumaLutCreate_offset, ratio);
        for (i = 0; i < MGE_MDCURVE_LEN; i++) {
            out->dyn.mdWgt_baseHdrL.sw_mgeT_lumaDiff2Wgt_val[i] = interpolation_f32(
                paut->dyn[ilow].mdWgt_baseHdrL.sw_mgeT_lumaDiff2Wgt_val[i],
                paut->dyn[ihigh].mdWgt_baseHdrL.sw_mgeT_lumaDiff2Wgt_val[i], ratio);
        }
        out->dyn.sw_mgeT_baseHdrL_mode = paut->dyn[ilow].sw_mgeT_baseHdrL_mode;
        if (out->dyn.sw_mgeT_baseHdrL_mode == mge_oeMdByLuma_rawCh_mode) {
            out->dyn.mdWgt_baseHdrL.sw_mgeT_rawChLutCreate_slope = interpolation_f32(
                paut->dyn[ilow].mdWgt_baseHdrL.sw_mgeT_rawChLutCreate_slope,
                paut->dyn[ihigh].mdWgt_baseHdrL.sw_mgeT_rawChLutCreate_slope, ratio);
            out->dyn.mdWgt_baseHdrL.sw_mgeT_rawChLutCreate_offset = interpolation_f32(
                paut->dyn[ilow].mdWgt_baseHdrL.sw_mgeT_rawChLutCreate_offset,
                paut->dyn[ihigh].mdWgt_baseHdrL.sw_mgeT_rawChLutCreate_offset, ratio);
            for (i = 0; i < MGE_MDCURVE_LEN; i++) {
                out->dyn.mdWgt_baseHdrL.sw_mgeT_rawChDiff2Wgt_val[i] = interpolation_f32(
                    paut->dyn[ilow].mdWgt_baseHdrL.sw_mgeT_rawChDiff2Wgt_val[i],
                    paut->dyn[ihigh].mdWgt_baseHdrL.sw_mgeT_rawChDiff2Wgt_val[i], ratio);
            }
        }
    } else if (out->dyn.sw_mgeT_baseFrm_mode == mge_baseHdrS_mode) {
        out->dyn.mdWgt_baseHdrS.hw_mgeT_wgtZero_thred =
            interpolation_f32(paut->dyn[ilow].mdWgt_baseHdrS.hw_mgeT_wgtZero_thred,
                              paut->dyn[ihigh].mdWgt_baseHdrS.hw_mgeT_wgtZero_thred, ratio);
        out->dyn.mdWgt_baseHdrS.sw_mgeT_wgtMaxTh_strg =
            interpolation_f32(paut->dyn[ilow].mdWgt_baseHdrS.sw_mgeT_wgtMaxTh_strg,
                              paut->dyn[ihigh].mdWgt_baseHdrS.sw_mgeT_wgtMaxTh_strg, ratio);
    }
    out->dyn.mdWgt_baseHdrS.sw_mgeT_lumaDiff_scale =
        interpolation_f32(paut->dyn[ilow].mdWgt_baseHdrS.sw_mgeT_lumaDiff_scale,
                          paut->dyn[ihigh].mdWgt_baseHdrS.sw_mgeT_lumaDiff_scale, ratio);
#if RKAIQ_HAVE_MERGE_V13
    out->dyn.sw_mgeT_baseHdrS_mode = paut->dyn[ilow].sw_mgeT_baseHdrS_mode;
    out->dyn.mdWgt_baseHdrS.sw_mgeT_lumaThred_minLimit =
        interpolation_f32(paut->dyn[ilow].mdWgt_baseHdrS.sw_mgeT_lumaThred_minLimit,
                          paut->dyn[ihigh].mdWgt_baseHdrS.sw_mgeT_lumaThred_minLimit, ratio);
    out->dyn.mdWgt_baseHdrS.sw_mgeT_lumaThred_maxLimit =
        interpolation_f32(paut->dyn[ilow].mdWgt_baseHdrS.sw_mgeT_lumaThred_maxLimit,
                          paut->dyn[ihigh].mdWgt_baseHdrS.sw_mgeT_lumaThred_maxLimit, ratio);
#endif

    pMergeCtx->CurrData.CtrlData.MoveCoef = pMergeCtx->NextCtrlData.MoveCoef;
    pMergeCtx->NextCtrlData.MergeOEDamp   = paut->sta.sw_mgeT_oeDamp_val;
    pMergeCtx->NextCtrlData.MergeMDDamp   = paut->sta.sw_mgeT_mdDamp_val;
    MergeDamping(pMergeCtx, out);

    memcpy(&pMergeCtx->CurrData.dyn, &out->dyn, sizeof(mge_params_dyn_t));

    pMergeCtx->CurrData.CtrlData.ExpoData.LongFrmMode =
        pMergeCtx->NextCtrlData.ExpoData.LongFrmMode;
    pMergeCtx->CurrData.CtrlData.ExpoData.RatioLS = pMergeCtx->NextCtrlData.ExpoData.RatioLS;
    pMergeCtx->CurrData.CtrlData.ExpoData.RatioLM = pMergeCtx->NextCtrlData.ExpoData.RatioLM;
    pMergeCtx->CurrData.CtrlData.ExpoData.SGain   = pMergeCtx->NextCtrlData.ExpoData.SGain;
    pMergeCtx->CurrData.CtrlData.ExpoData.MGain = pMergeCtx->NextCtrlData.ExpoData.MGain;
    pMergeCtx->CurrData.CtrlData.ExpoData.EnvLv   = pMergeCtx->NextCtrlData.ExpoData.EnvLv;

    return XCAM_RETURN_NO_ERROR;
}

void MergeDamping(MergeContext_t* pMergeCtx, mge_param_t* out) {
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);

    if (pMergeCtx->FrameID > 2) {
        if (out->dyn.sw_mgeT_baseFrm_mode == mge_baseHdrL_mode) {
            out->dyn.oeWgt.sw_mgeT_lutCreate_slope =
                pMergeCtx->NextCtrlData.MergeOEDamp * out->dyn.oeWgt.sw_mgeT_lutCreate_slope +
                (1.0f - pMergeCtx->NextCtrlData.MergeOEDamp) *
                    pMergeCtx->CurrData.dyn.oeWgt.sw_mgeT_lutCreate_slope;
            out->dyn.oeWgt.sw_mgeT_lutCreate_offset =
                pMergeCtx->NextCtrlData.MergeOEDamp * out->dyn.oeWgt.sw_mgeT_lutCreate_offset +
                (1.0f - pMergeCtx->NextCtrlData.MergeOEDamp) *
                    pMergeCtx->CurrData.dyn.oeWgt.sw_mgeT_lutCreate_offset;
            out->dyn.mdWgt_baseHdrL.sw_mgeT_lumaLutCreate_slope =
                pMergeCtx->NextCtrlData.MergeMDDamp *
                    out->dyn.mdWgt_baseHdrL.sw_mgeT_lumaLutCreate_slope +
                (1.0f - pMergeCtx->NextCtrlData.MergeMDDamp) *
                    pMergeCtx->CurrData.dyn.mdWgt_baseHdrL.sw_mgeT_lumaLutCreate_slope;
            out->dyn.mdWgt_baseHdrL.hw_mgeT_lumaLutCreate_offset =
                pMergeCtx->NextCtrlData.MergeMDDamp *
                    out->dyn.mdWgt_baseHdrL.hw_mgeT_lumaLutCreate_offset +
                (1.0f - pMergeCtx->NextCtrlData.MergeMDDamp) *
                    pMergeCtx->CurrData.dyn.mdWgt_baseHdrL.hw_mgeT_lumaLutCreate_offset;
        } else if (out->dyn.sw_mgeT_baseFrm_mode == mge_baseHdrS_mode) {
            out->dyn.oeWgt.sw_mgeT_lutCreate_slope =
                pMergeCtx->NextCtrlData.MergeOEDamp * out->dyn.oeWgt.sw_mgeT_lutCreate_slope +
                (1.0f - pMergeCtx->NextCtrlData.MergeOEDamp) *
                    pMergeCtx->CurrData.dyn.oeWgt.sw_mgeT_lutCreate_slope;
            out->dyn.oeWgt.sw_mgeT_lutCreate_offset =
                pMergeCtx->NextCtrlData.MergeOEDamp * out->dyn.oeWgt.sw_mgeT_lutCreate_offset +
                (1.0f - pMergeCtx->NextCtrlData.MergeOEDamp) *
                    pMergeCtx->CurrData.dyn.oeWgt.sw_mgeT_lutCreate_offset;
            out->dyn.mdWgt_baseHdrS.hw_mgeT_wgtZero_thred =
                pMergeCtx->NextCtrlData.MergeMDDamp *
                    out->dyn.mdWgt_baseHdrS.hw_mgeT_wgtZero_thred +
                (1.0f - pMergeCtx->NextCtrlData.MergeMDDamp) *
                    pMergeCtx->CurrData.dyn.mdWgt_baseHdrS.hw_mgeT_wgtZero_thred;
            out->dyn.mdWgt_baseHdrS.sw_mgeT_wgtMaxTh_strg =
                pMergeCtx->NextCtrlData.MergeMDDamp *
                    out->dyn.mdWgt_baseHdrS.sw_mgeT_wgtMaxTh_strg +
                (1.0f - pMergeCtx->NextCtrlData.MergeMDDamp) *
                    pMergeCtx->CurrData.dyn.mdWgt_baseHdrS.sw_mgeT_wgtMaxTh_strg;
        }
    }

    LOG1_AMERGE("%s:Eixt!\n", __FUNCTION__);
}

#if 0
XCamReturn
algo_merge_SetAttrib(RkAiqAlgoContext* ctx, mge_api_attrib_t *attr)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_AMERGE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    MergeContext_t* pMergeCtx = (MergeContext_t*)ctx;
    mge_api_attrib_t* merge_attrib = pMergeCtx->merge_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_AMERGE("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    merge_attrib->opMode = attr->opMode;
    merge_attrib->en = attr->en;
    merge_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO)
        merge_attrib->stAuto = attr->stAuto;
    else if (attr->opMode == RK_AIQ_OP_MODE_MANUAL)
        merge_attrib->stMan = attr->stMan;
    else {
        LOGW_AMERGE("wrong mode: %d\n", attr->opMode);
    }

    pMergeCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_merge_GetAttrib(RkAiqAlgoContext* ctx, mge_api_attrib_t* attr)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_AMERGE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    MergeContext_t* pMergeCtx = (MergeContext_t*)ctx;
    mge_api_attrib_t* merge_attrib = pMergeCtx->merge_attrib;

#if 0
    if (merge_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_AMERGE("not auto mode: %d", merge_attrib->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }
#endif

    attr->opMode = merge_attrib->opMode;
    attr->en = merge_attrib->en;
    attr->bypass = merge_attrib->bypass;
    memcpy(&attr->stAuto, &merge_attrib->stAuto, sizeof(mge_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}
#endif

#define RKISP_ALGO_MERGE_VERSION     "v0.1.0"
#define RKISP_ALGO_MERGE_VENDOR      "Rockchip"
#define RKISP_ALGO_MERGE_DESCRIPTION "Rockchip merge algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescMerge = {
    .common =
        {
            .version         = RKISP_ALGO_MERGE_VERSION,
            .vendor          = RKISP_ALGO_MERGE_VENDOR,
            .description     = RKISP_ALGO_MERGE_DESCRIPTION,
            .type            = RK_AIQ_ALGO_TYPE_AMERGE,
            .id              = 0,
            .create_context  = create_context,
            .destroy_context = destroy_context,
        },
    .prepare      = prepare,
    .pre_process  = NULL,
    .processing   = processing,
    .post_process = NULL,
};

// RKAIQ_END_DECLARE
