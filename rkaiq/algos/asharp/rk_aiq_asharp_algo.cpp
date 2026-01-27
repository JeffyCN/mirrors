
#include "rk_aiq_asharp_algo.h"
#include "rk_aiq_algo_asharp_itf.h"

RKAIQ_BEGIN_DECLARE

AsharpResult_t AsharpStart(AsharpContext_t *pAsharpCtx) 
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAsharpCtx == NULL) {
        return (ASHARP_RET_NULL_POINTER);
    }

    if ((ASHARP_STATE_RUNNING == pAsharpCtx->eState)
            || (ASHARP_STATE_LOCKED == pAsharpCtx->eState)) {
        return (ASHARP_RET_FAILURE);
    }

    pAsharpCtx->eState = ASHARP_STATE_RUNNING;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (ASHARP_RET_SUCCESS);
}


AsharpResult_t AsharpStop(AsharpContext_t *pAsharpCtx) 
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAsharpCtx == NULL) {
        return (ASHARP_RET_NULL_POINTER);
    }

    if (ASHARP_STATE_LOCKED == pAsharpCtx->eState) {
        return (ASHARP_RET_FAILURE);
    }

    pAsharpCtx->eState = ASHARP_STATE_STOPPED;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (ASHARP_RET_SUCCESS);
}

//anr reconfig
AsharpResult_t AsharpIQParaUpdate(AsharpContext_t *pAsharpCtx)
{
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?
	
	if(pAsharpCtx->isIQParaUpdate){	
		LOGD_ASHARP(" update iq para\n");
		#if(ASHARP_USE_JSON_PARA)
		ASharpConfigSettingParam_json(pAsharpCtx, pAsharpCtx->eParamMode, pAsharpCtx->stExpInfo.snr_mode);
		#else
		ASharpConfigSettingParam(pAsharpCtx, pAsharpCtx->eParamMode, pAsharpCtx->stExpInfo.snr_mode);
		#endif
		pAsharpCtx->isIQParaUpdate = false;
	}
	
    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP_RET_SUCCESS;
}


