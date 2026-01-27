
#include "rk_aiq_ayuvme_algo_yuvme_v1.h"

RKAIQ_BEGIN_DECLARE


Ayuvme_result_V1_t yuvme_select_params_by_ISO_V1(RK_YUVME_Params_V1_t *pParams, RK_YUVME_Params_V1_Select_t *pSelect, Ayuvme_ExpInfo_V1_t *pExpInfo)
{
    short multBit;
    float ratio = 0.0f;
    int iso = 50;
    RK_YUVME_Params_V1_Select_t *pParamHi = NULL;
    RK_YUVME_Params_V1_Select_t *pParamLo = NULL;


    Ayuvme_result_V1_t res = AYUVMEV1_RET_SUCCESS;

    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_NULL_POINTER;
    }

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_NULL_POINTER;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];

    int iso_div = 50;
    int lowIso = 50;
    int highIso = 50;
    int minIso = 50;
    int maxIso = 50;
    int isoLevelLow = 0;
    int isoLevelHig = 0;

    for(int i = 0; i < RK_YUVME_V1_MAX_ISO_NUM - 1; i++) {
#ifndef RK_SIMULATOR_HW
        lowIso = pParams->iso[i];
        highIso = pParams->iso[i + 1];
#else
        lowIso = iso_div * (1 << i);
        highIso = iso_div * (1 << (i + 1));
#endif
        if(iso >= lowIso && iso <= highIso) {
            ratio = (iso - lowIso ) / (float)(highIso - lowIso);
            pParamLo = &pParams->arYuvmeParamsISO[i];
            pParamHi = &pParams->arYuvmeParamsISO[i + 1];
            isoLevelLow = i;
            isoLevelHig = i + 1;
            break;
        }
    }

#ifndef RK_SIMULATOR_HW
    minIso = pParams->iso[0];
    maxIso = pParams->iso[RK_YUVME_V1_MAX_ISO_NUM - 1];
#else
    minIso = iso_div * (1 << 0);
    maxIso = iso_div * (1 << (RK_YUVME_V1_MAX_ISO_NUM - 1));
#endif

    if(iso < minIso) {
        ratio = 0;
        pParamLo = &pParams->arYuvmeParamsISO[0];
        pParamHi = &pParams->arYuvmeParamsISO[1];
        isoLevelLow = 0;
        isoLevelHig = 1;
    }

    if(iso > maxIso) {
        ratio = 1;
        pParamLo = &pParams->arYuvmeParamsISO[RK_YUVME_V1_MAX_ISO_NUM - 2];
        pParamHi = &pParams->arYuvmeParamsISO[RK_YUVME_V1_MAX_ISO_NUM - 1];
        isoLevelLow = RK_YUVME_V1_MAX_ISO_NUM - 2;
        isoLevelHig = RK_YUVME_V1_MAX_ISO_NUM - 1;
    }


    LOGD_ANR("oyyf %s:%d  iso:%d low:%d hight:%d ratio:%f n", __FUNCTION__, __LINE__,
             iso, lowIso, highIso, ratio);

    pExpInfo->isoLevelLow = isoLevelLow;
    pExpInfo->isoLevelHig = isoLevelHig;

    pSelect->enable = pParams->enable;
    pSelect->hw_yuvme_btnrMap_en = pParamLo->hw_yuvme_btnrMap_en;
    // pSelect->hw_yuvme_debug_mode = pParams->hw_yuvme_debug_mode;

    pSelect->hw_yuvme_searchRange_mode = ROUND_I32(INTERP1(pParamLo->hw_yuvme_searchRange_mode, pParamHi->hw_yuvme_searchRange_mode, ratio));
    pSelect->hw_yuvme_timeRelevance_offset = ROUND_I32(INTERP1(pParamLo->hw_yuvme_timeRelevance_offset, pParamHi->hw_yuvme_timeRelevance_offset, ratio));
    pSelect->hw_yuvme_spaceRelevance_offset = ROUND_I32(INTERP1(pParamLo->hw_yuvme_spaceRelevance_offset, pParamHi->hw_yuvme_spaceRelevance_offset, ratio));
    pSelect->hw_yuvme_staticDetect_thred = ROUND_I32(INTERP1(pParamLo->hw_yuvme_staticDetect_thred, pParamHi->hw_yuvme_staticDetect_thred, ratio));

    pSelect->sw_yuvme_globalNr_strg = INTERP1(pParamLo->sw_yuvme_globalNr_strg, pParamHi->sw_yuvme_globalNr_strg, ratio);
    pSelect->sw_yuvme_nrDiff_scale = INTERP1(pParamLo->sw_yuvme_nrDiff_scale, pParamHi->sw_yuvme_nrDiff_scale, ratio);
    pSelect->sw_yuvme_nrStatic_scale = INTERP1(pParamLo->sw_yuvme_nrStatic_scale, pParamHi->sw_yuvme_nrStatic_scale, ratio);

    for (int i = 0; i < 16; i++)
    {
        pSelect->hw_yuvme_nrLuma2Sigma_val[i] = ROUND_I32(INTERP1(pParamLo->hw_yuvme_nrLuma2Sigma_val[i], pParamHi->hw_yuvme_nrLuma2Sigma_val[i], ratio));
    }

    pSelect->sw_yuvme_nrFusion_limit = INTERP1(pParamLo->sw_yuvme_nrFusion_limit, pParamHi->sw_yuvme_nrFusion_limit, ratio);
    pSelect->sw_yuvme_nrMotion_scale = INTERP1(pParamLo->sw_yuvme_nrMotion_scale, pParamHi->sw_yuvme_nrMotion_scale, ratio);

    pSelect->sw_yuvme_curWeight_limit = INTERP1(pParamLo->sw_yuvme_curWeight_limit, pParamHi->sw_yuvme_curWeight_limit, ratio);
    pSelect->hw_yuvme_nrFusion_mode = ROUND_I32(INTERP1(pParamLo->hw_yuvme_nrFusion_mode, pParamHi->hw_yuvme_nrFusion_mode, ratio));
    return res;
}

