
#include "rk_aiq_types_algo_agic_prvt.h"


XCamReturn
AgicInit
(
    AgicContext_t *pAgicCtx,
    CamCalibDbV2Context_t* calib
)
{
    LOGI_AGIC("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAgicCtx == NULL) {
        LOGE_AGIC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    memset(pAgicCtx, 0, sizeof(AgicContext_t));
    pAgicCtx->state = AGIC_STATE_INITIALIZED;

    if(CHECK_ISP_HW_V20()) {
        CalibDbV2_Gic_V20_t* calibv2_agic_calib_V20 =
            (CalibDbV2_Gic_V20_t*)(CALIBDBV2_GET_MODULE_PTR(calib, agic_calib_v20));
        pAgicCtx->full_param.gic_v20 = calibv2_agic_calib_V20;
    } else if(CHECK_ISP_HW_V21()) {
        CalibDbV2_Gic_V21_t* calibv2_agic_calib_V21 =
            (CalibDbV2_Gic_V21_t*)(CALIBDBV2_GET_MODULE_PTR(calib, agic_calib_v21));
        pAgicCtx->full_param.gic_v21 = calibv2_agic_calib_V21;
    }
    pAgicCtx->calib_changed = true;
    pAgicCtx->state = AGIC_STATE_RUNNING;

    LOGI_AGIC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
AgicRelease
(
    AgicContext_t *pAgicCtx
)
{
    LOGI_AGIC("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAgicCtx == NULL) {
        LOGE_AGIC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    AgicStop(pAgicCtx);

    LOGI_AGIC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
AgicStop
(
    AgicContext_t *pAgicCtx
)
{
    LOGI_AGIC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAgicCtx == NULL) {
        LOGE_AGIC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    pAgicCtx->state = AGIC_STATE_STOPPED;
    LOGI_AGIC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AgicPreProcess(AgicContext_t *pAgicCtx)
{
    return XCAM_RETURN_NO_ERROR;
}

void AgicGetProcResultV21
(
    AgicContext_t*    pAgicCtx
)
{
    LOGI_AGIC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAgicCtx->ProcRes.gic_en = pAgicCtx->ConfigData.ConfigV21.gic_en == 0 ? false : true;
    pAgicCtx->ProcRes.ProcResV21.regmingradthrdark2 = pAgicCtx->ConfigData.ConfigV21.regmingradthrdark2;
    pAgicCtx->ProcRes.ProcResV21.regmingradthrdark1 = pAgicCtx->ConfigData.ConfigV21.regmingradthrdark1;
    pAgicCtx->ProcRes.ProcResV21.regminbusythre = pAgicCtx->ConfigData.ConfigV21.regminbusythre;
    pAgicCtx->ProcRes.ProcResV21.regdarkthre = pAgicCtx->ConfigData.ConfigV21.regdarkthre;
    pAgicCtx->ProcRes.ProcResV21.regmaxcorvboth = pAgicCtx->ConfigData.ConfigV21.regmaxcorvboth;
    pAgicCtx->ProcRes.ProcResV21.regdarktthrehi = pAgicCtx->ConfigData.ConfigV21.regdarktthrehi;
    pAgicCtx->ProcRes.ProcResV21.regkgrad2dark = (int)(log(double(pAgicCtx->ConfigData.ConfigV21.regkgrad2dark)) / log((double)2) + 0.5f);
    pAgicCtx->ProcRes.ProcResV21.regkgrad1dark = (int)(log(double(pAgicCtx->ConfigData.ConfigV21.regkgrad1dark)) / log((double)2) + 0.5f);

    pAgicCtx->ProcRes.ProcResV21.regstrengthglobal_fix =  (int)(pAgicCtx->ConfigData.ConfigV21.globalStrength * (1 << 7));
    if (pAgicCtx->ProcRes.ProcResV21.regstrengthglobal_fix > (1 << 7) - 1)
        pAgicCtx->ProcRes.ProcResV21.regstrengthglobal_fix = 7 + 1;
    else
        pAgicCtx->ProcRes.ProcResV21.regstrengthglobal_fix = int(log(double((1 << 7) - pAgicCtx->ProcRes.ProcResV21.regstrengthglobal_fix)) / log((double)2) + 0.5f);

    pAgicCtx->ProcRes.ProcResV21.regdarkthrestep = int(log(double(pAgicCtx->ConfigData.ConfigV21.regdarktthrehi - pAgicCtx->ConfigData.ConfigV21.regdarkthre)) / log((double)2) + 0.5f);
    pAgicCtx->ProcRes.ProcResV21.regkgrad2 = (int)(log(double(pAgicCtx->ConfigData.ConfigV21.regkgrad2)) / log((double)2) + 0.5f);
    pAgicCtx->ProcRes.ProcResV21.regkgrad1 = (int)(log(double(pAgicCtx->ConfigData.ConfigV21.regkgrad1)) / log((double)2) + 0.5f);
    pAgicCtx->ProcRes.ProcResV21.reggbthre = int(log(double(pAgicCtx->ConfigData.ConfigV21.reggbthre)) / log((double)2) + 0.5f);
    pAgicCtx->ProcRes.ProcResV21.regmaxcorv = pAgicCtx->ConfigData.ConfigV21.regmaxcorv;
    pAgicCtx->ProcRes.ProcResV21.regmingradthr1 = pAgicCtx->ConfigData.ConfigV21.regmingradthr1;
    pAgicCtx->ProcRes.ProcResV21.regmingradthr2 = pAgicCtx->ConfigData.ConfigV21.regmingradthr2;
    pAgicCtx->ProcRes.ProcResV21.gr_ratio = pAgicCtx->ConfigData.ConfigV21.gr_ratio;
    pAgicCtx->ProcRes.ProcResV21.noise_scale = (pAgicCtx->ConfigData.ConfigV21.noise_scale * 128 + 0.5);
    pAgicCtx->ProcRes.ProcResV21.noise_base = (int)(pAgicCtx->ConfigData.ConfigV21.noise_base * 4096 + 0.5);

    for (int i = 0; i < 15; i++)
    {
        pAgicCtx->ProcRes.ProcResV21.sigma_y[i] = (int)(pAgicCtx->ConfigData.ConfigV21.sigma_y[i] * (1 << 7));
    }

    pAgicCtx->ProcRes.ProcResV21.diff_clip = pAgicCtx->ConfigData.ConfigV21.diff_clip;

    if (pAgicCtx->ProcRes.ProcResV21.regkgrad2dark < pAgicCtx->ProcRes.ProcResV21.regkgrad2)
        GIC_SWAP(int, pAgicCtx->ProcRes.ProcResV21.regkgrad2dark, pAgicCtx->ProcRes.ProcResV21.regkgrad2);

    if (pAgicCtx->ProcRes.ProcResV21.regmingradthrdark1 < pAgicCtx->ProcRes.ProcResV21.regmingradthr1)
        GIC_SWAP(int, pAgicCtx->ProcRes.ProcResV21.regmingradthrdark1, pAgicCtx->ProcRes.ProcResV21.regmingradthr1);

    if (pAgicCtx->ProcRes.ProcResV21.regmingradthrdark2 < pAgicCtx->ProcRes.ProcResV21.regmingradthr2)
        GIC_SWAP(int, pAgicCtx->ProcRes.ProcResV21.regmingradthrdark2, pAgicCtx->ProcRes.ProcResV21.regmingradthr2);

    if (pAgicCtx->ProcRes.ProcResV21.regdarktthrehi < pAgicCtx->ProcRes.ProcResV21.regdarkthre)
        GIC_SWAP(int, pAgicCtx->ProcRes.ProcResV21.regdarktthrehi, pAgicCtx->ProcRes.ProcResV21.regdarkthre);


    LOGI_AGIC("%s(%d): exit!\n", __FUNCTION__, __LINE__);

}

void AgicGetProcResultV20
(
    AgicContext_t*    pAgicCtx
)
{
    LOGI_AGIC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAgicCtx->ProcRes.gic_en = pAgicCtx->ConfigData.ConfigV20.gic_en == 0 ? false : true;
    pAgicCtx->ProcRes.ProcResV20.edge_open = pAgicCtx->ConfigData.ConfigV20.edge_open;
    pAgicCtx->ProcRes.ProcResV20.regmingradthrdark2 = pAgicCtx->ConfigData.ConfigV20.regmingradthrdark2;
    pAgicCtx->ProcRes.ProcResV20.regmingradthrdark1 = pAgicCtx->ConfigData.ConfigV20.regmingradthrdark1;
    pAgicCtx->ProcRes.ProcResV20.regminbusythre = pAgicCtx->ConfigData.ConfigV20.regminbusythre;
    pAgicCtx->ProcRes.ProcResV20.regdarkthre = pAgicCtx->ConfigData.ConfigV20.regdarkthre;
    pAgicCtx->ProcRes.ProcResV20.regmaxcorvboth = pAgicCtx->ConfigData.ConfigV20.regmaxcorvboth;
    pAgicCtx->ProcRes.ProcResV20.regdarktthrehi = pAgicCtx->ConfigData.ConfigV20.regdarktthrehi;
    pAgicCtx->ProcRes.ProcResV20.regkgrad2dark = (int)(log(double(pAgicCtx->ConfigData.ConfigV20.regkgrad2dark)) / log((double)2) + 0.5f);
    pAgicCtx->ProcRes.ProcResV20.regkgrad1dark = (int)(log(double(pAgicCtx->ConfigData.ConfigV20.regkgrad1dark)) / log((double)2) + 0.5f);

    pAgicCtx->ProcRes.ProcResV20.regstrengthglobal_fix =  (int)(pAgicCtx->ConfigData.ConfigV20.globalStrength * (1 << 7));
    if (pAgicCtx->ProcRes.ProcResV20.regstrengthglobal_fix > (1 << 7) - 1)
        pAgicCtx->ProcRes.ProcResV20.regstrengthglobal_fix = 7 + 1;
    else
        pAgicCtx->ProcRes.ProcResV20.regstrengthglobal_fix = int(log(double((1 << 7) - pAgicCtx->ProcRes.ProcResV20.regstrengthglobal_fix)) / log((double)2) + 0.5f);

    pAgicCtx->ProcRes.ProcResV20.regdarkthrestep = int(log(double(pAgicCtx->ConfigData.ConfigV20.regdarktthrehi - pAgicCtx->ConfigData.ConfigV20.regdarkthre)) / log((double)2) + 0.5f);
    pAgicCtx->ProcRes.ProcResV20.regkgrad2 = (int)(log(double(pAgicCtx->ConfigData.ConfigV20.regkgrad2)) / log((double)2) + 0.5f);
    pAgicCtx->ProcRes.ProcResV20.regkgrad1 = (int)(log(double(pAgicCtx->ConfigData.ConfigV20.regkgrad1)) / log((double)2) + 0.5f);
    pAgicCtx->ProcRes.ProcResV20.reggbthre = int(log(double(pAgicCtx->ConfigData.ConfigV20.reggbthre)) / log((double)2) + 0.5f);
    pAgicCtx->ProcRes.ProcResV20.regmaxcorv = pAgicCtx->ConfigData.ConfigV20.regmaxcorv;
    pAgicCtx->ProcRes.ProcResV20.regmingradthr1 = pAgicCtx->ConfigData.ConfigV20.regmingradthr1;
    pAgicCtx->ProcRes.ProcResV20.regmingradthr2 = pAgicCtx->ConfigData.ConfigV20.regmingradthr2;
    pAgicCtx->ProcRes.ProcResV20.gr_ratio = pAgicCtx->ConfigData.ConfigV20.gr_ratio;
    pAgicCtx->ProcRes.ProcResV20.dnloscale = (int)(pAgicCtx->ConfigData.ConfigV20.dnloscale * (1 << 7));
    pAgicCtx->ProcRes.ProcResV20.dnhiscale = (int)(pAgicCtx->ConfigData.ConfigV20.dnhiscale * (1 << 7));
    pAgicCtx->ProcRes.ProcResV20.reglumapointsstep = pAgicCtx->ConfigData.ConfigV20.reglumapointsstep;
    pAgicCtx->ProcRes.ProcResV20.gvaluelimitlo = (int)pAgicCtx->ConfigData.ConfigV20.gvaluelimitlo;
    pAgicCtx->ProcRes.ProcResV20.gvaluelimithi = (int)pAgicCtx->ConfigData.ConfigV20.gvaluelimithi;
    pAgicCtx->ProcRes.ProcResV20.fusionratiohilimt1 = (int)(pAgicCtx->ConfigData.ConfigV20.fusionratiohilimt1 * (1 << 7));
    pAgicCtx->ProcRes.ProcResV20.regstrength_fix = (int)(pAgicCtx->ConfigData.ConfigV20.textureStrength * (1 << 7));

    for (int i = 0; i < 15; i++)
    {
        pAgicCtx->ProcRes.ProcResV20.sigma_y[i] = (int)(pAgicCtx->ConfigData.ConfigV20.sigma_y[i] * (1 << 7));
    }

    pAgicCtx->ProcRes.ProcResV20.noise_cut_en = pAgicCtx->ConfigData.ConfigV20.noise_cut_en;
    pAgicCtx->ProcRes.ProcResV20.noise_coe_a = pAgicCtx->ConfigData.ConfigV20.noise_coe_a;
    pAgicCtx->ProcRes.ProcResV20.noise_coe_b = pAgicCtx->ConfigData.ConfigV20.noise_coe_b;
    pAgicCtx->ProcRes.ProcResV20.diff_clip = pAgicCtx->ConfigData.ConfigV20.diff_clip;

    if (pAgicCtx->ProcRes.ProcResV20.regkgrad2dark < pAgicCtx->ProcRes.ProcResV20.regkgrad2)
        GIC_SWAP(int, pAgicCtx->ProcRes.ProcResV20.regkgrad2dark, pAgicCtx->ProcRes.ProcResV20.regkgrad2);

    if (pAgicCtx->ProcRes.ProcResV20.regmingradthrdark1 < pAgicCtx->ProcRes.ProcResV20.regmingradthr1)
        GIC_SWAP(int, pAgicCtx->ProcRes.ProcResV20.regmingradthrdark1, pAgicCtx->ProcRes.ProcResV20.regmingradthr1);

    if (pAgicCtx->ProcRes.ProcResV20.regmingradthrdark2 < pAgicCtx->ProcRes.ProcResV20.regmingradthr2)
        GIC_SWAP(int, pAgicCtx->ProcRes.ProcResV20.regmingradthrdark2, pAgicCtx->ProcRes.ProcResV20.regmingradthr2);

    if (pAgicCtx->ProcRes.ProcResV20.regdarktthrehi < pAgicCtx->ProcRes.ProcResV20.regdarkthre)
        GIC_SWAP(int, pAgicCtx->ProcRes.ProcResV20.regdarktthrehi, pAgicCtx->ProcRes.ProcResV20.regdarkthre);


    LOGI_AGIC("%s(%d): exit!\n", __FUNCTION__, __LINE__);

}


void AgicProcessV20
(
    AgicContext_t *pAgicCtx,
    int ISO
)
{
    LOGI_AGIC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    float ave1 = 0.0f, noiseSigma = 0.0f;
    short ratio = 0;
    short LumaPoints[] = { 0, 128, 256, 384, 512, 640, 768, 896, 1024, 1536, 2048, 2560, 3072, 3584, 4096 };
    short min_busy_threHi = 0, min_busy_threLo = 0;
    short min_grad_thrHi = 0, min_grad_thrLo = 0, min_grad_thr2Hi = 0, min_grad_thr2Lo = 0;
    short k_grad1Hi = 0, k_grad1Lo = 0, k_grad2Hi = 0, k_grad2Lo = 0;
    short gb_threHi = 0, gb_threLo = 0, maxCorVHi = 0, maxCorVLo = 0;
    short maxCorVbothHi = 0, maxCorVbothLo = 0;
    short dark_threHi = 0, dark_threLo = 0, dark_threUpHi = 0, dark_threUpLo = 0;
    short k_grad1_darkHi = 0, k_grad1_darkLo = 0, k_grad2_darkHi = 0, k_grad2_darkLo = 0, min_grad_thr_dark1Hi = 0;
    short min_grad_thr_dark1Lo = 0, min_grad_thr_dark2Hi = 0, min_grad_thr_dark2Lo = 0;
    short noiseCoeaHi = 0, noiseCoeaLo = 0, noiseCoebHi = 0, noiseCoebLo = 0, diffClipHi = 0, diffClipLo = 0;
    float scaleHiHi = 0.0f, scaleHiLo = 0.0f;
    float scaleLoHi = 0.0f, scaleLoLo = 0.0f;
    float strengthHi = 0.0f, strengthLo = 0.0f;
    float strengthGlobalHi = 0.0f, strengthGlobalLo = 0.0f;
    float gValueLimitHiHi = 0.0f, gValueLimitHiLo = 0.0f;
    float gValueLimitLoHi = 0.0f, gValueLimitLoLo = 0.0f;
    int index, iso_hi = 0, iso_lo = 0;
    float pNoiseCurveGicHi_0 = 0.0f, pNoiseCurveGicHi_1 = 0.0f;
    float pNoiseCurveGicLo_0 = 0.0f, pNoiseCurveGicLo_1 = 0.0f;

    LOGI_AGIC("%s(%d): enter, ISO=%d\n", __FUNCTION__, __LINE__, ISO);

    if(pAgicCtx == NULL)
        LOGE_AGIC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);

    if(ISO <= 50) {
        index = 0;
        ratio = 0;
    } else if (ISO > 12800) {
        index = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.ISO_len - 2;
        ratio = (1 << 4);
    } else {
        for (index = 0; index < (pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.ISO_len - 2); index++)
        {
            iso_lo = (int)(pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.ISO[index]);
            iso_hi = (int)(pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.ISO[index + 1]);
            LOGD_AGIC("index=%d,  iso_lo=%d, iso_hi=%d\n", index, iso_lo, iso_hi);
            if (ISO > iso_lo && ISO <= iso_hi)
            {
                break;
            }
        }
        ratio = ((ISO - iso_lo) * (1 << 4)) / (iso_hi - iso_lo);
    }
    min_busy_threHi      = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.min_busy_thre[index + 1];
    min_busy_threLo      = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.min_busy_thre[index];
    min_grad_thrHi       = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.min_grad_thr1[index + 1];
    min_grad_thrLo       = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.min_grad_thr1[index];
    min_grad_thr2Hi      = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.min_grad_thr2[index + 1];
    min_grad_thr2Lo      = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.min_grad_thr2[index];
    k_grad1Hi            = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.k_grad1[index + 1];
    k_grad1Lo            = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.k_grad1[index];
    k_grad2Hi            = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.k_grad2[index + 1];
    k_grad2Lo            = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.k_grad2[index];
    gb_threHi            = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.gb_thre[index + 1];
    gb_threLo            = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.gb_thre[index];
    maxCorVHi            = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.maxCorV[index + 1];
    maxCorVLo            = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.maxCorV[index];
    maxCorVbothHi        = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.maxCorVboth[index + 1];
    maxCorVbothLo        = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.maxCorVboth[index];
    dark_threHi          = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.dark_thre[index + 1];
    dark_threLo          = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.dark_thre[index];
    dark_threUpHi        = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.dark_threHi[index + 1];
    dark_threUpLo        = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.dark_threHi[index];
    k_grad1_darkHi       = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.k_grad1_dark[index + 1];
    k_grad1_darkLo       = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.k_grad1_dark[index];
    k_grad2_darkHi       = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.k_grad2_dark[index + 1];
    k_grad2_darkLo       = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.k_grad2_dark[index];
    min_grad_thr_dark1Hi = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.min_grad_thr_dark1[index + 1];
    min_grad_thr_dark1Lo = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.min_grad_thr_dark1[index];
    min_grad_thr_dark2Hi = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.min_grad_thr_dark2[index + 1];
    min_grad_thr_dark2Lo = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.min_grad_thr_dark2[index];
    pNoiseCurveGicHi_0 = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.noiseCurve_0[index + 1];
    pNoiseCurveGicLo_0 = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.noiseCurve_0[index];
    pNoiseCurveGicHi_1 = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.noiseCurve_1[index + 1];
    pNoiseCurveGicLo_1 = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.noiseCurve_1[index];
    scaleHiHi            = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.ScaleHi[index + 1];
    scaleHiLo            = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.ScaleHi[index];
    scaleLoHi            = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.ScaleLo[index + 1];
    scaleLoLo            = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.ScaleLo[index];
    strengthHi           = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.textureStrength[index + 1];
    strengthLo           = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.textureStrength[index];
    strengthGlobalHi     = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.globalStrength[index + 1];
    strengthGlobalLo     = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.globalStrength[index];
    gValueLimitHiHi      = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.GValueLimitHi[index + 1];
    gValueLimitHiLo      = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.GValueLimitHi[index];
    gValueLimitLoHi      = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.GValueLimitLo[index + 1];
    gValueLimitLoLo      = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.GValueLimitLo[index];
    noiseCoeaHi          = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.noise_coea[index + 1];
    noiseCoeaLo          = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.noise_coea[index];
    noiseCoebHi          = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.noise_coeb[index + 1];
    noiseCoebLo          = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.noise_coeb[index];
    diffClipHi           = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.diff_clip[index + 1];
    diffClipLo           = pAgicCtx->full_param.gic_v20->GicTuningPara.GicData.diff_clip[index];

    float ratioF = ratio / 16.0f;
    pAgicCtx->ConfigData.ConfigV20.gic_en = pAgicCtx->full_param.gic_v20->GicTuningPara.enable ? 1 : 0;
    pAgicCtx->ConfigData.ConfigV20.edge_open = pAgicCtx->full_param.gic_v20->GicTuningPara.edge_en ? 1 : 0;
    pAgicCtx->ConfigData.ConfigV20.regmingradthrdark1 = (ratio * (min_grad_thr_dark1Hi - min_grad_thr_dark1Lo) + min_grad_thr_dark1Lo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV20.regmingradthrdark2 = (ratio * (min_grad_thr_dark2Hi - min_grad_thr_dark2Lo) + min_grad_thr_dark2Lo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV20.regminbusythre = (ratio * (min_busy_threHi - min_busy_threLo) + min_busy_threLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV20.regdarkthre = (ratio * (dark_threHi - dark_threLo) + dark_threLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV20.regmaxcorvboth = (ratio * (maxCorVbothHi - maxCorVbothLo) + maxCorVbothLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV20.regdarktthrehi = (ratio * (dark_threUpHi - dark_threUpLo) + dark_threUpLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV20.regkgrad2dark = (ratio * (k_grad2_darkHi - k_grad2_darkLo) + k_grad2_darkLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV20.regkgrad1dark = (ratio * (k_grad1_darkHi - k_grad1_darkLo) + k_grad1_darkLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV20.globalStrength = ratioF * (strengthGlobalHi - strengthGlobalLo) + strengthGlobalLo;
    pAgicCtx->ConfigData.ConfigV20.regkgrad2 = (ratio * (k_grad2Hi - k_grad2Lo) + k_grad2Lo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV20.regkgrad1 = (ratio * (k_grad1Hi - k_grad1Lo) + k_grad1Lo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV20.reggbthre = (ratio * (gb_threHi - gb_threLo) + gb_threLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV20.regmaxcorv = (ratio * (maxCorVHi - maxCorVLo) + maxCorVLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV20.regmingradthr1 = (ratio * (min_grad_thrHi - min_grad_thrLo) + min_grad_thrLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV20.regmingradthr2 = (ratio * (min_grad_thr2Hi - min_grad_thr2Lo) + min_grad_thr2Lo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV20.gr_ratio = pAgicCtx->full_param.gic_v20->GicTuningPara.gr_ration;
    pAgicCtx->ConfigData.ConfigV20.dnhiscale = ratioF * (scaleHiHi - scaleHiLo) + scaleHiLo;
    pAgicCtx->ConfigData.ConfigV20.dnloscale = ratioF * (scaleLoHi - scaleLoLo) + scaleLoLo;
    pAgicCtx->ConfigData.ConfigV20.reglumapointsstep = 7;
    pAgicCtx->ConfigData.ConfigV20.gvaluelimithi = ratioF * (gValueLimitHiHi - gValueLimitHiLo) + gValueLimitHiLo;
    pAgicCtx->ConfigData.ConfigV20.gvaluelimitlo = ratioF * (gValueLimitLoHi - gValueLimitLoLo) + gValueLimitLoLo;
    pAgicCtx->ConfigData.ConfigV20.fusionratiohilimt1 = 0.75;
    pAgicCtx->ConfigData.ConfigV20.textureStrength = ratioF * (strengthHi - strengthLo) + strengthLo;
    pAgicCtx->ConfigData.ConfigV20.noiseCurve_0 = ratioF * (pNoiseCurveGicHi_0 - pNoiseCurveGicLo_0) + pNoiseCurveGicLo_0;
    pAgicCtx->ConfigData.ConfigV20.noiseCurve_1 =  ratioF * (pNoiseCurveGicHi_1 - pNoiseCurveGicLo_1) + pNoiseCurveGicLo_1;

    for (int i = 0; i < 15; i++) {
        ave1 = LumaPoints[i];
        noiseSigma = pAgicCtx->ConfigData.ConfigV20.noiseCurve_0 * sqrt(ave1) +  pAgicCtx->ConfigData.ConfigV20.noiseCurve_1;
        if (noiseSigma < 0)
        {
            noiseSigma = 0;
        }
        pAgicCtx->ConfigData.ConfigV20.sigma_y[i] = noiseSigma;
    }

    pAgicCtx->ConfigData.ConfigV20.noise_cut_en = pAgicCtx->full_param.gic_v20->GicTuningPara.noise_cut_en ? 1 : 0;
    pAgicCtx->ConfigData.ConfigV20.noise_coe_a = (ratio * (noiseCoeaHi - noiseCoeaLo) + noiseCoeaLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV20.noise_coe_b = (ratio * (noiseCoebHi - noiseCoebLo) + noiseCoebLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV20.diff_clip = (ratio * (diffClipHi - diffClipLo) +  diffClipLo * (1 << 4) + (1 << 3)) >> 4;


    short mulBit = 0;
    int bitValue = RKAIQ_GIC_BITS;
    if(bitValue > 10)
    {
        mulBit = 1 << (bitValue - 10);
    }
    else
    {
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

    LOGI_AGIC("%s(%d): exit!\n", __FUNCTION__, __LINE__);

}

void AgicProcessV21
(
    AgicContext_t *pAgicCtx,
    int ISO
)
{
    LOGI_AGIC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    float ave1 = 0.0f, noiseSigma = 0.0f;
    short ratio = 0;
    short LumaPoints[] = { 0, 128, 256, 384, 512, 640, 768, 896, 1024, 1536, 2048, 2560, 3072, 3584, 4096 };
    short min_busy_threHi = 0, min_busy_threLo = 0;
    short min_grad_thrHi = 0, min_grad_thrLo = 0, min_grad_thr2Hi = 0, min_grad_thr2Lo = 0;
    short k_grad1Hi = 0, k_grad1Lo = 0, k_grad2Hi = 0, k_grad2Lo = 0;
    short gb_threHi = 0, gb_threLo = 0, maxCorVHi = 0, maxCorVLo = 0;
    short maxCorVbothHi = 0, maxCorVbothLo = 0;
    short dark_threHi = 0, dark_threLo = 0, dark_threUpHi = 0, dark_threUpLo = 0;
    short k_grad1_darkHi = 0, k_grad1_darkLo = 0, k_grad2_darkHi = 0, k_grad2_darkLo = 0, min_grad_thr_dark1Hi = 0;
    short min_grad_thr_dark1Lo = 0, min_grad_thr_dark2Hi = 0, min_grad_thr_dark2Lo = 0;
    short diffClipHi = 0, diffClipLo = 0;
    float strengthGlobalHi = 0.0f, strengthGlobalLo = 0.0f;
    float NoiseScaleHi = 0.0f, NoiseScaleLo = 0.0f;
    float NoiseBaseHi = 0.0f, NoiseBaseLo = 0.0f;
    int index, iso_hi = 0, iso_lo = 0;
    float pNoiseCurveGicHi_0 = 0.0f, pNoiseCurveGicHi_1 = 0.0f;
    float pNoiseCurveGicLo_0 = 0.0f, pNoiseCurveGicLo_1 = 0.0f;

    LOGI_AGIC("%s(%d): enter, ISO=%d\n", __FUNCTION__, __LINE__, ISO);

    if(pAgicCtx == NULL)
        LOGE_AGIC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);


    if(ISO <= 50) {
        index = 0;
        ratio = 0;
    } else if (ISO > 12800) {
        index = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.ISO_len - 2;
        ratio = (1 << 4);
    } else {
        for (index = 0; index < (pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.ISO_len - 1); index++)
        {
            iso_lo = (int)(pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.ISO[index]);
            iso_hi = (int)(pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.ISO[index + 1]);
            LOGD_AGIC("index=%d,  iso_lo=%d, iso_hi=%d\n", index, iso_lo, iso_hi);
            if (ISO > iso_lo && ISO <= iso_hi)
            {
                break;
            }
        }
        ratio = ((ISO - iso_lo) * (1 << 4)) / (iso_hi - iso_lo);
    }

    min_busy_threHi      = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.min_busy_thre[index + 1];
    min_busy_threLo      = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.min_busy_thre[index];
    min_grad_thrHi       = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.min_grad_thr1[index + 1];
    min_grad_thrLo       = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.min_grad_thr1[index];
    min_grad_thr2Hi      = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.min_grad_thr2[index + 1];
    min_grad_thr2Lo      = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.min_grad_thr2[index];
    k_grad1Hi            = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.k_grad1[index + 1];
    k_grad1Lo            = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.k_grad1[index];
    k_grad2Hi            = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.k_grad2[index + 1];
    k_grad2Lo            = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.k_grad2[index];
    gb_threHi            = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.gb_thre[index + 1];
    gb_threLo            = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.gb_thre[index];
    maxCorVHi            = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.maxCorV[index + 1];
    maxCorVLo            = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.maxCorV[index];
    maxCorVbothHi        = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.maxCorVboth[index + 1];
    maxCorVbothLo        = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.maxCorVboth[index];
    dark_threHi          = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.dark_thre[index + 1];
    dark_threLo          = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.dark_thre[index];
    dark_threUpHi        = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.dark_threHi[index + 1];
    dark_threUpLo        = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.dark_threHi[index];
    k_grad1_darkHi       = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.k_grad1_dark[index + 1];
    k_grad1_darkLo       = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.k_grad1_dark[index];
    k_grad2_darkHi       = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.k_grad2_dark[index + 1];
    k_grad2_darkLo       = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.k_grad2_dark[index];
    min_grad_thr_dark1Hi = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.min_grad_thr_dark1[index + 1];
    min_grad_thr_dark1Lo = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.min_grad_thr_dark1[index];
    min_grad_thr_dark2Hi = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.min_grad_thr_dark2[index + 1];
    min_grad_thr_dark2Lo = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.min_grad_thr_dark2[index];
    pNoiseCurveGicHi_0 = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.noiseCurve_0[index + 1];
    pNoiseCurveGicLo_0 = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.noiseCurve_0[index];
    pNoiseCurveGicHi_1 = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.noiseCurve_1[index + 1];
    pNoiseCurveGicLo_1 = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.noiseCurve_1[index];
    strengthGlobalHi     = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.globalStrength[index + 1];
    strengthGlobalLo     = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.globalStrength[index];
    diffClipHi           = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.diff_clip[index + 1];
    diffClipLo           = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.diff_clip[index];
    NoiseScaleHi      = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.NoiseScale[index + 1];
    NoiseScaleLo      = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.NoiseScale[index];
    NoiseBaseHi      = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.NoiseBase[index + 1];
    NoiseBaseLo      = pAgicCtx->full_param.gic_v21->GicTuningPara.GicData.NoiseBase[index];

    float ratioF = ratio / 16.0f;
    pAgicCtx->ConfigData.ConfigV21.gic_en = pAgicCtx->full_param.gic_v21->GicTuningPara.enable ? 1 : 0;
    pAgicCtx->ConfigData.ConfigV21.regmingradthrdark1 = (ratio * (min_grad_thr_dark1Hi - min_grad_thr_dark1Lo) + min_grad_thr_dark1Lo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV21.regmingradthrdark2 = (ratio * (min_grad_thr_dark2Hi - min_grad_thr_dark2Lo) + min_grad_thr_dark2Lo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV21.regminbusythre = (ratio * (min_busy_threHi - min_busy_threLo) + min_busy_threLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV21.regdarkthre = (ratio * (dark_threHi - dark_threLo) + dark_threLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV21.regmaxcorvboth = (ratio * (maxCorVbothHi - maxCorVbothLo) + maxCorVbothLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV21.regdarktthrehi = (ratio * (dark_threUpHi - dark_threUpLo) + dark_threUpLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV21.regkgrad1dark = (ratio * (k_grad1_darkHi - k_grad1_darkLo) + k_grad1_darkLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV21.regkgrad2dark = (ratio * (k_grad2_darkHi - k_grad2_darkLo) + k_grad2_darkLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV21.regkgrad1 = (ratio * (k_grad1Hi - k_grad1Lo) + k_grad1Lo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV21.regkgrad2 = (ratio * (k_grad2Hi - k_grad2Lo) + k_grad2Lo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV21.reggbthre = (ratio * (gb_threHi - gb_threLo) + gb_threLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV21.regmaxcorv = (ratio * (maxCorVHi - maxCorVLo) + maxCorVLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV21.regmingradthr1 = (ratio * (min_grad_thrHi - min_grad_thrLo) + min_grad_thrLo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV21.regmingradthr2 = (ratio * (min_grad_thr2Hi - min_grad_thr2Lo) + min_grad_thr2Lo * (1 << 4) + (1 << 3)) >> 4;
    pAgicCtx->ConfigData.ConfigV21.gr_ratio = pAgicCtx->full_param.gic_v21->GicTuningPara.gr_ration;
    pAgicCtx->ConfigData.ConfigV21.noise_scale = ratioF * (NoiseScaleHi - NoiseScaleLo) + NoiseScaleLo;
    pAgicCtx->ConfigData.ConfigV21.noise_base = ratioF * (NoiseBaseHi - NoiseBaseLo) + NoiseBaseLo;
    pAgicCtx->ConfigData.ConfigV21.noiseCurve_0 = ratioF * (pNoiseCurveGicHi_0 - pNoiseCurveGicLo_0) + pNoiseCurveGicLo_0;
    pAgicCtx->ConfigData.ConfigV21.noiseCurve_1 =  ratioF * (pNoiseCurveGicHi_1 - pNoiseCurveGicLo_1) + pNoiseCurveGicLo_1;

    for (int i = 0; i < 15; i++) {
        ave1 = LumaPoints[i];
        noiseSigma = pAgicCtx->ConfigData.ConfigV21.noiseCurve_0 * sqrt(ave1) +  pAgicCtx->ConfigData.ConfigV21.noiseCurve_1;
        if (noiseSigma < 0)
        {
            noiseSigma = 0;
        }
        pAgicCtx->ConfigData.ConfigV20.sigma_y[i] = noiseSigma;
    }

    pAgicCtx->ConfigData.ConfigV21.globalStrength = ratioF * (strengthGlobalHi - strengthGlobalLo) + strengthGlobalLo;
    pAgicCtx->ConfigData.ConfigV21.diff_clip = (ratio * (diffClipHi - diffClipLo) +  diffClipLo * (1 << 4) + (1 << 3)) >> 4;


    short mulBit = 0;
    int bitValue = RKAIQ_GIC_BITS;
    if(bitValue > 10)
    {
        mulBit = 1 << (bitValue - 10);
    }
    else
    {
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

    LOGD_AGIC("%s(%d): gic_en:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.gic_en);
    LOGD_AGIC("%s(%d): regmingradthrdark2:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.regmingradthrdark2);
    LOGD_AGIC("%s(%d): regmingradthrdark1:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.regmingradthrdark1);
    LOGD_AGIC("%s(%d): regminbusythre:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.regminbusythre);
    LOGD_AGIC("%s(%d): regdarkthre:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.regdarkthre);
    LOGD_AGIC("%s(%d): regmaxcorvboth:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.regmaxcorvboth);
    LOGD_AGIC("%s(%d): regdarktthrehi:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.regdarktthrehi);
    LOGD_AGIC("%s(%d): regkgrad2dark:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.regkgrad2dark);
    LOGD_AGIC("%s(%d): regkgrad1dark:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.regkgrad1dark);
    LOGD_AGIC("%s(%d): regkgrad2:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.regkgrad2);
    LOGD_AGIC("%s(%d): regkgrad1:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.regkgrad1);
    LOGD_AGIC("%s(%d): reggbthre:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.reggbthre);
    LOGD_AGIC("%s(%d): regmaxcorv:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.regmaxcorv);
    LOGD_AGIC("%s(%d): regmingradthr2:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.regmingradthr2);
    LOGD_AGIC("%s(%d): regmingradthr1:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.regmingradthr1);
    LOGD_AGIC("%s(%d): gr_ratio:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.gr_ratio);
    LOGD_AGIC("%s(%d): noise_scale:%f\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.noise_scale);
    LOGD_AGIC("%s(%d): noise_base:%f\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.noise_base);
    LOGD_AGIC("%s(%d): globalStrength:%f\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.globalStrength);
    LOGD_AGIC("%s(%d): diff_clip:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.diff_clip);
    LOGD_AGIC("%s(%d): sigma_y[0]:%d ", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.sigma_y[0]);
    for(int i = 1; i < 7; i++)
        LOGD_AGIC("%s(%d): sigma_y[%d]:%d ", __FUNCTION__, __LINE__, i, pAgicCtx->ConfigData.ConfigV21.sigma_y[i]);
    LOGD_AGIC("%s(%d): sigma_y[7]:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.sigma_y[7]);
    LOGD_AGIC("%s(%d): sigma_y[8]:%d ", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.sigma_y[8]);
    for(int i = 9; i < 14; i++)
        LOGD_AGIC("%s(%d): sigma_y[%d]:%d ", __FUNCTION__, __LINE__, i, pAgicCtx->ConfigData.ConfigV21.sigma_y[i]);
    LOGD_AGIC("%s(%d): sigma_y[14]:%d\n", __FUNCTION__, __LINE__, pAgicCtx->ConfigData.ConfigV21.sigma_y[14]);

    LOGI_AGIC("%s(%d): exit!\n", __FUNCTION__, __LINE__);

}

XCamReturn
AgicProcess
(
    AgicContext_t *pAgicCtx,
    int ISO,
    int mode
)
{
    LOGI_AGIC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(CHECK_ISP_HW_V20())
        AgicProcessV20(pAgicCtx, ISO);
    else if(CHECK_ISP_HW_V21())
        AgicProcessV21(pAgicCtx, ISO);
    else
        LOGE_AGIC(" %s:Wrong hardware version!! \n", __func__);

    LOGI_AGIC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;

}

XCamReturn
AgicGetProcResult
(
    AgicContext_t*    pAgicCtx
)
{
    LOGI_AGIC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAgicCtx == NULL) {
        LOGE_AGIC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if(CHECK_ISP_HW_V20())
        AgicGetProcResultV20(pAgicCtx);
    else if(CHECK_ISP_HW_V21())
        AgicGetProcResultV21(pAgicCtx);
    else
        LOGE_AGIC(" %s:Wrong hardware version!! \n", __func__);

    LOGI_AGIC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;
}