AsharpResult_t AsharpInit(AsharpContext_t **ppAsharpCtx, CamCalibDbContext_t *pCalibDb)
{
    AsharpContext_t * pAsharpCtx;

    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAsharpCtx = (AsharpContext_t *)malloc(sizeof(AsharpContext_t));
    if(pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    memset(pAsharpCtx, 0x00, sizeof(AsharpContext_t));
    pAsharpCtx->eState = ASHARP_STATE_INITIALIZED;
    *ppAsharpCtx = pAsharpCtx;
    pAsharpCtx->fStrength = 1.0;
	
    //init params config
    pAsharpCtx->eMode = ASHARP_OP_MODE_AUTO;

#if ASHARP_USE_XML_FILE
    //get v1 params from xml file
    pAsharpCtx->stSharpCalib =
        *(CalibDb_Sharp_2_t*)(CALIBDB_GET_MODULE_PTR((void*)pCalibDb, sharp));
    pAsharpCtx->stEdgeFltCalib =
        *(CalibDb_EdgeFilter_2_t*)(CALIBDB_GET_MODULE_PTR((void*)pCalibDb, edgeFilter));
	pAsharpCtx->mfnr_mode_3to1 =
        ((CalibDb_MFNR_2_t*)(CALIBDB_GET_MODULE_PTR((void*)pCalibDb, mfnr)))->mode_3to1;;
#endif

#ifdef RK_SIMULATOR_HW
    //get v2 params from html file
    FILE *fp2 = fopen("rkaiq_asharp_html_params.bin", "rb");
    if(fp2 != NULL) {
        memset(&pAsharpCtx->stAuto.stSharpParam, 0, sizeof(RKAsharp_Sharp_Params_t));
        fread(&pAsharpCtx->stAuto.stSharpParam, 1, sizeof(RKAsharp_Sharp_Params_t), fp2);
        memset(&pAsharpCtx->stAuto.stEdgefilterParams, 0, sizeof(RKAsharp_EdgeFilter_Params_t));
        fread(&pAsharpCtx->stAuto.stEdgefilterParams, 1, sizeof(RKAsharp_EdgeFilter_Params_t), fp2);
        fclose(fp2);
        LOGD_ASHARP("oyyf: %s:(%d) read sharp html param file sucess! \n", __FUNCTION__, __LINE__);
    } else {
        LOGE_ASHARP("oyyf: %s:(%d) read sharp html param file failed! \n", __FUNCTION__, __LINE__);
        return ASHARP_RET_FAILURE;
    }
#endif


#if ASHARP_USE_XML_FILE
	pAsharpCtx->stExpInfo.snr_mode = 0;
	pAsharpCtx->eParamMode = ASHARP_PARAM_MODE_NORMAL;
	ASharpConfigSettingParam(pAsharpCtx, pAsharpCtx->eParamMode, pAsharpCtx->stExpInfo.snr_mode);
#endif

    LOGD_ASHARP("%s(%d): sharp %f %f %f %f %f %f", __FUNCTION__, __LINE__,
                pAsharpCtx->stAuto.stSharpParam.rk_sharpen_params_V1.hratio[0],
                pAsharpCtx->stAuto.stSharpParam.rk_sharpen_params_V1.lratio[0],
                pAsharpCtx->stAuto.stSharpParam.rk_sharpen_params_V1.H_ratio[0],
                pAsharpCtx->stAuto.stSharpParam.rk_sharpen_params_V1.M_ratio[0],
                pAsharpCtx->stAuto.stSharpParam.rk_sharpen_params_V1.hbf_gain[0],
                pAsharpCtx->stAuto.stSharpParam.rk_sharpen_params_V1.hbf_ratio[0]);

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP_RET_SUCCESS;
}



AsharpResult_t AsharpInit_json(AsharpContext_t **ppAsharpCtx, CamCalibDbV2Context_t *pCalibDbV2)
{
    AsharpContext_t * pAsharpCtx;

    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAsharpCtx = (AsharpContext_t *)malloc(sizeof(AsharpContext_t));
    if(pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

    memset(pAsharpCtx, 0x00, sizeof(AsharpContext_t));
    pAsharpCtx->eState = ASHARP_STATE_INITIALIZED;
    *ppAsharpCtx = pAsharpCtx;
    pAsharpCtx->fStrength = 1.0;
	
    //init params config
    pAsharpCtx->eMode = ASHARP_OP_MODE_AUTO;

#if ASHARP_USE_JSON_PARA
    //get v1 params from xml file

	CalibDbV2_SharpV1_t* calibv2_sharp_v1 =
            (CalibDbV2_SharpV1_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, sharp_v1));
	sharp_calibdbV2_assign(&pAsharpCtx->sharp_v1, calibv2_sharp_v1);

	CalibDbV2_Edgefilter_t* calibv2_edgefilter_v1 =
            (CalibDbV2_Edgefilter_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, edgefilter_v1));	 	
	edgefilter_calibdbV2_assign(&pAsharpCtx->edgefilter_v1, calibv2_edgefilter_v1);
	
	CalibDbV2_MFNR_t* calibv2_mfnr_v1 =
            (CalibDbV2_MFNR_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, mfnr_v1));
    pAsharpCtx->mfnr_mode_3to1 = calibv2_mfnr_v1->TuningPara.mode_3to1;
#endif

#ifdef RK_SIMULATOR_HW
    //get v2 params from html file
    FILE *fp2 = fopen("rkaiq_asharp_html_params.bin", "rb");
    if(fp2 != NULL) {
        memset(&pAsharpCtx->stAuto.stSharpParam, 0, sizeof(RKAsharp_Sharp_Params_t));
        fread(&pAsharpCtx->stAuto.stSharpParam, 1, sizeof(RKAsharp_Sharp_Params_t), fp2);
        memset(&pAsharpCtx->stAuto.stEdgefilterParams, 0, sizeof(RKAsharp_EdgeFilter_Params_t));
        fread(&pAsharpCtx->stAuto.stEdgefilterParams, 1, sizeof(RKAsharp_EdgeFilter_Params_t), fp2);
        fclose(fp2);
        LOGD_ASHARP("oyyf: %s:(%d) read sharp html param file sucess! \n", __FUNCTION__, __LINE__);
    } else {
        LOGE_ASHARP("oyyf: %s:(%d) read sharp html param file failed! \n", __FUNCTION__, __LINE__);
        return ASHARP_RET_FAILURE;
    }
