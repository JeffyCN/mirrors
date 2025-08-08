/*
 *  Copyright (c) 2021 Rockchip Corporation
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

#include "rk_aiq_user_api2_custom2_awb.h"
#include "RkAiqCalibDbV2Helper.h"
#include "awb/rk_aiq_algo_awb_itf.h"
#include "awb/rk_aiq_algo_camgroup_awb_itf.h"

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

RKAIQ_BEGIN_DECLARE

#define RKISP_ALGO_AWB_DEMO_VERSION     "v0.0.1"
#define RKISP_ALGO_AWB_DEMO_VENDOR      "Rockchip"
#define RKISP_ALGO_AWB_DEMO_DESCRIPTION "Rockchip Custom Awb"

/* instance was created by AIQ framework when rk_aiq_uapi_sysctl_regLib called */

static XCamReturn AwbDemoCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOGD_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = new_awb_context(context);
    RkAiqAwbAlgoContext2 *ctx = (RkAiqAwbAlgoContext2*)(*context);
    if (ctx == NULL) {
        printf( "%s: create awb context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    //xcam_get_awb_log_level(&ctx->log_level, &ctx->log_sub_modules);
    ret = init_awb_context(context,cfg);
    RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);
    if (cfg->isGroupMode) {
        AlgoCtxInstanceCfgCamGroup* grpCfg = (AlgoCtxInstanceCfgCamGroup*)cfg;
        memcpy(ctx->camIdArray, grpCfg->camIdArray, sizeof(ctx->camIdArray));
        ctx->camIdArrayLen = grpCfg->camIdArrayLen;
        ctx->isGroupMode = true;
    } else {
        ctx->camIdArrayLen = 1;
        ctx->isGroupMode = false;
    }
    if (cfg->cbs!=NULL) {
        memcpy(&ctx->cbs, cfg->cbs, sizeof(rk_aiq_customeAwb_cbs_t));
        ctx->isRkCb = false;
    }else{
        ctx->isRkCb = true;
    }
    LOGD_AWB_SUBM(0xff, "%s EXIT", __func__);
    return ret;
}

static XCamReturn AwbDemoDestroyCtx(RkAiqAlgoContext *context)
{
    LOGD_AWB_SUBM(0xff, "%s ENTER", __func__);
    RkAiqAwbAlgoContext2* context2 = (RkAiqAwbAlgoContext2*)context;
    if(context2 == NULL)
        return XCAM_RETURN_NO_ERROR;
    if ((!context2->isRkCb)&&context2->cbs.pfn_awb_exit) {
        context2->cbs.pfn_awb_exit(context2->aiq_ctx);
        context2->cutomAwbInit = false;
    }
    XCamReturn ret = destroy_awb_context(context);
    LOGD_AWB_SUBM(0xff, "%s EXIT", __func__);
    return ret;
}


static XCamReturn AwbDemoPrepare(RkAiqAlgoCom* params)
{
    LOGD_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAwbAlgoContext2* algo_ctx = (RkAiqAwbAlgoContext2*)params->ctx;
    algo_ctx->camId = params->cid;
    ret = awb_prepare(params);
    RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);
    if ((!algo_ctx->isRkCb)&&(!algo_ctx->cutomAwbInit)){
        algo_ctx->customRes.IsConverged = false;
        algo_ctx->customRes.awb_gain_algo.rgain =1.0;
        algo_ctx->customRes.awb_gain_algo.bgain =1.0;
        algo_ctx->customRes.awb_gain_algo.grgain =1.0;
        algo_ctx->customRes.awb_gain_algo.gbgain =1.0;
        algo_ctx->customRes.awb_smooth_factor = 0.5;
        initCustAwbHwConfig(params->ctx,&algo_ctx->customRes.awbHwConfig);
        if(algo_ctx->cbs.pfn_awb_init){
            algo_ctx->cbs.pfn_awb_init(algo_ctx->aiq_ctx);
        }
        algo_ctx->cutomAwbInit = true;
    }

    if(0/*params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGERES || params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGEMODE */){
        uint32_t cmdctrl=0;
        int pValue=0;
        if ((!algo_ctx->isRkCb)&&algo_ctx->cbs.pfn_awb_ctrl){
            algo_ctx->cbs.pfn_awb_ctrl(algo_ctx->aiq_ctx,  cmdctrl, (void*)(&pValue));
        }
    }
    LOGD_AWB_SUBM(0xff, "%s EXIT", __func__);
    if(!algo_ctx->isRkCb){
        //meas custom callback
        // todo, modify the wpc to gray world mode
    }
    return ret;
}

