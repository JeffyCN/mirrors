
#include "rk_aiq_aynr_algo_ynr_v24.h"

#include "amerge/rk_aiq_types_amerge_algo_prvt.h"

RKAIQ_BEGIN_DECLARE

int ynrClipFloatValueV24(float posx, int BitInt, int BitFloat) {
    int yOutInt    = 0;
    int yOutIntMax = (int)(pow(2, (BitFloat + BitInt)) - 1);
    int yOutIntMin = 0;

    yOutInt = LIMIT_VALUE((int)(posx * pow(2, BitFloat)), yOutIntMax, yOutIntMin);

    return yOutInt;
}

Aynr_result_V24_t ynr_select_params_by_ISO_V24(RK_YNR_Params_V24_t* pParams,
        RK_YNR_Params_V24_Select_t* pSelect,
        Aynr_ExpInfo_V24_t* pExpInfo) {
    short multBit;
    float ratio                           = 0.0f;
    int iso                               = 50;
    RK_YNR_Params_V24_Select_t* pParamHi  = NULL;
    RK_YNR_Params_V24_Select_t* pParamLo  = NULL;
    RK_YNR_Params_V24_Select_t* pParamTmp = NULL;

    Aynr_result_V24_t res = AYNRV24_RET_SUCCESS;

    if (pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV24_RET_NULL_POINTER;
    }

    if (pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV24_RET_NULL_POINTER;
    }

    if (pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV24_RET_NULL_POINTER;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];

    // choose integer type data
    int cur_iso_idx = 0;
    int idx         = 0;
    for (idx = 0; idx < RK_YNR_V24_MAX_ISO_NUM; idx++) {
        if (iso < pParams->iso[idx]) {
            if (idx == 0) {
                cur_iso_idx = 0;
                break;
            } else {
                int dist1   = iso - pParams->iso[idx - 1];
                int dist2   = pParams->iso[idx] - iso;
                cur_iso_idx = (dist1 > dist2) ? (idx) : (idx - 1);
                break;
            }
        }
    }
    if (idx == RK_YNR_V24_MAX_ISO_NUM) cur_iso_idx = RK_YNR_V24_MAX_ISO_NUM - 1;

    pParamTmp = &pParams->arYnrParamsISO[cur_iso_idx];

    int iso_div     = 50;
    int lowIso      = 50;
    int highIso     = 50;
    int minIso      = 50;
    int maxIso      = 50;
    int isoLevelLow = 0;
    int isoLevelHig = 0;

    for (int i = 0; i < RK_YNR_V24_MAX_ISO_NUM - 1; i++) {
#ifndef RK_SIMULATOR_HW
        lowIso  = pParams->iso[i];
        highIso = pParams->iso[i + 1];
#else
        lowIso  = iso_div * (1 << i);
        highIso = iso_div * (1 << (i + 1));
#endif
        if (iso >= lowIso && iso <= highIso) {
            ratio       = (iso - lowIso) / (float)(highIso - lowIso);
            pParamLo    = &pParams->arYnrParamsISO[i];
            pParamHi    = &pParams->arYnrParamsISO[i + 1];
            isoLevelLow = i;
            isoLevelHig = i + 1;
            break;
        }
    }

#ifndef RK_SIMULATOR_HW
    minIso = pParams->iso[0];
    maxIso = pParams->iso[RK_YNR_V24_MAX_ISO_NUM - 1];
#else
    minIso = iso_div * (1 << 0);
    maxIso = iso_div * (1 << (RK_YNR_V24_MAX_ISO_NUM - 1));
#endif

    if (iso < minIso) {
        ratio       = 0;
        pParamLo    = &pParams->arYnrParamsISO[0];
        pParamHi    = &pParams->arYnrParamsISO[1];
        isoLevelLow = 0;
        isoLevelHig = 1;
    }

    if (iso > maxIso) {
        ratio       = 1;
        pParamLo    = &pParams->arYnrParamsISO[RK_YNR_V24_MAX_ISO_NUM - 2];
        pParamHi    = &pParams->arYnrParamsISO[RK_YNR_V24_MAX_ISO_NUM - 1];
        isoLevelLow = RK_YNR_V24_MAX_ISO_NUM - 2;
        isoLevelHig = RK_YNR_V24_MAX_ISO_NUM - 1;
    }

    LOGD_ANR("oyyf %s:%d  iso:%d low:%d hight:%d ratio:%f iso_index:%d \n", __FUNCTION__, __LINE__,
             iso, lowIso, highIso, ratio, cur_iso_idx);

    pSelect->enable = pParams->enable;

    pExpInfo->isoLevelLow = isoLevelLow;
    pExpInfo->isoLevelHig = isoLevelHig;

    pSelect->sw_ynr_loSpnr_bypass = pParamTmp->sw_ynr_loSpnr_bypass;
    pSelect->sw_ynr_hiSpnr_bypass = pParamTmp->sw_ynr_hiSpnr_bypass;

    pSelect->ynr_lci = ratio * (pParamHi->ynr_lci - pParamLo->ynr_lci) + pParamLo->ynr_lci;
    // global gain local gain cfg
    pSelect->sw_ynr_gainMerge_alpha =
        ratio * (pParamHi->sw_ynr_gainMerge_alpha - pParamLo->sw_ynr_gainMerge_alpha) +
        pParamLo->sw_ynr_gainMerge_alpha;
    pSelect->sw_ynr_globalSet_gain =
        ratio * (pParamHi->sw_ynr_globalSet_gain - pParamLo->sw_ynr_globalSet_gain) +
        pParamLo->sw_ynr_globalSet_gain;

    pSelect->sw_ynr_hiSpnr_strg =
        ratio * (pParamHi->sw_ynr_hiSpnr_strg - pParamLo->sw_ynr_hiSpnr_strg) +
        pParamLo->sw_ynr_hiSpnr_strg;
    pSelect->sw_ynr_hiSpnrLocalGain_alpha =
        ratio * (pParamHi->sw_ynr_hiSpnrLocalGain_alpha - pParamLo->sw_ynr_hiSpnrLocalGain_alpha) +
        pParamLo->sw_ynr_hiSpnrLocalGain_alpha;
    pSelect->sw_ynr_hiSpnrFilt_centerWgt =
        ratio * (pParamHi->sw_ynr_hiSpnrFilt_centerWgt - pParamLo->sw_ynr_hiSpnrFilt_centerWgt) +
        pParamLo->sw_ynr_hiSpnrFilt_centerWgt;
    pSelect->sw_ynr_hiSpnrFilt_wgtOffset =
        ratio * (pParamHi->sw_ynr_hiSpnrFilt_wgtOffset - pParamLo->sw_ynr_hiSpnrFilt_wgtOffset) +
        pParamLo->sw_ynr_hiSpnrFilt_wgtOffset;
    pSelect->sw_ynr_hiSpnrSigma_minLimit =
        ratio * (pParamHi->sw_ynr_hiSpnrSigma_minLimit - pParamLo->sw_ynr_hiSpnrSigma_minLimit) +
        pParamLo->sw_ynr_hiSpnrSigma_minLimit;
    pSelect->sw_ynr_hiSpnrFilt_wgt =
        ratio * (pParamHi->sw_ynr_hiSpnrFilt_wgt - pParamLo->sw_ynr_hiSpnrFilt_wgt) +
        pParamLo->sw_ynr_hiSpnrFilt_wgt;
    pSelect->sw_ynr_hiSpnr_gainThred =
        ratio * (pParamHi->sw_ynr_hiSpnr_gainThred - pParamLo->sw_ynr_hiSpnr_gainThred) +
        pParamLo->sw_ynr_hiSpnr_gainThred;
    pSelect->sw_ynr_hiSpnrStrongEdge_scale = ratio * (pParamHi->sw_ynr_hiSpnrStrongEdge_scale -
            pParamLo->sw_ynr_hiSpnrStrongEdge_scale) +
            pParamLo->sw_ynr_hiSpnrStrongEdge_scale;
#if 0
    pSelect->sw_ynr_hiSpnrFilt_coeff0 =
        ratio * (pParamHi->sw_ynr_hiSpnrFilt_coeff0 - pParamLo->sw_ynr_hiSpnrFilt_coeff0) +
        pParamLo->sw_ynr_hiSpnrFilt_coeff0;
    pSelect->sw_ynr_hiSpnrFilt_coeff1 =
        ratio * (pParamHi->sw_ynr_hiSpnrFilt_coeff1 - pParamLo->sw_ynr_hiSpnrFilt_coeff1) +
        pParamLo->sw_ynr_hiSpnrFilt_coeff1;
    pSelect->sw_ynr_hiSpnrFilt_coeff2 =
        ratio * (pParamHi->sw_ynr_hiSpnrFilt_coeff2 - pParamLo->sw_ynr_hiSpnrFilt_coeff2) +
        pParamLo->sw_ynr_hiSpnrFilt_coeff2;
    pSelect->sw_ynr_hiSpnrFilt_coeff3 =
        ratio * (pParamHi->sw_ynr_hiSpnrFilt_coeff3 - pParamLo->sw_ynr_hiSpnrFilt_coeff3) +
        pParamLo->sw_ynr_hiSpnrFilt_coeff3;
    pSelect->sw_ynr_hiSpnrFilt_coeff4 =
        ratio * (pParamHi->sw_ynr_hiSpnrFilt_coeff4 - pParamLo->sw_ynr_hiSpnrFilt_coeff4) +
        pParamLo->sw_ynr_hiSpnrFilt_coeff4;
    pSelect->sw_ynr_hiSpnrFilt_coeff5 =
        ratio * (pParamHi->sw_ynr_hiSpnrFilt_coeff5 - pParamLo->sw_ynr_hiSpnrFilt_coeff5) +
        pParamLo->sw_ynr_hiSpnrFilt_coeff5;
#else
    pSelect->sw_ynr_hiSpnrFilt_coeff0 = pParamTmp->sw_ynr_hiSpnrFilt_coeff0;
    pSelect->sw_ynr_hiSpnrFilt_coeff1 = pParamTmp->sw_ynr_hiSpnrFilt_coeff1;
    pSelect->sw_ynr_hiSpnrFilt_coeff2 = pParamTmp->sw_ynr_hiSpnrFilt_coeff2;
    pSelect->sw_ynr_hiSpnrFilt_coeff3 = pParamTmp->sw_ynr_hiSpnrFilt_coeff3;
    pSelect->sw_ynr_hiSpnrFilt_coeff4 = pParamTmp->sw_ynr_hiSpnrFilt_coeff4;
    pSelect->sw_ynr_hiSpnrFilt_coeff5 = pParamTmp->sw_ynr_hiSpnrFilt_coeff5;
#endif

    pSelect->sw_ynr_dsImg_edgeScale =
        ratio * (pParamHi->sw_ynr_dsImg_edgeScale - pParamLo->sw_ynr_dsImg_edgeScale) +
        pParamLo->sw_ynr_dsImg_edgeScale;
    pSelect->sw_ynr_dsFiltSoftThred_scale =
        ratio * (pParamHi->sw_ynr_dsFiltSoftThred_scale - pParamLo->sw_ynr_dsFiltSoftThred_scale) +
        pParamLo->sw_ynr_dsFiltSoftThred_scale;
    pSelect->sw_ynr_dsFiltWgtThred_scale =
        ratio * (pParamHi->sw_ynr_dsFiltWgtThred_scale - pParamLo->sw_ynr_dsFiltWgtThred_scale) +
        pParamLo->sw_ynr_dsFiltWgtThred_scale;
    pSelect->sw_ynr_dsFilt_centerWgt =
        ratio * (pParamHi->sw_ynr_dsFilt_centerWgt - pParamLo->sw_ynr_dsFilt_centerWgt) +
        pParamLo->sw_ynr_dsFilt_centerWgt;
    pSelect->sw_ynr_dsFilt_strg =
        ratio * (pParamHi->sw_ynr_dsFilt_strg - pParamLo->sw_ynr_dsFilt_strg) +
        pParamLo->sw_ynr_dsFilt_strg;
    pSelect->sw_ynr_dsIIRinitWgt_scale =
        ratio * (pParamHi->sw_ynr_dsIIRinitWgt_scale - pParamLo->sw_ynr_dsIIRinitWgt_scale) +
        pParamLo->sw_ynr_dsIIRinitWgt_scale;
    pSelect->sw_ynr_dsFiltLocalGain_alpha =
        ratio * (pParamHi->sw_ynr_dsFiltLocalGain_alpha - pParamLo->sw_ynr_dsFiltLocalGain_alpha) +
        pParamLo->sw_ynr_dsFiltLocalGain_alpha;

    pSelect->sw_ynr_preFilt_strg =
        ratio * (pParamHi->sw_ynr_preFilt_strg - pParamLo->sw_ynr_preFilt_strg) +
        pParamLo->sw_ynr_preFilt_strg;
    pSelect->sw_ynr_loSpnr_wgt =
        ratio * (pParamHi->sw_ynr_loSpnr_wgt - pParamLo->sw_ynr_loSpnr_wgt) +
        pParamLo->sw_ynr_loSpnr_wgt;
    pSelect->sw_ynr_loSpnr_centerWgt =
        ratio * (pParamHi->sw_ynr_loSpnr_centerWgt - pParamLo->sw_ynr_loSpnr_centerWgt) +
        pParamLo->sw_ynr_loSpnr_centerWgt;
    pSelect->sw_ynr_loSpnr_strg =
        ratio * (pParamHi->sw_ynr_loSpnr_strg - pParamLo->sw_ynr_loSpnr_strg) +
        pParamLo->sw_ynr_loSpnr_strg;
    pSelect->sw_ynr_loSpnrDistVstrg_scale =
        ratio * (pParamHi->sw_ynr_loSpnrDistVstrg_scale - pParamLo->sw_ynr_loSpnrDistVstrg_scale) +
        pParamLo->sw_ynr_loSpnrDistVstrg_scale;
    pSelect->sw_ynr_loSpnrDistHstrg_scale =
        ratio * (pParamHi->sw_ynr_loSpnrDistHstrg_scale - pParamLo->sw_ynr_loSpnrDistHstrg_scale) +
        pParamLo->sw_ynr_loSpnrDistHstrg_scale;

    // get rnr parameters
    for (int i = 0; i < 17; i++) {
        pSelect->sw_ynr_radius2strg_val[i] =
            ratio * (pParamHi->sw_ynr_radius2strg_val[i] - pParamLo->sw_ynr_radius2strg_val[i]) +
            pParamLo->sw_ynr_radius2strg_val[i];
    }

    for (int i = 0; i < 9; i++) {
        pSelect->sw_ynr_loSpnrGain2Strg_val[i] = ratio * (pParamHi->sw_ynr_loSpnrGain2Strg_val[i] -
                pParamLo->sw_ynr_loSpnrGain2Strg_val[i]) +
                pParamLo->sw_ynr_loSpnrGain2Strg_val[i];
    }

    // noise curve
    for (int i = 0; i < YNR_V24_ISO_CURVE_POINT_NUM; i++) {
        pSelect->sigma[i] = ratio * (pParamHi->sigma[i] - pParamLo->sigma[i]) + pParamLo->sigma[i];
        pSelect->lumaPoint[i] = (short)(ratio * (pParamHi->lumaPoint[i] - pParamLo->lumaPoint[i]) +
                                        pParamLo->lumaPoint[i]);
    }

    for (int i = 0; i < 6; i++) {
        pSelect->sw_ynr_luma2loStrg_lumaPoint[i] =
            ratio * (pParamHi->sw_ynr_luma2loStrg_lumaPoint[i] -
                     pParamLo->sw_ynr_luma2loStrg_lumaPoint[i]) +
            pParamLo->sw_ynr_luma2loStrg_lumaPoint[i];
        pSelect->sw_ynr_luma2loStrg_val[i] =
            ratio * (pParamHi->sw_ynr_luma2loStrg_val[i] - pParamLo->sw_ynr_luma2loStrg_val[i]) +
            pParamLo->sw_ynr_luma2loStrg_val[i];

        pSelect->sw_ynr_luma2loStrg_lumaPoint[i] *= 4;  // curve point 8 bits -> 10 bits
    }

    return res;
}