Ayuvme_result_V1_t yuvme_fix_transfer_V1(RK_YUVME_Params_V1_Select_t* pSelect, RK_YUVME_Fix_V1_t *pFix, rk_aiq_yuvme_strength_v1_t* pStrength, Ayuvme_ExpInfo_V1_t *pExpInfo)
{
    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    Ayuvme_result_V1_t res = AYUVMEV1_RET_SUCCESS;

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_NULL_POINTER;
    }

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_NULL_POINTER;
    }

    if(pStrength == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_NULL_POINTER;
    }

    float fStrength = 1.0;

    if(pStrength->strength_enable) {
        fStrength = pStrength->percent;
    }
    if(fStrength <= 0.0) {
        fStrength = 0.000001;
    }

    LOGD_ANR("strength_enable:%d fStrength: %f  hw_yuvme_btnrMap_en:%d tnr_en:%d\n",
             pStrength->strength_enable, fStrength,
             pSelect->hw_yuvme_btnrMap_en, pExpInfo->bayertnr_en);

    LOGD_ANR("%s:%d strength:%f raw:width:%d height:%d\n",
             __FUNCTION__, __LINE__,
             fStrength, pExpInfo->rawHeight, pExpInfo->rawWidth);

    // YUVME_2700_GLOBAL_CTRL (0x0000)
    pFix->bypass = !pSelect->enable;
    pFix->tnr_wgt0_en = !pSelect->hw_yuvme_btnrMap_en;
    if(!pExpInfo->bayertnr_en) {
        //when tnr is disable , tnr_wgt0_en must be 1 to bypss tnr wgt for yuvme
        pFix->tnr_wgt0_en = 1;
    }

    /* YUVME_PARA0 */
    pFix->global_nr_strg =  CLIP(ROUND_I32(pSelect->sw_yuvme_globalNr_strg * 256), 0, 0xff);

#if 0
    pFix->wgt_fact3 = CLIP(int(pSelect->sw_yuvme_globalNr_strg  * 0.01 * 1023 * 1023 / 64 + 0.5), 0, 0xff);
#else
    float tmp = 0.0f;
    tmp = (pFix->global_nr_strg  * 0.01 * 1023 * 1023 / 64);
    if(tmp == 0.0f ) {
        pFix->wgt_fact3 = 0xff;
    } else {
        pFix->wgt_fact3 = CLIP(ROUND_I32(32768 / tmp), 0, 0xff);
    }
