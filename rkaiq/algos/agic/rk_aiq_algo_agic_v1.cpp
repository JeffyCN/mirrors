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

void GicV1CalibToAttr(CalibDbV2_Gic_V20_t* calib, rkaiq_gic_v1_api_attr_t* attr) {
    CalibDbV2_Gic_V20_t* db = calib;
    Gic_setting_v20_t* settings = &db->GicTuningPara.GicData;
    XCAM_ASSERT(RKAIQ_GIC_MAX_ISO_CNT >= settings->ISO_len);
    attr->gic_en       = db->GicTuningPara.enable;
    attr->edge_open    = db->GicTuningPara.edge_en;
    attr->noise_cut_en = db->GicTuningPara.noise_cut_en;
    attr->iso_cnt      = settings->ISO_len;

    for (int i = 0; i < settings->ISO_len; i++) {
        attr->auto_params[i].bypass             = 0;
        attr->auto_params[i].iso                = settings->ISO[i];
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
        attr->auto_params[i].noiseCurve_0       = settings->noiseCurve_0[i];
        attr->auto_params[i].noiseCurve_1       = settings->noiseCurve_1[i];
        attr->auto_params[i].GValueLimitLo      = settings->GValueLimitLo[i];
        attr->auto_params[i].GValueLimitHi      = settings->GValueLimitHi[i];
        attr->auto_params[i].textureStrength    = settings->textureStrength[i];
        attr->auto_params[i].ScaleLo            = settings->ScaleLo[i];
        attr->auto_params[i].ScaleHi            = settings->ScaleHi[i];
        attr->auto_params[i].globalStrength     = settings->globalStrength[i];
        attr->auto_params[i].diff_clip          = settings->diff_clip[i];
    }
}

#if 0
void GicV1SelectParam(rkaiq_gic_v1_param_selected_t* selected, int ratio, int index,
                      const rkaiq_gic_v1_param_selected_t* auto_params, int iso_cnt) {
    float ratioF                            = ratio / 16.0f;
    const rkaiq_gic_v1_param_selected_t* lo = &auto_params[index];
    const rkaiq_gic_v1_param_selected_t* hi = &auto_params[index + 1];

    selected->iso           = lo->iso;
    selected->bypass        = lo->bypass;
    selected->gr_ratio      = lo->gr_ratio;
    selected->min_busy_thre = INTERPOLATION(ratio, hi->min_busy_thre, lo->min_busy_thre);
    selected->min_grad_thr1 = INTERPOLATION(ratio, hi->min_grad_thr1, lo->min_grad_thr1);
    selected->min_grad_thr2 = INTERPOLATION(ratio, hi->min_grad_thr2, lo->min_grad_thr2);
    selected->k_grad1       = INTERPOLATION(ratio, hi->k_grad1, lo->k_grad1);
    selected->k_grad2       = INTERPOLATION(ratio, hi->k_grad2, lo->k_grad1);
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
    selected->GValueLimitLo   = INTERPOLATION_F(ratioF, hi->GValueLimitLo, lo->GValueLimitLo);
    selected->GValueLimitHi   = INTERPOLATION_F(ratioF, hi->GValueLimitHi, lo->GValueLimitHi);
    selected->textureStrength = INTERPOLATION_F(ratioF, hi->textureStrength, lo->textureStrength);
    selected->ScaleLo         = INTERPOLATION_F(ratioF, hi->ScaleLo, lo->ScaleLo);
    selected->ScaleHi         = INTERPOLATION_F(ratioF, hi->ScaleHi, lo->ScaleHi);
    selected->globalStrength  = INTERPOLATION_F(ratioF, hi->globalStrength, lo->globalStrength);
    selected->noiseCurve_0    = INTERPOLATION_F(ratioF, hi->noiseCurve_0, lo->noiseCurve_0);
    selected->noiseCurve_1    = INTERPOLATION_F(ratioF, hi->noiseCurve_1, lo->noiseCurve_1);
}
#else
void GicV1SelectParam(AgicConfigV20_t* selected, int ratio, int index,
                      const rkaiq_gic_v1_param_selected_t* auto_params, int iso_cnt) {
    float ratioF                            = ratio / 16.0f;
    const rkaiq_gic_v1_param_selected_t* lo = &auto_params[index];
    const rkaiq_gic_v1_param_selected_t* hi = &auto_params[index + 1];

    selected->gr_ratio       = lo->gr_ratio;
    selected->regminbusythre = INTERPOLATION(ratio, hi->min_busy_thre, lo->min_busy_thre);
    selected->regmingradthr1 = INTERPOLATION(ratio, hi->min_grad_thr1, lo->min_grad_thr1);
    selected->regmingradthr2 = INTERPOLATION(ratio, hi->min_grad_thr2, lo->min_grad_thr2);
    selected->regkgrad1      = INTERPOLATION(ratio, hi->k_grad1, lo->k_grad1);
    selected->regkgrad2      = INTERPOLATION(ratio, hi->k_grad2, lo->k_grad1);
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
    selected->gvaluelimitlo   = INTERPOLATION_F(ratioF, hi->GValueLimitLo, lo->GValueLimitLo);
    selected->gvaluelimithi   = INTERPOLATION_F(ratioF, hi->GValueLimitHi, lo->GValueLimitHi);
    selected->textureStrength = INTERPOLATION_F(ratioF, hi->textureStrength, lo->textureStrength);
    selected->dnloscale       = INTERPOLATION_F(ratioF, hi->ScaleLo, lo->ScaleLo);
    selected->dnhiscale       = INTERPOLATION_F(ratioF, hi->ScaleHi, lo->ScaleHi);
    selected->globalStrength  = INTERPOLATION_F(ratioF, hi->globalStrength, lo->globalStrength);
    selected->noiseCurve_0    = INTERPOLATION_F(ratioF, hi->noiseCurve_0, lo->noiseCurve_0);
    selected->noiseCurve_1    = INTERPOLATION_F(ratioF, hi->noiseCurve_1, lo->noiseCurve_1);
}
#endif

