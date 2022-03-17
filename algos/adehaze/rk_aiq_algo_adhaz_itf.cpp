/*
 * rk_aiq_algo_adhaz_itf.c
 *
 *  Copyright (c) 2019 Rockchip Corporation
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
 *
 */
/* for rockchip v2.0.0*/

#include "rk_aiq_algo_types_int.h"


#include "rk_aiq_algo_adhaz_itf.h"
#include "RkAiqCalibDbTypes.h"
#include "adehaze/rk_aiq_adehaze_algo.h"
#include "RkAiqCalibDbTypes.h"
#include "xcam_log.h"

RKAIQ_BEGIN_DECLARE

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t *AdehazeHandle = NULL;
    AlgoCtxInstanceCfgInt* instanc_int = (AlgoCtxInstanceCfgInt*)cfg;
    CamCalibDbV2Context_t* calibv2 = instanc_int->calibv2;

    ret = AdehazeInit(&AdehazeHandle, calibv2);

    *context = (RkAiqAlgoContext *)(AdehazeHandle);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t *AdehazeHandle = (AdehazeHandle_t *)context;

    ret = AdehazeRelease(AdehazeHandle);

    return ret;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoConfigAdhazInt* config = (RkAiqAlgoConfigAdhazInt*)params;
    AdehazeHandle_t * AdehazeHandle = (AdehazeHandle_t *)params->ctx;
    const CamCalibDbV2Context_t* pCalibDb = config->rk_com.u.prepare.calibv2;

    AdehazeHandle->working_mode = config->adhaz_config_com.com.u.prepare.working_mode;

    if (AdehazeHandle->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        AdehazeHandle->FrameNumber = 1;
    else if (AdehazeHandle->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             AdehazeHandle->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        AdehazeHandle->FrameNumber = 2;
    else
        AdehazeHandle->FrameNumber = 3;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        LOGD_ADEHAZE("%s: Adehaze Reload Para!\n", __FUNCTION__);

        CalibDbV2_dehaze_V20_t* calibv2_adehaze_calib_V20 =
            (CalibDbV2_dehaze_V20_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adehaze_calib_v20));
        if (calibv2_adehaze_calib_V20)
            memcpy(&AdehazeHandle->calib_dehazV20, calibv2_adehaze_calib_V20, sizeof(CalibDbV2_dehaze_V20_t));

        CalibDbV2_dehaze_V21_t* calibv2_adehaze_calib_V21 =
            (CalibDbV2_dehaze_V21_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adehaze_calib_v21));
        if (calibv2_adehaze_calib_V21)
            memcpy(&AdehazeHandle->calib_dehazV21, calibv2_adehaze_calib_V21, sizeof(CalibDbV2_dehaze_V21_t));
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;

}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    RkAiqAlgoPreAdhazInt* config = (RkAiqAlgoPreAdhazInt*)inparams;
    AdehazeHandle_t * AdehazeHandle = (AdehazeHandle_t *)inparams->ctx;

    AdehazeHandle->width = config->rawWidth;
    AdehazeHandle->height = config->rawHeight;

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t * AdehazeHandle = (AdehazeHandle_t *)inparams->ctx;
    RkAiqAlgoProcAdhazInt* procPara = (RkAiqAlgoProcAdhazInt*)inparams;
    RkAiqAlgoProcResAdhaz* procResPara = (RkAiqAlgoProcResAdhaz*)outparams;
    AdehazeGetStats(AdehazeHandle, &procPara->stats);

    if(AdehazeHandle->HWversion == ADEHAZE_ISP20) {
        int iso = 50;
        AdehazeExpInfo_t stExpInfo;
        memset(&stExpInfo, 0x00, sizeof(AdehazeExpInfo_t));

        stExpInfo.hdr_mode = 0;
        for(int i = 0; i < 3; i++) {
            stExpInfo.arIso[i] = 50;
            stExpInfo.arAGain[i] = 1.0;
            stExpInfo.arDGain[i] = 1.0;
            stExpInfo.arTime[i] = 0.01;
        }

        if(AdehazeHandle->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfo.hdr_mode = 0;
        } else if(RK_AIQ_HDR_GET_WORKING_MODE(AdehazeHandle->working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR2) {
            stExpInfo.hdr_mode = 1;
        } else if(RK_AIQ_HDR_GET_WORKING_MODE(AdehazeHandle->working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR3) {
            stExpInfo.hdr_mode = 2;
        }

        RkAiqAlgoPreResAeInt* pAEPreRes =
            (RkAiqAlgoPreResAeInt*)(procPara->rk_com.u.proc.pre_res_comb->ae_pre_res);

        if(pAEPreRes != NULL) {
            if(AdehazeHandle->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                stExpInfo.arAGain[0] = pAEPreRes->ae_pre_res_rk.LinearExp.exp_real_params.analog_gain;
                stExpInfo.arDGain[0] = pAEPreRes->ae_pre_res_rk.LinearExp.exp_real_params.digital_gain;
                stExpInfo.arTime[0] = pAEPreRes->ae_pre_res_rk.LinearExp.exp_real_params.integration_time;
                stExpInfo.arIso[0] = stExpInfo.arAGain[0] * stExpInfo.arDGain[0] * 50;
            } else {
                for(int i = 0; i < 3; i++) {
                    stExpInfo.arAGain[i] = pAEPreRes->ae_pre_res_rk.HdrExp[i].exp_real_params.analog_gain;
                    stExpInfo.arDGain[i] = pAEPreRes->ae_pre_res_rk.HdrExp[i].exp_real_params.digital_gain;
                    stExpInfo.arTime[i] = pAEPreRes->ae_pre_res_rk.HdrExp[i].exp_real_params.integration_time;
                    stExpInfo.arIso[i] = stExpInfo.arAGain[i] * stExpInfo.arDGain[i] * 50;

                    LOGD_ADEHAZE("index:%d again:%f dgain:%f time:%f iso:%d hdr_mode:%d\n",
                                 i,
                                 stExpInfo.arAGain[i],
                                 stExpInfo.arDGain[i],
                                 stExpInfo.arTime[i],
                                 stExpInfo.arIso[i],
                                 stExpInfo.hdr_mode);
                }
            }
        } else {
            LOGE_ADEHAZE("%s:%d pAEPreRes is NULL, so use default instead \n", __FUNCTION__, __LINE__);
        }

        iso = stExpInfo.arIso[stExpInfo.hdr_mode];
        AdehazeHandle->CurrData.V20.ISO = (float)iso;
    }
    else if(AdehazeHandle->HWversion == ADEHAZE_ISP21) {
        XCamVideoBuffer* xCamAePreRes = procPara->rk_com.u.proc.res_comb->ae_pre_res;
        RkAiqAlgoPreResAeInt* pAEPreRes = NULL;
        if (xCamAePreRes) {
            pAEPreRes = (RkAiqAlgoPreResAeInt*)xCamAePreRes->map(xCamAePreRes);
            AdehazeGetEnvLv(AdehazeHandle, pAEPreRes);
        }
        else {
            AdehazeHandle->CurrData.V21.EnvLv = 0.0;
            LOGE_ADEHAZE( "%s:PreResBuf is NULL!\n", __FUNCTION__);
        }
    }

    //process
    if(!(AdehazeByPassProcessing(AdehazeHandle)))
        ret = AdehazeProcess(AdehazeHandle, AdehazeHandle->HWversion);

    //store data
    AdehazeHandle->FrameID++;
    if(AdehazeHandle->HWversion == ADEHAZE_ISP20)
        AdehazeHandle->PreData.V20.ApiMode = AdehazeHandle->AdehazeAtrr.AdehazeAtrrV20.mode;
    else if(AdehazeHandle->HWversion == ADEHAZE_ISP21)
        AdehazeHandle->PreData.V21.ApiMode = AdehazeHandle->AdehazeAtrr.AdehazeAtrrV21.mode;


    //proc res
    if(AdehazeHandle->HWversion == ADEHAZE_ISP20) {
        AdehazeHandle->ProcRes.ProcResV20.enable = true;
        AdehazeHandle->ProcRes.ProcResV20.update = !(AdehazeHandle->byPassProc) ;
    }
    else if(AdehazeHandle->HWversion == ADEHAZE_ISP21) {
        AdehazeHandle->ProcRes.ProcResV21.enable = true;
        AdehazeHandle->ProcRes.ProcResV21.update = !(AdehazeHandle->byPassProc);
    }
    memcpy(&procResPara->AdehzeProcRes, &AdehazeHandle->ProcRes, sizeof(RkAiqAdehazeProcResult_t));

    LOGD_ADEHAZE("%s:/*************************Adehaze over******************/ \n", __func__);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAdhaz = {
    .common = {
        .version = RKISP_ALGO_ADHAZ_VERSION,
        .vendor  = RKISP_ALGO_ADHAZ_VENDOR,
        .description = RKISP_ALGO_ADHAZ_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ADHAZ,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = pre_process,
    .processing = processing,
    .post_process = post_process,
};

RKAIQ_END_DECLARE
