#include "rk_aiq_isp39_modules.h"

void rk_aiq_yme10_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo)
{
    int i;
    struct isp39_yuvme_cfg* pFix = &isp_params->isp_cfg->others.yuvme_cfg;
    yme_param_t *yme_param = (yme_param_t *) attr;
    yme_params_dyn_t* pdyn = &yme_param->dyn;
    yme_params_static_t * psta = &yme_param->sta;

    float fStrength = 1.0;

    // YUVME_2700_GLOBAL_CTRL (0x0000)
    pFix->tnr_wgt0_en = !psta->hw_yuvme_btnrMap_en;

    /* YUVME_PARA0 */
    pFix->global_nr_strg =  CLIP(ROUND_I32(pdyn->sw_yuvme_globalNr_strg * 256), 0, 0xff);

    float tmp = 0.0f;
    tmp = (pFix->global_nr_strg  * 0.01 * 1023 * 1023 / 64);
    if(tmp == 0.0f ) {
        pFix->wgt_fact3 = 0xff;
    } else {
        pFix->wgt_fact3 = CLIP(ROUND_I32(32768 / tmp), 0, 0xff);
    }

    pFix->search_range_mode = CLIP(ROUND_I32(pdyn->hw_yuvme_searchRange_mode), 0, 2);

    pFix->static_detect_thred = CLIP(ROUND_I32(pdyn->hw_yuvme_staticDetect_thred), 0, 63);

    /* YUVME_PARA1 */
    pFix->time_relevance_offset = CLIP(ROUND_I32(pdyn->hw_yuvme_timeRelevance_offset), 0, 15);

    pFix->space_relevance_offset = CLIP(ROUND_I32(pdyn->hw_yuvme_spaceRelevance_offset), 0, 15);
    pFix->nr_diff_scale = CLIP(ROUND_I32(pdyn->sw_yuvme_nrDiff_scale * 16), 1, 255);

    pFix->nr_fusion_limit = 1023 - CLIP(ROUND_I32(pdyn->sw_yuvme_nrFusion_limit * 1024), 0, 1023);
    /* YUVME_PARA2 */
    pFix->nr_static_scale = CLIP(ROUND_I32(pdyn->sw_yuvme_nrStatic_scale * 16), 1, 255);
    pFix->nr_motion_scale = CLIP(ROUND_I32(pdyn->sw_yuvme_nrMotion_scale * 256), 0, 511);

    /* YUVME_SIGMA */
    for (int i = 0; i < 16; i++)
    {
        pFix->nr_luma2sigma_val[i] = CLIP(pdyn->hw_yuvme_nrLuma2Sigma_val[i], 0, 1023);
    }

    pFix->nr_fusion_mode = CLIP(ROUND_I32(pdyn->hw_yuvme_nrFusion_mode), 0, 3);
    pFix->cur_weight_limit = CLIP(ROUND_I32(pdyn->sw_yuvme_curWeight_limit * 2048), 0, 2047);

    return;
}
