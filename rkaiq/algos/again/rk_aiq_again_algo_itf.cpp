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

#include "again/rk_aiq_again_algo_itf.h"
#include "again/rk_aiq_again_algo.h"
#include "rk_aiq_algo_types.h"

RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    void* place_holder[0];
} RkAiqAlgoContext;


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{

    XCamReturn result = XCAM_RETURN_NO_ERROR;
    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    Again_Context_V1_t* pAgainCtx = NULL;

	#if(AUVNR_USE_JSON_FILE_V1)
	Again_result_t ret = Again_Init_Json_V1(&pAgainCtx, cfg->calibv2);
	#else
    Again_result_t ret = Again_Init_V1(&pAgainCtx, cfg->calib);
	#endif	
    if(ret != AGAIN_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
    } else {
        *context = (RkAiqAlgoContext *)(pAgainCtx);
    }
#endif

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    Again_Context_V1_t* pAgainCtx = (Again_Context_V1_t*)context;
    Again_result_t ret = Again_Release_V1(pAgainCtx);
    if(ret != AGAIN_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: release ANR failed (%d)\n", __FUNCTION__, ret);
    }
#endif

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

    Again_Context_V1_t* pAgainCtx = (Again_Context_V1_t *)params->ctx;
    RkAiqAlgoConfigAgain* pCfgParam = (RkAiqAlgoConfigAgain*)params;
	pAgainCtx->prepare_type = params->u.prepare.conf_type;

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR)
            return XCAM_RETURN_NO_ERROR;
		#if AUVNR_USE_JSON_FILE_V1
		void *pCalibDbV2 = (void*)(pCfgParam->com.u.prepare.calibv2);
		CalibDbV2_MFNR_t* pCalibv2_mfnr_v1 =
            (CalibDbV2_MFNR_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, mfnr_v1));
		pAgainCtx->mfnr_mode_3to1 = pCalibv2_mfnr_v1->TuningPara.mode_3to1;
		pAgainCtx->mfnr_local_gain_en = pCalibv2_mfnr_v1->TuningPara.local_gain_en;
		#else
	  	void *pCalibDb = (void*)(pCfgParam->com.u.prepare.calib);
		CalibDb_MFNR_2_t *pMfnrCalib=
        (CalibDb_MFNR_2_t*)(CALIBDB_GET_MODULE_PTR((void*)pCalibDb, mfnr));
		pAgainCtx->mfnr_mode_3to1 = pMfnrCalib->mode_3to1;
		pAgainCtx->mfnr_local_gain_en = pMfnrCalib->local_gain_en;
		#endif
	}
	
    Again_result_t ret = Again_Prepare_V1(pAgainCtx, &pCfgParam->stAgainConfig);
    if(ret != AGAIN_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: config ANR failed (%d)\n", __FUNCTION__, ret);
    }

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );
    Again_Context_V1_t* pAgainCtx = (Again_Context_V1_t *)inparams->ctx;
	
    RkAiqAlgoPreAgain* pAnrPreParams = (RkAiqAlgoPreAgain*)inparams;

    if (pAnrPreParams->com.u.proc.gray_mode) {
        pAgainCtx->isGrayMode = true;
    }else {
        pAgainCtx->isGrayMode = false;
    }

    Again_result_t ret = Again_PreProcess_V1(pAgainCtx);
    if(ret != AGAIN_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: ANRPreProcess failed (%d)\n", __FUNCTION__, ret);
    }

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    RkAiqAlgoProcAgain* pAgainProcParams = (RkAiqAlgoProcAgain*)inparams;
    RkAiqAlgoProcResAgain* pAgainProcResParams = (RkAiqAlgoProcResAgain*)outparams;
    Again_Context_V1_t* pAgainCtx = (Again_Context_V1_t *)inparams->ctx;
    Again_ExpInfo_t stExpInfo;
    memset(&stExpInfo, 0x00, sizeof(Again_ExpInfo_t));

    LOGD_ANR("%s:%d init:%d hdr mode:%d  \n",
             __FUNCTION__, __LINE__,
             inparams->u.proc.init,
             pAgainProcParams->hdr_mode);

    stExpInfo.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    for(int i = 0; i < 3; i++) {
        stExpInfo.arIso[i] = 50;
        stExpInfo.arAGain[i] = 1.0;
        stExpInfo.arDGain[i] = 1.0;
        stExpInfo.arTime[i] = 0.01;
    }

    if(pAgainProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        stExpInfo.hdr_mode = 0;
    } else if(pAgainProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
              || pAgainProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR ) {
        stExpInfo.hdr_mode = 1;
    } else if(pAgainProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
              || pAgainProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR ) {
        stExpInfo.hdr_mode = 2;
    }
	stExpInfo.snr_mode = 0;

