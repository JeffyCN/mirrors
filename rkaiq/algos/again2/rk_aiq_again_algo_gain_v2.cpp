/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "rk_aiq_again_algo_gain_v2.h"


Again_result_V2_t gain_get_setting_by_name_json_V2(CalibDbV2_GainV2_t* pCalibdb, char *name, int *tuning_idx)
{
    int i = 0;
    Again_result_V2_t res = AGAINV2_RET_SUCCESS;

    if(pCalibdb == NULL || name == NULL || tuning_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_INVALID_PARM;
    }

    for(i = 0; i < pCalibdb->TuningPara.Setting_len; i++) {
        if(strncmp(name, pCalibdb->TuningPara.Setting[i].SNR_Mode, strlen(name)*sizeof(char)) == 0) {
            break;
        }
    }

    if(i < pCalibdb->TuningPara.Setting_len) {
        *tuning_idx = i;
    } else {
        *tuning_idx = 0;
    }

    LOGD_ANR("%s:%d snr_name:%s  snr_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, *tuning_idx, i);
    return res;


}


Again_result_V2_t gain_init_params_json_V2(RK_GAIN_Params_V2_t *pParams, CalibDbV2_GainV2_t* pCalibdb,  int tuning_idx)
{
    Again_result_V2_t res = AGAINV2_RET_SUCCESS;
    CalibDbV2_GainV2_T_ISO_t *pTuningIso = NULL;

    LOGI_ANR("%s:(%d) oyyf bayerner xml config start\n", __FUNCTION__, __LINE__);
    if(pParams == NULL || pCalibdb == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_INVALID_PARM;
    }

    pParams->hdrgain_ctrl_enable = pCalibdb->TuningPara.hdrgain_ctrl_enable;

    if(tuning_idx <  pCalibdb->TuningPara.Setting_len) {
        for(int i = 0; i < pCalibdb->TuningPara.Setting[tuning_idx].Tuning_ISO_len && i < RK_GAIN_V2_MAX_ISO_NUM; i++) {
            pTuningIso = &pCalibdb->TuningPara.Setting[tuning_idx].Tuning_ISO[i];
            pParams->iso[i] = pTuningIso->iso;
            pParams->iso_params[i].hdr_gain_scale_s = pTuningIso->hdr_gain_scale_s;
            pParams->iso_params[i].hdr_gain_scale_m = pTuningIso->hdr_gain_scale_m;
            LOGD_ANR("iso[%d]:%d hdr_gain_scale:%f %f\n",
                     i,
                     pParams->iso[i],
                     pParams->iso_params[i].hdr_gain_scale_s,
                     pParams->iso_params[i].hdr_gain_scale_m);
        }
    }

    LOGI_ANR("%s:(%d) oyyf bayerner xml config end!   \n", __FUNCTION__, __LINE__);

    return res;
}


Again_result_V2_t gain_config_setting_param_json_V2(RK_GAIN_Params_V2_t *pParams, CalibDbV2_GainV2_t* pCalibdbV2, char* param_mode, char * snr_name)
{
    Again_result_V2_t res = AGAINV2_RET_SUCCESS;
    int tuning_idx = 0;

    if(pParams == NULL || pCalibdbV2 == NULL
            || param_mode == NULL || snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_INVALID_PARM;
    }

    res = gain_get_setting_by_name_json_V2(pCalibdbV2, snr_name, &tuning_idx);
    if(res != AGAINV2_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);

    }

    res = gain_init_params_json_V2(pParams, pCalibdbV2,  tuning_idx);

    return res;

}

Again_result_V2_t gain_select_params_by_ISO_V2(RK_GAIN_Params_V2_t *pParams, RK_GAIN_Select_V2_t *pSelect, Again_ExpInfo_V2_t *pExpInfo)
{
    Again_result_V2_t res = AGAINV2_RET_SUCCESS;
    int iso = 50;

    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_INVALID_PARM;
    }

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_INVALID_PARM;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];

    LOGD_ANR("%s:%d iso:%d \n", __FUNCTION__, __LINE__, iso);

    int isoGainStd[RK_GAIN_V2_MAX_ISO_NUM];
    int isoGain = MAX(int(iso / 50), 1);
    int isoGainLow = 0;
    int isoGainHig = 0;
    int isoLevelLow = 0;
    int isoLevelHig = 0;
    int i;