static XCamReturn AwbDemoPreProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);

    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);

    return ret;
}
static
void _customAwbRes2rkAwbRes( RkAiqAlgoProcResAwb* rkAwbProcRes,rk_aiq_customeAwb2_results_t* customAwbProcRes)
{

    rkAwbProcRes->awbConverged = customAwbProcRes->IsConverged;
    memcpy(rkAwbProcRes->awb_gain_algo, &customAwbProcRes->awb_gain_algo, sizeof(rk_aiq_wb_gain_t));
    rkAwbProcRes->awb_smooth_factor = customAwbProcRes->awb_smooth_factor;
    rkAwbProcRes->awb_hw_cfg_priv->com=  customAwbProcRes->awbHwConfig;
    rkAwbProcRes->awb_hw_cfg_priv->mode = AWB_CFG_MODE_ThP;
    rkAwbProcRes->awb_cfg_update = true;
    rkAwbProcRes->awb_gain_update= true;
}
static void _rkAwbStats2CustomAwbStats(rk_aiq_customAwb2_stats_t* customStats,
                              const awbStats_stats_t* rkAwb)
{
    memcpy(&customStats->stats,rkAwb,sizeof(awbStats_stats_t));
    customStats->next = NULL;
}

static XCamReturn AwbDemoProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{

    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcAwb* AwbProcParams = (RkAiqAlgoProcAwb*)inparams;
    RkAiqAlgoProcResAwb* AwbProcResParams = (RkAiqAlgoProcResAwb*)outparams;
    RkAiqAwbAlgoContext2* algo_ctx = (RkAiqAwbAlgoContext2*)inparams->ctx;
    LOGI_AWB("cid(%d)----------------------------------------------frame_id (%d)----------------------------------------------\n",
     inparams->cid, inparams->frame_id);
    if(algo_ctx->isRkCb){
        ret= awb_processing2(inparams, outparams);
        return ret;
    }
    if (algo_ctx->isGroupMode) {
        LOGE_AWB("wrong awb mode");
        return ret;
    }

    if(!inparams->u.proc.init) { // init=ture, stats=null
        if (!AwbProcParams->awb_statsBuf_v39) {
            LOGE_AWB("awb stats is null");
            return(XCAM_RETURN_BYPASS);
        }
        if (algo_ctx->cbs.pfn_awb_run){
            rk_aiq_customAwb2_stats_t customStats;
            _rkAwbStats2CustomAwbStats(&customStats,(awbStats_stats_t*)AwbProcParams->awb_statsBuf_v39);
            algo_ctx->cbs.pfn_awb_run(algo_ctx->aiq_ctx,&customStats, &algo_ctx->customRes);
            _customAwbRes2rkAwbRes(AwbProcResParams,&algo_ctx->customRes);

        }
    }else{
        //initialize
        if (algo_ctx->cbs.pfn_awb_run){
            algo_ctx->cbs.pfn_awb_run(algo_ctx->aiq_ctx,NULL, &algo_ctx->customRes);
            _customAwbRes2rkAwbRes(AwbProcResParams,&algo_ctx->customRes);
        }
    }

    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _rkAwbStats2CustomGroupAwbStats2( rk_aiq_singlecam_3a_result_t ** rk_aiq_singlecam_3a_result , int camera_num,rk_aiq_customAwb2_stats_t *customStats)
{
    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_customAwb2_stats_t *awbStat = customStats;
    rk_aiq_customAwb2_stats_t *awbStat2 = customStats;
    for(int i=0;i<camera_num;i++){
        awbStats_stats_priv_t *awb_stats_v39= rk_aiq_singlecam_3a_result[i]->awb.awb_stats_v39;
        if (awb_stats_v39 == NULL) {
           LOGE_AWBGROUP("awb stats is null for %dth camera",i);
           return(XCAM_RETURN_ERROR_FAILED);
        }
        if(i>0){
            if(awbStat->next ==NULL){
                awbStat2 = (rk_aiq_customAwb2_stats_t*)aiq_mallocz(sizeof(rk_aiq_customAwb2_stats_t));
                memset(awbStat2,0,sizeof(rk_aiq_customAwb2_stats_t));
                awbStat->next = awbStat2;
               // LOGE_AWB_SUBM(0xff, "%s  %dth camera:,malloc awbStat:%p", __func__,i,awbStat2);
            }else{
                awbStat2 = awbStat->next;
            }
            awbStat = awbStat->next;

        }
        _rkAwbStats2CustomAwbStats(awbStat2, (awbStats_stats_t*)awb_stats_v39);
        //WriteMeasureResult(xAwbStats->awb_stats_v3x,XCORE_LOG_LEVEL_LOW1+1,i);
    }
    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
    return(ret);

}

static XCamReturn _customAwbStatsRelease( rk_aiq_customAwb2_stats_t *customStats)
{
    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_customAwb2_stats_t *awbStat = customStats->next;
    rk_aiq_customAwb2_stats_t *awbStat2;
    while(awbStat!=NULL){
        awbStat2 = awbStat->next;
        //LOGE_AWB_SUBM(0xff, "%s free awbStat %p ", __func__,awbStat);
        aiq_free(awbStat);
        awbStat = awbStat2;
    }
    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
    return(ret);

}

static XCamReturn _customGruopAwbRes2rkAwbRes(rk_aiq_singlecam_3a_result_t ** rk_aiq_singlecam_3a_result , int camera_num,
                          rk_aiq_customeAwb2_results_t* customAwbProcRes)
{
    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAwbShared_t* xCamAwbProcRes;
    RkAiqAlgoProcResAwbShared_t* procResPara;
    rk_aiq_customeAwb2_single_results_t* customAwbProcRes2 = customAwbProcRes->next;
    for(int i=0; i<camera_num; i++){
        xCamAwbProcRes = &rk_aiq_singlecam_3a_result[i]->awb._awbProcRes;
        procResPara = NULL;
        if (xCamAwbProcRes) {
            procResPara = (RkAiqAlgoProcResAwbShared_t*)xCamAwbProcRes;
            if(procResPara==NULL){
                LOGE_AWBGROUP("_awbProcRes is null for %dth camera",i);
                return(XCAM_RETURN_ERROR_FAILED);
            }
        }else {
           LOGE_AWBGROUP("_awbProcRes is null for %dth camera",i);
           return(XCAM_RETURN_ERROR_FAILED);
        }
        if(i>0 && customAwbProcRes2 != NULL){
            memcpy(rk_aiq_singlecam_3a_result[i]->awb._awbGainParams,&customAwbProcRes2->awb_gain_algo,sizeof(rk_aiq_wb_gain_t));
            memcpy(&procResPara->awb_gain_algo, &customAwbProcRes2->awb_gain_algo, sizeof(rk_aiq_wb_gain_t));
            customAwbProcRes2 = customAwbProcRes2->next;

        }else{
            memcpy(rk_aiq_singlecam_3a_result[i]->awb._awbGainParams,&customAwbProcRes->awb_gain_algo,sizeof(rk_aiq_wb_gain_t));
            memcpy(&procResPara->awb_gain_algo, &customAwbProcRes->awb_gain_algo, sizeof(rk_aiq_wb_gain_t));

        }
        rk_aiq_singlecam_3a_result[i]->awb._awbCfgV39->mode = AWB_CFG_MODE_ThP;
        memcpy(rk_aiq_singlecam_3a_result[i]->awb._awbCfgV39,&customAwbProcRes->awbHwConfig,sizeof(awbStats_cfg_t));
        procResPara->awb_smooth_factor = customAwbProcRes->awb_smooth_factor;
        procResPara->awbConverged = customAwbProcRes->IsConverged;
        //WriteDataForThirdParty(*rk_aiq_singlecam_3a_result[i]->awb._awbCfgV3x,XCORE_LOG_LEVEL_LOW1+1,i);
    }
    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
    return(ret);

}

static XCamReturn AwbDemoGroupProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{

    LOGD_AWB_SUBM(0xff, "%s ENTER --------------------frame%d  ---------------------", __func__,inparams->frame_id);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAwbAlgoContext2* algo_ctx = (RkAiqAwbAlgoContext2*)inparams->ctx;
    if(algo_ctx->isRkCb){
        ret= awbGroupProcessing2(inparams, outparams);
        return ret;
    }

#if RKAIQ_HAVE_AWB_V39
    RkAiqAlgoCamGroupProcIn* AwbProcParams = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* AwbProcResParams = (RkAiqAlgoCamGroupProcOut*)outparams;
    if (!algo_ctx->isGroupMode) {
        LOGE_AWB("wrong awb mode for group");
        return ret;
    }

    if(!inparams->u.proc.init) { // init=ture, stats=null
        rk_aiq_customAwb2_stats_t customStats;
        memset(&customStats,0,sizeof(customStats));
        ret = _rkAwbStats2CustomGroupAwbStats2(AwbProcParams->camgroupParmasArray,AwbProcParams->arraySize, &customStats);
        RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);

        if (algo_ctx->cbs.pfn_awb_run)
            algo_ctx->cbs.pfn_awb_run(algo_ctx->aiq_ctx,
                                     &customStats,
                                     &algo_ctx->customRes
                                  );
        _customAwbStatsRelease(&customStats);

    }else{

        if (algo_ctx->cbs.pfn_awb_run)
             algo_ctx->cbs.pfn_awb_run(algo_ctx->aiq_ctx,
                                      NULL,
                                      &algo_ctx->customRes
                                   );

    }
    // gen part of proc result which is from customRes
    _customGruopAwbRes2rkAwbRes(AwbProcResParams->camgroupParmasArray ,AwbProcResParams->arraySize,&algo_ctx->customRes);
    RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);
