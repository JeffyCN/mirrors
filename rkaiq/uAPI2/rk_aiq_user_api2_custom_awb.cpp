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

#include "rk_aiq_user_api2_custom_awb.h"
#include "RkAiqCalibDbV2Helper.h"

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#if RKAIQ_HAVE_AWB_V21
#include "rk_aiq_user_api2_custom_awb_v3x.cpp"
#elif RKAIQ_HAVE_AWB_V32
#include "rk_aiq_user_api2_custom_awb_v32.cpp"
#endif


RKAIQ_BEGIN_DECLARE

#define RKISP_ALGO_AWB_DEMO_VERSION     "v0.0.1"
#define RKISP_ALGO_AWB_DEMO_VENDOR      "Rockchip"
#define RKISP_ALGO_AWB_DEMO_DESCRIPTION "Rockchip Custom Awb"


/* instance was created by AIQ framework when rk_aiq_uapi_sysctl_regLib called */
typedef struct _RkAiqAwbAlgoContext {
    rk_aiq_customeAwb_cbs_t cbs; // set by register
    union {
        rk_aiq_sys_ctx_t* aiq_ctx;  // set by register
        rk_aiq_camgroup_ctx_t* group_ctx;  // set by register
    };
    rk_aiq_rkAwb_config_t rkCfg; // awb config of AIQ framework
    rk_aiq_customeAwb_results_t customRes; // result of pfn_awb_run
    bool cutomAwbInit;
    bool updateCalib;
    uint8_t log_level;
    uint8_t log_sub_modules;
    int camIdArray[6];
    int camIdArrayLen;
    bool isGroupMode;
} RkAiqAwbAlgoContext;

static XCamReturn AwbDemoCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOGD_AWB_SUBM(0xff, "%s ENTER", __func__);

    RESULT ret = RK_AIQ_RET_SUCCESS;
    RkAiqAwbAlgoContext *ctx = new RkAiqAwbAlgoContext();
    if (ctx == NULL) {
        printf( "%s: create awb context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    memset(ctx, 0, sizeof(*ctx));
    xcam_get_awb_log_level(&ctx->log_level, &ctx->log_sub_modules);
    if (cfg->isGroupMode) {
        AlgoCtxInstanceCfgCamGroup* grpCfg = (AlgoCtxInstanceCfgCamGroup*)cfg;
        memcpy(ctx->camIdArray, grpCfg->camIdArray, sizeof(ctx->camIdArray));
        ctx->camIdArrayLen = grpCfg->camIdArrayLen;
        ctx->isGroupMode = true;
    } else {
        ctx->camIdArrayLen = 0;
        ctx->isGroupMode = false;
    }
    *context = (RkAiqAlgoContext *)ctx;
    LOGD_AWB_SUBM(0xff, "%s EXIT", __func__);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AwbDemoDestroyCtx(RkAiqAlgoContext *context)
{
    LOGD_AWB_SUBM(0xff, "%s ENTER", __func__);
    RkAiqAwbAlgoContext* context2 = (RkAiqAwbAlgoContext*)context;
    if(context2 == NULL)
        return XCAM_RETURN_NO_ERROR;
    if (context2->cbs.pfn_awb_exit) {
        context2->cbs.pfn_awb_exit(context2->aiq_ctx);
        context2->cutomAwbInit = false;
    }
    delete context2;
    context2 = NULL;

    LOGD_AWB_SUBM(0xff, "%s EXIT", __func__);
    return XCAM_RETURN_NO_ERROR;
}


static XCamReturn AwbDemoPrepare(RkAiqAlgoCom* params)
{
    LOGD_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAwbAlgoContext* algo_ctx = (RkAiqAwbAlgoContext*)params->ctx;
    if(1/*params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGEMODE*/){
        algo_ctx->rkCfg.Working_mode = params->u.prepare.working_mode;
    }
    if (!algo_ctx->cutomAwbInit) {
        initAwbHwFullConfigGw(&algo_ctx->rkCfg.awbHwConfig);
        algo_ctx->rkCfg.RawWidth = params->u.prepare.sns_op_width;
        algo_ctx->rkCfg.RawHeight = params->u.prepare.sns_op_height;
        awb_window_init(algo_ctx->aiq_ctx,&algo_ctx->rkCfg);
        frame_choose_init(algo_ctx->aiq_ctx,&algo_ctx->rkCfg);
        initCustomAwbRes(&algo_ctx->customRes, &algo_ctx->rkCfg);
        algo_ctx->cbs.pfn_awb_init(algo_ctx->aiq_ctx);
        algo_ctx->cutomAwbInit = true;
    }

    if(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGERES){
        algo_ctx->rkCfg.RawWidth = params->u.prepare.sns_op_width;
        algo_ctx->rkCfg.RawHeight = params->u.prepare.sns_op_height;
        //awb_window_init(algo_ctx->aiq_ctx,&algo_ctx->rkCfg);
    }
    if(0/*params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGERES || params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGEMODE */){
        uint32_t cmdctrl=0;
        int pValue=0;
        if (algo_ctx->cbs.pfn_awb_ctrl){
            algo_ctx->cbs.pfn_awb_ctrl(algo_ctx->aiq_ctx,  cmdctrl, (void*)(&pValue));
        }
    }
    LOGD_AWB_SUBM(0xff, "%s EXIT", __func__);

    return ret;
}

static XCamReturn AwbDemoPreProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);

    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);

    return ret;
}