#endif


#if ASHARP_USE_JSON_PARA
	pAsharpCtx->stExpInfo.snr_mode = 0;
	pAsharpCtx->eParamMode = ASHARP_PARAM_MODE_NORMAL;
	ASharpConfigSettingParam_json(pAsharpCtx, pAsharpCtx->eParamMode, pAsharpCtx->stExpInfo.snr_mode);
#endif

    LOGD_ASHARP("%s(%d): sharp %f %f %f %f %f %f", __FUNCTION__, __LINE__,
                pAsharpCtx->stAuto.stSharpParam.rk_sharpen_params_V1.hratio[0],
                pAsharpCtx->stAuto.stSharpParam.rk_sharpen_params_V1.lratio[0],
                pAsharpCtx->stAuto.stSharpParam.rk_sharpen_params_V1.H_ratio[0],
                pAsharpCtx->stAuto.stSharpParam.rk_sharpen_params_V1.M_ratio[0],
                pAsharpCtx->stAuto.stSharpParam.rk_sharpen_params_V1.hbf_gain[0],
                pAsharpCtx->stAuto.stSharpParam.rk_sharpen_params_V1.hbf_ratio[0]);

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP_RET_SUCCESS;
}


//anr release
AsharpResult_t AsharpRelease(AsharpContext_t *pAsharpCtx)
{
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);
	AsharpResult_t result = ASHARP_RET_SUCCESS;
	
    if(pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_NULL_POINTER;
    }

	result = AsharpStop(pAsharpCtx);
    if (result != ASHARP_RET_SUCCESS) {
        LOGE_ASHARP( "%s: AsharpStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((ASHARP_STATE_RUNNING == pAsharpCtx->eState)
            || (ASHARP_STATE_LOCKED == pAsharpCtx->eState)) {
        return (ASHARP_RET_BUSY);
    }

	sharp_calibdbV2_free(&pAsharpCtx->sharp_v1);
	edgefilter_calibdbV2_free(&pAsharpCtx->edgefilter_v1);
	
    memset(pAsharpCtx, 0x00, sizeof(AsharpContext_t));
    free(pAsharpCtx);

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP_RET_SUCCESS;
}

//anr config
AsharpResult_t AsharpPrepare(AsharpContext_t *pAsharpCtx, AsharpConfig_t* pAsharpConfig)
{
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_INVALID_PARM;
    }

    if(pAsharpConfig == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_INVALID_PARM;
    }

    //pAsharpCtx->eMode = pAsharpConfig->eMode;
    //pAsharpCtx->eState = pAsharpConfig->eState;
	if(!!(pAsharpCtx->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)){	
		AsharpIQParaUpdate(pAsharpCtx);
	}
		
	AsharpStart(pAsharpCtx);
	
    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP_RET_SUCCESS;
}

//anr reconfig
AsharpResult_t AsharpReConfig(AsharpContext_t *pAsharpCtx, AsharpConfig_t* pAsharpConfig)
{
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP_RET_SUCCESS;
}


//anr preprocess
AsharpResult_t AsharpPreProcess(AsharpContext_t *pAsharpCtx)
{
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

	AsharpIQParaUpdate(pAsharpCtx);
	
    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP_RET_SUCCESS;
}

