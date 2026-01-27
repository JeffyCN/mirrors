#include "rk_aiq_isp39_modules.h"

void rk_aiq_cp10_params_cvt(void* attr, isp_params_t* isp_params)
{
    struct isp2x_cproc_cfg* pFix = &isp_params->isp_cfg->others.cproc_cfg;
    cp_param_t *cp_param = (cp_param_t *) attr;
    cp_params_static_t* psta = &cp_param->sta;

    pFix->contrast = (uint8_t)(psta->contrast);
    pFix->sat = (uint8_t)(psta->saturation);
    pFix->brightness = (uint8_t)(psta->brightness - 128);
    pFix->hue = (uint8_t)(psta->hue - 128);

    return;
}
