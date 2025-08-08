#include "rk_aiq_isp39_modules.h"
#include <math.h>

void rk_aiq_gic21_params_cvt(void* attr, struct isp39_gic_cfg* gic_cfg)
{
    int i, tmp;
    struct isp39_gic_cfg *pFix = gic_cfg;
    gic_param_t *gic_param = (gic_param_t *) attr;
    gic_params_dyn_t* pdyn = &gic_param->dyn;

    short mulBit; //?????gic_params;dat??10bit raw gic????????
    short bitValue = 12;
    if (bitValue > 10)  {
        mulBit = 1 << (bitValue - 10);
    }   else    {
        mulBit = 1;
    }

    pFix->regmingradthr1 = pdyn->strongEdg_lightArea.hw_gicT_hiEdgTh_minLimit * (1 << mulBit);
    pFix->regmingradthr2 = pdyn->strongEdg_lightArea.hw_gicT_loEdgTh_minLimit * (1 << mulBit);
    pFix->regmingradthrdark1 = pdyn->strongEdg_darkArea.hw_gicT_hiEdgTh_minLimit * (1 << mulBit);
    pFix->regmingradthrdark2 = pdyn->strongEdg_darkArea.hw_gicT_loEdgTh_minLimit * (1 << mulBit);
    pFix->regminbusythre = pdyn->textDct.hw_gicT_textTh_minLimit * (1 << mulBit);

    pFix->regdarkthre = pdyn->strongEdg_darkArea.hw_gicT_darkArea_minThred * (1 << mulBit);
    pFix->regdarktthrehi = pdyn->strongEdg_darkArea.hw_gicT_darkArea_maxThred * (1 << mulBit);
    //pFix->regmaxcorvboth = pdyn->flatDct;

    pFix->regkgrad1 =
        (int)(log((double)(pdyn->strongEdg_lightArea.hw_gicT_hiGrad2EdgTh_ratio * (1 << 1))) / log((double)2) + 0.5f);
    pFix->regkgrad2 =
        (int)(log((double)(pdyn->strongEdg_lightArea.hw_gicT_loGrad2EdgTh_ratio * (1 << 1))) / log((double)2) + 0.5f);
    pFix->regkgrad1dark =
        (int)(log((double)(pdyn->strongEdg_darkArea.hw_gicT_hiGrad2EdgTh_ratio * (1 << 1))) / log((double)2) + 0.5f);
    pFix->regkgrad2dark =
        (int)(log((double)(pdyn->strongEdg_darkArea.hw_gicT_loGrad2EdgTh_ratio * (1 << 1))) / log((double)2) + 0.5f);

    pFix->regstrengthglobal_fix =
        (int)(pdyn->gicProc.hw_gicT_correctGb_strg * (1 << 7));
    if (pFix->regstrengthglobal_fix > (1 << 7) - 1)
        pFix->regstrengthglobal_fix = 7 + 1;
    else
        pFix->regstrengthglobal_fix =
            (int)(log((double)((1 << 7) - pFix->regstrengthglobal_fix)) / log((double)2) + 0.5f);

    pFix->regdarkthrestep =
        (int)(log((double)(pFix->regdarktthrehi - pFix->regdarkthre) * (1 << mulBit)) / log((double)2) + 0.5f);

    pFix->gr_ratio = pdyn->gicProc.hw_gicT_correctGb2Gr_ratio;
    pFix->noise_scale = pdyn->gicProc.hw_gicT_gDiffSoftThd_scale ;
    pFix->noise_base = pdyn->weakEdg.hw_gicT_estNs_offset * (1 << mulBit);
    pFix->diff_clip = pdyn->gicProc.hw_gicT_gDiff_maxLimit;

    float ave1 = 0.0f, noiseSigma = 0.0f;
    short LumaPoints[] = {0,    128,  256,  384,  512,  640,  768, 896,
                          1024, 1536, 2048, 2560, 3072, 3584, 4096
                         };

    for (int i = 0; i < 15; i++) {
        ave1       = LumaPoints[i];
        noiseSigma = pdyn->gicProc.hw_gicT_sigma_scale * sqrt(ave1) +
                     pdyn->gicProc.hw_gicT_sigma_offset;
        if (noiseSigma < 0) {
            noiseSigma = 0;
        }
        pFix->sigma_y[i] = noiseSigma * (1 << 7) ;
    }
    return;
}