//anr process
AsharpResult_t AsharpProcess(AsharpContext_t *pAsharpCtx, AsharpExpInfo_t *pExpInfo)
{
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);
	AsharpParamMode_t mode = ASHARP_PARAM_MODE_INVALID;
	
    if(pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_INVALID_PARM;
    }

	if(pAsharpCtx->eState != ASHARP_STATE_RUNNING){
        return ASHARP_RET_SUCCESS;
	}

	AsharpParamModeProcess(pAsharpCtx, pExpInfo, &mode);
	pExpInfo->mfnr_mode_3to1 = pAsharpCtx->mfnr_mode_3to1;
    if(pExpInfo->mfnr_mode_3to1){
        pExpInfo->snr_mode = pExpInfo->pre_snr_mode;
    }else{
        pExpInfo->snr_mode = pExpInfo->cur_snr_mode;
    }

	#if ASHARP_USE_XML_FILE
	if(pExpInfo->snr_mode != pAsharpCtx->stExpInfo.snr_mode || pAsharpCtx->eParamMode != mode){
		LOGD_ASHARP(" sharp mode:%d snr_mode:%d\n", mode, pExpInfo->snr_mode);
		pAsharpCtx->eParamMode = mode;
		#if(ASHARP_USE_JSON_PARA)
		ASharpConfigSettingParam_json(pAsharpCtx, pAsharpCtx->eParamMode, pExpInfo->snr_mode);	
		#else
		ASharpConfigSettingParam(pAsharpCtx, pAsharpCtx->eParamMode, pExpInfo->snr_mode);	
		#endif
	}
	#endif
    memcpy(&pAsharpCtx->stExpInfo, pExpInfo, sizeof(AsharpExpInfo_t));

    if(pAsharpCtx->eMode == ASHARP_OP_MODE_AUTO) {
        //select param
        select_sharpen_params_by_ISO(&pAsharpCtx->stAuto.stSharpParam, &pAsharpCtx->stAuto.stSharpParamSelect, pExpInfo);
        select_edgefilter_params_by_ISO(&pAsharpCtx->stAuto.stEdgefilterParams, &pAsharpCtx->stAuto.stEdgefilterParamSelect, pExpInfo);
    } else if(pAsharpCtx->eMode == ASHARP_OP_MODE_MANUAL) {
        //TODO
    }

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP_RET_SUCCESS;

}

//anr get result
AsharpResult_t AsharpGetProcResult(AsharpContext_t *pAsharpCtx, AsharpProcResult_t* pAsharpResult)
{
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_INVALID_PARM;
    }

    if(pAsharpResult == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_INVALID_PARM;
    }

    if(pAsharpCtx->eMode == ASHARP_OP_MODE_AUTO) {
        pAsharpResult->sharpEn = pAsharpCtx->stAuto.sharpEn;
        pAsharpResult->edgeFltEn = pAsharpCtx->stAuto.edgeFltEn;
        pAsharpResult->stSharpParamSelect = pAsharpCtx->stAuto.stSharpParamSelect;
        pAsharpResult->stEdgefilterParamSelect = pAsharpCtx->stAuto.stEdgefilterParamSelect;

    } else if(pAsharpCtx->eMode == ASHARP_OP_MODE_MANUAL) {
        //TODO
        pAsharpResult->sharpEn = pAsharpCtx->stManual.sharpEn;
        pAsharpResult->stSharpParamSelect = pAsharpCtx->stManual.stSharpParamSelect;
        pAsharpResult->edgeFltEn = pAsharpCtx->stManual.edgeFltEn;
        pAsharpResult->stEdgefilterParamSelect = pAsharpCtx->stManual.stEdgefilterParamSelect;
	  pAsharpCtx->fStrength = 1.0;
    }

    rk_Sharp_fix_transfer(&pAsharpResult->stSharpParamSelect, &pAsharpResult->stSharpFix, pAsharpCtx->fStrength);
    edgefilter_fix_transfer(&pAsharpResult->stEdgefilterParamSelect, &pAsharpResult->stEdgefltFix,  pAsharpCtx->fStrength);
    pAsharpResult->stSharpFix.stSharpFixV1.sharp_en = pAsharpResult->sharpEn ;
    pAsharpResult->stEdgefltFix.edgeflt_en = pAsharpResult->edgeFltEn;
#if ASHARP_FIX_VALUE_PRINTF
    Asharp_fix_Printf(&pAsharpResult->stSharpFix.stSharpFixV1, &pAsharpResult->stEdgefltFix);
