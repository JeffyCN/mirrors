/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
 */
#include "algos/agic/rk_aiq_types_algo_agic_prvt.h"

#define INTERPOLATION(ratio, hi, lo)    (((ratio) * ((hi) - (lo)) + (lo) * (1 << 4) + (1 << 3)) >> 4)
#define INTERPOLATION_F(ratioF, hi, lo) ((ratioF) * ((hi) - (lo)) + (lo))

void GicV2CalibToAttr(CalibDbV2_Gic_V21_t* calib, rkaiq_gic_v2_api_attr_t* attr) {
    CalibDbV2_Gic_V21_t* db = calib;
    Gic_setting_v21_t* settings = &db->GicTuningPara.GicData;
    XCAM_ASSERT(RKAIQ_GIC_MAX_ISO_CNT >= settings->ISO_len);
    attr->gic_en  = db->GicTuningPara.enable;
    attr->iso_cnt = settings->ISO_len;

    for (int i = 0; i < settings->ISO_len; i++) {
        attr->auto_params[i].iso                = settings->ISO[i];
        attr->auto_params[i].bypass             = 0;
        attr->auto_params[i].gr_ratio           = db->GicTuningPara.gr_ration;
        attr->auto_params[i].min_busy_thre      = settings->min_busy_thre[i];
        attr->auto_params[i].min_grad_thr1      = settings->min_grad_thr1[i];
        attr->auto_params[i].min_grad_thr2      = settings->min_grad_thr2[i];
        attr->auto_params[i].k_grad1            = settings->k_grad1[i];
        attr->auto_params[i].k_grad2            = settings->k_grad2[i];
        attr->auto_params[i].gb_thre            = settings->gb_thre[i];
        attr->auto_params[i].maxCorV            = settings->maxCorV[i];
        attr->auto_params[i].maxCorVboth        = settings->maxCorVboth[i];
        attr->auto_params[i].dark_thre          = settings->dark_thre[i];
        attr->auto_params[i].dark_threHi        = settings->dark_threHi[i];
        attr->auto_params[i].k_grad1_dark       = settings->k_grad1_dark[i];
        attr->auto_params[i].k_grad2_dark       = settings->k_grad2_dark[i];
        attr->auto_params[i].min_grad_thr_dark1 = settings->min_grad_thr_dark1[i];
        attr->auto_params[i].min_grad_thr_dark2 = settings->min_grad_thr_dark2[i];
        attr->auto_params[i].NoiseScale         = settings->NoiseScale[i];
        attr->auto_params[i].NoiseBase          = settings->NoiseBase[i];
        attr->auto_params[i].noiseCurve_0       = settings->noiseCurve_0[i];
        attr->auto_params[i].noiseCurve_1       = settings->noiseCurve_1[i];
        attr->auto_params[i].globalStrength     = settings->globalStrength[i];
        attr->auto_params[i].diff_clip          = settings->diff_clip[i];
    }
}

