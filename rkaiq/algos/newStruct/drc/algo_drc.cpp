/*
 * rk_aiq_algo_debayer_itf.c
 *
 *  Copyright (c) 2024 Rockchip Corporation
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

#include "rk_aiq_algo_types.h"
#include "drc_types_prvt.h"
#include "xcam_log.h"

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "RkAiqHandle.h"

#include "interpolation.h"

//RKAIQ_BEGIN_DECLARE
#if RKAIQ_HAVE_DRC_V12
XCamReturn DrcSelectParam(DrcContext_t* pDrcCtx, drc_param_t* out, int iso);
bool DrcDamping(drc_param_t* out, CurrData_t* pCurrData, int FrameID);
void DrcExpoParaProcessing(DrcContext_t* pDrcCtx, drc_param_t* out);
#endif
#if RKAIQ_HAVE_DRC_V20
XCamReturn DrcSelectParam(DrcContext_t* pDrcCtx, drc_param_t* out, int iso);
void DrcAutoProcessing(drc_param_t* out, drc_param_auto_t* paut, float iso);
bool DrcDamping(drc_param_t* out, CurrData_t* pCurrData, int FrameID);
void DrcExpoParaProcessing(DrcContext_t* pDrcCtx, drc_param_t* out, int iso);
#endif
static XCamReturn
create_context
(
    RkAiqAlgoContext** context,
    const AlgoCtxInstanceCfg* cfg
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    DrcContext_t *ctx = new DrcContext_t();
    if (ctx == NULL) {
        LOGE_ATMO( "%s: create Drc context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }

    ctx->isCapture         = false;
    ctx->isDampStable      = true;
    ctx->isReCal_ = true;
    ctx->prepare_params = NULL;
    ctx->drc_attrib =
        (drc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, drc));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ATMO("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
destroy_context
(
    RkAiqAlgoContext* context
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    DrcContext_t* pDrcCtx = (DrcContext_t*)context;
    delete pDrcCtx;
    return result;
}

static XCamReturn
prepare
(
    RkAiqAlgoCom* params
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    DrcContext_t* pDrcCtx = (DrcContext_t*)params->ctx;
    RkAiqAlgoConfigDrc* DrcCfgParam = (RkAiqAlgoConfigDrc*)params;

    if (DrcCfgParam->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        pDrcCtx->FrameNumber = LINEAR_NUM;
    else if (DrcCfgParam->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             DrcCfgParam->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        pDrcCtx->FrameNumber = HDR_2X_NUM;
    else
        pDrcCtx->FrameNumber = HDR_3X_NUM;
    if (DrcCfgParam->compr_bit) {
        pDrcCtx->FrameNumber = SENSOR_MGE;
        pDrcCtx->compr_bit   = DrcCfgParam->compr_bit;
        if (pDrcCtx->compr_bit > ISP_HDR_BIT_NUM_MAX)
            LOGE_ATMO("%s:  SensorMgeBitNum(%d) > %d!!!\n", __FUNCTION__, pDrcCtx->compr_bit,
                      ISP_HDR_BIT_NUM_MAX);
        if (pDrcCtx->compr_bit < ISP_HDR_BIT_NUM_MIN)
            LOGE_ATMO("%s:  SensorMgeBitNum(%d) < %d!!!\n", __FUNCTION__, pDrcCtx->compr_bit,
                      ISP_HDR_BIT_NUM_MIN);
        pDrcCtx->compr_bit =
            CLIP(pDrcCtx->compr_bit, ISP_HDR_BIT_NUM_MAX, ISP_HDR_BIT_NUM_MIN);
    }

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pDrcCtx->drc_attrib =
                (drc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, drc));
            return XCAM_RETURN_NO_ERROR;
        }
    } else if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGERES) {
        pDrcCtx->isCapture = true;
    }

    pDrcCtx->drc_attrib =
        (drc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, drc));
    pDrcCtx->prepare_params = &params->u.prepare;
    pDrcCtx->isReCal_ = true;

    return result;
}

static XCamReturn
processing
(
    const RkAiqAlgoCom* inparams,
    RkAiqAlgoResCom* outparams
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    DrcContext_t* pDrcCtx = (DrcContext_t*)inparams->ctx;
    pDrcCtx->FrameID = inparams->frame_id;
    drc_api_attrib_t* drc_attrib = pDrcCtx->drc_attrib;
    RkAiqAlgoProcResDrc* pDrcProcResParams = (RkAiqAlgoProcResDrc*)outparams;
    RkAiqAlgoProcDrc* drc_proc_param = (RkAiqAlgoProcDrc*)inparams;
    pDrcCtx->blc_ob_enable = drc_proc_param->blc_ob_enable;
    pDrcCtx->isp_ob_predgain = drc_proc_param->isp_ob_predgain;
    pDrcCtx->hw_transCfg_trans_mode = drc_proc_param->hw_transCfg_trans_mode;
    pDrcCtx->hw_transCfg_transOfDrc_offset = drc_proc_param->hw_transCfg_transOfDrc_offset;

    LOGV_ATMO("%s: (enter)\n", __FUNCTION__);

    if (!drc_attrib) {
        LOGE_ATMO("drc_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }

    if (drc_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ATMO("mode is %d, not auto mode, ignore", drc_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (inparams->u.proc.is_attrib_update) {
        pDrcCtx->isReCal_ = true;
    }

    if (pDrcCtx->isCapture) {
        LOGD_ATMO("%s: It's capturing, using pre frame params\n", __func__);
        pDrcCtx->isCapture = false;
        return XCAM_RETURN_NO_ERROR;
    }

    if (pDrcCtx->FrameNumber == HDR_2X_NUM || pDrcCtx->FrameNumber == HDR_3X_NUM ||
        pDrcCtx->FrameNumber == SENSOR_MGE)
        outparams->en = true;
    else if (pDrcCtx->FrameNumber == LINEAR_NUM) {
        if (pDrcCtx->blc_ob_enable)
            outparams->en = true;
        else {
            if (drc_attrib->opMode == RK_AIQ_OP_MODE_AUTO) {
                outparams->en = drc_attrib->en;
            } else {
                outparams->en = false;
            }
        }
    }

    if (outparams->en)
        pDrcCtx->NextData.AEData.LongFrmMode = drc_proc_param->LongFrmMode;

    if(pDrcCtx->FrameNumber == LINEAR_NUM) {
        pDrcCtx->NextData.AEData.SExpo =
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.integration_time;
        if (pDrcCtx->NextData.AEData.SExpo < FLT_EPSILON) {
            pDrcCtx->NextData.AEData.SExpo =
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.integration_time;
        }
        pDrcCtx->NextData.AEData.MExpo = pDrcCtx->NextData.AEData.SExpo;
        pDrcCtx->NextData.AEData.LExpo = pDrcCtx->NextData.AEData.SExpo;
    }
    else if(pDrcCtx->FrameNumber == HDR_2X_NUM) {
        pDrcCtx->NextData.AEData.SExpo =
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time;
        pDrcCtx->NextData.AEData.MExpo =
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time;
        if (pDrcCtx->NextData.AEData.SExpo < FLT_EPSILON) {
            pDrcCtx->NextData.AEData.SExpo =
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.integration_time;
            pDrcCtx->NextData.AEData.MExpo =
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.integration_time;
        }
        pDrcCtx->NextData.AEData.LExpo = pDrcCtx->NextData.AEData.MExpo;
    }
    else if(pDrcCtx->FrameNumber == HDR_3X_NUM) {
        pDrcCtx->NextData.AEData.SExpo =
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time;
        pDrcCtx->NextData.AEData.MExpo =
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time;
        pDrcCtx->NextData.AEData.LExpo =
            drc_proc_param->com.u.proc.nxtExp->HdrExp[2].exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[2].exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[2].exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[2].exp_real_params.integration_time;
        if (pDrcCtx->NextData.AEData.SExpo < FLT_EPSILON) {
            pDrcCtx->NextData.AEData.SExpo =
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.integration_time;
            pDrcCtx->NextData.AEData.MExpo =
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.integration_time;
            pDrcCtx->NextData.AEData.LExpo =
                drc_proc_param->com.u.proc.curExp->HdrExp[2].exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[2].exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[2].exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->HdrExp[2].exp_real_params.integration_time;
        }
    } else if (pDrcCtx->FrameNumber == SENSOR_MGE) {
        pDrcCtx->NextData.AEData.MExpo =
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.integration_time;
        if (pDrcCtx->NextData.AEData.MExpo < FLT_EPSILON) {
            pDrcCtx->NextData.AEData.MExpo =
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.integration_time;
        }
        pDrcCtx->NextData.AEData.LExpo = pDrcCtx->NextData.AEData.MExpo;
        pDrcCtx->NextData.AEData.SExpo =
            pDrcCtx->NextData.AEData.MExpo /
            pow(2.0f, float(pDrcCtx->compr_bit - ISP_HDR_BIT_NUM_MIN));
    }
    if (pDrcCtx->NextData.AEData.SExpo > FLT_EPSILON) {
        pDrcCtx->NextData.AEData.L2S_Ratio =
            pDrcCtx->NextData.AEData.LExpo / pDrcCtx->NextData.AEData.SExpo;
        if (pDrcCtx->NextData.AEData.L2S_Ratio < RATIO_DEFAULT) {
            LOGE_ATMO("%s: Next L2S_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                        __FUNCTION__, pDrcCtx->NextData.AEData.L2S_Ratio);
            pDrcCtx->NextData.AEData.L2S_Ratio = RATIO_DEFAULT;
        }
        pDrcCtx->NextData.AEData.M2S_Ratio =
            pDrcCtx->NextData.AEData.MExpo / pDrcCtx->NextData.AEData.SExpo;
        if (pDrcCtx->NextData.AEData.M2S_Ratio < RATIO_DEFAULT) {
            LOGE_ATMO("%s: Next M2S_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                        __FUNCTION__, pDrcCtx->NextData.AEData.M2S_Ratio);
            pDrcCtx->NextData.AEData.M2S_Ratio = RATIO_DEFAULT;
        }
    }
    else
        LOGE_ATMO("%s: Next Short frame for drc expo sync is %f!!!\n", __FUNCTION__,
                    pDrcCtx->NextData.AEData.SExpo);
    if (pDrcCtx->NextData.AEData.MExpo > FLT_EPSILON) {
        pDrcCtx->NextData.AEData.L2M_Ratio =
            pDrcCtx->NextData.AEData.LExpo / pDrcCtx->NextData.AEData.MExpo;
        if (pDrcCtx->NextData.AEData.L2M_Ratio < RATIO_DEFAULT) {
            LOGE_ATMO("%s: Next L2M_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                        __FUNCTION__, pDrcCtx->NextData.AEData.L2M_Ratio);
            pDrcCtx->NextData.AEData.L2M_Ratio = RATIO_DEFAULT;
        }
    } else
        LOGE_ATMO("%s: Next Midlle frame for drc expo sync is %f!!!\n", __FUNCTION__,
                    pDrcCtx->NextData.AEData.MExpo);
    //clip for long frame mode
    if (pDrcCtx->NextData.AEData.LongFrmMode) {
        pDrcCtx->NextData.AEData.L2S_Ratio = LONG_FRAME_MODE_RATIO;
        pDrcCtx->NextData.AEData.M2S_Ratio = LONG_FRAME_MODE_RATIO;
        pDrcCtx->NextData.AEData.L2M_Ratio = LONG_FRAME_MODE_RATIO;
    }
    // clip L2M_ratio to 32x
    if (pDrcCtx->NextData.AEData.L2M_Ratio > AE_RATIO_L2M_MAX) {
        LOGE_ATMO("%s: Next L2M_ratio:%f out of range, clip to 32.0x!!!\n", __FUNCTION__,
                    pDrcCtx->NextData.AEData.L2M_Ratio);
        pDrcCtx->NextData.AEData.L2M_Ratio = AE_RATIO_L2M_MAX;
    }
    // clip L2S_ratio
    if (pDrcCtx->NextData.AEData.L2S_Ratio > AE_RATIO_MAX) {
        LOGE_ATMO("%s: Next L2S_Ratio:%f out of range, clip to 256.0x!!!\n", __FUNCTION__,
                    pDrcCtx->NextData.AEData.L2S_Ratio);
        pDrcCtx->NextData.AEData.L2S_Ratio = AE_RATIO_MAX;
    }
    // clip L2M_ratio and M2S_Ratio
    if (pDrcCtx->NextData.AEData.L2M_Ratio * pDrcCtx->NextData.AEData.M2S_Ratio >
        AE_RATIO_MAX) {
        LOGE_ATMO("%s: Next L2M_Ratio*M2S_Ratio:%f out of range, clip to 256.0x!!!\n",
                    __FUNCTION__,
                    pDrcCtx->NextData.AEData.L2M_Ratio * pDrcCtx->NextData.AEData.M2S_Ratio);
        pDrcCtx->NextData.AEData.M2S_Ratio =
            AE_RATIO_MAX / pDrcCtx->NextData.AEData.L2M_Ratio;
    }

    int iso = inparams->u.proc.iso;
    if (pDrcCtx->FrameNumber == LINEAR_NUM&&
        drc_proc_param->blc_ob_enable &&
        drc_proc_param->isp_ob_predgain >= ISP_PREDGAIN_DEFAULT)
        iso = iso * drc_proc_param->isp_ob_predgain;
    bool init = inparams->u.proc.init;
    float delta_iso = (float)abs(iso - pDrcCtx->iso);
    delta_iso /= (float)pDrcCtx->iso;

    outparams->cfg_update = false;

    if (inparams->u.proc.is_bw_sensor) {
        drc_attrib->en = false;
        outparams->cfg_update = init ? true : false;
        return XCAM_RETURN_NO_ERROR;
    }

    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO) {
        pDrcCtx->isReCal_ = true;
    }

    bool bypass_expo_params = true;
    // get bypass_expo_params
    if(!pDrcCtx->CurrData.AEData.LongFrmMode !=
                !pDrcCtx->NextData.AEData.LongFrmMode)
        bypass_expo_params = false;
    else if ((pDrcCtx->CurrData.AEData.L2M_Ratio - pDrcCtx->NextData.AEData.L2M_Ratio) >
                    FLT_EPSILON ||
                (pDrcCtx->CurrData.AEData.L2M_Ratio - pDrcCtx->NextData.AEData.L2M_Ratio) <
                    -FLT_EPSILON ||
                (pDrcCtx->CurrData.AEData.M2S_Ratio - pDrcCtx->NextData.AEData.M2S_Ratio) >
                    FLT_EPSILON ||
                (pDrcCtx->CurrData.AEData.M2S_Ratio - pDrcCtx->NextData.AEData.M2S_Ratio) <
                    -FLT_EPSILON ||
                (pDrcCtx->CurrData.AEData.L2S_Ratio - pDrcCtx->NextData.AEData.L2S_Ratio) >
                    FLT_EPSILON ||
                (pDrcCtx->CurrData.AEData.L2S_Ratio - pDrcCtx->NextData.AEData.L2S_Ratio) <
                    -FLT_EPSILON)
        bypass_expo_params = false;
    else
        bypass_expo_params = true;

    if (pDrcCtx->isReCal_ || !bypass_expo_params) {
#if RKAIQ_HAVE_DRC_V12
        DrcSelectParam(pDrcCtx, pDrcProcResParams->drcRes, iso);
        DrcExpoParaProcessing(pDrcCtx, pDrcProcResParams->drcRes);
#endif
#if RKAIQ_HAVE_DRC_V20
        DrcSelectParam(pDrcCtx, pDrcProcResParams->drcRes, iso);
        DrcExpoParaProcessing(pDrcCtx, pDrcProcResParams->drcRes, iso);
#endif
        outparams->cfg_update = true;
        outparams->en = drc_attrib->en;
        outparams->bypass = drc_attrib->bypass;
        LOGI_ATMO("drc en:%d, bypass:%d", outparams->en, outparams->bypass);
    }

    pDrcCtx->iso = iso;
    pDrcCtx->isReCal_ = false;

    pDrcCtx->CurrData.AEData.LongFrmMode = pDrcCtx->NextData.AEData.LongFrmMode;
    pDrcCtx->CurrData.AEData.L2M_Ratio   = pDrcCtx->NextData.AEData.L2M_Ratio;
    pDrcCtx->CurrData.AEData.M2S_Ratio   = pDrcCtx->NextData.AEData.M2S_Ratio;
    pDrcCtx->CurrData.AEData.L2S_Ratio   = pDrcCtx->NextData.AEData.L2S_Ratio;

    LOGV_ATMO("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_DRC_V12
XCamReturn DrcSelectParam
(
    DrcContext_t* pDrcCtx,
    drc_param_t* out, int iso)
{
    LOGI_ATMO("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pDrcCtx == NULL) {
        LOGE_ATMO("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    drc_param_auto_t* paut = &pDrcCtx->drc_attrib->stAuto;
    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0;
    float ratio = 0.0f;
    pre_interp(iso, NULL, 0, ilow, ihigh, ratio);

    out->dyn.DrcGain.DrcGain = interpolation_f32(paut->dyn[ilow].DrcGain.DrcGain,
            paut->dyn[ihigh].DrcGain.DrcGain, ratio);
    out->dyn.DrcGain.Alpha = interpolation_f32(paut->dyn[ilow].DrcGain.Alpha,
            paut->dyn[ihigh].DrcGain.Alpha, ratio);
    out->dyn.DrcGain.Clip = interpolation_f32(paut->dyn[ilow].DrcGain.Clip,
            paut->dyn[ihigh].DrcGain.Clip, ratio);
    out->dyn.HiLightData.Strength = interpolation_f32(paut->dyn[ilow].HiLightData.Strength,
            paut->dyn[ihigh].HiLightData.Strength, ratio);
    out->dyn.HiLightData.gas_t = interpolation_f32(paut->dyn[ilow].HiLightData.gas_t,
            paut->dyn[ihigh].HiLightData.gas_t, ratio);

    out->dyn.LocalData.LocalWeit = interpolation_f32(paut->dyn[ilow].LocalData.LocalWeit,
            paut->dyn[ihigh].LocalData.LocalWeit, ratio);
    out->dyn.LocalData.GlobalContrast = interpolation_f32(paut->dyn[ilow].LocalData.GlobalContrast,
            paut->dyn[ihigh].LocalData.GlobalContrast, ratio);
    out->dyn.LocalData.LoLitContrast = interpolation_f32(paut->dyn[ilow].LocalData.LoLitContrast,
            paut->dyn[ihigh].LocalData.LoLitContrast, ratio);
    out->dyn.LocalData.LocalAutoEnable = paut->dyn[ilow].LocalData.LocalAutoEnable;
    out->dyn.LocalData.LocalAutoWeit = interpolation_f32(paut->dyn[ilow].LocalData.LocalAutoWeit,
            paut->dyn[ihigh].LocalData.LocalAutoWeit, ratio);
    // get MotionStr
    out->dyn.MotionData.MotionStr = paut->dyn[0].MotionData.MotionStr;
    // compress
    out->sta.CompressSetting.Mode = paut->sta.CompressSetting.Mode;
    for (int i = 0; i < ADRC_Y_NUM; i++)
        out->sta.CompressSetting.Manual_curve[i] = paut->sta.CompressSetting.Manual_curve[i];
    out->sta.LocalSetting.curPixWeit = paut->sta.LocalSetting.curPixWeit;
    out->sta.LocalSetting.preFrameWeit = paut->sta.LocalSetting.preFrameWeit;
    out->sta.LocalSetting.Range_sgm_pre = paut->sta.LocalSetting.Range_sgm_pre;
    out->sta.LocalSetting.Space_sgm_pre = paut->sta.LocalSetting.Space_sgm_pre;

    out->sta.LocalSetting.Range_force_sgm = paut->sta.LocalSetting.Range_force_sgm;
    out->sta.LocalSetting.Range_sgm_cur = paut->sta.LocalSetting.Range_sgm_cur;
    out->sta.LocalSetting.Space_sgm_cur = paut->sta.LocalSetting.Space_sgm_cur;
    // scale y
    for (int i = 0; i < DRC_Y_NUM; i++)
        out->sta.Scale_y[i] =
            (unsigned short)paut->sta.Scale_y[i];

    out->sta.sw_drc_byPass_thred = paut->sta.sw_drc_byPass_thred;
    out->sta.Edge_Weit = paut->sta.Edge_Weit;
    out->sta.IIR_frame =
        (float)MIN(pDrcCtx->FrameID + 1, (uint32_t)paut->sta.IIR_frame);
    out->sta.damp = paut->sta.damp;
    out->sta.OutPutLongFrame =
        pDrcCtx->NextData.AEData.LongFrmMode || paut->sta.OutPutLongFrame;
    // drc v12 add
    out->sta.HiLight.gas_l0 = paut->sta.HiLight.gas_l0;
    out->sta.HiLight.gas_l1 = paut->sta.HiLight.gas_l1;
    out->sta.HiLight.gas_l2 = paut->sta.HiLight.gas_l2;
    out->sta.HiLight.gas_l3 = paut->sta.HiLight.gas_l3;
    pDrcCtx->isDampStable =
            DrcDamping(out, &pDrcCtx->CurrData, pDrcCtx->FrameID);

    // clip drc gain
    if (pDrcCtx->FrameNumber == HDR_2X_NUM || pDrcCtx->FrameNumber == HDR_3X_NUM) {
        if (pDrcCtx->NextData.AEData.L2S_Ratio * out->dyn.DrcGain.DrcGain >
            MAX_AE_DRC_GAIN) {
            out->dyn.DrcGain.DrcGain =
                MAX(MAX_AE_DRC_GAIN / pDrcCtx->NextData.AEData.L2S_Ratio, GAINMIN);
            LOGI_ATMO("%s:  AERatio*DrcGain > 256x, DrcGain Clip to %f!!!\n", __FUNCTION__,
                      out->dyn.DrcGain.DrcGain);
        }
    } else if (pDrcCtx->FrameNumber == LINEAR_NUM) {
        if (pDrcCtx->isp_ob_predgain * out->dyn.DrcGain.DrcGain > MAX_AE_DRC_GAIN) {
            if (pDrcCtx->isp_ob_predgain > MAX_AE_DRC_GAIN)
                LOGE_ATMO("%s:  predgain > 256x!!!\n", __FUNCTION__);
            else
                out->dyn.DrcGain.DrcGain =
                    MAX(MAX_AE_DRC_GAIN / pDrcCtx->isp_ob_predgain, GAINMIN);
            LOGI_ATMO("%s:  predgain*DrcGain > 256x, DrcGain clip to %f!!!\n", __FUNCTION__,
                      out->dyn.DrcGain.DrcGain);
        }
    } else if (pDrcCtx->FrameNumber == SENSOR_MGE) {
        if (pow(2.0f, float(pDrcCtx->compr_bit - ISP_HDR_BIT_NUM_MIN)) *
                out->dyn.DrcGain.DrcGain > MAX_AE_DRC_GAIN) {
            if (pow(2.0f, float(pDrcCtx->compr_bit - ISP_HDR_BIT_NUM_MIN)) > MAX_AE_DRC_GAIN)
                LOGE_ATMO("%s:  SensorMgeRatio > 256x!!!\n", __FUNCTION__);
            else
                out->dyn.DrcGain.DrcGain = MAX(
                    MAX_AE_DRC_GAIN / pow(2.0f, float(pDrcCtx->compr_bit - ISP_HDR_BIT_NUM_MIN)),
                    GAINMIN);
            LOGI_ATMO("%s:  SensorMgeRatio*DrcGain > 256x, DrcGain clip to %f!!!\n", __FUNCTION__,
                      out->dyn.DrcGain.DrcGain);
        }
    }
    // clip gas_l0~3
    if (out->sta.HiLight.gas_l0 == GAS_L_MAX) {
        LOGE_ATMO("%s: gas_l0 equals %d, use default value\n", __FUNCTION__, GAS_L_MAX);
        out->sta.HiLight.gas_l0 = GAS_L0_DEFAULT;
        out->sta.HiLight.gas_l1 = GAS_L1_DEFAULT;
        out->sta.HiLight.gas_l2 = GAS_L2_DEFAULT;
        out->sta.HiLight.gas_l3 = GAS_L3_DEFAULT;
    }
    if ((out->sta.HiLight.gas_l0 + 2 * out->sta.HiLight.gas_l1 +
         out->sta.HiLight.gas_l2 + 2 * out->sta.HiLight.gas_l3) !=
        GAS_L_MAX) {
        // LOGE_ATMO("%s: gas_l0 + gas_l1 + gas_l2 + gas_l3 DO NOT equal %d, use default value\n",
        //           __FUNCTION__, GAS_L_MAX);
        out->sta.HiLight.gas_l0 = GAS_L0_DEFAULT;
        out->sta.HiLight.gas_l1 = GAS_L1_DEFAULT;
        out->sta.HiLight.gas_l2 = GAS_L2_DEFAULT;
        out->sta.HiLight.gas_l3 = GAS_L3_DEFAULT;
    }
    out->sta.delta_scalein = DELTA_SCALEIN_FIX;
    out->sta.offset_pow2   = SW_DRC_OFFSET_POW2_FIX;
    float tmp = 0.0f;
    /*luma[i] = pow((1.0f - luma[i] / 4096.0f), 2.0f)*/
    float luma[DRC_Y_NUM] = {1.0f,    0.8789f, 0.7656f, 0.6602f, 0.5625f, 0.4727f,
                                 0.3906f, 0.3164f, 0.2500f, 0.1914f, 0.1406f, 0.0977f,
                                 0.0625f, 0.0352f, 0.0156f, 0.0039f, 0.0f};

    for (int i = 0; i < DRC_Y_NUM; ++i) {
        tmp = 1 - out->dyn.DrcGain.Alpha * luma[i];
        if (pDrcCtx->blc_ob_enable)
            tmp = 1024.0f * pow(out->dyn.DrcGain.DrcGain * pDrcCtx->isp_ob_predgain, tmp);
        else
            tmp = 1024.0f * pow(out->dyn.DrcGain.DrcGain, tmp);
        out->sta.gain_y[i] = (unsigned short)(tmp);
    }
    
    if (pDrcCtx->drc_attrib->opMode == RK_AIQ_OP_MODE_AUTO && !pDrcCtx->isDampStable)
        pDrcCtx->CurrData.dynParams = out->dyn;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);

    return XCAM_RETURN_NO_ERROR;
}

