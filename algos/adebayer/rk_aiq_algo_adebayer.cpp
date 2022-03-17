
#include "rk_aiq_types_algo_adebayer_prvt.h"

#ifndef MIN
#define MIN(a,b)             ((a) <= (b) ? (a):(b))
#endif
#ifndef MAX
#define MAX(a,b)             ((a) >= (b) ? (a):(b))
#endif

static int
calibDbParamConvertion
(
    const CamCalibDbContext_t *pCalib,
    AdebayerConfig_t *config, int iso
)
{
    const CalibDb_RKDM_t *calib_dm =
        (CalibDb_RKDM_t*)(CALIBDB_GET_MODULE_PTR((void*)pCalib, dm));

    for (int i = 0; i < 5; i++)
    {
        config->filter1_coe[i] = calib_dm->debayer_filter1[i];
        config->filter2_coe[i] = calib_dm->debayer_filter2[i];
    }
    config->gain_offset = calib_dm->debayer_gain_offset;

    int sharp_strength_tmp[9];

    for (int i = 0; i < 9; i ++)
    {
        float iso_index             = calib_dm->ISO[i];
        int gain                    = (int)(log((float)iso_index / 50) / log((float)2));
        sharp_strength_tmp[gain]    = calib_dm->sharp_strength[i];
    }
    config->offset = calib_dm->debayer_offset;

    int hfOffset_tmp[9];
    for (int i = 0; i < 9; i ++)
    {
        float iso_index             = calib_dm->ISO[i];
        int gain                    = (int)(log((float)iso_index / 50) / log((float)2));
        hfOffset_tmp[gain]  = calib_dm->debayer_hf_offset[i];
    }
    config->clip_en = calib_dm->debayer_clip_en;
    config->filter_g_en = calib_dm->debayer_filter_g_en;
    config->filter_c_en = calib_dm->debayer_filter_c_en;
    config->thed0 = calib_dm->debayer_thed0;
    config->thed1 = calib_dm->debayer_thed1;
    config->dist_scale = calib_dm->debayer_dist_scale;

    unsigned char false_color_remove_strength_table[10][2] = {
        {0, 19},
        {1, 18},
        {2, 17},
        {3, 16},
        {4, 15},
        {5, 14},
        {6, 13},
        {7, 12},
        {8, 11},
        {9, 10}
    };

    int index = calib_dm->debayer_cnr_strength;
    config->order_min = false_color_remove_strength_table[index][0];
    config->order_max = false_color_remove_strength_table[index][1];
    config->shift_num = calib_dm->debayer_shift_num;
    //select sharp params
    int iso_low, iso_high;
    int gain_high, gain_low;
    float ratio;
    int iso_div             = 50;
    int max_iso_step        = 9;
    for (int i = max_iso_step - 1; i >= 0; i--)
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

    gain_low        = MIN(MAX(gain_low, 0), 8);
    gain_high       = MIN(MAX(gain_high, 0), 8);

    config->max_ratio = ((ratio) * (sharp_strength_tmp[gain_high] - sharp_strength_tmp[gain_low]) + sharp_strength_tmp[gain_low]);
    config->hf_offset  = ((ratio) * (hfOffset_tmp[gain_high] - hfOffset_tmp[gain_low]) + hfOffset_tmp[gain_low]);

    return 0;

}

