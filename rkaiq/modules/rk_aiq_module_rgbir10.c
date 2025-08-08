#include "rk_aiq_isp39_modules.h"
#define BIT_MIN               (0)
#define BIT_3_MAX             (7)
#define BIT_4_MAX             (15)
#define BIT_8_MAX             (255)
#define BIT_9_MAX             (511)
#define BIT_10_MAX            (1023)
#define BIT_11_MAX            (2047)
#define BIT_12_MAX            (4095)
#define BIT_13_MAX            (8191)
#define BIT_14_MAX            (16383)

void rk_aiq_rgbir10_params_cvt(void* attr, isp_params_t* isp_params)
{
    struct isp39_rgbir_cfg* pFix = &isp_params->isp_cfg->others.rgbir_cfg;
    rgbir_param_t *rgbir_param = (rgbir_param_t *) attr;
    rgbir_params_dyn_t* pdyn = &rgbir_param->dyn;

    pFix->coe_delta = CLIP(pdyn->hw_rgbir_remosaic_edge_coef, BIT_MIN + 1, BIT_12_MAX);
    pFix->coe_theta = CLIP(pdyn->hw_rgbir_remosaic_ir_blk_level, BIT_MIN, BIT_14_MAX);
    pFix->scale[0] = CLIP(pdyn->sw_rgbir_remosaic_r_scale_coef * 128, BIT_MIN + 1, BIT_8_MAX + 1);
    pFix->scale[1] = CLIP(pdyn->sw_rgbir_remosaic_gr_scale_coef * 128, BIT_MIN + 1, BIT_8_MAX + 1);
    pFix->scale[2] = CLIP(pdyn->sw_rgbir_remosaic_gb_scale_coef * 128, BIT_MIN + 1, BIT_8_MAX + 1);
    pFix->scale[3] = CLIP(pdyn->sw_rgbir_remosaic_b_scale_coef * 128, BIT_MIN + 1, BIT_8_MAX + 1);
    for (int i = 0; i < RGBIR_LUM2SCALE_TABLE_LEN - 1; i++) {
        pFix->luma_point[i] =
            CLIP(pdyn->hw_rgbir_remosaic_lum2scale_idx[i], BIT_MIN, BIT_10_MAX);
    }
    pFix->luma_point[RGBIR_LUM2SCALE_TABLE_LEN - 1] =
        CLIP(pdyn->hw_rgbir_remosaic_lum2scale_idx[RGBIR_LUM2SCALE_TABLE_LEN - 1], BIT_MIN, BIT_10_MAX + 1);
    for (int i = 0; i < RGBIR_LUM2SCALE_TABLE_LEN; i++) {
        pFix->scale_map[i] =
            CLIP(pdyn->hw_rgbir_remosaic_lum2scale_val[i], BIT_MIN, BIT_8_MAX + 1);
    }

    LOGD_ARGBIR(
        "%s: edge_aware_coef:%d ir_black_level:%d b_scale_coef:%d gb_scale_coef:%d "
        "gr_scale_coef:%d r_scale_coef:%d\n",
        __FUNCTION__, isp_params->isp_cfg->others.rgbir_cfg.coe_delta, isp_params->isp_cfg->others.rgbir_cfg.coe_theta,
        isp_params->isp_cfg->others.rgbir_cfg.scale[3], isp_params->isp_cfg->others.rgbir_cfg.scale[2],
        isp_params->isp_cfg->others.rgbir_cfg.scale[1], isp_params->isp_cfg->others.rgbir_cfg.scale[0]);
    LOGD_ARGBIR("%s: luma_point: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__, isp_params->isp_cfg->others.rgbir_cfg.luma_point[0],
                isp_params->isp_cfg->others.rgbir_cfg.luma_point[1], isp_params->isp_cfg->others.rgbir_cfg.luma_point[2],
                isp_params->isp_cfg->others.rgbir_cfg.luma_point[3], isp_params->isp_cfg->others.rgbir_cfg.luma_point[4],
                isp_params->isp_cfg->others.rgbir_cfg.luma_point[5], isp_params->isp_cfg->others.rgbir_cfg.luma_point[6],
                isp_params->isp_cfg->others.rgbir_cfg.luma_point[7], isp_params->isp_cfg->others.rgbir_cfg.luma_point[8],
                isp_params->isp_cfg->others.rgbir_cfg.luma_point[9], isp_params->isp_cfg->others.rgbir_cfg.luma_point[10],
                isp_params->isp_cfg->others.rgbir_cfg.luma_point[11], isp_params->isp_cfg->others.rgbir_cfg.luma_point[12],
                isp_params->isp_cfg->others.rgbir_cfg.luma_point[13], isp_params->isp_cfg->others.rgbir_cfg.luma_point[14],
                isp_params->isp_cfg->others.rgbir_cfg.luma_point[15], isp_params->isp_cfg->others.rgbir_cfg.luma_point[16]);
    LOGD_ARGBIR("%s: scale_map: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__,
                isp_params->isp_cfg->others.rgbir_cfg.scale_map[0], isp_params->isp_cfg->others.rgbir_cfg.scale_map[1],
                isp_params->isp_cfg->others.rgbir_cfg.scale_map[2], isp_params->isp_cfg->others.rgbir_cfg.scale_map[3],
                isp_params->isp_cfg->others.rgbir_cfg.scale_map[4], isp_params->isp_cfg->others.rgbir_cfg.scale_map[5],
                isp_params->isp_cfg->others.rgbir_cfg.scale_map[6], isp_params->isp_cfg->others.rgbir_cfg.scale_map[7],
                isp_params->isp_cfg->others.rgbir_cfg.scale_map[8], isp_params->isp_cfg->others.rgbir_cfg.scale_map[9],
                isp_params->isp_cfg->others.rgbir_cfg.scale_map[10], isp_params->isp_cfg->others.rgbir_cfg.scale_map[11],
                isp_params->isp_cfg->others.rgbir_cfg.scale_map[12], isp_params->isp_cfg->others.rgbir_cfg.scale_map[13],
                isp_params->isp_cfg->others.rgbir_cfg.scale_map[14], isp_params->isp_cfg->others.rgbir_cfg.scale_map[15],
                isp_params->isp_cfg->others.rgbir_cfg.scale_map[16]);
    return;
}
