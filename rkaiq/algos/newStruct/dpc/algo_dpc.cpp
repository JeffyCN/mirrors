/*
 * rk_aiq_algo_debayer_itf.c
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

#include "rk_aiq_algo_types.h"
#include "dpc_types_prvt.h"
#include "xcam_log.h"

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "RkAiqHandle.h"

#include "interpolation.h"

 //RKAIQ_BEGIN_DECLARE
#ifdef RKAIQ_HAVE_DPCC_V1
XCamReturn DpcSelectParam(DpcContext_t* pDpcCtx, dpc_param_t* out, int iso);
static XCamReturn Expert_mode_select_basic_params(
    Dpc_basic_params_select_t* pSelect,
    Dpc_basic_params_select_t* pParams);
static XCamReturn Fast_mode_select_basic_params(
    DpcContext_t* pDpcCtx,
    Dpc_basic_params_select_t* pSelect,
    int ilow, int ihigh, float ratio);
static XCamReturn select_bpt_params(Dpc_bpt_params_t* pParams, Dpc_bpt_params_t* pSelect);
static XCamReturn select_pdaf_params(Dpc_pdaf_params_t* pParams, Dpc_pdaf_params_t* pSelect);
static void Sensor_dpc_process(dpc_param_auto_t* paut, int ilow, int ihigh, float ratio);
#endif
#ifdef RKAIQ_HAVE_DPCC_V2
XCamReturn DpcSelectParam(DpcContext_t* pDpcCtx, dpc_param_t* out, int iso);
static void SingleSelectParam(dpc_dpDct_cfgEngine_t* dpDctEngine, unsigned char strg);
static void SmallClusterSelectParam(dpc_dpDct_cfgEngine_t* dpDctEngine, unsigned char strg);
static void BigClusterSelectParam(dpc_dpDct_cfgEngine_t* dpDctEngine,
    dpc_dpDct_fixEngine_t* dpDct_fixEngine, unsigned char strg);
#endif
static XCamReturn
create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t* pCalibDbV2 = cfg->calibv2;

    DpcContext_t* ctx = new DpcContext_t();
    if (ctx == NULL) {
        LOGE_ADPCC("%s: create Dpc context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }

    ctx->isReCal_ = true;
    ctx->prepare_params = NULL;
    ctx->dpc_attrib =
        (dpc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, dpc));

    *context = (RkAiqAlgoContext*)ctx;
    LOGV_ADPCC("%s: (exit)\n", __FUNCTION__);
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext* context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    DpcContext_t* pDpcCtx = (DpcContext_t*)context;
    delete pDpcCtx;
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    DpcContext_t* pDpcCtx = (DpcContext_t*)params->ctx;

    if (!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pDpcCtx->dpc_attrib =
                (dpc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, dpc));
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pDpcCtx->dpc_attrib =
        (dpc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, dpc));
    pDpcCtx->prepare_params = &params->u.prepare;
    pDpcCtx->isReCal_ = true;

    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    DpcContext_t* pDpcCtx = (DpcContext_t*)inparams->ctx;
    dpc_api_attrib_t* dpc_attrib = pDpcCtx->dpc_attrib;
    RkAiqAlgoProcResDpcc* pDpcProcResParams = (RkAiqAlgoProcResDpcc*)outparams;

    LOGV_ADPCC("%s: Dpc (enter)\n", __FUNCTION__);

    if (!dpc_attrib) {
        LOGE_ADPCC("dpc_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }

    int iso = inparams->u.proc.iso;
    bool init = inparams->u.proc.init;
    int delta_iso = abs(iso - pDpcCtx->iso);

    outparams->cfg_update = false;

    pDpcCtx->isBlackSensor = inparams->u.proc.is_bw_sensor;

    if (dpc_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ADPCC("mode is %d, not auto mode, ignore", dpc_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (inparams->u.proc.is_attrib_update) {
        pDpcCtx->isReCal_ = true;
    }

    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO) {
        pDpcCtx->isReCal_ = true;
    }

    if (pDpcCtx->isReCal_) {
        DpcSelectParam(pDpcCtx, pDpcProcResParams->dpcRes, iso);
        outparams->cfg_update = true;
        outparams->en = dpc_attrib->en;
        outparams->bypass = dpc_attrib->bypass;
        LOGD_ADPCC("dpc en:%d, bypass:%d", outparams->en, outparams->bypass);
    }

    pDpcCtx->iso = iso;
    pDpcCtx->isReCal_ = false;

    LOGV_ADPCC("%s: Dpc (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}
#ifdef RKAIQ_HAVE_DPCC_V1
XCamReturn DpcSelectParam(DpcContext_t *pDpcCtx, dpc_param_t* out, int iso)
{
    LOGD_ADPCC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pDpcCtx == NULL) {
        LOGE_ADPCC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    dpc_param_auto_t* paut = &pDpcCtx->dpc_attrib->stAuto;
    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0;
    float ratio = 0.0f;
    pre_interp(iso, NULL, 0, ilow, ihigh, ratio);

    bool fast_enable = paut->sta.fast_mode_en == 0 ? false : true;
    if(fast_enable == false)
        Expert_mode_select_basic_params(&out->dyn.arBasic, &paut->dyn[ilow].arBasic);
    else
        Fast_mode_select_basic_params(pDpcCtx, &out->dyn.arBasic, ilow, ihigh ,ratio);

    select_bpt_params(&paut->sta.stBptParams, &out->sta.stBptParams);
    select_pdaf_params(&paut->sta.stPdafParams, &out->sta.stPdafParams);

    //sensor dpc
    if(paut->sta.Sensor_dpc_en != 0 )
        Sensor_dpc_process(paut, ilow, ihigh ,ratio);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn Expert_mode_select_basic_params(
    Dpc_basic_params_select_t *pSelect,
    Dpc_basic_params_select_t *pParams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pSelect == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pParams == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    //mode 0x0000
    pSelect->stage1_enable = pParams->stage1_enable;
    pSelect->grayscale_mode = pParams->grayscale_mode;
    pSelect->enable = pParams->enable;

    //output_mode 0x0004
    pSelect->sw_rk_out_sel = pParams->sw_rk_out_sel;
    pSelect->sw_dpc_output_sel = pParams->sw_dpc_output_sel;
    pSelect->stage1_rb_3x3 = pParams->stage1_rb_3x3;
    pSelect->stage1_g_3x3 = pParams->stage1_g_3x3;
    pSelect->stage1_incl_rb_center = pParams->stage1_incl_rb_center;
    pSelect->stage1_incl_green_center = pParams->stage1_incl_green_center;

    //set_use 0x0008
    pSelect->stage1_use_fix_set = pParams->stage1_use_fix_set;
    pSelect->stage1_use_set_3 = pParams->stage1_use_set_3;
    pSelect->stage1_use_set_2 = pParams->stage1_use_set_2;
    pSelect->stage1_use_set_1 = pParams->stage1_use_set_1;

    //methods_set_1 0x000c
    pSelect->sw_rk_red_blue1_en = pParams->sw_rk_red_blue1_en;
    pSelect->rg_red_blue1_enable = pParams->rg_red_blue1_enable;
    pSelect->rnd_red_blue1_enable = pParams->rnd_red_blue1_enable;
    pSelect->ro_red_blue1_enable = pParams->ro_red_blue1_enable;
    pSelect->lc_red_blue1_enable = pParams->lc_red_blue1_enable;
    pSelect->pg_red_blue1_enable = pParams->pg_red_blue1_enable;
    pSelect->sw_rk_green1_en = pParams->sw_rk_green1_en;
    pSelect->rg_green1_enable = pParams->rg_green1_enable;
    pSelect->rnd_green1_enable = pParams->rnd_green1_enable;
    pSelect->ro_green1_enable = pParams->ro_green1_enable;
    pSelect->lc_green1_enable = pParams->lc_green1_enable;
    pSelect->pg_green1_enable = pParams->pg_green1_enable;

    //methods_set_2 0x0010
    pSelect->sw_rk_red_blue2_en = pParams->sw_rk_red_blue2_en;
    pSelect->rg_red_blue2_enable = pParams->rg_red_blue2_enable;
    pSelect->rnd_red_blue2_enable = pParams->rnd_red_blue2_enable;
    pSelect->ro_red_blue2_enable = pParams->ro_red_blue2_enable;
    pSelect->lc_red_blue2_enable = pParams->lc_red_blue2_enable;
    pSelect->pg_red_blue2_enable = pParams->pg_red_blue2_enable;
    pSelect->sw_rk_green2_en = pParams->sw_rk_green2_en;
    pSelect->rg_green2_enable = pParams->rg_green2_enable;
    pSelect->rnd_green2_enable = pParams->rnd_green2_enable;
    pSelect->ro_green2_enable = pParams->ro_green2_enable;
    pSelect->lc_green2_enable = pParams->lc_green2_enable;
    pSelect->pg_green2_enable = pParams->pg_green2_enable;

    //methods_set_3 0x0014
    pSelect->sw_rk_red_blue3_en = pParams->sw_rk_red_blue3_en;
    pSelect->rg_red_blue3_enable = pParams->rg_red_blue3_enable;
    pSelect->rnd_red_blue3_enable = pParams->rnd_red_blue3_enable;
    pSelect->ro_red_blue3_enable = pParams->ro_red_blue3_enable;
    pSelect->lc_red_blue3_enable = pParams->lc_red_blue3_enable;
    pSelect->pg_red_blue3_enable = pParams->pg_red_blue3_enable;
    pSelect->sw_rk_green3_en = pParams->sw_rk_green3_en;
    pSelect->rg_green3_enable = pParams->rg_green3_enable;
    pSelect->rnd_green3_enable = pParams->rnd_green3_enable;
    pSelect->ro_green3_enable = pParams->ro_green3_enable;
    pSelect->lc_green3_enable = pParams->lc_green3_enable;
    pSelect->pg_green3_enable = pParams->pg_green3_enable;

    //line_thresh_1 0x0018
    pSelect->sw_mindis1_rb = pParams->sw_mindis1_rb;
    pSelect->sw_mindis1_g = pParams->sw_mindis1_g;
    pSelect->line_thr_1_rb = pParams->line_thr_1_rb;
    pSelect->line_thr_1_g = pParams->line_thr_1_g;

    //line_mad_fac_1 0x001c
    pSelect->sw_dis_scale_min1 = pParams->sw_dis_scale_min1;
    pSelect->sw_dis_scale_max1 = pParams->sw_dis_scale_max1;
    pSelect->line_mad_fac_1_rb = pParams->line_mad_fac_1_rb;
    pSelect->line_mad_fac_1_g = pParams->line_mad_fac_1_g;

    //pg_fac_1 0x0020
    pSelect->pg_fac_1_rb = pParams->pg_fac_1_rb;
    pSelect->pg_fac_1_g = pParams->pg_fac_1_g;

    //rnd_thresh_1 0x0024
    pSelect->rnd_thr_1_rb = pParams->rnd_thr_1_rb;
    pSelect->rnd_thr_1_g = pParams->rnd_thr_1_g;

    //rg_fac_1 0x0028
    pSelect->rg_fac_1_rb = pParams->rg_fac_1_rb;
    pSelect->rg_fac_1_g = pParams->rg_fac_1_g;


    //line_thresh_2 0x002c
    pSelect->sw_mindis2_rb = pParams->sw_mindis2_rb;
    pSelect->sw_mindis2_g = pParams->sw_mindis2_g;
    pSelect->line_thr_2_rb = pParams->line_thr_2_rb;
    pSelect->line_thr_2_g = pParams->line_thr_2_g;

    //line_mad_fac_2 0x0030
    pSelect->sw_dis_scale_min2 = pParams->sw_dis_scale_min2;
    pSelect->sw_dis_scale_max2 = pParams->sw_dis_scale_max2;
    pSelect->line_mad_fac_2_rb = pParams->line_mad_fac_2_rb;
    pSelect->line_mad_fac_2_g = pParams->line_mad_fac_2_g;

    //pg_fac_2 0x0034
    pSelect->pg_fac_2_rb = pParams->pg_fac_2_rb;
    pSelect->pg_fac_2_g = pParams->pg_fac_2_g;

    //rnd_thresh_2 0x0038
    pSelect->rnd_thr_2_rb = pParams->rnd_thr_2_rb;
    pSelect->rnd_thr_2_g = pParams->rnd_thr_2_g;

    //rg_fac_2 0x003c
    pSelect->rg_fac_2_rb = pParams->rg_fac_2_rb;
    pSelect->rg_fac_2_g = pParams->rg_fac_2_g;


    //line_thresh_3 0x0040
    pSelect->sw_mindis3_rb = pParams->sw_mindis3_rb;
    pSelect->sw_mindis3_g = pParams->sw_mindis3_g;
    pSelect->line_thr_3_rb = pParams->line_thr_3_rb;
    pSelect->line_thr_3_g = pParams->line_thr_3_g;

    //line_mad_fac_3 0x0044
    pSelect->sw_dis_scale_min3 = pParams->sw_dis_scale_min3;
    pSelect->sw_dis_scale_max3 = pParams->sw_dis_scale_max3;
    pSelect->line_mad_fac_3_rb = pParams->line_mad_fac_3_rb;
    pSelect->line_mad_fac_3_g = pParams->line_mad_fac_3_g;

    //pg_fac_3 0x0048
    pSelect->pg_fac_3_rb = pParams->pg_fac_3_rb;
    pSelect->pg_fac_3_g = pParams->pg_fac_3_g;

    //rnd_thresh_3 0x004c
    pSelect->rnd_thr_3_rb = pParams->rnd_thr_3_rb;
    pSelect->rnd_thr_3_g = pParams->rnd_thr_3_g;

    //rg_fac_3 0x0050
    pSelect->rg_fac_3_rb = pParams->rg_fac_3_rb;
    pSelect->rg_fac_3_g = pParams->rg_fac_3_g;

    //ro_limits 0x0054
    pSelect->ro_lim_3_rb = pParams->ro_lim_3_rb;
    pSelect->ro_lim_3_g = pParams->ro_lim_3_g;
    pSelect->ro_lim_2_rb = pParams->ro_lim_2_rb;
    pSelect->ro_lim_2_g = pParams->ro_lim_2_g;
    pSelect->ro_lim_1_rb = pParams->ro_lim_1_rb;
    pSelect->ro_lim_1_g = pParams->ro_lim_1_g;

    //rnd_offs 0x0058
    pSelect->rnd_offs_3_rb = pParams->rnd_offs_3_rb;
    pSelect->rnd_offs_3_g = pParams->rnd_offs_3_g;
    pSelect->rnd_offs_2_rb = pParams->rnd_offs_2_rb;
    pSelect->rnd_offs_2_g = pParams->rnd_offs_2_g;
    pSelect->rnd_offs_1_rb = pParams->rnd_offs_1_rb;
    pSelect->rnd_offs_1_g = pParams->rnd_offs_1_g;


    LOGD_ADPCC("%s:(%d) %d %d %d %d %d", __FUNCTION__, __LINE__,
               pSelect->enable, pSelect->line_thr_2_g, pSelect->line_mad_fac_2_rb,
               pSelect->ro_lim_2_g, pSelect->rnd_offs_2_g);

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ret;
}

void Fast_mode_Triple_level_Setting(
    Dpc_basic_params_select_t *pSelect,
    int level)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    switch (level)
    {
    case 1:
        pSelect->sw_rk_green3_en = 1;
        pSelect->sw_mindis3_rb = 0x5;
        pSelect->sw_mindis3_g = 0x5;
        pSelect->sw_dis_scale_min3 = 0x3;
        pSelect->sw_dis_scale_max3 = 0x3;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 4;
        pSelect->rnd_thr_3_g = 4;
        pSelect->rnd_offs_3_rb = 2;
        pSelect->rnd_offs_3_g = 2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 2;
        pSelect->ro_lim_3_g = 2;

        pSelect->lc_red_blue3_enable = 1;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0x3;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0x3;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 1;
        pSelect->pg_green3_enable = 1;
        pSelect->pg_fac_3_rb = 0x3;
        pSelect->pg_fac_3_g = 0x3;
        break;
    case 2:
        pSelect->sw_rk_green3_en = 1;
        pSelect->sw_mindis3_rb = 0x5;
        pSelect->sw_mindis3_g = 0x5;
        pSelect->sw_dis_scale_min3 = 0x3;
        pSelect->sw_dis_scale_max3 = 0x3;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 4;
        pSelect->rnd_thr_3_g = 4;
        pSelect->rnd_offs_3_rb = 2;
        pSelect->rnd_offs_3_g = 2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 2;
        pSelect->ro_lim_3_g = 2;

        pSelect->lc_red_blue3_enable = 1;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0x2;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0x2;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 1;
        pSelect->pg_green3_enable = 1;
        pSelect->pg_fac_3_rb = 0x3;
        pSelect->pg_fac_3_g = 0x3;
        break;
    case 3:
        pSelect->sw_rk_green3_en = 1;
        pSelect->sw_mindis3_rb = 0x5;
        pSelect->sw_mindis3_g = 0x5;
        pSelect->sw_dis_scale_min3 = 0x3;
        pSelect->sw_dis_scale_max3 = 0x3;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 4;
        pSelect->rnd_thr_3_g = 4;
        pSelect->rnd_offs_3_rb = 2;
        pSelect->rnd_offs_3_g = 2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 2;
        pSelect->ro_lim_3_g = 2;

        pSelect->lc_red_blue3_enable = 0;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 1;
        pSelect->pg_green3_enable = 1;
        pSelect->pg_fac_3_rb = 0x3;
        pSelect->pg_fac_3_g = 0x3;
        break;
    case 4:
        pSelect->sw_rk_green3_en = 1;
        pSelect->sw_mindis3_rb = 0x5;
        pSelect->sw_mindis3_g = 0x5;
        pSelect->sw_dis_scale_min3 = 0x3;
        pSelect->sw_dis_scale_max3 = 0x3;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 4;
        pSelect->rnd_thr_3_g = 4;
        pSelect->rnd_offs_3_rb = 2;
        pSelect->rnd_offs_3_g = 2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 2;
        pSelect->ro_lim_3_g = 2;

        pSelect->lc_red_blue3_enable = 0;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 1;
        pSelect->pg_green3_enable = 1;
        pSelect->pg_fac_3_rb = 0x2;
        pSelect->pg_fac_3_g = 0x2;
        break;
    case 5:
        pSelect->sw_rk_green3_en = 1;
        pSelect->sw_mindis3_rb = 0x5;
        pSelect->sw_mindis3_g = 0x5;
        pSelect->sw_dis_scale_min3 = 0x3;
        pSelect->sw_dis_scale_max3 = 0x3;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 4;
        pSelect->rnd_thr_3_g = 4;
        pSelect->rnd_offs_3_rb = 2;
        pSelect->rnd_offs_3_g = 2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 2;
        pSelect->ro_lim_3_g = 2;

        pSelect->lc_red_blue3_enable = 0;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 0;
        pSelect->pg_green3_enable = 0;
        pSelect->pg_fac_3_rb = 0;
        pSelect->pg_fac_3_g = 0;
        break;
    case 6:
        pSelect->stage1_use_fix_set = 0x1;
        pSelect->sw_rk_red_blue3_en = 1;
        pSelect->sw_rk_green3_en = 1;
        pSelect->sw_mindis3_rb = 0x5;
        pSelect->sw_mindis3_g = 0x5;
        pSelect->sw_dis_scale_min3 = 0x3;
        pSelect->sw_dis_scale_max3 = 0x3;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 4;
        pSelect->rnd_thr_3_g = 4;
        pSelect->rnd_offs_3_rb = 2;
        pSelect->rnd_offs_3_g = 2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 2;
        pSelect->ro_lim_3_g = 2;

        pSelect->lc_red_blue3_enable = 0;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 0;
        pSelect->pg_green3_enable = 0;
        pSelect->pg_fac_3_rb = 0;
        pSelect->pg_fac_3_g = 0;
        break;
    case 7:
        pSelect->stage1_use_fix_set = 0x1;
        pSelect->sw_rk_red_blue3_en = 1;
        pSelect->sw_rk_green3_en = 1;
        pSelect->sw_mindis3_rb = 0x5;
        pSelect->sw_mindis3_g = 0x5;
        pSelect->sw_dis_scale_min3 = 0x3;
        pSelect->sw_dis_scale_max3 = 0x3;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 4;
        pSelect->rnd_thr_3_g = 4;
        pSelect->rnd_offs_3_rb = 2;
        pSelect->rnd_offs_3_g = 2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 2;
        pSelect->ro_lim_3_g = 2;

        pSelect->lc_red_blue3_enable = 0;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 0;
        pSelect->pg_green3_enable = 0;
        pSelect->pg_fac_3_rb = 0;
        pSelect->pg_fac_3_g = 0;
        break;
    case 8:
        pSelect->stage1_use_fix_set = 0x1;
        pSelect->sw_rk_red_blue3_en = 1;
        pSelect->sw_rk_green3_en = 1;
        pSelect->sw_mindis3_rb = 0x3;
        pSelect->sw_mindis3_g = 0x3;
        pSelect->sw_dis_scale_min3 = 0x2;
        pSelect->sw_dis_scale_max3 = 0x2;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 0x4;
        pSelect->rnd_thr_3_g = 0x4;
        pSelect->rnd_offs_3_rb = 0x2;
        pSelect->rnd_offs_3_g = 0x2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 0x2;
        pSelect->ro_lim_3_g = 0x2;

        pSelect->lc_red_blue3_enable = 0;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 0;
        pSelect->pg_green3_enable = 0;
        pSelect->pg_fac_3_rb = 0;
        pSelect->pg_fac_3_g = 0;
        break;
    case 9:
        pSelect->stage1_use_fix_set = 0x1;
        pSelect->sw_rk_red_blue3_en = 0;
        pSelect->sw_rk_green3_en = 0;
        pSelect->sw_mindis3_rb = 0;
        pSelect->sw_mindis3_g = 0;
        pSelect->sw_dis_scale_min3 = 0;
        pSelect->sw_dis_scale_max3 = 0;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 0x4;
        pSelect->rnd_thr_3_g = 0x4;
        pSelect->rnd_offs_3_rb = 0x2;
        pSelect->rnd_offs_3_g = 0x2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 0x2;
        pSelect->ro_lim_3_g = 0x2;

        pSelect->lc_red_blue3_enable = 0;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 0;
        pSelect->pg_green3_enable = 0;
        pSelect->pg_fac_3_rb = 0;
        pSelect->pg_fac_3_g = 0;
        break;
    case 10:
        pSelect->stage1_use_fix_set = 0x1;
        pSelect->sw_rk_red_blue3_en = 0;
        pSelect->sw_rk_green3_en = 0;
        pSelect->sw_mindis3_rb = 0;
        pSelect->sw_mindis3_g = 0;
        pSelect->sw_dis_scale_min3 = 0;
        pSelect->sw_dis_scale_max3 = 0;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 0x3;
        pSelect->rnd_thr_3_g = 0x3;
        pSelect->rnd_offs_3_rb = 0x1;
        pSelect->rnd_offs_3_g = 0x1;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 0x2;
        pSelect->ro_lim_3_g = 0x1;

        pSelect->lc_red_blue3_enable = 0;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 0;
        pSelect->pg_green3_enable = 0;
        pSelect->pg_fac_3_rb = 0;
        pSelect->pg_fac_3_g = 0;
        break;
    default:
        LOGE_ADPCC("%s(%d): Wrong fast mode level!!!", __FUNCTION__, __LINE__);
        break;
    }

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);

}

void Fast_mode_Triple_Setting(
    dpc_param_auto_t* paut,
    Dpc_basic_params_select_t *pSelect,
    int ilow, int ihigh, float ratio)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    int level = interpolation_i32(
        paut->dyn[ilow].stFastMode.fast_mode_triple_level, paut->dyn[ihigh].stFastMode.fast_mode_triple_level, ratio);

    if(paut->sta.fast_mode_triple_en != 0)
        pSelect->stage1_use_set_3 = 0x1;
    else
        pSelect->stage1_use_set_3 = 0x0;

    level = LIMIT_VALUE(level, FASTMODELEVELMAX, FASTMODELEVELMIN);
    Fast_mode_Triple_level_Setting(pSelect, level);

    LOGD_ADPCC("%s(%d): Dpc fast mode triple en:%d level:%d", __FUNCTION__, __LINE__,
               paut->sta.fast_mode_triple_en, level);

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);

}

void Fast_mode_Double_level_Setting(
    Dpc_basic_params_select_t *pSelect,
    int level)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    switch (level)
    {
    case 1:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x15;
        pSelect->sw_mindis2_g = 0x20;
        pSelect->sw_dis_scale_min2 = 0x10;
        pSelect->sw_dis_scale_max2 = 0x12;

        pSelect->rg_red_blue2_enable = 1;
        pSelect->rg_green2_enable = 1;
        pSelect->rg_fac_2_rb = 0x15;
        pSelect->rg_fac_2_g = 0x20;

        pSelect->rnd_red_blue2_enable = 1;
        pSelect->rnd_green2_enable = 1;
        pSelect->rnd_thr_2_rb = 0x9;
        pSelect->rnd_thr_2_g = 0xa;
        pSelect->rnd_offs_2_rb = 0x1;
        pSelect->rnd_offs_2_g = 0x1;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x1;
        pSelect->ro_lim_2_g = 0x2;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x6;
        pSelect->line_thr_2_g = 0x8;
        pSelect->line_mad_fac_2_rb = 0x3;
        pSelect->line_mad_fac_2_g = 0x4;

        pSelect->pg_red_blue2_enable = 1;
        pSelect->pg_green1_enable = 1;
        pSelect->pg_fac_2_rb = 0x6;
        pSelect->pg_fac_2_g = 0x8;
        break;
    case 2:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x12;
        pSelect->sw_mindis2_g = 0x16;
        pSelect->sw_dis_scale_min2 = 0x6;
        pSelect->sw_dis_scale_max2 = 0x8;

        pSelect->rg_red_blue2_enable = 1;
        pSelect->rg_green2_enable = 1;
        pSelect->rg_fac_2_rb = 0x7;
        pSelect->rg_fac_2_g = 0x10;

        pSelect->rnd_red_blue2_enable = 1;
        pSelect->rnd_green2_enable = 1;
        pSelect->rnd_thr_2_rb = 0x5;
        pSelect->rnd_thr_2_g = 0x6;
        pSelect->rnd_offs_2_rb = 0x1;
        pSelect->rnd_offs_2_g = 0x1;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x3;
        pSelect->ro_lim_2_g = 0x2;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x12;
        pSelect->line_thr_2_g = 0x16;
        pSelect->line_mad_fac_2_rb = 0x8;
        pSelect->line_mad_fac_2_g = 0x10;

        pSelect->pg_red_blue2_enable = 1;
        pSelect->pg_green2_enable = 1;
        pSelect->pg_fac_2_rb = 0x5;
        pSelect->pg_fac_2_g = 0x6;
        break;
    case 3:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x12;
        pSelect->sw_mindis2_g = 0x16;
        pSelect->sw_dis_scale_min2 = 0x6;
        pSelect->sw_dis_scale_max2 = 0x8;

        pSelect->rg_red_blue2_enable = 0;
        pSelect->rg_green2_enable = 0;
        pSelect->rg_fac_2_rb = 0;
        pSelect->rg_fac_2_g = 0;

        pSelect->rnd_red_blue2_enable = 1;
        pSelect->rnd_green2_enable = 1;
        pSelect->rnd_thr_2_rb = 0x5;
        pSelect->rnd_thr_2_g = 0x6;
        pSelect->rnd_offs_2_rb = 0x1;
        pSelect->rnd_offs_2_g = 0x1;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x3;
        pSelect->ro_lim_2_g = 0x2;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x12;
        pSelect->line_thr_2_g = 0x16;
        pSelect->line_mad_fac_2_rb = 0x8;
        pSelect->line_mad_fac_2_g = 0x10;

        pSelect->pg_red_blue2_enable = 1;
        pSelect->pg_green2_enable = 1;
        pSelect->pg_fac_2_rb = 0x5;
        pSelect->pg_fac_2_g = 0x6;
        break;
    case 4:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x12;
        pSelect->sw_mindis2_g = 0x16;
        pSelect->sw_dis_scale_min2 = 0x6;
        pSelect->sw_dis_scale_max2 = 0x8;

        pSelect->rg_red_blue2_enable = 0;
        pSelect->rg_green2_enable = 0;
        pSelect->rg_fac_2_rb = 0;
        pSelect->rg_fac_2_g = 0;

        pSelect->rnd_red_blue2_enable = 0;
        pSelect->rnd_green2_enable = 0;
        pSelect->rnd_thr_2_rb = 0;
        pSelect->rnd_thr_2_g = 0;
        pSelect->rnd_offs_2_rb = 0;
        pSelect->rnd_offs_2_g = 0;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x3;
        pSelect->ro_lim_2_g = 0x2;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x12;
        pSelect->line_thr_2_g = 0x16;
        pSelect->line_mad_fac_2_rb = 0x8;
        pSelect->line_mad_fac_2_g = 0x10;

        pSelect->pg_red_blue2_enable = 1;
        pSelect->pg_green2_enable = 1;
        pSelect->pg_fac_2_rb = 0x5;
        pSelect->pg_fac_2_g = 0x6;
        break;
    case 5:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x10;
        pSelect->sw_mindis2_g = 0x14;
        pSelect->sw_dis_scale_min2 = 0x6;
        pSelect->sw_dis_scale_max2 = 0xc;

        pSelect->rg_red_blue2_enable = 0;
        pSelect->rg_green2_enable = 0;
        pSelect->rg_fac_2_rb = 0;
        pSelect->rg_fac_2_g = 0;

        pSelect->rnd_red_blue2_enable = 0;
        pSelect->rnd_green2_enable = 0;
        pSelect->rnd_thr_2_rb = 0;
        pSelect->rnd_thr_2_g = 0;
        pSelect->rnd_offs_2_rb = 0;
        pSelect->rnd_offs_2_g = 0;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x3;
        pSelect->ro_lim_2_g = 0x3;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x7;
        pSelect->line_thr_2_g = 0xc;
        pSelect->line_mad_fac_2_rb = 0x7;
        pSelect->line_mad_fac_2_g = 0x9;

        pSelect->pg_red_blue2_enable = 1;
        pSelect->pg_green2_enable = 1;
        pSelect->pg_fac_2_rb = 0x3;
        pSelect->pg_fac_2_g = 0x4;
        break;
    case 6:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x7;
        pSelect->sw_mindis2_g = 0x10;
        pSelect->sw_dis_scale_min2 = 0x6;
        pSelect->sw_dis_scale_max2 = 0x8;

        pSelect->rg_red_blue2_enable = 0;
        pSelect->rg_green2_enable = 0;
        pSelect->rg_fac_2_rb = 0;
        pSelect->rg_fac_2_g = 0;

        pSelect->rnd_red_blue2_enable = 0;
        pSelect->rnd_green2_enable = 0;
        pSelect->rnd_thr_2_rb = 0;
        pSelect->rnd_thr_2_g = 0;
        pSelect->rnd_offs_2_rb = 0;
        pSelect->rnd_offs_2_g = 0;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x3;
        pSelect->ro_lim_2_g = 0x3;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x7;
        pSelect->line_thr_2_g = 0x9;
        pSelect->line_mad_fac_2_rb = 0x5;
        pSelect->line_mad_fac_2_g = 0x7;

        pSelect->pg_red_blue2_enable = 1;
        pSelect->pg_green2_enable = 1;
        pSelect->pg_fac_2_rb = 0x3;
        pSelect->pg_fac_2_g = 0x4;
        break;
    case 7:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x5;
        pSelect->sw_mindis2_g = 0x8;
        pSelect->sw_dis_scale_min2 = 0x3;
        pSelect->sw_dis_scale_max2 = 0x6;

        pSelect->rg_red_blue2_enable = 0;
        pSelect->rg_green2_enable = 0;
        pSelect->rg_fac_2_rb = 0;
        pSelect->rg_fac_2_g = 0;

        pSelect->rnd_red_blue2_enable = 0;
        pSelect->rnd_green2_enable = 0;
        pSelect->rnd_thr_2_rb = 0;
        pSelect->rnd_thr_2_g = 0;
        pSelect->rnd_offs_2_rb = 0;
        pSelect->rnd_offs_2_g = 0;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x4;
        pSelect->ro_lim_2_g = 0x3;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x5;
        pSelect->line_thr_2_g = 0x7;
        pSelect->line_mad_fac_2_rb = 0x3;
        pSelect->line_mad_fac_2_g = 0x5;

        pSelect->pg_red_blue2_enable = 1;
        pSelect->pg_green2_enable = 1;
        pSelect->pg_fac_2_rb = 0x2;
        pSelect->pg_fac_2_g = 0x1;
        break;
    case 8:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x5;
        pSelect->sw_mindis2_g = 0x8;
        pSelect->sw_dis_scale_min2 = 0x3;
        pSelect->sw_dis_scale_max2 = 0x6;

        pSelect->rg_red_blue2_enable = 0;
        pSelect->rg_green2_enable = 0;
        pSelect->rg_fac_2_rb = 0;
        pSelect->rg_fac_2_g = 0;

        pSelect->rnd_red_blue2_enable = 0;
        pSelect->rnd_green2_enable = 0;
        pSelect->rnd_thr_2_rb = 0;
        pSelect->rnd_thr_2_g = 0;
        pSelect->rnd_offs_2_rb = 0;
        pSelect->rnd_offs_2_g = 0;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x1;
        pSelect->ro_lim_2_g = 0x3;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x5;
        pSelect->line_thr_2_g = 0x7;
        pSelect->line_mad_fac_2_rb = 0x3;
        pSelect->line_mad_fac_2_g = 0x5;

        pSelect->pg_red_blue2_enable = 0;
        pSelect->pg_green2_enable = 0;
        pSelect->pg_fac_2_rb = 0;
        pSelect->pg_fac_2_g = 0;
        break;
    case 9:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x4;
        pSelect->sw_mindis2_g = 0x8;
        pSelect->sw_dis_scale_min2 = 0x2;
        pSelect->sw_dis_scale_max2 = 0x6;

        pSelect->rg_red_blue2_enable = 0;
        pSelect->rg_green2_enable = 0;
        pSelect->rg_fac_2_rb = 0;
        pSelect->rg_fac_2_g = 0;

        pSelect->rnd_red_blue2_enable = 0;
        pSelect->rnd_green2_enable = 0;
        pSelect->rnd_thr_2_rb = 0;
        pSelect->rnd_thr_2_g = 0;
        pSelect->rnd_offs_2_rb = 0;
        pSelect->rnd_offs_2_g = 0;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x4;
        pSelect->ro_lim_2_g = 0x3;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x1;
        pSelect->line_thr_2_g = 0x3;
        pSelect->line_mad_fac_2_rb = 0x2;
        pSelect->line_mad_fac_2_g = 0x2;

        pSelect->pg_red_blue2_enable = 0;
        pSelect->pg_green2_enable = 0;
        pSelect->pg_fac_2_rb = 0;
        pSelect->pg_fac_2_g = 0;
        break;
    case 10:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x4;
        pSelect->sw_mindis2_g = 0x8;
        pSelect->sw_dis_scale_min2 = 0x3;
        pSelect->sw_dis_scale_max2 = 0x6;

        pSelect->rg_red_blue2_enable = 0;
        pSelect->rg_green2_enable = 0;
        pSelect->rg_fac_2_rb = 0;
        pSelect->rg_fac_2_g = 0;

        pSelect->rnd_red_blue2_enable = 0;
        pSelect->rnd_green2_enable = 0;
        pSelect->rnd_thr_2_rb = 0;
        pSelect->rnd_thr_2_g = 0;
        pSelect->rnd_offs_2_rb = 0;
        pSelect->rnd_offs_2_g = 0;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x4;
        pSelect->ro_lim_2_g = 0x3;

        pSelect->lc_red_blue2_enable = 0;
        pSelect->lc_green2_enable = 0;
        pSelect->line_thr_2_rb = 0;
        pSelect->line_thr_2_g = 0;
        pSelect->line_mad_fac_2_rb = 0;
        pSelect->line_mad_fac_2_g = 0;

        pSelect->pg_red_blue2_enable = 0;
        pSelect->pg_green2_enable = 0;
        pSelect->pg_fac_2_rb = 0;
        pSelect->pg_fac_2_g = 0;
        break;
    default:
        LOGE_ADPCC("%s(%d): Wrong fast mode level!!!", __FUNCTION__, __LINE__);
        break;
    }


    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);

}

void Fast_mode_Double_Setting(
    dpc_param_auto_t* paut,
    Dpc_basic_params_select_t *pSelect,
    int ilow, int ihigh, float ratio)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);


    int level = interpolation_i32(
        paut->dyn[ilow].stFastMode.fast_mode_double_level, paut->dyn[ihigh].stFastMode.fast_mode_double_level, ratio);

    if(paut->sta.fast_mode_double_en != 0)
        pSelect->stage1_use_set_2 = 0x1;
    else
        pSelect->stage1_use_set_2 = 0x0;

    level = LIMIT_VALUE(level, FASTMODELEVELMAX, FASTMODELEVELMIN);
    Fast_mode_Double_level_Setting(pSelect, level);

    LOGD_ADPCC("%s(%d): Dpc fast mode double en:%d level:%d", __FUNCTION__, __LINE__,
               paut->sta.fast_mode_double_en, level);

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);

}

void Fast_mode_Single_level_Setting(
    Dpc_basic_params_select_t *pSelect,
    int level)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    switch (level)
    {
    case 1:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x20;
        pSelect->sw_mindis1_g = 0x20;
        pSelect->sw_dis_scale_min1 = 0x12;
        pSelect->sw_dis_scale_max1 = 0x12;

        pSelect->rg_red_blue1_enable = 1;
        pSelect->rg_green1_enable = 1;
        pSelect->rg_fac_1_rb = 0x20;
        pSelect->rg_fac_1_g = 0x20;

        pSelect->rnd_red_blue1_enable = 1;
        pSelect->rnd_green1_enable = 1;
        pSelect->rnd_thr_1_rb = 0xa;
        pSelect->rnd_thr_1_g = 0xa;
        pSelect->rnd_offs_1_rb = 0x1;
        pSelect->rnd_offs_1_g = 0x1;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x2;
        pSelect->ro_lim_1_g = 0x2;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0x8;
        pSelect->line_thr_1_g = 0x8;
        pSelect->line_mad_fac_1_rb = 0x4;
        pSelect->line_mad_fac_1_g = 0x4;

        pSelect->pg_red_blue1_enable = 1;
        pSelect->pg_green1_enable = 1;
        pSelect->pg_fac_1_rb = 0x8;
        pSelect->pg_fac_1_g = 0x8;
        break;
    case 2:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x16;
        pSelect->sw_mindis1_g = 0x16;
        pSelect->sw_dis_scale_min1 = 0x8;
        pSelect->sw_dis_scale_max1 = 0x8;

        pSelect->rg_red_blue1_enable = 1;
        pSelect->rg_green1_enable = 1;
        pSelect->rg_fac_1_rb = 0x10;
        pSelect->rg_fac_1_g = 0x10;

        pSelect->rnd_red_blue1_enable = 1;
        pSelect->rnd_green1_enable = 1;
        pSelect->rnd_thr_1_rb = 0x6;
        pSelect->rnd_thr_1_g = 0x6;
        pSelect->rnd_offs_1_rb = 0x1;
        pSelect->rnd_offs_1_g = 0x1;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x2;
        pSelect->ro_lim_1_g = 0x2;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0x16;
        pSelect->line_thr_1_g = 0x16;
        pSelect->line_mad_fac_1_rb = 0x10;
        pSelect->line_mad_fac_1_g = 0x10;

        pSelect->pg_red_blue1_enable = 1;
        pSelect->pg_green1_enable = 1;
        pSelect->pg_fac_1_rb = 0x6;
        pSelect->pg_fac_1_g = 0x6;
        break;
    case 3:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x16;
        pSelect->sw_mindis1_g = 0x16;
        pSelect->sw_dis_scale_min1 = 0x8;
        pSelect->sw_dis_scale_max1 = 0x8;

        pSelect->rg_red_blue1_enable = 0;
        pSelect->rg_green1_enable = 0;
        pSelect->rg_fac_1_rb = 0;
        pSelect->rg_fac_1_g = 0;

        pSelect->rnd_red_blue1_enable = 1;
        pSelect->rnd_green1_enable = 1;
        pSelect->rnd_thr_1_rb = 0x6;
        pSelect->rnd_thr_1_g = 0x6;
        pSelect->rnd_offs_1_rb = 0x1;
        pSelect->rnd_offs_1_g = 0x1;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x2;
        pSelect->ro_lim_1_g = 0x2;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0x16;
        pSelect->line_thr_1_g = 0x16;
        pSelect->line_mad_fac_1_rb = 0x10;
        pSelect->line_mad_fac_1_g = 0x10;

        pSelect->pg_red_blue1_enable = 1;
        pSelect->pg_green1_enable = 1;
        pSelect->pg_fac_1_rb = 0x6;
        pSelect->pg_fac_1_g = 0x6;
        break;
    case 4:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x16;
        pSelect->sw_mindis1_g = 0x16;
        pSelect->sw_dis_scale_min1 = 0x8;
        pSelect->sw_dis_scale_max1 = 0x8;

        pSelect->rg_red_blue1_enable = 0;
        pSelect->rg_green1_enable = 0;
        pSelect->rg_fac_1_rb = 0;
        pSelect->rg_fac_1_g = 0;

        pSelect->rnd_red_blue1_enable = 0;
        pSelect->rnd_green1_enable = 0;
        pSelect->rnd_thr_1_rb = 0;
        pSelect->rnd_thr_1_g = 0;
        pSelect->rnd_offs_1_rb = 0;
        pSelect->rnd_offs_1_g = 0;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x2;
        pSelect->ro_lim_1_g = 0x2;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0x16;
        pSelect->line_thr_1_g = 0x16;
        pSelect->line_mad_fac_1_rb = 0x10;
        pSelect->line_mad_fac_1_g = 0x10;

        pSelect->pg_red_blue1_enable = 1;
        pSelect->pg_green1_enable = 1;
        pSelect->pg_fac_1_rb = 0x6;
        pSelect->pg_fac_1_g = 0x6;
        break;
    case 5:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x14;
        pSelect->sw_mindis1_g = 0x14;
        pSelect->sw_dis_scale_min1 = 0xc;
        pSelect->sw_dis_scale_max1 = 0xc;

        pSelect->rg_red_blue1_enable = 0;
        pSelect->rg_green1_enable = 0;
        pSelect->rg_fac_1_rb = 0;
        pSelect->rg_fac_1_g = 0;

        pSelect->rnd_red_blue1_enable = 0;
        pSelect->rnd_green1_enable = 0;
        pSelect->rnd_thr_1_rb = 0;
        pSelect->rnd_thr_1_g = 0;
        pSelect->rnd_offs_1_rb = 0;
        pSelect->rnd_offs_1_g = 0;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x3;
        pSelect->ro_lim_1_g = 0x3;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0xc;
        pSelect->line_thr_1_g = 0xc;
        pSelect->line_mad_fac_1_rb = 0x9;
        pSelect->line_mad_fac_1_g = 0x9;

        pSelect->pg_red_blue1_enable = 1;
        pSelect->pg_green1_enable = 1;
        pSelect->pg_fac_1_rb = 0x5;
        pSelect->pg_fac_1_g = 0x4;
        break;
    case 6:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x10;
        pSelect->sw_mindis1_g = 0x10;
        pSelect->sw_dis_scale_min1 = 0x8;
        pSelect->sw_dis_scale_max1 = 0x8;

        pSelect->rg_red_blue1_enable = 0;
        pSelect->rg_green1_enable = 0;
        pSelect->rg_fac_1_rb = 0;
        pSelect->rg_fac_1_g = 0;

        pSelect->rnd_red_blue1_enable = 0;
        pSelect->rnd_green1_enable = 0;
        pSelect->rnd_thr_1_rb = 0;
        pSelect->rnd_thr_1_g = 0;
        pSelect->rnd_offs_1_rb = 0;
        pSelect->rnd_offs_1_g = 0;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x3;
        pSelect->ro_lim_1_g = 0x3;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0x9;
        pSelect->line_thr_1_g = 0x9;
        pSelect->line_mad_fac_1_rb = 0x7;
        pSelect->line_mad_fac_1_g = 0x7;

        pSelect->pg_red_blue1_enable = 1;
        pSelect->pg_green1_enable = 1;
        pSelect->pg_fac_1_rb = 0x5;
        pSelect->pg_fac_1_g = 0x4;
        break;
    case 7:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x8;
        pSelect->sw_mindis1_g = 0x8;
        pSelect->sw_dis_scale_min1 = 0x6;
        pSelect->sw_dis_scale_max1 = 0x6;

        pSelect->rg_red_blue1_enable = 0;
        pSelect->rg_green1_enable = 0;
        pSelect->rg_fac_1_rb = 0;
        pSelect->rg_fac_1_g = 0;

        pSelect->rnd_red_blue1_enable = 0;
        pSelect->rnd_green1_enable = 0;
        pSelect->rnd_thr_1_rb = 0;
        pSelect->rnd_thr_1_g = 0;
        pSelect->rnd_offs_1_rb = 0;
        pSelect->rnd_offs_1_g = 0;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x3;
        pSelect->ro_lim_1_g = 0x3;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0x7;
        pSelect->line_thr_1_g = 0x7;
        pSelect->line_mad_fac_1_rb = 0x5;
        pSelect->line_mad_fac_1_g = 0x5;

        pSelect->pg_red_blue1_enable = 1;
        pSelect->pg_green1_enable = 1;
        pSelect->pg_fac_1_rb = 0x3;
        pSelect->pg_fac_1_g = 0x1;
        break;
    case 8:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x8;
        pSelect->sw_mindis1_g = 0x8;
        pSelect->sw_dis_scale_min1 = 0x6;
        pSelect->sw_dis_scale_max1 = 0x6;

        pSelect->rg_red_blue1_enable = 0;
        pSelect->rg_green1_enable = 0;
        pSelect->rg_fac_1_rb = 0;
        pSelect->rg_fac_1_g = 0;

        pSelect->rnd_red_blue1_enable = 0;
        pSelect->rnd_green1_enable = 0;
        pSelect->rnd_thr_1_rb = 0;
        pSelect->rnd_thr_1_g = 0;
        pSelect->rnd_offs_1_rb = 0;
        pSelect->rnd_offs_1_g = 0;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x3;
        pSelect->ro_lim_1_g = 0x3;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0x7;
        pSelect->line_thr_1_g = 0x7;
        pSelect->line_mad_fac_1_rb = 0x5;
        pSelect->line_mad_fac_1_g = 0x5;

        pSelect->pg_red_blue1_enable = 0;
        pSelect->pg_green1_enable = 0;
        pSelect->pg_fac_1_rb = 0;
        pSelect->pg_fac_1_g = 0;
        break;
    case 9:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x8;
        pSelect->sw_mindis1_g = 0x8;
        pSelect->sw_dis_scale_min1 = 0x6;
        pSelect->sw_dis_scale_max1 = 0x6;

        pSelect->rg_red_blue1_enable = 0;
        pSelect->rg_green1_enable = 0;
        pSelect->rg_fac_1_rb = 0;
        pSelect->rg_fac_1_g = 0;

        pSelect->rnd_red_blue1_enable = 0;
        pSelect->rnd_green1_enable = 0;
        pSelect->rnd_thr_1_rb = 0;
        pSelect->rnd_thr_1_g = 0;
        pSelect->rnd_offs_1_rb = 0;
        pSelect->rnd_offs_1_g = 0;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x3;
        pSelect->ro_lim_1_g = 0x3;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0x3;
        pSelect->line_thr_1_g = 0x3;
        pSelect->line_mad_fac_1_rb = 0x2;
        pSelect->line_mad_fac_1_g = 0x2;

        pSelect->pg_red_blue1_enable = 0;
        pSelect->pg_green1_enable = 0;
        pSelect->pg_fac_1_rb = 0;
        pSelect->pg_fac_1_g = 0;
        break;
    case 10:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x8;
        pSelect->sw_mindis1_g = 0x8;
        pSelect->sw_dis_scale_min1 = 0x6;
        pSelect->sw_dis_scale_max1 = 0x6;

        pSelect->rg_red_blue1_enable = 0;
        pSelect->rg_green1_enable = 0;
        pSelect->rg_fac_1_rb = 0;
        pSelect->rg_fac_1_g = 0;

        pSelect->rnd_red_blue1_enable = 0;
        pSelect->rnd_green1_enable = 0;
        pSelect->rnd_thr_1_rb = 0;
        pSelect->rnd_thr_1_g = 0;
        pSelect->rnd_offs_1_rb = 0;
        pSelect->rnd_offs_1_g = 0;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x3;
        pSelect->ro_lim_1_g = 0x3;

        pSelect->lc_red_blue1_enable = 0;
        pSelect->lc_green1_enable = 0;
        pSelect->line_thr_1_rb = 0;
        pSelect->line_thr_1_g = 0;
        pSelect->line_mad_fac_1_rb = 0;
        pSelect->line_mad_fac_1_g = 0;

        pSelect->pg_red_blue1_enable = 0;
        pSelect->pg_green1_enable = 0;
        pSelect->pg_fac_1_rb = 0;
        pSelect->pg_fac_1_g = 0;
        break;
    default:
        LOGE_ADPCC("%s(%d): Wrong fast mode level!!!", __FUNCTION__, __LINE__);
        break;
    }

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);

}

void Fast_mode_Single_Setting(
    dpc_param_auto_t* paut,
    Dpc_basic_params_select_t *pSelect,
    int ilow, int ihigh, float ratio)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    int level = interpolation_i32(
        paut->dyn[ilow].stFastMode.fast_mode_single_level, paut->dyn[ihigh].stFastMode.fast_mode_single_level, ratio);

    if(paut->sta.fast_mode_single_en != 0)
        pSelect->stage1_use_set_1 = 0x1;
    else
        pSelect->stage1_use_set_1 = 0x0;

    level = LIMIT_VALUE(level, FASTMODELEVELMAX, FASTMODELEVELMIN);
    Fast_mode_Single_level_Setting(pSelect, level);


    LOGD_ADPCC("%s(%d): Dpc fast mode single en:%d level:%d", __FUNCTION__, __LINE__, paut->sta.fast_mode_single_en, level);

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);

}

static XCamReturn Fast_mode_select_basic_params(
    DpcContext_t *pDpcCtx,
    Dpc_basic_params_select_t *pSelect,
    int ilow, int ihigh, float ratio)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pDpcCtx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pSelect == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    //set dpc ctrl params
    //mode 0x0000
    pSelect->stage1_enable = 1;

    if(pDpcCtx->isBlackSensor)
        pSelect->grayscale_mode = 0x1;
    else
        pSelect->grayscale_mode = 0x0;

    pSelect->enable = 0x1;

    //output_mode 0x0004
    pSelect->sw_rk_out_sel = 1;
    pSelect->sw_dpc_output_sel = 1;
    pSelect->stage1_rb_3x3 = 0;
    pSelect->stage1_g_3x3 = 0;
    pSelect->stage1_incl_rb_center = 1;
    pSelect->stage1_incl_green_center = 1;

    //set_use 0x0008
    pSelect->stage1_use_fix_set = 0x0;

    //get current fast mode single level
    Fast_mode_Single_Setting(&pDpcCtx->dpc_attrib->stAuto, pSelect, ilow, ihigh ,ratio);

    //get current fast mode double level
    Fast_mode_Double_Setting(&pDpcCtx->dpc_attrib->stAuto, pSelect, ilow, ihigh ,ratio);

    //get current fast mode triple level
    Fast_mode_Triple_Setting(&pDpcCtx->dpc_attrib->stAuto, pSelect, ilow, ihigh ,ratio);


    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ret;
}

static XCamReturn select_bpt_params(
    Dpc_bpt_params_t *pParams,
    Dpc_bpt_params_t *pSelect)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pParams == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pSelect == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    memcpy(pSelect, pParams, sizeof(Dpc_bpt_params_t));

    LOG1_ADPCC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ret;
}

static XCamReturn select_pdaf_params(
    Dpc_pdaf_params_t *pParams,
    Dpc_pdaf_params_t *pSelect)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pParams == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pSelect == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    memcpy(pSelect, pParams, sizeof(Dpc_pdaf_params_t));

    LOG1_ADPCC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ret;
}

static void Sensor_dpc_process(
    dpc_param_auto_t* paut,
    int ilow, int ihigh, float ratio)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    float max = paut->sta.Sensor_dpc_max_level;
    for (int i = 0; i < 12; i++) {
        paut->dyn[i].stSensorDpc.level_single = MIN(paut->dyn[i].stSensorDpc.level_single, max);
        paut->dyn[i].stSensorDpc.level_multiple = MIN(paut->dyn[i].stSensorDpc.level_multiple, max);
    }
        
    float sensor_dpc_level_single = interpolation_f32(
        paut->dyn[ilow].stSensorDpc.level_single, paut->dyn[ihigh].stSensorDpc.level_single, ratio);
    float sensor_dpc_level_multi = interpolation_f32(
        paut->dyn[ilow].stSensorDpc.level_multiple, paut->dyn[ihigh].stSensorDpc.level_multiple, ratio);

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
}
#endif
#ifdef RKAIQ_HAVE_DPCC_V2
XCamReturn DpcSelectParam(DpcContext_t* pDpcCtx, dpc_param_t* out, int iso) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pDpcCtx == NULL) {
        LOGE_ADPCC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    dpc_param_auto_t* paut = &pDpcCtx->dpc_attrib->stAuto;
    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0;
    float ratio = 0.0f;
    pre_interp(iso, NULL, 0, ilow, ihigh, ratio);
    uint16_t uratio = ratio * (1 << RATIO_FIXBIT);

    out->dyn.dpcProc = paut->dyn[ilow].dpcProc;
    out->dyn.dpDct_fixEngine.hw_dpcT_engine_en =
        paut->dyn[ilow].dpDct_fixEngine.hw_dpcT_engine_en;
    out->sta.hw_dpcCfg_src_fmt = paut->sta.hw_dpcCfg_src_fmt;
    if (pDpcCtx->isBlackSensor)
        out->sta.hw_dpcCfg_src_fmt = dpc_srcFmt_bw;
    else
        out->sta.hw_dpcCfg_src_fmt = dpc_srcFmt_bayer;
    out->sta.hw_dpcCfg_dpcROI_mode = paut->sta.hw_dpcCfg_dpcROI_mode;

    for (i = 0;i < 3;i++) {
        out->dyn.dpDct_cfgEngine[i].hw_dpcT_engine_en =
            paut->dyn[ilow].dpDct_cfgEngine[i].hw_dpcT_engine_en;
        // get sw_dpcCfg_engine_mode
        out->dyn.dpDct_cfgEngine[i].sw_dpcT_engine_mode =
            paut->dyn[ilow].dpDct_cfgEngine[i].sw_dpcT_engine_mode;
        out->dyn.dpDct_cfgEngine[i].sw_dpcT_defaultMode_strg =
            interpolation_u8(paut->dyn[ilow].dpDct_cfgEngine[i].sw_dpcT_defaultMode_strg,
                paut->dyn[ihigh].dpDct_cfgEngine[i].sw_dpcT_defaultMode_strg, uratio);
        switch (out->dyn.dpDct_cfgEngine[i].sw_dpcT_engine_mode) {
        case dpc_usrCfg_mode:
            out->dyn.dpDct_cfgEngine[i] = paut->dyn[ilow].dpDct_cfgEngine[i];
            break;
        case dpc_dpSingleDefault_mode:
            SingleSelectParam(&out->dyn.dpDct_cfgEngine[i],
                out->dyn.dpDct_cfgEngine[i].sw_dpcT_defaultMode_strg);
            break;
        case dpc_dpSmallClusDefault_mode:
            SmallClusterSelectParam(&out->dyn.dpDct_cfgEngine[i],
                out->dyn.dpDct_cfgEngine[i].sw_dpcT_defaultMode_strg);
            break;
        case dpc_dpBigClusDefault_mode:
            BigClusterSelectParam(&out->dyn.dpDct_cfgEngine[i], &out->dyn.dpDct_fixEngine,
                out->dyn.dpDct_cfgEngine[i].sw_dpcT_defaultMode_strg);
            break;
        default:
            LOGE_ADPCC("DPC engine mode error\n");
            return XCAM_RETURN_ERROR_PARAM;
        }
    }

    out->sta.spc.hw_dpcCfg_spc_en = paut->sta.spc.hw_dpcCfg_spc_en;
    out->sta.spc.hw_dpcCfg_win_x = paut->sta.spc.hw_dpcCfg_win_x;
    out->sta.spc.hw_dpcCfg_win_y = paut->sta.spc.hw_dpcCfg_win_y;
    out->sta.spc.hw_dpcCfg_zone_width = paut->sta.spc.hw_dpcCfg_zone_width;
    out->sta.spc.hw_dpcCfg_zone_height = paut->sta.spc.hw_dpcCfg_zone_height;
    out->sta.spc.hw_dpcCfg_zonesRow_num = paut->sta.spc.hw_dpcCfg_zonesRow_num;
    out->sta.spc.hw_dpcCfg_zonesCol_num = paut->sta.spc.hw_dpcCfg_zonesCol_num;
    for (int i = 0; i < DPC_PDAF_POINT_NUM; i++) {
        out->sta.spc.hw_dpcCfg_spcEnInZone_bit[i] =
            paut->sta.spc.hw_dpcCfg_spcEnInZone_bit[i];
        out->sta.spc.hw_dpcCfg_spInZoneCoord_x[i] =
            paut->sta.spc.hw_dpcCfg_spInZoneCoord_x[i];
        out->sta.spc.hw_dpcCfg_spInZoneCoord_y[i] =
            paut->sta.spc.hw_dpcCfg_spInZoneCoord_y[i];
    }
    out->sta.spc.hw_dpc_Cfg_spc_mode = paut->sta.spc.hw_dpc_Cfg_spc_mode;
    return ret;
}

static void SingleSelectParam(dpc_dpDct_cfgEngine_t* dpDctEngine, unsigned char strg) {
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    switch (strg)
    {
    case 1:
        //rankOrd
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x20;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x20;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x12;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x12;
        //rank
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0x20;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0x20;
        //rankDiff
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0xa;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0xa;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0x1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0x1;
        //ord
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x2;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x2;
        //edg
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0x8;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x8;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x4;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x4;
        //peak
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x8;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x8;
        break;
    case 2:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x16;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x16;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x8;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x8;

        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0x10;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0x10;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0x6;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0x6;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0x1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0x1;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x2;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x2;

        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0x16;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x16;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x10;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x10;

        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x6;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x6;
        break;
    case 3:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x16;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x16;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x8;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x8;

        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0x6;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0x6;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0x1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0x1;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x2;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x2;

        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0x16;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x16;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x10;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x10;

        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x6;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x6;
        break;
    case 4:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x16;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x16;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x8;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x8;

        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x2;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x2;

        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0x16;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x16;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x10;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x10;

        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x6;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x6;
        break;
    case 5:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x14;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x14;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0xc;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0xc;

        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x3;

        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0xc;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0xc;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x9;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x9;

        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x4;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x5;
        break;
    case 6:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x10;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x10;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x8;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x8;

        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x3;

        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0x9;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x9;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x7;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x7;

        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x4;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x5;
        break;
    case 7:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x8;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x8;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x6;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x6;

        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x3;

        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0x7;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x7;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x5;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x5;

        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x3;
        break;
    case 8:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x8;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x8;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x6;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x6;

        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x3;

        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0x7;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x7;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x5;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x5;

        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0;
        break;
    case 9:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x8;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x8;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x6;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x6;

        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x3;

        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0x3;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x3;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x2;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x2;

        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0;
        break;
    case 10:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x8;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x8;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x6;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x6;

        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x3;

        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0;

        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0;
        break;
    default:
        LOGE_ADPCC("%s(%d): Wrong fast mode level!!!", __FUNCTION__, __LINE__);
        break;
    }
}

static void SmallClusterSelectParam(dpc_dpDct_cfgEngine_t* dpDctEngine, unsigned char strg) {
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);
    switch (strg)
    {
    case 1:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x15;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x20;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x10;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x12;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0x15;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0x20;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0x9;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0xa;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0x1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0x1;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x2;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x6;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0x8;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x3;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x4;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x6;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x8;
        break;
    case 2:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x12;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x16;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x6;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x8;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0x7;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0x10;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0x5;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0x6;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0x1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0x1;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x2;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x12;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0x16;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x8;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x10;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x5;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x6;
        break;
    case 3:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x12;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x16;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x6;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x8;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0x5;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0x6;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0x1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0x1;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x2;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x12;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0x16;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x8;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x10;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x5;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x6;
        break;
    case 4:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x12;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x16;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x6;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x8;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x2;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x12;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0x16;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x8;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x10;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x5;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x6;
        break;
    case 5:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x10;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x14;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x6;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0xc;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x3;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x7;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0xc;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x7;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x9;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x3;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x4;
        break;
    case 6:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x7;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x10;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x6;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x8;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x3;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x7;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0x9;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x5;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x7;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x3;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x4;
        break;
    case 7:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x5;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x8;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x6;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x4;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x3;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x5;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0x7;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x3;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x5;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x2;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x1;
        break;
    case 8:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x5;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x8;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x6;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x3;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x5;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0x7;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x3;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x5;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0;
        break;
    case 9:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x4;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x8;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x2;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x6;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x4;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x3;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x1;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0x3;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x2;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0x2;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0;
        break;
    case 10:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x4;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x8;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x6;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x4;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x3;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0;
        break;
    default:
        LOGE_ADPCC("%s(%d): Wrong fast mode level!!!", __FUNCTION__, __LINE__);
        break;
    }
}

static void BigClusterSelectParam(dpc_dpDct_cfgEngine_t* dpDctEngine,
    dpc_dpDct_fixEngine_t* dpDct_fixEngine, unsigned char strg) {
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    switch (strg)
    {
    case 1:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x5;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x5;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x3;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 2;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 2;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 2;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 2;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x3;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x3;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x3;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x3;
        break;
    case 2:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x5;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x5;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x3;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 2;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 2;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 2;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 2;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0x2;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0x2;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x3;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x3;
        break;
    case 3:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x5;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x5;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x3;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 2;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 2;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 2;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 2;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x3;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x3;
        break;
    case 4:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x5;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x5;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x3;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 2;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 2;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 2;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 2;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0x2;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0x2;
        break;
    case 5:
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x5;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x5;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x3;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 2;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 2;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 2;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 2;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0;
        break;
    case 6:
        dpDct_fixEngine->hw_dpcT_engine_en = 0x1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x5;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x5;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x3;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 2;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 2;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 2;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 2;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0;
        break;
    case 7:
        dpDct_fixEngine->hw_dpcT_engine_en = 0x1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x5;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x5;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x3;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 2;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 2;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 2;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 2;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0;
        break;
    case 8:
        dpDct_fixEngine->hw_dpcT_engine_en = 0x1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0x3;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0x2;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0x2;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0x4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0x4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0x2;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0x2;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x2;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x2;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0;
        break;
    case 9:
        dpDct_fixEngine->hw_dpcT_engine_en = 0x1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0x4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0x4;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0x2;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0x2;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x2;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x2;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0;
        break;
    case 10:
        dpDct_fixEngine->hw_dpcT_engine_en = 0x1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit = 0;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit = 0;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale = 0;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale = 0;

        dpDctEngine->dpDct_grad.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThRB_scale = 0;
        dpDctEngine->dpDct_grad.hw_dpcT_dpGradThG_scale = 0;

        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctRB_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dctG_en = 1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread = 0x3;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_dpLumaG_thread = 0x3;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx = 0x1;
        dpDctEngine->dpDct_dpThCfg.hw_dpcT_ordDpThG_idx = 0x1;

        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en = 1;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx = 0x2;
        dpDctEngine->dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx = 0x1;

        dpDctEngine->dpDct_edg.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_offset = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThRB_scale = 0;
        dpDctEngine->dpDct_edg.hw_dpcT_dpEdgThG_scale = 0;

        dpDctEngine->dpDct_peak.hw_dpcT_dctRB_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dctG_en = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThRb_scale = 0;
        dpDctEngine->dpDct_peak.hw_dpcT_dpPeakThG_scale = 0;
        break;
    default:
        LOGE_ADPCC("%s(%d): Wrong fast mode level!!!", __FUNCTION__, __LINE__);
        break;
    }

}

#endif
XCamReturn
algo_dpc_SetAttrib(RkAiqAlgoContext* ctx, dpc_api_attrib_t* attr) {
    if (ctx == NULL || attr == NULL) {
        LOGE_ADPCC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    DpcContext_t* pDpcCtx = (DpcContext_t*)ctx;
    dpc_api_attrib_t* dpc_attrib = pDpcCtx->dpc_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ADPCC("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    dpc_attrib->opMode = attr->opMode;
    dpc_attrib->en = attr->en;
    dpc_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO)
        dpc_attrib->stAuto = attr->stAuto;
    else if (attr->opMode == RK_AIQ_OP_MODE_MANUAL)
        dpc_attrib->stMan = attr->stMan;
    else {
        LOGW_ADPCC("wrong mode: %d\n", attr->opMode);
    }

    pDpcCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
algo_dpc_GetAttrib(RkAiqAlgoContext* ctx, dpc_api_attrib_t* attr)
{
    if (ctx == NULL || attr == NULL) {
        LOGE_ADPCC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    DpcContext_t* pDpcCtx = (DpcContext_t*)ctx;
    dpc_api_attrib_t* dpc_attrib = pDpcCtx->dpc_attrib;

#if 0
    if (dpc_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ADPCC("not auto mode: %d", dpc_attrib->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }
#endif

    attr->opMode = dpc_attrib->opMode;
    attr->en = dpc_attrib->en;
    attr->bypass = dpc_attrib->bypass;
    memcpy(&attr->stAuto, &dpc_attrib->stAuto, sizeof(dpc_param_auto_t));
    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_ALGO_DPC_VERSION     "v0.0.1"
#define RKISP_ALGO_DPC_VENDOR      "Rockchip"
#define RKISP_ALGO_DPC_DESCRIPTION "Rockchip dpc algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescDpc = {
    .common = {
        .version = RKISP_ALGO_DPC_VERSION,
        .vendor = RKISP_ALGO_DPC_VENDOR,
        .description = RKISP_ALGO_DPC_DESCRIPTION,
        .type = RK_AIQ_ALGO_TYPE_ADPCC,
        .id = 0,
        .create_context = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

//RKAIQ_END_DECLARE