#if 0
void GicV2SelectParam(rkaiq_gic_v2_param_selected_t* selected, int ratio, int index,
                      const rkaiq_gic_v2_param_selected_t* auto_params, int iso_cnt) {
    float ratioF                            = ratio / 16.0f;
    const rkaiq_gic_v2_param_selected_t* lo = &auto_params[index];
    const rkaiq_gic_v2_param_selected_t* hi = &auto_params[index + 1];

    selected->iso           = lo->iso;
    selected->bypass        = lo->bypass;
    selected->gr_ratio      = lo->gr_ratio;
    selected->min_busy_thre = INTERPOLATION(ratio, hi->min_busy_thre, lo->min_busy_thre);
    selected->min_grad_thr1 = INTERPOLATION(ratio, hi->min_grad_thr1, lo->min_grad_thr1);
    selected->min_grad_thr2 = INTERPOLATION(ratio, hi->min_grad_thr2, lo->min_grad_thr2);
    selected->k_grad1       = INTERPOLATION(ratio, hi->k_grad1, lo->k_grad1);
    selected->k_grad2       = INTERPOLATION(ratio, hi->k_grad2, lo->k_grad2);
    selected->gb_thre       = INTERPOLATION(ratio, hi->gb_thre, lo->gb_thre);
    selected->maxCorV       = INTERPOLATION(ratio, hi->maxCorV, lo->maxCorV);
    selected->maxCorVboth   = INTERPOLATION(ratio, hi->maxCorVboth, lo->maxCorVboth);
    selected->dark_thre     = INTERPOLATION(ratio, hi->dark_thre, lo->dark_thre);
    selected->dark_threHi   = INTERPOLATION(ratio, hi->dark_threHi, lo->dark_threHi);
    selected->k_grad1_dark  = INTERPOLATION(ratio, hi->k_grad1_dark, lo->k_grad1_dark);
    selected->k_grad2_dark  = INTERPOLATION(ratio, hi->k_grad2_dark, lo->k_grad2_dark);
    selected->min_grad_thr_dark1 =
        INTERPOLATION(ratio, hi->min_grad_thr_dark1, lo->min_grad_thr_dark1);
    selected->min_grad_thr_dark2 =
        INTERPOLATION(ratio, hi->min_grad_thr_dark2, lo->min_grad_thr_dark2);
    selected->NoiseScale     = INTERPOLATION(ratio, hi->NoiseScale, lo->NoiseScale);
    selected->NoiseBase      = INTERPOLATION(ratio, hi->NoiseBase, lo->NoiseBase);
    selected->globalStrength = INTERPOLATION_F(ratioF, hi->globalStrength, lo->globalStrength);
    selected->diff_clip      = INTERPOLATION(ratio, hi->diff_clip, lo->diff_clip);
}
#else
void GicV2SelectParam(AgicConfigV21_t* selected, int ratio, int index,
                      const rkaiq_gic_v2_param_selected_t* auto_params, int iso_cnt) {
    float ratioF                            = ratio / 16.0f;
    const rkaiq_gic_v2_param_selected_t* lo;
    const rkaiq_gic_v2_param_selected_t* hi;
    if (iso_cnt ==1) {
        lo = &auto_params[index];
        hi = &auto_params[index];
    } else {
        lo = &auto_params[index];
        hi = &auto_params[index + 1];
    }


    selected->gr_ratio       = lo->gr_ratio;
    selected->regminbusythre = INTERPOLATION(ratio, hi->min_busy_thre, lo->min_busy_thre);
    selected->regmingradthr1 = INTERPOLATION(ratio, hi->min_grad_thr1, lo->min_grad_thr1);
    selected->regmingradthr2 = INTERPOLATION(ratio, hi->min_grad_thr2, lo->min_grad_thr2);
    selected->regkgrad1      = INTERPOLATION(ratio, hi->k_grad1, lo->k_grad1);
    selected->regkgrad2      = INTERPOLATION(ratio, hi->k_grad2, lo->k_grad2);
    selected->reggbthre      = INTERPOLATION(ratio, hi->gb_thre, lo->gb_thre);
    selected->regmaxcorv     = INTERPOLATION(ratio, hi->maxCorV, lo->maxCorV);
    selected->regmaxcorvboth = INTERPOLATION(ratio, hi->maxCorVboth, lo->maxCorVboth);
    selected->regdarkthre    = INTERPOLATION(ratio, hi->dark_thre, lo->dark_thre);
    selected->regdarktthrehi = INTERPOLATION(ratio, hi->dark_threHi, lo->dark_threHi);
    selected->regkgrad1dark  = INTERPOLATION(ratio, hi->k_grad1_dark, lo->k_grad1_dark);
    selected->regkgrad2dark  = INTERPOLATION(ratio, hi->k_grad2_dark, lo->k_grad2_dark);
    selected->regmingradthrdark1 =
        INTERPOLATION(ratio, hi->min_grad_thr_dark1, lo->min_grad_thr_dark1);
    selected->regmingradthrdark2 =
        INTERPOLATION(ratio, hi->min_grad_thr_dark2, lo->min_grad_thr_dark2);
    selected->noise_scale    = INTERPOLATION(ratio, hi->NoiseScale, lo->NoiseScale);
    selected->noise_base     = INTERPOLATION(ratio, hi->NoiseBase, lo->NoiseBase);
    selected->diff_clip      = INTERPOLATION(ratio, hi->diff_clip, lo->diff_clip);
    selected->noiseCurve_0   = INTERPOLATION_F(ratioF, hi->noiseCurve_0, lo->noiseCurve_0);
    selected->noiseCurve_1   = INTERPOLATION_F(ratioF, hi->noiseCurve_1, lo->noiseCurve_1);
    selected->globalStrength = INTERPOLATION_F(ratioF, hi->globalStrength, lo->globalStrength);
}
#endif

