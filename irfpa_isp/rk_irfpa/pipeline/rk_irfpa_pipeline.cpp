#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "cJSON.h"
#include "rk_irfpa_pipeline.h"

typedef struct {
    rk_irfpa_pipeline_ctx_t ctx;

    rk_irfpa_pipeline_mode_e mode;
    int inbits;
    int image_pixels;

} pipeline_priv_t;

rk_irfpa_pipeline_ctx_t *rk_irfpa_pipeline_create_ctx(void)
{
    pipeline_priv_t *priv = new pipeline_priv_t;
    rk_irfpa_pipeline_ctx_t *ctx = &priv->ctx;

    return ctx;
}

bool pipeline_init_isphw(pipeline_priv_t *priv)
{
    bool ret = false;
    rk_irfpa_pipeline_ctx_t *ctx = &priv->ctx;
    rk_irfpa_pipeline_config_t *cfg = &ctx->cfg;
    int w = cfg->width;
    int h = cfg->height;

    rk_irfpa_log_trace(priv->ctx.log, "enter: %s", __func__);

    ctx->isphw = rk_irfpa_isphw_create_ctx();
    if (!ctx->isphw) {
        rk_irfpa_log_error(priv->ctx.log, "rk_irfpa_isphw_create_ctx failed");
        return false;
    }

    ctx->isphw->cfg.width = w;
    ctx->isphw->cfg.height = h;
    ctx->isphw->cfg.isp_media = cfg->isp_media;

    ret = rk_irfpa_isphw_init(ctx->isphw);
    if (!ret) {
        rk_irfpa_log_error(priv->ctx.log, "isphw_init error: %s", ctx->isphw->error_str);
        return false;
    }
    return true;
}

bool pipeline_init_sw_all(pipeline_priv_t *priv)
{
    rk_irfpa_pipeline_ctx_t *ctx = &priv->ctx;
    rk_irfpa_pipeline_config_t *cfg = &ctx->cfg;
    int w = cfg->width;
    int h = cfg->height;
    int bits = cfg->bits;

    rk_irfpa_log_trace(priv->ctx.log, "enter: %s", __func__);

    ctx->sw_dpc = rk_irfpa_sw_dpc_init(w, h);
    ctx->sw_decover = rk_irfpa_sw_decover_init(w, h);
    ctx->sw_nuc = rk_irfpa_sw_nuc_init(w, h);
    ctx->sw_destripe = rk_irfpa_sw_destripe_init(w, h);
    ctx->sw_tnr = rk_irfpa_sw_tnr_init(w, h);
    ctx->sw_spnr = rk_irfpa_sw_spnr_init(w, h);
    ctx->sw_sharp = rk_irfpa_sw_sharp_init(w, h);
    ctx->sw_drc = rk_irfpa_sw_drc_init(w, h, bits);
    ctx->sw_pseudo = rk_irfpa_sw_pseudo_init(w, h);

    return true;
}

bool pipeline_init_hw_tnr(pipeline_priv_t *priv)
{
    bool ret = false;
    rk_irfpa_pipeline_ctx_t *ctx = &priv->ctx;
    rk_irfpa_pipeline_config_t *cfg = &ctx->cfg;
    int w = cfg->width;
    int h = cfg->height;
    int bits = cfg->bits;

    rk_irfpa_log_trace(priv->ctx.log, "enter: %s", __func__);

    ctx->hw_tnr = rk_irfpa_hw_tnr_init(w, h);
    ctx->sw_nuc = rk_irfpa_sw_nuc_init(w, h);
    ctx->sw_destripe = rk_irfpa_sw_destripe_init(w, h);
    ctx->sw_decover = rk_irfpa_sw_decover_init(w, h);
    ctx->sw_spnr = rk_irfpa_sw_spnr_init(w, h);
    ctx->sw_sharp = rk_irfpa_sw_sharp_init(w, h);
    ctx->sw_drc = rk_irfpa_sw_drc_init(w, h, bits);
    ctx->sw_pseudo = rk_irfpa_sw_pseudo_init(w, h);

    return true;
}

