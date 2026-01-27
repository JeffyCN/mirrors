#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "rk_irfpa_isphw.h"
#include "rk_irfpa_sw_drc.h"
#include "rk_irfpa_hw_tnr.h"
#include "rk_irfpa_hw_sharp.h"
#include "rk_irfpa_hw_ynr.h"
#include "rk_irfpa_hw_enh.h"
#include "rk_irfpa_hw_histeq.h"
#include <cstring>
#include <cstdlib>

int rawreader_setup(const char* fn, int w, int h);
uint16_t *rawreader_getFrame(void);
void rawreader_reset(void);
void debug_save_y16(const char *name, int16_t *data, uint32_t seq);
void debug_save_y8(const char *name, uint8_t *data, uint32_t seq);
void debug_save_rgb(const char *name, uint32_t *data, uint32_t seq);

int main(void) {
    bool ret = false;
    int w = 400, h = 300;
    uint16_t* frame = NULL;
    uint32_t seq = 0;
    rk_irfpa_isphw_ctx_t* ctx = rk_irfpa_isphw_create_ctx();
    rk_irfpa_hw_tnr_ctx_t* tnr = rk_irfpa_hw_tnr_init(w, h);
    rk_irfpa_hw_sharp_ctx_t* sharp = rk_irfpa_hw_sharp_init(w, h);
    rk_irfpa_hw_ynr_ctx_t* ynr = rk_irfpa_hw_ynr_init(w, h);
    rk_irfpa_sw_drc_ctx_t* drc;
    drc = rk_irfpa_sw_drc_init(w, h, 14);
    rk_irfpa_hw_enh_ctx_t* enh = rk_irfpa_hw_enh_init(w, h);
    rk_irfpa_hw_histeq_ctx_t* histeq = rk_irfpa_hw_histeq_init(w, h);

    ctx->cfg.width = w;
    ctx->cfg.height = h;
    ctx->cfg.isp_media = "/dev/media3";
    ret = rk_irfpa_isphw_init(ctx);
    if (!ret) {
        printf("rk_irfpa_isphw_init error: %s\n", ctx->error_str);
        goto FAIL;
    }
    rawreader_setup("/data/destrip2_default_y16.raw", w, h);

    while (1) {
        frame = rawreader_getFrame();
        if (!frame)
            break;

        printf("process %d\n", seq);

        // part 1: use tnr to get mdwgt
        tnr->reconfig = true;
        sharp->reconfig = true;
        ynr->reconfig = true;
        enh->reconfig = true;
        histeq->reconfig = true;
        rk_irfpa_hw_tnr_process(tnr, ctx, frame);
        rk_irfpa_hw_tnr_postprocess(tnr, ctx, frame);

        ynr->params.bypass = false;
        sharp->sharp_params.bypass = false;
        enh->params.bypass = false;
        histeq->params.bypass = false;
        rk_irfpa_hw_ynr_process(ynr, ctx);
        rk_irfpa_hw_sharp_process(sharp, &ynr->params, ctx);
        rk_irfpa_hw_enh_process(enh, ctx);
        histeq->params.enable = enh->params.enable  & (!enh->params.bypass);
        rk_irfpa_hw_histeq_process(histeq, ctx);

        for (int i = 0; i < w * h; i++) {
            frame[i] = frame[i] / 4;
        }
        rk_irfpa_isphw_preProcess(ctx,seq);
        ret = rk_irfpa_isphw_process(ctx, frame, seq);
        rk_irfpa_isphw_postProcess(ctx);
        if (!ret) {
            printf("rk_irfpa_isphw_process error: %s\n", ctx->error_str);
            goto FAIL;
        }
        // debug_save_y16("res", (int16_t*)ctx->res.iir, seq);
        debug_save_y8("mdwgt", ctx->res.y8, seq); 

        // part 2: use mdwgt to stack frames
        // seq++;
        tnr->reconfig = true;
        sharp->reconfig = true;
        ynr->reconfig = true;
        enh->reconfig = true;
        histeq->reconfig = true;
        rk_irfpa_hw_tnr_process2(tnr, ctx, frame, ctx->res.y8);
        ynr->params.bypass = true;
        sharp->sharp_params.bypass = true;
        enh->params.bypass = true;
        histeq->params.bypass = true;
        rk_irfpa_hw_ynr_process(ynr, ctx);
        rk_irfpa_hw_sharp_process(sharp, &ynr->params, ctx);
        rk_irfpa_hw_enh_process(enh, ctx);
        histeq->params.enable = enh->params.enable  & (!enh->params.bypass);
        rk_irfpa_hw_histeq_process(histeq, ctx);

        rk_irfpa_sw_drc_process(drc, (int16_t*)frame, (int16_t*)frame);

        for (int i = 0; i < w * h; i++) {
            frame[i] = frame[i] / 4;
        }
        debug_save_y16("btnr+map", (int16_t*)frame, seq);

        ret = rk_irfpa_isphw_process(ctx, frame, seq);

        if (!ret) {
            printf("rk_irfpa_isphw_process error: %s\n", ctx->error_str);
            goto FAIL;
        }

        ret = rk_irfpa_isphw_save_stats(ctx);
        debug_save_y8("res", ctx->res.y8, seq);

        sleep(0.1);
        seq+=1;
    }

    

FAIL:
    rawreader_reset();
    rk_irfpa_isphw_deinit(ctx);
    rk_irfpa_hw_tnr_deinit(tnr);
    rk_irfpa_sw_drc_deinit(drc);

    return 0;
}