void GicV2SetManualParam(AgicConfigV21_t* selected,
                         const rkaiq_gic_v2_param_selected_t* manual_param) {
    selected->gr_ratio           = manual_param->gr_ratio;
    selected->regminbusythre     = manual_param->min_busy_thre;
    selected->regmingradthr1     = manual_param->min_grad_thr1;
    selected->regmingradthr2     = manual_param->min_grad_thr2;
    selected->regkgrad1          = manual_param->k_grad1;
    selected->regkgrad2          = manual_param->k_grad2;
    selected->reggbthre          = manual_param->gb_thre;
    selected->regmaxcorv         = manual_param->maxCorV;
    selected->regmaxcorvboth     = manual_param->maxCorVboth;
    selected->regdarkthre        = manual_param->dark_thre;
    selected->regdarktthrehi     = manual_param->dark_threHi;
    selected->regkgrad1dark      = manual_param->k_grad1_dark;
    selected->regkgrad2dark      = manual_param->k_grad2_dark;
    selected->regmingradthrdark1 = manual_param->min_grad_thr_dark1;
    selected->regmingradthrdark2 = manual_param->min_grad_thr_dark2;
    selected->noise_scale        = manual_param->NoiseScale;
    selected->noise_base         = manual_param->NoiseBase;
    selected->noiseCurve_0       = manual_param->noiseCurve_0;
    selected->noiseCurve_1       = manual_param->noiseCurve_1;
    selected->globalStrength     = manual_param->globalStrength;
    selected->diff_clip          = manual_param->diff_clip;
}

void GicV2DumpReg(const rkaiq_gic_v2_hw_param_t* hw_param) {
#ifdef NDEBUG
    (void)(hw_param);
#endif
    LOGV_AGIC(
        "GIC V2 reg values: \n"
        " regmingradthrdark2 %d\n"
        " regmingradthrdark1 %d\n"
        " regminbusythre %d\n"
        " regdarkthre %d\n"
        " regmaxcorvboth %d\n"
        " regdarktthrehi %d\n"
        " regkgrad2dark %d\n"
        " regkgrad1dark %d\n"
        " regstrengthglobal_fix %d\n"
        " regdarkthrestep %d\n"
        " regkgrad2 %d\n"
        " regkgrad1 %d\n"
        " reggbthre %d\n"
        " regmaxcorv %d\n"
        " regmingradthr2 %d\n"
        " regmingradthr1 %d\n"
        " gr_ratio %d\n"
        " noise_scale %d\n"
        " noise_base %d\n"
        " diff_clip %d\n",
        hw_param->regmingradthrdark2, hw_param->regmingradthrdark1, hw_param->regminbusythre,
        hw_param->regdarkthre, hw_param->regmaxcorvboth, hw_param->regdarktthrehi,
        hw_param->regkgrad2dark, hw_param->regkgrad1dark, hw_param->regstrengthglobal_fix,
        hw_param->regdarkthrestep, hw_param->regkgrad2, hw_param->regkgrad1, hw_param->reggbthre,
        hw_param->regmaxcorv, hw_param->regmingradthr2, hw_param->regmingradthr1,
        hw_param->gr_ratio, hw_param->noise_scale, hw_param->noise_base, hw_param->diff_clip);
    for (int i = 0; i < 15; i++) LOGV_AGIC("sigma %d", hw_param->sigma_y[i]);
}