bool pipeline_init_hw_all(pipeline_priv_t *priv)
{
    bool ret = false;
    rk_irfpa_pipeline_ctx_t *ctx = &priv->ctx;
    rk_irfpa_pipeline_config_t *cfg = &ctx->cfg;
    int w = cfg->width;
    int h = cfg->height;
    int bits = cfg->bits;

    rk_irfpa_log_trace(priv->ctx.log, "enter: %s", __func__);

    ctx->hw_tnr = rk_irfpa_hw_tnr_init(w, h);
    ctx->hw_sharp = rk_irfpa_hw_sharp_init(w, h);
    ctx->hw_ynr = rk_irfpa_hw_ynr_init(w, h);
    ctx->hw_enh = rk_irfpa_hw_enh_init(w, h);
    ctx->hw_histeq = rk_irfpa_hw_histeq_init(w, h);
    ctx->sw_nuc = rk_irfpa_sw_nuc_init(w, h);
    ctx->sw_dpc = rk_irfpa_sw_dpc_init(w, h);
    ctx->sw_destripe = rk_irfpa_sw_destripe_init(w, h);
    ctx->sw_decover = rk_irfpa_sw_decover_init(w, h);
    // ctx->sw_spnr = rk_irfpa_sw_spnr_init(w, h);
    ctx->sw_drc = rk_irfpa_sw_drc_init(w, h, bits);
    ctx->sw_pseudo = rk_irfpa_sw_pseudo_init(w, h);
    return true;
}

bool rk_irfpa_pipeline_init(rk_irfpa_pipeline_ctx_t *ctx)
{
    pipeline_priv_t *priv = (pipeline_priv_t *)ctx;
    rk_irfpa_pipeline_config_t *cfg = &ctx->cfg;
    bool ret = false;

    int w = cfg->width;
    int h = cfg->height;

    rk_irfpa_log_trace(priv->ctx.log, "enter: %s", __func__);

    ctx->isphw = NULL;
    ctx->sw_nuc = NULL;
    ctx->sw_dpc = NULL;
    ctx->sw_destripe = NULL;
    ctx->sw_decover = NULL;
    ctx->sw_tnr = NULL;
    ctx->sw_spnr = NULL;
    ctx->sw_sharp = NULL;
    ctx->sw_drc = NULL;
    ctx->sw_pseudo = NULL;
    ctx->hw_tnr = NULL;
    ctx->hw_sharp = NULL;
    ctx->hw_ynr = NULL;
    ctx->hw_enh = NULL;
    ctx->hw_histeq = NULL;

    ret = pipeline_init_isphw(priv);
    if (!ret) {
        return false;
    }

    if (cfg->mode == RK_IRFPA_PIPELINE_MODE_SW_ALL) {
        ret = pipeline_init_sw_all(priv);
    } else if (cfg->mode == RK_IRFPA_PIPELINE_MODE_HW_TNR) {
        ret = pipeline_init_hw_tnr(priv);
    } else if (cfg->mode == RK_IRFPA_PIPELINE_MODE_HW_ALL) {
        ret = pipeline_init_hw_all(priv);
    } else {
        rk_irfpa_log_error(priv->ctx.log, "pipeline mode %d, not support yet", cfg->mode);
        ret = false;
    }
    if (!ret)
        return false;

    ctx->res_y8 = (uint8_t *)malloc( w * h );
    ctx->res_yuv = (uint8_t *)malloc( w * h *3 / 2);

    priv->image_pixels = w * h;
    priv->mode = cfg->mode;
    priv->inbits = cfg->bits;
    return true;
}

void rk_irfpa_pipeline_deinit(rk_irfpa_pipeline_ctx_t *ctx)
{
    pipeline_priv_t *priv = (pipeline_priv_t *)ctx;
    if (ctx->sw_nuc)
        rk_irfpa_sw_nuc_deinit(ctx->sw_nuc);
    if (ctx->sw_dpc)
        rk_irfpa_sw_dpc_deinit(ctx->sw_dpc);
    if (ctx->sw_destripe)
        rk_irfpa_sw_destripe_deinit(ctx->sw_destripe);
    if (ctx->sw_decover)
        rk_irfpa_sw_decover_deinit(ctx->sw_decover);
    if (ctx->sw_tnr)
        rk_irfpa_sw_tnr_deinit(ctx->sw_tnr);
    if (ctx->sw_spnr)
        rk_irfpa_sw_spnr_deinit(ctx->sw_spnr);
    if (ctx->sw_sharp)
        rk_irfpa_sw_sharp_deinit(ctx->sw_sharp);
    if (ctx->sw_drc)
        rk_irfpa_sw_drc_deinit(ctx->sw_drc);
    if (ctx->sw_pseudo)
        rk_irfpa_sw_pseudo_deinit(ctx->sw_pseudo);
    if (ctx->isphw)
        rk_irfpa_isphw_deinit(ctx->isphw);
    if (ctx->hw_tnr)
        rk_irfpa_hw_tnr_deinit(ctx->hw_tnr);
    if (ctx->hw_sharp)
        rk_irfpa_hw_sharp_deinit(ctx->hw_sharp);
    if (ctx->hw_ynr)
        rk_irfpa_hw_ynr_deinit(ctx->hw_ynr);
    if (ctx->hw_enh)
        rk_irfpa_hw_enh_deinit(ctx->hw_enh);
    if (ctx->hw_histeq)
        rk_irfpa_hw_histeq_deinit(ctx->hw_histeq);

    if (ctx->res_y8)
        free(ctx->res_y8);
    if (ctx->res_yuv)
        free(ctx->res_yuv);
}

