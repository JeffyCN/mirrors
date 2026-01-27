#include "rk_aiq_isp39_modules.h"

#define ISP3X_SEGNUM_LOG_49     (2)
#define ISP3X_SEGNUM_LOG_45     (0)
#define ISP3X_SEGNUM_EQU_45     (1)

void rk_aiq_gamma21_params_dump(void* attr, isp_params_t* isp_params) {
    LOG1_AGAMMA("%s: offset:%d \n", __FUNCTION__, isp_params->isp_cfg->others.gammaout_cfg.offset);
    LOG1_AGAMMA("%s: gamma_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
              __FUNCTION__, isp_params->isp_cfg->others.gammaout_cfg.gamma_y[0], isp_params->isp_cfg->others.gammaout_cfg.gamma_y[1],
              isp_params->isp_cfg->others.gammaout_cfg.gamma_y[2], isp_params->isp_cfg->others.gammaout_cfg.gamma_y[3],
              isp_params->isp_cfg->others.gammaout_cfg.gamma_y[4], isp_params->isp_cfg->others.gammaout_cfg.gamma_y[5],
              isp_params->isp_cfg->others.gammaout_cfg.gamma_y[6], isp_params->isp_cfg->others.gammaout_cfg.gamma_y[7],
              isp_params->isp_cfg->others.gammaout_cfg.gamma_y[8], isp_params->isp_cfg->others.gammaout_cfg.gamma_y[9],
              isp_params->isp_cfg->others.gammaout_cfg.gamma_y[10], isp_params->isp_cfg->others.gammaout_cfg.gamma_y[11],
              isp_params->isp_cfg->others.gammaout_cfg.gamma_y[12], isp_params->isp_cfg->others.gammaout_cfg.gamma_y[13],
              isp_params->isp_cfg->others.gammaout_cfg.gamma_y[14], isp_params->isp_cfg->others.gammaout_cfg.gamma_y[15],
              isp_params->isp_cfg->others.gammaout_cfg.gamma_y[16]);
}

void rk_aiq_gamma21_params_cvt(void* attr, isp_params_t* isp_params)
{
    struct isp3x_gammaout_cfg *phwcfg = &isp_params->isp_cfg->others.gammaout_cfg;
    gamma_param_t *gamma_param = (gamma_param_t*) attr;
    gamma_params_dyn_t* pdyn          = &gamma_param->dyn;

    int gamma_out_segnum = ISP3X_SEGNUM_LOG_49;

    int8_t equ_segm = gamma_out_segnum > 1 ? ISP3X_SEGNUM_LOG_45: gamma_out_segnum;
    bool EnableDot49 = gamma_out_segnum == ISP3X_SEGNUM_LOG_49 ? true : false;
    phwcfg->offset                    = CLIP(pdyn->hw_gammaT_outCurve_offset, 0, 4095);
    phwcfg->finalx4_dense_en          = EnableDot49 ? 1 : 0;
    phwcfg->equ_segm                  = equ_segm;
    for (int i = 0; i < 49; i++)
        phwcfg->gamma_y[i] = CLIP(pdyn->hw_gammaT_outCurve_val[i], 0, 4095);

    rk_aiq_gamma21_params_dump(attr, isp_params);
    return;
}
