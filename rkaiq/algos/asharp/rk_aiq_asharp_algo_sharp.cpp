
#include "rk_aiq_asharp_algo_sharp.h"

RKAIQ_BEGIN_DECLARE

#define MAX_SHARP_LUMA_CLIP_VALUE (8.0)

void sharp_filter_merge(float *src0, float *src1, float* dst, int size, float alpha)
{
    for(int i = 0; i < size; i++)
    {
        dst[i] = src0[i] * alpha + src1[i] * (1 - alpha);
        LOGD_ANR("sharp filter_merge idx[%d]; src1:%f src2:%f alpha:%f dst:%f\n",
                 i, src0[i], src1[i], alpha, dst[i]);
    }
}

AsharpResult_t sharp_get_mode_cell_idx_by_name_v1(CalibDb_Sharp_2_t *pCalibdb, char *name, int *mode_idx)
{
    int i = 0;
    AsharpResult_t res = ASHARP_RET_SUCCESS;

    if(pCalibdb == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(name == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(mode_idx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(pCalibdb->mode_num < 1) {
        LOGE_ASHARP("%s(%d): sharp mode cell num is zero\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    for(i = 0; i < pCalibdb->mode_num; i++) {
        if(strncmp(name, pCalibdb->mode_cell[i].name, sizeof(pCalibdb->mode_cell[i].name)) == 0) {
            break;
        }
    }

    if(i < pCalibdb->mode_num) {
        *mode_idx = i;
        res = ASHARP_RET_SUCCESS;
    } else {
        *mode_idx = 0;
        res = ASHARP_RET_FAILURE;
    }

    LOGD_ASHARP("%s:%d mode_name:%s  mode_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, * mode_idx, i);
    return res;

}

AsharpResult_t sharp_get_setting_idx_by_name_v1(CalibDb_Sharp_2_t *pCalibdb, char *name, int mode_idx, int *setting_idx)
{
    int i = 0;
    AsharpResult_t res = ASHARP_RET_SUCCESS;

    if(pCalibdb == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(name == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(setting_idx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    for(i = 0; i < CALIBDB_NR_SHARP_SETTING_LEVEL; i++) {
        if(strncmp(name, pCalibdb->mode_cell[mode_idx].setting[i].snr_mode, sizeof(pCalibdb->mode_cell[mode_idx].setting[i].snr_mode)) == 0) {
            break;
        }
    }

    if(i < CALIBDB_NR_SHARP_SETTING_LEVEL) {
        *setting_idx = i;
        res = ASHARP_RET_SUCCESS;
    } else {
        *setting_idx = 0;
        res = ASHARP_RET_FAILURE;
    }

    LOGD_ASHARP("%s:%d snr_name:%s  snr_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, * setting_idx, i);
    return res;

}

AsharpResult_t sharp_config_setting_param_v1(RKAsharp_Sharp_HW_Params_t *pParams, CalibDb_Sharp_2_t *pCalibdb, char *param_mode, char* snr_name)
{
    AsharpResult_t res = ASHARP_RET_SUCCESS;
    int mode_idx = 0;
    int setting_idx = 0;

    if(pParams == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(pCalibdb == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }


    res = sharp_get_mode_cell_idx_by_name_v1(pCalibdb, param_mode, &mode_idx);
    if(res != ASHARP_RET_SUCCESS) {
        LOGE_ASHARP("%s(%d): error!!!  can't find mode name in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = sharp_get_setting_idx_by_name_v1(pCalibdb, snr_name, mode_idx, &setting_idx);
    if(res != ASHARP_RET_SUCCESS) {
        LOGE_ASHARP("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = init_sharp_params_v1(pParams, pCalibdb, mode_idx, setting_idx);

    LOGD_ASHARP("%s(%d): finnal mode:%d snr_mode:%d \n", __FUNCTION__, __LINE__, mode_idx, setting_idx);
    return res;

}
AsharpResult_t init_sharp_params_v1(RKAsharp_Sharp_HW_Params_t *pParams, CalibDb_Sharp_2_t *pCalibdb, int mode_idx, int setting_idx)
{
    AsharpResult_t res = ASHARP_RET_SUCCESS;
    int i = 0;
    int j = 0;
    int max_iso_step = MAX_ISO_STEP;

    if(pParams == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(pCalibdb == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    CalibDb_Sharp_Setting_t *pSetting = &pCalibdb->mode_cell[mode_idx].setting[setting_idx];
    for (i = 0; i < max_iso_step; i++) {
#ifndef RK_SIMULATOR_HW
        pParams->iso[i] = pSetting->sharp_iso[i].iso;
#endif
        pParams->lratio[i] = pSetting->sharp_iso[i].lratio;
        pParams->hratio[i] = pSetting->sharp_iso[i].hratio;
        pParams->M_ratio[i] = pSetting->sharp_iso[i].mf_sharp_ratio;
        pParams->H_ratio[i] = pSetting->sharp_iso[i].hf_sharp_ratio;
    }

    for(int j = 0; j < RK_EDGEFILTER_LUMA_POINT_NUM; j++) {
        pParams->luma_point[j] = pCalibdb->luma_point[j];
    }

    for (i = 0; i < max_iso_step; i++) {
        for(int j = 0; j < RK_EDGEFILTER_LUMA_POINT_NUM; j++) {
            pParams->luma_sigma[i][j] = pSetting->sharp_iso[i].luma_sigma[j];
        }
        pParams->pbf_gain[i] = pSetting->sharp_iso[i].pbf_gain;
        pParams->pbf_ratio[i] = pSetting->sharp_iso[i].pbf_ratio;
        pParams->pbf_add[i] = pSetting->sharp_iso[i].pbf_add;

        for(int j = 0; j < RK_EDGEFILTER_LUMA_POINT_NUM; j++) {
            pParams->lum_clp_m[i][j] = pSetting->sharp_iso[i].mf_clip_pos[j];
        }

        for(int j = 0; j < RK_EDGEFILTER_LUMA_POINT_NUM; j++) {
            pParams->lum_min_m[i][j] = pSetting->sharp_iso[i].mf_clip_neg[j];
        }

        for(int j = 0; j < RK_SHARPFILTER_LUMA_POINT_NUM; j++) {
            pParams->lum_clp_h[i][j] = pSetting->sharp_iso[i].hf_clip[j];
        }

        pParams->mbf_gain[i] = pSetting->sharp_iso[i].mbf_gain;
        pParams->hbf_gain[i] = pSetting->sharp_iso[i].hbf_gain;
        pParams->hbf_ratio[i] = pSetting->sharp_iso[i].hbf_ratio;
        pParams->mbf_add[i] = pSetting->sharp_iso[i].mbf_add;
        pParams->hbf_add[i] = pSetting->sharp_iso[i].hbf_add;
        pParams->ehf_th[i] = pSetting->sharp_iso[i].local_sharp_strength;
        pParams->pbf_coeff_percent[i] = pSetting->sharp_iso[i].pbf_coeff_percent;
        pParams->rf_m_coeff_percent[i] = pSetting->sharp_iso[i].rf_m_coeff_percent;
        pParams->rf_h_coeff_percent[i] = pSetting->sharp_iso[i].rf_h_coeff_percent;
        pParams->hbf_coeff_percent[i] = pSetting->sharp_iso[i].hbf_coeff_percent;
    }

    //////////////////////////////////////////////////////////////////////////
    // init filter params
    // Gaussian filter params
    float gaus_luma_coeff[RKSHAPRENHW_PBF_DIAM * RKSHAPRENHW_PBF_DIAM] =
    {
        0.0625, 0.125, 0.0625,
        0.125, 0.25, 0.125,
        0.0625, 0.125, 0.0625
    };
    // pre bf params
    float pbf_coeff[RKSHAPRENHW_PBF_DIAM * RKSHAPRENHW_PBF_DIAM] =
    {
        0.156250, 0.25, 0.156250,
        0.25, 0.375000, 0.25,
        0.156250, 0.25, 0.156250
    };
    // mf range filter params
    float rf_m_coeff[RKSHAPRENHW_MRF_DIAM * RKSHAPRENHW_MRF_DIAM] =
    {
        0.023438, 0.031250, 0.039063, 0.031250, 0.023438,
        0.031250, 0.046875, 0.054688, 0.046875, 0.031250,
        0.039063, 0.054688, 0.093750, 0.054688, 0.039063,
        0.031250, 0.046875, 0.054688, 0.046875, 0.031250,
        0.023438, 0.031250, 0.039063, 0.031250, 0.023438

    };
    // mf bf params
    float mbf_coeff[RKSHAPRENHW_MBF_DIAM_Y * RKSHAPRENHW_MBF_DIAM_X] =
    {
        0.0, 0.0, 0.0, 0.0, 0.406250, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.406250, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.343750, 0.0, 0.328125, 0.0, 0.343750, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.406250, 0.0, 0.359375, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.359375, 0.0, 0.406250, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.296875, 0.0, 0.234375, 0.0, 0.234375, 0.0, 0.296875, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.406250, 0.0, 0.343750, 0.0, 0.0, 0.0, 0.0, 0.171875, 0.0, 0.0, 0.0, 0.0, 0.343750, 0.0, 0.406250, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.234375, 0.0, 0.140625, 0.109375, 0.140625, 0.0, 0.234375, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.437500, 0.0, 0.0, 0.328125, 0.0, 0.0, 0.171875, 0.109375, 0.0, 0.109375, 0.171875, 0.0, 0.0, 0.328125, 0.0, 0.0, 0.437500,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.234375, 0.0, 0.140625, 0.109375, 0.140625, 0.0, 0.234375, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.406250, 0.0, 0.343750, 0.0, 0.0, 0.0, 0.0, 0.171875, 0.0, 0.0, 0.0, 0.0, 0.343750, 0.0, 0.406250, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.296875, 0.0, 0.234375, 0.0, 0.234375, 0.0, 0.296875, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.406250, 0.0, 0.359375, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.359375, 0.0, 0.406250, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.343750, 0.0, 0.328125, 0.0, 0.343750, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.406250, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.406250, 0.0, 0.0, 0.0, 0.0
    };
    // hf range filter params
    float rf_h_coeff[RKSHAPRENHW_HRF_DIAM * RKSHAPRENHW_HRF_DIAM] =
    {
        0.0, 0.015625, 0.023438, 0.015625, 0.0,
        0.015625, 0.062500, 0.101563, 0.062500, 0.015625,
        0.023438, 0.101563, 0.125000, 0.101563, 0.023438,
        0.015625, 0.062500, 0.101563, 0.062500, 0.015625,
        0.0, 0.015625, 0.023438, 0.015625, 0.0
    };
    // hf bf params
    float hbf_coeff[RKSHAPRENHW_HBF_DIAM * RKSHAPRENHW_HBF_DIAM] =
    {
        0.156250, 0.25, 0.156250,
        0.25, 0.375000, 0.25,
        0.156250, 0.25, 0.156250
    };

    float* p_gaus_luma_kernel = gaus_luma_coeff;
#ifndef RK_SIMULATOR_HW
    if(pCalibdb->mode_cell[mode_idx].gauss_luma_coeff[RKSHAPRENHW_GAU_DIAM * RKSHAPRENHW_GAU_DIAM / 2] != 0) {
        p_gaus_luma_kernel = pCalibdb->mode_cell[mode_idx].gauss_luma_coeff;
    }
#endif

    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_GAU_DIAM;
        int w = RKSHAPRENHW_GAU_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++) {
                pParams->gaus_luma_kernel[i][m * w + n] = p_gaus_luma_kernel[m * w + n];
            }
        }
    }

    float* p_kernel_pbf_l = pbf_coeff;
    float* p_kernel_pbf_h = pbf_coeff;
#if 0
    float* p_kernel_pbf = pbf_coeff;
    if(pCalibdb->mode_cell[mode_idx].pbf_coeff[RKSHAPRENHW_PBF_DIAM * RKSHAPRENHW_PBF_DIAM / 2] != 0) {
        p_kernel_pbf = pCalibdb->mode_cell[mode_idx].pbf_coeff;
    }

    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_PBF_DIAM;
        int w = RKSHAPRENHW_PBF_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++) {
                pParams->kernel_pbf[i][m * w + n] = p_kernel_pbf[m * w + n];
            }
        }
    }
#endif

#ifndef RK_SIMULATOR_HW
    if(pCalibdb->mode_cell[mode_idx].pbf_coeff_l[RKSHAPRENHW_PBF_DIAM * RKSHAPRENHW_PBF_DIAM / 2] != 0
            && pCalibdb->mode_cell[mode_idx].pbf_coeff_h[RKSHAPRENHW_PBF_DIAM * RKSHAPRENHW_PBF_DIAM / 2] != 0) {
        p_kernel_pbf_l = pCalibdb->mode_cell[mode_idx].pbf_coeff_l;
        p_kernel_pbf_h = pCalibdb->mode_cell[mode_idx].pbf_coeff_h;
    }
#endif
    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_PBF_DIAM;
        int w = RKSHAPRENHW_PBF_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++) {
                pParams->kernel_pbf_l[i][m * w + n] = p_kernel_pbf_l[m * w + n];
                pParams->kernel_pbf_h[i][m * w + n] = p_kernel_pbf_h[m * w + n];
            }
        }
    }

#if 0
    float* p_h_rf_m = rf_m_coeff;
#ifndef RK_SIMULATOR_HW
    if(pCalibdb->mode_cell[mode_idx].rf_m_coeff[RKSHAPRENHW_MRF_DIAM * RKSHAPRENHW_MRF_DIAM / 2] != 0) {
        p_h_rf_m = pCalibdb->mode_cell[mode_idx].rf_m_coeff;
    }
#endif
    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_MRF_DIAM;
        int w = RKSHAPRENHW_MRF_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++)
                pParams->h_rf_m[i][m * w + n] = p_h_rf_m[m * w + n];
        }
    }
#else
    float* p_h_rf_m_l = rf_m_coeff;
    float* p_h_rf_m_h = rf_m_coeff;
#ifndef RK_SIMULATOR_HW
    if(pCalibdb->mode_cell[mode_idx].rf_m_coeff_l[RKSHAPRENHW_MRF_DIAM * RKSHAPRENHW_MRF_DIAM / 2] != 0
            && pCalibdb->mode_cell[mode_idx].rf_m_coeff_h[RKSHAPRENHW_MRF_DIAM * RKSHAPRENHW_MRF_DIAM / 2] != 0) {
        p_h_rf_m_l = pCalibdb->mode_cell[mode_idx].rf_m_coeff_l;
        p_h_rf_m_h = pCalibdb->mode_cell[mode_idx].rf_m_coeff_h;
    }
#endif

    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_MRF_DIAM;
        int w = RKSHAPRENHW_MRF_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++) {
                pParams->h_rf_m_l[i][m * w + n] = p_h_rf_m_l[m * w + n];
                pParams->h_rf_m_h[i][m * w + n] = p_h_rf_m_h[m * w + n];
            }
        }
    }
#endif

    float* p_kernel_mbf = mbf_coeff;
#ifndef RK_SIMULATOR_HW
    if(pCalibdb->mode_cell[mode_idx].mbf_coeff[RKSHAPRENHW_MBF_DIAM_Y * RKSHAPRENHW_MBF_DIAM_X / 2 - 1] != 0) {
        p_kernel_mbf = pCalibdb->mode_cell[mode_idx].mbf_coeff;
    }
#endif
    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_MBF_DIAM_Y;
        int w = RKSHAPRENHW_MBF_DIAM_X;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++)
                pParams->kernel_mbf[i][m * w + n] = p_kernel_mbf[m * w + n];
        }
    }