void pipeline_process_sw_all(pipeline_priv_t *priv, uint16_t *datain, uint32_t seq)
{
    rk_irfpa_pipeline_ctx_t *ctx = &priv->ctx;

    // move to sensorhw
    //rk_irfpa_sw_nuc_process(ctx->sw_nuc, (int16_t *)datain, (int16_t *)datain);
    rk_irfpa_sw_dpc_process(ctx->sw_dpc, (int16_t*)datain, (int16_t*)datain);
    rk_irfpa_sw_drc_stat(ctx->sw_drc, (int16_t*)datain, (int16_t*)datain);
    rk_irfpa_sw_destripe_process(ctx->sw_destripe, (int16_t*)datain, (int16_t*)datain, &ctx->sw_drc->info);
    rk_irfpa_sw_decover_process(ctx->sw_decover, (int16_t*)datain, (int16_t*)datain);
    rk_irfpa_sw_tnr_process(ctx->sw_tnr, (int16_t*)datain, (int16_t*)datain, &ctx->sw_drc->info);
    rk_irfpa_sw_spnr_process(ctx->sw_spnr, (int16_t *)datain, (int16_t *)datain, &ctx->sw_drc->info);
    rk_irfpa_sw_sharp_process(ctx->sw_sharp, (int16_t*)datain, (int16_t*)datain, &ctx->sw_drc->info);
    rk_irfpa_sw_drc_process(ctx->sw_drc, (int16_t*)datain, (int16_t*)datain);

    for (int i=0; i<priv->image_pixels; i++) {
        ctx->res_y8[i] = datain[i] / 64;
    }

    rk_irfpa_sw_pseudo_process(ctx->sw_pseudo, ctx->res_y8, ctx->res_yuv);
}

/*
void debug_save_y8(const char *name, uint8_t *data, int image_pixels, uint32_t seq)
{
    char fn[128];
    FILE *fp = NULL;
    sprintf(fn, "/tmp/%s_y8.raw", name);
    if (seq == 0) {
        printf("savefn %s\n", fn);
        fp = fopen(fn, "wb");
    } else {
        fp = fopen(fn, "ab");
    }
    fwrite(data, image_pixels, 1, fp);
    fclose(fp);
}

void debug_save_nv12(const char *name, uint8_t *data, int image_pixels, uint32_t seq)
{
    char fn[128];
    FILE *fp = NULL;
    sprintf(fn, "/tmp/%s.yuv", name);
    if (seq == 0) {
        printf("savefn %s\n", fn);
        fp = fopen(fn, "wb");
    } else {
        fp = fopen(fn, "ab");
    }
    fwrite(data, image_pixels * 3 / 2, 1, fp);
    fclose(fp);
}
*/

static void checkEnable(rk_irfpa_pipeline_ctx_t* ctx) {
    bool ynr_en = ctx->hw_ynr->params.enable;
    bool sharp_en = ctx->hw_sharp->sharp_params.enable;
    bool enh_en = ctx->hw_enh->params.enable;

    if (!(ynr_en == sharp_en && ynr_en == enh_en)) {
        ctx->hw_ynr->params.bypass = !ynr_en ? 1 : ctx->hw_ynr->params.bypass;
        ctx->hw_ynr->params.enable = 1;
        ctx->hw_sharp->sharp_params.bypass = !sharp_en ? 1 : ctx->hw_sharp->sharp_params.bypass;
        ctx->hw_sharp->sharp_params.enable = 1;
        ctx->hw_enh->params.bypass = !enh_en ? 1 : ctx->hw_enh->params.bypass;
        ctx->hw_enh->params.enable = 1;
    }
}