static XCamReturn AwbDemoProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{

    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAwb* AwbProcParams = (RkAiqAlgoProcAwb*)inparams;
    RkAiqAlgoProcResAwb* AwbProcResParams = (RkAiqAlgoProcResAwb*)outparams;
    RkAiqAwbAlgoContext* algo_ctx = (RkAiqAwbAlgoContext*)inparams->ctx;

    AwbProcResParams->awb_cfg_update = true;
    AwbProcResParams->awb_gain_update= true;

    if (algo_ctx->isGroupMode) {
        LOGE_AWB("wrong awb mode");
        return ret;
    }

    if(!inparams->u.proc.init) { // init=ture, stats=null
        rk_aiq_customAwb_stats_t customStats;
#if RKAIQ_HAVE_AWB_V21
    #if defined(ISP_HW_V30)
        if (!AwbProcParams->awb_statsBuf_v3x) {
            LOGE_AWB("awb stats is null");
            return(XCAM_RETURN_BYPASS);
        }
    #else
        if (!AwbProcParams->awb_statsBuf_v201) {
            LOGE_AWB("awb stats is null");
            return(XCAM_RETURN_BYPASS);
        }
    #endif
#elif RKAIQ_HAVE_AWB_V32
        if (!AwbProcParams->awb_statsBuf_v32) {
            LOGE_AWB("awb stats is null");
            return(XCAM_RETURN_BYPASS);
        }
#endif
#if RKAIQ_HAVE_AWB_V21
        rk_aiq_isp_awb_stats_v3x_t* xAwbStats = AwbProcParams->awb_statsBuf_v3x;
        _rkAwbStats2CustomAwbStats(&customStats, xAwbStats);
        WriteMeasureResult(*xAwbStats,algo_ctx->log_level);
#elif RKAIQ_HAVE_AWB_V32
        rk_aiq_isp_awb_stats_v32_t* xAwbStats = AwbProcParams->awb_statsBuf_v32;
        _rkAwbStats2CustomAwbStats(&customStats, xAwbStats);
        WriteMeasureResult(*xAwbStats,algo_ctx->log_level);
#endif
        if (algo_ctx->cbs.pfn_awb_run)
            algo_ctx->cbs.pfn_awb_run(algo_ctx->aiq_ctx,
                                     &customStats,
                                     &algo_ctx->customRes
                                  );
    }else{
        if (algo_ctx->cbs.pfn_awb_run)
             algo_ctx->cbs.pfn_awb_run(algo_ctx->aiq_ctx,
                                      nullptr,
                                      &algo_ctx->customRes
                                   );

    }

#if RKAIQ_HAVE_AWB_V21
    //check para
    awb_window_check(&algo_ctx->rkCfg,algo_ctx->customRes.awbHwConfig.windowSet);
    _customAwbHw2rkAwbHwCfg(&algo_ctx->customRes, &algo_ctx->rkCfg.awbHwConfig);
    // gen part of proc result which is from customRes
    _customAwbRes2rkAwbRes(AwbProcResParams, &algo_ctx->customRes,algo_ctx->rkCfg.awbHwConfig);
    WriteDataForThirdParty(*AwbProcResParams->awb_hw1_para,algo_ctx->log_level);
#elif RKAIQ_HAVE_AWB_V32

#if (RKAIQ_HAVE_BLC_V32)
       AblcProc_V32_t *ablc_res_v32 = AwbProcParams->ablcProcResV32;
#else
       AblcProc_V32_t *ablc_res_v32 = nullptr;
#endif

    RKAiqAecExpInfo_t *curExp = AwbProcParams->com.u.proc.curExp;
    float aec_iso =  50;
    float hdrmge_gain0_1=  1.0;
    int working_mode = algo_ctx->rkCfg.Working_mode;
    if(curExp != NULL) {
        if(working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            if(curExp->LinearExp.exp_real_params.analog_gain < 1.0) {
                aec_iso= 1.0;
                LOGW_AWB("linear mode again is wrong, use 1.0 instead\n");
            } else {
                aec_iso = curExp->LinearExp.exp_real_params.analog_gain;
            }
            if(curExp->LinearExp.exp_real_params.digital_gain < 1.0) {
                aec_iso *= 1.0;
                LOGW_AWB("linear mode dgain is wrong, use 1.0 instead\n");
            } else {
                aec_iso *= curExp->LinearExp.exp_real_params.digital_gain;
            }
        } else {
            int i=0;
            if ((rk_aiq_working_mode_t)working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2
                && (rk_aiq_working_mode_t)working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3) {
                i = 1;
            }else if ((rk_aiq_working_mode_t)working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR3) {
                i = 2;
            }

            if(curExp->HdrExp[i].exp_real_params.analog_gain < 1.0) {
                aec_iso = 1.0;
                LOGW_AWB("hdr mode again is wrong, use 1.0 instead\n");
            } else {
                aec_iso = curExp->HdrExp[i].exp_real_params.analog_gain;
            }
            if(curExp->HdrExp[i].exp_real_params.digital_gain < 1.0) {
                aec_iso  *= 1.0;
            } else {
                LOGW_AWB("hdr mode dgain is wrong, use 1.0 instead\n");
                aec_iso *= curExp->HdrExp[i].exp_real_params.digital_gain;
            }
            float sExpo = curExp->HdrExp[0].exp_real_params.analog_gain *
                curExp->HdrExp[0].exp_real_params.digital_gain *
                curExp->HdrExp[0].exp_real_params.integration_time;

            float lExpo = curExp->HdrExp[i].exp_real_params.analog_gain *
            curExp->HdrExp[i].exp_real_params.digital_gain *
            curExp->HdrExp[i].exp_real_params.integration_time;
            if(sExpo>DIVMIN){
                hdrmge_gain0_1 = lExpo/sExpo;
            }
        }
        aec_iso *=  50;
    }
    LOGE_AWB("aec_iso = %f, hdrmge_gain0_1=%f,",aec_iso,hdrmge_gain0_1);
    rk_aiq_isp_awb_meas_cfg_v32_t *awbHwConfigFull = &algo_ctx->rkCfg.awbHwConfig;
    //check para
    awb_window_check(&algo_ctx->rkCfg,awbHwConfigFull->windowSet);
    _customAwbHw2rkAwbHwCfg(&algo_ctx->customRes, awbHwConfigFull);
    //update by other para
    AwbProcResParams->awb_gain_algo->applyPosition =
        ((rk_aiq_working_mode_t)working_mode == RK_AIQ_WORKING_MODE_NORMAL) ? IN_AWBGAIN1 :IN_AWBGAIN0;
    calcInputBitIs12Bit( &awbHwConfigFull->inputBitIs12Bit, awbHwConfigFull->frameChoose, working_mode,ablc_res_v32);
    calcInputRightShift212Bit(&awbHwConfigFull->inputShiftEnable,awbHwConfigFull->frameChoose, working_mode,ablc_res_v32);
    ConfigWbgainBaseOnBlc(ablc_res_v32,AwbProcResParams->awb_gain_algo->applyPosition,&algo_ctx->customRes.awb_gain_algo);
    ret = ConfigPreWbgain2(awbHwConfigFull,algo_ctx->customRes.awb_gain_algo,AwbProcResParams->awb_gain_algo->applyPosition);
    RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);
    ret = ConfigBlc2(ablc_res_v32,algo_ctx->customRes.awb_gain_algo,
        AwbProcResParams->awb_gain_algo->applyPosition,working_mode,awbHwConfigFull);
    RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);
    ConfigOverexposureValue(ablc_res_v32,hdrmge_gain0_1,working_mode, awbHwConfigFull);
    if( awbHwConfigFull->frameChoose == CALIB_AWB_INPUT_DRC){
        awbHwConfigFull->lscBypEnable = true;
    }else{
        awbHwConfigFull->lscBypEnable = false;
    }
    // gen part of proc result which is from customRes
    _customAwbRes2rkAwbRes(AwbProcResParams, &algo_ctx->customRes,*awbHwConfigFull);
    WriteDataForThirdParty(*AwbProcResParams->awb_hw32_para,algo_ctx->log_level);