#endif
    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AwbDemoPostProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    // RESULT ret = RK_AIQ_RET_SUCCESS;
    // RkAiqAwbAlgoContext* algo_ctx = (RkAiqAwbAlgoContext*)inparams->ctx;


    return XCAM_RETURN_NO_ERROR;
}

//static std::map<rk_aiq_sys_ctx_t*, RkAiqAlgoDescription*> g_customAwb_desc_map;
#if RKAIQ_HAVE_DUMPSYS
static int dump(const RkAiqAlgoCom* inparams, st_string* result)
{

    //aiq_info_dump_title(result, "awb status");
    RkAiqAwbAlgoContext2* algo_ctx = (RkAiqAwbAlgoContext2*)inparams->ctx;
    if(algo_ctx->isRkCb){
        return (awb_dump(inparams, result));
    }

    return 0;
}
#endif

XCamReturn
rk_aiq_uapi2_awb_register(const rk_aiq_sys_ctx_t* ctx, rk_aiq_customeAwb_cbs_t* cbs)
{
    LOGD_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    const rk_aiq_camgroup_ctx_t* group_ctx = NULL;
    rk_aiq_sys_ctx_t* single_ctx = NULL;
    int algoType = 0;
    int algoId = 0;

    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGI_AWB_SUBM(0xff, "group awb");
#ifdef RKAIQ_ENABLE_CAMGROUP
        group_ctx  = (const rk_aiq_camgroup_ctx_t*)ctx;
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            if (!single_ctx && group_ctx->cam_ctxs_array[i]) {
                single_ctx = group_ctx->cam_ctxs_array[i];
                break;
            }
        }
        algoType   = g_RkIspAlgoDescCamgroupAwb.common.type;
        algoId     = g_RkIspAlgoDescCamgroupAwb.common.id;
