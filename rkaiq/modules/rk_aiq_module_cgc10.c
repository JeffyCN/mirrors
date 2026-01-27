#include "rk_aiq_isp39_modules.h"

void rk_aiq_cgc10_params_cvt(void* attr, isp_params_t* isp_params)
{
    struct isp21_cgc_cfg* pFix = &isp_params->isp_cfg->others.cgc_cfg;
    cgc_param_t *cgc_param = (cgc_param_t *) attr;
    cgc_params_static_t * psta = &cgc_param->sta;

    pFix->ratio_en = psta->cgc_ratio_en;
    pFix->yuv_limit = psta->cgc_yuv_limit;
    return;
}
