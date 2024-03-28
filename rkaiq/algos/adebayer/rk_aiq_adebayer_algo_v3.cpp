
#include "rk_aiq_adebayer_algo_v3.h"

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

        CalibDbV2_Debayer_v3_t* debayer =
            (CalibDbV2_Debayer_v3_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, debayer_v3));

        if(debayer == NULL)
            LOGE_ADEBAYER("%s(%d): copy calib params failed!\n", __FUNCTION__, __LINE__);

        pAdebayerCtx->full_param_v3.en = debayer->param.en;
        pAdebayerCtx->full_param_v3.sta.cFlt = debayer->param.sta.cFlt;
        pAdebayerCtx->full_param_v3.sta.gDrctWgt = debayer->param.sta.gDrctWgt;

        pAdebayerCtx->full_param_v3.dyn.gInterp = debayer->param.dyn.gInterp;
        pAdebayerCtx->full_param_v3.dyn.gDrctWgt = debayer->param.dyn.gDrctWgt;
        pAdebayerCtx->full_param_v3.dyn.gOutlsFlt = debayer->param.dyn.gOutlsFlt;
        pAdebayerCtx->full_param_v3.dyn.cFlt = debayer->param.dyn.cFlt;

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
    memset(&pAdebayerCtx->full_param_v3, 0, sizeof(CalibDbV2_Debayer_Tuning_V3_t));
    memset(&pAdebayerCtx->select_param_v3, 0, sizeof(AdebayerSeletedParamV3_t));


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

    int i = 0;
    int iso_low = 0, iso_high = 0, iso_low_index = 0, iso_high_index = 0;
    float ratio = 0.0f;


    for(i = 0; i < RK_DEBAYER_ISO_STEP_MAX; i++) {
        if (iso < pAdebayerCtx->full_param_v3.dyn.gInterp.iso[i])
        {
            iso_low = pAdebayerCtx->full_param_v3.dyn.gInterp.iso[MAX(0, i - 1)];
            iso_high = pAdebayerCtx->full_param_v3.dyn.gInterp.iso[i];
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
        iso_low = pAdebayerCtx->full_param_v3.dyn.gInterp.iso[i - 1];
        iso_high = pAdebayerCtx->full_param_v3.dyn.gInterp.iso[i - 1];
        iso_low_index = i - 1;
        iso_high_index = i - 1;
        ratio = 1;
    }

    //g_interp

    pAdebayerCtx->select_param_v3.hw_dmT_gInterpClip_en = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gInterp.hw_dmT_gInterpClip_en[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gInterp.hw_dmT_gInterpClip_en[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.hw_dmT_gInterpSharpStrg_maxLimit = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gInterp.hw_dmT_gInterpSharpStrg_maxLim[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gInterp.hw_dmT_gInterpSharpStrg_maxLim[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.hw_dmT_gInterpSharpStrg_offset = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gInterp.hw_dmT_gInterpSharpStrg_offset[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gInterp.hw_dmT_gInterpSharpStrg_offset[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.sw_dmT_gInterpWgtFlt_alpha = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gInterp.sw_dmT_gInterpWgtFlt_alpha[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gInterp.sw_dmT_gInterpWgtFlt_alpha[iso_high_index], ratio);

    LOGD_ADEBAYER("g_interp:ISO(%d),SharpStrg_offset=%d,maxLim=%d,clip_en=%d,WgtFlt_alpha=%f", iso,
                  pAdebayerCtx->select_param_v3.hw_dmT_gInterpSharpStrg_offset,
                  pAdebayerCtx->select_param_v3.hw_dmT_gInterpSharpStrg_maxLimit,
                  pAdebayerCtx->select_param_v3.hw_dmT_gInterpClip_en,
                  pAdebayerCtx->select_param_v3.sw_dmT_gInterpWgtFlt_alpha);


    //g_drctwgt

    pAdebayerCtx->select_param_v3.hw_dmT_loDrct_thred = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gDrctWgt.hw_dmT_loDrct_thred[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gDrctWgt.hw_dmT_loDrct_thred[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.hw_dmT_hiDrct_thred = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gDrctWgt.hw_dmT_hiDrct_thred[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gDrctWgt.hw_dmT_hiDrct_thred[iso_high_index], ratio));

    pAdebayerCtx->select_param_v3.hw_dmT_hiTexture_thred = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gDrctWgt.hw_dmT_hiTexture_thred[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gDrctWgt.hw_dmT_hiTexture_thred[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.hw_dmT_drctMethod_thred = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gDrctWgt.hw_dmT_drctMethod_thred[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gDrctWgt.hw_dmT_drctMethod_thred[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.sw_dmT_gradLoFlt_alpha = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_gradLoFlt_alpha[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_gradLoFlt_alpha[iso_high_index], ratio);

    pAdebayerCtx->select_param_v3.sw_dmT_drct_offset[0] = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_drct_offset0[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_drct_offset0[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.sw_dmT_drct_offset[1] = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_drct_offset1[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_drct_offset1[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.sw_dmT_drct_offset[2] = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_drct_offset2[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_drct_offset2[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.sw_dmT_drct_offset[3] = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_drct_offset3[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_drct_offset3[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.sw_dmT_drct_offset[4] = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_drct_offset4[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_drct_offset4[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.sw_dmT_drct_offset[5] = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_drct_offset5[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_drct_offset5[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.sw_dmT_drct_offset[6] = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_drct_offset6[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_drct_offset6[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.sw_dmT_drct_offset[7] = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_drct_offset7[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gDrctWgt.sw_dmT_drct_offset7[iso_high_index], ratio));


    LOGD_ADEBAYER("g_drctwgt:ISO(%d),lo_thed=%d,hi_thed=%d,hiTex_thred=%d,drctMethod_tred=%d,gradLoFlt=%f", iso,
                  pAdebayerCtx->select_param_v3.hw_dmT_loDrct_thred, pAdebayerCtx->select_param_v3.hw_dmT_hiDrct_thred,
                  pAdebayerCtx->select_param_v3.hw_dmT_hiTexture_thred, pAdebayerCtx->select_param_v3.hw_dmT_drctMethod_thred,
                  pAdebayerCtx->select_param_v3.sw_dmT_gradLoFlt_alpha);

    LOGD_ADEBAYER("g_drctwgt:ISO(%d),drct offset0~7=%d %d %d %d %d %d %d %d", iso,
                  pAdebayerCtx->select_param_v3.sw_dmT_drct_offset[0], pAdebayerCtx->select_param_v3.sw_dmT_drct_offset[1],
                  pAdebayerCtx->select_param_v3.sw_dmT_drct_offset[2], pAdebayerCtx->select_param_v3.sw_dmT_drct_offset[3],
                  pAdebayerCtx->select_param_v3.sw_dmT_drct_offset[4], pAdebayerCtx->select_param_v3.sw_dmT_drct_offset[5],
                  pAdebayerCtx->select_param_v3.sw_dmT_drct_offset[6], pAdebayerCtx->select_param_v3.sw_dmT_drct_offset[7]);

    //g_filter

    pAdebayerCtx->select_param_v3.hw_dmT_gOutlsFlt_en = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.hw_dmT_gOutlsFlt_en[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.hw_dmT_gOutlsFlt_en[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.hw_dmT_gOutlsFlt_mode = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.hw_dmT_gOutlsFlt_mode[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.hw_dmT_gOutlsFlt_mode[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.hw_dmT_gOutlsFltRange_offset = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.hw_dmT_gOutlsFltRange_offset[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.hw_dmT_gOutlsFltRange_offset[iso_high_index], ratio));

    pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFltRsigma_en = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFltRsigma_en[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFltRsigma_en[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_rsigma = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_rsigma[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_rsigma[iso_high_index], ratio);
    pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_ratio = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_ratio[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_ratio[iso_high_index], ratio);

    pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_vsigma[0] = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_vsigma0[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_vsigma0[iso_high_index], ratio);
    pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_vsigma[1] = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_vsigma1[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_vsigma1[iso_high_index], ratio);
    pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_vsigma[2] = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_vsigma2[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_vsigma2[iso_high_index], ratio);
    pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_vsigma[3] = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_vsigma3[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_vsigma3[iso_high_index], ratio);
    pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_vsigma[4] = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_vsigma4[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_vsigma4[iso_high_index], ratio);
    pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_vsigma[5] = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_vsigma5[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_vsigma5[iso_high_index], ratio);
    pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_vsigma[6] = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_vsigma6[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_vsigma6[iso_high_index], ratio);
    pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_vsigma[7] = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_vsigma7[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_vsigma7[iso_high_index], ratio);

    pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_coeff[0] = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_coeff0[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_coeff0[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_coeff[1] = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_coeff1[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_coeff1[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_coeff[2] = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_coeff2[iso_low_index], pAdebayerCtx->full_param_v3.dyn.gOutlsFlt.sw_dmT_gOutlsFlt_coeff2[iso_high_index], ratio));


    LOGD_ADEBAYER("g_filter:ISO(%d),gfilter_en=%d,FltRange_offset=%d,rsigma=%f,ratio=%f", iso,
                  pAdebayerCtx->select_param_v3.hw_dmT_gOutlsFlt_en, pAdebayerCtx->select_param_v3.hw_dmT_gOutlsFltRange_offset,
                  pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_rsigma, pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_ratio);

    //c_filter

    pAdebayerCtx->select_param_v3.hw_dmT_cnrFlt_en = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.cFlt.hw_dmT_cnrFlt_en[iso_low_index], pAdebayerCtx->full_param_v3.dyn.cFlt.hw_dmT_cnrFlt_en[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.hw_dmT_cnrEdgeAlpha_offset = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.cFlt.hw_dmT_cnrEdgeAlpha_offset[iso_low_index], pAdebayerCtx->full_param_v3.dyn.cFlt.hw_dmT_cnrEdgeAlpha_offset[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.hw_dmT_cnrMoireAlpha_offset = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.cFlt.hw_dmT_cnrMoireAlpha_offset[iso_low_index], pAdebayerCtx->full_param_v3.dyn.cFlt.hw_dmT_cnrMoireAlpha_offset[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.hw_dmT_cnrLogGrad_offset = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.cFlt.hw_dmT_cnrLogGrad_offset[iso_low_index], pAdebayerCtx->full_param_v3.dyn.cFlt.hw_dmT_cnrLogGrad_offset[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.hw_dmT_cnrLogGuide_offset = ROUND_F(INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.cFlt.hw_dmT_cnrLogGuide_offset[iso_low_index], pAdebayerCtx->full_param_v3.dyn.cFlt.hw_dmT_cnrLogGuide_offset[iso_high_index], ratio));
    pAdebayerCtx->select_param_v3.hw_dmT_cnrHiFltCur_wgt = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.cFlt.hw_dmT_cnrHiFltCur_wgt[iso_low_index], pAdebayerCtx->full_param_v3.dyn.cFlt.hw_dmT_cnrHiFltCur_wgt[iso_high_index], ratio);
    pAdebayerCtx->select_param_v3.hw_dmT_cnrHiFltWgt_minLimit = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.cFlt.hw_dmT_cnrHiFltWgt_minLimit[iso_low_index], pAdebayerCtx->full_param_v3.dyn.cFlt.hw_dmT_cnrHiFltWgt_minLimit[iso_high_index], ratio);

    pAdebayerCtx->select_param_v3.sw_dmT_cnrEdgeAlpha_scale = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.cFlt.sw_dmT_cnrEdgeAlpha_scale[iso_low_index], pAdebayerCtx->full_param_v3.dyn.cFlt.sw_dmT_cnrEdgeAlpha_scale[iso_high_index], ratio);
    pAdebayerCtx->select_param_v3.sw_dmT_cnrMoireAlpha_scale = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.cFlt.sw_dmT_cnrMoireAlpha_scale[iso_low_index], pAdebayerCtx->full_param_v3.dyn.cFlt.sw_dmT_cnrMoireAlpha_scale[iso_high_index], ratio);
    pAdebayerCtx->select_param_v3.sw_dmT_cnrLoFltWgt_maxLimit = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.cFlt.sw_dmT_cnrLoFltWgt_maxLimit[iso_low_index], pAdebayerCtx->full_param_v3.dyn.cFlt.sw_dmT_cnrLoFltWgt_maxLimit[iso_high_index], ratio);
    pAdebayerCtx->select_param_v3.sw_dmT_cnrLoFltWgt_minThred = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.cFlt.sw_dmT_cnrLoFltWgt_minThred[iso_low_index], pAdebayerCtx->full_param_v3.dyn.cFlt.sw_dmT_cnrLoFltWgt_minThred[iso_high_index], ratio);
    pAdebayerCtx->select_param_v3.sw_dmT_cnrLoFltWgt_slope = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.cFlt.sw_dmT_cnrLoFltWgt_slope[iso_low_index], pAdebayerCtx->full_param_v3.dyn.cFlt.sw_dmT_cnrLoFltWgt_slope[iso_high_index], ratio);
    pAdebayerCtx->select_param_v3.sw_dmT_cnrLoFlt_vsigma = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.cFlt.sw_dmT_cnrLoFlt_vsigma[iso_low_index], pAdebayerCtx->full_param_v3.dyn.cFlt.sw_dmT_cnrLoFlt_vsigma[iso_high_index], ratio);
    pAdebayerCtx->select_param_v3.sw_dmT_cnrHiFlt_vsigma = INTERP_DEBAYER_V2(pAdebayerCtx->full_param_v3.dyn.cFlt.sw_dmT_cnrHiFlt_vsigma[iso_low_index], pAdebayerCtx->full_param_v3.dyn.cFlt.sw_dmT_cnrHiFlt_vsigma[iso_high_index], ratio);

    LOGD_ADEBAYER("c_filter:ISO(%d),cfilter_en=%d,LogGrad_offset=%d,LogGuide_offset=%d,Wgt_maxLimit=%f,Wgt_minThred=%f,wgtslope=%f", iso,
                  pAdebayerCtx->select_param_v3.hw_dmT_cnrFlt_en, pAdebayerCtx->select_param_v3.hw_dmT_cnrLogGrad_offset,
                  pAdebayerCtx->select_param_v3.hw_dmT_cnrLogGuide_offset, pAdebayerCtx->select_param_v3.sw_dmT_cnrLoFltWgt_maxLimit,
                  pAdebayerCtx->select_param_v3.sw_dmT_cnrLoFltWgt_minThred, pAdebayerCtx->select_param_v3.sw_dmT_cnrLoFltWgt_slope);

    LOGD_ADEBAYER("c_filter:ISO(%d),LoFlt_vsigma=%f,HiFlt_vsigma=%f,HiFltCur_wgt=%f,HiFltWgt_minLimit=%f", iso,
                  pAdebayerCtx->select_param_v3.sw_dmT_cnrLoFlt_vsigma, pAdebayerCtx->select_param_v3.sw_dmT_cnrHiFlt_vsigma,
                  pAdebayerCtx->select_param_v3.hw_dmT_cnrHiFltCur_wgt, pAdebayerCtx->select_param_v3.hw_dmT_cnrHiFltWgt_minLimit);

    LOGD_ADEBAYER("c_filter:ISO(%d),alpha_offset=%f,alpha_scale=%f,edge_offset=%f,edge_scale=%f", iso,
                  pAdebayerCtx->select_param_v3.hw_dmT_cnrMoireAlpha_offset, pAdebayerCtx->select_param_v3.sw_dmT_cnrMoireAlpha_scale,
                  pAdebayerCtx->select_param_v3.hw_dmT_cnrEdgeAlpha_offset, pAdebayerCtx->select_param_v3.sw_dmT_cnrEdgeAlpha_scale);

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

        //static

        pAdebayerCtx->select_param_v3.hw_dmT_en = pAdebayerCtx->full_param_v3.en;

        pAdebayerCtx->select_param_v3.hw_dmT_hiDrctFlt_coeff[0] = pAdebayerCtx->full_param_v3.sta.gDrctWgt.hw_dmT_hiDrctFlt_coeff[0];
        pAdebayerCtx->select_param_v3.hw_dmT_hiDrctFlt_coeff[1] = pAdebayerCtx->full_param_v3.sta.gDrctWgt.hw_dmT_hiDrctFlt_coeff[1];
        pAdebayerCtx->select_param_v3.hw_dmT_hiDrctFlt_coeff[2] = pAdebayerCtx->full_param_v3.sta.gDrctWgt.hw_dmT_hiDrctFlt_coeff[2];
        pAdebayerCtx->select_param_v3.hw_dmT_hiDrctFlt_coeff[3] = pAdebayerCtx->full_param_v3.sta.gDrctWgt.hw_dmT_hiDrctFlt_coeff[3];

        pAdebayerCtx->select_param_v3.hw_dmT_loDrctFlt_coeff[0] = pAdebayerCtx->full_param_v3.sta.gDrctWgt.hw_dmT_loDrctFlt_coeff[0];
        pAdebayerCtx->select_param_v3.hw_dmT_loDrctFlt_coeff[1] = pAdebayerCtx->full_param_v3.sta.gDrctWgt.hw_dmT_loDrctFlt_coeff[1];
        pAdebayerCtx->select_param_v3.hw_dmT_loDrctFlt_coeff[2] = pAdebayerCtx->full_param_v3.sta.gDrctWgt.hw_dmT_loDrctFlt_coeff[2];
        pAdebayerCtx->select_param_v3.hw_dmT_loDrctFlt_coeff[3] = pAdebayerCtx->full_param_v3.sta.gDrctWgt.hw_dmT_loDrctFlt_coeff[3];

        pAdebayerCtx->select_param_v3.sw_dmT_luma_val[0] = pAdebayerCtx->full_param_v3.sta.gDrctWgt.sw_dmT_luma_val[0];
        pAdebayerCtx->select_param_v3.sw_dmT_luma_val[1] = pAdebayerCtx->full_param_v3.sta.gDrctWgt.sw_dmT_luma_val[1];
        pAdebayerCtx->select_param_v3.sw_dmT_luma_val[2] = pAdebayerCtx->full_param_v3.sta.gDrctWgt.sw_dmT_luma_val[2];
        pAdebayerCtx->select_param_v3.sw_dmT_luma_val[3] = pAdebayerCtx->full_param_v3.sta.gDrctWgt.sw_dmT_luma_val[3];
        pAdebayerCtx->select_param_v3.sw_dmT_luma_val[4] = pAdebayerCtx->full_param_v3.sta.gDrctWgt.sw_dmT_luma_val[4];
        pAdebayerCtx->select_param_v3.sw_dmT_luma_val[5] = pAdebayerCtx->full_param_v3.sta.gDrctWgt.sw_dmT_luma_val[5];
        pAdebayerCtx->select_param_v3.sw_dmT_luma_val[6] = pAdebayerCtx->full_param_v3.sta.gDrctWgt.sw_dmT_luma_val[6];
        pAdebayerCtx->select_param_v3.sw_dmT_luma_val[7] = pAdebayerCtx->full_param_v3.sta.gDrctWgt.sw_dmT_luma_val[7];

        pAdebayerCtx->select_param_v3.hw_dmT_cnrAlphaLpf_coeff[0] = pAdebayerCtx->full_param_v3.sta.cFlt.hw_dmT_cnrAlphaLpf_coeff[0];
        pAdebayerCtx->select_param_v3.hw_dmT_cnrAlphaLpf_coeff[1] = pAdebayerCtx->full_param_v3.sta.cFlt.hw_dmT_cnrAlphaLpf_coeff[1];
        pAdebayerCtx->select_param_v3.hw_dmT_cnrAlphaLpf_coeff[2] = pAdebayerCtx->full_param_v3.sta.cFlt.hw_dmT_cnrAlphaLpf_coeff[2];

        pAdebayerCtx->select_param_v3.hw_dmT_cnrLoGuideLpf_coeff[0] = pAdebayerCtx->full_param_v3.sta.cFlt.hw_dmT_cnrLoGuideLpf_coeff[0];
        pAdebayerCtx->select_param_v3.hw_dmT_cnrLoGuideLpf_coeff[1] = pAdebayerCtx->full_param_v3.sta.cFlt.hw_dmT_cnrLoGuideLpf_coeff[1];
        pAdebayerCtx->select_param_v3.hw_dmT_cnrLoGuideLpf_coeff[2] = pAdebayerCtx->full_param_v3.sta.cFlt.hw_dmT_cnrLoGuideLpf_coeff[2];

        pAdebayerCtx->select_param_v3.hw_dmT_cnrPreFlt_coeff[0] = pAdebayerCtx->full_param_v3.sta.cFlt.hw_dmT_cnrPreFlt_coeff[0];
        pAdebayerCtx->select_param_v3.hw_dmT_cnrPreFlt_coeff[1] = pAdebayerCtx->full_param_v3.sta.cFlt.hw_dmT_cnrPreFlt_coeff[1];
        pAdebayerCtx->select_param_v3.hw_dmT_cnrPreFlt_coeff[2] = pAdebayerCtx->full_param_v3.sta.cFlt.hw_dmT_cnrPreFlt_coeff[2];

        // TODO: copy params from json, and select params according to ISO

        AdebayerSelectParam(pAdebayerCtx, iso);

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
    AdebayerProcResultV3_t* pAdebayerResult
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

    /* CONTROL */

    pAdebayerCtx->config.enable = pAdebayerCtx->select_param_v3.hw_dmT_en;
    pAdebayerCtx->config.filter_g_en = pAdebayerCtx->select_param_v3.hw_dmT_gOutlsFlt_en;
    pAdebayerCtx->config.filter_c_en = pAdebayerCtx->select_param_v3.hw_dmT_cnrFlt_en;

    LOGD_ADEBAYER("FIX 2 REG: debayer_en=%d,filter_g_en=%d,filter_c_en=%d",
                  pAdebayerCtx->config.enable, pAdebayerCtx->config.filter_g_en,
                  pAdebayerCtx->config.filter_c_en);

    /* LUMA_DX */
    pAdebayerCtx->config.luma_dx[0] = MIN(15, LOG2(pAdebayerCtx->select_param_v3.sw_dmT_luma_val[1]));
    pAdebayerCtx->config.luma_dx[1] = MIN(15, LOG2(pAdebayerCtx->select_param_v3.sw_dmT_luma_val[2] - pAdebayerCtx->select_param_v3.sw_dmT_luma_val[1]));
    pAdebayerCtx->config.luma_dx[2] = MIN(15, LOG2(pAdebayerCtx->select_param_v3.sw_dmT_luma_val[3] - pAdebayerCtx->select_param_v3.sw_dmT_luma_val[2]));
    pAdebayerCtx->config.luma_dx[3] = MIN(15, LOG2(pAdebayerCtx->select_param_v3.sw_dmT_luma_val[4] - pAdebayerCtx->select_param_v3.sw_dmT_luma_val[3]));
    pAdebayerCtx->config.luma_dx[4] = MIN(15, LOG2(pAdebayerCtx->select_param_v3.sw_dmT_luma_val[5] - pAdebayerCtx->select_param_v3.sw_dmT_luma_val[4]));
    pAdebayerCtx->config.luma_dx[5] = MIN(15, LOG2(pAdebayerCtx->select_param_v3.sw_dmT_luma_val[6] - pAdebayerCtx->select_param_v3.sw_dmT_luma_val[5]));
    pAdebayerCtx->config.luma_dx[6] = MIN(15, LOG2(pAdebayerCtx->select_param_v3.sw_dmT_luma_val[7] - pAdebayerCtx->select_param_v3.sw_dmT_luma_val[6]));

    LOGD_ADEBAYER("FIX 2 REG: luma_dx0~6=%d %d %d %d %d %d %d",
                  pAdebayerCtx->config.luma_dx[0], pAdebayerCtx->config.luma_dx[1],
                  pAdebayerCtx->config.luma_dx[2], pAdebayerCtx->config.luma_dx[3],
                  pAdebayerCtx->config.luma_dx[4], pAdebayerCtx->config.luma_dx[5],
                  pAdebayerCtx->config.luma_dx[6]);

    /* G_INTERP */

    pAdebayerCtx->config.clip_en = pAdebayerCtx->select_param_v3.hw_dmT_gInterpClip_en;
    pAdebayerCtx->config.dist_scale = pAdebayerCtx->select_param_v3.hw_dmT_hiTexture_thred;
    pAdebayerCtx->config.thed0 = pAdebayerCtx->select_param_v3.hw_dmT_hiDrct_thred;
    pAdebayerCtx->config.thed1 = pAdebayerCtx->select_param_v3.hw_dmT_loDrct_thred;
    pAdebayerCtx->config.select_thed = pAdebayerCtx->select_param_v3.hw_dmT_drctMethod_thred;
    pAdebayerCtx->config.max_ratio = pAdebayerCtx->select_param_v3.hw_dmT_gInterpSharpStrg_maxLimit;

    LOGD_ADEBAYER("FIX 2 REG: clip_en=%d,dist_scale=%d,thed0=%d,thed1=%d,select_thed=%d,max_ratio=%d",
                  pAdebayerCtx->config.clip_en, pAdebayerCtx->config.dist_scale, pAdebayerCtx->config.thed0, pAdebayerCtx->config.thed1,
                  pAdebayerCtx->config.select_thed, pAdebayerCtx->config.max_ratio);

    /* G_INTERP_FILTER1 */
    pAdebayerCtx->config.filter1_coe1 = pAdebayerCtx->select_param_v3.hw_dmT_loDrctFlt_coeff[0];
    pAdebayerCtx->config.filter1_coe2 = pAdebayerCtx->select_param_v3.hw_dmT_loDrctFlt_coeff[1];
    pAdebayerCtx->config.filter1_coe3 = pAdebayerCtx->select_param_v3.hw_dmT_loDrctFlt_coeff[2];
    pAdebayerCtx->config.filter1_coe4 = pAdebayerCtx->select_param_v3.hw_dmT_loDrctFlt_coeff[3];

    LOGD_ADEBAYER("FIX 2 REG: filter1_coe=%d %d %d %d",
                  pAdebayerCtx->config.filter1_coe1,
                  pAdebayerCtx->config.filter1_coe2,
                  pAdebayerCtx->config.filter1_coe3,
                  pAdebayerCtx->config.filter1_coe4);

    /* G_INTERP_FILTER2 */
    pAdebayerCtx->config.filter2_coe1 = pAdebayerCtx->select_param_v3.hw_dmT_hiDrctFlt_coeff[0];
    pAdebayerCtx->config.filter2_coe2 = pAdebayerCtx->select_param_v3.hw_dmT_hiDrctFlt_coeff[1];
    pAdebayerCtx->config.filter2_coe3 = pAdebayerCtx->select_param_v3.hw_dmT_hiDrctFlt_coeff[2];
    pAdebayerCtx->config.filter2_coe4 = pAdebayerCtx->select_param_v3.hw_dmT_hiDrctFlt_coeff[3];

    LOGD_ADEBAYER("FIX 2 REG: filter2_coe=%d %d %d %d",
                  pAdebayerCtx->config.filter2_coe1,
                  pAdebayerCtx->config.filter2_coe2,
                  pAdebayerCtx->config.filter2_coe3,
                  pAdebayerCtx->config.filter2_coe4);

    /* G_INTERP_OFFSET_ALPHA */
    pAdebayerCtx->config.gain_offset = pAdebayerCtx->select_param_v3.hw_dmT_gInterpSharpStrg_offset;
    pAdebayerCtx->config.gradloflt_alpha = ROUND_F(pAdebayerCtx->select_param_v3.sw_dmT_gradLoFlt_alpha * (1 << RK_DEBAYER_V31_FIX_BIT_GRAD_FLT_ALPHA));
    pAdebayerCtx->config.wgt_alpha = ROUND_F(pAdebayerCtx->select_param_v3.sw_dmT_gInterpWgtFlt_alpha * (1 << RK_DEBAYER_V31_FIX_BIT_WGT_FLT_ALPHA));

    LOGD_ADEBAYER("FIX 2 REG: gain_offset=%d gradloflt_alpha=%d wgt_alpha=%d",
                  pAdebayerCtx->config.gain_offset,
                  pAdebayerCtx->config.gradloflt_alpha,
                  pAdebayerCtx->config.wgt_alpha);


    /* G_INTERP_DRCT_OFFSET */
    for (int i = 0; i < RK_DEBAYER_V31_LUMA_POINT_NUM; i++) {
        // 128 : sum grad coeff : 8, sum grad filter coeff : 16, 8 * 16 = 128;
        pAdebayerCtx->config.drct_offset[i] = MAX(pAdebayerCtx->select_param_v3.sw_dmT_drct_offset[i] * 128, 1);
    }

    LOGD_ADEBAYER("FIX 2 REG: drct offset0~7=%d %d %d %d %d %d %d %d",
                  pAdebayerCtx->config.drct_offset[0], pAdebayerCtx->config.drct_offset[1],
                  pAdebayerCtx->config.drct_offset[2], pAdebayerCtx->config.drct_offset[3],
                  pAdebayerCtx->config.drct_offset[4], pAdebayerCtx->config.drct_offset[5],
                  pAdebayerCtx->config.drct_offset[6], pAdebayerCtx->config.drct_offset[7]);

    /* G_FILTER_MODE_OFFSET */
    pAdebayerCtx->config.gfilter_mode = pAdebayerCtx->select_param_v3.hw_dmT_gOutlsFlt_mode;
    pAdebayerCtx->config.bf_ratio = ROUND_F(pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_ratio * (1 << RK_DEBAYER_V31_FIX_BIT_BF_RATIO));
    pAdebayerCtx->config.offset = pAdebayerCtx->select_param_v3.hw_dmT_gOutlsFltRange_offset;

    LOGD_ADEBAYER("FIX 2 REG: gfilter_mode=%d bf_ratio=%d offset=%d",
                  pAdebayerCtx->config.gfilter_mode,
                  pAdebayerCtx->config.bf_ratio,
                  pAdebayerCtx->config.offset);

    /* G_FILTER_FILTER */

    if (pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFltRsigma_en)
    {
        float dis_table_3x3[3] = { 0.0, 1.0, 2.0};
        float gaus_table[3];

        float sigma = pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_rsigma;
        double e = 2.71828182845905;
        for (int k = 0; k < 3; k++)
        {
            float tmp = pow(e, -dis_table_3x3[k] / 2.0 / sigma / sigma);
            gaus_table[k] = tmp;
        }

        float sumTable = 0;
        sumTable = gaus_table[0]
                   + 4 * gaus_table[1]
                   + 4 * gaus_table[2];

        for (int k = 0; k < 3; k++)
        {
            gaus_table[k] = gaus_table[k] / sumTable;
            pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_coeff[k] = gaus_table[k];
        }
    }

    int filter_coe[3];
    for (int i = 0; i < 3; i ++)
    {
        filter_coe[i] = ROUND_F(pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_coeff[i] * (1 << RK_DEBAYER_V31_FIX_BIT_BF_WGT));
    }

    // check filter coeff
    int sum_coeff, offset;
    sum_coeff = filter_coe[0] + 4 * filter_coe[1] + 4 * filter_coe[2];
    offset = (1 << RK_DEBAYER_V31_FIX_BIT_BF_WGT) - sum_coeff;
    filter_coe[0] = filter_coe[0] + offset;

    pAdebayerCtx->config.filter_coe0 = filter_coe[0];
    pAdebayerCtx->config.filter_coe1 = filter_coe[1];
    pAdebayerCtx->config.filter_coe2 = filter_coe[2];

    LOGD_ADEBAYER("FIX 2 REG: filter_coe=%d %d %d",
                  pAdebayerCtx->config.filter_coe0,
                  pAdebayerCtx->config.filter_coe1,
                  pAdebayerCtx->config.filter_coe2);

    /* G_FILTER_VSIGMA */
    for(int i = 0; i < RK_DEBAYER_V31_LUMA_POINT_NUM; i++)
    {
        pAdebayerCtx->config.vsigma[i]      = ROUND_F((1 << RK_DEBAYER_V31_FIX_BIT_INV_BF_SIGMA) / (float)pAdebayerCtx->select_param_v3.sw_dmT_gOutlsFlt_vsigma[i]);
    }

    LOGD_ADEBAYER("FIX 2 REG: vsigma0~7=%d %d %d %d %d %d %d %d",
                  pAdebayerCtx->config.vsigma[0], pAdebayerCtx->config.vsigma[1],
                  pAdebayerCtx->config.vsigma[2], pAdebayerCtx->config.vsigma[3],
                  pAdebayerCtx->config.vsigma[4], pAdebayerCtx->config.vsigma[5],
                  pAdebayerCtx->config.vsigma[6], pAdebayerCtx->config.vsigma[7]);


    /* C_FILTER_GUIDE_GAUS */
    pAdebayerCtx->config.guid_gaus_coe0 = pAdebayerCtx->select_param_v3.hw_dmT_cnrLoGuideLpf_coeff[0];
    pAdebayerCtx->config.guid_gaus_coe1 = pAdebayerCtx->select_param_v3.hw_dmT_cnrLoGuideLpf_coeff[1];
    pAdebayerCtx->config.guid_gaus_coe2 = pAdebayerCtx->select_param_v3.hw_dmT_cnrLoGuideLpf_coeff[2];

    /* C_FILTER_CE_GAUS */
    pAdebayerCtx->config.ce_gaus_coe0 = pAdebayerCtx->select_param_v3.hw_dmT_cnrPreFlt_coeff[0];
    pAdebayerCtx->config.ce_gaus_coe1 = pAdebayerCtx->select_param_v3.hw_dmT_cnrPreFlt_coeff[1];
    pAdebayerCtx->config.ce_gaus_coe2 = pAdebayerCtx->select_param_v3.hw_dmT_cnrPreFlt_coeff[2];

    /* C_FILTER_ALPHA_GAUS */
    pAdebayerCtx->config.alpha_gaus_coe0 = pAdebayerCtx->select_param_v3.hw_dmT_cnrAlphaLpf_coeff[0];
    pAdebayerCtx->config.alpha_gaus_coe1 = pAdebayerCtx->select_param_v3.hw_dmT_cnrAlphaLpf_coeff[1];
    pAdebayerCtx->config.alpha_gaus_coe2 = pAdebayerCtx->select_param_v3.hw_dmT_cnrAlphaLpf_coeff[2];

    LOGD_ADEBAYER("FIX 2 REG: guid_gaus_coe=%d %d %d, ce_gaus_coe=%d %d %d,alpha_gaus_coe=%d %d %d",
                  pAdebayerCtx->config.guid_gaus_coe0,          pAdebayerCtx->config.guid_gaus_coe1,
                  pAdebayerCtx->config.guid_gaus_coe2,          pAdebayerCtx->config.ce_gaus_coe0,
                  pAdebayerCtx->config.ce_gaus_coe1,          pAdebayerCtx->config.ce_gaus_coe2,
                  pAdebayerCtx->config.alpha_gaus_coe0,       pAdebayerCtx->config.alpha_gaus_coe1,
                  pAdebayerCtx->config.alpha_gaus_coe2);


    /* C_FILTER_LOG_OFFSET */
    if(pAdebayerCtx->compr_bit > 0)
        pAdebayerCtx->config.log_en = 0;
    else
        pAdebayerCtx->config.log_en = 1;

    pAdebayerCtx->config.loghf_offset = pAdebayerCtx->select_param_v3.hw_dmT_cnrLogGrad_offset;
    pAdebayerCtx->config.loggd_offset = pAdebayerCtx->select_param_v3.hw_dmT_cnrLogGuide_offset;

    LOGD_ADEBAYER("FIX 2 REG: log_en=%d loghf_offset=%d loggd_offset=%d",
                  pAdebayerCtx->config.log_en,
                  pAdebayerCtx->config.loghf_offset,
                  pAdebayerCtx->config.loggd_offset);

    /* C_FILTER_ALPHA */
    pAdebayerCtx->config.alpha_offset = pAdebayerCtx->select_param_v3.hw_dmT_cnrMoireAlpha_offset;
    pAdebayerCtx->config.alpha_scale =  ROUND_F(pAdebayerCtx->select_param_v3.sw_dmT_cnrMoireAlpha_scale * (1 << RK_DEBAYER_V2_FIX_BIT_ALPHA_SCALE));

    /* C_FILTER_EDGE */
    pAdebayerCtx->config.edge_offset = pAdebayerCtx->select_param_v3.hw_dmT_cnrEdgeAlpha_offset;
    pAdebayerCtx->config.edge_scale = ROUND_F(pAdebayerCtx->select_param_v3.sw_dmT_cnrEdgeAlpha_scale * (1 << RK_DEBAYER_V2_FIX_BIT_EDGE_SCALE));

    LOGD_ADEBAYER("FIX 2 REG: alpha_offset=%d alpha_scale=%d edge_offset=%d, edge_scale=%d",
                  pAdebayerCtx->config.alpha_offset, pAdebayerCtx->config.alpha_scale,
                  pAdebayerCtx->config.edge_offset, pAdebayerCtx->config.edge_scale);

    /* C_FILTER_IIR_0 */

    // log fix bit : 10, 1 / sigma, (1 << RK_DEBAYER_V31_FIX_BIT_INV_SIGMA) / (sigma)
    float sqrtLog2e                         = 1.2011;
    int hw_dmT_cnrLoFltVsigma_inv           = ROUND_F((1 << RK_DEBAYER_V31_FIX_BIT_INV_SIGMA) * sqrtLog2e / ((1 << RK_DEBAYER_V31_FIX_BIT_LOG2) * pAdebayerCtx->select_param_v3.sw_dmT_cnrLoFlt_vsigma));
    pAdebayerCtx->config.wgtslope           = ROUND_F(pAdebayerCtx->select_param_v3.sw_dmT_cnrLoFltWgt_slope * (1 << RK_DEBAYER_V31_FIX_BIT_SLOPE));

    // chromaFilterStrength * wgtSlope, int to float
    int tmptmp                              = hw_dmT_cnrLoFltVsigma_inv * pAdebayerCtx->config.wgtslope;
    int shiftBit                            = LOG2(tmptmp) - RK_DEBAYER_V31_FIX_BIT_INT_TO_FLOAT;
    shiftBit                                = MAX(shiftBit, 0);
    pAdebayerCtx->config.ce_sgm             = ROUND_F((float)tmptmp / (1 << shiftBit));
    pAdebayerCtx->config.exp_shift          = RK_DEBAYER_V31_FIX_BIT_INV_SIGMA - shiftBit;

    /* C_FILTER_IIR_1 */
    pAdebayerCtx->config.wet_clip           = ROUND_F(pAdebayerCtx->select_param_v3.sw_dmT_cnrLoFltWgt_maxLimit * (1 << RK_DEBAYER_V31_FIX_BIT_WGT_RATIO));
    pAdebayerCtx->config.wet_ghost          = ROUND_F(pAdebayerCtx->select_param_v3.sw_dmT_cnrLoFltWgt_minThred * (1 << RK_DEBAYER_V31_FIX_BIT_IIR_WGT));

    /* C_FILTER_BF */
    pAdebayerCtx->config.bf_clip = pAdebayerCtx->select_param_v3.hw_dmT_cnrHiFltWgt_minLimit;
    pAdebayerCtx->config.bf_curwgt = pAdebayerCtx->select_param_v3.hw_dmT_cnrHiFltCur_wgt;

    int scale                   = (1 << 12) - 1; //rawbit: 12
    float log2e                 = 0.8493;
    pAdebayerCtx->config.bf_sgm = (int)((1 << RK_DEBAYER_V2_FIX_BIT_INV_BF_SIGMA) * log2e / (pAdebayerCtx->select_param_v3.sw_dmT_cnrHiFlt_vsigma * scale));

    LOGD_ADEBAYER("FIX 2 REG: wgtslope=%d,ce_sgm=%d,exp_shift=%d,wet_clip=%d,wet_ghost=%d,bf_clip=%d,bf_curwgt=%d,bf_sgm=%d",
                  pAdebayerCtx->config.wgtslope, pAdebayerCtx->config.ce_sgm, pAdebayerCtx->config.exp_shift,
                  pAdebayerCtx->config.wet_clip, pAdebayerCtx->config.wet_ghost,  pAdebayerCtx->config.bf_clip,
                  pAdebayerCtx->config.bf_curwgt, pAdebayerCtx->config.bf_sgm);


    // TODO: copy regvalue result
    *pAdebayerResult->config = pAdebayerCtx->config;
    pAdebayerCtx->config.updatecfg = false;

    LOGI_ADEBAYER("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;
}