#endif
    } else {
        LOGI_AWB_SUBM(0xff, "single awb");
        single_ctx = (rk_aiq_sys_ctx_t*)(ctx);
        algoType   = g_RkIspAlgoDescAwb.common.type;
        algoId     = g_RkIspAlgoDescAwb.common.id;
    }

    bool isAwbRgst = rk_aiq_uapi2_sysctl_getAxlibStatus(ctx,
                                                        algoType,
                                                        algoId);
    if (cbs == NULL && isAwbRgst) {
        return XCAM_RETURN_BYPASS;
    } else if (isAwbRgst) {
        // unregister rk awb to register custom awb
        rk_aiq_uapi2_awb_unRegister(ctx);
        LOGK_AWB("unRegister rk awb");
    }

    RkAiqAlgoDescription* desc = NULL;

#if 0
    std::map<rk_aiq_sys_ctx_t*, RkAiqAlgoDescription*>::iterator it =
        g_customAwb_desc_map.find(single_ctx);

    if (it == g_customAwb_desc_map.end()) {
        desc = new RkAiqAlgoDescription();
        g_customAwb_desc_map[single_ctx] = desc;
    } else {
        desc = it->second;
    }
#else
	if(single_ctx->awb_desc == NULL)
		single_ctx->awb_desc = (RkAiqAlgoDescription*)aiq_mallocz(1 * sizeof(RkAiqAlgoDescription));
	desc = single_ctx->awb_desc;
