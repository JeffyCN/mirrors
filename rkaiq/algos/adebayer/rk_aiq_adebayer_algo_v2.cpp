
#include "rk_aiq_adebayer_algo_v2.h"

#ifndef MIN
#define MIN(a,b)             ((a) <= (b) ? (a):(b))
#endif
#ifndef MAX
#define MAX(a,b)             ((a) >= (b) ? (a):(b))
#endif

XCamReturn
AdebayerCalibConfig
(
    AdebayerContext_t *pAdebayerCtx,
    CamCalibDbV2Context_t *pCalibDbV2
)
{
    LOGV_ADEBAYER("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAdebayerCtx == NULL) {
        LOGE_ADEBAYER("%s(%d): null pAdebayerCtx pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if(pCalibDbV2 == NULL) {
        LOGE_ADEBAYER("%s(%d): null pCalibDbV2 pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (pCalibDbV2) {

#if RKAIQ_HAVE_DEBAYER_V2

        CalibDbV2_Debayer_v2_t* debayer =
            (CalibDbV2_Debayer_v2_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, debayer_v2));

        pAdebayerCtx->full_param_v2.debayer_en = debayer->param.debayer_en;

        pAdebayerCtx->full_param_v2.lowfreq_filter1[0] = debayer->param.lowfreq_filter1[0];
        pAdebayerCtx->full_param_v2.lowfreq_filter1[1] = debayer->param.lowfreq_filter1[1];
        pAdebayerCtx->full_param_v2.lowfreq_filter1[2] = debayer->param.lowfreq_filter1[2];
        pAdebayerCtx->full_param_v2.lowfreq_filter1[3] = debayer->param.lowfreq_filter1[3];

        pAdebayerCtx->full_param_v2.highfreq_filter2[0] = debayer->param.highfreq_filter2[0];
        pAdebayerCtx->full_param_v2.highfreq_filter2[1] = debayer->param.highfreq_filter2[1];
        pAdebayerCtx->full_param_v2.highfreq_filter2[2] = debayer->param.highfreq_filter2[2];
        pAdebayerCtx->full_param_v2.highfreq_filter2[3] = debayer->param.highfreq_filter2[3];

        pAdebayerCtx->full_param_v2.c_alpha_gaus_coe[0] = debayer->param.c_alpha_gaus_coe[0];
        pAdebayerCtx->full_param_v2.c_alpha_gaus_coe[1] = debayer->param.c_alpha_gaus_coe[1];
        pAdebayerCtx->full_param_v2.c_alpha_gaus_coe[2] = debayer->param.c_alpha_gaus_coe[2];

        pAdebayerCtx->full_param_v2.c_ce_gaus_coe[0] = debayer->param.c_ce_gaus_coe[0];
        pAdebayerCtx->full_param_v2.c_ce_gaus_coe[1] = debayer->param.c_ce_gaus_coe[1];
        pAdebayerCtx->full_param_v2.c_ce_gaus_coe[2] = debayer->param.c_ce_gaus_coe[2];

        pAdebayerCtx->full_param_v2.c_guid_gaus_coe[0] = debayer->param.c_guid_gaus_coe[0];
        pAdebayerCtx->full_param_v2.c_guid_gaus_coe[1] = debayer->param.c_guid_gaus_coe[1];
        pAdebayerCtx->full_param_v2.c_guid_gaus_coe[2] = debayer->param.c_guid_gaus_coe[2];

        for(int i = 0; i < RK_DEBAYER_ISO_STEP_MAX; i++) {

            //g_interp
            pAdebayerCtx->full_param_v2.g_interp.iso[i] = debayer->param.g_interp.iso[i];
            pAdebayerCtx->full_param_v2.g_interp.debayer_clip_en[i] = debayer->param.g_interp.debayer_clip_en[i];
            pAdebayerCtx->full_param_v2.g_interp.debayer_gain_offset[i] = debayer->param.g_interp.debayer_gain_offset[i];
            pAdebayerCtx->full_param_v2.g_interp.debayer_max_ratio[i] = debayer->param.g_interp.debayer_max_ratio[i];

            //g_drctwgt

            pAdebayerCtx->full_param_v2.g_drctwgt.iso[i] = debayer->param.g_drctwgt.iso[i];

            pAdebayerCtx->full_param_v2.g_drctwgt.debayer_hf_offset[i] = debayer->param.g_drctwgt.debayer_hf_offset[i];
            pAdebayerCtx->full_param_v2.g_drctwgt.debayer_thed0[i] = debayer->param.g_drctwgt.debayer_thed0[i];
            pAdebayerCtx->full_param_v2.g_drctwgt.debayer_thed1[i] = debayer->param.g_drctwgt.debayer_thed1[i];
            pAdebayerCtx->full_param_v2.g_drctwgt.debayer_dist_scale[i] = debayer->param.g_drctwgt.debayer_dist_scale[i];
            pAdebayerCtx->full_param_v2.g_drctwgt.debayer_select_thed[i] = debayer->param.g_drctwgt.debayer_select_thed[i];

            //g_filter

            pAdebayerCtx->full_param_v2.g_filter.iso[i] = debayer->param.g_filter.iso[i];
            pAdebayerCtx->full_param_v2.g_filter.debayer_gfilter_en[i] = debayer->param.g_filter.debayer_gfilter_en[i];
            pAdebayerCtx->full_param_v2.g_filter.debayer_gfilter_offset[i] = debayer->param.g_filter.debayer_gfilter_offset[i];


            //c_filter

            pAdebayerCtx->full_param_v2.c_filter.iso[i] = debayer->param.c_filter.iso[i];
            pAdebayerCtx->full_param_v2.c_filter.debayer_cfilter_en[i] = debayer->param.c_filter.debayer_cfilter_en[i];

            pAdebayerCtx->full_param_v2.c_filter.debayer_alpha_offset[i] = debayer->param.c_filter.debayer_alpha_offset[i];
            pAdebayerCtx->full_param_v2.c_filter.debayer_alpha_scale[i] = debayer->param.c_filter.debayer_alpha_scale[i];
            pAdebayerCtx->full_param_v2.c_filter.debayer_edge_offset[i] = debayer->param.c_filter.debayer_edge_offset[i];
            pAdebayerCtx->full_param_v2.c_filter.debayer_edge_scale[i] = debayer->param.c_filter.debayer_edge_scale[i];

            pAdebayerCtx->full_param_v2.c_filter.debayer_bf_clip[i] = debayer->param.c_filter.debayer_bf_clip[i];
            pAdebayerCtx->full_param_v2.c_filter.debayer_bf_curwgt[i] = debayer->param.c_filter.debayer_bf_curwgt[i];
            pAdebayerCtx->full_param_v2.c_filter.debayer_bf_sgm[i] = debayer->param.c_filter.debayer_bf_sgm[i];

            pAdebayerCtx->full_param_v2.c_filter.debayer_cfilter_str[i] = debayer->param.c_filter.debayer_cfilter_str[i];
            pAdebayerCtx->full_param_v2.c_filter.debayer_wet_clip[i] = debayer->param.c_filter.debayer_wet_clip[i];
            pAdebayerCtx->full_param_v2.c_filter.debayer_wet_ghost[i] = debayer->param.c_filter.debayer_wet_ghost[i];
            pAdebayerCtx->full_param_v2.c_filter.debayer_wgtslope[i] = debayer->param.c_filter.debayer_wgtslope[i];

            pAdebayerCtx->full_param_v2.c_filter.debayer_loggd_offset[i] = debayer->param.c_filter.debayer_loggd_offset[i];
            pAdebayerCtx->full_param_v2.c_filter.debayer_loghf_offset[i] = debayer->param.c_filter.debayer_loghf_offset[i];

        }
#endif

#if RKAIQ_HAVE_DEBAYER_V2_LITE

        CalibDbV2_Debayer_v2_lite_t* debayer =
            (CalibDbV2_Debayer_v2_lite_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, debayer_v2_lite));

        pAdebayerCtx->full_param_v2_lite.debayer_en = debayer->param.debayer_en;

        pAdebayerCtx->full_param_v2_lite.lowfreq_filter1[0] = debayer->param.lowfreq_filter1[0];
        pAdebayerCtx->full_param_v2_lite.lowfreq_filter1[1] = debayer->param.lowfreq_filter1[1];
        pAdebayerCtx->full_param_v2_lite.lowfreq_filter1[2] = debayer->param.lowfreq_filter1[2];
        pAdebayerCtx->full_param_v2_lite.lowfreq_filter1[3] = debayer->param.lowfreq_filter1[3];

        pAdebayerCtx->full_param_v2_lite.highfreq_filter2[0] = debayer->param.highfreq_filter2[0];
        pAdebayerCtx->full_param_v2_lite.highfreq_filter2[1] = debayer->param.highfreq_filter2[1];
        pAdebayerCtx->full_param_v2_lite.highfreq_filter2[2] = debayer->param.highfreq_filter2[2];
        pAdebayerCtx->full_param_v2_lite.highfreq_filter2[3] = debayer->param.highfreq_filter2[3];

        for(int i = 0; i < RK_DEBAYER_ISO_STEP_MAX; i++) {

            //g_interp
            pAdebayerCtx->full_param_v2_lite.g_interp.iso[i] = debayer->param.g_interp.iso[i];
            pAdebayerCtx->full_param_v2_lite.g_interp.debayer_clip_en[i] = debayer->param.g_interp.debayer_clip_en[i];
            pAdebayerCtx->full_param_v2_lite.g_interp.debayer_gain_offset[i] = debayer->param.g_interp.debayer_gain_offset[i];
            pAdebayerCtx->full_param_v2_lite.g_interp.debayer_max_ratio[i] = debayer->param.g_interp.debayer_max_ratio[i];

            //g_drctwgt

            pAdebayerCtx->full_param_v2_lite.g_drctwgt.iso[i] = debayer->param.g_drctwgt.iso[i];

            pAdebayerCtx->full_param_v2_lite.g_drctwgt.debayer_hf_offset[i] = debayer->param.g_drctwgt.debayer_hf_offset[i];
            pAdebayerCtx->full_param_v2_lite.g_drctwgt.debayer_thed0[i] = debayer->param.g_drctwgt.debayer_thed0[i];
            pAdebayerCtx->full_param_v2_lite.g_drctwgt.debayer_thed1[i] = debayer->param.g_drctwgt.debayer_thed1[i];
            pAdebayerCtx->full_param_v2_lite.g_drctwgt.debayer_dist_scale[i] = debayer->param.g_drctwgt.debayer_dist_scale[i];
            pAdebayerCtx->full_param_v2_lite.g_drctwgt.debayer_select_thed[i] = debayer->param.g_drctwgt.debayer_select_thed[i];

            //g_filter

            pAdebayerCtx->full_param_v2_lite.g_filter.iso[i] = debayer->param.g_filter.iso[i];
            pAdebayerCtx->full_param_v2_lite.g_filter.debayer_gfilter_en[i] = debayer->param.g_filter.debayer_gfilter_en[i];
            pAdebayerCtx->full_param_v2_lite.g_filter.debayer_gfilter_offset[i] = debayer->param.g_filter.debayer_gfilter_offset[i];
        }
#endif
    }

    LOGV_ADEBAYER("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;
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
    LOGV_ADEBAYER("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(pAdebayerCtx == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    memset(&pAdebayerCtx->config, 0, sizeof(AdebayerHwConfigV2_t));

#if RKAIQ_HAVE_DEBAYER_V2
    memset(&pAdebayerCtx->full_param_v2, 0, sizeof(CalibDbV2_Debayer_Tuning_t));
    memset(&pAdebayerCtx->select_param_v2, 0, sizeof(AdebayerSeletedParamV2_t));
#endif

#if RKAIQ_HAVE_DEBAYER_V2_LITE
    memset(&pAdebayerCtx->full_param_v2_lite, 0, sizeof(CalibDbV2_Debayer_Tuning_Lite_t));
    memset(&pAdebayerCtx->select_param_v2_lite, 0, sizeof(AdebayerSeletedParamV2Lite_t));
#endif
    //copy json params
    ret = AdebayerCalibConfig(pAdebayerCtx, pCalibDbV2);

    //init other params
    pAdebayerCtx->mode = RK_AIQ_DEBAYER_MODE_AUTO;
    pAdebayerCtx->state = ADEBAYER_STATE_INITIALIZED;
    pAdebayerCtx->is_reconfig = false;

    LOGV_ADEBAYER("%s(%d): exit!\n", __FUNCTION__, __LINE__);
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

//debayer preprocess
XCamReturn AdebayerPreProcess(AdebayerContext_t *pAdebayerCtx)
{
    LOGI_ADEBAYER("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ADEBAYER("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AdebayerSelectParam
(
    AdebayerContext_t *pAdebayerCtx,
    int iso)
{
    LOGI_ADEBAYER("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAdebayerCtx == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

#if RKAIQ_HAVE_DEBAYER_V2

    int i = 0;
    int iso_low = 0, iso_high = 0, iso_low_index = 0, iso_high_index = 0;
    float ratio = 0.0f;

    //g_interp

    for(i = 0; i < RK_DEBAYER_ISO_STEP_MAX; i++) {
        if (iso < pAdebayerCtx->full_param_v2.g_interp.iso[i])
        {

            iso_low = pAdebayerCtx->full_param_v2.g_interp.iso[MAX(0, i - 1)];
            iso_high = pAdebayerCtx->full_param_v2.g_interp.iso[i];
            iso_low_index = MAX(0, i - 1);
            iso_high_index = i;

            if(i == 0)
                ratio = 0.0f;
            else
                ratio = (float)(iso - iso_low) / (iso_high - iso_low);

            break;
        }
    }

    if(i == RK_DEBAYER_ISO_STEP_MAX) {
        iso_low = pAdebayerCtx->full_param_v2.g_interp.iso[i - 1];
        iso_high = pAdebayerCtx->full_param_v2.g_interp.iso[i - 1];
        iso_low_index = i - 1;
        iso_high_index = i - 1;
        ratio = 1;
    }


    pAdebayerCtx->select_param_v2.debayer_gain_offset = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.g_interp.debayer_gain_offset[iso_low_index], pAdebayerCtx->full_param_v2.g_interp.debayer_gain_offset[iso_high_index], ratio));
    pAdebayerCtx->select_param_v2.debayer_max_ratio = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.g_interp.debayer_max_ratio[iso_low_index], pAdebayerCtx->full_param_v2.g_interp.debayer_max_ratio[iso_high_index], ratio));
    pAdebayerCtx->select_param_v2.debayer_clip_en = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.g_interp.debayer_clip_en[iso_low_index], pAdebayerCtx->full_param_v2.g_interp.debayer_clip_en[iso_high_index], ratio));

    LOGD_ADEBAYER("g_interp:ISO(%d),gain_offset=%d,max_ratio=%d,clip_en=%d", iso,
                  pAdebayerCtx->select_param_v2.debayer_gain_offset,
                  pAdebayerCtx->select_param_v2.debayer_max_ratio,
                  pAdebayerCtx->select_param_v2.debayer_clip_en);


    //g_drctwgt
    for(i = 0; i < RK_DEBAYER_ISO_STEP_MAX; i++) {
        if (iso < pAdebayerCtx->full_param_v2.g_drctwgt.iso[i])
        {

            iso_low = pAdebayerCtx->full_param_v2.g_drctwgt.iso[MAX(0, i - 1)];
            iso_high = pAdebayerCtx->full_param_v2.g_drctwgt.iso[i];
            iso_low_index = MAX(0, i - 1);
            iso_high_index = i;

            if(i == 0)
                ratio = 0;
            else
                ratio = (float)(iso - iso_low) / (iso_high - iso_low);

            break;
        }
    }

    if(i == RK_DEBAYER_ISO_STEP_MAX) {
        iso_low = pAdebayerCtx->full_param_v2.g_drctwgt.iso[i - 1];
        iso_high = pAdebayerCtx->full_param_v2.g_drctwgt.iso[i - 1];
        iso_low_index = i - 1;
        iso_high_index = i - 1;
        ratio = 1;
    }

    pAdebayerCtx->select_param_v2.debayer_thed0 = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.g_drctwgt.debayer_thed0[iso_low_index], pAdebayerCtx->full_param_v2.g_drctwgt.debayer_thed0[iso_high_index], ratio));
    pAdebayerCtx->select_param_v2.debayer_thed1 = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.g_drctwgt.debayer_thed1[iso_low_index], pAdebayerCtx->full_param_v2.g_drctwgt.debayer_thed1[iso_high_index], ratio));

    pAdebayerCtx->select_param_v2.debayer_dist_scale = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.g_drctwgt.debayer_dist_scale[iso_low_index], pAdebayerCtx->full_param_v2.g_drctwgt.debayer_dist_scale[iso_high_index], ratio));
    pAdebayerCtx->select_param_v2.debayer_hf_offset = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.g_drctwgt.debayer_hf_offset[iso_low_index], pAdebayerCtx->full_param_v2.g_drctwgt.debayer_hf_offset[iso_high_index], ratio));
    pAdebayerCtx->select_param_v2.debayer_select_thed = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.g_drctwgt.debayer_select_thed[iso_low_index], pAdebayerCtx->full_param_v2.g_drctwgt.debayer_select_thed[iso_high_index], ratio));

    LOGD_ADEBAYER("g_drctwgt:ISO(%d),thed0=%d,thed1=%d,dist_scale=%d,hf_offset=%d,select_thed=%d", iso,
                  pAdebayerCtx->select_param_v2.debayer_thed0, pAdebayerCtx->select_param_v2.debayer_thed1,
                  pAdebayerCtx->select_param_v2.debayer_dist_scale, pAdebayerCtx->select_param_v2.debayer_hf_offset,
                  pAdebayerCtx->select_param_v2.debayer_select_thed);

    //g_filter
    for(i = 0; i < RK_DEBAYER_ISO_STEP_MAX; i++) {
        if (iso < pAdebayerCtx->full_param_v2.g_filter.iso[i])
        {

            iso_low = pAdebayerCtx->full_param_v2.g_filter.iso[MAX(0, i - 1)];
            iso_high = pAdebayerCtx->full_param_v2.g_filter.iso[i];
            iso_low_index = MAX(0, i - 1);
            iso_high_index = i;

            if(i == 0)
                ratio = 0;
            else
                ratio = (float)(iso - iso_low) / (iso_high - iso_low);

            break;
        }
    }

    if(i == RK_DEBAYER_ISO_STEP_MAX) {
        iso_low = pAdebayerCtx->full_param_v2.g_filter.iso[i - 1];
        iso_high = pAdebayerCtx->full_param_v2.g_filter.iso[i - 1];
        iso_low_index = i - 1;
        iso_high_index = i - 1;
        ratio = 1;
    }

    pAdebayerCtx->select_param_v2.debayer_gfilter_en = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.g_filter.debayer_gfilter_en[iso_low_index], pAdebayerCtx->full_param_v2.g_filter.debayer_gfilter_en[iso_high_index], ratio));
    pAdebayerCtx->select_param_v2.debayer_gfilter_offset = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.g_filter.debayer_gfilter_offset[iso_low_index], pAdebayerCtx->full_param_v2.g_filter.debayer_gfilter_offset[iso_high_index], ratio));

    LOGD_ADEBAYER("g_filter:ISO(%d),gfilter_en=%d,gfilter_offset=%d", iso,
                  pAdebayerCtx->select_param_v2.debayer_gfilter_en, pAdebayerCtx->select_param_v2.debayer_gfilter_offset);

    //c_filter

    for(i = 0; i < RK_DEBAYER_ISO_STEP_MAX; i++) {
        if (iso < pAdebayerCtx->full_param_v2.c_filter.iso[i])
        {

            iso_low = pAdebayerCtx->full_param_v2.c_filter.iso[MAX(0, i - 1)];
            iso_high = pAdebayerCtx->full_param_v2.c_filter.iso[i];
            iso_low_index = MAX(0, i - 1);
            iso_high_index = i;

            if(i == 0)
                ratio = 0;
            else
                ratio = (float)(iso - iso_low) / (iso_high - iso_low);

            break;
        }
    }

    if(i == RK_DEBAYER_ISO_STEP_MAX) {
        iso_low = pAdebayerCtx->full_param_v2.c_filter.iso[i - 1];
        iso_high = pAdebayerCtx->full_param_v2.c_filter.iso[i - 1];
        iso_low_index = i - 1;
        iso_high_index = i - 1;
        ratio = 1;
    }

    pAdebayerCtx->select_param_v2.debayer_cfilter_en = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.c_filter.debayer_cfilter_en[iso_low_index], pAdebayerCtx->full_param_v2.c_filter.debayer_cfilter_en[iso_high_index], ratio));

    pAdebayerCtx->select_param_v2.debayer_alpha_offset = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.c_filter.debayer_alpha_offset[iso_low_index], pAdebayerCtx->full_param_v2.c_filter.debayer_alpha_offset[iso_high_index], ratio));
    pAdebayerCtx->select_param_v2.debayer_alpha_scale = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.c_filter.debayer_alpha_scale[iso_low_index], pAdebayerCtx->full_param_v2.c_filter.debayer_alpha_scale[iso_high_index], ratio);
    pAdebayerCtx->select_param_v2.debayer_edge_offset = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.c_filter.debayer_edge_offset[iso_low_index], pAdebayerCtx->full_param_v2.c_filter.debayer_edge_offset[iso_high_index], ratio));
    pAdebayerCtx->select_param_v2.debayer_edge_scale = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.c_filter.debayer_edge_scale[iso_low_index], pAdebayerCtx->full_param_v2.c_filter.debayer_edge_scale[iso_high_index], ratio);

    pAdebayerCtx->select_param_v2.debayer_bf_sgm = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.c_filter.debayer_bf_sgm[iso_low_index], pAdebayerCtx->full_param_v2.c_filter.debayer_bf_sgm[iso_high_index], ratio);
    pAdebayerCtx->select_param_v2.debayer_bf_curwgt = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.c_filter.debayer_bf_curwgt[iso_low_index], pAdebayerCtx->full_param_v2.c_filter.debayer_bf_curwgt[iso_high_index], ratio));
    pAdebayerCtx->select_param_v2.debayer_bf_clip = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.c_filter.debayer_bf_clip[iso_low_index], pAdebayerCtx->full_param_v2.c_filter.debayer_bf_clip[iso_high_index], ratio));

    pAdebayerCtx->select_param_v2.debayer_loggd_offset = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.c_filter.debayer_loggd_offset[iso_low_index], pAdebayerCtx->full_param_v2.c_filter.debayer_loggd_offset[iso_high_index], ratio));
    pAdebayerCtx->select_param_v2.debayer_loghf_offset = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.c_filter.debayer_loghf_offset[iso_low_index], pAdebayerCtx->full_param_v2.c_filter.debayer_loghf_offset[iso_high_index], ratio));

    pAdebayerCtx->select_param_v2.debayer_cfilter_str = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.c_filter.debayer_cfilter_str[iso_low_index], pAdebayerCtx->full_param_v2.c_filter.debayer_cfilter_str[iso_high_index], ratio);
    pAdebayerCtx->select_param_v2.debayer_wet_clip = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.c_filter.debayer_wet_clip[iso_low_index], pAdebayerCtx->full_param_v2.c_filter.debayer_wet_clip[iso_high_index], ratio);
    pAdebayerCtx->select_param_v2.debayer_wet_ghost = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.c_filter.debayer_wet_ghost[iso_low_index], pAdebayerCtx->full_param_v2.c_filter.debayer_wet_ghost[iso_high_index], ratio);
    pAdebayerCtx->select_param_v2.debayer_wgtslope = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2.c_filter.debayer_wgtslope[iso_low_index], pAdebayerCtx->full_param_v2.c_filter.debayer_wgtslope[iso_high_index], ratio);

    LOGD_ADEBAYER("c_filter:ISO(%d),cfilter_en=%d,loggd_offset=%d,cfilter_str=%f,wet_clip=%f,wet_ghost=%f,wgtslope=%f", iso,
                  pAdebayerCtx->select_param_v2.debayer_cfilter_en, pAdebayerCtx->select_param_v2.debayer_loggd_offset,
                  pAdebayerCtx->select_param_v2.debayer_cfilter_str, pAdebayerCtx->select_param_v2.debayer_wet_clip,
                  pAdebayerCtx->select_param_v2.debayer_wet_ghost, pAdebayerCtx->select_param_v2.debayer_wgtslope);

    LOGD_ADEBAYER("c_filter:ISO(%d),bf_sgm=%f,bf_curwgt=%d,bf_clip=%d,loghf_offset=%d", iso,
                  pAdebayerCtx->select_param_v2.debayer_bf_sgm, pAdebayerCtx->select_param_v2.debayer_bf_curwgt,
                  pAdebayerCtx->select_param_v2.debayer_bf_clip, pAdebayerCtx->select_param_v2.debayer_loghf_offset);

    LOGD_ADEBAYER("c_filter:ISO(%d),alpha_offset=%d,alpha_scale=%f,edge_offset=%d,edge_scale=%f", iso,
                  pAdebayerCtx->select_param_v2.debayer_alpha_offset, pAdebayerCtx->select_param_v2.debayer_alpha_scale,
                  pAdebayerCtx->select_param_v2.debayer_edge_offset, pAdebayerCtx->select_param_v2.debayer_edge_scale);