Aynr_result_V24_t ynr_fix_transfer_V24(RK_YNR_Params_V24_Select_t* pSelect, RK_YNR_Fix_V24_t* pFix,
                                       rk_aiq_ynr_strength_v24_t* pStrength,
                                       Aynr_ExpInfo_V24_t* pExpInfo) {
    LOG1_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    Aynr_result_V24_t res = AYNRV24_RET_SUCCESS;
    int tmp;

    if (pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV24_RET_NULL_POINTER;
    }

    if (pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV24_RET_NULL_POINTER;
    }

    if (pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV24_RET_NULL_POINTER;
    }

    if (pStrength == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV24_RET_NULL_POINTER;
    }

    float fStrength = 1.0;

    if (pStrength->strength_enable) {
        fStrength = pStrength->percent;
    }
    if (fStrength <= 0.0) {
        fStrength = 0.000001;
    }

    float fLoStrength = 1.0;
    if (fStrength < 1.0) {
        fLoStrength = fStrength;
    }
    LOGD_ANR("strength_enable:%d fStrength: %f \n", pStrength->strength_enable, fStrength);

    LOGD_ANR("%s:%d strength:%f raw:width:%d height:%d\n", __FUNCTION__, __LINE__, fStrength,
             pExpInfo->rawHeight, pExpInfo->rawWidth);

    // YNR_2700_GLOBAL_CTRL (0x0000)
    pFix->ynr_en = FUNCTION_ENABLE;
    if (pSelect->enable) {
        pFix->hispnr_bypass = pSelect->sw_ynr_hiSpnr_bypass ? FUNCTION_ENABLE : FUNCTION_DISABLE;
        pFix->lospnr_bypass = pSelect->sw_ynr_loSpnr_bypass ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    } else {
        pFix->hispnr_bypass = FUNCTION_ENABLE;
        pFix->lospnr_bypass = FUNCTION_ENABLE;
    }
    pFix->exgain_bypass   = FUNCTION_DISABLE;
    pFix->global_set_gain = ynrClipFloatValueV24(pSelect->sw_ynr_globalSet_gain, 6, 4);
    pFix->gain_merge_alpha =
        LIMIT_VALUE(pSelect->sw_ynr_gainMerge_alpha * 8.0f, BIT_3_MAX + 1, BIT_MIN);
    pFix->rnr_en = FUNCTION_ENABLE;

    // YNR_2700_RNR_MAX_R  (0x0004)
    int rows             = pExpInfo->rawHeight;                  // raw height
    int cols             = pExpInfo->rawWidth;                   // raw  width
    float r_sq_inv       = 16.0f / (cols * cols + rows * rows);  // divide 2
    int* number_ptr      = (int*)(&r_sq_inv);
    int EE               = ((*number_ptr) >> 23) & (0x0ff);
    EE                   = -(EE - 127 + 1);
    int MM               = (*number_ptr) & 0x7fffff;
    float tmp2           = ((MM / float(1 << 23)) + 1) / 2;
    MM                   = int(256 * tmp2 + 0.5);
    tmp                  = (MM << 5) + EE;
    pFix->rnr_max_radius = CLIP(tmp, 0, 0x3fff);
    // local gain scale
    // tmp = ( sqrt(double(50) / pExpInfo->arIso[pExpInfo->hdr_mode])) * (1 << 7);  //old
    tmp                    = (1.0) * (1 << 7);
    pFix->local_gain_scale = CLIP(tmp, 0, 0x80);

    // YNR_2700_CENTRE_COOR (0x0008)
    pFix->rnr_center_coorv = rows / 2;
    pFix->rnr_center_coorh = cols / 2;

    // YNR_2700_LOCAL_GAIN_CTRL (0x000c) register deleted

    // YNR_2700_LOWNR_CTRL0 (0x0010)
    pFix->ds_filt_soft_thred_scale =
        ynrClipFloatValueV24(pSelect->sw_ynr_dsFiltSoftThred_scale, 4, 5);
    pFix->ds_img_edge_scale = ynrClipFloatValueV24(pSelect->sw_ynr_dsImg_edgeScale, 5, 0);
    pFix->ds_filt_wgt_thred_scale =
        ynrClipFloatValueV24(pSelect->sw_ynr_dsFiltWgtThred_scale, 3, 6);

    // YNR_2700_LOWNR_CTRL1 (0x0014)
    pFix->ds_filt_local_gain_alpha =
        LIMIT_VALUE(pSelect->sw_ynr_dsFiltLocalGain_alpha * 16.0f, BIT_4_MAX + 1, BIT_MIN);
    pFix->ds_iir_init_wgt_scale = ynrClipFloatValueV24(pSelect->sw_ynr_dsIIRinitWgt_scale, 0, 6);
    pFix->ds_filt_center_wgt    = ynrClipFloatValueV24(pSelect->sw_ynr_dsFilt_centerWgt, 1, 10);

    // YNR_2700_LOWNR_CTRL2 (0x0018)
    if (pSelect->sw_ynr_dsFilt_strg == 0.0f)
        pFix->ds_filt_inv_strg = 0x3fff;
    else
        pFix->ds_filt_inv_strg = ynrClipFloatValueV24(1.0f / pSelect->sw_ynr_dsFilt_strg, 5, 9);
    pFix->lospnr_wgt = LIMIT_VALUE(pSelect->sw_ynr_loSpnr_wgt * fLoStrength * 1024.0f, BIT_10_MAX + 1, BIT_MIN);

    // YNR_2700_LOWNR_CTRL3 (0x001c)
    pFix->lospnr_center_wgt = ynrClipFloatValueV24(pSelect->sw_ynr_loSpnr_centerWgt, 2, 10);
    pFix->lospnr_strg       = ynrClipFloatValueV24(pSelect->sw_ynr_loSpnr_strg * fLoStrength, 5, 7);

    // YNR_2700_LOWNR_CTRL4 (0x002c)
    pFix->lospnr_dist_vstrg_scale =
        ynrClipFloatValueV24(pSelect->sw_ynr_loSpnrDistVstrg_scale, 3, 6);
    pFix->lospnr_dist_hstrg_scale =
        ynrClipFloatValueV24(pSelect->sw_ynr_loSpnrDistHstrg_scale, 3, 6);

    // YNR_2700_GAUSS_COEFF (0x0030)
    float filter1_sigma   = pSelect->sw_ynr_preFilt_strg;
    float filt1_coeff1    = exp(-1 / (2 * filter1_sigma * filter1_sigma));
    float filt1_coeff0    = filt1_coeff1 * filt1_coeff1;
    float coeff1_sum      = 1 + 4 * filt1_coeff1 + 4 * filt1_coeff0;
    int w2                = int(filt1_coeff0 / coeff1_sum * 128 + 0.5);
    int w1                = int(filt1_coeff1 / coeff1_sum * 128 + 0.5);
    int w0                = 128 - w1 * 4 - w2 * 4;
    pFix->pre_filt_coeff0 = w0;
    pFix->pre_filt_coeff1 = w1;
    pFix->pre_filt_coeff2 = w2;

    // YNR_2700_LOW_GAIN_ADJ (0x0034 ~ 0x003c)
    for (int i = 0; i < 9; i++) {
        pFix->lospnr_gain2strg_val[i] =
            ynrClipFloatValueV24(pSelect->sw_ynr_loSpnrGain2Strg_val[i], 4, 4);
    }

    // YNR_2700_SGM_DX (0x0040 ~ 0x0060)
    // YNR_2700_luma2sima_val (0x0070 ~ 0x0060)
    // get noise curve
    for (int i = 0; i < YNR_V24_ISO_CURVE_POINT_NUM; i++) {
        tmp                    = pSelect->lumaPoint[i];
        pFix->luma2sima_idx[i] = CLIP(tmp, 0, 0x400);
        tmp                    = (int)(pSelect->sigma[i] * pSelect->ynr_lci  * (1 << YNR_V24_NOISE_SIGMA_FIX_BIT));
        pFix->luma2sima_val[i] = CLIP(tmp, 0, 0xfff);
    }

    float sw_ynr_luma2loStrg_lumaPoint[6];
    float sw_ynr_luma2loStrg_val[6];
    for (int i = 0; i < 6; i++) {
        sw_ynr_luma2loStrg_lumaPoint[i] = pSelect->sw_ynr_luma2loStrg_lumaPoint[i];
        sw_ynr_luma2loStrg_val[i]       = pSelect->sw_ynr_luma2loStrg_val[i];
    }

    // update lo noise curve;
    for (int i = 0; i < YNR_V22_ISO_CURVE_POINT_NUM; i++) {
        float rate;
        int j = 0;
        for (j = 0; j < 6; j++) {
            if (pFix->luma2sima_idx[i] <= sw_ynr_luma2loStrg_lumaPoint[j]) break;
        }

        if (j <= 0)
            rate = sw_ynr_luma2loStrg_val[0];
        else if (j >= 6)
            rate = sw_ynr_luma2loStrg_val[5];
        else {
            rate = ((float)pFix->luma2sima_idx[i] - sw_ynr_luma2loStrg_lumaPoint[j - 1]) /
                   (sw_ynr_luma2loStrg_lumaPoint[j] - sw_ynr_luma2loStrg_lumaPoint[j - 1]);
            rate = sw_ynr_luma2loStrg_val[j - 1] +
                   rate * (sw_ynr_luma2loStrg_val[j] - sw_ynr_luma2loStrg_val[j - 1]);
        }
        tmp                    = (int)(rate * pFix->luma2sima_val[i]);
        pFix->luma2sima_val[i] = CLIP(tmp, 0, 0xfff);
    }

    // YNR_2700_RNR_STRENGTH03 (0x00d0- 0x00e0)
    for (int i = 0; i < 17; i++) {
        tmp                      = int(pSelect->sw_ynr_radius2strg_val[i] * 16);
        pFix->radius2strg_val[i] = CLIP(tmp, 0, 0xff);
    }

    // YNR_2700_NLM_STRONG_EDGE (0x00ec)
    pFix->hispnr_strong_edge = ynrClipFloatValueV24(pSelect->sw_ynr_hiSpnrStrongEdge_scale, 5, 3);

    // YNR_2700_NLM_SIGMA_GAIN (0x00f0)
    pFix->hispnr_sigma_min_limit =
        ynrClipFloatValueV24(pSelect->sw_ynr_hiSpnrSigma_minLimit, 0, 11);
    pFix->hispnr_local_gain_alpha =
        LIMIT_VALUE(pSelect->sw_ynr_hiSpnrLocalGain_alpha * 16.0f, BIT_4_MAX + 1, BIT_MIN);
    pFix->hispnr_strg = ynrClipFloatValueV24((pSelect->sw_ynr_hiSpnr_strg * fStrength), 4, 6);

    // YNR_2700_NLM_COE (0x00f4)
    pFix->hispnr_filt_coeff[0] = ynrClipFloatValueV24(pSelect->sw_ynr_hiSpnrFilt_coeff0, 4, 0);
    pFix->hispnr_filt_coeff[1] = ynrClipFloatValueV24(pSelect->sw_ynr_hiSpnrFilt_coeff1, 4, 0);
    pFix->hispnr_filt_coeff[2] = ynrClipFloatValueV24(pSelect->sw_ynr_hiSpnrFilt_coeff2, 4, 0);
    pFix->hispnr_filt_coeff[3] = ynrClipFloatValueV24(pSelect->sw_ynr_hiSpnrFilt_coeff3, 4, 0);
    pFix->hispnr_filt_coeff[4] = ynrClipFloatValueV24(pSelect->sw_ynr_hiSpnrFilt_coeff4, 4, 0);
    pFix->hispnr_filt_coeff[5] = ynrClipFloatValueV24(pSelect->sw_ynr_hiSpnrFilt_coeff5, 4, 0);

    // YNR_2700_NLM_WEIGHT (0x00f8)
    pFix->hispnr_filt_wgt_offset =
        ynrClipFloatValueV24(pSelect->sw_ynr_hiSpnrFilt_wgtOffset, 0, 10);
    pFix->hispnr_filt_center_wgt =
        LIMIT_VALUE(pSelect->sw_ynr_hiSpnrFilt_centerWgt / fStrength * 1024.0f, BIT_17_MAX + 1, BIT_MIN);

    // YNR_2700_NLM_NR_WEIGHT (0x00fc)
    pFix->hispnr_filt_wgt =
        LIMIT_VALUE(pSelect->sw_ynr_hiSpnrFilt_wgt * fStrength * 1024.0f, BIT_10_MAX + 1, BIT_MIN);
    pFix->hispnr_gain_thred = LIMIT_VALUE(pSelect->sw_ynr_hiSpnr_gainThred * 16.0, BIT_10_MAX, 1);

    ynr_fix_printf_V24(pFix, pSelect);
    return res;
}