#endif
    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AwbDemoGroupProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{

    LOGD_AWB_SUBM(0xff, "%s ENTER --------------------frame%d  ---------------------", __func__,inparams->frame_id);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if RKAIQ_HAVE_AWB_V21
    RkAiqAlgoCamGroupProcIn* AwbProcParams = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* AwbProcResParams = (RkAiqAlgoCamGroupProcOut*)outparams;
    RkAiqAwbAlgoContext* algo_ctx = (RkAiqAwbAlgoContext*)inparams->ctx;
    if (!algo_ctx->isGroupMode) {
        LOGE_AWB("wrong awb mode for group");
        return ret;
    }

    if(!inparams->u.proc.init) { // init=ture, stats=null
        rk_aiq_customAwb_stats_t customStats;
        memset(&customStats,0,sizeof(customStats));
        ret = _rkAwbStats2CustomGroupAwbStats(AwbProcParams->camgroupParmasArray,AwbProcParams->arraySize, &customStats);
        RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);

        if (algo_ctx->cbs.pfn_awb_run)
            algo_ctx->cbs.pfn_awb_run(algo_ctx->aiq_ctx,
                                     &customStats,
                                     &algo_ctx->customRes
                                  );
        customAwbStatsRelease(&customStats);

    }else{

        if (algo_ctx->cbs.pfn_awb_run)
             algo_ctx->cbs.pfn_awb_run(algo_ctx->aiq_ctx,
                                      nullptr,
                                      &algo_ctx->customRes
                                   );

    }
    // gen part of proc result which is from customRes
    _customGruopAwbRes2rkAwbRes(AwbProcResParams->camgroupParmasArray ,AwbProcResParams->arraySize,&algo_ctx->rkCfg, &algo_ctx->customRes);
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

