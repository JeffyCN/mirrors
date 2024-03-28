/*
 * rk_aiq_algo_amd_itf.c
 *
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

#include "amd/rk_aiq_algo_amd_itf.h"
#include "xcam_log.h"
#include "amd/rk_aiq_types_algo_amd_prvt.h"
#include "md_lib/motion_detect.h"
#include "media_buffer/media_buffer_pool.h"
#include "rk_aiq_algo_types.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

static uint8_t static_ratio_l;
static uint8_t static_ratio_l_bit;
static uint8_t static_ratio_r_bit;
static int gain_blk_isp_stride;

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_AMD( "%s: create amd context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    memset(ctx, 0, sizeof(RkAiqAlgoContext));
    ctx->amdCtx.calib = cfg->calib;
    ctx->amdCtx.calibv2 = cfg->calibv2;

    if (ctx->amdCtx.calib) {
        CalibDb_MFNR_t *calib_mfnr =
            (CalibDb_MFNR_t*)(CALIBDB_GET_MODULE_PTR(ctx->amdCtx.calib, mfnr));
        if (!calib_mfnr)
           LOGE_AMD( "%s: get  calib_mfnr fail!\n", __FUNCTION__);
    } else if (ctx->amdCtx.calibv2) {
        CalibDbV2_MFNR_t* calibv2_mfnr =
                (CalibDbV2_MFNR_t*)(CALIBDBV2_GET_MODULE_PTR(ctx->amdCtx.calibv2, mfnr_v1));
        if (calibv2_mfnr) {
            ctx->amdCtx.enable = calibv2_mfnr->TuningPara.motion_detect_en;
            ctx->amdCtx.motion = (CalibDbV2_MFNR_TuningPara_Motion_t)calibv2_mfnr->TuningPara.Motion;
        }else {
          LOGE_AMD( "%s: get  calibv2_mfnr fail!\n", __FUNCTION__);
        }
    }

    *context = ctx;

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    if (context->amdCtx.pPreAlpha)
        free(context->amdCtx.pPreAlpha);
    if (context->amdCtx.pTmpBuf)
        free(context->amdCtx.pTmpBuf);

    for (int i=0; i<AMD_RATIO_BUF_NUM; i++) {
        if (context->amdCtx.ratio_out[i])
            free(context->amdCtx.ratio_out[i]);
    }
    if (context->amdCtx.spImage_prev)
        context->amdCtx.spImage_prev->unref(context->amdCtx.spImage_prev);//release prev image
    if (context->amdCtx.ispGain_prev)
        context->amdCtx.ispGain_prev->unref(context->amdCtx.ispGain_prev);//release prev gain

    MediaBufPoolDestroy(&context->amdCtx.BufPool);
    if (context->amdCtx.BufPoolMem.pMetaDataMemory)
        free(context->amdCtx.BufPoolMem.pMetaDataMemory);

    delete context;
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    RkAiqAlgoConfigAmd* pCfgParam = (RkAiqAlgoConfigAmd*)params;
    RkAiqAlgoContext* ctx = params->ctx;

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        if (pCfgParam->com.u.prepare.calib) {
            CalibDb_MFNR_t *calib_mfnr =
                (CalibDb_MFNR_t*)(CALIBDB_GET_MODULE_PTR(pCfgParam->com.u.prepare.calib, mfnr));
            if (!calib_mfnr)
               LOGE_AMD( "%s: get calib_mfnr fail!\n", __FUNCTION__);
        } else if (pCfgParam->com.u.prepare.calibv2) {
            CalibDbV2_MFNR_t* calibv2_mfnr =
                    (CalibDbV2_MFNR_t*)(CALIBDBV2_GET_MODULE_PTR(pCfgParam->com.u.prepare.calibv2, mfnr_v1));
            if (calibv2_mfnr) {
                ctx->amdCtx.enable = calibv2_mfnr->TuningPara.motion_detect_en;
                ctx->amdCtx.motion = (CalibDbV2_MFNR_TuningPara_Motion_t)calibv2_mfnr->TuningPara.Motion;
            }else {
              LOGE_AMD( "%s: get calibv2_mfnr fail!\n", __FUNCTION__);
            }
        }
    }

    if (!ctx->amdCtx.enable)
        return XCAM_RETURN_NO_ERROR;

    ctx->amdCtx.imgAlignedW = pCfgParam->spAlignedW;
    ctx->amdCtx.imgAlignedH = pCfgParam->spAlignedH;
    ctx->amdCtx.imgWidth = pCfgParam->spWidth;
    ctx->amdCtx.imgHeight = pCfgParam->spHeight;

    int gain_blk_isp_w                      = ctx->amdCtx.imgWidth;
    gain_blk_isp_stride                     = ((gain_blk_isp_w + 15) / 16) * 16;
    int gain_blk_isp_h                      = ctx->amdCtx.imgHeight;
    int ratio_stride                        = ((gain_blk_isp_w + 7) / 8) * 8;
    int gain_kg_tile_h_align                = (gain_blk_isp_h + 15) & 0xfff0;

    static_ratio_l_bit                  = RATIO_BITS_NUM;
    static_ratio_r_bit                  = RATIO_BITS_R_NUM;
    static_ratio_l                      = 1 << static_ratio_l_bit;


    ctx->amdCtx.ratio_size = ratio_stride * gain_kg_tile_h_align;
	ctx->amdCtx.pPreAlpha = (uint8_t*)malloc(ctx->amdCtx.ratio_size);
    memset(ctx->amdCtx.pPreAlpha, 0, ctx->amdCtx.ratio_size);

    ctx->amdCtx.pTmpBuf = (short*)malloc(gain_blk_isp_w * gain_blk_isp_h * 6 * sizeof(ctx->amdCtx.pTmpBuf[0]));

    for (int i=0; i<AMD_RATIO_BUF_NUM; i++) {
        ctx->amdCtx.ratio_out[i] = (uint8_t*)malloc(ctx->amdCtx.ratio_size);
        memset(ctx->amdCtx.ratio_out[i], static_ratio_l, ctx->amdCtx.ratio_size);
    }
    //create buffer pool
    ctx->amdCtx.BufPoolConfig.bufNum = AMD_RATIO_BUF_NUM;
    ctx->amdCtx.BufPoolConfig.maxBufNum = AMD_RATIO_BUF_NUM;
    ctx->amdCtx.BufPoolConfig.bufSize = ctx->amdCtx.ratio_size;
    ctx->amdCtx.BufPoolConfig.bufAlign = 1;
    ctx->amdCtx.BufPoolConfig.metaDataSizeMediaBuf = sizeof(MotionBufMetaData_t);
    ctx->amdCtx.BufPoolConfig.flags = 0;

    MediaBufPoolGetSize(&ctx->amdCtx.BufPoolConfig);

    ctx->amdCtx.BufPoolMem.pMetaDataMemory = malloc(ctx->amdCtx.BufPoolConfig.metaDataMemSize);
    ctx->amdCtx.BufPoolMem.pBufferMemory = (void *)ctx->amdCtx.ratio_out;
    MediaBufPoolCreate(&ctx->amdCtx.BufPool, &ctx->amdCtx.BufPoolConfig, ctx->amdCtx.BufPoolMem);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn select_motion_params_by_iso(Mt_Params_Select_t *motion_params_selected, uint32_t iso, CalibDbV2_MFNR_TuningPara_Motion_t &motion_full_params)
{
    float gain                              = iso / 50;
    float gain_f                            = log2(gain);
    uint8_t gain_l                          = ceil(gain_f);
    uint8_t gain_r                          = floor(gain_f);
    float ratio                             = (float)(gain_f - gain_r);

    motion_params_selected->enable              = true;//motion_full_params.enable;
    motion_params_selected->sigmaHScale         = (motion_full_params.Motion_ISO[gain_l].sigmaHScale        * ratio + motion_full_params.Motion_ISO[gain_r].sigmaHScale        * (1 - ratio));
    motion_params_selected->sigmaLScale         = (motion_full_params.Motion_ISO[gain_l].sigmaLScale        * ratio + motion_full_params.Motion_ISO[gain_r].sigmaLScale        * (1 - ratio));
    motion_params_selected->light_clp           = (motion_full_params.Motion_ISO[gain_l].lightClp           * ratio + motion_full_params.Motion_ISO[gain_r].lightClp           * (1 - ratio));
    motion_params_selected->uv_weight           = (motion_full_params.Motion_ISO[gain_l].uvWeight           * ratio + motion_full_params.Motion_ISO[gain_r].uvWeight           * (1 - ratio));
    motion_params_selected->yuvnr_gain_scale[0] = (motion_full_params.Motion_ISO[gain_l].yuvnrGainScale0    * ratio + motion_full_params.Motion_ISO[gain_r].yuvnrGainScale0    * (1 - ratio));
    motion_params_selected->yuvnr_gain_scale[1] = (motion_full_params.Motion_ISO[gain_l].yuvnrGainScale1    * ratio + motion_full_params.Motion_ISO[gain_r].yuvnrGainScale1    * (1 - ratio));
    motion_params_selected->yuvnr_gain_scale[2] = (motion_full_params.Motion_ISO[gain_l].yuvnrGainScale2    * ratio + motion_full_params.Motion_ISO[gain_r].yuvnrGainScale2    * (1 - ratio));
    motion_params_selected->frame_limit_y       = (motion_full_params.Motion_ISO[gain_l].frame_limit_y      * ratio + motion_full_params.Motion_ISO[gain_r].frame_limit_y      * (1 - ratio));
    motion_params_selected->frame_limit_uv      = (motion_full_params.Motion_ISO[gain_l].frame_limit_uv     * ratio + motion_full_params.Motion_ISO[gain_r].frame_limit_uv     * (1 - ratio));
    motion_params_selected->mfnr_sigma_scale    = (motion_full_params.Motion_ISO[gain_l].mfnrSigmaScale     * ratio + motion_full_params.Motion_ISO[gain_r].mfnrSigmaScale     * (1 - ratio));
    if(motion_params_selected->mfnr_sigma_scale > 0)
        static_ratio_r_bit = static_ratio_l_bit - ceil(log2(motion_params_selected->mfnr_sigma_scale));
        //static_ratio_r_bit = static_ratio_l_bit - ceil(log2(motion_params_selected->mfnr_sigma_scale)) - ceil(log2(motion_params_selected->motion_dn_str));
    else
        LOGE_AMD("motion_params_selected->mfnr_sigma_scale %d is out of range\n", motion_params_selected->mfnr_sigma_scale);

//    motion_params_selected->gain_scale_l_y      = (motion_full_params.Motion_ISO[gain_l].reserved7          * ratio + motion_full_params.Motion_ISO[gain_r].reserved7           * (1 - ratio));
//    motion_params_selected->gain_scale_l_uv     = (motion_full_params.Motion_ISO[gain_l].reserved6          * ratio + motion_full_params.Motion_ISO[gain_r].reserved6           * (1 - ratio));
//    motion_params_selected->gain_scale_h_y      = (motion_full_params.Motion_ISO[gain_l].reserved5          * ratio + motion_full_params.Motion_ISO[gain_r].reserved5           * (1 - ratio));
//    motion_params_selected->gain_scale_h_uv     = (motion_full_params.Motion_ISO[gain_l].reserved4          * ratio + motion_full_params.Motion_ISO[gain_r].reserved4           * (1 - ratio));
    LOGD_AMD("selected:gain_r %d gain_l:%d iso %d ratio %f, %f,%f,%f,%f,%f,%f,%f %f %6f %2f %2f %2f %2f %d\n",gain_r,gain_l,iso, ratio,
    motion_params_selected->sigmaHScale,motion_params_selected->sigmaLScale ,motion_params_selected->light_clp,motion_params_selected->uv_weight,
    motion_params_selected->yuvnr_gain_scale[0],motion_params_selected->yuvnr_gain_scale[1],motion_params_selected->yuvnr_gain_scale[2],
    motion_params_selected->frame_limit_y,motion_params_selected->frame_limit_uv, motion_params_selected->gain_scale_l_y, motion_params_selected->gain_scale_l_uv,
    motion_params_selected->gain_scale_h_y, motion_params_selected->gain_scale_h_uv, 0);
    return XCAM_RETURN_NO_ERROR;
}



static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    RkAiqAlgoProcResAmd* res = (RkAiqAlgoProcResAmd*)outparams;
    RkAiqAlgoContext* ctx = inparams->ctx;
    RkAiqAlgoProcAmd *pAmdProcParams = (RkAiqAlgoProcAmd*)inparams;
    rk_aiq_amd_algo_stat_t *stats = &pAmdProcParams->stats;
    int ISO = 50;

    if (!ctx->amdCtx.enable)
        return XCAM_RETURN_NO_ERROR;


    return XCAM_RETURN_NO_ERROR;//test


    if (!stats->ispGain && !stats->spImage) //first time coming without value, filter it.
        return XCAM_RETURN_NO_ERROR;

    if (ctx->amdCtx.ispGain_prev == NULL) {
        ctx->amdCtx.ispGain_prev = stats->ispGain;
        ctx->amdCtx.ispGain_prev->ref(ctx->amdCtx.ispGain_prev);
        LOGD_AMD("ispGain>>>frame_id=%d,fd=%d\n",ctx->amdCtx.ispGain_prev->frame_id, ctx->amdCtx.ispGain_prev->get_fd(ctx->amdCtx.ispGain_prev));
        ctx->amdCtx.spImage_prev = stats->spImage;
        ctx->amdCtx.spImage_prev->ref(ctx->amdCtx.spImage_prev);
        LOGD_AMD("spImage>>>frame_id=%d,fd=%d\n",ctx->amdCtx.spImage_prev->frame_id, ctx->amdCtx.spImage_prev->get_fd(ctx->amdCtx.spImage_prev));
        return XCAM_RETURN_NO_ERROR;
    }
    if (ctx->amdCtx.ispGain == NULL) {
        ctx->amdCtx.ispGain = stats->ispGain;
        ctx->amdCtx.ispGain->ref(ctx->amdCtx.ispGain);
        LOGD_AMD("ispGain>>>frame_id=%d,fd=%d\n",ctx->amdCtx.ispGain->frame_id, ctx->amdCtx.ispGain->get_fd(ctx->amdCtx.ispGain));
        ctx->amdCtx.spImage = stats->spImage;
        ctx->amdCtx.spImage->ref(ctx->amdCtx.spImage);
        LOGD_AMD("spImage>>>frame_id=%d,fd=%d\n",ctx->amdCtx.spImage->frame_id, ctx->amdCtx.spImage->get_fd(ctx->amdCtx.spImage));
    }

    Mt_Params_Select_t mparams_selected;
    select_motion_params_by_iso(&mparams_selected, ISO, ctx->amdCtx.motion);

    assert ( ctx->amdCtx.ispGain && ctx->amdCtx.ispGain_prev && ctx->amdCtx.spImage && ctx->amdCtx.spImage_prev );
    uint8_t *pCurGain = ctx->amdCtx.ispGain->map(ctx->amdCtx.ispGain);
    uint8_t *pPreGain = ctx->amdCtx.ispGain_prev->map(ctx->amdCtx.ispGain_prev);
	uint8_t *pCurIn = ctx->amdCtx.spImage->map(ctx->amdCtx.spImage);
	uint8_t *pPreIn = ctx->amdCtx.spImage_prev->map(ctx->amdCtx.spImage_prev);
    assert ( pCurGain && pPreGain && pCurIn && pPreIn );

    int imgStride = XCAM_ALIGN_UP(ctx->amdCtx.imgAlignedW, 32);
    LOGD_AMD("motion detect start,%d,%d",ctx->amdCtx.imgAlignedH,ctx->amdCtx.imgAlignedW);
    MediaBuffer_t* mediabuf = MediaBufPoolGetBuffer(&ctx->amdCtx.BufPool);
    if (mediabuf) {
        LOGD_AMD("MediaBufPool get free buffer, baseaddress %p",mediabuf->pBaseAddress);
        MotionBufMetaData_t *metadata = (MotionBufMetaData_t *)mediabuf->pMetaData;
        metadata->frame_id = ctx->amdCtx.spImage->frame_id;
        motion_detect(pCurIn, pPreIn, ctx->amdCtx.pTmpBuf, mediabuf->pBaseAddress, ctx->amdCtx.pPreAlpha, (uint8_t*)pCurGain,
                      ctx->amdCtx.imgAlignedH, imgStride, ctx->amdCtx.imgHeight, ctx->amdCtx.imgWidth, gain_blk_isp_stride,
                      mparams_selected.sigmaHScale, mparams_selected.sigmaLScale, mparams_selected.uv_weight, mparams_selected.light_clp, static_ratio_r_bit);
    }
    LOGD_AMD("motion detect end");

	ctx->amdCtx.spImage->unmap(ctx->amdCtx.spImage);
	ctx->amdCtx.spImage_prev->unmap(ctx->amdCtx.spImage_prev);
    ctx->amdCtx.ispGain->unmap(ctx->amdCtx.ispGain);
    ctx->amdCtx.ispGain_prev->unmap(ctx->amdCtx.ispGain_prev);
    //switch image
    ctx->amdCtx.spImage_prev->unref(ctx->amdCtx.spImage_prev);//release prev image
    ctx->amdCtx.ispGain_prev->unref(ctx->amdCtx.ispGain_prev);//release prev gain
    ctx->amdCtx.spImage_prev = ctx->amdCtx.spImage;
    ctx->amdCtx.ispGain_prev = ctx->amdCtx.ispGain;
    ctx->amdCtx.spImage = NULL;
    ctx->amdCtx.ispGain = NULL;

#if 0
    //output result
    int prev_index = (ctx->amdCtx.cur_index -1 + AMD_RATIO_BUF_NUM) % AMD_RATIO_BUF_NUM;
    ctx->amdCtx.params.ratio_prev = ctx->amdCtx.ratio_out[prev_index];//first time it's not correct,but just first frame only, it's okay.
    ctx->amdCtx.params.ratio_cur = ctx->amdCtx.ratio_out[ctx->amdCtx.cur_index];
    ctx->amdCtx.params.ratio_size = ctx->amdCtx.ratio_size;
    res_com->amd_proc_res = ctx->amdCtx.params;
    ctx->amdCtx.cur_index = (ctx->amdCtx.cur_index + 1) % AMD_RATIO_BUF_NUM;
#endif
    ctx->amdCtx.params.st_ratio = mediabuf;
    res->amd_proc_res = ctx->amdCtx.params;
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAmd = {
    .common = {
        .version = RKISP_ALGO_AMD_VERSION,
        .vendor  = RKISP_ALGO_AMD_VENDOR,
        .description = RKISP_ALGO_AMD_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AMD,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
