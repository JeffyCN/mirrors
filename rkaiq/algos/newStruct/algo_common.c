#include "algo_common.h"
#include <stdlib.h>

void get_illu_estm_info(
        illu_estm_info_t *info,
        RkAiqAlgoProcResAwbShared_t *awbRes,
        RKAiqAecExpInfo_t *aeRes,
        int working_mode
) {
    info->awbGain[0] = 1.0;
    info->awbGain[1] = 1.0;
    info->awbIIRDampCoef = 0.0;
    info->varianceLuma   = 0.0;
    info->awbConverged   = 1;
    //swinfo->grayMode = inparams->u.proc.gray_mode;

    if (awbRes) {
        if (awbRes->awb_gain_algo.grgain < DIVMIN || awbRes->awb_gain_algo.gbgain < DIVMIN) {
            LOGW("get wrong awb gain from AWB module ,use default value ");
        } else {
            info->awbGain[0] =
                awbRes->awb_gain_algo.rgain / awbRes->awb_gain_algo.grgain;

            info->awbGain[1] =
                awbRes->awb_gain_algo.bgain / awbRes->awb_gain_algo.gbgain;
        }
        info->awbIIRDampCoef = awbRes->awb_smooth_factor;
        info->varianceLuma   = awbRes->varianceLuma;
        info->awbConverged   = awbRes->awbConverged;
    } else {
        LOGW("fail to get awb gain form AWB module,use default value ");
    }

    if (aeRes) {
        if ((rk_aiq_working_mode_t)working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            info->sensorGain =
                aeRes->LinearExp.exp_real_params.analog_gain *
                aeRes->LinearExp.exp_real_params.digital_gain *
                aeRes->LinearExp.exp_real_params.isp_dgain;
        } else if ((rk_aiq_working_mode_t)working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2 &&
                   (rk_aiq_working_mode_t)working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3) {
            LOGD("sensor gain choose from second hdr frame for acolor");
            info->sensorGain =
                aeRes->HdrExp[1].exp_real_params.analog_gain *
                aeRes->HdrExp[1].exp_real_params.digital_gain *
                aeRes->HdrExp[1].exp_real_params.isp_dgain;
        } else if ((rk_aiq_working_mode_t)working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2 &&
                   (rk_aiq_working_mode_t)working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR3) {
            LOGD("sensor gain choose from third hdr frame for acolor");
            info->sensorGain =
                aeRes->HdrExp[2].exp_real_params.analog_gain *
                aeRes->HdrExp[2].exp_real_params.digital_gain *
                aeRes->HdrExp[2].exp_real_params.isp_dgain;
        } else {
            LOGW(
                "working_mode (%d) is invaild ,fail to get sensor gain form AE module,use default "
                "value ", working_mode);
        }
    } else {
        LOGW("fail to get sensor gain form AE module,use default value ");
    }
}

float algo_strength_to_percent(float fStrength) {
    float fPercent = 0.0;
    float fslope = 4.0;
    if (fStrength <= 0.5) {
        fPercent = fStrength / 0.5;
    } else {
        if (fStrength >= 0.999999) fStrength = 0.999999;
        fPercent = 0.5 * fslope / (1.0 - fStrength) - fslope + 1;
    }
    return fPercent;
}