bool pipeline_process_hw_all(pipeline_priv_t* priv, uint16_t* frame, uint32_t seq)
{
    bool ret;
    rk_irfpa_pipeline_ctx_t *ctx = &priv->ctx;
    rk_irfpa_isphw_ctx_t* isphw = ctx->isphw;
    rk_irfpa_hw_tnr_ctx_t* tnr = ctx->hw_tnr;
    rk_irfpa_hw_sharp_ctx_t* sharp = ctx->hw_sharp;
    rk_irfpa_hw_ynr_ctx_t* ynr = ctx->hw_ynr;
    rk_irfpa_hw_enh_ctx_t* enh = ctx->hw_enh;
    rk_irfpa_hw_histeq_ctx_t* histeq = ctx->hw_histeq;

    // move to sensorhw 
    //rk_irfpa_sw_nuc_process(ctx->sw_nuc, (int16_t*)frame, (int16_t*)frame);
    if (seq == 20) {
        ctx->sw_dpc->reconfig = true;
    }
    rk_irfpa_sw_dpc_process(ctx->sw_dpc, (int16_t*)frame, (int16_t*)frame);
    rk_irfpa_sw_drc_stat(ctx->sw_drc, (int16_t*)frame, (int16_t*)frame);
    int iso = ctx->sw_drc->info.iso;

    rk_irfpa_sw_destripe_process(ctx->sw_destripe, (int16_t*)frame, (int16_t*)frame, &ctx->sw_drc->info);
    rk_irfpa_sw_decover_process(ctx->sw_decover, (int16_t*)frame, (int16_t*)frame);
    // part 1: use tnr to get mdwgt
    tnr->reconfig = true;
    sharp->reconfig = true;
    ynr->reconfig = true;
    enh->reconfig = true;
    histeq->reconfig = true;
    rk_irfpa_hw_tnr_process(tnr, isphw, frame, &ctx->sw_drc->info);
    rk_irfpa_hw_tnr_postprocess(tnr, isphw, frame);

    ynr->delaybypass = false;
    sharp->delaybypass = false;
    enh->delaybypass = false;
    histeq->delaybypass = false;
    checkEnable(ctx);
    rk_irfpa_hw_ynr_process(ynr, isphw, &ctx->sw_drc->info);
    rk_irfpa_hw_sharp_process(sharp, &ynr->params, isphw, &ctx->sw_drc->info);
    rk_irfpa_hw_enh_process(enh, isphw, &ctx->sw_drc->info);
    if(histeq->params.enable)
        histeq->params.enable = enh->params.enable & (!enh->params.bypass || enh->delaybypass);
    rk_irfpa_hw_histeq_process(histeq, isphw, &ctx->sw_drc->info);

    for (int i=0; i<priv->image_pixels; i++) {
        frame[i] = frame[i] / 4;
    }

    rk_irfpa_isphw_preProcess(isphw,seq);
    ret = rk_irfpa_isphw_process(isphw, frame, seq);
    rk_irfpa_isphw_postProcess(isphw);
    if (!ret) {
        rk_irfpa_log_error(priv->ctx.log,"isphw_process 1 error: %s", isphw->error_str);
        return false;
    }

    // part 2: use mdwgt to stack frames
    tnr->reconfig = true;
    sharp->reconfig = true;
    ynr->reconfig = true;
    enh->reconfig = true;
    histeq->reconfig = true;
    rk_irfpa_hw_tnr_process2(tnr, isphw, frame, isphw->res.y8);

    ynr->delaybypass = true;
    sharp->delaybypass = true;
    enh->delaybypass = true;
    histeq->delaybypass = true;
    rk_irfpa_hw_ynr_process(ynr, isphw, &ctx->sw_drc->info);
    rk_irfpa_hw_sharp_process(sharp, &ynr->params, isphw, &ctx->sw_drc->info);
    rk_irfpa_hw_enh_process(enh, isphw, &ctx->sw_drc->info);
    if(histeq->params.enable)
        histeq->params.enable = enh->params.enable & (!enh->params.bypass || enh->delaybypass);
    rk_irfpa_hw_histeq_process(histeq, isphw, &ctx->sw_drc->info);
    rk_irfpa_sw_drc_process(ctx->sw_drc, (int16_t*)frame, (int16_t*)frame);
    
    for (int i=0; i<priv->image_pixels; i++) {
        frame[i] = frame[i] / 4;
    }

    ret = rk_irfpa_isphw_process(isphw, frame, seq);
    if (!ret) {
        rk_irfpa_log_error(priv->ctx.log,"isphw_process 2 error: %s", isphw->error_str);
        return false;
    }

    rk_irfpa_isphw_save_stats(isphw);

    memcpy(ctx->res_y8, isphw->res.y8, priv->image_pixels * sizeof(uint8_t));

    rk_irfpa_sw_pseudo_process(ctx->sw_pseudo, ctx->res_y8, ctx->res_yuv);
    return true;
}