#endif

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP_RET_SUCCESS;
}

AsharpResult_t Asharp_fix_Printf(RKAsharp_Sharp_HW_Fix_t  * pSharpCfg, RKAsharp_Edgefilter_Fix_t *pEdgefltCfg)
{
    LOGD_ASHARP("%s:(%d) enter \n", __FUNCTION__, __LINE__);
    int i = 0;
    AsharpResult_t res = ASHARP_RET_SUCCESS;

    if(pSharpCfg == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_INVALID_PARM;
    }

    if(pEdgefltCfg == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_RET_INVALID_PARM;
    }

    //0x0080
    LOGD_ASHARP("(0x0080) alpha_adp_en:%d yin_flt_en:%d edge_avg_en:%d\n",
                pEdgefltCfg->alpha_adp_en,
                pSharpCfg->yin_flt_en,
                pSharpCfg->edge_avg_en);


    //0x0084
    LOGD_ASHARP("(0x0084) hbf_ratio:%d ehf_th:%d pbf_ratio:%d\n",
                pSharpCfg->hbf_ratio,
                pSharpCfg->ehf_th,
                pSharpCfg->pbf_ratio);

    //0x0088
    LOGD_ASHARP("(0x0088) edge_thed:%d dir_min:%d smoth_th4:%d\n",
                pEdgefltCfg->edge_thed,
                pEdgefltCfg->dir_min,
                pEdgefltCfg->smoth_th4);

    //0x008c
    LOGD_ASHARP("(0x008c) l_alpha:%d g_alpha:%d \n",
                pEdgefltCfg->l_alpha,
                pEdgefltCfg->g_alpha);


    //0x0090
    for(i = 0; i < 3; i++) {
        LOGD_ASHARP("(0x0090) pbf_k[%d]:%d  \n",
                    i, pSharpCfg->pbf_k[i]);
    }



    //0x0094 - 0x0098
    for(i = 0; i < 6; i++) {
        LOGD_ASHARP("(0x0094 - 0x0098) mrf_k[%d]:%d  \n",
                    i, pSharpCfg->mrf_k[i]);
    }


    //0x009c -0x00a4
    for(i = 0; i < 12; i++) {
        LOGD_ASHARP("(0x009c -0x00a4) mbf_k[%d]:%d  \n",
                    i, pSharpCfg->mbf_k[i]);
    }


    //0x00a8 -0x00ac
    for(i = 0; i < 6; i++) {
        LOGD_ASHARP("(0x00a8 -0x00ac) hrf_k[%d]:%d  \n",
                    i, pSharpCfg->hrf_k[i]);
    }


    //0x00b0
    for(i = 0; i < 3; i++) {
        LOGD_ASHARP("(0x00b0) hbf_k[%d]:%d  \n",
                    i, pSharpCfg->hbf_k[i]);
    }


    //0x00b4
    for(i = 0; i < 3; i++) {
        LOGD_ASHARP("(0x00b4) eg_coef[%d]:%d  \n",
                    i, pEdgefltCfg->eg_coef[i]);
    }

    //0x00b8
    for(i = 0; i < 3; i++) {
        LOGD_ASHARP("(0x00b8) eg_smoth[%d]:%d  \n",
                    i, pEdgefltCfg->eg_smoth[i]);
    }


    //0x00bc - 0x00c0
    for(i = 0; i < 6; i++) {
        LOGD_ASHARP("(0x00bc - 0x00c0) eg_gaus[%d]:%d  \n",
                    i, pEdgefltCfg->eg_gaus[i]);
    }


    //0x00c4 - 0x00c8
    for(i = 0; i < 6; i++) {
        LOGD_ASHARP("(0x00c4 - 0x00c8) dog_k[%d]:%d  \n",
                    i, pEdgefltCfg->dog_k[i]);
    }


    //0x00cc - 0x00d0
    for(i = 0; i < 6; i++) {
        LOGD_ASHARP("(0x00cc - 0x00d0) lum_point[%d]:%d  \n",
                    i, pSharpCfg->lum_point[i]);
    }

    //0x00d4
    LOGD_ASHARP("(0x00d4) pbf_shf_bits:%d  mbf_shf_bits:%d hbf_shf_bits:%d\n",
                pSharpCfg->pbf_shf_bits,
                pSharpCfg->mbf_shf_bits,
                pSharpCfg->hbf_shf_bits);


    //0x00d8 - 0x00dc
    for(i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x00d8 - 0x00dc) pbf_sigma[%d]:%d  \n",
                    i, pSharpCfg->pbf_sigma[i]);
    }

    //0x00e0 - 0x00e4
    for(i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x00e0 - 0x00e4) lum_clp_m[%d]:%d  \n",
                    i, pSharpCfg->lum_clp_m[i]);
    }

    //0x00e8 - 0x00ec
    for(i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x00e8 - 0x00ec) lum_min_m[%d]:%d  \n",
                    i, pSharpCfg->lum_min_m[i]);
    }

    //0x00f0 - 0x00f4
    for(i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x00f0 - 0x00f4) mbf_sigma[%d]:%d  \n",
                    i, pSharpCfg->mbf_sigma[i]);
    }

    //0x00f8 - 0x00fc
    for(i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x00f8 - 0x00fc) lum_clp_h[%d]:%d  \n",
                    i, pSharpCfg->lum_clp_h[i]);
    }

    //0x0100 - 0x0104
    for(i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x0100 - 0x0104) hbf_sigma[%d]:%d  \n",
                    i, pSharpCfg->hbf_sigma[i]);
    }

    //0x0108 - 0x010c
    for(i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x0108 - 0x010c) edge_lum_thed[%d]:%d  \n",
                    i, pEdgefltCfg->edge_lum_thed[i]);
    }

    //0x0110 - 0x0114
    for(i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x0110 - 0x0114) clamp_pos[%d]:%d  \n",
                    i, pEdgefltCfg->clamp_pos[i]);
    }

    //0x0118 - 0x011c
    for(i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x0118 - 0x011c) clamp_neg[%d]:%d  \n",
                    i, pEdgefltCfg->clamp_neg[i]);
    }

    //0x0120 - 0x0124
    for(i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x0120 - 0x0124) detail_alpha[%d]:%d  \n",
                    i, pEdgefltCfg->detail_alpha[i]);
    }

    //0x0128
    LOGD_ASHARP("(0x0128) rfl_ratio:%d  rfh_ratio:%d\n",
                pSharpCfg->rfl_ratio, pSharpCfg->rfh_ratio);

    // mf/hf ratio

    //0x012C
    LOGD_ASHARP("(0x012C) m_ratio:%d  h_ratio:%d\n",
                pSharpCfg->m_ratio, pSharpCfg->h_ratio);

    LOGD_ASHARP("%s:(%d) exit \n", __FUNCTION__, __LINE__);

    return res;
}

