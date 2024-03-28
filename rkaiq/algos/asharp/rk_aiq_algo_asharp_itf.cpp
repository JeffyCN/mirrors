/*
 * rk_aiq_algo_anr_itf.c
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

#include "asharp/rk_aiq_algo_asharp_itf.h"
#include "asharp/rk_aiq_asharp_algo.h"
#include "rk_aiq_algo_types.h"

RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    void* place_holder[0];
} RkAiqAlgoContext;


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{

    XCamReturn result = XCAM_RETURN_NO_ERROR;
    LOGI_ASHARP("%s: (enter)\n", __FUNCTION__ );

#if 1
    AsharpContext_t* pAsharpCtx = NULL;
    #if(ASHARP_USE_JSON_PARA)
    AsharpResult_t ret = AsharpInit_json(&pAsharpCtx, cfg->calibv2);
    #else
    AsharpResult_t ret = AsharpInit(&pAsharpCtx, cfg->calib);
    #endif
	
    if(ret != ASHARP_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("%s: Initializaion Asharp failed (%d)\n", __FUNCTION__, ret);
    } else {
        *context = (RkAiqAlgoContext *)(pAsharpCtx);
    }
#endif

    LOGI_ASHARP("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ASHARP("%s: (enter)\n", __FUNCTION__ );

#if 1
    AsharpContext_t* pAsharpCtx = (AsharpContext_t*)context;
    AsharpResult_t ret = AsharpRelease(pAsharpCtx);
    if(ret != ASHARP_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("%s: release Asharp failed (%d)\n", __FUNCTION__, ret);
    }
#endif

    LOGI_ASHARP("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ASHARP("%s: (enter)\n", __FUNCTION__ );

#if 1
    AsharpContext_t* pAsharpCtx = (AsharpContext_t *)params->ctx;
    RkAiqAlgoConfigAsharp* pCfgParam = (RkAiqAlgoConfigAsharp*)params;
	pAsharpCtx->prepare_type = params->u.prepare.conf_type;

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
		#if(ASHARP_USE_JSON_PARA)	
		CalibDbV2_SharpV1_t *calibv2_sharp = (CalibDbV2_SharpV1_t *)(CALIBDBV2_GET_MODULE_PTR(pCfgParam->com.u.prepare.calibv2, sharp_v1));
		CalibDbV2_Edgefilter_t *calibv2_edgefilter = (CalibDbV2_Edgefilter_t*)(CALIBDBV2_GET_MODULE_PTR(pCfgParam->com.u.prepare.calibv2, edgefilter_v1));
		if(calibv2_sharp != NULL){
			sharp_calibdbV2_assign(&pAsharpCtx->sharp_v1, calibv2_sharp);
		}
		if(calibv2_edgefilter != NULL){
			edgefilter_calibdbV2_assign(&pAsharpCtx->edgefilter_v1, calibv2_edgefilter);
		}
		#else
        pAsharpCtx->stSharpCalib = *(CalibDb_Sharp_2_t*)(CALIBDB_GET_MODULE_PTR(pCfgParam->com.u.prepare.calib, sharp));
		pAsharpCtx->stEdgeFltCalib = *(CalibDb_EdgeFilter_2_t*)(CALIBDB_GET_MODULE_PTR(pCfgParam->com.u.prepare.calib, edgeFilter));
		#endif
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            return XCAM_RETURN_NO_ERROR;
        }
		pAsharpCtx->isIQParaUpdate = true;
    }

    AsharpResult_t ret = AsharpPrepare(pAsharpCtx, &pCfgParam->stAsharpConfig);
    if(ret != ASHARP_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("%s: config Asharp failed (%d)\n", __FUNCTION__, ret);
    }

#endif

    LOGI_ASHARP("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ASHARP("%s: (enter)\n", __FUNCTION__ );
	AsharpContext_t* pAsharpCtx = (AsharpContext_t *)inparams->ctx;

	RkAiqAlgoPreAsharp* pAsharpPreParams = (RkAiqAlgoPreAsharp*)inparams;
	
	if (pAsharpPreParams->com.u.proc.gray_mode) {
        pAsharpCtx->isGrayMode = true;
    }else {
        pAsharpCtx->isGrayMode = false;
    }
	
    //nothing todo now
    AsharpResult_t ret = AsharpPreProcess(pAsharpCtx);
	if(ret != ASHARP_RET_SUCCESS) {
		result = XCAM_RETURN_ERROR_FAILED;
		LOGE_ASHARP("%s: AsharpPreProcess failed (%d)\n", __FUNCTION__, ret);
	}

    LOGI_ASHARP("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ASHARP("%s: (enter)\n", __FUNCTION__ );

#if 1
    RkAiqAlgoProcAsharp* pAsharpProcParams = (RkAiqAlgoProcAsharp*)inparams;
    RkAiqAlgoProcResAsharp* pAsharpProcResParams = (RkAiqAlgoProcResAsharp*)outparams;
    AsharpContext_t* pAsharpCtx = (AsharpContext_t *)inparams->ctx;
    AsharpExpInfo_t stExpInfo;
    memset(&stExpInfo, 0x00, sizeof(AsharpExpInfo_t));

    LOGD_ASHARP("%s:%d init:%d hdr mode:%d  \n",
                __FUNCTION__, __LINE__,
                inparams->u.proc.init,
                pAsharpProcParams->hdr_mode);

    stExpInfo.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    for(int i = 0; i < 3; i++) {
        stExpInfo.arIso[i] = 50;
        stExpInfo.arAGain[i] = 1.0;
        stExpInfo.arDGain[i] = 1.0;
        stExpInfo.arTime[i] = 0.01;
    }

    if(pAsharpProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        stExpInfo.hdr_mode = 0;
    } else if(pAsharpProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
              || pAsharpProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR ) {
        stExpInfo.hdr_mode = 1;
    } else if(pAsharpProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
              || pAsharpProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR ) {
        stExpInfo.hdr_mode = 2;
    }
	stExpInfo.snr_mode = 0;

#if 1
    RKAiqAecExpInfo_t *preExp = pAsharpProcParams->com.u.proc.preExp;
    RKAiqAecExpInfo_t *curExp = pAsharpProcParams->com.u.proc.curExp;
    if(preExp != NULL && curExp != NULL) {
        stExpInfo.cur_snr_mode = curExp->CISFeature.SNR;
        stExpInfo.pre_snr_mode = preExp->CISFeature.SNR;
        if(pAsharpProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfo.hdr_mode = 0;
            stExpInfo.arAGain[0] = curExp->LinearExp.exp_real_params.analog_gain;
            stExpInfo.arDGain[0] = curExp->LinearExp.exp_real_params.digital_gain;
            stExpInfo.arTime[0] = curExp->LinearExp.exp_real_params.integration_time;
			stExpInfo.arDcgMode[0] = curExp->LinearExp.exp_real_params.dcg_mode;
            stExpInfo.arIso[0] = stExpInfo.arAGain[0] * stExpInfo.arDGain[0] * 50;

			stExpInfo.preAGain[0] = preExp->LinearExp.exp_real_params.analog_gain;
            stExpInfo.preDGain[0] = preExp->LinearExp.exp_real_params.digital_gain;
            stExpInfo.preTime[0] = preExp->LinearExp.exp_real_params.integration_time;
			stExpInfo.preDcgMode[0] = preExp->LinearExp.exp_real_params.dcg_mode;
            stExpInfo.preIso[0] = stExpInfo.preAGain[0] * stExpInfo.preDGain[0] * 50;
            LOGD_ANR("asharp: %s-%d, preExp(%f, %f), curExp(%f, %f)\n",
                    __FUNCTION__, __LINE__,
                    preExp->LinearExp.exp_real_params.analog_gain,
                    preExp->LinearExp.exp_real_params.integration_time,
                    curExp->LinearExp.exp_real_params.analog_gain,
                    curExp->LinearExp.exp_real_params.integration_time);
        } else {
            for(int i = 0; i < 3; i++) {
                stExpInfo.arAGain[i] = curExp->HdrExp[i].exp_real_params.analog_gain;
                stExpInfo.arDGain[i] = curExp->HdrExp[i].exp_real_params.digital_gain;
                stExpInfo.arTime[i] = curExp->HdrExp[i].exp_real_params.integration_time;
				stExpInfo.arDcgMode[i] = curExp->HdrExp[i].exp_real_params.dcg_mode;
                stExpInfo.arIso[i] = stExpInfo.arAGain[i] * stExpInfo.arDGain[i] * 50;

				stExpInfo.preAGain[0] = preExp->HdrExp[i].exp_real_params.analog_gain;
            	stExpInfo.preDGain[0] = preExp->HdrExp[i].exp_real_params.digital_gain;
            	stExpInfo.preTime[0] = preExp->HdrExp[i].exp_real_params.integration_time;
				stExpInfo.preDcgMode[i] = preExp->HdrExp[i].exp_real_params.dcg_mode;
            	stExpInfo.preIso[0] = stExpInfo.preAGain[0] * stExpInfo.preDGain[0] * 50;

                LOGD_ASHARP("%s:%d index:%d again:%f dgain:%f time:%f iso:%d hdr_mode:%d\n",
                            __FUNCTION__, __LINE__,
                            i,
                            stExpInfo.arAGain[i],
                            stExpInfo.arDGain[i],
                            stExpInfo.arTime[i],
                            stExpInfo.arIso[i],
                            stExpInfo.hdr_mode);
            }
        }
    } else {
        LOGE_ASHARP("%s:%d pAEPreRes is NULL, so use default instead \n", __FUNCTION__, __LINE__);
    }
#endif

    AsharpResult_t ret = AsharpProcess(pAsharpCtx, &stExpInfo);
    if(ret != ASHARP_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("%s: processing Asharp failed (%d)\n", __FUNCTION__, ret);
    }

    AsharpGetProcResult(pAsharpCtx, &pAsharpProcResParams->stAsharpProcResult);
#endif

    LOGI_ASHARP("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_ASHARP("%s: (enter)\n", __FUNCTION__ );

    //nothing todo now

    LOGI_ASHARP("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAsharp = {
    .common = {
        .version = RKISP_ALGO_ASHARP_VERSION,
        .vendor  = RKISP_ALGO_ASHARP_VENDOR,
        .description = RKISP_ALGO_ASHARP_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ASHARP,
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