bool DrcDamping(drc_param_t* out, CurrData_t* pCurrData, int FrameID) {
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    bool isDampStable = false;

    if (FrameID > INIT_CALC_PARAMS_NUM) {
        bool isDampStable_DrcGain = false, isDampStable_Alpha = false, isDampStable_Clip = false,
             isDampStable_Strength = false, isDampStable_LocalWeit = false,
             isDampStable_LocalAutoWeit = false, isDampStable_GlobalContrast = false,
             isDampStable_LoLitContrast = false, isDampStable_gas_t = false,
             isDampStable_MotionStr = false;

        if ((out->dyn.DrcGain.DrcGain - pCurrData->dynParams.DrcGain.DrcGain) <=
                FLT_EPSILON &&
            (out->dyn.DrcGain.DrcGain - pCurrData->dynParams.DrcGain.DrcGain) >=
                -FLT_EPSILON) {
            isDampStable_DrcGain = true;
        } else {
            out->dyn.DrcGain.DrcGain =
                out->sta.damp * out->dyn.DrcGain.DrcGain +
                (1.0f - out->sta.damp) * pCurrData->dynParams.DrcGain.DrcGain;
            isDampStable_DrcGain = false;
        }
        if ((out->dyn.DrcGain.Alpha - pCurrData->dynParams.DrcGain.Alpha) <=
                FLT_EPSILON &&
            (out->dyn.DrcGain.Alpha - pCurrData->dynParams.DrcGain.Alpha) >=
                -FLT_EPSILON) {
            isDampStable_Alpha = true;
        } else {
            out->dyn.DrcGain.Alpha =
                out->sta.damp * out->dyn.DrcGain.Alpha +
                (1.0f - out->sta.damp) * pCurrData->dynParams.DrcGain.Alpha;
            isDampStable_Alpha = false;
        }
        if ((out->dyn.DrcGain.Clip - pCurrData->dynParams.DrcGain.Clip) <=
                FLT_EPSILON &&
            (out->dyn.DrcGain.Clip - pCurrData->dynParams.DrcGain.Clip) >=
                -FLT_EPSILON) {
            isDampStable_Clip = true;
        } else {
            out->dyn.DrcGain.Clip =
                out->sta.damp * out->dyn.DrcGain.Clip +
                (1.0f - out->sta.damp) * pCurrData->dynParams.DrcGain.Clip;
            isDampStable_Clip = false;
        }
        if ((out->dyn.HiLightData.Strength - pCurrData->dynParams.HiLightData.Strength) <=
                FLT_EPSILON &&
            (out->dyn.HiLightData.Strength - pCurrData->dynParams.HiLightData.Strength) >=
                -FLT_EPSILON) {
            isDampStable_Strength = true;
        } else {
            out->dyn.HiLightData.Strength =
                out->sta.damp * out->dyn.HiLightData.Strength +
                (1.0f - out->sta.damp) * pCurrData->dynParams.HiLightData.Strength;
            isDampStable_Strength = false;
        }
        if ((out->dyn.LocalData.LocalWeit - pCurrData->dynParams.LocalData.LocalWeit) <=
                FLT_EPSILON &&
            (out->dyn.LocalData.LocalWeit - pCurrData->dynParams.LocalData.LocalWeit) >=
                -FLT_EPSILON) {
            isDampStable_LocalWeit = true;
        } else {
            out->dyn.LocalData.LocalWeit =
                out->sta.damp * out->dyn.LocalData.LocalWeit +
                (1.0f - out->sta.damp) * pCurrData->dynParams.LocalData.LocalWeit;
            isDampStable_LocalWeit = false;
        }
        if ((out->dyn.LocalData.LocalAutoWeit -
             pCurrData->dynParams.LocalData.LocalAutoWeit) <= FLT_EPSILON &&
            (out->dyn.LocalData.LocalAutoWeit -
             pCurrData->dynParams.LocalData.LocalAutoWeit) >= -FLT_EPSILON) {
            isDampStable_LocalAutoWeit = true;
        } else {
            out->dyn.LocalData.LocalAutoWeit =
                out->sta.damp * out->dyn.LocalData.LocalAutoWeit +
                (1.0f - out->sta.damp) * pCurrData->dynParams.LocalData.LocalAutoWeit;
            isDampStable_LocalAutoWeit = false;
        }
        if ((out->dyn.LocalData.GlobalContrast -
             pCurrData->dynParams.LocalData.GlobalContrast) <= FLT_EPSILON &&
            (out->dyn.LocalData.GlobalContrast -
             pCurrData->dynParams.LocalData.GlobalContrast) >= -FLT_EPSILON) {
            isDampStable_GlobalContrast = true;
        } else {
            out->dyn.LocalData.GlobalContrast =
                out->sta.damp * out->dyn.LocalData.GlobalContrast +
                (1.0f - out->sta.damp) * pCurrData->dynParams.LocalData.GlobalContrast;
            isDampStable_GlobalContrast = false;
        }
        if ((out->dyn.LocalData.LoLitContrast -
             pCurrData->dynParams.LocalData.LoLitContrast) <= FLT_EPSILON &&
            (out->dyn.LocalData.LoLitContrast -
             pCurrData->dynParams.LocalData.LoLitContrast) >= -FLT_EPSILON) {
            isDampStable_LoLitContrast = true;
        } else {
            out->dyn.LocalData.LoLitContrast =
                out->sta.damp * out->dyn.LocalData.LoLitContrast +
                (1.0f - out->sta.damp) * pCurrData->dynParams.LocalData.LoLitContrast;
            isDampStable_LoLitContrast = false;
        }
        // drc v12
        if ((out->dyn.HiLightData.gas_t - pCurrData->dynParams.HiLightData.gas_t) <=
                FLT_EPSILON &&
            (out->dyn.HiLightData.gas_t - pCurrData->dynParams.HiLightData.gas_t) >=
                -FLT_EPSILON) {
            isDampStable_gas_t = true;
        } else {
            out->dyn.HiLightData.gas_t =
                out->sta.damp * out->dyn.HiLightData.gas_t +
                (1.0f - out->sta.damp) * pCurrData->dynParams.HiLightData.gas_t;
            isDampStable_gas_t = false;
        }
        if ((out->dyn.MotionData.MotionStr - pCurrData->dynParams.MotionData.MotionStr) <=
                FLT_EPSILON &&
            (out->dyn.MotionData.MotionStr - pCurrData->dynParams.MotionData.MotionStr) >=
                -FLT_EPSILON) {
            isDampStable_MotionStr = true;
        } else {
            out->dyn.MotionData.MotionStr =
                out->sta.damp * out->dyn.MotionData.MotionStr +
                (1.0f - out->sta.damp) * pCurrData->dynParams.MotionData.MotionStr;
            isDampStable_MotionStr = false;
        }
        isDampStable = isDampStable_DrcGain && isDampStable_Alpha && isDampStable_Clip &&
                       isDampStable_Strength && isDampStable_LocalWeit &&
                       isDampStable_LocalAutoWeit && isDampStable_GlobalContrast &&
                       isDampStable_LoLitContrast && isDampStable_gas_t && isDampStable_MotionStr;
    } else
        isDampStable = false;

    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);
    return isDampStable;
}