Aynr_result_V24_t ynr_fix_printf_V24(RK_YNR_Fix_V24_t* pFix, RK_YNR_Params_V24_Select_t* pSelect) {
    LOG1_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    Aynr_result_V24_t res = AYNRV24_RET_SUCCESS;

    if (pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV24_RET_NULL_POINTER;
    }

    LOGD_ANR("%s(%d) ynr sigma(0~8): %f %f %f %f %f %f\n", __func__, __LINE__, pSelect->sigma[0],
             pSelect->sigma[1], pSelect->sigma[2], pSelect->sigma[3], pSelect->sigma[4],
             pSelect->sigma[5], pSelect->sigma[6], pSelect->sigma[7], pSelect->sigma[8]);

    // YNR_2700_GLOBAL_CTRL (0x0000)
    LOGD_ANR(
        "(0x0000) ynr_en:%d sw_ynr_loSpnr_bypass:% sw_ynr_hiSpnr_bypass:%d exgain_bypass:%d "
        "sw_ynr_gainMerge_alpha:0x%x sw_ynr_globalSet_gain:0x%x \n",
        pFix->ynr_en, pFix->lospnr_bypass, pFix->hispnr_bypass, pFix->exgain_bypass,
        pFix->gain_merge_alpha, pFix->global_set_gain);

    // YNR_2700_RNR_MAX_R  (0x0004)
    LOGD_ANR("(0x0004) ynr_rnr_max_r:0x%x  ynr_local_gainscale:0x%x\n", pFix->rnr_max_radius,
             pFix->local_gain_scale);

    // YNR_2700_RNR_MAX_R  (0x0008)
    LOGD_ANR("(0x0008) ynr_rnr_center_coorv:0x%x  ynr_rnr_center_coorh:0x%x\n",
             pFix->rnr_center_coorv, pFix->rnr_center_coorh);

    // YNR_2700_LOWNR_CTRL0 (0x0010)
    LOGD_ANR(
        "(0x0010) sw_ynr_dsFiltSoftThred_scale:0x%x  sw_ynr_dsImg_edgeScale:0x%x   "
        "sw_ynr_dsFiltWgtThred_scale:0x%x\n",
        pFix->ds_filt_soft_thred_scale, pFix->ds_img_edge_scale, pFix->ds_filt_wgt_thred_scale);

    // YNR_2700_LOWNR_CTRL1 (0x0014)
    LOGD_ANR(
        "(0x0014) sw_ynr_dsFiltLocalGain_alpha:0x%x  sw_ynr_dsIIRinitWgt_scale:0x%x   "
        "sw_ynr_dsFilt_centerWgt:0x%x\n",
        pFix->ds_filt_local_gain_alpha, pFix->ds_iir_init_wgt_scale, pFix->ds_filt_center_wgt);

    // YNR_2700_LOWNR_CTRL2 (0x0018)
    LOGD_ANR("(0x0018) sw_ynr_dsFilt_strg:0x%x  sw_ynr_loSpnr_wgt:0x%x\n", pFix->ds_filt_inv_strg,
             pFix->lospnr_wgt);

    // YNR_2700_LOWNR_CTRL3 (0x001c)
    LOGD_ANR("(0x001c) sw_ynr_loSpnr_centerWgt:0x%x  sw_ynr_loSpnr_strg:0x%x\n",
             pFix->lospnr_center_wgt, pFix->lospnr_strg);

    // YNR_2700_LOWNR_CTRL4 (0x002c)
    LOGD_ANR("(0x002c) sw_ynr_loSpnrDistVstrg_scale:0x%x  sw_ynr_loSpnrDistHstrg_scale:0x%x\n",
             pFix->lospnr_dist_vstrg_scale, pFix->lospnr_dist_hstrg_scale);

    // YNR_2700_GAUSS_COEFF (0x0030)
    LOGD_ANR("(0x0030) pre_filt_coeff0:0x%x  pre_filt_coeff1:0x%x   pre_filt_coeff2:0x%x\n",
             pFix->pre_filt_coeff0, pFix->pre_filt_coeff1, pFix->pre_filt_coeff2);

    // YNR_2700_LOW_GAIN_ADJ (0x0034 ~ 0x003c)
    LOGD_ANR(
        "(0x0034- 0x003c) sw_ynr_loSpnrGain2Strg_val[0~8]:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x "
        "0x%x\n",
        pFix->lospnr_gain2strg_val[0], pFix->lospnr_gain2strg_val[1], pFix->lospnr_gain2strg_val[2],
        pFix->lospnr_gain2strg_val[3], pFix->lospnr_gain2strg_val[4], pFix->lospnr_gain2strg_val[5],
        pFix->lospnr_gain2strg_val[6], pFix->lospnr_gain2strg_val[7],
        pFix->lospnr_gain2strg_val[8]);

    // YNR_2700_SGM_DX_0_1 (0x0040 - 0x0060)
    LOGD_ANR(
        "(0x0040- 0x0060) ynr_luma2sima_idx[0~16]:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x "
        "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
        pFix->luma2sima_idx[0], pFix->luma2sima_idx[1], pFix->luma2sima_idx[2],
        pFix->luma2sima_idx[3], pFix->luma2sima_idx[4], pFix->luma2sima_idx[5],
        pFix->luma2sima_idx[6], pFix->luma2sima_idx[7], pFix->luma2sima_idx[8],
        pFix->luma2sima_idx[9], pFix->luma2sima_idx[10], pFix->luma2sima_idx[11],
        pFix->luma2sima_idx[12], pFix->luma2sima_idx[13], pFix->luma2sima_idx[14],
        pFix->luma2sima_idx[15], pFix->luma2sima_idx[16]);

    // YNR_2700_LSGM_Y_0_1 (0x0070- 0x0090)
    LOGD_ANR(
        "0x0070- 0x0090) ynr_luma2sima_val[0~16]:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x "
        "0x%x "
        "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
        pFix->luma2sima_val[0], pFix->luma2sima_val[1], pFix->luma2sima_val[2],
        pFix->luma2sima_val[3], pFix->luma2sima_val[4], pFix->luma2sima_val[5],
        pFix->luma2sima_val[6], pFix->luma2sima_val[7], pFix->luma2sima_val[8],
        pFix->luma2sima_val[9], pFix->luma2sima_val[10], pFix->luma2sima_val[11],
        pFix->luma2sima_val[12], pFix->luma2sima_val[13], pFix->luma2sima_val[14],
        pFix->luma2sima_val[15], pFix->luma2sima_val[16]);

    // YNR_2700_RNR_STRENGTH03 (0x00d0- 0x00e0)
    LOGD_ANR(
        "(0x00d0- 0x00e0) ynr_sw_ynr_radius2strg_val[0~16]:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x "
        "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
        pFix->radius2strg_val[0], pFix->radius2strg_val[1], pFix->radius2strg_val[2],
        pFix->radius2strg_val[3], pFix->radius2strg_val[4], pFix->radius2strg_val[5],
        pFix->radius2strg_val[6], pFix->radius2strg_val[7], pFix->radius2strg_val[8],
        pFix->radius2strg_val[9], pFix->radius2strg_val[10], pFix->radius2strg_val[11],
        pFix->radius2strg_val[12], pFix->radius2strg_val[13], pFix->radius2strg_val[14],
        pFix->radius2strg_val[15], pFix->radius2strg_val[16]);

    // YNR_NLM_SIGMA_GAIN (0x00f0)
    LOGD_ANR(
        "(0x00f0) ynr_hispnr_sigma_min_limit:0x%x ynr_hispnr_local_gain_alpha:0x%x  "
        "ynr_hispnr_strg:0x%x \n",
        pFix->hispnr_sigma_min_limit, pFix->hispnr_local_gain_alpha, pFix->hispnr_strg);

    // YNR_NLM_COE  (0x00f4)
    LOGD_ANR("(0x00f4) ynr_hispnr_filt_coeff:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
             pFix->hispnr_filt_coeff[0], pFix->hispnr_filt_coeff[1], pFix->hispnr_filt_coeff[2],
             pFix->hispnr_filt_coeff[3], pFix->hispnr_filt_coeff[4], pFix->hispnr_filt_coeff[5]);

    // YNR_NLM_WEIGHT  (0x00f8)
    LOGD_ANR("(0x00f8) ynr_hispnr_filt_center_wgt:0x%x ynr_hispnr_filt_wgt_offset:0x%x \n",
             pFix->hispnr_filt_center_wgt, pFix->hispnr_filt_wgt_offset);

    // YNR_NLM_NR_WEIGHT  (0x00fc)
    LOGD_ANR("(0x00fc) ynr_hispnr_filt_wgt:0x%x ynr_hispnr_gain_thred:0x%x\n",
             pFix->hispnr_filt_wgt, pFix->hispnr_gain_thred);

    LOG1_ANR("%s:(%d) exit \n", __FUNCTION__, __LINE__);
    return res;
}