#if 0
    float* p_h_rf_h = rf_h_coeff;
#ifndef RK_SIMULATOR_HW
    if(pCalibdb->mode_cell[mode_idx].rf_h_coeff[RKSHAPRENHW_HRF_DIAM * RKSHAPRENHW_HRF_DIAM / 2] != 0) {
        p_h_rf_h = pCalibdb->mode_cell[mode_idx].rf_h_coeff;
    }
#endif
    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_HRF_DIAM;
        int w = RKSHAPRENHW_HRF_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++)
                pParams->h_rf_h[i][m * w + n] = p_h_rf_h[m * w + n];
        }
    }
#else
    float* p_h_rf_h_l = rf_h_coeff;
    float* p_h_rf_h_h = rf_h_coeff;
#ifndef RK_SIMULATOR_HW
    if(pCalibdb->mode_cell[mode_idx].rf_h_coeff_l[RKSHAPRENHW_HRF_DIAM * RKSHAPRENHW_HRF_DIAM / 2] != 0
            && pCalibdb->mode_cell[mode_idx].rf_h_coeff_h[RKSHAPRENHW_HRF_DIAM * RKSHAPRENHW_HRF_DIAM / 2] != 0) {
        p_h_rf_h_l = pCalibdb->mode_cell[mode_idx].rf_h_coeff_l;
        p_h_rf_h_h = pCalibdb->mode_cell[mode_idx].rf_h_coeff_h;
    }
#endif
    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_HRF_DIAM;
        int w = RKSHAPRENHW_HRF_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++) {
                pParams->h_rf_h_l[i][m * w + n] = p_h_rf_h_l[m * w + n];
                pParams->h_rf_h_h[i][m * w + n] = p_h_rf_h_h[m * w + n];
            }
        }
    }
#endif


#if 0
    float* p_kernel_hbf = hbf_coeff;
#ifndef RK_SIMULATOR_HW
    if(pCalibdb->mode_cell[mode_idx].hbf_coeff[RKSHAPRENHW_HBF_DIAM * RKSHAPRENHW_HBF_DIAM / 2] != 0) {
        p_kernel_hbf = pCalibdb->mode_cell[mode_idx].hbf_coeff;
    }
#endif
    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_HBF_DIAM;
        int w = RKSHAPRENHW_HBF_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++)
                pParams->kernel_hbf[i][m * w + n] = p_kernel_hbf[m * w + n];
        }
    }
#else
    float* p_kernel_hbf_l = hbf_coeff;
    float* p_kernel_hbf_h = hbf_coeff;
#ifndef RK_SIMULATOR_HW
    if(pCalibdb->mode_cell[mode_idx].hbf_coeff_l[RKSHAPRENHW_HBF_DIAM * RKSHAPRENHW_HBF_DIAM / 2] != 0
            && pCalibdb->mode_cell[mode_idx].hbf_coeff_h[RKSHAPRENHW_HBF_DIAM * RKSHAPRENHW_HBF_DIAM / 2] != 0) {
        p_kernel_hbf_l = pCalibdb->mode_cell[mode_idx].hbf_coeff_l;
        p_kernel_hbf_h = pCalibdb->mode_cell[mode_idx].hbf_coeff_h;
    }
#endif
    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_HBF_DIAM;
        int w = RKSHAPRENHW_HBF_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++) {
                pParams->kernel_hbf_l[i][m * w + n] = p_kernel_hbf_l[m * w + n];
                pParams->kernel_hbf_h[i][m * w + n] = p_kernel_hbf_h[m * w + n];
            }
        }
    }
#endif


    LOGD_ASHARP("oyyf sharp iso50 lratio:%f hratio:%f\n",
                pParams->lratio[0],
                pParams->hratio[0]);

    return res;

}