#endif


    pFix->search_range_mode = CLIP(ROUND_I32(pSelect->hw_yuvme_searchRange_mode), 0, 2);

    pFix->static_detect_thred = CLIP(ROUND_I32(pSelect->hw_yuvme_staticDetect_thred), 0, 63);

    /* YUVME_PARA1 */
    pFix->time_relevance_offset = CLIP(ROUND_I32(pSelect->hw_yuvme_timeRelevance_offset), 0, 15);

    pFix->space_relevance_offset = CLIP(ROUND_I32(pSelect->hw_yuvme_spaceRelevance_offset), 0, 15);
    pFix->nr_diff_scale = CLIP(ROUND_I32(pSelect->sw_yuvme_nrDiff_scale * 16), 1, 255);

    pFix->nr_fusion_limit = 1023 - CLIP(ROUND_I32(pSelect->sw_yuvme_nrFusion_limit * 1024), 0, 1023);
    /* YUVME_PARA2 */
    pFix->nr_static_scale = CLIP(ROUND_I32(pSelect->sw_yuvme_nrStatic_scale * 16), 1, 255);
    pFix->nr_motion_scale = CLIP(ROUND_I32(pSelect->sw_yuvme_nrMotion_scale * 256), 0, 511);

    /* YUVME_SIGMA */
    for (int i = 0; i < 16; i++)
    {
        pFix->nr_luma2sigma_val[i] = CLIP(pSelect->hw_yuvme_nrLuma2Sigma_val[i], 0, 1023);
    }

    pFix->nr_fusion_mode = CLIP(ROUND_I32(pSelect->hw_yuvme_nrFusion_mode), 0, 3);
    pFix->cur_weight_limit = CLIP(ROUND_I32(pSelect->sw_yuvme_curWeight_limit * 2048), 0, 2047);

    yuvme_fix_printf_V1(pFix);
    return res;
}

Ayuvme_result_V1_t yuvme_fix_printf_V1(RK_YUVME_Fix_V1_t * pFix)
{
    LOGD_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    Ayuvme_result_V1_t res = AYUVMEV1_RET_SUCCESS;

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_NULL_POINTER;
    }


    // CTRL (0x0000)
    LOGD_ANR("(0x0000) bypass:0x%x tnr_wgt0_en:0x%x enable:0x%x \n",
             pFix->bypass,
             pFix->tnr_wgt0_en,
             !pFix->bypass);

    // YUVME_PARA0 (0x0004)
    LOGD_ANR("(0x0004) global_nr_strg:0x%x wgt_fact3:0x%x search_range_mode:0x%x  static_detect_thred:0x%x \n",
             pFix->global_nr_strg,
             pFix->wgt_fact3,
             pFix->search_range_mode,
             pFix->static_detect_thred);


    // YUVME_PARA1 (0x0008)
    LOGD_ANR("(0x0008) time_relevance_offset:0x%x space_relevance_offset:0x%x nr_diff_scale:0x%x  nr_fusion_limit:0x%x \n",
             pFix->time_relevance_offset,
             pFix->space_relevance_offset,
             pFix->nr_diff_scale,
             pFix->nr_fusion_limit);

    // YUVME_PARA2  (0x000c)
    LOGD_ANR("(0x000c) nr_static_scale:0x%x  nr_motion_scale:0x%x\n",
             pFix->nr_static_scale, pFix->nr_motion_scale);

    // YUVME_SIGMA (0x0010 - 0x0024)
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x0010- 0x0024) nr_luma2sigma_val[%d]:0x%x \n",
                 i, pFix->nr_luma2sigma_val[i]);
    }
    return res;
}



Ayuvme_result_V1_t yuvme_get_setting_by_name_json_V1(CalibDbV2_YuvmeV1_t *pCalibdbV2, char *name, int* tuning_idx)
{
    int i = 0;
    Ayuvme_result_V1_t res = AYUVMEV1_RET_SUCCESS;

    if(pCalibdbV2 == NULL || name == NULL
            || tuning_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_NULL_POINTER;
    }

    for(i = 0; i < pCalibdbV2->TuningPara.Setting_len; i++) {
        if(strncmp(name, pCalibdbV2->TuningPara.Setting[i].SNR_Mode, strlen(name)*sizeof(char)) == 0) {
            break;
        }
    }

    if(i < pCalibdbV2->TuningPara.Setting_len) {
        *tuning_idx = i;
    } else {
        *tuning_idx = 0;
    }

    LOGD_ANR("%s:%d snr_name:%s  snr_idx:%d i:%d \n",
             __FUNCTION__, __LINE__,
             name, *tuning_idx, i);

    return res;
}