Aynr_result_V24_t ynr_get_setting_by_name_json_V24(CalibDbV2_YnrV24_t* pCalibdbV2, char* name,
        int* calib_idx, int* tuning_idx) {
    int i                 = 0;
    Aynr_result_V24_t res = AYNRV24_RET_SUCCESS;

    if (pCalibdbV2 == NULL || name == NULL || calib_idx == NULL || tuning_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV24_RET_NULL_POINTER;
    }

    for (i = 0; i < pCalibdbV2->TuningPara.Setting_len; i++) {
        if (strncmp(name, pCalibdbV2->TuningPara.Setting[i].SNR_Mode,
                    strlen(name) * sizeof(char)) == 0) {
            break;
        }
    }

    if (i < pCalibdbV2->TuningPara.Setting_len) {
        *tuning_idx = i;
    } else {
        *tuning_idx = 0;
    }

    for (i = 0; i < pCalibdbV2->CalibPara.Setting_len; i++) {
        if (strncmp(name, pCalibdbV2->CalibPara.Setting[i].SNR_Mode, strlen(name) * sizeof(char)) ==
                0) {
            break;
        }
    }

    if (i < pCalibdbV2->CalibPara.Setting_len) {
        *calib_idx = i;
    } else {
        *calib_idx = 0;
    }

    LOGD_ANR("%s:%d snr_name:%s  snr_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, *calib_idx, i);

    return res;
}

Aynr_result_V24_t ynr_init_params_json_V24(RK_YNR_Params_V24_t* pYnrParams,
        CalibDbV2_YnrV24_t* pCalibdbV2, int calib_idx,
        int tuning_idx) {
    Aynr_result_V24_t res = AYNRV24_RET_SUCCESS;
    int i                 = 0;
    int j                 = 0;
    short isoCurveSectValue;
    short isoCurveSectValue1;
    float ave1, ave2, ave3, ave4;
    int bit_calib = 12;
    int bit_proc;
    int bit_shift;

    CalibDbV2_YnrV24_T_ISO_t* pISO      = NULL;
    CalibDbV2_YnrV24_C_ISO_t* pCalibISO = NULL;

    LOGD_ANR("%s(%d): enter\n", __FUNCTION__, __LINE__);

    if (pYnrParams == NULL || pCalibdbV2 == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV24_RET_NULL_POINTER;
    }

    bit_proc  = YNR_V24_SIGMA_BITS;  // for V24, YNR_SIGMA_BITS = 10
    bit_shift = bit_calib - bit_proc;

    isoCurveSectValue  = (1 << (bit_calib - YNR_V24_ISO_CURVE_POINT_BIT));
    isoCurveSectValue1 = (1 << bit_calib);

    for (j = 0;
            j < pCalibdbV2->CalibPara.Setting[tuning_idx].Calib_ISO_len && j < RK_YNR_V24_MAX_ISO_NUM;
            j++) {
        pCalibISO          = &pCalibdbV2->CalibPara.Setting[tuning_idx].Calib_ISO[j];
        pYnrParams->iso[j] = pCalibISO->iso;
        pYnrParams->arYnrParamsISO[j].ynr_lci = pCalibISO->ynr_lci;

        // get noise sigma sample data at [0, 64, 128, ... , 1024]
        for (i = 0; i < YNR_V24_ISO_CURVE_POINT_NUM; i++) {
            if (i == (YNR_V24_ISO_CURVE_POINT_NUM - 1)) {
                ave1 = (float)isoCurveSectValue1;
            } else {
                ave1 = (float)(i * isoCurveSectValue);
            }
            pYnrParams->arYnrParamsISO[j].lumaPoint[i] = (short)ave1;
            ave2                                       = ave1 * ave1;
            ave3                                       = ave2 * ave1;
            ave4                                       = ave3 * ave1;
            pYnrParams->arYnrParamsISO[j].sigma[i] = pCalibISO->sw_ynr_luma2sigma_curve[0] * ave4 +
                    pCalibISO->sw_ynr_luma2sigma_curve[1] * ave3 +
                    pCalibISO->sw_ynr_luma2sigma_curve[2] * ave2 +
                    pCalibISO->sw_ynr_luma2sigma_curve[3] * ave1 +
                    pCalibISO->sw_ynr_luma2sigma_curve[4];

            if (pYnrParams->arYnrParamsISO[j].sigma[i] < 0) {
                pYnrParams->arYnrParamsISO[j].sigma[i] = 0;
            }

            if (bit_shift > 0) {
                pYnrParams->arYnrParamsISO[j].lumaPoint[i] >>= bit_shift;
            } else {
                pYnrParams->arYnrParamsISO[j].lumaPoint[i] <<= ABS(bit_shift);
            }
        }
    }

    LOGD_ANR(" iso len:%d calib_max:%d\n",
             pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO_len, RK_YNR_V24_MAX_ISO_NUM);

    for (j = 0; j < pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO_len &&
            j < RK_YNR_V24_MAX_ISO_NUM;
            j++) {
        pISO = &pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO[j];

        pYnrParams->arYnrParamsISO[j].sw_ynr_loSpnr_bypass = pISO->sw_ynr_loSpnr_bypass;
        pYnrParams->arYnrParamsISO[j].sw_ynr_hiSpnr_bypass = pISO->sw_ynr_hiSpnr_bypass;

        // luma param
        for (int k = 0; k < 6; k++) {
            pYnrParams->arYnrParamsISO[j].sw_ynr_luma2loStrg_lumaPoint[k] =
                pISO->lumaPara.sw_ynr_luma2loStrg_lumaPoint[k];
            pYnrParams->arYnrParamsISO[j].sw_ynr_luma2loStrg_val[k] =
                pISO->lumaPara.sw_ynr_luma2loStrg_val[k];
        }

        // low freq
        pYnrParams->arYnrParamsISO[j].sw_ynr_gainMerge_alpha = pISO->sw_ynr_gainMerge_alpha;
        pYnrParams->arYnrParamsISO[j].sw_ynr_globalSet_gain  = pISO->sw_ynr_globalSet_gain;

        pYnrParams->arYnrParamsISO[j].sw_ynr_hiSpnr_strg = pISO->sw_ynr_hiSpnr_strg;
        pYnrParams->arYnrParamsISO[j].sw_ynr_hiSpnrLocalGain_alpha =
            pISO->sw_ynr_hiSpnrLocalGain_alpha;
        pYnrParams->arYnrParamsISO[j].sw_ynr_hiSpnrFilt_centerWgt =
            pISO->sw_ynr_hiSpnrFilt_centerWgt;
        pYnrParams->arYnrParamsISO[j].sw_ynr_hiSpnrFilt_wgtOffset =
            pISO->sw_ynr_hiSpnrFilt_wgtOffset;
        pYnrParams->arYnrParamsISO[j].sw_ynr_hiSpnrSigma_minLimit =
            pISO->sw_ynr_hiSpnrSigma_minLimit;
        pYnrParams->arYnrParamsISO[j].sw_ynr_hiSpnrFilt_wgt   = pISO->sw_ynr_hiSpnrFilt_wgt;
        pYnrParams->arYnrParamsISO[j].sw_ynr_hiSpnr_gainThred = pISO->sw_ynr_hiSpnr_gainThred;
        pYnrParams->arYnrParamsISO[j].sw_ynr_hiSpnrStrongEdge_scale =
            pISO->sw_ynr_hiSpnrStrongEdge_scale;
        pYnrParams->arYnrParamsISO[j].sw_ynr_hiSpnrFilt_coeff0 = pISO->sw_ynr_hiSpnrFilt_coeff0;
        pYnrParams->arYnrParamsISO[j].sw_ynr_hiSpnrFilt_coeff1 = pISO->sw_ynr_hiSpnrFilt_coeff1;
        pYnrParams->arYnrParamsISO[j].sw_ynr_hiSpnrFilt_coeff2 = pISO->sw_ynr_hiSpnrFilt_coeff2;
        pYnrParams->arYnrParamsISO[j].sw_ynr_hiSpnrFilt_coeff3 = pISO->sw_ynr_hiSpnrFilt_coeff3;
        pYnrParams->arYnrParamsISO[j].sw_ynr_hiSpnrFilt_coeff4 = pISO->sw_ynr_hiSpnrFilt_coeff4;
        pYnrParams->arYnrParamsISO[j].sw_ynr_hiSpnrFilt_coeff5 = pISO->sw_ynr_hiSpnrFilt_coeff5;

        pYnrParams->arYnrParamsISO[j].sw_ynr_dsImg_edgeScale = pISO->sw_ynr_dsImg_edgeScale;
        pYnrParams->arYnrParamsISO[j].sw_ynr_dsFiltSoftThred_scale =
            pISO->sw_ynr_dsFiltSoftThred_scale;
        pYnrParams->arYnrParamsISO[j].sw_ynr_dsFiltWgtThred_scale =
            pISO->sw_ynr_dsFiltWgtThred_scale;
        pYnrParams->arYnrParamsISO[j].sw_ynr_dsFilt_centerWgt   = pISO->sw_ynr_dsFilt_centerWgt;
        pYnrParams->arYnrParamsISO[j].sw_ynr_dsFilt_strg        = pISO->sw_ynr_dsFilt_strg;
        pYnrParams->arYnrParamsISO[j].sw_ynr_dsIIRinitWgt_scale = pISO->sw_ynr_dsIIRinitWgt_scale;
        pYnrParams->arYnrParamsISO[j].sw_ynr_dsFiltLocalGain_alpha =
            pISO->sw_ynr_dsFiltLocalGain_alpha;

        pYnrParams->arYnrParamsISO[j].sw_ynr_preFilt_strg     = pISO->sw_ynr_preFilt_strg;
        pYnrParams->arYnrParamsISO[j].sw_ynr_loSpnr_wgt       = pISO->sw_ynr_loSpnr_wgt;
        pYnrParams->arYnrParamsISO[j].sw_ynr_loSpnr_centerWgt = pISO->sw_ynr_loSpnr_centerWgt;
        pYnrParams->arYnrParamsISO[j].sw_ynr_loSpnr_strg      = pISO->sw_ynr_loSpnr_strg;
        pYnrParams->arYnrParamsISO[j].sw_ynr_loSpnrDistVstrg_scale =
            pISO->sw_ynr_loSpnrDistVstrg_scale;
        pYnrParams->arYnrParamsISO[j].sw_ynr_loSpnrDistHstrg_scale =
            pISO->sw_ynr_loSpnrDistHstrg_scale;

        for (int k = 0; k < 17; k++) {
            pYnrParams->arYnrParamsISO[j].sw_ynr_radius2strg_val[k] =
                pISO->sw_ynr_radius2strg_val[k];
        }

        for (int k = 0; k < 9; k++) {
            pYnrParams->arYnrParamsISO[j].sw_ynr_loSpnrGain2Strg_val[k] =
                pISO->sw_ynr_loSpnrGain2Strg_val[k];
        }
    }

    LOGD_ANR("%s(%d): exit\n", __FUNCTION__, __LINE__);

    return res;
}

Aynr_result_V24_t ynr_config_setting_param_json_V24(RK_YNR_Params_V24_t* pParams,
        CalibDbV2_YnrV24_t* pCalibdbV2,
        char* param_mode, char* snr_name) {
    Aynr_result_V24_t res = AYNRV24_RET_SUCCESS;
    int calib_idx         = 0;
    int tuning_idx        = 0;

    if (pParams == NULL || pCalibdbV2 == NULL || param_mode == NULL || snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV24_RET_NULL_POINTER;
    }

    res = ynr_get_setting_by_name_json_V24(pCalibdbV2, snr_name, &calib_idx, &tuning_idx);
    if (res != AYNRV24_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__,
                 __LINE__);
    }

    res             = ynr_init_params_json_V24(pParams, pCalibdbV2, calib_idx, tuning_idx);
    pParams->enable = pCalibdbV2->TuningPara.enable;
    return res;
}

RKAIQ_END_DECLARE