AsharpResult_t sharp_get_setting_idx_by_name_v1_json(CalibDbV2_SharpV1_t *pCalibdb, char *name,  int *setting_idx)
{
    int i = 0;
    AsharpResult_t res = ASHARP_RET_SUCCESS;

    if(pCalibdb == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(name == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(setting_idx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    for(i = 0; i < pCalibdb->TuningPara.Setting_len; i++) {
        LOGD_ASHARP("snr name:%s setName:%s\n", name, pCalibdb->TuningPara.Setting[i].SNR_Mode);
        if(strncmp(name, pCalibdb->TuningPara.Setting[i].SNR_Mode, strlen(name)*sizeof(char)) == 0) {
            break;
        }
    }

    if(i < pCalibdb->TuningPara.Setting_len) {
        *setting_idx = i;
        res = ASHARP_RET_SUCCESS;
    } else {
        *setting_idx = 0;
        res = ASHARP_RET_FAILURE;
    }

    LOGD_ASHARP("%s:%d snr_name:%s  snr_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, * setting_idx, i);
    return res;

}

AsharpResult_t init_sharp_params_v1_json(RKAsharp_Sharp_HW_Params_t *pParams, CalibDbV2_SharpV1_t *pCalibdb,  int setting_idx)
{
    AsharpResult_t res = ASHARP_RET_SUCCESS;
    int i = 0;
    int j = 0;
    int max_iso_step = MAX_ISO_STEP;
    CalibDbV2_SharpV1_T_Set_t *pSetting = NULL;
    CalibDbV2_SharpV1_T_ISO_t *pTuning_ISO = NULL;
    CalibDbV2_SharpV1_Kernel_t *pKernel_coeff = NULL;

    if(pParams == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(pCalibdb == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    pSetting = &pCalibdb->TuningPara.Setting[setting_idx];

    for (i = 0; i < pSetting->Tuning_ISO_len; i++) {
        pTuning_ISO = &pSetting->Tuning_ISO[i];
#ifndef RK_SIMULATOR_HW
        pParams->iso[i] = pTuning_ISO->iso;
#endif
        pParams->lratio[i] = pTuning_ISO->lratio;
        pParams->hratio[i] = pTuning_ISO->hratio;
        pParams->M_ratio[i] = pTuning_ISO->mf_sharp_ratio;
        pParams->H_ratio[i] = pTuning_ISO->hf_sharp_ratio;

        pParams->pbf_gain[i] =  pTuning_ISO->pbf_gain;
        pParams->pbf_ratio[i] =  pTuning_ISO->pbf_ratio;
        pParams->pbf_add[i] =  pTuning_ISO->pbf_add;

        pParams->mbf_gain[i] = pTuning_ISO->mbf_gain;
        pParams->hbf_gain[i] = pTuning_ISO->hbf_gain;
        pParams->hbf_ratio[i] = pTuning_ISO->hbf_ratio;
        pParams->mbf_add[i] = pTuning_ISO->mbf_add;
        pParams->hbf_add[i] = pTuning_ISO->hbf_add;
        pParams->ehf_th[i] = pTuning_ISO->local_sharp_strength;
        pParams->pbf_coeff_percent[i] = pTuning_ISO->pbf_coeff_percent;
        pParams->rf_m_coeff_percent[i] = pTuning_ISO->rf_m_coeff_percent;
        pParams->rf_h_coeff_percent[i] = pTuning_ISO->rf_h_coeff_percent;
        pParams->hbf_coeff_percent[i] = pTuning_ISO->hbf_coeff_percent;

        for(int j = 0; j < RK_EDGEFILTER_LUMA_POINT_NUM; j++) {
            pParams->luma_point[j] = pTuning_ISO->luma_para.luma_point[j];
            pParams->luma_sigma[i][j] = pTuning_ISO->luma_para.luma_sigma[j];
            pParams->lum_clp_m[i][j] = pTuning_ISO->luma_para.mf_clip_pos[j];
            pParams->lum_min_m[i][j] = pTuning_ISO->luma_para.mf_clip_neg[j];
            pParams->lum_clp_h[i][j] = pTuning_ISO->luma_para.hf_clip[j];
        }
    }

    pKernel_coeff = &pCalibdb->TuningPara.kernel_coeff;

    //////////////////////////////////////////////////////////////////////////
    // init filter params
    // Gaussian filter params
    float gaus_luma_coeff[RKSHAPRENHW_PBF_DIAM * RKSHAPRENHW_PBF_DIAM] =
    {
        0.0625, 0.125, 0.0625,
        0.125, 0.25, 0.125,
        0.0625, 0.125, 0.0625
    };
    // pre bf params
    float pbf_coeff[RKSHAPRENHW_PBF_DIAM * RKSHAPRENHW_PBF_DIAM] =
    {
        0.156250, 0.25, 0.156250,
        0.25, 0.375000, 0.25,
        0.156250, 0.25, 0.156250
    };
    // mf range filter params
    float rf_m_coeff[RKSHAPRENHW_MRF_DIAM * RKSHAPRENHW_MRF_DIAM] =
    {
        0.023438, 0.031250, 0.039063, 0.031250, 0.023438,
        0.031250, 0.046875, 0.054688, 0.046875, 0.031250,
        0.039063, 0.054688, 0.093750, 0.054688, 0.039063,
        0.031250, 0.046875, 0.054688, 0.046875, 0.031250,
        0.023438, 0.031250, 0.039063, 0.031250, 0.023438

    };
    // mf bf params
    float mbf_coeff[RKSHAPRENHW_MBF_DIAM_Y * RKSHAPRENHW_MBF_DIAM_X] =
    {
        0.0, 0.0, 0.0, 0.0, 0.406250, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.406250, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.343750, 0.0, 0.328125, 0.0, 0.343750, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.406250, 0.0, 0.359375, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.359375, 0.0, 0.406250, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.296875, 0.0, 0.234375, 0.0, 0.234375, 0.0, 0.296875, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.406250, 0.0, 0.343750, 0.0, 0.0, 0.0, 0.0, 0.171875, 0.0, 0.0, 0.0, 0.0, 0.343750, 0.0, 0.406250, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.234375, 0.0, 0.140625, 0.109375, 0.140625, 0.0, 0.234375, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.437500, 0.0, 0.0, 0.328125, 0.0, 0.0, 0.171875, 0.109375, 0.0, 0.109375, 0.171875, 0.0, 0.0, 0.328125, 0.0, 0.0, 0.437500,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.234375, 0.0, 0.140625, 0.109375, 0.140625, 0.0, 0.234375, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.406250, 0.0, 0.343750, 0.0, 0.0, 0.0, 0.0, 0.171875, 0.0, 0.0, 0.0, 0.0, 0.343750, 0.0, 0.406250, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.296875, 0.0, 0.234375, 0.0, 0.234375, 0.0, 0.296875, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.406250, 0.0, 0.359375, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.359375, 0.0, 0.406250, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.343750, 0.0, 0.328125, 0.0, 0.343750, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.406250, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.406250, 0.0, 0.0, 0.0, 0.0
    };
    // hf range filter params
    float rf_h_coeff[RKSHAPRENHW_HRF_DIAM * RKSHAPRENHW_HRF_DIAM] =
    {
        0.0, 0.015625, 0.023438, 0.015625, 0.0,
        0.015625, 0.062500, 0.101563, 0.062500, 0.015625,
        0.023438, 0.101563, 0.125000, 0.101563, 0.023438,
        0.015625, 0.062500, 0.101563, 0.062500, 0.015625,
        0.0, 0.015625, 0.023438, 0.015625, 0.0
    };
    // hf bf params
    float hbf_coeff[RKSHAPRENHW_HBF_DIAM * RKSHAPRENHW_HBF_DIAM] =
    {
        0.156250, 0.25, 0.156250,
        0.25, 0.375000, 0.25,
        0.156250, 0.25, 0.156250
    };

    float* p_gaus_luma_kernel = gaus_luma_coeff;
#ifndef RK_SIMULATOR_HW
    if(pKernel_coeff->gauss_luma_coeff[RKSHAPRENHW_GAU_DIAM * RKSHAPRENHW_GAU_DIAM / 2] != 0) {
        p_gaus_luma_kernel = pKernel_coeff->gauss_luma_coeff;
    }
#endif

    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_GAU_DIAM;
        int w = RKSHAPRENHW_GAU_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++) {
                pParams->gaus_luma_kernel[i][m * w + n] = p_gaus_luma_kernel[m * w + n];
            }
        }
    }

    float* p_kernel_pbf_l = pbf_coeff;
    float* p_kernel_pbf_h = pbf_coeff;
#if 0
    float* p_kernel_pbf = pbf_coeff;
    if(pCalibdb->mode_cell[mode_idx].pbf_coeff[RKSHAPRENHW_PBF_DIAM * RKSHAPRENHW_PBF_DIAM / 2] != 0) {
        p_kernel_pbf = pCalibdb->mode_cell[mode_idx].pbf_coeff;
    }

    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_PBF_DIAM;
        int w = RKSHAPRENHW_PBF_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++) {
                pParams->kernel_pbf[i][m * w + n] = p_kernel_pbf[m * w + n];
            }
        }
    }
#endif

#ifndef RK_SIMULATOR_HW
    if(pKernel_coeff->pbf_coeff_l[RKSHAPRENHW_PBF_DIAM * RKSHAPRENHW_PBF_DIAM / 2] != 0
            && pKernel_coeff->pbf_coeff_h[RKSHAPRENHW_PBF_DIAM * RKSHAPRENHW_PBF_DIAM / 2] != 0) {
        p_kernel_pbf_l = pKernel_coeff->pbf_coeff_l;
        p_kernel_pbf_h = pKernel_coeff->pbf_coeff_h;
    }
#endif
    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_PBF_DIAM;
        int w = RKSHAPRENHW_PBF_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++) {
                pParams->kernel_pbf_l[i][m * w + n] = p_kernel_pbf_l[m * w + n];
                pParams->kernel_pbf_h[i][m * w + n] = p_kernel_pbf_h[m * w + n];
            }
        }
    }

#if 0
    float* p_h_rf_m = rf_m_coeff;
#ifndef RK_SIMULATOR_HW
    if(pCalibdb->mode_cell[mode_idx].rf_m_coeff[RKSHAPRENHW_MRF_DIAM * RKSHAPRENHW_MRF_DIAM / 2] != 0) {
        p_h_rf_m = pCalibdb->mode_cell[mode_idx].rf_m_coeff;
    }
#endif
    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_MRF_DIAM;
        int w = RKSHAPRENHW_MRF_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++)
                pParams->h_rf_m[i][m * w + n] = p_h_rf_m[m * w + n];
        }
    }
#else
    float* p_h_rf_m_l = rf_m_coeff;
    float* p_h_rf_m_h = rf_m_coeff;
#ifndef RK_SIMULATOR_HW
    if(pKernel_coeff->rf_m_coeff_l[RKSHAPRENHW_MRF_DIAM * RKSHAPRENHW_MRF_DIAM / 2] != 0
            && pKernel_coeff->rf_m_coeff_h[RKSHAPRENHW_MRF_DIAM * RKSHAPRENHW_MRF_DIAM / 2] != 0) {
        p_h_rf_m_l = pKernel_coeff->rf_m_coeff_l;
        p_h_rf_m_h = pKernel_coeff->rf_m_coeff_h;
    }
#endif

    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_MRF_DIAM;
        int w = RKSHAPRENHW_MRF_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++) {
                pParams->h_rf_m_l[i][m * w + n] = p_h_rf_m_l[m * w + n];
                pParams->h_rf_m_h[i][m * w + n] = p_h_rf_m_h[m * w + n];
            }
        }
    }
#endif

    float* p_kernel_mbf = mbf_coeff;
#ifndef RK_SIMULATOR_HW
    if(pKernel_coeff->mbf_coeff[RKSHAPRENHW_MBF_DIAM_Y * RKSHAPRENHW_MBF_DIAM_X / 2 - 1] != 0) {
        p_kernel_mbf = pKernel_coeff->mbf_coeff;
    }