bool rk_irfpa_pipeline_process(rk_irfpa_pipeline_ctx_t *ctx, uint16_t *datain, uint32_t seq)
{
    bool ret = false;
    pipeline_priv_t *priv = (pipeline_priv_t *)ctx;
    rk_irfpa_pipeline_config_t *cfg = &ctx->cfg;

    if (priv->mode == RK_IRFPA_PIPELINE_MODE_SW_ALL) {
        pipeline_process_sw_all(priv, datain, seq);
        ret = true;
    } else if (cfg->mode == RK_IRFPA_PIPELINE_MODE_HW_TNR) {
        //ret = pipeline_process_sw_all(priv);
    } else if (cfg->mode == RK_IRFPA_PIPELINE_MODE_HW_ALL) {
        ret = pipeline_process_hw_all(priv, datain, seq);
    } else {
        rk_irfpa_log_error(priv->ctx.log, "pipeline mode %d, not support yet", cfg->mode);
        ret = false;
    }
    return ret;
}

cJSON* rk_irfpa_sw_nuc_params_t_to_json(rk_irfpa_sw_nuc_params_t* p);
cJSON* rk_irfpa_sw_dpc_params_t_to_json(rk_irfpa_sw_dpc_params_t* p);
cJSON* rk_irfpa_sw_destripe_params_t_to_json(rk_irfpa_sw_destripe_params_t* p);
cJSON* rk_irfpa_sw_decover_params_t_to_json(rk_irfpa_sw_decover_params_t* p);
cJSON* rk_irfpa_sw_tnr_params_t_to_json(rk_irfpa_sw_tnr_params_t* p);
cJSON* rk_irfpa_sw_spnr_params_t_to_json(rk_irfpa_sw_spnr_params_t *p);
cJSON* rk_irfpa_sw_sharp_params_t_to_json(rk_irfpa_sw_sharp_params_t *p);
cJSON* rk_irfpa_sw_drc_params_t_to_json(rk_irfpa_sw_drc_params_t *p);
cJSON* rk_irfpa_sw_pseudo_params_t_to_json(rk_irfpa_sw_pseudo_params_t *p);
cJSON* rk_irfpa_hw_tnr_params_t_to_json(rk_irfpa_hw_tnr_params_t *p);
cJSON* rk_irfpa_hw_ynr_params_t_to_json(rk_irfpa_hw_ynr_params_t *p);
cJSON* rk_irfpa_hw_sharp_params_t_to_json(rk_irfpa_hw_sharp_params_t *p);
cJSON* rk_irfpa_hw_texEst_params_t_to_json(rk_irfpa_hw_texEst_params_t* p);
cJSON* rk_irfpa_hw_enh_params_t_to_json(rk_irfpa_hw_enh_params_t* p);
cJSON* rk_irfpa_hw_histeq_params_t_to_json(rk_irfpa_hw_histeq_params_t* p);
void rk_irfpa_sw_spnr_params_t_from_json(rk_irfpa_sw_spnr_params_t* p, cJSON* root);
void rk_irfpa_sw_destripe_params_t_from_json(rk_irfpa_sw_destripe_params_t* p, cJSON* root);
void rk_irfpa_sw_decover_params_t_from_json(rk_irfpa_sw_decover_params_t* p, cJSON* root);
void rk_irfpa_sw_dpc_params_t_from_json(rk_irfpa_sw_dpc_params_t* p, cJSON* root);
void rk_irfpa_sw_nuc_params_t_from_json(rk_irfpa_sw_nuc_params_t* p, cJSON* root);
void rk_irfpa_sw_pseudo_params_t_from_json(rk_irfpa_sw_pseudo_params_t *p, cJSON *root);
void rk_irfpa_sw_sharp_params_t_from_json(rk_irfpa_sw_sharp_params_t *p, cJSON *root);
void rk_irfpa_sw_drc_params_t_from_json(rk_irfpa_sw_drc_params_t *p, cJSON *root);
void rk_irfpa_sw_tnr_params_t_from_json(rk_irfpa_sw_tnr_params_t *p, cJSON *root);
void rk_irfpa_hw_tnr_params_t_from_json(rk_irfpa_hw_tnr_params_t *p, cJSON *root);
void rk_irfpa_hw_ynr_params_t_from_json(rk_irfpa_hw_ynr_params_t *p, cJSON *root);
void rk_irfpa_hw_sharp_params_t_from_json(rk_irfpa_hw_sharp_params_t *p, cJSON *root);
void rk_irfpa_hw_texEst_params_t_from_json(rk_irfpa_hw_texEst_params_t* p, cJSON* root);
void rk_irfpa_hw_enh_params_t_from_json(rk_irfpa_hw_enh_params_t* p, cJSON* root);
void rk_irfpa_hw_histeq_params_t_from_json(rk_irfpa_hw_histeq_params_t* p, cJSON* root);

