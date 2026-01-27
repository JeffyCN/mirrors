#include "rk_aiq_isp39_modules.h"

typedef struct Gain_ExpInfo_s {
    int hdr_mode;
    float arTime[3];
    float arAGain[3];
    float arDGain[3];
    int   arIso[3];
    float isp_dgain[3];
    int isoLevelLow;
    int isoLevelHig;
} Gain_ExpInfo_t;

uint32_t gain_float_lim2_int(float In, int bit_deci_dst, int type)
{
    uint8_t *in_u8 = (uint8_t *)&In;
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

#define     GAIN_HDR_MERGE_IN_FIX_BITS_DECI             6
#define     GAIN_HDR_MERGE_IN0_FIX_BITS_INTE            8
#define     GAIN_HDR_MERGE_IN1_FIX_BITS_INTE            8
#define     GAIN_HDR_MERGE_IN2_FIX_BITS_INTE            12

void rk_aiq_gain20_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo)
{
    struct isp3x_gain_cfg* pFix = &isp_params->isp_cfg->others.gain_cfg;
    gain_param_t *gain_param = (gain_param_t *) attr;
    gain_params_dyn_t* pdyn = &gain_param->dyn;

    Gain_ExpInfo_t stExpInfo;
    memset(&stExpInfo, 0x00, sizeof(Gain_ExpInfo_t));
    RKAiqAecExpInfo_t* curExp = cvtinfo->ae_exp;
    stExpInfo.hdr_mode = cvtinfo->frameNum - 1;
    if (cvtinfo->frameNum == 1) {
        if(curExp->LinearExp.exp_real_params.analog_gain < 1.0) {
            stExpInfo.arAGain[0] = 1.0;
            LOGW_ANR("leanr mode again is wrong, use 1.0 instead\n");
        } else {
            stExpInfo.arAGain[0] = curExp->LinearExp.exp_real_params.analog_gain;
        }
        if(curExp->LinearExp.exp_real_params.digital_gain < 1.0) {
            stExpInfo.arDGain[0] = 1.0;
            LOGW_ANR("leanr mode dgain is wrong, use 1.0 instead\n");
        } else {
            stExpInfo.arDGain[0] = curExp->LinearExp.exp_real_params.digital_gain;
        }
        if(curExp->LinearExp.exp_real_params.isp_dgain < 1.0) {
            stExpInfo.isp_dgain[0] = 1.0;
            LOGW_ANR("leanr mode isp_dgain is wrong, use 1.0 instead\n");
        } else {
            stExpInfo.isp_dgain[0] = curExp->LinearExp.exp_real_params.isp_dgain;
        }
        // stExpInfo.arAGain[0] = 64.0;
        stExpInfo.arTime[0] = curExp->LinearExp.exp_real_params.integration_time;
        stExpInfo.arIso[0] = stExpInfo.arAGain[0] * stExpInfo.arDGain[0] * 50 * stExpInfo.isp_dgain[0];
    }
    else {
        for(int i = 0; i < 3; i++) {
            if(curExp->HdrExp[i].exp_real_params.analog_gain < 1.0) {
                stExpInfo.arAGain[i] = 1.0;
                if (i < cvtinfo->frameNum) {
                    LOGW_ANR("hdr mode again is wrong, use 1.0 instead\n");
                }
            } else {
                stExpInfo.arAGain[i] = curExp->HdrExp[i].exp_real_params.analog_gain;
            }
            if(curExp->HdrExp[i].exp_real_params.digital_gain < 1.0) {
                stExpInfo.arDGain[i] = 1.0;
                if (i < cvtinfo->frameNum) {
                    LOGW_ANR("hdr mode dgain is wrong, use 1.0 instead\n");
                }
            } else {
                stExpInfo.arDGain[i] = curExp->HdrExp[i].exp_real_params.digital_gain;
            }
            if(curExp->HdrExp[i].exp_real_params.isp_dgain < 1.0) {
                stExpInfo.isp_dgain[i] = 1.0;
                if (i < cvtinfo->frameNum) {
                    LOGW_ANR("hdr mode isp_dgain is wrong, use 1.0 instead\n");
                }
            } else {
                stExpInfo.isp_dgain[i] = curExp->HdrExp[i].exp_real_params.isp_dgain;
            }
            stExpInfo.arTime[i] = curExp->HdrExp[i].exp_real_params.integration_time;
            stExpInfo.arIso[i] = stExpInfo.arAGain[i] * stExpInfo.arDGain[i] * 50 * stExpInfo.isp_dgain[i];
        }
    }
    float exp_gain[3];
    float dGain[3];
    float frameiso[3];
    float frameEt[3];
    // frame_gain_in
    float frame_exp_val[3];
    float frame_exp_ratio[3];
    int HDR_frame_num = cvtinfo->frameNum;

    // get exposure time and gain information (store as float number)
    for (int i = 0; i < 3; ++i)
    {
        exp_gain[i] = stExpInfo.arAGain[i] * stExpInfo.arDGain[i];
        frameEt[i] = stExpInfo.arTime[i];
        frameiso[i] = stExpInfo.arIso[i];
        if (i >= HDR_frame_num)
        {
            exp_gain[i] = stExpInfo.arAGain[HDR_frame_num - 1] * stExpInfo.arDGain[HDR_frame_num - 1];
            frameEt[i] = stExpInfo.arTime[HDR_frame_num - 1];
            frameiso[i] = stExpInfo.arIso[HDR_frame_num - 1];
        }
        frame_exp_val[i] = frameiso[i] * frameEt[i];

        LOGD_ANR("again: idx:%d gain:%d time:%f HDR_frame_num:%d exp: %f %f %f \n",
                 i, stExpInfo.arIso[i], stExpInfo.arTime[i], HDR_frame_num,
                 frameiso[i], frameEt[i], frame_exp_val[i]);
    }


    // calculate exposure ratio (store as float number)
    for (int i = 0; i < 3; i++)
    {
        frame_exp_ratio[i] = frame_exp_val[HDR_frame_num - 1] / frame_exp_val[i];
    }

    unsigned int sw_gain[3];
    // calculate the fixed gain number {12i, 6f}
    for (int i = 2; i >= 0; i--)
    {
        uint32_t a = (1 << (GAIN_HDR_MERGE_IN2_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1;
        dGain[i] = (frame_exp_ratio[i] * exp_gain[i]) / exp_gain[2];
        if (!CHECK_ISP_HW_V20() && !CHECK_ISP_HW_V21() && !CHECK_ISP_HW_V30() ) {
            if(stExpInfo.hdr_mode == 0 && cvtinfo->blc_res.obcPostTnr.sw_blcT_obcPostTnr_en &&  cvtinfo->preDGain > 1.0) {
                dGain[i] *= cvtinfo->preDGain;
            }
        }
        sw_gain[i] = gain_float_lim2_int(dGain[i], GAIN_HDR_MERGE_IN_FIX_BITS_DECI, 1);       // 12:6


        if(stExpInfo.hdr_mode == 0) {
            if (i == 0)
                sw_gain[i] = MIN(sw_gain[i], (1 << (GAIN_HDR_MERGE_IN2_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
            else if (i == 1)
                sw_gain[i] = MIN(sw_gain[i], (1 << (GAIN_HDR_MERGE_IN1_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
            else
                sw_gain[i] = MIN(sw_gain[i], (1 << (GAIN_HDR_MERGE_IN0_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
        } else if(stExpInfo.hdr_mode == 1) {
            LOGD_ANR("enter 2fram hdr mode, scale_s:%f\n", pdyn->hdr_gain_scale_s);
            if (i == 0) {
                if(pdyn->hdrgain_ctrl_enable) {
                    sw_gain[i] = MIN(sw_gain[i] * pdyn->hdr_gain_scale_s, (1 << (GAIN_HDR_MERGE_IN2_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
                } else {
                    sw_gain[i] = MIN(sw_gain[i], (1 << (GAIN_HDR_MERGE_IN2_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
                }
            } else if (i == 1)
                sw_gain[i] = MIN(sw_gain[i], (1 << (GAIN_HDR_MERGE_IN1_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
            else
                sw_gain[i] = MIN(sw_gain[i], (1 << (GAIN_HDR_MERGE_IN0_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
        } else if(stExpInfo.hdr_mode == 2) {
            if (i == 0) {
                if(pdyn->hdrgain_ctrl_enable) {
                    sw_gain[i] = MIN(sw_gain[i] * pdyn->hdr_gain_scale_s, (1 << (GAIN_HDR_MERGE_IN2_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
                } else {
                    sw_gain[i] = MIN(sw_gain[i], (1 << (GAIN_HDR_MERGE_IN2_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
                }
            } else if (i == 1) {
                if(pdyn->hdrgain_ctrl_enable) {
                    sw_gain[i] = MIN(sw_gain[i] * pdyn->hdr_gain_scale_m, (1 << (GAIN_HDR_MERGE_IN1_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
                } else {
                    sw_gain[i] = MIN(sw_gain[i], (1 << (GAIN_HDR_MERGE_IN1_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
                }
            } else
                sw_gain[i] = MIN(sw_gain[i], (1 << (GAIN_HDR_MERGE_IN0_FIX_BITS_INTE + GAIN_HDR_MERGE_IN_FIX_BITS_DECI)) - 1);
        }
    }
    pFix->g0 = sw_gain[0];
    pFix->g1 = sw_gain[1];
    pFix->g2 = sw_gain[2];

    return;
}