void GicV1SetManualParam(AgicConfigV20_t* selected,
                         const rkaiq_gic_v1_param_selected_t* manual_param) {
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

    selected->gvaluelimitlo   = manual_param->GValueLimitLo;
    selected->gvaluelimithi   = manual_param->GValueLimitHi;
    selected->textureStrength = manual_param->textureStrength;
    selected->dnloscale       = manual_param->ScaleLo;
    selected->dnhiscale       = manual_param->ScaleHi;
    selected->globalStrength  = manual_param->globalStrength;
    selected->noiseCurve_0    = manual_param->noiseCurve_0;
    selected->noiseCurve_1    = manual_param->noiseCurve_1;
}

void GicV1DumpReg(const rkaiq_gic_v1_hw_param_t* hw_param) {
#ifdef NDEBUG
    (void)(hw_param);
#endif
    LOG1_AGIC(
        " GIC V1 reg values: "
        " regmingradthrdark2 %d"
        " regmingradthrdark1 %d"
        " regminbusythre %d"
        " regdarkthre %d"
        " regmaxcorvboth %d"
        " regdarktthrehi %d"
        " regkgrad2dark %d"
        " regkgrad1dark %d"
        " regstrengthglobal_fix %d"
        " regdarkthrestep %d"
        " regkgrad2 %d"
        " regkgrad1 %d"
        " reggbthre %d"
        " regmaxcorv %d"
        " regmingradthr2 %d"
        " regmingradthr1 %d"
        " gr_ratio %d"
        " dnloscale %d"
        " dnhiscale %d"
        " reglumapointsstep %d"
        " gvaluelimitlo %d"
        " gvaluelimithi %d"
        " fusionratiohilimt1 %d"
        " regstrength_fix %d"
        " noise_cut_en %d"
        " noise_coe_a %d"
        " noise_coe_b %d"
        " diff_clip %d",
        hw_param->regmingradthrdark2, hw_param->regmingradthrdark1, hw_param->regminbusythre,
        hw_param->regdarkthre, hw_param->regmaxcorvboth, hw_param->regdarktthrehi,
        hw_param->regkgrad2dark, hw_param->regkgrad1dark, hw_param->regstrengthglobal_fix,
        hw_param->regdarkthrestep, hw_param->regkgrad2, hw_param->regkgrad1, hw_param->reggbthre,
        hw_param->regmaxcorv, hw_param->regmingradthr2, hw_param->regmingradthr1,
        hw_param->gr_ratio, hw_param->dnloscale, hw_param->dnhiscale, hw_param->reglumapointsstep,
        hw_param->gvaluelimitlo, hw_param->gvaluelimithi, hw_param->fusionratiohilimt1,
        hw_param->regstrength_fix, hw_param->noise_cut_en, hw_param->noise_coe_a,
        hw_param->noise_coe_b, hw_param->diff_clip);
}