void DrcExpoParaProcessing(DrcContext_t* pDrcCtx, drc_param_t* out) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // get sw_drc_compres_scl
    float adrc_gain = 1.0f;
    if (pDrcCtx->FrameNumber == LINEAR_NUM && pDrcCtx->blc_ob_enable)
        adrc_gain = out->dyn.DrcGain.DrcGain * pDrcCtx->isp_ob_predgain;
    else
        adrc_gain = out->dyn.DrcGain.DrcGain;
    float log_ratio2     = log(pDrcCtx->NextData.AEData.L2S_Ratio * adrc_gain) / log(2.0f) + 12.0f;
    float offsetbits_int = out->sta.offset_pow2;
    float offsetbits     = offsetbits_int * (1 << MFHDR_LOG_Q_BITS);
    float hdrbits        = log_ratio2 * (1 << MFHDR_LOG_Q_BITS);
    float hdrvalidbits   = hdrbits - offsetbits;
    out->sta.compres_scl = (12.0f * (1 << (MFHDR_LOG_Q_BITS * 2))) / hdrvalidbits;

    // get sw_drc_min_ogain
    if (out->sta.OutPutLongFrame)
        out->sta.sw_drc_min_ogain = 1;
    else {
        out->sta.sw_drc_min_ogain = 1.0f / (pDrcCtx->NextData.AEData.L2S_Ratio * adrc_gain);
    }

    // get sw_drc_compres_y
    if (out->sta.CompressSetting.Mode == drc_COMPRESS_AUTO) {
        float curveparam, curveparam2, curveparam3, tmp;
        float luma2[DRC_Y_NUM] = {0.0f,     1024.0f,  2048.0f,  3072.0f,  4096.0f,  5120.0f,
                                      6144.0f,  7168.0f,  8192.0f,  10240.0f, 12288.0f, 14336.0f,
                                      16384.0f, 18432.0f, 20480.0f, 22528.0f, 24576.0f};
        float curveTable;
        float dstbits   = ISP_RAW_BIT * (1 << MFHDR_LOG_Q_BITS);
        float validbits = dstbits - offsetbits;
        for (int i = 0; i < DRC_Y_NUM; ++i) {
            curveparam =
                (float)(validbits - 0.0f) / (hdrvalidbits - validbits + DRC_COMPRESS_Y_OFFSET);
            curveparam2 = validbits * (1.0f + curveparam);
            curveparam3   = hdrvalidbits * curveparam;
            tmp           = luma2[i] * hdrvalidbits / 24576.0f;
            curveTable    = (tmp * curveparam2 / (tmp + curveparam3));
            out->sta.compres_y[i] = (unsigned short)(curveTable);
        }
    } else if (out->sta.CompressSetting.Mode == drc_COMPRESS_MANUAL) {
        for (int i = 0; i < DRC_Y_NUM; ++i)
            out->sta.compres_y[i] =
                (unsigned short)(out->sta.CompressSetting.Manual_curve[i]);
    }
}
#endif