bool rk_irfpa_pipeline_dump_json(rk_irfpa_pipeline_ctx_t *ctx, const char *fn)
{
    bool ret = false;
    rk_irfpa_sw_nuc_ctx_t* sw_nuc = ctx->sw_nuc;
    rk_irfpa_sw_dpc_ctx_t* sw_dpc = ctx->sw_dpc;
    rk_irfpa_sw_destripe_ctx_t* sw_destripe = ctx->sw_destripe;
    rk_irfpa_sw_decover_ctx_t* sw_decover = ctx->sw_decover;
    rk_irfpa_sw_tnr_ctx_t* sw_tnr = ctx->sw_tnr;
    rk_irfpa_sw_spnr_ctx_t *sw_spnr = ctx->sw_spnr;
    rk_irfpa_sw_sharp_ctx_t *sw_sharp = ctx->sw_sharp;
    rk_irfpa_sw_drc_ctx_t *sw_drc = ctx->sw_drc;
    rk_irfpa_sw_pseudo_ctx_t *sw_pseudo = ctx->sw_pseudo;
    rk_irfpa_hw_tnr_ctx_t *hw_tnr = ctx->hw_tnr;
    rk_irfpa_hw_sharp_ctx_t *hw_sharp = ctx->hw_sharp;
    rk_irfpa_hw_ynr_ctx_t* hw_ynr = ctx->hw_ynr;
    rk_irfpa_hw_enh_ctx_t* hw_enh = ctx->hw_enh;
    rk_irfpa_hw_histeq_ctx_t* hw_histeq = ctx->hw_histeq;

    cJSON *root = cJSON_CreateObject();
    cJSON *irfpaisp = cJSON_CreateObject(); // 创建irfpaisp根节点
    
    // 将所有模块添加到irfpaisp节点下
    if (sw_nuc)
        cJSON_AddItemToObject(irfpaisp, "nuc", rk_irfpa_sw_nuc_params_t_to_json(&sw_nuc->params));
    if (sw_dpc)
        cJSON_AddItemToObject(irfpaisp, "dpc", rk_irfpa_sw_dpc_params_t_to_json(&sw_dpc->params));
    if (sw_destripe)
        cJSON_AddItemToObject(irfpaisp, "destripe", rk_irfpa_sw_destripe_params_t_to_json(&sw_destripe->params));
    if (sw_decover)
        cJSON_AddItemToObject(irfpaisp, "decover", rk_irfpa_sw_decover_params_t_to_json(&sw_decover->params));
    if (sw_tnr)
        cJSON_AddItemToObject(irfpaisp, "tnr", rk_irfpa_sw_tnr_params_t_to_json(&sw_tnr->params));
    if (sw_spnr)
        cJSON_AddItemToObject(irfpaisp, "spnr", rk_irfpa_sw_spnr_params_t_to_json(&sw_spnr->params));
    if (sw_sharp)
        cJSON_AddItemToObject(irfpaisp, "sharp", rk_irfpa_sw_sharp_params_t_to_json(&sw_sharp->params));

    if (hw_tnr)
        cJSON_AddItemToObject(irfpaisp, "tnr", rk_irfpa_hw_tnr_params_t_to_json(&hw_tnr->params));
    if (sw_drc)
        cJSON_AddItemToObject(irfpaisp, "drc", rk_irfpa_sw_drc_params_t_to_json(&sw_drc->params));
    if (hw_ynr)
        cJSON_AddItemToObject(irfpaisp, "ynr", rk_irfpa_hw_ynr_params_t_to_json(&hw_ynr->params));
    if (hw_sharp) {
        cJSON_AddItemToObject(irfpaisp, "sharp", rk_irfpa_hw_sharp_params_t_to_json(&hw_sharp->sharp_params));
        cJSON_AddItemToObject(irfpaisp, "texEst", rk_irfpa_hw_texEst_params_t_to_json(&hw_sharp->texEst_params));
    }
    if (hw_enh) {
        cJSON_AddItemToObject(irfpaisp, "enh", rk_irfpa_hw_enh_params_t_to_json(&hw_enh->params));
    }
    if (hw_histeq) {
        cJSON_AddItemToObject(irfpaisp, "histeq", rk_irfpa_hw_histeq_params_t_to_json(&hw_histeq->params));
    }
    if (sw_pseudo)
        cJSON_AddItemToObject(irfpaisp, "pseudo", rk_irfpa_sw_pseudo_params_t_to_json(&sw_pseudo->params));

    // 将irfpaisp节点添加到根节点
    pipeline_priv_t *priv = (pipeline_priv_t *)ctx;
    rk_irfpa_pipeline_config_t* cfg = &ctx->cfg;
    if (cfg->mode == RK_IRFPA_PIPELINE_MODE_SW_ALL) {
        cJSON_AddItemToObject(root, "irfpaisp10", irfpaisp);
    } else if (cfg->mode == RK_IRFPA_PIPELINE_MODE_HW_TNR) {
        cJSON_AddItemToObject(root, "irfpaisp11", irfpaisp);
    } else if (cfg->mode == RK_IRFPA_PIPELINE_MODE_HW_ALL) {
        cJSON_AddItemToObject(root, "irfpaisp12", irfpaisp);
    } else {
        rk_irfpa_log_error(priv->ctx.log, "pipeline mode %d, not support yet", cfg->mode);
        return false;
    }

    char* string = cJSON_Print(root);
    if (string) {
        rk_irfpa_log_info(ctx->log, "dump json to %s", fn);
        FILE *fp = fopen(fn, "w");
        fwrite(string, strlen(string), 1, fp);
        fclose(fp);
        ret = true;
    }
    cJSON_Delete(root);
    return ret;
}

