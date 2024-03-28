#include "rk_aiq_isp32_modules.h"

void rk_aiq_gamma21_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg)
{
    struct isp3x_gammaout_cfg *phwcfg = &isp_cfg->others.gammaout_cfg;
    gamma_param_t *gamma21_attrib = (gamma_param_t*) attr;
    gamma_params_static_t* psta = &gamma21_attrib->sta;

    phwcfg->offset                    = psta->Gamma_out_offset;
    phwcfg->finalx4_dense_en          = psta->EnableDot49 ? 1 : 0;
    phwcfg->equ_segm                  = psta->equ_segm;
    for (int i = 0; i < 49; i++)
        phwcfg->gamma_y[i] = CLIP(psta->Gamma_curve[i], 0, 4095);

    return;
}