XCamReturn AgicInit(AgicContext_t* pAgicCtx, CamCalibDbV2Context_t* calib) {
    LOG1_AGIC("enter!");

    memset(pAgicCtx, 0, sizeof(AgicContext_t));
    pAgicCtx->state = AGIC_STATE_INITIALIZED;

    CalibDbV2_Gic_V20_t* calibv2_agic_calib_V20 =
        (CalibDbV2_Gic_V20_t*)(CALIBDBV2_GET_MODULE_PTR(calib, agic_calib_v20));
    GicV1CalibToAttr(calibv2_agic_calib_V20, &pAgicCtx->attr.v1);
    pAgicCtx->attr.v1.op_mode = RKAIQ_GIC_API_OPMODE_AUTO;

    pAgicCtx->calib_changed = true;
    pAgicCtx->state         = AGIC_STATE_RUNNING;

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
    return XCAM_RETURN_NO_ERROR;
}

void AgicGetProcResultV20(AgicContext_t* pAgicCtx, AgicProcResult_t* pAgicRes) {
    LOG1_AGIC("enter!");

    pAgicRes->gic_en = pAgicCtx->ConfigData.ConfigV20.gic_en == 0 ? false : true;
    pAgicRes->ProcResV20.edge_open = pAgicCtx->ConfigData.ConfigV20.edge_open;
    pAgicRes->ProcResV20.regmingradthrdark2 =
        pAgicCtx->ConfigData.ConfigV20.regmingradthrdark2;
    pAgicRes->ProcResV20.regmingradthrdark1 =
        pAgicCtx->ConfigData.ConfigV20.regmingradthrdark1;
    pAgicRes->ProcResV20.regminbusythre = pAgicCtx->ConfigData.ConfigV20.regminbusythre;
    pAgicRes->ProcResV20.regdarkthre    = pAgicCtx->ConfigData.ConfigV20.regdarkthre;
    pAgicRes->ProcResV20.regmaxcorvboth = pAgicCtx->ConfigData.ConfigV20.regmaxcorvboth;
    pAgicRes->ProcResV20.regdarktthrehi = pAgicCtx->ConfigData.ConfigV20.regdarktthrehi;
    pAgicRes->ProcResV20.regkgrad2dark =
        (int)(log(double(pAgicCtx->ConfigData.ConfigV20.regkgrad2dark)) / log((double)2) + 0.5f);
    pAgicRes->ProcResV20.regkgrad1dark =
        (int)(log(double(pAgicCtx->ConfigData.ConfigV20.regkgrad1dark)) / log((double)2) + 0.5f);

    pAgicRes->ProcResV20.regstrengthglobal_fix =
        (int)(pAgicCtx->ConfigData.ConfigV20.globalStrength * (1 << 7));
    if (pAgicRes->ProcResV20.regstrengthglobal_fix > (1 << 7) - 1)
        pAgicRes->ProcResV20.regstrengthglobal_fix = 7 + 1;
    else
        pAgicRes->ProcResV20.regstrengthglobal_fix =
            int(log(double((1 << 7) - pAgicRes->ProcResV20.regstrengthglobal_fix)) /
                log((double)2) +
                0.5f);

    pAgicRes->ProcResV20.regdarkthrestep =
        int(log(double(pAgicCtx->ConfigData.ConfigV20.regdarktthrehi -
                       pAgicCtx->ConfigData.ConfigV20.regdarkthre)) /
            log((double)2) +
            0.5f);
    pAgicRes->ProcResV20.regkgrad2 =
        (int)(log(double(pAgicCtx->ConfigData.ConfigV20.regkgrad2)) / log((double)2) + 0.5f);
    pAgicRes->ProcResV20.regkgrad1 =
        (int)(log(double(pAgicCtx->ConfigData.ConfigV20.regkgrad1)) / log((double)2) + 0.5f);
    pAgicRes->ProcResV20.reggbthre =
        int(log(double(pAgicCtx->ConfigData.ConfigV20.reggbthre)) / log((double)2) + 0.5f);
    pAgicRes->ProcResV20.regmaxcorv     = pAgicCtx->ConfigData.ConfigV20.regmaxcorv;
    pAgicRes->ProcResV20.regmingradthr1 = pAgicCtx->ConfigData.ConfigV20.regmingradthr1;
    pAgicRes->ProcResV20.regmingradthr2 = pAgicCtx->ConfigData.ConfigV20.regmingradthr2;
    pAgicRes->ProcResV20.gr_ratio       = pAgicCtx->ConfigData.ConfigV20.gr_ratio;
    pAgicRes->ProcResV20.dnloscale =
        (int)(pAgicCtx->ConfigData.ConfigV20.dnloscale * (1 << 7));
    pAgicRes->ProcResV20.dnhiscale =
        (int)(pAgicCtx->ConfigData.ConfigV20.dnhiscale * (1 << 7));
    pAgicRes->ProcResV20.reglumapointsstep =
        pAgicCtx->ConfigData.ConfigV20.reglumapointsstep;
    pAgicRes->ProcResV20.gvaluelimitlo = (int)pAgicCtx->ConfigData.ConfigV20.gvaluelimitlo;
    pAgicRes->ProcResV20.gvaluelimithi = (int)pAgicCtx->ConfigData.ConfigV20.gvaluelimithi;
    pAgicRes->ProcResV20.fusionratiohilimt1 =
        (int)(pAgicCtx->ConfigData.ConfigV20.fusionratiohilimt1 * (1 << 7));
    pAgicRes->ProcResV20.regstrength_fix =
        (int)(pAgicCtx->ConfigData.ConfigV20.textureStrength * (1 << 7));

    for (int i = 0; i < 15; i++) {
        pAgicRes->ProcResV20.sigma_y[i] =
            (int)(pAgicCtx->ConfigData.ConfigV20.sigma_y[i] * (1 << 7));
    }

    pAgicRes->ProcResV20.noise_cut_en = pAgicCtx->ConfigData.ConfigV20.noise_cut_en;
    pAgicRes->ProcResV20.noise_coe_a  = pAgicCtx->ConfigData.ConfigV20.noise_coe_a;
    pAgicRes->ProcResV20.noise_coe_b  = pAgicCtx->ConfigData.ConfigV20.noise_coe_b;
    pAgicRes->ProcResV20.diff_clip    = pAgicCtx->ConfigData.ConfigV20.diff_clip;

    if (pAgicRes->ProcResV20.regkgrad2dark < pAgicRes->ProcResV20.regkgrad2)
        GIC_SWAP(int, pAgicRes->ProcResV20.regkgrad2dark,
                 pAgicRes->ProcResV20.regkgrad2);

    if (pAgicRes->ProcResV20.regmingradthrdark1 <
            pAgicRes->ProcResV20.regmingradthr1)
        GIC_SWAP(int, pAgicRes->ProcResV20.regmingradthrdark1,
                 pAgicRes->ProcResV20.regmingradthr1);

    if (pAgicRes->ProcResV20.regmingradthrdark2 <
            pAgicRes->ProcResV20.regmingradthr2)
        GIC_SWAP(int, pAgicRes->ProcResV20.regmingradthrdark2,
                 pAgicRes->ProcResV20.regmingradthr2);

    if (pAgicRes->ProcResV20.regdarktthrehi < pAgicRes->ProcResV20.regdarkthre)
        GIC_SWAP(int, pAgicRes->ProcResV20.regdarktthrehi,
                 pAgicRes->ProcResV20.regdarkthre);

    GicV1DumpReg(&pAgicRes->ProcResV20);

    LOG1_AGIC("exit!");
}