Ayuvme_result_V1_t yuvme_init_params_json_V1(RK_YUVME_Params_V1_t *pYuvmeParams, CalibDbV2_YuvmeV1_t *pCalibdbV2,  int tuning_idx)
{
    Ayuvme_result_V1_t res = AYUVMEV1_RET_SUCCESS;
    int i = 0;
    int j = 0;

    CalibDbV2_YuvmeV1_T_ISO_t *pISO = NULL;

    LOGD_ANR("%s(%d): enter\n", __FUNCTION__, __LINE__);

    if(pYuvmeParams == NULL || pCalibdbV2 == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_NULL_POINTER;
    }

    LOGD_ANR(" iso len:%d calib_max:%d\n", pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO_len, RK_YUVME_V1_MAX_ISO_NUM);

    pYuvmeParams->enable = pCalibdbV2->TuningPara.enable;

    for(j = 0; j < pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO_len && j < RK_YUVME_V1_MAX_ISO_NUM; j++) {
        pISO = &pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO[j];

        for(i = 0; i < 16; i++) {
            pYuvmeParams->arYuvmeParamsISO[j].hw_yuvme_nrLuma2Sigma_val[i] = pISO->hw_yuvme_nrLuma2Sigma_val[i];
        }
        pYuvmeParams->iso[j] = pISO->iso;
        pYuvmeParams->arYuvmeParamsISO[j].enable = pCalibdbV2->TuningPara.enable;
        pYuvmeParams->arYuvmeParamsISO[j].hw_yuvme_btnrMap_en = pCalibdbV2->TuningPara.hw_yuvme_btnrMap_en;
        pYuvmeParams->arYuvmeParamsISO[j].hw_yuvme_searchRange_mode = pISO->hw_yuvme_searchRange_mode;
        pYuvmeParams->arYuvmeParamsISO[j].hw_yuvme_timeRelevance_offset = pISO->hw_yuvme_timeRelevance_offset;
        pYuvmeParams->arYuvmeParamsISO[j].hw_yuvme_spaceRelevance_offset = pISO->hw_yuvme_spaceRelevance_offset;
        pYuvmeParams->arYuvmeParamsISO[j].hw_yuvme_staticDetect_thred = pISO->hw_yuvme_staticDetect_thred;

        //low freq
        pYuvmeParams->arYuvmeParamsISO[j].sw_yuvme_globalNr_strg = pISO->sw_yuvme_globalNr_strg;
        pYuvmeParams->arYuvmeParamsISO[j].sw_yuvme_nrDiff_scale = pISO->sw_yuvme_nrDiff_scale;
        pYuvmeParams->arYuvmeParamsISO[j].sw_yuvme_nrStatic_scale = pISO->sw_yuvme_nrStatic_scale;
        pYuvmeParams->arYuvmeParamsISO[j].sw_yuvme_nrFusion_limit = pISO->sw_yuvme_nrFusion_limit;
        pYuvmeParams->arYuvmeParamsISO[j].sw_yuvme_nrMotion_scale = pISO->sw_yuvme_nrMotion_scale;
        pYuvmeParams->arYuvmeParamsISO[j].hw_yuvme_nrFusion_mode = pISO->hw_yuvme_nrFusion_mode;
        pYuvmeParams->arYuvmeParamsISO[j].sw_yuvme_curWeight_limit = pISO->sw_yuvme_curWeight_limit;

    }

    LOGD_ANR("%s(%d): exit\n", __FUNCTION__, __LINE__);

    return res;
}

Ayuvme_result_V1_t yuvme_config_setting_param_json_V1(RK_YUVME_Params_V1_t *pParams, CalibDbV2_YuvmeV1_t *pCalibdbV2, char* param_mode, char * snr_name)
{
    Ayuvme_result_V1_t res = AYUVMEV1_RET_SUCCESS;
    int tuning_idx = 0;

    if(pParams == NULL || pCalibdbV2 == NULL
            || param_mode == NULL || snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_NULL_POINTER;
    }

    res = yuvme_get_setting_by_name_json_V1(pCalibdbV2, snr_name,  &tuning_idx);
    if(res != AYUVMEV1_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = yuvme_init_params_json_V1(pParams, pCalibdbV2,  tuning_idx);
    pParams->enable = pCalibdbV2->TuningPara.enable;
    return res;

}

RKAIQ_END_DECLARE