static int
AdebayerFullParamsInit
(
    AdebayerContext_t *pAdebayerCtx
)
{
    if (pAdebayerCtx->pCalibDb) {
        const CalibDb_RKDM_t *calib_dm =
            (CalibDb_RKDM_t*)(CALIBDB_GET_MODULE_PTR(pAdebayerCtx->pCalibDb, dm));

        pAdebayerCtx->full_param.enable = calib_dm->debayer_en;
        for (int i = 0; i < 9; i++) {
            pAdebayerCtx->full_param.iso[i] = calib_dm->ISO[i];
            pAdebayerCtx->full_param.hf_offset[i] = calib_dm->debayer_hf_offset[i];
            pAdebayerCtx->full_param.sharp_strength[i] = calib_dm->sharp_strength[i];
        }
        for (int i = 0; i < 5; i++) {
            pAdebayerCtx->full_param.filter1[i] = calib_dm->debayer_filter1[i];
            pAdebayerCtx->full_param.filter2[i] = calib_dm->debayer_filter2[i];
        }
        pAdebayerCtx->full_param.clip_en = calib_dm->debayer_clip_en;
        pAdebayerCtx->full_param.filter_g_en = calib_dm->debayer_filter_g_en;
        pAdebayerCtx->full_param.filter_c_en = calib_dm->debayer_filter_c_en;
        pAdebayerCtx->full_param.thed0 = calib_dm->debayer_thed0;
        pAdebayerCtx->full_param.thed1 = calib_dm->debayer_thed1;
        pAdebayerCtx->full_param.dist_scale = calib_dm->debayer_dist_scale;
        pAdebayerCtx->full_param.gain_offset = calib_dm->debayer_gain_offset;
        pAdebayerCtx->full_param.offset = calib_dm->debayer_offset;
        pAdebayerCtx->full_param.shift_num = calib_dm->debayer_shift_num;
        pAdebayerCtx->full_param.cnr_strength = calib_dm->debayer_cnr_strength;
    } else if (pAdebayerCtx->pCalibDbV2) {
        CalibDbV2_Debayer_t* debayer =
                (CalibDbV2_Debayer_t*)(CALIBDBV2_GET_MODULE_PTR(pAdebayerCtx->pCalibDbV2, debayer));
        pAdebayerCtx->full_param.enable = debayer->param.debayer_en;
        for (int i = 0; i < 9; i++) {
            pAdebayerCtx->full_param.iso[i] = debayer->param.array.ISO[i];
            pAdebayerCtx->full_param.hf_offset[i] = debayer->param.array.debayer_hf_offset[i];
            pAdebayerCtx->full_param.sharp_strength[i] = debayer->param.array.sharp_strength[i];
        }
        for (int i = 0; i < 5; i++) {
            pAdebayerCtx->full_param.filter1[i] = debayer->param.debayer_filter1[i];
            pAdebayerCtx->full_param.filter2[i] = debayer->param.debayer_filter2[i];
        }
        pAdebayerCtx->full_param.clip_en = debayer->param.debayer_clip_en;
        pAdebayerCtx->full_param.filter_g_en = debayer->param.debayer_filter_g_en;
        pAdebayerCtx->full_param.filter_c_en = debayer->param.debayer_filter_c_en;
        pAdebayerCtx->full_param.thed0 = debayer->param.debayer_thed0;
        pAdebayerCtx->full_param.thed1 = debayer->param.debayer_thed1;
        pAdebayerCtx->full_param.dist_scale = debayer->param.debayer_dist_scale;
        pAdebayerCtx->full_param.gain_offset = debayer->param.debayer_gain_offset;
        pAdebayerCtx->full_param.offset = debayer->param.debayer_offset;
        pAdebayerCtx->full_param.shift_num = debayer->param.debayer_shift_num;
        pAdebayerCtx->full_param.cnr_strength = debayer->param.debayer_cnr_strength;
        pAdebayerCtx->full_param.updated = false;
    } else {
        LOGE_ADEBAYER("%s(%d): calibDb are all null!\n", __FUNCTION__, __LINE__);
    }

    return 0;
}