bool rk_irfpa_pipeline_from_json(rk_irfpa_pipeline_ctx_t *ctx, const char *iqfile)
{
    pipeline_priv_t *priv = (pipeline_priv_t *)ctx;
    FILE* fp = fopen(iqfile, "rb");
    if(fp == NULL){
        rk_irfpa_log_error(priv->ctx.log, "can't open iqfile");
        return false;
    }
    // 获取文件大小和帧数
    fseek(fp,0,SEEK_END);
    int fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buf = new char[fsize];
    fread(buf, fsize, 1, fp);
    fclose(fp);

    cJSON *root = cJSON_Parse(buf);
    cJSON *obj = NULL;
    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            rk_irfpa_log_error(priv->ctx.log, "cJSON_Parse error %s", error_ptr);
        } else {
            rk_irfpa_log_error(priv->ctx.log, "cJSON_Parse error");
        }
        return false;
    }
    rk_irfpa_log_info(ctx->log, "load iqfile from %s\n", iqfile);

    rk_irfpa_sw_nuc_ctx_t* sw_nuc = ctx->sw_nuc;
    rk_irfpa_sw_dpc_ctx_t* sw_dpc = ctx->sw_dpc;
    rk_irfpa_sw_destripe_ctx_t* sw_destripe = ctx->sw_destripe;
    rk_irfpa_sw_decover_ctx_t* sw_decover = ctx->sw_decover;
    rk_irfpa_sw_tnr_ctx_t* sw_tnr = ctx->sw_tnr;
    rk_irfpa_sw_spnr_ctx_t *sw_spnr = ctx->sw_spnr;
    rk_irfpa_sw_sharp_ctx_t *sw_sharp = ctx->sw_sharp;
    rk_irfpa_sw_drc_ctx_t *sw_drc = ctx->sw_drc;
    rk_irfpa_sw_pseudo_ctx_t *sw_pseudo = ctx->sw_pseudo;
    rk_irfpa_hw_tnr_ctx_t *hw_tnr = ctx->hw_tnr;
    rk_irfpa_hw_sharp_ctx_t *hw_sharp = ctx->hw_sharp;
    rk_irfpa_hw_ynr_ctx_t* hw_ynr = ctx->hw_ynr;
    rk_irfpa_hw_enh_ctx_t* hw_enh = ctx->hw_enh;
    rk_irfpa_hw_histeq_ctx_t* hw_histeq = ctx->hw_histeq;

    // 首先尝试从irfpaisp节点获取数据
    rk_irfpa_pipeline_config_t* cfg = &ctx->cfg;
    cJSON* irfpaisp = NULL;
    if (cfg->mode == RK_IRFPA_PIPELINE_MODE_SW_ALL) {
        irfpaisp = cJSON_GetObjectItemCaseSensitive(root, "irfpaisp10");
    } else if (cfg->mode == RK_IRFPA_PIPELINE_MODE_HW_TNR) {
        irfpaisp = cJSON_GetObjectItemCaseSensitive(root, "irfpaisp11");
    } else if (cfg->mode == RK_IRFPA_PIPELINE_MODE_HW_ALL) {
        irfpaisp = cJSON_GetObjectItemCaseSensitive(root, "irfpaisp12");
    } else {
        rk_irfpa_log_error(priv->ctx.log, "pipeline mode %d, not support yet", cfg->mode);
        return false;
    }
    cJSON* source_obj = irfpaisp ? irfpaisp : root; // 如果存在irfpaisp节点则使用它，否则使用根节点

    if (sw_nuc) {
        obj = cJSON_GetObjectItemCaseSensitive(source_obj, "nuc");
        if (obj) {
            rk_irfpa_sw_nuc_params_t_from_json(&sw_nuc->params, obj);
        }
    }
    if (sw_dpc) {
        obj = cJSON_GetObjectItemCaseSensitive(source_obj, "dpc");
        if (obj) {
            rk_irfpa_sw_dpc_params_t_from_json(&sw_dpc->params, obj);
        }
    }
    if (sw_destripe) {
        obj = cJSON_GetObjectItemCaseSensitive(source_obj, "destripe");
        if (obj) {
            rk_irfpa_sw_destripe_params_t_from_json(&sw_destripe->params, obj);
        }
    }
    if (sw_decover) {
        obj = cJSON_GetObjectItemCaseSensitive(source_obj, "decover");
        if (obj) {
            rk_irfpa_sw_decover_params_t_from_json(&sw_decover->params, obj);
        }
    }
    if (sw_tnr) {
        obj = cJSON_GetObjectItemCaseSensitive(source_obj, "tnr");
        if (obj) {
            rk_irfpa_sw_tnr_params_t_from_json(&sw_tnr->params, obj);
        }
    }
    if (sw_spnr) {
        obj = cJSON_GetObjectItemCaseSensitive(source_obj, "spnr");
        if (obj) {
            rk_irfpa_sw_spnr_params_t_from_json(&sw_spnr->params, obj);
            sw_spnr->reconfig = true;
        }
    }
    if (sw_sharp) {
        obj = cJSON_GetObjectItemCaseSensitive(source_obj, "sharp");
        if (obj) {
            rk_irfpa_sw_sharp_params_t_from_json(&sw_sharp->params, obj);
            sw_sharp->reconfig = true;
        }
    }
    if (hw_tnr) {
        obj = cJSON_GetObjectItemCaseSensitive(source_obj, "tnr");
        if (obj) {
            rk_irfpa_hw_tnr_params_t_from_json(&hw_tnr->params, obj);
            hw_tnr->reconfig = true;
        }
    }
    if (sw_drc) {
        obj = cJSON_GetObjectItemCaseSensitive(source_obj, "drc");
        if (obj) {
            rk_irfpa_sw_drc_params_t_from_json(&sw_drc->params, obj);
        }
    }
    if (hw_ynr) {
        obj = cJSON_GetObjectItemCaseSensitive(source_obj, "ynr");
        if (obj) {
            rk_irfpa_hw_ynr_params_t_from_json(&hw_ynr->params, obj);
            hw_ynr->reconfig = true;
        }
    }
    if (hw_sharp) {
        obj = cJSON_GetObjectItemCaseSensitive(source_obj, "sharp");
        if (obj) {
            rk_irfpa_hw_sharp_params_t_from_json(&hw_sharp->sharp_params, obj);
            hw_sharp->reconfig = true;
        }
        obj = cJSON_GetObjectItemCaseSensitive(source_obj, "texEst");
        if (obj) {
            rk_irfpa_hw_texEst_params_t_from_json(&hw_sharp->texEst_params, obj);
            hw_sharp->reconfig = true;
        }
    }
    if (hw_enh) {
        obj = cJSON_GetObjectItemCaseSensitive(source_obj, "enh");
        if (obj) {
            rk_irfpa_hw_enh_params_t_from_json(&hw_enh->params, obj);
            hw_enh->reconfig = true;
        }
    }
    if (hw_histeq) {
        obj = cJSON_GetObjectItemCaseSensitive(source_obj, "histeq");
        if (obj) {
            rk_irfpa_hw_histeq_params_t_from_json(&hw_histeq->params, obj);
            hw_histeq->reconfig = true;
        }
    }
    if (sw_pseudo) {
        obj = cJSON_GetObjectItemCaseSensitive(source_obj, "pseudo");
        if (obj) {
            rk_irfpa_sw_pseudo_params_t_from_json(&sw_pseudo->params, obj);
            sw_pseudo->reconfig = true;
        }
    }

    cJSON_Delete(root);
    return true;
}