#endif

#if RKAIQ_HAVE_DEBAYER_V2_LITE
    int i = 0;
    int iso_low = 0, iso_high = 0, iso_low_index = 0, iso_high_index = 0;
    float ratio = 0.0f;

    //g_interp

    for(i = 0; i < RK_DEBAYER_ISO_STEP_MAX; i++) {
        if (iso < pAdebayerCtx->full_param_v2_lite.g_interp.iso[i])
        {

            iso_low = pAdebayerCtx->full_param_v2_lite.g_interp.iso[MAX(0, i - 1)];
            iso_high = pAdebayerCtx->full_param_v2_lite.g_interp.iso[i];
            iso_low_index = MAX(0, i - 1);
            iso_high_index = i;

            if(i == 0)
                ratio = 0.0f;
            else
                ratio = (float)(iso - iso_low) / (iso_high - iso_low);

            break;
        }
    }

    if(i == RK_DEBAYER_ISO_STEP_MAX) {
        iso_low = pAdebayerCtx->full_param_v2_lite.g_interp.iso[i - 1];
        iso_high = pAdebayerCtx->full_param_v2_lite.g_interp.iso[i - 1];
        iso_low_index = i - 1;
        iso_high_index = i - 1;
        ratio = 1;
    }


    pAdebayerCtx->select_param_v2_lite.debayer_gain_offset = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2_lite.g_interp.debayer_gain_offset[iso_low_index], pAdebayerCtx->full_param_v2_lite.g_interp.debayer_gain_offset[iso_high_index], ratio));
    pAdebayerCtx->select_param_v2_lite.debayer_max_ratio = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2_lite.g_interp.debayer_max_ratio[iso_low_index], pAdebayerCtx->full_param_v2_lite.g_interp.debayer_max_ratio[iso_high_index], ratio));
    pAdebayerCtx->select_param_v2_lite.debayer_clip_en = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2_lite.g_interp.debayer_clip_en[iso_low_index], pAdebayerCtx->full_param_v2_lite.g_interp.debayer_clip_en[iso_high_index], ratio));

    LOGD_ADEBAYER("g_interp:ISO(%d),gain_offset=%d,max_ratio=%d,clip_en=%d", iso,
                  pAdebayerCtx->select_param_v2_lite.debayer_gain_offset,
                  pAdebayerCtx->select_param_v2_lite.debayer_max_ratio,
                  pAdebayerCtx->select_param_v2_lite.debayer_clip_en);


    //g_drctwgt
    for(i = 0; i < RK_DEBAYER_ISO_STEP_MAX; i++) {
        if (iso < pAdebayerCtx->full_param_v2_lite.g_drctwgt.iso[i])
        {

            iso_low = pAdebayerCtx->full_param_v2_lite.g_drctwgt.iso[MAX(0, i - 1)];
            iso_high = pAdebayerCtx->full_param_v2_lite.g_drctwgt.iso[i];
            iso_low_index = MAX(0, i - 1);
            iso_high_index = i;

            if(i == 0)
                ratio = 0;
            else
                ratio = (float)(iso - iso_low) / (iso_high - iso_low);

            break;
        }
    }

    if(i == RK_DEBAYER_ISO_STEP_MAX) {
        iso_low = pAdebayerCtx->full_param_v2_lite.g_drctwgt.iso[i - 1];
        iso_high = pAdebayerCtx->full_param_v2_lite.g_drctwgt.iso[i - 1];
        iso_low_index = i - 1;
        iso_high_index = i - 1;
        ratio = 1;
    }

    pAdebayerCtx->select_param_v2_lite.debayer_thed0 = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2_lite.g_drctwgt.debayer_thed0[iso_low_index], pAdebayerCtx->full_param_v2_lite.g_drctwgt.debayer_thed0[iso_high_index], ratio));
    pAdebayerCtx->select_param_v2_lite.debayer_thed1 = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2_lite.g_drctwgt.debayer_thed1[iso_low_index], pAdebayerCtx->full_param_v2_lite.g_drctwgt.debayer_thed1[iso_high_index], ratio));

    pAdebayerCtx->select_param_v2_lite.debayer_dist_scale = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2_lite.g_drctwgt.debayer_dist_scale[iso_low_index], pAdebayerCtx->full_param_v2_lite.g_drctwgt.debayer_dist_scale[iso_high_index], ratio));
    pAdebayerCtx->select_param_v2_lite.debayer_hf_offset = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2_lite.g_drctwgt.debayer_hf_offset[iso_low_index], pAdebayerCtx->full_param_v2_lite.g_drctwgt.debayer_hf_offset[iso_high_index], ratio));
    pAdebayerCtx->select_param_v2_lite.debayer_select_thed = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2_lite.g_drctwgt.debayer_select_thed[iso_low_index], pAdebayerCtx->full_param_v2_lite.g_drctwgt.debayer_select_thed[iso_high_index], ratio));

    LOGD_ADEBAYER("g_drctwgt:ISO(%d),thed0=%d,thed1=%d,dist_scale=%d,hf_offset=%d,select_thed=%d", iso,
                  pAdebayerCtx->select_param_v2_lite.debayer_thed0, pAdebayerCtx->select_param_v2_lite.debayer_thed1,
                  pAdebayerCtx->select_param_v2_lite.debayer_dist_scale, pAdebayerCtx->select_param_v2_lite.debayer_hf_offset,
                  pAdebayerCtx->select_param_v2_lite.debayer_select_thed);

    //g_filter
    for(i = 0; i < RK_DEBAYER_ISO_STEP_MAX; i++) {
        if (iso < pAdebayerCtx->full_param_v2_lite.g_filter.iso[i])
        {

            iso_low = pAdebayerCtx->full_param_v2_lite.g_filter.iso[MAX(0, i - 1)];
            iso_high = pAdebayerCtx->full_param_v2_lite.g_filter.iso[i];
            iso_low_index = MAX(0, i - 1);
            iso_high_index = i;

            if(i == 0)
                ratio = 0;
            else
                ratio = (float)(iso - iso_low) / (iso_high - iso_low);

            break;
        }
    }

    if(i == RK_DEBAYER_ISO_STEP_MAX) {
        iso_low = pAdebayerCtx->full_param_v2_lite.g_filter.iso[i - 1];
        iso_high = pAdebayerCtx->full_param_v2_lite.g_filter.iso[i - 1];
        iso_low_index = i - 1;
        iso_high_index = i - 1;
        ratio = 1;
    }

    pAdebayerCtx->select_param_v2_lite.debayer_gfilter_en = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2_lite.g_filter.debayer_gfilter_en[iso_low_index], pAdebayerCtx->full_param_v2_lite.g_filter.debayer_gfilter_en[iso_high_index], ratio));
    pAdebayerCtx->select_param_v2_lite.debayer_gfilter_offset = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v2_lite.g_filter.debayer_gfilter_offset[iso_low_index], pAdebayerCtx->full_param_v2_lite.g_filter.debayer_gfilter_offset[iso_high_index], ratio));

    LOGD_ADEBAYER("g_filter:ISO(%d),gfilter_en=%d,gfilter_offset=%d", iso,
                  pAdebayerCtx->select_param_v2_lite.debayer_gfilter_en, pAdebayerCtx->select_param_v2_lite.debayer_gfilter_offset);

