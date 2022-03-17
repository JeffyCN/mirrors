#include "rk_aiq_again_algo_gain_v2.h"


uint32_t gain_float_lim2_int(float In, int bit_deci_dst, int type)
{
    int exp_val = (((uint32_t*)(&In))[0] >> 23) & 0xff;
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


Again_result_V2_t gain_fix_transfer_v2(RK_GAIN_Params_V2_Select_t *pSelect, RK_GAIN_Fix_V2_t* pGainFix,  Again_ExpInfo_V2_t *pExpInfo, float gain_ratio)
{
    int i;
    double max_val = 0;


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


    float exp_time[3];
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
        if (i == 0)
            pGainFix->sw_gain[i] = MIN(pGainFix->sw_gain[i], (1 << (GAIN_HDR_MERGE_IN2_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
        else if (i == 1)
            pGainFix->sw_gain[i] = MIN(pGainFix->sw_gain[i], (1 << (GAIN_HDR_MERGE_IN1_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
        else
            pGainFix->sw_gain[i] = MIN(pGainFix->sw_gain[i], (1 << (GAIN_HDR_MERGE_IN0_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);

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