#ifndef RK_SIMULATOR_HW
    for(i = 0; i < RK_GAIN_V2_MAX_ISO_NUM; i++) {
        isoGainStd[i] = pParams->iso[i] / 50;
    }
#else
    for(i = 0; i < RK_GAIN_V2_MAX_ISO_NUM; i++) {
        isoGainStd[i] = 1 * (1 << i);
    }
#endif

    for (i = 0; i < RK_GAIN_V2_MAX_ISO_NUM - 1; i++)
    {
        if (isoGain >= isoGainStd[i] && isoGain <= isoGainStd[i + 1])
        {
            isoGainLow = isoGainStd[i];
            isoGainHig = isoGainStd[i + 1];
            isoLevelLow = i;
            isoLevelHig = i + 1;
        }
    }

    if(isoGain > isoGainStd[RK_GAIN_V2_MAX_ISO_NUM - 1]) {
        isoGainLow = isoGainStd[RK_GAIN_V2_MAX_ISO_NUM - 2];
        isoGainHig = isoGainStd[RK_GAIN_V2_MAX_ISO_NUM - 1];
        isoLevelLow = RK_GAIN_V2_MAX_ISO_NUM - 2;
        isoLevelHig = RK_GAIN_V2_MAX_ISO_NUM - 1;
    }

    if(isoGain < isoGainStd[1]) {
        isoGainLow = isoGainStd[0];
        isoGainHig = isoGainStd[1];
        isoLevelLow = 0;
        isoLevelHig = 1;
    }

    pExpInfo->isoLevelLow = isoLevelLow;
    pExpInfo->isoLevelHig = isoLevelHig;
    pSelect->hdrgain_ctrl_enable = pParams->hdrgain_ctrl_enable;

    float ratio = 0;
    ratio = float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) ;

    pSelect->hdr_gain_scale_s = ratio * ( pParams->iso_params[isoLevelHig].hdr_gain_scale_s - pParams->iso_params[isoLevelLow].hdr_gain_scale_s)
                                + pParams->iso_params[isoLevelLow].hdr_gain_scale_s;


    pSelect->hdr_gain_scale_m = ratio * ( pParams->iso_params[isoLevelHig].hdr_gain_scale_m - pParams->iso_params[isoLevelLow].hdr_gain_scale_m)
                                + pParams->iso_params[isoLevelLow].hdr_gain_scale_m;

    LOGD_ANR("%s:%d iso:%d high:%d low:%d hdr_gain_scale:%f %f\n",
             __FUNCTION__, __LINE__,
             isoGain, isoGainHig, isoGainLow,
             pSelect->hdr_gain_scale_s,
             pSelect->hdr_gain_scale_m);
    return res;
}

uint32_t gain_float_lim2_int(float In, int bit_deci_dst, int type)
{
    uint8_t *in_u8 = reinterpret_cast<uint8_t *>(&In);
    int exp_val = ((in_u8[3] << 1) & (in_u8[2] >> 7) & 0xff);
    uint32_t dst;
    int shf_bit;
    if (exp_val - 127 <= bit_deci_dst || type == 1)
    {
        shf_bit = bit_deci_dst - (exp_val - 127);
        dst = ROUND_F(In * (1 << bit_deci_dst));
    }
    else
    {
        shf_bit = (exp_val - 127) - bit_deci_dst;
        dst = ROUND_F(In / (1 << bit_deci_dst));
    }
    return dst;
}