AsharpResult_t ASharpConfigSettingParam(AsharpContext_t *pAsharpCtx, AsharpParamMode_t param_mode, int snr_mode)
{
	char mode_name[CALIBDB_MAX_MODE_NAME_LENGTH];
	char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
	memset(mode_name, 0x00, sizeof(mode_name));
	memset(snr_name, 0x00, sizeof(snr_name));

	if(pAsharpCtx == NULL) {
        	LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        	return ASHARP_RET_NULL_POINTER;
    	}

	
	if(param_mode == ASHARP_PARAM_MODE_NORMAL){
		sprintf(mode_name, "%s", "normal");
	}else if(param_mode == ASHARP_PARAM_MODE_HDR){
		sprintf(mode_name, "%s", "hdr");
	}else if(param_mode == ASHARP_PARAM_MODE_GRAY){
		sprintf(mode_name, "%s", "gray");
	}else{
		LOGE_ASHARP("%s(%d): not support mode cell name!\n", __FUNCTION__, __LINE__);
		sprintf(mode_name, "%s", "normal");
	}

	if(snr_mode == 1){
		sprintf(snr_name, "%s", "HSNR");
	}else if(snr_mode == 0){
		sprintf(snr_name, "%s", "LSNR");
	}else{
		LOGE_ASHARP("%s(%d): not support snr mode!\n", __FUNCTION__, __LINE__);
		sprintf(snr_name, "%s", "LSNR");
	}

	pAsharpCtx->stAuto.sharpEn = pAsharpCtx->stSharpCalib.enable;
	sharp_config_setting_param_v1(&pAsharpCtx->stAuto.stSharpParam.rk_sharpen_params_V1, &pAsharpCtx->stSharpCalib, mode_name,  snr_name);
	pAsharpCtx->stAuto.edgeFltEn = pAsharpCtx->stEdgeFltCalib.enable;
	edgefilter_config_setting_param(&pAsharpCtx->stAuto.stEdgefilterParams, &pAsharpCtx->stEdgeFltCalib, mode_name, snr_name);

	return ASHARP_RET_SUCCESS;
}