#endif
    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_MBF_DIAM_Y;
        int w = RKSHAPRENHW_MBF_DIAM_X;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++)
                pParams->kernel_mbf[i][m * w + n] = p_kernel_mbf[m * w + n];
        }
    }

#if 0
    float* p_h_rf_h = rf_h_coeff;
#ifndef RK_SIMULATOR_HW
    if(pCalibdb->mode_cell[mode_idx].rf_h_coeff[RKSHAPRENHW_HRF_DIAM * RKSHAPRENHW_HRF_DIAM / 2] != 0) {
        p_h_rf_h = pCalibdb->mode_cell[mode_idx].rf_h_coeff;
    }
#endif
    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_HRF_DIAM;
        int w = RKSHAPRENHW_HRF_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++)
                pParams->h_rf_h[i][m * w + n] = p_h_rf_h[m * w + n];
        }
    }
#else
    float* p_h_rf_h_l = rf_h_coeff;
    float* p_h_rf_h_h = rf_h_coeff;
#ifndef RK_SIMULATOR_HW
    if(pKernel_coeff->rf_h_coeff_l[RKSHAPRENHW_HRF_DIAM * RKSHAPRENHW_HRF_DIAM / 2] != 0
            && pKernel_coeff->rf_h_coeff_h[RKSHAPRENHW_HRF_DIAM * RKSHAPRENHW_HRF_DIAM / 2] != 0) {
        p_h_rf_h_l = pKernel_coeff->rf_h_coeff_l;
        p_h_rf_h_h = pKernel_coeff->rf_h_coeff_h;
    }
#endif
    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_HRF_DIAM;
        int w = RKSHAPRENHW_HRF_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++) {
                pParams->h_rf_h_l[i][m * w + n] = p_h_rf_h_l[m * w + n];
                pParams->h_rf_h_h[i][m * w + n] = p_h_rf_h_h[m * w + n];
            }
        }
    }
#endif


#if 0
    float* p_kernel_hbf = hbf_coeff;
#ifndef RK_SIMULATOR_HW
    if(pCalibdb->mode_cell[mode_idx].hbf_coeff[RKSHAPRENHW_HBF_DIAM * RKSHAPRENHW_HBF_DIAM / 2] != 0) {
        p_kernel_hbf = pCalibdb->mode_cell[mode_idx].hbf_coeff;
    }
#endif
    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_HBF_DIAM;
        int w = RKSHAPRENHW_HBF_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++)
                pParams->kernel_hbf[i][m * w + n] = p_kernel_hbf[m * w + n];
        }
    }
#else
    float* p_kernel_hbf_l = hbf_coeff;
    float* p_kernel_hbf_h = hbf_coeff;
#ifndef RK_SIMULATOR_HW
    if(pKernel_coeff->hbf_coeff_l[RKSHAPRENHW_HBF_DIAM * RKSHAPRENHW_HBF_DIAM / 2] != 0
            && pKernel_coeff->hbf_coeff_h[RKSHAPRENHW_HBF_DIAM * RKSHAPRENHW_HBF_DIAM / 2] != 0) {
        p_kernel_hbf_l = pKernel_coeff->hbf_coeff_l;
        p_kernel_hbf_h = pKernel_coeff->hbf_coeff_h;
    }
#endif
    for (i = 0; i < max_iso_step; i++) {
        int h = RKSHAPRENHW_HBF_DIAM;
        int w = RKSHAPRENHW_HBF_DIAM;
        for(int m = 0; m < h; m++) {
            for(int n = 0; n < w; n++) {
                pParams->kernel_hbf_l[i][m * w + n] = p_kernel_hbf_l[m * w + n];
                pParams->kernel_hbf_h[i][m * w + n] = p_kernel_hbf_h[m * w + n];
            }
        }
    }
#endif

    LOGD_ASHARP("oyyf sharp iso50 lratio:%f hratio:%f\n",
                pParams->lratio[0],
                pParams->hratio[0]);

    sharp_algo_param_printf(pParams);

    return res;

}

AsharpResult_t sharp_algo_param_printf(RKAsharp_Sharp_HW_Params_t *pParams)
{
    int i;

    if(pParams != NULL) {

        for(i = 0; i < MAX_ISO_STEP; i++) {
#ifndef RK_SIMULATOR_HW
            LOGD_ASHARP(" sharp: ISO:%f\n", pParams->iso[i]);
#endif

            LOGD_ASHARP(" lratio:%f  hratio:%f mf_sharp_ratio:%f hf_sharp_ratio:%f\n",
                        pParams->lratio[i],
                        pParams->hratio[i],
                        pParams->M_ratio[i],
                        pParams->H_ratio[i]);

            LOGD_ASHARP(" pbf_gain:%f  pbf_ratio:%f pbf_add:%f\n",
                        pParams->pbf_gain[i],
                        pParams->pbf_ratio[i],
                        pParams->pbf_add[i]);


            LOGD_ASHARP(" mbf_gain:%f  mbf_add:%f local_sharp_strength:%d\n",
                        pParams->mbf_gain[i],
                        pParams->mbf_add[i],
                        pParams->ehf_th[i]);

            LOGD_ASHARP(" hbf_gain:%f  hbf_ratio:%f hbf_add:%f\n",
                        pParams->hbf_gain[i],
                        pParams->hbf_ratio[i],
                        pParams->hbf_add[i]);

            LOGD_ASHARP(" pbf_coeff_percent:%f  rf_m_coeff_percent:%f rf_h_coeff_percent:%f hbf_coeff_percent:%f\n",
                        pParams->pbf_coeff_percent[i],
                        pParams->rf_m_coeff_percent[i],
                        pParams->rf_h_coeff_percent[i],
                        pParams->hbf_coeff_percent[i]);


            for(int j = 0; j < RK_EDGEFILTER_LUMA_POINT_NUM; j++) {

                LOGD_ASHARP(" luma_point:%d luma_sigma:%f mf_clip_pos:%d mf_clip_neg:%f hf_clip:%d\n",
                            pParams->luma_point[j],
                            pParams->luma_sigma[i][j],
                            pParams->lum_clp_m[i][j],
                            pParams->lum_min_m[i][j],
                            pParams->lum_clp_h[i][j]);
            }

        }
    }

    return ASHARP_RET_SUCCESS;
}