void AgicProcessV20(AgicContext_t* pAgicCtx, int ISO) {
    LOG1_AGIC("enter!");

    float ave1 = 0.0f, noiseSigma = 0.0f;
    int ratio  = 0;
    int iso_lo = 50, iso_hi = 50;
    int index          = 0;
    short LumaPoints[] = {0,    128,  256,  384,  512,  640,  768, 896,
                          1024, 1536, 2048, 2560, 3072, 3584, 4096
                         };

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
        for (i = 0; i < int(pAgicCtx->attr.v1.iso_cnt - 2); i++) {
            iso_lo = (int)(pAgicCtx->attr.v1.auto_params[i].iso);
            iso_hi = (int)(pAgicCtx->attr.v1.auto_params[i + 1].iso);
            LOGD_AGIC("index=%d,  iso_lo=%d, iso_hi=%d", index, iso_lo, iso_hi);
            if (ISO > iso_lo && ISO <= iso_hi) {
                break;
            }
        }
        ratio = ((ISO - iso_lo) * (1 << 4)) / (iso_hi - iso_lo);
        index = i;
    }

    if (pAgicCtx->attr.v1.op_mode == RKAIQ_GIC_API_OPMODE_AUTO) {
        GicV1SelectParam(&pAgicCtx->ConfigData.ConfigV20, ratio, index,
                         pAgicCtx->attr.v1.auto_params, pAgicCtx->attr.v1.iso_cnt);
    } else {
        GicV1SetManualParam(&pAgicCtx->ConfigData.ConfigV20, &pAgicCtx->attr.v1.manual_param);
    }
    pAgicCtx->ConfigData.ConfigV20.gic_en             = pAgicCtx->attr.v1.gic_en;
    pAgicCtx->ConfigData.ConfigV20.edge_open          = pAgicCtx->attr.v1.edge_open;
    pAgicCtx->ConfigData.ConfigV20.noise_cut_en       = pAgicCtx->attr.v1.noise_cut_en;
    pAgicCtx->ConfigData.ConfigV20.reglumapointsstep  = 7;
    pAgicCtx->ConfigData.ConfigV20.fusionratiohilimt1 = 0.75;

    for (int i = 0; i < 15; i++) {
        ave1       = LumaPoints[i];
        noiseSigma = pAgicCtx->ConfigData.ConfigV20.noiseCurve_0 * sqrt(ave1) +
                     pAgicCtx->ConfigData.ConfigV20.noiseCurve_1;
        if (noiseSigma < 0) {
            noiseSigma = 0;
        }
        pAgicCtx->ConfigData.ConfigV20.sigma_y[i] = noiseSigma;
    }

    short mulBit = 0;
    int bitValue = pAgicCtx->raw_bits;
    if (bitValue > 10) {
        mulBit = 1 << (bitValue - 10);
    } else {
        mulBit = 1;
    }
    pAgicCtx->ConfigData.ConfigV20.regminbusythre *= mulBit;
    pAgicCtx->ConfigData.ConfigV20.regmingradthr1 *= mulBit;
    pAgicCtx->ConfigData.ConfigV20.regmingradthr2 *= mulBit;
    pAgicCtx->ConfigData.ConfigV20.reggbthre *= mulBit;
    pAgicCtx->ConfigData.ConfigV20.regmaxcorv *= mulBit;
    pAgicCtx->ConfigData.ConfigV20.regmaxcorvboth *= mulBit;
    pAgicCtx->ConfigData.ConfigV20.regdarkthre *= mulBit;
    pAgicCtx->ConfigData.ConfigV20.regdarktthrehi *= mulBit;
    pAgicCtx->ConfigData.ConfigV20.regmingradthrdark1 *= mulBit;
    pAgicCtx->ConfigData.ConfigV20.regmingradthrdark2 *= mulBit;

    LOG1_AGIC("exit!");
}

XCamReturn AgicProcess(AgicContext_t* pAgicCtx, int ISO, int mode) {
    (void)(mode);

    LOG1_AGIC("enter!");

    AgicProcessV20(pAgicCtx, ISO);

    LOG1_AGIC("exit!");
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AgicGetProcResult(AgicContext_t* pAgicCtx, AgicProcResult_t* pAgicRes) {
    LOG1_AGIC("enter!");

    if (pAgicCtx == NULL) {
        LOGE_AGIC("null pointer");
        return XCAM_RETURN_ERROR_PARAM;
    }

    AgicGetProcResultV20(pAgicCtx, pAgicRes);

    LOG1_AGIC("exit!");
    return XCAM_RETURN_NO_ERROR;
}