AsharpResult_t AsharpParamModeProcess(AsharpContext_t *pAsharpCtx, AsharpExpInfo_t *pExpInfo, AsharpParamMode_t *mode)
{
	AsharpResult_t res  = ASHARP_RET_SUCCESS;
		
	if(pAsharpCtx == NULL) {
    	LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
    	return ASHARP_RET_NULL_POINTER;
	}

    *mode = pAsharpCtx->eParamMode;

	if(pAsharpCtx->isGrayMode){
		*mode = ASHARP_PARAM_MODE_GRAY;
	}else if(pExpInfo->hdr_mode == 0){
		*mode = ASHARP_PARAM_MODE_NORMAL;
	}else if(pExpInfo->hdr_mode >= 1){
		*mode = ASHARP_PARAM_MODE_HDR;
	}else{
		*mode = ASHARP_PARAM_MODE_NORMAL;
	}

	return res;
}


AsharpResult_t ASharpConfigSettingParam_json(AsharpContext_t *pAsharpCtx, AsharpParamMode_t param_mode, int snr_mode)
{
	char mode_name[CALIBDB_MAX_MODE_NAME_LENGTH];
	char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
	memset(mode_name, 0x00, sizeof(mode_name));
	memset(snr_name, 0x00, sizeof(snr_name));

	if(pAsharpCtx == NULL) {
        	LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        	return ASHARP_RET_NULL_POINTER;
    	}

	
	if(param_mode == ASHARP_PARAM_MODE_NORMAL){
		sprintf(mode_name, "%s", "normal");
	}else if(param_mode == ASHARP_PARAM_MODE_HDR){
		sprintf(mode_name, "%s", "hdr");
	}else if(param_mode == ASHARP_PARAM_MODE_GRAY){
		sprintf(mode_name, "%s", "gray");
	}else{
		LOGE_ASHARP("%s(%d): not support mode cell name!\n", __FUNCTION__, __LINE__);
		sprintf(mode_name, "%s", "normal");
	}

	if(snr_mode == 1){
		sprintf(snr_name, "%s", "HSNR");
	}else if(snr_mode == 0){
		sprintf(snr_name, "%s", "LSNR");
	}else{
		LOGE_ASHARP("%s(%d): not support snr mode!\n", __FUNCTION__, __LINE__);
		sprintf(snr_name, "%s", "LSNR");
	}

	pAsharpCtx->stAuto.sharpEn = pAsharpCtx->sharp_v1.TuningPara.enable;
	sharp_config_setting_param_v1_json(&pAsharpCtx->stAuto.stSharpParam.rk_sharpen_params_V1, &pAsharpCtx->sharp_v1, mode_name,  snr_name);
	pAsharpCtx->stAuto.edgeFltEn = pAsharpCtx->edgefilter_v1.TuningPara.enable;
	edgefilter_config_setting_param_json(&pAsharpCtx->stAuto.stEdgefilterParams, &pAsharpCtx->edgefilter_v1, mode_name, snr_name);

	return ASHARP_RET_SUCCESS;
}

RKAIQ_END_DECLARE