#endif
    desc->common.version = g_RkIspAlgoDescAwb.common.version;
    desc->common.vendor  = g_RkIspAlgoDescAwb.common.vendor;
    desc->common.description = g_RkIspAlgoDescAwb.common.description;
    desc->common.type    = g_RkIspAlgoDescAwb.common.type;
    desc->common.id      = g_RkIspAlgoDescAwb.common.id;
    desc->common.create_context  = AwbDemoCreateCtx;
    desc->common.destroy_context = AwbDemoDestroyCtx;
    desc->prepare = AwbDemoPrepare;
    desc->pre_process = AwbDemoPreProcess;
    desc->processing = AwbDemoProcessing;
    desc->post_process = AwbDemoPostProcess;
#if RKAIQ_HAVE_DUMPSYS
    desc->dump= dump;
#endif
    static RkAiqGrpCondition_t awbGrpCond[] = {
        [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
        [1] = {XCAM_MESSAGE_AE_PRE_RES_OK, 0},
        [2] = {XCAM_MESSAGE_AWB_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
        [3] = {XCAM_MESSAGE_BLC_V32_PROC_RES_OK, 0},
    };
    RkAiqGrpConditions_t awbGrpConds = {grp_conds_array_info(awbGrpCond)};

    struct RkAiqAlgoDesCommExt algoDes[] = {
        { &desc->common, RK_AIQ_CORE_ANALYZE_AWB,  1, 2, 39, awbGrpConds},
        { NULL, RK_AIQ_CORE_ANALYZE_ALL, 0,  0,  0, {0, 0} },
    };

    if (group_ctx) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            if (group_ctx->cam_ctxs_array[i]) {
                isAwbRgst = rk_aiq_uapi2_sysctl_getAxlibStatus(group_ctx->cam_ctxs_array[i],
                                                               algoType, algoId);
                if (isAwbRgst) {
                    continue;
                }

                ret = rk_aiq_uapi2_sysctl_register3Aalgo(group_ctx->cam_ctxs_array[i], &algoDes, NULL);
                if (ret == XCAM_RETURN_ERROR_ANALYZER) {
                    LOGE_AWB_SUBM(0xff, "no current aiq core status, please stop aiq before register custome awb!");
                    return ret;
                } else if (ret != XCAM_RETURN_NO_ERROR) {
                    LOGE_AWB_SUBM(0xff, "awb register error, ret %d", ret);
                    return ret;
                }
            }
        }
#endif
    } else {
        ret = rk_aiq_uapi2_sysctl_register3Aalgo(ctx, &algoDes, cbs);
        if (ret == XCAM_RETURN_ERROR_ANALYZER) {
            LOGE_AWB_SUBM(0xff, "no current aiq core status, please stop aiq before register custome awb!");
            return ret;
        } else if (ret != XCAM_RETURN_NO_ERROR) {
            LOGE_AWB_SUBM(0xff, "awb register error, ret %d", ret);
            return ret;
        }

        RkAiqAwbAlgoContext2* algoCtx = (RkAiqAwbAlgoContext2*)
        rk_aiq_uapi2_sysctl_getAxlibCtx(ctx,
                                       desc->common.type,
                                       desc->common.id);
        if (algoCtx == NULL) {
            LOGE_AWB_SUBM(0xff, "can't get custom awb algo %d ctx!", desc->common.id);
            return XCAM_RETURN_ERROR_FAILED;
        }

        algoCtx->aiq_ctx = (rk_aiq_sys_ctx_t*)(ctx);
    }

    if (group_ctx == NULL)
        return ret;
    rk_aiq_sys_ctx_t* cast_ctx = (rk_aiq_sys_ctx_t*)(ctx);