XCamReturn AgicInit(AgicContext_t* pAgicCtx, CamCalibDbV2Context_t* calib) {
    LOG1_AGIC("enter!");

    memset(pAgicCtx, 0, sizeof(AgicContext_t));
    pAgicCtx->state = AGIC_STATE_INITIALIZED;

    CalibDbV2_Gic_V21_t* calibv2_agic_calib_V21 =
        (CalibDbV2_Gic_V21_t*)(CALIBDBV2_GET_MODULE_PTR(calib, agic_calib_v21));
    GicV2CalibToAttr(calibv2_agic_calib_V21, &pAgicCtx->attr.v2);
    //init manula params
    pAgicCtx->attr.v2.manual_param = pAgicCtx->attr.v2.auto_params[0];
    pAgicCtx->attr.v2.op_mode = RKAIQ_GIC_API_OPMODE_AUTO;
    pAgicCtx->calib_changed   = true;
    pAgicCtx->state           = AGIC_STATE_RUNNING;

    LOG1_AGIC("exit!");
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AgicRelease(AgicContext_t* pAgicCtx) {
    LOG1_AGIC("enter!");

    AgicStop(pAgicCtx);

    LOG1_AGIC("exit!");
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AgicStop(AgicContext_t* pAgicCtx) {
    LOG1_AGIC("enter!");

    pAgicCtx->state = AGIC_STATE_STOPPED;

    LOG1_AGIC("exit!");
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AgicPreProcess(AgicContext_t* pAgicCtx) {
    (void)(pAgicCtx);
    return XCAM_RETURN_NO_ERROR;
}

void AgicGetProcResultV21(AgicContext_t* pAgicCtx, AgicProcResult_t* pAgicRes) {
    LOG1_AGIC("enter!");

    pAgicRes->gic_en = pAgicCtx->ConfigData.ConfigV21.gic_en == 0 ? false : true;
    pAgicRes->ProcResV21.regmingradthrdark2 =
        pAgicCtx->ConfigData.ConfigV21.regmingradthrdark2;
    pAgicRes->ProcResV21.regmingradthrdark1 =
        pAgicCtx->ConfigData.ConfigV21.regmingradthrdark1;
    pAgicRes->ProcResV21.regminbusythre = pAgicCtx->ConfigData.ConfigV21.regminbusythre;
    pAgicRes->ProcResV21.regdarkthre    = pAgicCtx->ConfigData.ConfigV21.regdarkthre;
    pAgicRes->ProcResV21.regmaxcorvboth = pAgicCtx->ConfigData.ConfigV21.regmaxcorvboth;
    pAgicRes->ProcResV21.regdarktthrehi = pAgicCtx->ConfigData.ConfigV21.regdarktthrehi;
    pAgicRes->ProcResV21.regkgrad2dark =
        (int)(log(double(pAgicCtx->ConfigData.ConfigV21.regkgrad2dark)) / log((double)2) + 0.5f);
    pAgicRes->ProcResV21.regkgrad1dark =
        (int)(log(double(pAgicCtx->ConfigData.ConfigV21.regkgrad1dark)) / log((double)2) + 0.5f);

    pAgicRes->ProcResV21.regstrengthglobal_fix =
        (int)(pAgicCtx->ConfigData.ConfigV21.globalStrength * (1 << 7));
    if (pAgicRes->ProcResV21.regstrengthglobal_fix > (1 << 7) - 1)
        pAgicRes->ProcResV21.regstrengthglobal_fix = 7 + 1;
    else
        pAgicRes->ProcResV21.regstrengthglobal_fix =
            int(log(double((1 << 7) - pAgicRes->ProcResV21.regstrengthglobal_fix)) /
                log((double)2) +
                0.5f);

    pAgicRes->ProcResV21.regdarkthrestep =
        int(log(double(pAgicCtx->ConfigData.ConfigV21.regdarktthrehi -
                       pAgicCtx->ConfigData.ConfigV21.regdarkthre)) /
            log((double)2) +
            0.5f);
    pAgicRes->ProcResV21.regkgrad2 =
        (int)(log(double(pAgicCtx->ConfigData.ConfigV21.regkgrad2)) / log((double)2) + 0.5f);
    pAgicRes->ProcResV21.regkgrad1 =
        (int)(log(double(pAgicCtx->ConfigData.ConfigV21.regkgrad1)) / log((double)2) + 0.5f);
    pAgicRes->ProcResV21.reggbthre =
        int(log(double(pAgicCtx->ConfigData.ConfigV21.reggbthre)) / log((double)2) + 0.5f);
    pAgicRes->ProcResV21.regmaxcorv     = pAgicCtx->ConfigData.ConfigV21.regmaxcorv;
    pAgicRes->ProcResV21.regmingradthr1 = pAgicCtx->ConfigData.ConfigV21.regmingradthr1;
    pAgicRes->ProcResV21.regmingradthr2 = pAgicCtx->ConfigData.ConfigV21.regmingradthr2;
    pAgicRes->ProcResV21.gr_ratio       = pAgicCtx->ConfigData.ConfigV21.gr_ratio;
    pAgicRes->ProcResV21.noise_scale =
        (pAgicCtx->ConfigData.ConfigV21.noise_scale);
    pAgicRes->ProcResV21.noise_base =
        (int)(pAgicCtx->ConfigData.ConfigV21.noise_base);

    for (int i = 0; i < 15; i++) {
        pAgicRes->ProcResV21.sigma_y[i] =
            (int)(pAgicCtx->ConfigData.ConfigV21.sigma_y[i] * (1 << 7));
    }

    pAgicRes->ProcResV21.diff_clip = pAgicCtx->ConfigData.ConfigV21.diff_clip;

    if (pAgicRes->ProcResV21.regkgrad2dark < pAgicRes->ProcResV21.regkgrad2)
        GIC_SWAP(int, pAgicRes->ProcResV21.regkgrad2dark,
                 pAgicRes->ProcResV21.regkgrad2);

    if (pAgicRes->ProcResV21.regmingradthrdark1 <
            pAgicRes->ProcResV21.regmingradthr1)
        GIC_SWAP(int, pAgicRes->ProcResV21.regmingradthrdark1,
                 pAgicRes->ProcResV21.regmingradthr1);

    if (pAgicRes->ProcResV21.regmingradthrdark2 <
            pAgicRes->ProcResV21.regmingradthr2)
        GIC_SWAP(int, pAgicRes->ProcResV21.regmingradthrdark2,
                 pAgicRes->ProcResV21.regmingradthr2);

    if (pAgicRes->ProcResV21.regdarktthrehi < pAgicRes->ProcResV21.regdarkthre)
        GIC_SWAP(int, pAgicRes->ProcResV21.regdarktthrehi,
                 pAgicRes->ProcResV21.regdarkthre);

    GicV2DumpReg(&pAgicRes->ProcResV21);

    LOG1_AGIC("exit!");
}

void AgicProcessV21(AgicContext_t* pAgicCtx, int ISO) {
    LOG1_AGIC("enter!");

    float ave1 = 0.0f, noiseSigma = 0.0f;
    short ratio        = 0;
    short LumaPoints[] = {0,    128,  256,  384,  512,  640,  768, 896,
                          1024, 1536, 2048, 2560, 3072, 3584, 4096
                         };
    int index = 0, iso_hi = 0, iso_lo = 0;

    LOGD_AGIC("enter, ISO=%d", ISO);

    if (pAgicCtx == NULL) LOGE_AGIC("null pointer");

    if (ISO <= 50) {
        index = 0;
        ratio = 0;
    } else if (ISO > 12800) {
        index = pAgicCtx->attr.v2.iso_cnt - 2;
        ratio = (1 << 4);
    } else {
        int i = 0;
        iso_hi = ISO;
        iso_lo = 50;
        for (i = 0; i < (int)(pAgicCtx->attr.v2.iso_cnt - 2); i++) {
            iso_lo = (int)(pAgicCtx->attr.v2.auto_params[i].iso);
            iso_hi = (int)(pAgicCtx->attr.v2.auto_params[i + 1].iso);
            LOGD_AGIC("index=%d,  iso_lo=%d, iso_hi=%d", index, iso_lo, iso_hi);
            if (ISO > iso_lo && ISO <= iso_hi) {
                break;
            }
        }
        if (iso_hi == iso_lo){
            ratio = 0;
        } else {
            ratio = ((ISO - iso_lo) * (1 << 4)) / (iso_hi - iso_lo);
        }
        index = i;
    }

    if (pAgicCtx->attr.v2.op_mode == RKAIQ_GIC_API_OPMODE_AUTO) {
        GicV2SelectParam(&pAgicCtx->ConfigData.ConfigV21, ratio, index,
                         pAgicCtx->attr.v2.auto_params, pAgicCtx->attr.v2.iso_cnt);
    } else {
        GicV2SetManualParam(&pAgicCtx->ConfigData.ConfigV21, &pAgicCtx->attr.v2.manual_param);
    }
    pAgicCtx->ConfigData.ConfigV21.gic_en = pAgicCtx->attr.v2.gic_en;

    for (int i = 0; i < 15; i++) {
        ave1       = LumaPoints[i];
        noiseSigma = pAgicCtx->ConfigData.ConfigV21.noiseCurve_0 * sqrt(ave1) +
                     pAgicCtx->ConfigData.ConfigV21.noiseCurve_1;
        if (noiseSigma < 0) {
            noiseSigma = 0;
        }
        pAgicCtx->ConfigData.ConfigV21.sigma_y[i] = noiseSigma;
    }

    short mulBit = 0;
    int bitValue = pAgicCtx->raw_bits;
    if (bitValue > 10) {
        mulBit = 1 << (bitValue - 10);
    } else {
        mulBit = 1;
    }
    pAgicCtx->ConfigData.ConfigV21.regminbusythre *= mulBit;
    pAgicCtx->ConfigData.ConfigV21.regmingradthr1 *= mulBit;
    pAgicCtx->ConfigData.ConfigV21.regmingradthr2 *= mulBit;
    pAgicCtx->ConfigData.ConfigV21.reggbthre *= mulBit;
    pAgicCtx->ConfigData.ConfigV21.regmaxcorv *= mulBit;
    pAgicCtx->ConfigData.ConfigV21.regmaxcorvboth *= mulBit;
    pAgicCtx->ConfigData.ConfigV21.regdarkthre *= mulBit;
    pAgicCtx->ConfigData.ConfigV21.regdarktthrehi *= mulBit;
    pAgicCtx->ConfigData.ConfigV21.regmingradthrdark1 *= mulBit;
    pAgicCtx->ConfigData.ConfigV21.regmingradthrdark2 *= mulBit;
    pAgicCtx->ConfigData.ConfigV21.noise_base *= mulBit;

    LOG1_AGIC("exit!");
}

XCamReturn AgicProcess(AgicContext_t* pAgicCtx, int ISO, int mode) {
    (void)(mode);

    LOG1_AGIC("enter!");

    AgicProcessV21(pAgicCtx, ISO);

    LOG1_AGIC("exit!");
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AgicGetProcResult(AgicContext_t* pAgicCtx, AgicProcResult_t* pAgicRes) {
    LOG1_AGIC("enter!");

    if (pAgicCtx == NULL) {
        LOGE_AGIC("null pointer");
        return XCAM_RETURN_ERROR_PARAM;
    }

    AgicGetProcResultV21(pAgicCtx, pAgicRes);

    LOG1_AGIC("exit!");
    return XCAM_RETURN_NO_ERROR;
}