Again_result_V2_t gain_fix_transfer_v2( RK_GAIN_Select_V2_t *pSelect, RK_GAIN_Fix_V2_t* pGainFix,  Again_ExpInfo_V2_t *pExpInfo, float gain_ratio)
{
    LOGI_ANR("%s:(%d) enter\n", __FUNCTION__, __LINE__);

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_NULL_POINTER;
    }

    if(pGainFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_NULL_POINTER;
    }


#if 1
    pGainFix->sw_gain2ddr_mode = 0;
    pGainFix->sw_gain2ddr_wr_en = 0;
    pGainFix->sw_3dlut_gain_en = 1;
    pGainFix->sw_dhaz_gain_en = 1;
    pGainFix->sw_adrc_gain_en = 1;
    pGainFix->sw_lsc_gain_en = 1;
    pGainFix->sw_gain_module_free_mode = 0;
    pGainFix->sw_gain_dmard_mode_en = 0;
    pGainFix->sw_bayer3dnr_gain_en = 1;
    pGainFix->sw_gain_mp_pipe_dis = 0;
    pGainFix->sw_gain_gate_always_on = 0;
    pGainFix->sw_mge_gain_en = 1;
    pGainFix->sw_gain_en = 1;
#endif


    float exp_gain[3];
    float dGain[3];
    float frameiso[3];
    float frameEt[3];
    // frame_gain_in
    float frame_exp_val[3];
    float frame_exp_ratio[3];
    int HDR_frame_num = pExpInfo->hdr_mode + 1;

    // get exposure time and gain information (store as float number)
    for (int i = 0; i < 3; ++i)
    {
        exp_gain[i] = pExpInfo->arAGain[i] * pExpInfo->arDGain[i];
        frameEt[i] = pExpInfo->arTime[i];
        frameiso[i] = pExpInfo->arIso[i];
        if (i >= HDR_frame_num)
        {
            exp_gain[i] = pExpInfo->arAGain[HDR_frame_num - 1] * pExpInfo->arDGain[HDR_frame_num - 1];
            frameEt[i] = pExpInfo->arTime[HDR_frame_num - 1];
            frameiso[i] = pExpInfo->arIso[HDR_frame_num - 1];
        }
        frame_exp_val[i] = frameiso[i] * frameEt[i];

        LOGD_ANR("again: idx:%d gain:%d time:%f HDR_frame_num:%d exp: %f %f %f \n",
                 i, pExpInfo->arIso[i], pExpInfo->arTime[i], HDR_frame_num,
                 frameiso[i], frameEt[i], frame_exp_val[i]);
    }


    // calculate exposure ratio (store as float number)
    for (int i = 0; i < 3; i++)
    {
        frame_exp_ratio[i] = frame_exp_val[HDR_frame_num - 1] / frame_exp_val[i];
    }

    // calculate the fixed gain number {12i, 6f}
    for (int i = 2; i >= 0; i--)
    {
        uint32_t a = (1 << (GAIN_HDR_MERGE_IN2_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1;
        dGain[i] = (frame_exp_ratio[i] * exp_gain[i]) / exp_gain[2];
        pGainFix->sw_gain[i] = gain_float_lim2_int(dGain[i], GAIN_HDR_MERGE_IN_FIX_BITS_DECI, 1);       // 12:6

        if(pExpInfo->hdr_mode == 0) {
            if (i == 0)
                pGainFix->sw_gain[i] = MIN(pGainFix->sw_gain[i], (1 << (GAIN_HDR_MERGE_IN2_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
            else if (i == 1)
                pGainFix->sw_gain[i] = MIN(pGainFix->sw_gain[i], (1 << (GAIN_HDR_MERGE_IN1_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
            else
                pGainFix->sw_gain[i] = MIN(pGainFix->sw_gain[i], (1 << (GAIN_HDR_MERGE_IN0_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
        } else if(pExpInfo->hdr_mode == 1) {
            LOGD_ANR("enter 2fram hdr mode, scale_s:%f\n", pSelect->hdr_gain_scale_s);
            if (i == 0) {
                if(pSelect->hdrgain_ctrl_enable) {
                    pGainFix->sw_gain[i] = MIN(pGainFix->sw_gain[i] * pSelect->hdr_gain_scale_s, (1 << (GAIN_HDR_MERGE_IN2_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
                } else {
                    pGainFix->sw_gain[i] = MIN(pGainFix->sw_gain[i], (1 << (GAIN_HDR_MERGE_IN2_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
                }
            } else if (i == 1)
                pGainFix->sw_gain[i] = MIN(pGainFix->sw_gain[i], (1 << (GAIN_HDR_MERGE_IN1_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
            else
                pGainFix->sw_gain[i] = MIN(pGainFix->sw_gain[i], (1 << (GAIN_HDR_MERGE_IN0_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
        } else if(pExpInfo->hdr_mode == 2) {
            if (i == 0) {
                if(pSelect->hdrgain_ctrl_enable) {
                    pGainFix->sw_gain[i] = MIN(pGainFix->sw_gain[i] * pSelect->hdr_gain_scale_s, (1 << (GAIN_HDR_MERGE_IN2_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
                } else {
                    pGainFix->sw_gain[i] = MIN(pGainFix->sw_gain[i], (1 << (GAIN_HDR_MERGE_IN2_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
                }
            } else if (i == 1) {
                if(pSelect->hdrgain_ctrl_enable) {
                    pGainFix->sw_gain[i] = MIN(pGainFix->sw_gain[i] * pSelect->hdr_gain_scale_m, (1 << (GAIN_HDR_MERGE_IN1_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
                } else {
                    pGainFix->sw_gain[i] = MIN(pGainFix->sw_gain[i], (1 << (GAIN_HDR_MERGE_IN1_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
                }
            } else
                pGainFix->sw_gain[i] = MIN(pGainFix->sw_gain[i], (1 << (GAIN_HDR_MERGE_IN0_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
        }

    }

    gain_fix_Printf_v2(pGainFix);
    LOGI_ANR("%s:(%d)  exit\n", __FUNCTION__, __LINE__);

    return AGAINV2_RET_SUCCESS;

}


Again_result_V2_t gain_fix_Printf_v2(RK_GAIN_Fix_V2_t  * pFix)
{
    LOGI_ANR("%s:(%d)  enter\n", __FUNCTION__, __LINE__);

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_NULL_POINTER;
    }


    LOGD_ANR("0x3f00: sw_gain2ddr_mode:0x%x sw_gain2ddr_wr_en:0x%x sw_3dlut_gain_en:0x%x sw_dhaz_gain_en:0x%x sw_adrc_gain_en:0x%x sw_lsc_gain_en:0x%x\n",
             pFix->sw_gain2ddr_mode,
             pFix->sw_gain2ddr_wr_en,
             pFix->sw_3dlut_gain_en,
             pFix->sw_dhaz_gain_en,
             pFix->sw_adrc_gain_en,
             pFix->sw_lsc_gain_en);

    LOGD_ANR("0x3f00: sw_gain_module_free_mode:0x%x sw_gain_dmard_mode_en:0x%x sw_bayer3dnr_gain_en:0x%x sw_gain_mp_pipe_dis:0x%x sw_gain_gate_always_on:%d sw_mge_gain_en:0x%x sw_gain_en:0x%x\n",
             pFix->sw_gain_module_free_mode,
             pFix->sw_gain_dmard_mode_en,
             pFix->sw_bayer3dnr_gain_en,
             pFix->sw_gain_mp_pipe_dis,
             pFix->sw_gain_gate_always_on,
             pFix->sw_mge_gain_en,
             pFix->sw_gain_en);

    LOGD_ANR("0x3f04: sw_gain: 0x%x 0x%x 0x%x\n",
             pFix->sw_gain[0],
             pFix->sw_gain[1],
             pFix->sw_gain[2]);

    LOGI_ANR("%s:(%d)  exit\n", __FUNCTION__, __LINE__);

    return AGAINV2_RET_SUCCESS;

}






