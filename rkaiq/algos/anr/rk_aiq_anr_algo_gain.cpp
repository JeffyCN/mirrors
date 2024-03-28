#include "rk_aiq_anr_algo_gain.h"


typedef double              FLOAT_GAIN;

uint32_t FLOAT_LIM2_INT(float In, int bit_deci_dst, int type = 0)
{
    // would trigger strict-aliasing compile warning on -Os level
    //int exp_val = (((uint32_t*)(&In))[0] >> 23) & 0xff;
    uint8_t *in_u8 = reinterpret_cast<uint8_t *>(&In);
    int exp_val = ((in_u8[3] << 1) & (in_u8[2] >> 7) & 0xff);

    uint32_t dst;
    if(exp_val - 127 <= bit_deci_dst || type == 1)
    {

        dst     = ROUND_F(In * (1 << bit_deci_dst));
    }
    else
    {
        dst     = ROUND_F(In / (1 << bit_deci_dst));
    }
    return dst;
}

template<typename T>
T FLOAT_FIX(T In, int bit_inte_dst, int bit_deci_dst)
{
    uint32_t tmp;
    T out;

    tmp = ROUND_F(In * (1 << bit_deci_dst));
    out = ((FLOAT_GAIN)tmp) / (1 << bit_deci_dst);
    return out;

}


int gain_find_data_bits(int data)
{
    int i, j = 1;
    int bits = 0;

    for(i = 0; i < 32; i++)
    {
        if(data & j)
        {
            bits = i + 1;
        }
        j = j << 1;
    }

    return bits;
}

ANRresult_t gain_fix_transfer(RKAnr_Mfnr_Params_Select_t *pMfnrSelect, RKAnr_Gain_Fix_t* pGainFix,  ANRExpInfo_t *pExpInfo, float gain_ratio)
{
    int i;
    double max_val = 0;
    uint16_t sigma_bits_max;
    uint16_t sigma_bits_act = GAIN_SIGMA_BITS_ACT;
    double noise_sigma_dehaze[MAX_INTEPORATATION_LUMAPOINT];

    LOGI_ANR("%s:(%d) oyyf bayerner xml config start\n", __FUNCTION__, __LINE__);
    if(pMfnrSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pGainFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    //pGainFix->gain_table_en = 1;
    memcpy(noise_sigma_dehaze, pMfnrSelect->noise_sigma_dehaze, sizeof(pMfnrSelect->noise_sigma_dehaze));

    for(i = 0; i < MAX_INTEPORATATION_LUMAPOINT - 2; i++) {
        pGainFix->idx[i] = pMfnrSelect->fix_x_pos_dehaze[i + 1];
        if(pGainFix->idx[i] > 255) {
            pGainFix->idx[i] = 255;
        }
        LOGD_ANR("%s:%d sigma x: %d\n", __FUNCTION__, __LINE__, pGainFix->idx[i]);
    }

    for(i = 0; i < MAX_INTEPORATATION_LUMAPOINT; i++) {
        if(max_val < pMfnrSelect->noise_sigma_dehaze[i])
            max_val = pMfnrSelect->noise_sigma_dehaze[i];
    }
    sigma_bits_max = gain_find_data_bits((int)max_val);//CEIL(log((FLOAT_GAIN)max_val)  / log((float)2));

    for(i = 0; i < MAX_INTEPORATATION_LUMAPOINT; i++) {
        noise_sigma_dehaze[i] = FLOAT_FIX(pMfnrSelect->noise_sigma_dehaze[i], sigma_bits_max, sigma_bits_act - sigma_bits_max);
    }

    for(int i = 0; i < MAX_INTEPORATATION_LUMAPOINT; i++)
    {
        pGainFix->lut[i] = FLOAT_LIM2_INT((float)noise_sigma_dehaze[i], sigma_bits_act - sigma_bits_max);
    }


#if 1
    pGainFix->dhaz_en = 1;
    pGainFix->wdr_en = 0;
    pGainFix->tmo_en = 1;
    pGainFix->lsc_en = 1;
    pGainFix->mge_en = 1;
#endif


    int hdr_frameNum = pExpInfo->hdr_mode + 1;
    if(hdr_frameNum > 1)
    {
        float frameiso[3];
        float frameEt[3];
        float frame_exp_val[3];
        float frame_exp_ratio[3];
        float dGain[3];

        for (int i = 0; i < hdr_frameNum; i++) {
            frameiso[i] = pExpInfo->arAGain[i];
            frameEt[i]  =  pExpInfo->arTime[i];
            LOGD_ANR("%s:%d idx:%d gain:%f time:%f exp:%f\n",
                     __FUNCTION__, __LINE__, i,
                     pExpInfo->arAGain[i], pExpInfo->arTime[i],
                     pExpInfo->arAGain[i] * pExpInfo->arTime[i]);
        }

        for (int i = 0; i < 3; i++) {
            if(i >= hdr_frameNum) {
                frame_exp_val[i]    = frame_exp_val[hdr_frameNum - 1];
                frameiso[i]         = frameiso[hdr_frameNum - 1];
            } else {
                frame_exp_val[i]    = frameiso[i] * frameEt[i];
                frameiso[i]         = frameiso[i] * 50;
            }
        }

        for (int i = 0; i < 3; i++) {
            frame_exp_ratio[i]  = frame_exp_val[hdr_frameNum - 1] / frame_exp_val[i];
        }

        for (int i = 2; i >= 0; i--) {
            dGain[i] = (frame_exp_ratio[i] * pExpInfo->arAGain[i] * pExpInfo->arDGain[i]);
            LOGD_ANR("%s:%d idx:%d ratio:%f dgain %f\n",
                     __FUNCTION__, __LINE__, i,
                     frame_exp_ratio[i], dGain[i]);
            pGainFix->mge_gain[i]   = FLOAT_LIM2_INT(dGain[i] * gain_ratio, GAIN_HDR_MERGE_IN_FIX_BITS_DECI, 1);       // 12:6
            if(i == 0)
                pGainFix->mge_gain[i] = MIN(pGainFix->mge_gain[i], (1 << (GAIN_HDR_MERGE_IN2_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
            else
                pGainFix->mge_gain[i] = MIN(pGainFix->mge_gain[i], (1 << (GAIN_HDR_MERGE_IN0_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
        }
    }
    else
    {
        pGainFix->mge_gain[0]   = FLOAT_LIM2_INT(pExpInfo->arAGain[0] * pExpInfo->arDGain[0] * gain_ratio, GAIN_HDR_MERGE_IN_FIX_BITS_DECI, 1);        // 12:6
        pGainFix->mge_gain[1]   = FLOAT_LIM2_INT(pExpInfo->arAGain[0] * pExpInfo->arDGain[0] * gain_ratio, GAIN_HDR_MERGE_IN_FIX_BITS_DECI, 1);        // 12:6
        pGainFix->mge_gain[2]   = FLOAT_LIM2_INT(pExpInfo->arAGain[0] * pExpInfo->arDGain[0] * gain_ratio, GAIN_HDR_MERGE_IN_FIX_BITS_DECI, 1);        // 12:6
    }

    return ANR_RET_SUCCESS;

}