static std::map<rk_aiq_sys_ctx_t*, RkAiqAlgoDescription*> g_customAwb_desc_map;

XCamReturn
rk_aiq_uapi2_customAWB_register(const rk_aiq_sys_ctx_t* ctx, rk_aiq_customeAwb_cbs_t* cbs)
{
    LOGD_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (!cbs)
        return XCAM_RETURN_ERROR_PARAM;
    const rk_aiq_camgroup_ctx_t* group_ctx = NULL;

    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
       LOGI_AWB_SUBM(0xff, "group awb");
       group_ctx = (const rk_aiq_camgroup_ctx_t*)ctx;

    } else {
       LOGI_AWB_SUBM(0xff, "single awb");
    }
    RkAiqAlgoDescription* desc = NULL;
    rk_aiq_sys_ctx_t* cast_ctx = const_cast<rk_aiq_sys_ctx_t*>(ctx);

    std::map<rk_aiq_sys_ctx_t*, RkAiqAlgoDescription*>::iterator it =
        g_customAwb_desc_map.find(cast_ctx);

    if (it == g_customAwb_desc_map.end()) {
        desc = new RkAiqAlgoDescription();
        g_customAwb_desc_map[cast_ctx] = desc;
    } else {
        desc = it->second;
    }

    desc->common.version = RKISP_ALGO_AWB_DEMO_VERSION;
    desc->common.vendor  = RKISP_ALGO_AWB_DEMO_VENDOR;
    desc->common.description = RKISP_ALGO_AWB_DEMO_DESCRIPTION;
    desc->common.type    = RK_AIQ_ALGO_TYPE_AWB;
    desc->common.id      = 0;
    desc->common.create_context  = AwbDemoCreateCtx;
    desc->common.destroy_context = AwbDemoDestroyCtx;
    desc->prepare = AwbDemoPrepare;
    desc->pre_process = AwbDemoPreProcess;
    if (!group_ctx)
        desc->processing = AwbDemoProcessing;
    else
        desc->processing = AwbDemoGroupProcessing;
    desc->post_process = AwbDemoPostProcess;

    ret = rk_aiq_uapi_sysctl_regLib(ctx, &desc->common);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_AWB_SUBM(0xff, "register %d failed !", desc->common.id);
        return ret;
    }

    RkAiqAwbAlgoContext* algoCtx = (RkAiqAwbAlgoContext*)
        rk_aiq_uapi_sysctl_getAxlibCtx(ctx,
                                       desc->common.type,
                                       desc->common.id);
    if (algoCtx == NULL) {
        LOGE_AWB_SUBM(0xff, "can't get custom awb algo %d ctx!", desc->common.id);
        return XCAM_RETURN_ERROR_FAILED;
    }

    algoCtx->cbs = *cbs;
    algoCtx->aiq_ctx = const_cast<rk_aiq_sys_ctx_t*>(ctx);

    LOGD_AWB_SUBM(0xff, "register custom awb algo sucess for sys_ctx %p, lib_id %d !",
                  ctx,
                  desc->common.id);
    LOGD_AWB_SUBM(0xff, "%s EXIT", __func__);

    return ret;
}