#if 0
    it = g_customAwb_desc_map.find(cast_ctx);

    if (it == g_customAwb_desc_map.end()) {
        desc = new RkAiqAlgoDescription();
        g_customAwb_desc_map[cast_ctx] = desc;
    } else {
        desc = it->second;
    }
#else
	if(cast_ctx->awb_desc == NULL)
		cast_ctx->awb_desc = (RkAiqAlgoDescription*)aiq_mallocz(1 * sizeof(RkAiqAlgoDescription));
	desc = cast_ctx->awb_desc;
#endif

    desc->common.version = g_RkIspAlgoDescCamgroupAwb.common.version;
    desc->common.vendor  = g_RkIspAlgoDescCamgroupAwb.common.vendor;
    desc->common.description = g_RkIspAlgoDescCamgroupAwb.common.description;
    desc->common.type    = g_RkIspAlgoDescCamgroupAwb.common.type;
    desc->common.id      = g_RkIspAlgoDescCamgroupAwb.common.id;
    desc->common.create_context  = AwbDemoCreateCtx;
    desc->common.destroy_context = AwbDemoDestroyCtx;
    desc->prepare = AwbDemoPrepare;
    desc->pre_process = AwbDemoPreProcess;
    desc->processing = AwbDemoGroupProcessing;
    desc->post_process = AwbDemoPostProcess;
#if RKAIQ_HAVE_DUMPSYS
    desc->dump= dump;
#endif
    struct RkAiqAlgoDesCommExt algoDes_camgroup[] = {
        { &desc->common, RK_AIQ_CORE_ANALYZE_AWB,  1, 2, 32, {0, 0}},
        { NULL, RK_AIQ_CORE_ANALYZE_ALL, 0,  0,  0, {0, 0} },
    };

    ret = rk_aiq_uapi2_sysctl_register3Aalgo(ctx, &algoDes_camgroup, cbs);
    if (ret == XCAM_RETURN_ERROR_ANALYZER) {
        LOGE_AWB_SUBM(0xff, "no current aiq core status, please stop aiq before register custome awb!");
        return ret;
    } else if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_AWB_SUBM(0xff, "awb register error, ret %d", ret);
        return ret;
    }

    RkAiqAwbAlgoContext2* algoCtx = (RkAiqAwbAlgoContext2*)
        rk_aiq_uapi2_sysctl_getAxlibCtx(ctx,
                                       desc->common.type,
                                       desc->common.id);
    if (algoCtx == NULL) {
        LOGE_AWB_SUBM(0xff, "can't get custom awb algo %d ctx!", desc->common.id);
        return XCAM_RETURN_ERROR_FAILED;
    }

    algoCtx->aiq_ctx = (rk_aiq_sys_ctx_t*)(ctx);

    LOGD_AWB_SUBM(0xff, "%s EXIT", __func__);

    return ret;
}

