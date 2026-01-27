#include "rk_aiq_again_algo_gain.h"



typedef double              FLOAT_GAIN_V1;

uint32_t FLOAT_LIM2_INT_V1(float In, int bit_deci_dst, int type = 0)
{
    // would trigger strict-aliasing compile warning on -Os level
    int exp_val = (((uint32_t*)(&In))[0] >> 23) & 0xff;
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
T FLOAT_FIX_V1(T In, int bit_inte_dst, int bit_deci_dst)
{
    uint32_t tmp;
    T out;

    tmp = ROUND_F(In * (1 << bit_deci_dst));
    out = ((FLOAT_GAIN_V1)tmp) / (1 << bit_deci_dst);
    return out;

}


int gain_find_data_bits_v1(int data)
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

Again_result_t gain_fix_transfer_v1(RK_GAIN_Params_V1_Select_t *pSelect, RK_GAIN_Fix_V1_t* pGainFix,  Again_ExpInfo_t *pExpInfo, float gain_ratio)
{
    int i;
    double max_val = 0;
    uint16_t sigma_bits_max;
    uint16_t sigma_bits_act = GAIN_SIGMA_BITS_ACT_V1;
    double noise_sigma_dehaze[GAIN_MAX_INTEPORATATION_LUMAPOINT];


    LOGI_ANR("%s:(%d) enter\n", __FUNCTION__, __LINE__);
	
    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAIN_RET_NULL_POINTER;
    }

    if(pGainFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAIN_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAIN_RET_NULL_POINTER;
    }

    //pGainFix->gain_table_en = 1;
    memcpy(noise_sigma_dehaze, pSelect->noise_sigma_dehaze, sizeof(pSelect->noise_sigma_dehaze));

    for(i = 0; i < GAIN_MAX_INTEPORATATION_LUMAPOINT - 2; i++) {
        pGainFix->idx[i] = pSelect->fix_x_pos_dehaze[i + 1];
        if(pGainFix->idx[i] > 255) {
            pGainFix->idx[i] = 255;
        }
        LOGD_ANR("%s:%d sigma x: %d\n", __FUNCTION__, __LINE__, pGainFix->idx[i]);
    }

    for(i = 0; i < GAIN_MAX_INTEPORATATION_LUMAPOINT; i++) {
        if(max_val < pSelect->noise_sigma_dehaze[i])
            max_val = pSelect->noise_sigma_dehaze[i];
    }
    sigma_bits_max = gain_find_data_bits_v1((int)max_val);//CEIL(log((FLOAT_GAIN_V1)max_val)  / log((float)2));

    for(i = 0; i < GAIN_MAX_INTEPORATATION_LUMAPOINT; i++) {
        noise_sigma_dehaze[i] = FLOAT_FIX_V1(pSelect->noise_sigma_dehaze[i], sigma_bits_max, sigma_bits_act - sigma_bits_max);
    }

    for(int i = 0; i < GAIN_MAX_INTEPORATATION_LUMAPOINT; i++)
    {
        pGainFix->lut[i] = FLOAT_LIM2_INT_V1((float)noise_sigma_dehaze[i], sigma_bits_act - sigma_bits_max);
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
            pGainFix->mge_gain[i]   = FLOAT_LIM2_INT_V1(dGain[i] * gain_ratio, GAIN_HDR_MERGE_IN_FIX_BITS_DECI_V1, 1);       // 12:6
            if(i == 0)
                pGainFix->mge_gain[i] = MIN(pGainFix->mge_gain[i], (1 << (GAIN_HDR_MERGE_IN2_FIX_BITS_INTE_V1 + GAIN_HDR_MERGE_IN_FIX_BITS_DECI_V1)) - 1);
            else
                pGainFix->mge_gain[i] = MIN(pGainFix->mge_gain[i], (1 << (GAIN_HDR_MERGE_IN0_FIX_BITS_INTE_V1 + GAIN_HDR_MERGE_IN_FIX_BITS_DECI_V1)) - 1);
        }
    }
    else
    {
        pGainFix->mge_gain[0]   = FLOAT_LIM2_INT_V1(pExpInfo->arAGain[0] * pExpInfo->arDGain[0] * gain_ratio, GAIN_HDR_MERGE_IN_FIX_BITS_DECI_V1, 1);        // 12:6
        pGainFix->mge_gain[1]   = FLOAT_LIM2_INT_V1(pExpInfo->arAGain[0] * pExpInfo->arDGain[0] * gain_ratio, GAIN_HDR_MERGE_IN_FIX_BITS_DECI_V1, 1);        // 12:6
        pGainFix->mge_gain[2]   = FLOAT_LIM2_INT_V1(pExpInfo->arAGain[0] * pExpInfo->arDGain[0] * gain_ratio, GAIN_HDR_MERGE_IN_FIX_BITS_DECI_V1, 1);        // 12:6
    }

	LOGI_ANR("%s:(%d)  exit\n", __FUNCTION__, __LINE__);
	
    return AGAIN_RET_SUCCESS;

}


Again_result_t gain_fix_Printf_v1(RK_GAIN_Fix_V1_t  * pFix)
{
	LOGI_ANR("%s:(%d)  enter\n", __FUNCTION__, __LINE__);
	
    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAIN_RET_NULL_POINTER;
    }

	LOGD_ANR("0x3f00: gain_table_en:%d dhaz_en:%d wdr_en:%d tmo_en:%d lsc_en:%d mge_en:%d\n",
		pFix->gain_table_en,
		pFix->dhaz_en,
		pFix->wdr_en,
		pFix->tmo_en,
		pFix->lsc_en,
		pFix->mge_en);

	LOGD_ANR("0x3f00: mge_gain: %d %d %d\n",
		pFix->mge_gain[0],
		pFix->mge_gain[1],
		pFix->mge_gain[2]);

	for(int i=0; i<15; i++){
		LOGD_ANR("0x3f00: mge_gain[%d]: %d\n",
		i,
		pFix->idx[i]);
	}

	for(int i=0; i<17; i++){
		LOGD_ANR("0x3f00: mge_gain[%d]: %d\n",
		i,
		pFix->lut[i]);
	}

	LOGI_ANR("%s:(%d)  exit\n", __FUNCTION__, __LINE__);

	return AGAIN_RET_SUCCESS;
	
}