#if 1// TODO Merge
    
	RKAiqAecExpInfo_t *preExp = pAgainProcParams->com.u.proc.preExp;
    RKAiqAecExpInfo_t *curExp = pAgainProcParams->com.u.proc.curExp;

    if(preExp != NULL && curExp != NULL) {
        stExpInfo.cur_snr_mode = curExp->CISFeature.SNR;
        stExpInfo.pre_snr_mode = preExp->CISFeature.SNR;
        if(pAgainProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
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
            LOGD_ANR("anr: %s-%d, preExp(%f, %f, %f, %d, %d), curExp(%f, %f, %f, %d, %d)\n",
                    __FUNCTION__, __LINE__,
                    preExp->LinearExp.exp_real_params.analog_gain,
                    preExp->LinearExp.exp_real_params.integration_time,
                    preExp->LinearExp.exp_real_params.digital_gain,
                    preExp->LinearExp.exp_real_params.dcg_mode,
                    preExp->CISFeature.SNR,
                    curExp->LinearExp.exp_real_params.analog_gain,
                    curExp->LinearExp.exp_real_params.integration_time,
                    curExp->LinearExp.exp_real_params.digital_gain,
                    curExp->LinearExp.exp_real_params.dcg_mode,
                    curExp->CISFeature.SNR);
        } else {
            for(int i = 0; i < 3; i++) {
                stExpInfo.arAGain[i] =  curExp->HdrExp[i].exp_real_params.analog_gain,
                stExpInfo.arDGain[i] = curExp->HdrExp[i].exp_real_params.digital_gain;
                stExpInfo.arTime[i] = curExp->HdrExp[i].exp_real_params.integration_time;
				stExpInfo.arDcgMode[i] = curExp->HdrExp[i].exp_real_params.dcg_mode;
                stExpInfo.arIso[i] = stExpInfo.arAGain[i] * stExpInfo.arDGain[i] * 50;

				stExpInfo.preAGain[i] =  preExp->HdrExp[i].exp_real_params.analog_gain,
                stExpInfo.preDGain[i] = preExp->HdrExp[i].exp_real_params.digital_gain;
                stExpInfo.preTime[i] = preExp->HdrExp[i].exp_real_params.integration_time;
				stExpInfo.preDcgMode[i] = preExp->HdrExp[i].exp_real_params.dcg_mode;
                stExpInfo.preIso[i] = stExpInfo.preAGain[i] * stExpInfo.preDGain[i] * 50;

                LOGD_ANR("%s:%d index:%d again:%f %f dgain:%f %f time:%f %f iso:%d %d hdr_mode:%d  \n",
                         __FUNCTION__, __LINE__,
                         i,
                         stExpInfo.preAGain[i], stExpInfo.arAGain[i],
                         stExpInfo.preDGain[i], stExpInfo.arDGain[i],
                         stExpInfo.preTime[i], stExpInfo.arTime[i],
                         stExpInfo.preIso[i], stExpInfo.arIso[i],
                         stExpInfo.hdr_mode);
            }
        }
    } else {
        LOGE_ANR("%s:%d preExp(%p) or curExp(%p) is NULL, so use default instead \n",
                 __FUNCTION__, __LINE__, preExp, curExp);
    }
    

#if 0
    static int anr_cnt = 0;
    anr_cnt++;

    if(anr_cnt % 50 == 0) {
        for(int i = 0; i < stExpInfo.hdr_mode + 1; i++) {
            printf("%s:%d index:%d again:%f dgain:%f time:%f iso:%d hdr_mode:%d\n",
                   __FUNCTION__, __LINE__,
                   i,
                   stExpInfo.arAGain[i],
                   stExpInfo.arDGain[i],
                   stExpInfo.arTime[i],
                   stExpInfo.arIso[i],
                   stExpInfo.hdr_mode);
        }
    }
#endif

#if 0 //TODO: get mfnr result
	//get mfnr select result	
	RkAiqAlgoProcResAmfnr* pAmfnrProcRes = 
		(RkAiqAlgoProcResAmfnr*)(pAgainProcParams->com.u.proc.proc_res_comb->amfnr_proc_res);
	for(int i=0; i<17; i++){
		pAgainCtx->stAuto.stSelect.fix_x_pos_dehaze[i] = pAmfnrProcRes->stAmfnrProcResult.stSelect.fix_x_pos_dehaze[i];
		pAgainCtx->stAuto.stSelect.noise_sigma_dehaze[i] = pAmfnrProcRes->stAmfnrProcResult.stSelect.noise_sigma_dehaze[i];
	}
#endif
#endif

    Again_result_t ret = Again_Process_V1(pAgainCtx, &stExpInfo);
    if(ret != AGAIN_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
    }

    Again_GetProcResult_V1(pAgainCtx, &pAgainProcResParams->stAgainProcResult);
    outparams->cfg_update = true;
#endif

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

    //nothing todo now

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAgain = {
    .common = {
        .version = RKISP_ALGO_AGAIN_VERSION_V1,
        .vendor  = RKISP_ALGO_AGAIN_VENDOR_V1,
        .description = RKISP_ALGO_AGAIN_DESCRIPTION_V1,
        .type    = RK_AIQ_ALGO_TYPE_AGAIN,
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