AsharpResult_t sharp_config_setting_param_v1_json(RKAsharp_Sharp_HW_Params_t *pParams, CalibDbV2_SharpV1_t *pCalibdb, char *param_mode, char* snr_name)
{
    AsharpResult_t res = ASHARP_RET_SUCCESS;
    int setting_idx = 0;

    if(pParams == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(pCalibdb == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    res = sharp_get_setting_idx_by_name_v1_json(pCalibdb, snr_name, &setting_idx);
    if(res != ASHARP_RET_SUCCESS) {
        LOGE_ASHARP("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = init_sharp_params_v1_json(pParams, pCalibdb, setting_idx);

    LOGD_ASHARP("%s(%d): finnal snr_mode:%d \n", __FUNCTION__, __LINE__,  setting_idx);
    return res;

}


AsharpResult_t select_rk_sharpen_hw_params_by_ISO(
    RKAsharp_Sharp_HW_Params_t *strksharpenParams,
    RKAsharp_Sharp_HW_Params_Select_t *strksharpenParamsSelected,
    AsharpExpInfo_t *pExpInfo
)
{
    int i;
    int gain_high = 0, gain_low = 0;
    float ratio = 0.0f;
    int iso_div             = 50;
    int max_iso_step        = MAX_ISO_STEP;
    AsharpResult_t res = ASHARP_RET_SUCCESS;
    int iso = 50;
    int iso_low = iso, iso_high = iso;

    if(strksharpenParams == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(strksharpenParamsSelected == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(pExpInfo->mfnr_mode_3to1) {
        iso = pExpInfo->preIso[pExpInfo->hdr_mode];
    } else {
        iso = pExpInfo->arIso[pExpInfo->hdr_mode];
    }

#ifndef RK_SIMULATOR_HW
    for (i = 0; i < max_iso_step - 1; i++) {
        if (iso >=  strksharpenParams->iso[i]  &&  iso <=  strksharpenParams->iso[i + 1] ) {
            iso_low = strksharpenParams->iso[i] ;
            iso_high = strksharpenParams->iso[i + 1];
            gain_low = i;
            gain_high = i + 1;
            ratio = (float)(iso - iso_low) / (iso_high - iso_low);
            break;
        }
    }

    if(i == max_iso_step - 1) {
        if(iso < strksharpenParams->iso[0] ) {
            iso_low = strksharpenParams->iso[0] ;
            iso_high = strksharpenParams->iso[1];
            gain_low = 0;
            gain_high = 1;
            ratio = 0;
        }

        if(iso > strksharpenParams->iso[max_iso_step - 1] ) {
            iso_low = strksharpenParams->iso[max_iso_step - 2] ;
            iso_high = strksharpenParams->iso[max_iso_step - 1];
            gain_low = max_iso_step - 2;
            gain_high = max_iso_step - 1;
            ratio = 1;
        }
    }
#else
    for (i = max_iso_step - 1; i >= 0; i--)
    {
        if (iso < iso_div * (2 << i))
        {
            iso_low = iso_div * (2 << (i)) / 2;
            iso_high = iso_div * (2 << i);
            break;
        }
    }

    ratio = (float)(iso - iso_low) / (iso_high - iso_low);
    if (iso_low == iso)
    {
        iso_high = iso;
        ratio = 0;
    }
    if (iso_high == iso )
    {
        iso_low = iso;
        ratio = 1;
    }
    gain_high       = (int)(log((float)iso_high / 50) / log((float)2));
    gain_low        = (int)(log((float)iso_low / 50) / log((float)2));


    gain_low        = MIN(MAX(gain_low, 0), max_iso_step - 1);
    gain_high       = MIN(MAX(gain_high, 0), max_iso_step - 1);
#endif

    LOGD_ASHARP("%s:%d iso:%d iso_low:%d iso_high:%d gainlow:%d gain_high:%d ratio:%f\n",
                __FUNCTION__, __LINE__,
                iso, iso_low, iso_high, gain_low, gain_high, ratio);
    strksharpenParamsSelected->lratio               = INTERP1(strksharpenParams->lratio     [gain_low],     strksharpenParams->lratio       [gain_high],    ratio);
    strksharpenParamsSelected->hratio               = INTERP1(strksharpenParams->hratio     [gain_low],     strksharpenParams->hratio       [gain_high],    ratio);
    strksharpenParamsSelected->M_ratio              = INTERP1(strksharpenParams->M_ratio    [gain_low],     strksharpenParams->M_ratio      [gain_high],    ratio);
    strksharpenParamsSelected->H_ratio              = INTERP1(strksharpenParams->H_ratio    [gain_low],     strksharpenParams->H_ratio      [gain_high],    ratio);
    strksharpenParamsSelected->pbf_ratio            = INTERP1(strksharpenParams->pbf_ratio  [gain_low],     strksharpenParams->pbf_ratio    [gain_high],    ratio);
    strksharpenParamsSelected->hbf_ratio            = INTERP1(strksharpenParams->hbf_ratio  [gain_low],     strksharpenParams->hbf_ratio    [gain_high],    ratio);
    strksharpenParamsSelected->ehf_th               = (short)ROUND_F(INTERP1(strksharpenParams->ehf_th     [gain_low],     strksharpenParams->ehf_th       [gain_high],    ratio));
    for(int i = 0; i < RK_EDGEFILTER_LUMA_POINT_NUM; i++)
    {
        strksharpenParamsSelected->luma_point[i]    = strksharpenParams->luma_point[i];
        strksharpenParamsSelected->luma_sigma[i]    = INTERP1(strksharpenParams->luma_sigma [gain_low][i],  strksharpenParams->luma_sigma   [gain_high][i], ratio);

        strksharpenParamsSelected->lum_clp_m[i]     = (short)ROUND_F(INTERP1(strksharpenParams->lum_clp_m  [gain_low][i],  strksharpenParams->lum_clp_m    [gain_high][i], ratio));
        strksharpenParamsSelected->lum_min_m[i]     = INTERP1(strksharpenParams->lum_min_m  [gain_low][i],  strksharpenParams->lum_min_m    [gain_high][i], ratio);

        strksharpenParamsSelected->lum_clp_h[i]     = (short)ROUND_F(INTERP1(strksharpenParams->lum_clp_h  [gain_low][i],  strksharpenParams->lum_clp_h    [gain_high][i], ratio));
    }
    strksharpenParamsSelected->pbf_gain      = INTERP1(strksharpenParams->pbf_gain   [gain_low],  strksharpenParams->pbf_gain     [gain_high], ratio);
    strksharpenParamsSelected->pbf_add       = INTERP1(strksharpenParams->pbf_add    [gain_low],  strksharpenParams->pbf_add      [gain_high], ratio);

    strksharpenParamsSelected->mbf_gain      = INTERP1(strksharpenParams->mbf_gain   [gain_low],  strksharpenParams->mbf_gain     [gain_high], ratio);
    strksharpenParamsSelected->mbf_add       = INTERP1(strksharpenParams->mbf_add    [gain_low],  strksharpenParams->mbf_add      [gain_high], ratio);

    strksharpenParamsSelected->hbf_gain      = INTERP1(strksharpenParams->hbf_gain   [gain_low],  strksharpenParams->hbf_gain     [gain_high], ratio);
    strksharpenParamsSelected->hbf_add       = INTERP1(strksharpenParams->hbf_add    [gain_low],  strksharpenParams->hbf_add      [gain_high], ratio);


    for(unsigned int i = 0; i < sizeof(strksharpenParamsSelected->gaus_luma_kernel) / sizeof(strksharpenParamsSelected->gaus_luma_kernel[0]); i++)
        strksharpenParamsSelected->gaus_luma_kernel[i]    = INTERP1(strksharpenParams->gaus_luma_kernel [gain_low][i],  strksharpenParams->gaus_luma_kernel   [gain_high][i], ratio);
    for(unsigned int i = 0; i < sizeof(strksharpenParamsSelected->kernel_mbf) / sizeof(strksharpenParamsSelected->kernel_mbf[0]); i++)
        strksharpenParamsSelected->kernel_mbf[i]    = INTERP1(strksharpenParams->kernel_mbf [gain_low][i],  strksharpenParams->kernel_mbf   [gain_high][i], ratio);
#if 0
    for(unsigned int i = 0; i < sizeof(strksharpenParamsSelected->kernel_pbf) / sizeof(strksharpenParamsSelected->kernel_pbf[0]); i++)
        strksharpenParamsSelected->kernel_pbf[i]    = INTERP1(strksharpenParams->kernel_pbf [gain_low][i],  strksharpenParams->kernel_pbf   [gain_high][i], ratio);
    for(unsigned int i = 0; i < sizeof(strksharpenParamsSelected->h_rf_m) / sizeof(strksharpenParamsSelected->h_rf_m[0]); i++)
        strksharpenParamsSelected->h_rf_m[i]        = INTERP1(strksharpenParams->h_rf_m [gain_low][i],      strksharpenParams->h_rf_m       [gain_high][i], ratio);

    for(unsigned int i = 0; i < sizeof(strksharpenParamsSelected->h_rf_h) / sizeof(strksharpenParamsSelected->h_rf_h[0]); i++)
        strksharpenParamsSelected->h_rf_h[i]        = INTERP1(strksharpenParams->h_rf_h [gain_low][i],      strksharpenParams->h_rf_h       [gain_high][i], ratio);
    for(unsigned int i = 0; i < sizeof(strksharpenParamsSelected->kernel_hbf) / sizeof(strksharpenParamsSelected->kernel_hbf[0]); i++)
        strksharpenParamsSelected->kernel_hbf[i]    = INTERP1(strksharpenParams->kernel_hbf [gain_low][i],  strksharpenParams->kernel_hbf   [gain_high][i], ratio);
#else

    float pbf_coeff_percent      = INTERP1(strksharpenParams->pbf_coeff_percent    [gain_low],  strksharpenParams->pbf_coeff_percent      [gain_high], ratio);
    float rf_m_coeff_percent      = INTERP1(strksharpenParams->rf_m_coeff_percent    [gain_low],  strksharpenParams->rf_m_coeff_percent      [gain_high], ratio);
    float rf_h_coeff_percent      = INTERP1(strksharpenParams->rf_h_coeff_percent    [gain_low],  strksharpenParams->rf_h_coeff_percent      [gain_high], ratio);
    float hbf_coeff_percent      = INTERP1(strksharpenParams->hbf_coeff_percent    [gain_low],  strksharpenParams->hbf_coeff_percent      [gain_high], ratio);

    sharp_filter_merge(strksharpenParams->kernel_pbf_h[gain_low], strksharpenParams->kernel_pbf_l[gain_low], strksharpenParamsSelected->kernel_pbf, 9, pbf_coeff_percent);
    sharp_filter_merge(strksharpenParams->h_rf_m_h[gain_low], strksharpenParams->h_rf_m_l[gain_low], strksharpenParamsSelected->h_rf_m, 25, rf_m_coeff_percent);
    sharp_filter_merge(strksharpenParams->h_rf_h_h[gain_low], strksharpenParams->h_rf_h_l[gain_low], strksharpenParamsSelected->h_rf_h, 25, rf_h_coeff_percent);
    sharp_filter_merge(strksharpenParams->kernel_hbf_h[gain_low], strksharpenParams->kernel_hbf_l[gain_low], strksharpenParamsSelected->kernel_hbf, 9, hbf_coeff_percent);
#endif
    return res;
}

AsharpResult_t select_rk_sharpen_hw_v2_params_by_ISO(
    RKAsharp_Sharp_HW_V2_Params_t *strksharpenParams,
    RKAsharp_Sharp_HW_V2_Params_Select_t *strksharpenParamsSelected,
    AsharpExpInfo_t *pExpInfo
)
{
    int i;
    int gain_high, gain_low;
    float ratio = 0.0f;
    int iso_div             = 50;
    int max_iso_step        = MAX_ISO_STEP;
    AsharpResult_t res = ASHARP_RET_SUCCESS;
    int iso = 50;
    int iso_low = iso, iso_high = iso;

    if(strksharpenParams == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(strksharpenParamsSelected == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];
    for (i = max_iso_step - 1; i >= 0; i--)
    {
        if (iso < iso_div * (2 << i))
        {
            iso_low = iso_div * (2 << (i)) / 2;
            iso_high = iso_div * (2 << i);
        }
    }
    ratio = (float)(iso - iso_low) / (iso_high - iso_low);
    if (iso_low == iso)
    {
        iso_high = iso;
        ratio = 0;
    }
    if (iso_high == iso )
    {
        iso_low = iso;
        ratio = 1;
    }
    gain_high       = (int)(log((float)iso_high / 50) / log((float)2));
    gain_low        = (int)(log((float)iso_low / 50) / log((float)2));

    gain_low        = MIN(MAX(gain_low, 0), max_iso_step - 1);
    gain_high       = MIN(MAX(gain_high, 0), max_iso_step - 1);

    strksharpenParamsSelected->pbf_gain             = INTERP1(strksharpenParams->pbf_gain   [gain_low],     strksharpenParams->pbf_gain     [gain_high],    ratio);
    strksharpenParamsSelected->pbf_add              = INTERP1(strksharpenParams->pbf_add    [gain_low],     strksharpenParams->pbf_add      [gain_high],    ratio);
    strksharpenParamsSelected->pbf_ratio            = INTERP1(strksharpenParams->pbf_ratio  [gain_low],     strksharpenParams->pbf_ratio    [gain_high],    ratio);
    strksharpenParamsSelected->lratio               = INTERP1(strksharpenParams->lratio     [gain_low],     strksharpenParams->lratio       [gain_high],    ratio);
    strksharpenParamsSelected->hratio               = INTERP1(strksharpenParams->hratio     [gain_low],     strksharpenParams->hratio       [gain_high],    ratio);
    strksharpenParamsSelected->sharp_ratio          = INTERP1(strksharpenParams->sharp_ratio[gain_low],     strksharpenParams->sharp_ratio  [gain_high],    ratio);
    strksharpenParamsSelected->hbf_gain             = INTERP1(strksharpenParams->hbf_gain   [gain_low],     strksharpenParams->hbf_gain     [gain_high],    ratio);
    strksharpenParamsSelected->hbf_add              = INTERP1(strksharpenParams->hbf_add    [gain_low],     strksharpenParams->hbf_add      [gain_high],    ratio);
    strksharpenParamsSelected->hbf_ratio            = INTERP1(strksharpenParams->hbf_ratio  [gain_low],     strksharpenParams->hbf_ratio    [gain_high],    ratio);
    strksharpenParamsSelected->ehf_th               = (short)ROUND_F(INTERP1(strksharpenParams->ehf_th     [gain_low],     strksharpenParams->ehf_th       [gain_high],    ratio));
    for(int i = 0; i < RK_EDGEFILTER_LUMA_POINT_NUM; i++)
    {
        strksharpenParamsSelected->luma_point[i]    = strksharpenParams->luma_point[i];
        strksharpenParamsSelected->luma_sigma[i]    = INTERP1(strksharpenParams->luma_sigma [gain_low][i],  strksharpenParams->luma_sigma   [gain_high][i], ratio);
        strksharpenParamsSelected->lum_clip_h[i]    = (short)ROUND_F(INTERP1(strksharpenParams->lum_clip_h  [gain_low][i],  strksharpenParams->lum_clip_h    [gain_high][i], ratio));
    }
    for(int i = 0; i < RKSHAPRENHW_V2_GAU_DIAM * RKSHAPRENHW_V2_GAU_DIAM; i++)
    {
        strksharpenParamsSelected->gaus_luma_kernel[i] = INTERP1(strksharpenParams->gaus_luma_kernel [gain_low][i],  strksharpenParams->gaus_luma_kernel[gain_high][i], ratio);
    }

    for(int i = 0; i < RKSHAPRENHW_V2_PBF_DIAM * RKSHAPRENHW_V2_PBF_DIAM; i++)
    {
        strksharpenParamsSelected->kernel_pre_bf[i] = INTERP1(strksharpenParams->kernel_pre_bf [gain_low][i], strksharpenParams->kernel_pre_bf[gain_high][i], ratio);
    }
    for(int i = 0; i < RKSHAPRENHW_V2_HRF_DIAM * RKSHAPRENHW_V2_HRF_DIAM; i++)
    {
        strksharpenParamsSelected->kernel_range_filter[i] = INTERP1(strksharpenParams->kernel_range_filter[gain_low][i], strksharpenParams->kernel_range_filter[gain_high][i], ratio);
    }
    for(int i = 0; i < RKSHAPRENHW_V2_HBF_DIAM_Y * RKSHAPRENHW_V2_HBF_DIAM_X; i++)
    {
        strksharpenParamsSelected->kernel_hf_bf[i] = INTERP1(strksharpenParams->kernel_hf_bf[gain_low][i],  strksharpenParams->kernel_hf_bf[gain_high][i], ratio);
    }

    return res;
}

AsharpResult_t select_rk_sharpen_hw_v3_params_by_ISO(
    RKAsharp_Sharp_HW_V3_Params_t *strksharpenParams,
    RKAsharp_Sharp_HW_V3_Params_Select_t *strksharpenParamsSelected,
    AsharpExpInfo_t *pExpInfo
)
{
    int i;
    int gain_high, gain_low;
    float ratio = 0.0f;
    int iso_div             = 50;
    int max_iso_step        = MAX_ISO_STEP;
    AsharpResult_t res = ASHARP_RET_SUCCESS;
    int iso = 50;
    int iso_low = iso, iso_high = iso;

    if(strksharpenParams == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(strksharpenParamsSelected == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];
    for (i = max_iso_step - 1; i >= 0; i--)
    {
        if (iso < iso_div * (2 << i))
        {
            iso_low = iso_div * (2 << (i)) / 2;
            iso_high = iso_div * (2 << i);
        }
    }
    ratio = (float)(iso - iso_low) / (iso_high - iso_low);
    if (iso_low == iso)
    {
        iso_high = iso;
        ratio = 0;
    }
    if (iso_high == iso )
    {
        iso_low = iso;
        ratio = 1;
    }
    gain_high       = (int)(log((float)iso_high / 50) / log((float)2));
    gain_low        = (int)(log((float)iso_low / 50) / log((float)2));

    gain_low        = MIN(MAX(gain_low, 0), max_iso_step - 1);
    gain_high       = MIN(MAX(gain_high, 0), max_iso_step - 1);

    strksharpenParamsSelected->lratio               = INTERP1(strksharpenParams->lratio     [gain_low],     strksharpenParams->lratio       [gain_high],    ratio);
    strksharpenParamsSelected->hratio               = INTERP1(strksharpenParams->hratio     [gain_low],     strksharpenParams->hratio       [gain_high],    ratio);
    strksharpenParamsSelected->sharp_ratio          = INTERP1(strksharpenParams->sharp_ratio[gain_low],     strksharpenParams->sharp_ratio  [gain_high],    ratio);
    strksharpenParamsSelected->ehf_th               = (short)ROUND_F(INTERP1(strksharpenParams->ehf_th     [gain_low],     strksharpenParams->ehf_th       [gain_high],    ratio));
    for(int i = 0; i < RK_EDGEFILTER_LUMA_POINT_NUM; i++)
    {
        strksharpenParamsSelected->luma_point[i]    = strksharpenParams->luma_point[i];
        strksharpenParamsSelected->lum_clip_h[i]    = (short)ROUND_F(INTERP1(strksharpenParams->lum_clip_h  [gain_low][i],  strksharpenParams->lum_clip_h    [gain_high][i], ratio));
    }
    for(int i = 0; i < RKSHAPRENHW_V3_RF_DIAM * RKSHAPRENHW_V3_RF_DIAM; i++)
    {
        strksharpenParamsSelected->kernel_range_filter[i] = INTERP1(strksharpenParams->kernel_range_filter [gain_low][i], strksharpenParams->kernel_range_filter[gain_high][i], ratio);
    }

    return res;
}

void select_sharpen_params_by_ISO   (
    RKAsharp_Sharp_Params_t *strksharpenParams,
    RKAsharp_Sharp_Params_Select_t *strksharpenParamsSelected,
    AsharpExpInfo_t *pExpInfo
)
{
    select_rk_sharpen_hw_params_by_ISO(&strksharpenParams->rk_sharpen_params_V1, &strksharpenParamsSelected->rk_sharpen_params_selected_V1, pExpInfo);

    select_rk_sharpen_hw_v2_params_by_ISO(&strksharpenParams->rk_sharpen_params_V2, &strksharpenParamsSelected->rk_sharpen_params_selected_V2, pExpInfo);

    select_rk_sharpen_hw_v3_params_by_ISO(&strksharpenParams->rk_sharpen_params_V3, &strksharpenParamsSelected->rk_sharpen_params_selected_V3, pExpInfo);
}



AsharpResult_t rk_Sharp_V1_fix_transfer(RKAsharp_Sharp_HW_Params_Select_t *pSharpV1, RKAsharp_Sharp_HW_Fix_t* pSharpCfg, float fPercent)
{
    int i = 0;
    int k = 0;
    int tmp = 0;
    int sum_coeff, offset;
    float sum_coeff_float;
    AsharpResult_t res = ASHARP_RET_SUCCESS;
    float mClipStrength = 1.0;
    float hClipStrength = 1.0;

    if(pSharpV1 == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(pSharpCfg == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(fPercent <= 0.0) {
        fPercent = 0.000001;
    }

    if(fPercent > (MAX_SHARP_LUMA_CLIP_VALUE / pSharpV1->M_ratio )) {
        mClipStrength = fPercent / (MAX_SHARP_LUMA_CLIP_VALUE / pSharpV1->M_ratio ) ;
    }

    if(fPercent > (MAX_SHARP_LUMA_CLIP_VALUE / pSharpV1->H_ratio)) {
        hClipStrength = fPercent / (MAX_SHARP_LUMA_CLIP_VALUE / pSharpV1->H_ratio ) ;
    }

    LOGD_ASHARP("%s:%d percent:%f m&h_strength:%f %f\n",
                __FUNCTION__, __LINE__, fPercent,
                mClipStrength, hClipStrength);

    pSharpV1->pbf_gain  = pSharpV1->pbf_gain / fPercent;
    if(pSharpV1->pbf_gain > 2.0)
        pSharpV1->pbf_gain  = 2.0;

    pSharpV1->pbf_add = pSharpV1->pbf_add / fPercent;
    if(pSharpV1->pbf_add > 255)
        pSharpV1->pbf_add = 255;

    pSharpV1->mbf_gain  = pSharpV1->mbf_gain / fPercent;
    if(pSharpV1->mbf_gain > 2.0)
        pSharpV1->mbf_gain  = 2.0;

    pSharpV1->mbf_add = pSharpV1->mbf_add / fPercent;
    if(pSharpV1->mbf_add > 255)
        pSharpV1->mbf_add = 255;

    pSharpV1->hbf_gain  = pSharpV1->hbf_gain / fPercent;
    if(pSharpV1->hbf_gain > 2.0)
        pSharpV1->hbf_gain  = 2.0;

    pSharpV1->hbf_add = pSharpV1->hbf_add / fPercent;
    if(pSharpV1->hbf_add > 255)
        pSharpV1->hbf_add = 255;


    //0x0080
    pSharpCfg->yin_flt_en = 1;
    pSharpCfg->edge_avg_en = 1;

    //0x0084
    tmp = ROUND_F(pSharpV1->hbf_ratio / fPercent * (1 << reg_sharpenHW_hbf_ratio_fix_bits));
    if(tmp > 0x100) {
        tmp = 0x100;
    }
    pSharpCfg->hbf_ratio = (unsigned short)tmp;
    tmp = ROUND_F(pSharpV1->ehf_th * fPercent);
    if(tmp > 0xff) {
        tmp = 0xff;
    }
    pSharpCfg->ehf_th = (unsigned char) tmp;
    tmp = ROUND_F(pSharpV1->pbf_ratio / fPercent * (1 << reg_sharpenHW_pbf_ratio_fix_bits));
    if(tmp > 0x80) {
        tmp = 0x80;
    }
    pSharpCfg->pbf_ratio = (unsigned short)tmp;


    //0x0090
    sum_coeff = 0;
    sum_coeff_float = 0;
    pSharpCfg->pbf_k[0] = (unsigned char)ROUND_F(pSharpV1->kernel_pbf[4] * (1 << reg_sharpenHW_pPBfCoeff_fix_bits));
    pSharpCfg->pbf_k[1] = (unsigned char)ROUND_F(pSharpV1->kernel_pbf[1] * (1 << reg_sharpenHW_pPBfCoeff_fix_bits));
    pSharpCfg->pbf_k[2] = (unsigned char)ROUND_F(pSharpV1->kernel_pbf[0] * (1 << reg_sharpenHW_pPBfCoeff_fix_bits));
    for(k = 0; k < RKSHAPRENHW_PBF_DIAM * RKSHAPRENHW_PBF_DIAM; k ++)
    {
        sum_coeff += ROUND_F(pSharpV1->kernel_pbf[k] * (1 << reg_sharpenHW_pPBfCoeff_fix_bits));
        sum_coeff_float += pSharpV1->kernel_pbf[k];
    }
    offset = int(sum_coeff_float * (1 << reg_sharpenHW_pPBfCoeff_fix_bits)) - sum_coeff;
    pSharpCfg->pbf_k[0] = pSharpCfg->pbf_k[0] + offset;


    //0x0094 - 0x0098
    sum_coeff = 0;
    sum_coeff_float = 0;
    pSharpCfg->mrf_k[0] = (unsigned char)ROUND_F(pSharpV1->h_rf_m[12] * (1 << reg_sharpenHW_pMRfCoeff_fix_bits));
    pSharpCfg->mrf_k[1] = (unsigned char)ROUND_F(pSharpV1->h_rf_m[7] * (1 << reg_sharpenHW_pMRfCoeff_fix_bits));
    pSharpCfg->mrf_k[2] = (unsigned char)ROUND_F(pSharpV1->h_rf_m[6] * (1 << reg_sharpenHW_pMRfCoeff_fix_bits));
    pSharpCfg->mrf_k[3] = (unsigned char)ROUND_F(pSharpV1->h_rf_m[2] * (1 << reg_sharpenHW_pMRfCoeff_fix_bits));
    pSharpCfg->mrf_k[4] = (unsigned char)ROUND_F(pSharpV1->h_rf_m[1] * (1 << reg_sharpenHW_pMRfCoeff_fix_bits));
    pSharpCfg->mrf_k[5] = (unsigned char)ROUND_F(pSharpV1->h_rf_m[0] * (1 << reg_sharpenHW_pMRfCoeff_fix_bits));
    for(k = 0; k < RKSHAPRENHW_MRF_DIAM * RKSHAPRENHW_MRF_DIAM; k ++)
    {
        sum_coeff += ROUND_F(pSharpV1->h_rf_m[k] * (1 << reg_sharpenHW_pMRfCoeff_fix_bits));
        sum_coeff_float += pSharpV1->h_rf_m[k];
    }
    offset = int(sum_coeff_float * (1 << reg_sharpenHW_pMRfCoeff_fix_bits)) - sum_coeff;
    pSharpCfg->mrf_k[0] = pSharpCfg->mrf_k[0] + offset;

    //0x009c -0x00a4
    sum_coeff = 0;
    sum_coeff_float = 0;
    pSharpCfg->mbf_k[0] = (unsigned char)ROUND_F(pSharpV1->kernel_mbf[110] * (1 << reg_sharpenHW_pMBfCoeff_fix_bits));
    pSharpCfg->mbf_k[1] = (unsigned char)ROUND_F(pSharpV1->kernel_mbf[93] * (1 << reg_sharpenHW_pMBfCoeff_fix_bits));
    pSharpCfg->mbf_k[2] = (unsigned char)ROUND_F(pSharpV1->kernel_mbf[92] * (1 << reg_sharpenHW_pMBfCoeff_fix_bits));
    pSharpCfg->mbf_k[3] = (unsigned char)ROUND_F(pSharpV1->kernel_mbf[76] * (1 << reg_sharpenHW_pMBfCoeff_fix_bits));
    pSharpCfg->mbf_k[4] = (unsigned char)ROUND_F(pSharpV1->kernel_mbf[58] * (1 << reg_sharpenHW_pMBfCoeff_fix_bits));
    pSharpCfg->mbf_k[5] = (unsigned char)ROUND_F(pSharpV1->kernel_mbf[56] * (1 << reg_sharpenHW_pMBfCoeff_fix_bits));
    pSharpCfg->mbf_k[6] = (unsigned char)ROUND_F(pSharpV1->kernel_mbf[25] * (1 << reg_sharpenHW_pMBfCoeff_fix_bits));
    pSharpCfg->mbf_k[7] = (unsigned char)ROUND_F(pSharpV1->kernel_mbf[23] * (1 << reg_sharpenHW_pMBfCoeff_fix_bits));
    pSharpCfg->mbf_k[8] = (unsigned char)ROUND_F(pSharpV1->kernel_mbf[38] * (1 << reg_sharpenHW_pMBfCoeff_fix_bits));
    pSharpCfg->mbf_k[9] = (unsigned char)ROUND_F(pSharpV1->kernel_mbf[4] * (1 << reg_sharpenHW_pMBfCoeff_fix_bits));
    pSharpCfg->mbf_k[10] = (unsigned char)ROUND_F(pSharpV1->kernel_mbf[69] * (1 << reg_sharpenHW_pMBfCoeff_fix_bits));
    pSharpCfg->mbf_k[11] = (unsigned char)ROUND_F(pSharpV1->kernel_mbf[102] * (1 << reg_sharpenHW_pMBfCoeff_fix_bits));
    for(k = 0; k < RKSHAPRENHW_MBF_DIAM_Y * RKSHAPRENHW_MBF_DIAM_X; k ++)
    {
        sum_coeff += ROUND_F(pSharpV1->kernel_mbf[k] * (1 << reg_sharpenHW_pMBfCoeff_fix_bits));
        sum_coeff_float += pSharpV1->kernel_mbf[k];
    }
    offset = int(sum_coeff_float * (1 << reg_sharpenHW_pMBfCoeff_fix_bits)) - sum_coeff;
    pSharpCfg->mbf_k[0] = pSharpCfg->mbf_k[0] + offset;

    //0x00a8 -0x00ac
    sum_coeff = 0;
    sum_coeff_float = 0;
    pSharpCfg->hrf_k[0] = (unsigned char)ROUND_F(pSharpV1->h_rf_h[12] * (1 << reg_sharpenHW_pHRfCoeff_fix_bits));
    pSharpCfg->hrf_k[1] = (unsigned char)ROUND_F(pSharpV1->h_rf_h[7] * (1 << reg_sharpenHW_pHRfCoeff_fix_bits));
    pSharpCfg->hrf_k[2] = (unsigned char)ROUND_F(pSharpV1->h_rf_h[6] * (1 << reg_sharpenHW_pHRfCoeff_fix_bits));
    pSharpCfg->hrf_k[3] = (unsigned char)ROUND_F(pSharpV1->h_rf_h[2] * (1 << reg_sharpenHW_pHRfCoeff_fix_bits));
    pSharpCfg->hrf_k[4] = (unsigned char)ROUND_F(pSharpV1->h_rf_h[1] * (1 << reg_sharpenHW_pHRfCoeff_fix_bits));
    pSharpCfg->hrf_k[5] = (unsigned char)ROUND_F(pSharpV1->h_rf_h[0] * (1 << reg_sharpenHW_pHRfCoeff_fix_bits));
    for(k = 0; k < RKSHAPRENHW_HRF_DIAM * RKSHAPRENHW_HRF_DIAM; k ++)
    {
        sum_coeff += ROUND_F(pSharpV1->h_rf_h[k] * (1 << reg_sharpenHW_pHRfCoeff_fix_bits));
        sum_coeff_float += pSharpV1->h_rf_h[k];
    }
    offset = int(sum_coeff_float * (1 << reg_sharpenHW_pHRfCoeff_fix_bits)) - sum_coeff;
    pSharpCfg->hrf_k[0] = pSharpCfg->hrf_k[0] + offset;

    //0x00b0
    sum_coeff = 0;
    sum_coeff_float = 0;
    pSharpCfg->hbf_k[0] = (unsigned char)ROUND_F(pSharpV1->kernel_hbf[4] * (1 << reg_sharpenHW_pHBfCoeff_fix_bits));
    pSharpCfg->hbf_k[1] = (unsigned char)ROUND_F(pSharpV1->kernel_hbf[1] * (1 << reg_sharpenHW_pHBfCoeff_fix_bits));
    pSharpCfg->hbf_k[2] = (unsigned char)ROUND_F(pSharpV1->kernel_hbf[0] * (1 << reg_sharpenHW_pHBfCoeff_fix_bits));
    for(k = 0; k < RKSHAPRENHW_HBF_DIAM * RKSHAPRENHW_HBF_DIAM; k ++)
    {
        sum_coeff += ROUND_F(pSharpV1->kernel_hbf[k] * (1 << reg_sharpenHW_pHBfCoeff_fix_bits));
        sum_coeff_float += pSharpV1->kernel_hbf[k];
    }
    offset = int(sum_coeff_float * (1 << reg_sharpenHW_pHBfCoeff_fix_bits)) - sum_coeff;
    pSharpCfg->hbf_k[0] = pSharpCfg->hbf_k[0] + offset;

    //0x00b4

    //0x00b8

    //0x00bc - 0x00c0

    //0x00c4 - 0x00c8

    //0x00cc - 0x00d0
    for(i = 0; i < 6; i++) {
        pSharpCfg->lum_point[i] = (unsigned char)ROUND_F(pSharpV1->luma_point[i + 1]);
    }

    //0x00d4
    //pbf
    int sigma_deci_bits = 7;
    int sigma_inte_bits = 1;
    int max_val         = 0;
    int min_val         = 65536;
    short sigma_bits[3];
    for(int i = 0; i < 8; i++)
    {
        int cur_sigma = FLOOR((pSharpV1->luma_sigma[i] * pSharpV1->pbf_gain + pSharpV1->pbf_add));
        if(max_val < cur_sigma)
            max_val = cur_sigma;
        if(min_val > cur_sigma)
            min_val = cur_sigma;
    }
    sigma_bits[0] = FLOOR(log((float)min_val) / log((float)2));
    sigma_bits[1] = MAX(sigma_inte_bits - sigma_bits[0], 0);
    sigma_bits[2] = sigma_deci_bits + sigma_bits[0];
    pSharpCfg->pbf_shf_bits = sigma_bits[2] - 5;

    // mf bf sigma inv
    max_val = 0;
    min_val = 65536;
    for(int i = 0; i < 8; i++)
    {
        int cur_sigma = FLOOR(pSharpV1->luma_sigma[i]
                              * pSharpV1->mbf_gain
                              + pSharpV1->mbf_add);
        if(max_val < cur_sigma)
            max_val = cur_sigma;
        if(min_val > cur_sigma)
            min_val = cur_sigma;
    }
    sigma_bits[0] = FLOOR(log((float)min_val) / log((float)2));
    sigma_bits[1] = MAX(sigma_inte_bits - sigma_bits[0], 0);
    sigma_bits[2] = sigma_deci_bits + sigma_bits[0];
    pSharpCfg->mbf_shf_bits = (unsigned char)(sigma_bits[2] - 5);

    // hf bf sigma inv
    max_val = 0;
    min_val = 65536;
    for(int i = 0; i < 8; i++)
    {
        int cur_sigma = FLOOR(pSharpV1->luma_sigma[i] * pSharpV1->hbf_gain + pSharpV1->hbf_add);
        if(max_val < cur_sigma)
            max_val = cur_sigma;
        if(min_val > cur_sigma)
            min_val = cur_sigma;
    }
    sigma_bits[0] = FLOOR(log((float)min_val) / log((float)2));
    sigma_bits[1] = MAX(sigma_inte_bits - sigma_bits[0], 0);
    sigma_bits[2] = sigma_deci_bits + sigma_bits[0];
    pSharpCfg->hbf_shf_bits = (unsigned char)(sigma_bits[2] - 5);


    //0x00d8 - 0x00dc
    // pre bf sigma inv
    sigma_deci_bits = 7;
    sigma_inte_bits = 1;
    max_val         = 0;
    min_val         = 65536;
    for(int i = 0; i < RK_SHARPFILTER_LUMA_POINT_NUM; i++)
    {
        int cur_sigma = FLOOR((pSharpV1->luma_sigma[i] * pSharpV1->pbf_gain + pSharpV1->pbf_add));
        if(max_val < cur_sigma)
            max_val = cur_sigma;
        if(min_val > cur_sigma)
            min_val = cur_sigma;
    }
    sigma_bits[0] = FLOOR(log((float)min_val) / log((float)2));
    sigma_bits[1] = MAX(sigma_inte_bits - sigma_bits[0], 0);
    sigma_bits[2] = sigma_deci_bits + sigma_bits[0];

    for(i = 0; i < 8; i++) {
        pSharpCfg->pbf_sigma[i] = (unsigned char)ROUND_F((float)1
                                  / (pSharpV1->luma_sigma[i] * pSharpV1->pbf_gain + pSharpV1->pbf_add)
                                  * (1 << sigma_bits[2]));
    }

    //0x00e0 - 0x00e4
    for(i = 0; i < 8; i++) {
        tmp = ((pSharpV1->lum_clp_m[i]) * mClipStrength);
        if(tmp > 0xff) {
            tmp = 0xff;
        }
        pSharpCfg->lum_clp_m[i] = (unsigned char)tmp;
    }

    //0x00e8 - 0x00ec
    for(i = 0; i < 8; i++) {
        tmp = ROUND_F(pSharpV1->lum_min_m[i] * mClipStrength * (1 << reg_sharpenHW_lum_min_m_fix_bits));
        if(tmp < -32) {
            tmp = -32;
        }
        pSharpCfg->lum_min_m[i] = (char)tmp;
    }

    //0x00f0 - 0x00f4


    // mf bf sigma inv
    max_val = 0;
    min_val = 65536;
    for(int i = 0; i < RK_SHARPFILTER_LUMA_POINT_NUM; i++)
    {
        int cur_sigma = FLOOR(pSharpV1->luma_sigma[i] * pSharpV1->mbf_gain + pSharpV1->mbf_add);
        if(max_val < cur_sigma)
            max_val = cur_sigma;
        if(min_val > cur_sigma)
            min_val = cur_sigma;
    }
    sigma_bits[0] = FLOOR(log((float)min_val) / log((float)2));
    sigma_bits[1] = MAX(sigma_inte_bits - sigma_bits[0], 0);
    sigma_bits[2] = sigma_deci_bits + sigma_bits[0];

    for(i = 0; i < 8; i++) {
        pSharpCfg->mbf_sigma[i] = (unsigned char)ROUND_F((float)1
                                  / (pSharpV1->luma_sigma[i] * pSharpV1->mbf_gain + pSharpV1->mbf_add)
                                  * (1 << sigma_bits[2]));
    }

    //0x00f8 - 0x00fc
    for(i = 0; i < 8; i++) {
        tmp = (pSharpV1->lum_clp_h[i] * hClipStrength);
        if(tmp > 0xff) {
            tmp = 0xff;
        }
        pSharpCfg->lum_clp_h[i] = (unsigned char)tmp;
    }

    //0x0100 - 0x0104
    //hbf

    max_val = 0;
    min_val = 65536;
    for(int i = 0; i < RK_SHARPFILTER_LUMA_POINT_NUM; i++)
    {
        int cur_sigma = FLOOR(pSharpV1->luma_sigma[i] * pSharpV1->hbf_gain + pSharpV1->hbf_add);
        if(max_val < cur_sigma)
            max_val = cur_sigma;
        if(min_val > cur_sigma)
            min_val = cur_sigma;
    }
    sigma_bits[0] = FLOOR(log((float)min_val) / log((float)2));
    sigma_bits[1] = MAX(sigma_inte_bits - sigma_bits[0], 0);
    sigma_bits[2] = sigma_deci_bits + sigma_bits[0];

    for(i = 0; i < 8; i++) {
        pSharpCfg->hbf_sigma[i] = (unsigned char)ROUND_F((float)1
                                  / (pSharpV1->luma_sigma[i] * pSharpV1->hbf_gain + pSharpV1->hbf_add)
                                  * (1 << sigma_bits[2]));
    }

    //0x0128
    tmp = ROUND_F(pSharpV1->lratio /  fPercent * (1 << reg_sharpenHW_lratio_fix_bits));
    if(tmp > 0x100) {
        tmp = 0x100;
    }
    pSharpCfg->rfl_ratio = (unsigned short) tmp;

    tmp = ROUND_F(pSharpV1->hratio * fPercent * (1 << reg_sharpenHW_hratio_fix_bits));
    if(tmp > 0x3ff) {
        tmp = 0x3ff;
    }
    if(tmp < 0x100) {
        tmp = 0x100;
    }
    pSharpCfg->rfh_ratio = (unsigned short) tmp;

    //0x012C
    pSharpCfg->m_ratio = (unsigned char)ROUND_F(pSharpV1->M_ratio * fPercent * (1 << reg_sharpenHW_M_ratio_fix_bits));
    if(pSharpCfg->m_ratio > 0x1f) {
        pSharpCfg->m_ratio = 0x1f;
    }
    pSharpCfg->h_ratio = (unsigned char)ROUND_F(pSharpV1->H_ratio *  fPercent * (1 << reg_sharpenHW_H_ratio_fix_bits));
    if(pSharpCfg->h_ratio > 0x1f) {
        pSharpCfg->h_ratio = 0x1f;
    }


    return res;
}

AsharpResult_t rk_Sharp_fix_transfer(RKAsharp_Sharp_Params_Select_t* sharp, RKAsharp_Sharp_Fix_t* pSharpCfg, float fPercent)
{
    AsharpResult_t res = ASHARP_RET_SUCCESS;

    if(sharp == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(pSharpCfg == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    rk_Sharp_V1_fix_transfer(&sharp->rk_sharpen_params_selected_V1, &pSharpCfg->stSharpFixV1, fPercent);

    return res;
}

AsharpResult_t sharp_calibdbV2_assign(CalibDbV2_SharpV1_t *pDst, CalibDbV2_SharpV1_t *pSrc)
{
    AsharpResult_t res = ASHARP_RET_SUCCESS;
    CalibDbV2_SharpV1_Tuning_t *pSrcTuningParaV2 = NULL;
    CalibDbV2_SharpV1_Tuning_t *pDstTuningParaV2 = NULL;
    int setting_len = 0;
    int iso_len = 0;


    if(pDst == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    if(pSrc == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }
    sharp_calibdbV2_free(pDst);

    pSrcTuningParaV2 = &pSrc->TuningPara;
    pDstTuningParaV2 = &pDst->TuningPara;

    //assign the value
    pDst->Version = strdup(pSrc->Version);
    pDstTuningParaV2->enable = pSrcTuningParaV2->enable;

    //malloc iso size
    setting_len = pSrcTuningParaV2->Setting_len;
    pDstTuningParaV2->Setting = (CalibDbV2_SharpV1_T_Set_t *)malloc(setting_len * sizeof(CalibDbV2_SharpV1_T_Set_t));
    memset(pDstTuningParaV2->Setting, 0x00, setting_len * sizeof(CalibDbV2_SharpV1_T_Set_t));
    pDstTuningParaV2->Setting_len = setting_len;

    for(int i = 0; i < setting_len; i++) {
        iso_len = pSrcTuningParaV2->Setting[i].Tuning_ISO_len;
        pDstTuningParaV2->Setting[i].Tuning_ISO = (CalibDbV2_SharpV1_T_ISO_t *)malloc(iso_len * sizeof(CalibDbV2_SharpV1_T_ISO_t));
        memset(pDstTuningParaV2->Setting[i].Tuning_ISO, 0x00, iso_len * sizeof(CalibDbV2_SharpV1_T_ISO_t));
        pDstTuningParaV2->Setting[i].Tuning_ISO_len = iso_len;
    }

    for(int i = 0; i < setting_len; i++) {
        iso_len = pSrcTuningParaV2->Setting[i].Tuning_ISO_len;
        pDstTuningParaV2->Setting[i].SNR_Mode = strdup(pSrcTuningParaV2->Setting[i].SNR_Mode);
        pDstTuningParaV2->Setting[i].Sensor_Mode = strdup(pSrcTuningParaV2->Setting[i].Sensor_Mode);

        for(int j = 0; j < iso_len; j++) {
            pDstTuningParaV2->Setting[i].Tuning_ISO[j] = pSrcTuningParaV2->Setting[i].Tuning_ISO[j];
        }
    }

    pDstTuningParaV2->kernel_coeff = pSrcTuningParaV2->kernel_coeff;

    return res;

}



void sharp_calibdbV2_free(CalibDbV2_SharpV1_t *pCalibdbV2)
{
    if(pCalibdbV2) {
        if(pCalibdbV2->Version) {
            free(pCalibdbV2->Version);
        }

        if(pCalibdbV2->TuningPara.Setting) {
            for(int i = 0; i < pCalibdbV2->TuningPara.Setting_len; i++) {
                if(pCalibdbV2->TuningPara.Setting[i].SNR_Mode) {
                    free(pCalibdbV2->TuningPara.Setting[i].SNR_Mode);
                }

                if(pCalibdbV2->TuningPara.Setting[i].Sensor_Mode) {
                    free(pCalibdbV2->TuningPara.Setting[i].Sensor_Mode);
                }

                if(pCalibdbV2->TuningPara.Setting[i].Tuning_ISO) {
                    free(pCalibdbV2->TuningPara.Setting[i].Tuning_ISO);
                }
            }

            free(pCalibdbV2->TuningPara.Setting);
        }
    }
}


RKAIQ_END_DECLARE