XCamReturn
rk_aiq_uapi2_customAWB_enable(const rk_aiq_sys_ctx_t* ctx, bool enable)
{

    LOGD_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoDescription* desc = NULL;
    rk_aiq_sys_ctx_t* cast_ctx = (rk_aiq_sys_ctx_t*)(ctx);
#if 0
    std::map<rk_aiq_sys_ctx_t*, RkAiqAlgoDescription*>::iterator it =
        g_customAwb_desc_map.find(cast_ctx);

    if (it == g_customAwb_desc_map.end()) {
        LOGE_AWB_SUBM(0xff, "can't find custom awb algo for sys_ctx %p !", ctx);
        return XCAM_RETURN_ERROR_FAILED;
    } else {
        desc = it->second;
    }
#else
    if(!cast_ctx->awb_desc) {
        LOGE_AWB_SUBM(0xff, "can't find custom awb algo for sys_ctx %p !", ctx);
        return XCAM_RETURN_ERROR_FAILED;
	}
	desc = cast_ctx->awb_desc;
#endif

    ret = rk_aiq_uapi2_sysctl_enableAxlib(ctx,
                                         desc->common.type,
                                         desc->common.id,
                                         enable);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_AWB_SUBM(0xff, "enable custom awb lib id %d failed !");
        return ret;
    }
    // now rk and custom awb are running concurrently,
    // because other algos will depend on results of rk awb
#if 0
    //only run custom awb
    if (enable)
        ret = rk_aiq_uapi_sysctl_enableAxlib(ctx,
                                             desc->common.type,
                                             0,
                                             !enable);
#endif

    LOGD_AWB_SUBM(0xff, "enable custom awb algo sucess for sys_ctx %p, lib_id %d !",
                  ctx,
                  desc->common.id);
    LOGD_AWB_SUBM(0xff, "%s EXIT", __func__);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_awb_unRegister(const rk_aiq_sys_ctx_t* ctx)
{

    LOGD_AWB_SUBM(0xff, "%s ENTER", __func__);
    RkAiqAlgoDescription* desc = NULL;
    rk_aiq_sys_ctx_t* cast_ctx = (rk_aiq_sys_ctx_t*)(ctx);

#if 0
    std::map<rk_aiq_sys_ctx_t*, RkAiqAlgoDescription*>::iterator it =
        g_customAwb_desc_map.find(cast_ctx);

    if (it == g_customAwb_desc_map.end()) {
        LOGE_AWB_SUBM(0xff, "can't find custom awb algo for sys_ctx %p !", ctx);
        return XCAM_RETURN_ERROR_FAILED;
    } else {
        desc = it->second;
    }
#else
    if(!cast_ctx->awb_desc) {
        LOGE_AWB_SUBM(0xff, "can't find custom awb algo for sys_ctx %p !", ctx);
        return XCAM_RETURN_ERROR_FAILED;
	}
	desc = cast_ctx->awb_desc;
#endif

    rk_aiq_uapi2_sysctl_unRegister3Aalgo(ctx,
                                        desc->common.type);

    LOGD_AWB_SUBM(0xff, "unregister custom awb algo sucess for sys_ctx %p, lib_id %d !",
                  ctx,
                  desc->common.id);
#if 0
    delete it->second;
    g_customAwb_desc_map.erase(it);
#else
    if(cast_ctx->awb_desc)
        aiq_free(cast_ctx->awb_desc);
    cast_ctx->awb_desc = NULL;
#endif
    LOGD_AWB_SUBM(0xff, "%s EXIT", __func__);
    return XCAM_RETURN_NO_ERROR;
}



RKAIQ_END_DECLARE

