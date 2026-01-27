#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include "rk_irfpa_log.h"
#include "rk_irfpa_simple_api.h"
#include "rk_irfpa_sensorhw.h"
#include "rk_irfpa_pipeline.h"

typedef struct {
    rk_irfpa_simple_api_ctx_t ctx;
    rk_irfpa_streamhw_ctx_t *streamhw;
    rk_irfpa_pipeline_ctx_t *pipeline;
    rk_irfpa_sensorhw_ctx_t *sensorhw;
    rk_irfpa_log_ctx_t *log;
    bool exit = false;
    pthread_t main_thread;
    uint32_t process_seq;
} simple_api_priv_t;

rk_irfpa_simple_api_ctx_t* rk_irfpa_simple_api_create_ctx(void)
{
    simple_api_priv_t *priv = new simple_api_priv_t;
    rk_irfpa_simple_api_ctx_t *ctx = &priv->ctx;
    return ctx;
}

static void *rk_irfpa_simple_loop(void *arg) {
    bool ret;
    simple_api_priv_t *priv = (simple_api_priv_t *)arg;
    rk_irfpa_pipeline_ctx_t *pipeline = priv->pipeline;
    rk_irfpa_streamhw_ctx_t *streamhw = priv->streamhw;

    while (!priv->exit) {

        ret = rk_irfpa_sensorhw_process(priv->sensorhw);
        if (!ret) {
            if (priv->exit)
                break;
            priv->ctx.ok = false;
            break;
        }

        if (priv->process_seq % 100 == 0)
            rk_irfpa_log_info(priv->log, "simple_loop seq %d", priv->process_seq);

        rk_irfpa_pipeline_process(pipeline, streamhw->res.y16, priv->process_seq++);

        if (streamhw->info.output.pixelformat == V4L2_PIX_FMT_GREY) {
            rk_irfpa_streamhw_irfparx(streamhw, pipeline->res_y8);
        } else {
            rk_irfpa_streamhw_irfparx(streamhw, pipeline->res_yuv);
        }
    }
    return NULL;
}

bool rk_irfpa_simple_api_init(rk_irfpa_simple_api_ctx_t *ctx)
{
    bool ret;
    int w, h, bits;
    int rc;
    simple_api_priv_t *priv = (simple_api_priv_t *)ctx;

    rk_irfpa_streamhw_ctx_t *streamhw = rk_irfpa_streamhw_create_ctx();
    rk_irfpa_sensorhw_ctx_t *sensorhw = rk_irfpa_sensorhw_create_ctx();
    rk_irfpa_pipeline_ctx_t *pipeline = rk_irfpa_pipeline_create_ctx();
    rk_irfpa_log_ctx_t *log = rk_irfpa_log_create_ctx();
    priv->log = log;

    log->cfg.use_stdio = true;
    log->cfg.stdio_color = true;
    log->cfg.level = RK_IRFPA_LOG_LEVEL_INFO;
    switch (ctx->cfg.log_mode) {
    case RK_IRFPA_SIMPLE_API_LOG_QUIET:
        log->cfg.use_stdio = false;
        break;
    case RK_IRFPA_SIMPLE_API_LOG_ERROR:
        log->cfg.level = RK_IRFPA_LOG_LEVEL_ERROR;
        break;
    case RK_IRFPA_SIMPLE_API_LOG_DEBUG:
        log->cfg.level = RK_IRFPA_LOG_LEVEL_DEBUG;
        break;
    case RK_IRFPA_SIMPLE_API_LOG_TRACE:
        log->cfg.level = RK_IRFPA_LOG_LEVEL_TRACE;
        break;
    }
    rk_irfpa_log_init(log);

    streamhw->cfg.ooc_media = ctx->cfg.ooc_media;
    streamhw->cfg.vicap_media = ctx->cfg.vicap_media;
    streamhw->log = log;
    ret = rk_irfpa_streamhw_init(streamhw);
    if (!ret) {
        goto FAIL;
    }

    ret = rk_irfpa_sensorhw_init(sensorhw, streamhw);
    if (!ret) {
        goto FAIL;
    }

    w = streamhw->info.vi_width;
    h = streamhw->info.vi_height;
    bits = streamhw->info.vi_bits;

    pipeline->cfg.width = w;
    pipeline->cfg.height = h;
    pipeline->cfg.bits = bits;
    if (ctx->cfg.isp_mode == RK_IRFPA_SIMPLE_API_HW_ISP)
        pipeline->cfg.mode = RK_IRFPA_PIPELINE_MODE_HW_ALL;
    else
        pipeline->cfg.mode = RK_IRFPA_PIPELINE_MODE_SW_ALL;
    pipeline->cfg.isp_media = ctx->cfg.isp_media;
    ret = rk_irfpa_pipeline_init(pipeline);
    if (!ret) {
        goto FAIL;
    }

    priv->streamhw = streamhw;
    priv->sensorhw = sensorhw;
    priv->pipeline = pipeline;

    if (ctx->cfg.iqfile) {
        rk_irfpa_pipeline_from_json(pipeline, ctx->cfg.iqfile);
    }
    rk_irfpa_pipeline_dump_json(pipeline, "/tmp/irfpaisp.json");

    rc = pthread_create(&priv->main_thread, NULL, rk_irfpa_simple_loop, priv);
    if (rc) {
        rk_irfpa_log_error(priv->log, "simple_api thread create failed");
        goto FAIL;
    }
    return true;
FAIL:
    return false;
}

void rk_irfpa_simple_api_deinit(rk_irfpa_simple_api_ctx_t *ctx)
{
    simple_api_priv_t *priv = (simple_api_priv_t *)ctx;

    priv->exit = true;
    rk_irfpa_streamhw_stop(priv->streamhw);
    pthread_join(priv->main_thread, NULL);

    rk_irfpa_streamhw_deinit(priv->streamhw);

    rk_irfpa_pipeline_deinit(priv->pipeline);
    delete priv;
}

bool rk_irfpa_simple_api_start(rk_irfpa_simple_api_ctx_t *ctx)
{
    simple_api_priv_t *priv = (simple_api_priv_t *)ctx;
    priv->process_seq = 0;
    bool ret = rk_irfpa_streamhw_start(priv->streamhw);
    if (!ret) {
        return false;
    }
    return true;
}

bool rk_irfpa_simple_api_stop(rk_irfpa_simple_api_ctx_t *ctx)
{
    simple_api_priv_t *priv = (simple_api_priv_t *)ctx;
    rk_irfpa_streamhw_stop(priv->streamhw);
    return true;
}