#endif


    LOGI_ADEBAYER("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;

}

//debayer process
XCamReturn
AdebayerProcess
(
    AdebayerContext_t *pAdebayerCtx,
    int iso
)
{
    LOGI_ADEBAYER("%s(%d): enter! ISO=%d\n", __FUNCTION__, __LINE__, iso);

    if(pAdebayerCtx == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    if(pAdebayerCtx->mode == RK_AIQ_DEBAYER_MODE_AUTO) {

#if RKAIQ_HAVE_DEBAYER_V2

        pAdebayerCtx->select_param_v2.debayer_en = pAdebayerCtx->full_param_v2.debayer_en;

        pAdebayerCtx->select_param_v2.lowfreq_filter1[0] = pAdebayerCtx->full_param_v2.lowfreq_filter1[0];
        pAdebayerCtx->select_param_v2.lowfreq_filter1[1] = pAdebayerCtx->full_param_v2.lowfreq_filter1[1];
        pAdebayerCtx->select_param_v2.lowfreq_filter1[2] = pAdebayerCtx->full_param_v2.lowfreq_filter1[2];
        pAdebayerCtx->select_param_v2.lowfreq_filter1[3] = pAdebayerCtx->full_param_v2.lowfreq_filter1[3];

        pAdebayerCtx->select_param_v2.highfreq_filter2[0] = pAdebayerCtx->full_param_v2.highfreq_filter2[0];
        pAdebayerCtx->select_param_v2.highfreq_filter2[1] = pAdebayerCtx->full_param_v2.highfreq_filter2[1];
        pAdebayerCtx->select_param_v2.highfreq_filter2[2] = pAdebayerCtx->full_param_v2.highfreq_filter2[2];
        pAdebayerCtx->select_param_v2.highfreq_filter2[3] = pAdebayerCtx->full_param_v2.highfreq_filter2[3];

        pAdebayerCtx->select_param_v2.c_alpha_gaus_coe[0] = pAdebayerCtx->full_param_v2.c_alpha_gaus_coe[0];
        pAdebayerCtx->select_param_v2.c_alpha_gaus_coe[1] = pAdebayerCtx->full_param_v2.c_alpha_gaus_coe[1];
        pAdebayerCtx->select_param_v2.c_alpha_gaus_coe[2] = pAdebayerCtx->full_param_v2.c_alpha_gaus_coe[2];

        pAdebayerCtx->select_param_v2.c_ce_gaus_coe[0] = pAdebayerCtx->full_param_v2.c_ce_gaus_coe[0];
        pAdebayerCtx->select_param_v2.c_ce_gaus_coe[1] = pAdebayerCtx->full_param_v2.c_ce_gaus_coe[1];
        pAdebayerCtx->select_param_v2.c_ce_gaus_coe[2] = pAdebayerCtx->full_param_v2.c_ce_gaus_coe[2];

        pAdebayerCtx->select_param_v2.c_guid_gaus_coe[0] = pAdebayerCtx->full_param_v2.c_guid_gaus_coe[0];
        pAdebayerCtx->select_param_v2.c_guid_gaus_coe[1] = pAdebayerCtx->full_param_v2.c_guid_gaus_coe[1];
        pAdebayerCtx->select_param_v2.c_guid_gaus_coe[2] = pAdebayerCtx->full_param_v2.c_guid_gaus_coe[2];

        // TODO: copy params from json, and select params according to ISO

        AdebayerSelectParam(pAdebayerCtx, iso);
#endif

#if RKAIQ_HAVE_DEBAYER_V2_LITE

        pAdebayerCtx->select_param_v2_lite.debayer_en = pAdebayerCtx->full_param_v2_lite.debayer_en;

        pAdebayerCtx->select_param_v2_lite.lowfreq_filter1[0] = pAdebayerCtx->full_param_v2_lite.lowfreq_filter1[0];
        pAdebayerCtx->select_param_v2_lite.lowfreq_filter1[1] = pAdebayerCtx->full_param_v2_lite.lowfreq_filter1[1];
        pAdebayerCtx->select_param_v2_lite.lowfreq_filter1[2] = pAdebayerCtx->full_param_v2_lite.lowfreq_filter1[2];
        pAdebayerCtx->select_param_v2_lite.lowfreq_filter1[3] = pAdebayerCtx->full_param_v2_lite.lowfreq_filter1[3];

        pAdebayerCtx->select_param_v2_lite.highfreq_filter2[0] = pAdebayerCtx->full_param_v2_lite.highfreq_filter2[0];
        pAdebayerCtx->select_param_v2_lite.highfreq_filter2[1] = pAdebayerCtx->full_param_v2_lite.highfreq_filter2[1];
        pAdebayerCtx->select_param_v2_lite.highfreq_filter2[2] = pAdebayerCtx->full_param_v2_lite.highfreq_filter2[2];
        pAdebayerCtx->select_param_v2_lite.highfreq_filter2[3] = pAdebayerCtx->full_param_v2_lite.highfreq_filter2[3];

        // TODO: copy params from json, and select params according to ISO

        AdebayerSelectParam(pAdebayerCtx, iso);
#endif

    } else {

        //only api would use manual mode ,and manual params have been written in seletced params

    }

    LOGI_ADEBAYER("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;

}

//debayer get result
XCamReturn
AdebayerGetProcResult
(
    AdebayerContext_t*    pAdebayerCtx,
    AdebayerProcResultV2_t* pAdebayerResult
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

    // TODO: FIX TO REG VALUE

#if RKAIQ_HAVE_DEBAYER_V2
    /* CONTROL */

    pAdebayerCtx->config.enable = pAdebayerCtx->select_param_v2.debayer_en;
    pAdebayerCtx->config.filter_g_en = pAdebayerCtx->select_param_v2.debayer_gfilter_en;
    pAdebayerCtx->config.filter_c_en = pAdebayerCtx->select_param_v2.debayer_cfilter_en;

    /* G_INTERP */

    pAdebayerCtx->config.clip_en = pAdebayerCtx->select_param_v2.debayer_clip_en;
    pAdebayerCtx->config.dist_scale = pAdebayerCtx->select_param_v2.debayer_dist_scale;
    pAdebayerCtx->config.thed0 = pAdebayerCtx->select_param_v2.debayer_thed0;
    pAdebayerCtx->config.thed1 = pAdebayerCtx->select_param_v2.debayer_thed1;
    pAdebayerCtx->config.select_thed = pAdebayerCtx->select_param_v2.debayer_select_thed;
    pAdebayerCtx->config.max_ratio = pAdebayerCtx->select_param_v2.debayer_max_ratio;

    /* G_INTERP_FILTER */
    pAdebayerCtx->config.filter1_coe[0] = pAdebayerCtx->select_param_v2.lowfreq_filter1[0];
    pAdebayerCtx->config.filter1_coe[1] = pAdebayerCtx->select_param_v2.lowfreq_filter1[1];
    pAdebayerCtx->config.filter1_coe[2] = pAdebayerCtx->select_param_v2.lowfreq_filter1[2];
    pAdebayerCtx->config.filter1_coe[3] = pAdebayerCtx->select_param_v2.lowfreq_filter1[3];

    pAdebayerCtx->config.filter2_coe[0] = pAdebayerCtx->select_param_v2.highfreq_filter2[0];
    pAdebayerCtx->config.filter2_coe[1] = pAdebayerCtx->select_param_v2.highfreq_filter2[1];
    pAdebayerCtx->config.filter2_coe[2] = pAdebayerCtx->select_param_v2.highfreq_filter2[2];
    pAdebayerCtx->config.filter2_coe[3] = pAdebayerCtx->select_param_v2.highfreq_filter2[3];

    /* C_FILTER_GUIDE_GAUS */
    pAdebayerCtx->config.guid_gaus_coe[0] = pAdebayerCtx->select_param_v2.c_guid_gaus_coe[0];
    pAdebayerCtx->config.guid_gaus_coe[1] = pAdebayerCtx->select_param_v2.c_guid_gaus_coe[1];
    pAdebayerCtx->config.guid_gaus_coe[2] = pAdebayerCtx->select_param_v2.c_guid_gaus_coe[2];

    /* C_FILTER_CE_GAUS */
    pAdebayerCtx->config.ce_gaus_coe[0] = pAdebayerCtx->select_param_v2.c_ce_gaus_coe[0];
    pAdebayerCtx->config.ce_gaus_coe[1] = pAdebayerCtx->select_param_v2.c_ce_gaus_coe[1];
    pAdebayerCtx->config.ce_gaus_coe[2] = pAdebayerCtx->select_param_v2.c_ce_gaus_coe[2];

    /* C_FILTER_ALPHA_GAUS */
    pAdebayerCtx->config.alpha_gaus_coe[0] = pAdebayerCtx->select_param_v2.c_alpha_gaus_coe[0];
    pAdebayerCtx->config.alpha_gaus_coe[1] = pAdebayerCtx->select_param_v2.c_alpha_gaus_coe[1];
    pAdebayerCtx->config.alpha_gaus_coe[2] = pAdebayerCtx->select_param_v2.c_alpha_gaus_coe[2];

    /* G_INTERP_OFFSET */
    pAdebayerCtx->config.hf_offset = pAdebayerCtx->select_param_v2.debayer_hf_offset;
    pAdebayerCtx->config.gain_offset = pAdebayerCtx->select_param_v2.debayer_gain_offset;

    /* G_FILTER_OFFSET */
    pAdebayerCtx->config.offset = pAdebayerCtx->select_param_v2.debayer_gfilter_offset;

    /* C_FILTER_LOG_OFFSET */
    pAdebayerCtx->config.loghf_offset = pAdebayerCtx->select_param_v2.debayer_loghf_offset;
    pAdebayerCtx->config.loggd_offset = pAdebayerCtx->select_param_v2.debayer_loggd_offset;

    /* C_FILTER_ALPHA */
    pAdebayerCtx->config.alpha_offset = pAdebayerCtx->select_param_v2.debayer_alpha_offset;
    pAdebayerCtx->config.alpha_scale =  ROUND_F(pAdebayerCtx->select_param_v2.debayer_alpha_scale * (1 << RK_DEBAYER_V2_FIX_BIT_ALPHA_SCALE));

    /* C_FILTER_EDGE */
    pAdebayerCtx->config.edge_offset = pAdebayerCtx->select_param_v2.debayer_edge_offset;
    pAdebayerCtx->config.edge_scale = ROUND_F(pAdebayerCtx->select_param_v2.debayer_edge_scale * (1 << RK_DEBAYER_V2_FIX_BIT_EDGE_SCALE));

    /* C_FILTER_IIR_0 */

    // log fix bit : 10, 1 / sigma, (1 << RK_DEBAYER_V30_FIX_BIT_INV_SIGMA) / (sigma)
    float sqrtLog2e                     = 1.2011;
    uint32_t ce_sgm                     = ROUND_F((1 << RK_DEBAYER_V2_FIX_BIT_INV_SIGMA) * sqrtLog2e / ((1 << RK_DEBAYER_V2_FIX_BIT_LOG2) * pAdebayerCtx->select_param_v2.debayer_cfilter_str));
    pAdebayerCtx->config.wgtslope       = ROUND_F(pAdebayerCtx->select_param_v2.debayer_wgtslope * (1 << RK_DEBAYER_V2_FIX_BIT_WGT_SLOPE));

    /* C_FILTER_IIR_1 */
    pAdebayerCtx->config.wet_clip = ROUND_F(pAdebayerCtx->select_param_v2.debayer_wet_clip * (1 << RK_DEBAYER_V2_FIX_BIT_IIR_WGT_CLIP));
    pAdebayerCtx->config.wet_ghost = ROUND_F(pAdebayerCtx->select_param_v2.debayer_wet_ghost * (1 << RK_DEBAYER_V2_FIX_BIT_IIR_GHOST));

    // chromaFilterStrength * wgtSlope, int to float
    int tmptmp                          = ce_sgm * pAdebayerCtx->config.wgtslope;
    int shiftBit                        = LOG2(tmptmp) - RK_DEBAYER_V2_FIX_BIT_INT_TO_FLOAT;
    shiftBit                            = MAX(shiftBit, 0);
    pAdebayerCtx->config.ce_sgm         = ROUND_F((float)tmptmp / (1 << shiftBit));
    pAdebayerCtx->config.exp_shift      = RK_DEBAYER_V2_FIX_BIT_INV_SIGMA - shiftBit;

    /* C_FILTER_BF */
    pAdebayerCtx->config.bf_clip = pAdebayerCtx->select_param_v2.debayer_bf_clip;
    pAdebayerCtx->config.bf_curwgt = pAdebayerCtx->select_param_v2.debayer_bf_curwgt;

    int scale                   = (1 << 12) - 1; //rawbit: 12
    float log2e                 = 0.8493;
    pAdebayerCtx->config.bf_sgm = (int)((1 << RK_DEBAYER_V2_FIX_BIT_INV_BF_SIGMA) * log2e / (pAdebayerCtx->select_param_v2.debayer_bf_sgm * scale));

    LOGD_ADEBAYER("FIX 2 REG: alpha_scale=%d,edge_scale=%d,ce_sgm=%d,exp_shift=%d,wgtslope=%d,wet_clip=%d,wet_ghost=%d,bf_sgm=%d",
                  pAdebayerCtx->config.alpha_scale, pAdebayerCtx->config.edge_scale, pAdebayerCtx->config.ce_sgm,
                  pAdebayerCtx->config.exp_shift, pAdebayerCtx->config.wgtslope,  pAdebayerCtx->config.wet_clip,
                  pAdebayerCtx->config.wet_ghost, pAdebayerCtx->config.bf_sgm);

    LOGD_ADEBAYER("FIX 2 REG: max_ratio=%d,hf_offset=%d,gain_offset=%d,loghf_offset=%d,loggd_offset=%d",
                  pAdebayerCtx->config.max_ratio, pAdebayerCtx->config.hf_offset,  pAdebayerCtx->config.gain_offset,
                  pAdebayerCtx->config.loghf_offset, pAdebayerCtx->config.loggd_offset);
#endif

#if RKAIQ_HAVE_DEBAYER_V2_LITE
    /* CONTROL */

    pAdebayerCtx->config.enable = pAdebayerCtx->select_param_v2_lite.debayer_en;
    pAdebayerCtx->config.filter_g_en = pAdebayerCtx->select_param_v2_lite.debayer_gfilter_en;
    pAdebayerCtx->config.filter_c_en = 0;//pAdebayerCtx->select_param_v2_lite.debayer_cfilter_en;

    /* G_INTERP */

    pAdebayerCtx->config.clip_en = pAdebayerCtx->select_param_v2_lite.debayer_clip_en;
    pAdebayerCtx->config.dist_scale = pAdebayerCtx->select_param_v2_lite.debayer_dist_scale;
    pAdebayerCtx->config.thed0 = pAdebayerCtx->select_param_v2_lite.debayer_thed0;
    pAdebayerCtx->config.thed1 = pAdebayerCtx->select_param_v2_lite.debayer_thed1;
    pAdebayerCtx->config.select_thed = pAdebayerCtx->select_param_v2_lite.debayer_select_thed;
    pAdebayerCtx->config.max_ratio = pAdebayerCtx->select_param_v2_lite.debayer_max_ratio;

    /* G_INTERP_FILTER */
    pAdebayerCtx->config.filter1_coe[0] = pAdebayerCtx->select_param_v2_lite.lowfreq_filter1[0];
    pAdebayerCtx->config.filter1_coe[1] = pAdebayerCtx->select_param_v2_lite.lowfreq_filter1[1];
    pAdebayerCtx->config.filter1_coe[2] = pAdebayerCtx->select_param_v2_lite.lowfreq_filter1[2];
    pAdebayerCtx->config.filter1_coe[3] = pAdebayerCtx->select_param_v2_lite.lowfreq_filter1[3];

    pAdebayerCtx->config.filter2_coe[0] = pAdebayerCtx->select_param_v2_lite.highfreq_filter2[0];
    pAdebayerCtx->config.filter2_coe[1] = pAdebayerCtx->select_param_v2_lite.highfreq_filter2[1];
    pAdebayerCtx->config.filter2_coe[2] = pAdebayerCtx->select_param_v2_lite.highfreq_filter2[2];
    pAdebayerCtx->config.filter2_coe[3] = pAdebayerCtx->select_param_v2_lite.highfreq_filter2[3];

    /* G_INTERP_OFFSET */
    pAdebayerCtx->config.hf_offset = pAdebayerCtx->select_param_v2_lite.debayer_hf_offset;
    pAdebayerCtx->config.gain_offset = pAdebayerCtx->select_param_v2_lite.debayer_gain_offset;

    /* G_FILTER_OFFSET */
    pAdebayerCtx->config.offset = pAdebayerCtx->select_param_v2_lite.debayer_gfilter_offset;

    LOGD_ADEBAYER("FIX 2 REG: max_ratio=%d,hf_offset=%d,gain_offset=%d",
                  pAdebayerCtx->config.max_ratio, pAdebayerCtx->config.hf_offset,  pAdebayerCtx->config.gain_offset);
#endif


    // TODO: copy regvalue result
    *pAdebayerResult->config = pAdebayerCtx->config;
    pAdebayerCtx->config.updatecfg = false;

    LOGI_ADEBAYER("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;
}