//debayer inint
XCamReturn
AdebayerInit
(
    AdebayerContext_t *pAdebayerCtx,
    CamCalibDbContext_t *pCalibDb,
    CamCalibDbV2Context_t *pCalibDbV2
)
{
    LOGI_ADEBAYER("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAdebayerCtx == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    memset(pAdebayerCtx, 0, sizeof(AdebayerContext_t));
    pAdebayerCtx->pCalibDb = pCalibDb;
    pAdebayerCtx->pCalibDbV2 = pCalibDbV2;
    AdebayerFullParamsInit(pAdebayerCtx);
    pAdebayerCtx->state = ADEBAYER_STATE_INITIALIZED;

    LOGI_ADEBAYER("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;
}

//debayer release
XCamReturn
AdebayerRelease
(
    AdebayerContext_t *pAdebayerCtx
)
{
    LOGI_ADEBAYER("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAdebayerCtx == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    AdebayerStop(pAdebayerCtx);

    LOGI_ADEBAYER("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;
}

//debayer config
XCamReturn
AdebayerConfig
(
    AdebayerContext_t *pAdebayerCtx,
    AdebayerConfig_t* pAdebayerConfig
)
{
    LOGI_ADEBAYER("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAdebayerCtx == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if(pAdebayerConfig == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    //TO DO

    LOGI_ADEBAYER("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
AdebayerStart
(
    AdebayerContext_t *pAdebayerCtx
)
{
    LOGI_ADEBAYER("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAdebayerCtx == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    pAdebayerCtx->state = ADEBAYER_STATE_RUNNING;
    LOGI_ADEBAYER("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
AdebayerStop
(
    AdebayerContext_t *pAdebayerCtx
)
{
    LOGI_ADEBAYER("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAdebayerCtx == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    pAdebayerCtx->state = ADEBAYER_STATE_STOPPED;
    LOGI_ADEBAYER("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;
}

//debayer reconfig
XCamReturn
AdebayerReConfig
(
    AdebayerContext_t* pAdebayerCtx,
    AdebayerConfig_t*  pAdebayerConfig
)
{
    LOGI_ADEBAYER("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ADEBAYER("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;
}

//debayer preprocess
XCamReturn AdebayerPreProcess(AdebayerContext_t *pAdebayerCtx)
{
    LOGI_ADEBAYER("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ADEBAYER("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;
}

//debayer process
XCamReturn
AdebayerProcess
(
    AdebayerContext_t *pAdebayerCtx,
    int ISO
)
{
    LOGI_ADEBAYER("%s(%d): enter! ISO=%d\n", __FUNCTION__, __LINE__, ISO);

    if(pAdebayerCtx == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    pAdebayerCtx->config.enable = pAdebayerCtx->full_param.enable;
    for (int i = 0; i < 5; i++)
    {
        pAdebayerCtx->config.filter1_coe[i] = pAdebayerCtx->full_param.filter1[i];
        pAdebayerCtx->config.filter2_coe[i] = pAdebayerCtx->full_param.filter2[i];
    }
    pAdebayerCtx->config.gain_offset = pAdebayerCtx->full_param.gain_offset;

    int sharp_strength_tmp[9];

    for (int i = 0; i < 9; i ++)
    {
        float iso_index = pAdebayerCtx->full_param.iso[i];
        int gain = (int)(log((float)iso_index / 50) / log((float)2));
        sharp_strength_tmp[gain] = pAdebayerCtx->full_param.sharp_strength[i];
    }
    pAdebayerCtx->config.offset = pAdebayerCtx->full_param.offset;

    int hfOffset_tmp[9];
    for (int i = 0; i < 9; i ++)
    {
        float iso_index = pAdebayerCtx->full_param.iso[i];
        int gain = (int)(log((float)iso_index / 50) / log((float)2));
        hfOffset_tmp[gain]  = pAdebayerCtx->full_param.hf_offset[i];
    }
    pAdebayerCtx->config.clip_en = pAdebayerCtx->full_param.clip_en;
    pAdebayerCtx->config.filter_g_en = pAdebayerCtx->full_param.filter_g_en;
    pAdebayerCtx->config.filter_c_en = pAdebayerCtx->full_param.filter_c_en;
    pAdebayerCtx->config.thed0 = pAdebayerCtx->full_param.thed0;
    pAdebayerCtx->config.thed1 = pAdebayerCtx->full_param.thed1;
    pAdebayerCtx->config.dist_scale = pAdebayerCtx->full_param.dist_scale;

    unsigned char false_color_remove_strength_table[10][2] = {
        {0, 19},
        {1, 18},
        {2, 17},
        {3, 16},
        {4, 15},
        {5, 14},
        {6, 13},
        {7, 12},
        {8, 11},
        {9, 10}
    };

    int index = pAdebayerCtx->full_param.cnr_strength;
    pAdebayerCtx->config.order_min = false_color_remove_strength_table[index][0];
    pAdebayerCtx->config.order_max = false_color_remove_strength_table[index][1];
    pAdebayerCtx->config.shift_num = pAdebayerCtx->full_param.shift_num;
    //select sharp params
    int iso_low = ISO, iso_high = ISO;
    int gain_high, gain_low;
    float ratio = 0.0f;
    int iso_div             = 50;
    int max_iso_step        = 9;
    for (int i = max_iso_step - 1; i >= 0; i--)
    {
        if (ISO < iso_div * (2 << i))
        {
            iso_low = iso_div * (2 << (i)) / 2;
            iso_high = iso_div * (2 << i);
        }
    }
    ratio = (float)(ISO - iso_low) / (iso_high - iso_low);
    if (iso_low == ISO)
    {
        iso_high = ISO;
        ratio = 0;
    }
    if (iso_high == ISO )
    {
        iso_low = ISO;
        ratio = 1;
    }
    gain_high = (int)(log((float)iso_high / 50) / log((float)2));
    gain_low = (int)(log((float)iso_low / 50) / log((float)2));

    gain_low = MIN(MAX(gain_low, 0), 8);
    gain_high = MIN(MAX(gain_high, 0), 8);

    pAdebayerCtx->config.max_ratio = ((ratio) * (sharp_strength_tmp[gain_high] - sharp_strength_tmp[gain_low]) + sharp_strength_tmp[gain_low]);
    pAdebayerCtx->config.hf_offset = ((ratio) * (hfOffset_tmp[gain_high] - hfOffset_tmp[gain_low]) + hfOffset_tmp[gain_low]);

    LOGI_ADEBAYER("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;

}

//debayer get result
XCamReturn
AdebayerGetProcResult
(
    AdebayerContext_t*    pAdebayerCtx,
    AdebayerProcResult_t* pAdebayerResult
)
{
    LOGI_ADEBAYER("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAdebayerCtx == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if(pAdebayerResult == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    pAdebayerResult->config = pAdebayerCtx->config;
    pAdebayerCtx->config.updatecfg = false;
    LOGI_ADEBAYER("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;
}