XCamReturn
rk_aiq_uapi2_customAWB_enable(const rk_aiq_sys_ctx_t* ctx, bool enable)
{

    LOGD_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoDescription* desc = NULL;
    rk_aiq_sys_ctx_t* cast_ctx = const_cast<rk_aiq_sys_ctx_t*>(ctx);

    std::map<rk_aiq_sys_ctx_t*, RkAiqAlgoDescription*>::iterator it =
        g_customAwb_desc_map.find(cast_ctx);

    if (it == g_customAwb_desc_map.end()) {
        LOGE_AWB_SUBM(0xff, "can't find custom awb algo for sys_ctx %p !", ctx);
        return XCAM_RETURN_ERROR_FAILED;
    } else {
        desc = it->second;
    }

    ret = rk_aiq_uapi_sysctl_enableAxlib(ctx,
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
rk_aiq_uapi2_customAWB_unRegister(const rk_aiq_sys_ctx_t* ctx)
{

    LOGD_AWB_SUBM(0xff, "%s ENTER", __func__);
    RkAiqAlgoDescription* desc = NULL;
    rk_aiq_sys_ctx_t* cast_ctx = const_cast<rk_aiq_sys_ctx_t*>(ctx);

    std::map<rk_aiq_sys_ctx_t*, RkAiqAlgoDescription*>::iterator it =
        g_customAwb_desc_map.find(cast_ctx);

    if (it == g_customAwb_desc_map.end()) {
        LOGE_AWB_SUBM(0xff, "can't find custom awb algo for sys_ctx %p !", ctx);
        return XCAM_RETURN_ERROR_FAILED;
    } else {
        desc = it->second;
    }

    rk_aiq_uapi_sysctl_unRegLib(ctx,
                                desc->common.type,
                                desc->common.id);

    LOGD_AWB_SUBM(0xff, "unregister custom awb algo sucess for sys_ctx %p, lib_id %d !",
                  ctx,
                  desc->common.id);

    delete it->second;
    g_customAwb_desc_map.erase(it);

    LOGD_AWB_SUBM(0xff, "%s EXIT", __func__);
    return XCAM_RETURN_NO_ERROR;
}



RKAIQ_END_DECLARE