#if RKAIQ_HAVE_DRC_V20
XCamReturn DrcSelectParam
(
    DrcContext_t* pDrcCtx,
    drc_param_t* out, int iso)
{
    LOGI_ATMO("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pDrcCtx == NULL) {
        LOGE_ATMO("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    drc_param_auto_t* paut = &pDrcCtx->drc_attrib->stAuto;
    DrcAutoProcessing(out, paut, iso);
    // pDrcCtx->isDampStable = DrcDamping(out, &pDrcCtx->CurrData, pDrcCtx->FrameID);
    // clip drc gain
    if (pDrcCtx->FrameNumber == HDR_2X_NUM || pDrcCtx->FrameNumber == HDR_3X_NUM) {
        if (pDrcCtx->NextData.AEData.L2S_Ratio *
                out->dyn.preProc.sw_drcT_toneGain_maxLimit > MAX_AE_DRC_GAIN) {
            LOGE_ATMO("%s:  AERatio*sw_drcT_toneGain_maxLimit > 256!!!\n", __FUNCTION__);
            out->dyn.preProc.sw_drcT_toneGain_maxLimit =
                MAX(MAX_AE_DRC_GAIN / pDrcCtx->NextData.AEData.L2S_Ratio, GAINMIN);
        }
    } else if (pDrcCtx->FrameNumber == LINEAR_NUM) {
        if (pDrcCtx->isp_ob_predgain *
                out->dyn.preProc.sw_drcT_toneGain_maxLimit > MAX_AE_DRC_GAIN) {
            LOGE_ATMO("%s:  predgain*sw_drcT_toneGain_maxLimit > 256!!!\n", __FUNCTION__);
            if (pDrcCtx->isp_ob_predgain > MAX_AE_DRC_GAIN)
                LOGE_ATMO("%s:  predgain > 256!!!\n", __FUNCTION__);
            else
                out->dyn.preProc.sw_drcT_toneGain_maxLimit =
                    MAX(MAX_AE_DRC_GAIN / pDrcCtx->isp_ob_predgain, GAINMIN);
        }
    } else if (pDrcCtx->FrameNumber == SENSOR_MGE) {
        if (pow(2.0f, float(pDrcCtx->compr_bit - ISP_HDR_BIT_NUM_MIN)) *
                out->dyn.preProc.sw_drcT_toneGain_maxLimit > MAX_AE_DRC_GAIN) {
            if (pow(2.0f, float(pDrcCtx->compr_bit - ISP_HDR_BIT_NUM_MIN)) > MAX_AE_DRC_GAIN)
                LOGE_ATMO("%s:  SensorMgeRatio > 256x!!!\n", __FUNCTION__);
            else
                out->dyn.preProc.sw_drcT_toneGain_maxLimit = MAX(
                    MAX_AE_DRC_GAIN / pow(2.0f, float(pDrcCtx->compr_bit - ISP_HDR_BIT_NUM_MIN)),
                    GAINMIN);
            LOGI_ATMO(
                "%s:  SensorMgeRatio*sw_drcT_toneGain_maxLimit > 256x, sw_drcT_toneGain_maxLimit "
                "clip to %f!!!\n",
                __FUNCTION__,
                out->dyn.preProc.sw_drcT_toneGain_maxLimit);
        }
    }

    // get sw_drc_gain_y
    if (out->dyn.preProc.sw_drcT_toneCurve_mode == drc_cfgCurveCtrlCoeff_mode) {
        float tmp = 0.0f;
        /*luma[i] = pow((1.0f - luma[i] / 4096.0f), 2.0f)*/
        float luma[DRC_CURVE_LEN] = {1.0f,    0.8789f, 0.7656f, 0.6602f, 0.5625f, 0.4727f,
                                    0.3906f, 0.3164f, 0.2500f, 0.1914f, 0.1406f, 0.0977f,
                                    0.0625f, 0.0352f, 0.0156f, 0.0039f, 0.0f };
        float DrcGain = out->dyn.preProc.sw_drcT_toneGain_maxLimit;
        float alpha = out->dyn.preProc.sw_drcT_toneCurveK_coeff;
        for (int i = 0; i < DRC_CURVE_LEN; ++i) {
            tmp = 1 - alpha * luma[i];
            if (pDrcCtx->blc_ob_enable)
                tmp = 1024.0f * pow(DrcGain * pDrcCtx->isp_ob_predgain, tmp) *
                    pow(pDrcCtx->isp_ob_predgain, -alpha * luma[i]);
            else
                tmp = 1024.0f * pow(DrcGain, tmp);
            out->dyn.preProc.hw_drcT_luma2ToneGain_val[i] = (unsigned short)(tmp);
        }
    }

    if (pDrcCtx->drc_attrib->opMode == RK_AIQ_OP_MODE_AUTO)
        pDrcCtx->CurrData.dynParams = out->dyn;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);

    return XCAM_RETURN_NO_ERROR;
}

void DrcAutoProcessing(drc_param_t* out, drc_param_auto_t* paut, float iso) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    pre_interp(iso, NULL, 0, ilow, ihigh, ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    // get preProc
    // get sw_drcT_toneGain_maxLimit
    out->dyn.preProc.sw_drcT_toneGain_maxLimit =
        interpolation_f32(paut->dyn[ilow].preProc.sw_drcT_toneGain_maxLimit,
            paut->dyn[ihigh].preProc.sw_drcT_toneGain_maxLimit, ratio);
    // get sw_drcT_toneCurveK_coeff
    out->dyn.preProc.sw_drcT_toneCurveK_coeff =
        interpolation_f32(paut->dyn[ilow].preProc.sw_drcT_toneCurveK_coeff,
            paut->dyn[ihigh].preProc.sw_drcT_toneCurveK_coeff, ratio);
    // get sw_drcT_toneCurve_mode
    out->dyn.preProc.sw_drcT_toneCurve_mode =
        paut->dyn[ilow].preProc.sw_drcT_toneCurve_mode;
    // get hw_drcT_toneCurveIdx_scale
    out->dyn.preProc.hw_drcT_toneCurveIdx_scale =
        interpolation_f32(paut->dyn[ilow].preProc.hw_drcT_toneCurveIdx_scale,
            paut->dyn[ihigh].preProc.hw_drcT_toneCurveIdx_scale, ratio);
    // get hw_drcT_lpfSoftThd_thred
    out->dyn.preProc.hw_drcT_lpfSoftThd_thred =
        interpolation_f32(paut->dyn[ilow].preProc.hw_drcT_lpfSoftThd_thred,
            paut->dyn[ihigh].preProc.hw_drcT_lpfSoftThd_thred, ratio);
    // get hw_drcT_luma2ToneGain_val
    for (int i = 0; i < DRC_CURVE_LEN; i++)
        out->dyn.preProc.hw_drcT_luma2ToneGain_val[i] =
            interpolation_f32(paut->dyn[ilow].preProc.hw_drcT_luma2ToneGain_val[i],
                paut->dyn[ihigh].preProc.hw_drcT_luma2ToneGain_val[i], ratio);

    // get bifilt_filter
    // get hw_drcT_bifiltOut_alpha
    out->dyn.bifilt_filter.hw_drcT_bifiltOut_alpha =
        interpolation_u8(paut->dyn[ilow].bifilt_filter.hw_drcT_bifiltOut_alpha,
            paut->dyn[ihigh].bifilt_filter.hw_drcT_bifiltOut_alpha, uratio);
    // get hw_drcT_softThd_en
    out->dyn.bifilt_filter.hw_drcT_softThd_en =
        paut->dyn[ilow].bifilt_filter.hw_drcT_softThd_en;
    // get hw_drcT_softThd_thred
    out->dyn.bifilt_filter.hw_drcT_softThd_thred =
        interpolation_f32(paut->dyn[ilow].bifilt_filter.hw_drcT_softThd_thred,
            paut->dyn[ihigh].bifilt_filter.hw_drcT_softThd_thred, uratio);
    // get hw_drcT_centerPixel_wgt
    out->dyn.bifilt_filter.hw_drcT_centerPixel_wgt =
        interpolation_u8(paut->dyn[ilow].bifilt_filter.hw_drcT_centerPixel_wgt,
            paut->dyn[ihigh].bifilt_filter.hw_drcT_centerPixel_wgt, uratio);
    // get hw_drcT_midWgt_alpha
    out->dyn.bifilt_filter.hw_drcT_midWgt_alpha =
        interpolation_u8(paut->dyn[ilow].bifilt_filter.hw_drcT_midWgt_alpha,
            paut->dyn[ihigh].bifilt_filter.hw_drcT_midWgt_alpha, uratio);
    // get hw_drcT_rgeWgt_negOff
    out->dyn.bifilt_filter.hw_drcT_rgeWgt_negOff =
        interpolation_u8(paut->dyn[ilow].bifilt_filter.hw_drcT_rgeWgt_negOff,
            paut->dyn[ihigh].bifilt_filter.hw_drcT_rgeWgt_negOff, uratio);
    // get hw_drcT_midRgeSgm_val
    out->dyn.bifilt_filter.hw_drcT_midRgeSgm_val =
        interpolation_f32(paut->dyn[ilow].bifilt_filter.hw_drcT_midRgeSgm_val,
            paut->dyn[ihigh].bifilt_filter.hw_drcT_midRgeSgm_val, uratio);
    // get hw_drcT_loRgeSgm_val
    out->dyn.bifilt_filter.hw_drcT_loRgeSgm_val =
        interpolation_f32(paut->dyn[ilow].bifilt_filter.hw_drcT_loRgeSgm_val,
            paut->dyn[ihigh].bifilt_filter.hw_drcT_loRgeSgm_val, uratio);

    // get bifilt_guideDiff
    // get hw_drc_thumb_maxLimit
    out->dyn.bifilt_guideDiff.hw_drcT_guideDiffLmt_en =
        interpolation_bool(paut->dyn[ilow].bifilt_guideDiff.hw_drcT_guideDiffLmt_en,
            paut->dyn[ihigh].bifilt_guideDiff.hw_drcT_guideDiffLmt_en, uratio);
    out->dyn.bifilt_guideDiff.hw_drcT_guideLuma_maxLimit =
        interpolation_f32(paut->dyn[ilow].bifilt_guideDiff.hw_drcT_guideLuma_maxLimit,
            paut->dyn[ihigh].bifilt_guideDiff.hw_drcT_guideLuma_maxLimit, uratio);
    // get hw_drcT_maxLutIdx_scale
    out->dyn.bifilt_guideDiff.hw_drcT_maxLutIdx_scale =
        interpolation_f32(paut->dyn[ilow].bifilt_guideDiff.hw_drcT_maxLutIdx_scale,
            paut->dyn[ihigh].bifilt_guideDiff.hw_drcT_maxLutIdx_scale, uratio);
    // get hw_drcT_guideDiff_minLimit
    out->dyn.bifilt_guideDiff.hw_drcT_guideDiff_minLimit =
        interpolation_f32(paut->dyn[ilow].bifilt_guideDiff.hw_drcT_guideDiff_minLimit,
            paut->dyn[ihigh].bifilt_guideDiff.hw_drcT_guideDiff_minLimit, uratio);
    // get sw_drcT_gdDiffMaxLut_mode
    out->dyn.bifilt_guideDiff.sw_drcT_gdDiffMaxLut_mode = 
        paut->dyn[ilow].bifilt_guideDiff.sw_drcT_gdDiffMaxLut_mode;
    // get hw_drcT_gdLuma2DiffMax_lut
    for (int i = 0; i < DRC_CURVE_LEN; i++)
        out->dyn.bifilt_guideDiff.hw_drcT_gdLuma2DiffMax_lut[i] =
            interpolation_f32(paut->dyn[ilow].bifilt_guideDiff.hw_drcT_gdLuma2DiffMax_lut[i],
                paut->dyn[ihigh].bifilt_guideDiff.hw_drcT_gdLuma2DiffMax_lut[i], uratio);
    // get sw_drc_thumbThred_minLimit
    out->dyn.bifilt_guideDiff.sw_drcT_maxLutCreate_minLimit =
        interpolation_f32(paut->dyn[ilow].bifilt_guideDiff.sw_drcT_maxLutCreate_minLimit,
            paut->dyn[ihigh].bifilt_guideDiff.sw_drcT_maxLutCreate_minLimit, uratio);
    // get sw_drc_thumbThred_maxLimit
    out->dyn.bifilt_guideDiff.sw_drcT_maxLutCreate_maxLimit =
        interpolation_f32(paut->dyn[ilow].bifilt_guideDiff.sw_drcT_maxLutCreate_maxLimit,
            paut->dyn[ihigh].bifilt_guideDiff.sw_drcT_maxLutCreate_maxLimit, uratio);
    // get sw_drc_thumbThredCurve_slope
    out->dyn.bifilt_guideDiff.sw_drcT_maxLutCreate_slope =
        interpolation_f32(paut->dyn[ilow].bifilt_guideDiff.sw_drcT_maxLutCreate_slope,
            paut->dyn[ihigh].bifilt_guideDiff.sw_drcT_maxLutCreate_slope, uratio);
    // get sw_drc_thumbThredCurve_offset
    out->dyn.bifilt_guideDiff.sw_drcT_maxLutCreate_offset =
        interpolation_f32(paut->dyn[ilow].bifilt_guideDiff.sw_drcT_maxLutCreate_offset,
            paut->dyn[ihigh].bifilt_guideDiff.sw_drcT_maxLutCreate_offset, uratio);

    // get drcProc
    // get hw_drcT_loDetail_strg
    out->dyn.drcProc.hw_drcT_loDetail_strg =
        interpolation_f32(paut->dyn[ilow].drcProc.hw_drcT_loDetail_strg,
            paut->dyn[ihigh].drcProc.hw_drcT_loDetail_strg, uratio);
    // get hw_drcT_drcStrg_alpha
    out->dyn.drcProc.hw_drcT_drcStrg_alpha =
        interpolation_f32(paut->dyn[ilow].drcProc.hw_drcT_drcStrg_alpha,
            paut->dyn[ihigh].drcProc.hw_drcT_drcStrg_alpha, uratio);
    // get hw_drcT_drcStrgLutLuma_scale
    out->dyn.drcProc.hw_drcT_drcStrgLutLuma_scale =
        interpolation_f32(paut->dyn[ilow].drcProc.hw_drcT_drcStrgLutLuma_scale,
            paut->dyn[ihigh].drcProc.hw_drcT_drcStrgLutLuma_scale, uratio);
    // get sw_drcT_drcStrgLut_mode
    out->dyn.drcProc.sw_drcT_drcStrgLut_mode =
        paut->dyn[ilow].drcProc.sw_drcT_drcStrgLut_mode; //TODO
    // get hw_drc_luma2compsGainScale_val
    for (int i = 0; i < DRC_CURVE_LEN; i++)
        out->dyn.drcProc.hw_drcT_luma2DrcStrg_val[i] =
            interpolation_f32(paut->dyn[ilow].drcProc.hw_drcT_luma2DrcStrg_val[i],
                paut->dyn[ihigh].drcProc.hw_drcT_luma2DrcStrg_val[i], uratio);
    // get sw_drcT_drcCurve_mode
    out->dyn.drcProc.sw_drcT_drcCurve_mode =
        paut->dyn[ilow].drcProc.sw_drcT_drcCurve_mode;
    // get sw_drcT_drcGainLimit_mode
    out->dyn.drcProc.sw_drcT_drcGainLimit_mode =
        paut->dyn[ilow].drcProc.sw_drcT_drcGainLimit_mode;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

bool DrcDamping(drc_param_t* out, CurrData_t* pCurrData, int FrameID) {
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    bool isDampStable = false;

    // if (FrameID > 2) {
    //     bool isDampStable_DrcGain = false, isDampStable_Alpha = false,
    //          isDampStable_LocalAutoWeitEn = false, isDampStable_LocalWeit = false,
    //          isDampStable_LocalAutoWeit = false, isDampStable_GlobalContrast = false,
    //          isDampStable_LoLitContrast = false;

    //     // isDampStable_DrcGain
    //     if ((out->dyn.preProc.sw_drcT_toneGain_maxLimit -
    //          pCurrData->dynParams.preProc
    //              .sw_drcT_toneGain_maxLimit) <= FLT_EPSILON &&
    //         (out->dyn.preProc.sw_drcT_toneGain_maxLimit -
    //          pCurrData->dynParams.preProc
    //              .sw_drcT_toneGain_maxLimit) >= -FLT_EPSILON) {
    //         isDampStable_DrcGain = true;
    //     } else {
    //         out->dyn.preProc.sw_drcT_toneGain_maxLimit =
    //             out->sta.sw_drc_damp_coef * out->dyn.preProc.sw_drcT_toneGain_maxLimit +
    //             (1.0f - out->sta.sw_drc_damp_coef) * out->dyn.preProc.sw_drcT_toneGain_maxLimit;
    //         isDampStable_DrcGain = false;
    //     }
    //     // isDampStable_Alpha
    //     if ((out->dyn.preProc.hw_drcT_toneCurveIdx_scale -
    //          pCurrData->dynParams.preProc.hw_drcT_toneCurveIdx_scale) <= FLT_EPSILON &&
    //         (out->dyn.preProc.hw_drcT_toneCurveIdx_scale -
    //          pCurrData->dynParams.preProc.hw_drcT_toneCurveIdx_scale) >= -FLT_EPSILON) {
    //         isDampStable_Alpha = true;
    //     } else {
    //         out->dyn.preProc.hw_drcT_toneCurveIdx_scale =
    //             out->sta.sw_drc_damp_coef * out->dyn.preProc.hw_drcT_toneCurveIdx_scale +
    //             (1.0f - out->sta.sw_drc_damp_coef)
    //             * pCurrData->dynParams.preProc.hw_drcT_toneCurveIdx_scale;
    //         isDampStable_Alpha = false;
    //     }
    //     // isDampStable_LocalWeit
    //     if (out->dyn.bifilt_filter.hw_drcT_bifiltOut_alpha ==
    //          pCurrData->dynParams.bifilt_filter.hw_drcT_bifiltOut_alpha) {
    //         isDampStable_LocalWeit = true;
    //     } else {
    //         out->dyn.bifilt_filter.hw_drcT_bifiltOut_alpha =
    //             out->sta.sw_drc_damp_coef * out->dyn.bifilt_filter.hw_drcT_bifiltOut_alpha +
    //             (1.0f - out->sta.sw_drc_damp_coef) * pCurrData->dynParams.bifilt_filter.hw_drcT_bifiltOut_alpha;
    //         isDampStable_LocalWeit = false;
    //     }
    //     // isDampStable_LocalAutoWeit
    //     if (out->dyn.bifilt_filter.hw_adrc_softThd_thred ==
    //          pCurrData->dynParams.bifilt_filter.hw_adrc_softThd_thred) {
    //         isDampStable_LocalAutoWeit = true;
    //     } else {
    //         out->dyn.bifilt_filter.hw_adrc_softThd_thred =
    //             out->sta.sw_drc_damp_coef * out->dyn.bifilt_filter.hw_adrc_softThd_thred +
    //             (1.0f - out->sta.sw_drc_damp_coef) * pCurrData->dynParams.bifilt_filter.hw_adrc_softThd_thred;
    //         isDampStable_LocalAutoWeit = false;
    //     }
    //     // isDampStable_LocalAutoWeitEn
    //     if (out->dyn.bifilt_filter.hw_adrc_softThd_en ==
    //          pCurrData->dynParams.bifilt_filter.hw_adrc_softThd_en) {
    //         isDampStable_LocalAutoWeitEn = true;
    //     } else {
    //         out->dyn.bifilt_filter.hw_adrc_softThd_en =
    //             out->sta.sw_drc_damp_coef * out->dyn.bifilt_filter.hw_adrc_softThd_en +
    //             (1.0f - out->sta.sw_drc_damp_coef) * pCurrData->dynParams.bifilt_filter.hw_adrc_softThd_en;
    //         isDampStable_LocalAutoWeitEn = false;
    //     }
    //     // isDampStable_GlobalContrast
    //     if (out->dyn.drcProc.hw_drcT_drcStrg_alpha ==
    //          pCurrData->dynParams.drcProc.hw_drcT_drcStrg_alpha) {
    //         isDampStable_GlobalContrast = true;
    //     } else {
    //         out->dyn.drcProc.hw_drcT_drcStrg_alpha =
    //             out->sta.sw_drc_damp_coef * out->dyn.drcProc.hw_drcT_drcStrg_alpha +
    //             (1.0f - out->sta.sw_drc_damp_coef) * pCurrData->dynParams.drcProc.hw_drcT_drcStrg_alpha;
    //         isDampStable_GlobalContrast = false;
    //     }
    //     // isDampStable_LoLitContrast
    //     if (out->dyn.drcProc.hw_drcT_loDetail_strg ==
    //          pCurrData->dynParams.drcProc.hw_drcT_loDetail_strg) {
    //         isDampStable_LoLitContrast = true;
    //     } else {
    //         out->dyn.drcProc.hw_drcT_loDetail_strg =
    //             out->sta.sw_drc_damp_coef * out->dyn.drcProc.hw_drcT_loDetail_strg +
    //             (1.0f - out->sta.sw_drc_damp_coef) * pCurrData->dynParams.drcProc.hw_drcT_loDetail_strg;
    //         isDampStable_LoLitContrast = false;
    //     }

    //     isDampStable = isDampStable_DrcGain && isDampStable_Alpha && isDampStable_LocalAutoWeitEn &&
    //                    isDampStable_LocalWeit && isDampStable_LocalAutoWeit &&
    //                    isDampStable_GlobalContrast && isDampStable_LoLitContrast;
    // } else
    //     isDampStable = false;

    // LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);
    return isDampStable;
}

void DrcExpoParaProcessing(DrcContext_t* pDrcCtx, drc_param_t* out, int iso) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // get sw_drc_compres_scl
    float drc_gain = 1.0f;
    if (pDrcCtx->FrameNumber == LINEAR_NUM && pDrcCtx->blc_ob_enable)
        drc_gain = out->dyn.preProc.sw_drcT_toneGain_maxLimit * pDrcCtx->isp_ob_predgain;
    else
        drc_gain = out->dyn.preProc.sw_drcT_toneGain_maxLimit;
    float log_ratio2     = log(pDrcCtx->NextData.AEData.L2S_Ratio * drc_gain) / log(2.0f) + 12.0f;
    float offsetbits_int = CLIP(pDrcCtx->hw_transCfg_transOfDrc_offset, 0, 15);
    
    unsigned char cmps_fixbit_mode = (pDrcCtx->hw_transCfg_trans_mode == trans_lgi3f9_mode ?
                                      FUNCTION_ENABLE : FUNCTION_DISABLE);
    LOGD_ATMO("pDrcCtx->hw_transCfg_transOfDrc_offset %d", pDrcCtx->hw_transCfg_transOfDrc_offset);
    int cmps_fix_bit = 8 + cmps_fixbit_mode;
    float offsetbits = offsetbits_int * (1 << cmps_fix_bit);
    float hdrbits        = log_ratio2 * (1 << cmps_fix_bit);
    float hdrvalidbits   = hdrbits - offsetbits;
    out->dyn.preProc.hw_drcT_drcCurveLuma_scale = (12.0f * (1 << (cmps_fix_bit + 11))) / hdrvalidbits;

    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    pre_interp(iso, NULL, 0, ilow, ihigh, ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    // get hw_drc_comps_gain_minLimit
    drc_param_auto_t* paut = &pDrcCtx->drc_attrib->stAuto;
    if (out->dyn.drcProc.sw_drcT_drcGainLimit_mode == drc_drcGainLmt_auto_mode) {
        out->dyn.drcProc.hw_drcT_drcGain_minLimit =
            1.0f / (pDrcCtx->NextData.AEData.L2S_Ratio * drc_gain);
    }
    else {
        out->dyn.drcProc.hw_drcT_drcGain_minLimit =
            interpolation_f32(paut->dyn[ilow].drcProc.hw_drcT_drcGain_minLimit,
                paut->dyn[ihigh].drcProc.hw_drcT_drcGain_minLimit, uratio);
    }

    // // clip hw_drc_compsGain_minLimit
    // if (pDrcCtx->NextData.AEData.LongFrmMode)
    //     out->dyn.drcProc.hw_drcT_drcGain_minLimit = 1;

    // get sw_drc_compres_y
    if (out->dyn.drcProc.sw_drcT_drcCurve_mode == drc_vendorDefault_mode) {
        float luma2[DRC_CURVE_LEN] = {0.0f,     1024.0f,  2048.0f,  3072.0f,  4096.0f,  5120.0f,
                                      6144.0f,  7168.0f,  8192.0f,  10240.0f, 12288.0f, 14336.0f,
                                      16384.0f, 18432.0f, 20480.0f, 22528.0f, 24576.0f};
        float dstbits              = (float)(ISP_RAW_BIT << cmps_fix_bit);
        float validbits            = dstbits - offsetbits;
        float curveparam  = (float)(validbits - 0.0f) / (hdrvalidbits - validbits + 0.0156f);
        float curveparam2 = validbits * (1.0f + curveparam);
        float curveparam3 = hdrvalidbits * curveparam;
        for (int i = 0; i < DRC_CURVE_LEN; ++i) {
            float tmp                                     = luma2[i] * hdrvalidbits / 24576.0f;
            float curveTable                              = tmp * curveparam2 / (tmp + curveparam3);
            out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[i] =
                ((unsigned short)(curveTable)) >> cmps_fixbit_mode;
        }
    } else if (out->dyn.drcProc.sw_drcT_drcCurve_mode == drc_usrConfig_mode) {
        for (int i = 0; i < DRC_CURVE_LEN; ++i)
            out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[i] =
                interpolation_f32(paut->dyn[ilow].drcProc.hw_drcT_hdr2Sdr_curve[i],
                    paut->dyn[ihigh].drcProc.hw_drcT_hdr2Sdr_curve[i], uratio);
    }
}
#endif
XCamReturn
algo_drc_SetAttrib
(
    RkAiqAlgoContext* ctx,
    drc_api_attrib_t *attr
) {
    if(ctx == NULL || attr == NULL) {
        LOGE_ATMO("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    DrcContext_t* pDrcCtx = (DrcContext_t*)ctx;
    drc_api_attrib_t* drc_attrib = pDrcCtx->drc_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ATMO("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    drc_attrib->opMode = attr->opMode;
    drc_attrib->en = attr->en;
    drc_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO)
        drc_attrib->stAuto = attr->stAuto;
    else if (attr->opMode == RK_AIQ_OP_MODE_MANUAL)
        drc_attrib->stMan = attr->stMan;
    else {
        LOGW_ATMO("wrong mode: %d\n", attr->opMode);
    }

    pDrcCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_drc_GetAttrib
(
    RkAiqAlgoContext* ctx,
    drc_api_attrib_t* attr
)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_ATMO("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    DrcContext_t* pDrcCtx = (DrcContext_t*)ctx;
    drc_api_attrib_t* drc_attrib = pDrcCtx->drc_attrib;

#if 0
    if (drc_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ATMO("not auto mode: %d", drc_attrib->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }
#endif

    attr->opMode = drc_attrib->opMode;
    attr->en = drc_attrib->en;
    attr->bypass = drc_attrib->bypass;
    memcpy(&attr->stAuto, &drc_attrib->stAuto, sizeof(drc_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_ALGO_DRC_VERSION     "v0.1.0"
#define RKISP_ALGO_DRC_VENDOR      "Rockchip"
#define RKISP_ALGO_DRC_DESCRIPTION "Rockchip drc algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescDrc = {
    .common = {
        .version = RKISP_ALGO_DRC_VERSION,
        .vendor  = RKISP_ALGO_DRC_VENDOR,
        .description = RKISP_ALGO_DRC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ADRC,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

//RKAIQ_END_DECLARE
