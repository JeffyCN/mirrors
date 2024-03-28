#include "socket_server.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "socket_server.cpp"

int ProcessCommand(rk_aiq_sys_ctx_t* ctx, RkAiqSocketPacket *dataRecv, RkAiqSocketPacket *dataReply) {
    switch(dataRecv->commandID) {
    case ENUM_ID_AE_SETEXPSWATTR:
        dataReply->commandResult = setExpSwAttr(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    case ENUM_ID_AE_GETEXPSWATTR:
        dataReply->dataSize = sizeof(Uapi_ExpSwAttr_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getExpSwAttr(ctx, dataReply->data);
        break;
    case ENUM_ID_AE_SETLINAEDAYROUTEATTR:
        dataReply->commandResult = setLinAeRouteAttr(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    case ENUM_ID_AE_GETLINAEDAYROUTEATTR:
        dataReply->dataSize = sizeof(Uapi_LinAeRouteAttr_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getLinAeRouteAttr(ctx, dataReply->data);
        break;
    case ENUM_ID_AE_SETHDRAEDAYROUTEATTR:
        dataReply->commandResult = setHdrAeRouteAttr(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    case ENUM_ID_AE_GETHDRAEDAYROUTEATTR:
        dataReply->dataSize = sizeof(Uapi_HdrAeRouteAttr_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getHdrAeRouteAttr(ctx, dataReply->data);
        break;
    case ENUM_ID_AE_QUERYEXPRESINFO:
        dataReply->dataSize = sizeof(Uapi_ExpQueryInfo_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = queryExpResInfo(ctx, dataReply->data);
        break;
    case ENUM_ID_AE_SETLINEXPATTR:
        dataReply->commandResult = setLinExpAttr(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    case ENUM_ID_AE_GETLINEXPATTR:
        dataReply->dataSize = sizeof(Uapi_LinExpAttr_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getLinExpAttr(ctx, dataReply->data);
        break;
    case ENUM_ID_AE_SETHDREXPATTR:
        dataReply->commandResult = setHdrExpAttr(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    case ENUM_ID_AE_GETHDREXPATTR:
        dataReply->dataSize = sizeof(Uapi_HdrExpAttr_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getHdrExpAttr(ctx, dataReply->data);
        break;
    case ENUM_ID_IMGPROC_SETGRAYMODE:
        dataReply->commandResult = setGrayMode(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    case ENUM_ID_IMGPROC_GETGRAYMODE:
        dataReply->dataSize = sizeof(rk_aiq_gray_mode_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        *(int *)dataReply->data = getGrayMode(ctx);
        dataReply->commandResult = 0;
        break;
    case ENUM_ID_ANR_SETBAYERNRATTR: {
        rk_aiq_nr_IQPara_t param;
        param.module_bits = 1 << ANR_MODULE_BAYERNR;
        param.stBayernrPara = *(CalibDb_BayerNr_t *)dataRecv->data;
        dataReply->commandResult = setAnrIQPara(ctx, (char *)&param);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    }
    case ENUM_ID_ANR_SETMFNRATTR: {
        rk_aiq_nr_IQPara_t param;
        param.module_bits = 1 << ANR_MODULE_MFNR;
        //LOGE("MFNR size is %d", sizeof(CalibDb_MFNR_t ));
        param.stMfnrPara = *(CalibDb_MFNR_t *)dataRecv->data;
        dataReply->commandResult = setAnrIQPara(ctx, (char *)&param);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    }
    case ENUM_ID_ANR_SETUVNRATTR: {
        rk_aiq_nr_IQPara_t param;
        param.module_bits = 1 << ANR_MODULE_UVNR;
        param.stUvnrPara = *(CalibDb_UVNR_t *)dataRecv->data;
        dataReply->commandResult = setAnrIQPara(ctx, (char *)&param);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    }
    case ENUM_ID_ANR_SETYNRATTR: {
        rk_aiq_nr_IQPara_t param;
        param.module_bits = 1 << ANR_MODULE_YNR;
        param.stYnrPara = *(CalibDb_YNR_t *)dataRecv->data;
        dataReply->commandResult = setAnrIQPara(ctx, (char *)&param);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    }
    case ENUM_ID_ANR_GETBAYERNRATTR: {
        rk_aiq_nr_IQPara_t param;
        dataReply->dataSize = sizeof(CalibDb_BayerNr_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        param.module_bits = 1 << ANR_MODULE_BAYERNR;
        dataReply->commandResult = getAnrIQPara(ctx, (char*)&param);
        memcpy(dataReply->data, &param.stBayernrPara, sizeof(CalibDb_BayerNr_t));
    }
    break;
    case ENUM_ID_ANR_GETMFNRATTR: {
        rk_aiq_nr_IQPara_t param;
        dataReply->dataSize = sizeof(CalibDb_MFNR_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        param.module_bits = 1 << ANR_MODULE_MFNR;
        dataReply->commandResult = getAnrIQPara(ctx, (char*)&param);
        memcpy(dataReply->data, &param.stMfnrPara, sizeof(CalibDb_MFNR_t));
    }
    break;
    case ENUM_ID_ANR_GETUVNRATTR: {
        rk_aiq_nr_IQPara_t param;
        dataReply->dataSize = sizeof(CalibDb_UVNR_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        param.module_bits = 1 << ANR_MODULE_UVNR;
        dataReply->commandResult = getAnrIQPara(ctx, (char*)&param);
        memcpy(dataReply->data, &param.stUvnrPara, sizeof(CalibDb_UVNR_t));
    }
    break;
    case ENUM_ID_ANR_GETYNRATTR: {
        rk_aiq_nr_IQPara_t param;
        dataReply->dataSize = sizeof(CalibDb_YNR_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        param.module_bits = 1 << ANR_MODULE_YNR;
        dataReply->commandResult = getAnrIQPara(ctx, (char*)&param);
        memcpy(dataReply->data, &param.stYnrPara, sizeof(CalibDb_YNR_t));
    }
    break;
    case ENUM_ID_ANR_SETATTRIB:
        dataReply->commandResult = setAnrAttrib(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    case ENUM_ID_ANR_GETATTRIB:
        dataReply->dataSize = sizeof(rk_aiq_nr_attrib_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getAnrAttrib(ctx, dataReply->data);
        break;
    case ENUM_ID_ANR_SETLUMASFSTRENGTH:
        dataReply->commandResult = setLumaSFStrength(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    case ENUM_ID_ANR_SETLUMATFSTRENGTH:
        dataReply->commandResult = setLumaTFStrength(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    case ENUM_ID_ANR_GETLUMASFSTRENGTH:
        dataReply->dataSize = sizeof(float);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getLumaSFStrength(ctx, dataReply->data);
        break;
    case ENUM_ID_ANR_GETLUMATFSTRENGTH:
        dataReply->dataSize = sizeof(float);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getLumaTFStrength(ctx, dataReply->data);
        break;
    case ENUM_ID_ANR_SETCHROMASFSTRENGTH:
        dataReply->commandResult = setChromaSFStrength(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    case ENUM_ID_ANR_SETCHROMATFSTRENGTH:
        dataReply->commandResult = setChromaTFStrength(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    case ENUM_ID_ANR_GETCHROMASFSTRENGTH:
        dataReply->dataSize = sizeof(float);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getChromaSFStrength(ctx, dataReply->data);
        break;

    case ENUM_ID_ANR_GETCHROMATFSTRENGTH:
        dataReply->dataSize = sizeof(float);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getChromaTFStrength(ctx, dataReply->data);
        break;
    case ENUM_ID_ANR_SETRAWNRSFSTRENGTH:
        dataReply->commandResult = setRawnrSFStrength(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    case ENUM_ID_ANR_GETRAWNRSFSTRENGTH:
        dataReply->dataSize = sizeof(float);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getRawnrSFStrength(ctx, dataReply->data);
        break;
    case ENUM_ID_SHARP_SET_ATTR:
        dataReply->commandResult = setSharpAttrib(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    case ENUM_ID_SHARP_GET_ATTR:
        dataReply->dataSize = sizeof(rk_aiq_sharp_attrib_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getSharpAttrib(ctx, dataReply->data);
        break;
    case ENUM_ID_SHARP_SET_IQPARA: {
        rk_aiq_sharp_IQpara_t param;
        param.module_bits = 1 << ASHARP_MODULE_SHARP;
        param.stSharpPara = *(CalibDb_Sharp_t *)dataRecv->data;
        dataReply->commandResult = setSharpIQPara(ctx, (char *)&param);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
    }
    break;
    case ENUM_ID_SHARP_GET_IQPARA: {
        rk_aiq_sharp_IQpara_t param;
        dataReply->dataSize = sizeof(CalibDb_Sharp_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        param.module_bits = 1 << ASHARP_MODULE_SHARP;
        dataReply->commandResult = getSharpIQPara(ctx, (char *)&param);
        memcpy(dataReply->data, &param.stSharpPara, sizeof(CalibDb_Sharp_t));
    }
    break;
    case ENUM_ID_SHARP_SET_EF_IQPARA: {
        rk_aiq_sharp_IQpara_t param;
        param.module_bits = 1 << ASHARP_MODULE_EDGEFILTER;
        param.stEdgeFltPara = *(CalibDb_EdgeFilter_t *)dataRecv->data;
        //LOGE("THE Edge sieze is%d'",sizeof(CalibDb_EdgeFilter_t));
        dataReply->commandResult = setSharpIQPara(ctx, (char *)&param);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
    }
    break;
    case ENUM_ID_SHARP_GET_EF_IQPARA: {
        rk_aiq_sharp_IQpara_t param;
        dataReply->dataSize = sizeof(CalibDb_EdgeFilter_t);
        LOGE("THE Edge sieze is%d'", sizeof(CalibDb_EdgeFilter_t));
        dataReply->data = (char*)malloc(dataReply->dataSize);
        param.module_bits = 1 << ASHARP_MODULE_EDGEFILTER;
        dataReply->commandResult = getSharpIQPara(ctx, (char *)&param);
        memcpy(dataReply->data, &param.stEdgeFltPara, sizeof(CalibDb_EdgeFilter_t));
    }
    break;
    case ENUM_ID_SHARP_SET_STRENGTH:
        dataReply->commandResult = setSharpStrength(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    case ENUM_ID_SHARP_GET_STRENGTH:
        dataReply->dataSize = sizeof(float);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getSharpStrength(ctx, dataReply->data);
        break;
    case ENUM_ID_SYSCTL_SETCPSLTCFG:
        dataReply->commandResult = setCpsLtCfg(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    case ENUM_ID_SYSCTL_GETCPSLTINFO:
        dataReply->dataSize = sizeof(rk_aiq_cpsl_info_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getCpsLtInfo(ctx, dataReply->data);
        break;
    case ENUM_ID_SYSCTL_QUERYCPSLTCAP:
        dataReply->dataSize = sizeof(rk_aiq_cpsl_cap_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult =  queryCpsLtCap(ctx, dataReply->data);
        break;
    case ENUM_ID_SYSCTL_SETWORKINGMODE:
        dataReply->commandResult = setWorkingModeDyn(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;

    case ENUM_ID_AMERGE_SETATTRIB: {
#if RKAIQ_HAVE_MERGE_V10
        dataReply->commandResult = setMergeAttribV10(ctx, dataRecv->data);
#endif
#if RKAIQ_HAVE_MERGE_V11
        dataReply->commandResult = setMergeAttribV11(ctx, dataRecv->data);
#endif
#if RKAIQ_HAVE_MERGE_V12
        dataReply->commandResult = setMergeAttribV12(ctx, dataRecv->data);
#endif
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    }
    case ENUM_ID_AMERGE_GETATTRIB: {
#if RKAIQ_HAVE_MERGE_V10
        dataReply->dataSize      = sizeof(mergeAttrV10_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getMergeAttribV10(ctx, dataReply->data);
#endif
#if RKAIQ_HAVE_MERGE_V11
        dataReply->dataSize      = sizeof(mergeAttrV11_t);
        dataReply->data          = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getMergeAttribV11(ctx, dataReply->data);
#endif
#if RKAIQ_HAVE_MERGE_V12
        dataReply->dataSize      = sizeof(mergeAttrV12_t);
        dataReply->data          = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getMergeAttribV12(ctx, dataReply->data);
#endif
        break;
    }
    case ENUM_ID_ATMO_SETATTRIB: {
        dataReply->commandResult = setTmoAttrib(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    }
    case ENUM_ID_ATMO_GETATTRIB: {
        dataReply->dataSize = sizeof(atmo_attrib_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult =  getTmoAttrib(ctx, dataReply->data);
        break;
    }
    case ENUM_ID_AGAMMA_SETATTRIB: {
        dataReply->commandResult = setGammaAttrib(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    }
    case ENUM_ID_AGAMMA_GETATTRIB: {
        dataReply->dataSize = sizeof(rk_aiq_gamma_attrib_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult =  getGammaAttrib(ctx, dataReply->data);
        break;
    }
    case ENUM_ID_ADPCC_SETATTRIB: {
        dataReply->commandResult = setDpccAttrib(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    }
    case ENUM_ID_ADPCC_GETATTRIB: {
        dataReply->dataSize = sizeof(rk_aiq_dpcc_attrib_V20_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult =  getDpccAttrib(ctx, dataReply->data);
        break;
    }
    case ENUM_ID_ACCM_SETATTRIB: {
        dataReply->commandResult = setCcmAttrib(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    }
    case ENUM_ID_ACCM_GETATTRIB:
    {
#if RKAIQ_HAVE_CCM_V1
        dataReply->dataSize = sizeof(rk_aiq_ccm_attrib_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult =  getCcmAttrib(ctx, dataReply->data);
#endif
#if RKAIQ_HAVE_CCM_V2 || RKAIQ_HAVE_CCM_V3
        dataReply->dataSize = sizeof(rk_aiq_ccm_v2_attrib_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult =  getCcmAttrib(ctx, dataReply->data);
#endif
        break;
    }
    case ENUM_ID_ACCM_QUERYCCMINFO:
    {
        dataReply->dataSize = sizeof(rk_aiq_ccm_querry_info_t );
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult =  queryCCMInfo(ctx, dataReply->data);
        break;
    }
    case ENUM_ID_AWB_SETATTRIB:
    {
        dataReply->commandResult = setAwbAttrib(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    }
    case ENUM_ID_AWB_GETATTRIB:
    {
        dataReply->dataSize = sizeof(rk_aiq_wb_attrib_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult =  getAwbAttrib(ctx, dataReply->data);
        break;
    }
    case ENUM_ID_AWB_QUERYWBINFO:
    {
        dataReply->dataSize = sizeof(rk_aiq_wb_querry_info_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult =  queryWBInfo(ctx, dataReply->data);
        break;
    }
    case ENUM_ID_CPROC_SETATTRIB:
    {
        dataReply->commandResult = setAcpAttrib(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
    }
    break;
    case ENUM_ID_CPROC_GETATTRIB:
    {
        dataReply->dataSize = sizeof(acp_attrib_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult =  getAcpAttrib(ctx, dataReply->data);
        break;
    }
    case ENUM_ID_SYSCTL_ENQUEUERKRAWFILE:
    {
        dataReply->commandResult = enqueueRkRawFile(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    }
    case ENUM_ID_AIQ_UAPI_SYSCTL_GET3ASTATS:
    {
        dataReply->dataSize = sizeof(rk_aiq_isp_stats_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = get3AStats(ctx, dataReply->data);
        break;
    }
    case ENUM_ID_AIQ_UAPI_SYSCTL_GET3ASTATSBLK:
    {
        dataReply->dataSize = sizeof(rk_aiq_isp_stats_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = get3AStatsBlk(ctx, dataReply->data);
        break;
    }
    case ENUM_ID_AIQ_UAPI2_AWB_WRITEAWBIN:
    {
        dataReply->commandResult = writeAwbIn(ctx, dataRecv->data);
        dataReply->data = NULL;
        dataReply->dataSize = 0;
        break;
    }
    case ENUM_ID_AIQ_UAPI_SYSCTL_GETTOOLSERVER3ASTATS:
    {
        dataReply->dataSize = sizeof(rk_aiq_isp_tool_stats_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getTool3AStats(ctx, dataReply->data);
        break;
    }
    case ENUM_ID_AIQ_UAPI_SYSCTL_GETTOOLSERVER3ASTATSBLK:
    {
        dataReply->dataSize = sizeof(rk_aiq_isp_tool_stats_t);
        dataReply->data = (char*)malloc(dataReply->dataSize);
        dataReply->commandResult = getTool3AStatsBlk(ctx, dataReply->data);
        break;
    }
    default:
        LOGE("AIQ IPC UNKNOWN CMD: %d\n", dataRecv->commandID);
        return -1;
        break;
    }


    dataReply->commandID = dataRecv->commandID;
    if (dataReply->dataSize != 0 ) {
        dataReply->dataHash = RkMSG::MessageParser::MurMurHash(dataReply->data,
                                                               dataReply->dataSize);
    }
    else {
        dataReply->dataHash = 0;
    }
    return 0;
}


