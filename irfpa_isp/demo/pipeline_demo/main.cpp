#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include "rk_irfpa_pipeline.h"

int rawreader_setup(const char *fn, int w, int h);
uint16_t *rawreader_getFrame(void);
void rawreader_reset(void);
void debug_save_y16(const char *name, int16_t *data, uint32_t seq);
void debug_save_y8(const char *name, uint8_t *data, uint32_t seq);
void debug_save_rgb(const char *name, uint32_t *data, uint32_t seq);
void debug_save_nv12(const char *name, uint8_t *data, uint32_t seq);

int main(void) {
    bool ret = false;
    int w = 400, h = 300;
    uint32_t seq = 0;
    uint16_t *frame;

    rawreader_setup("/data/nuc_default_y16.raw", w, h);

    rk_irfpa_pipeline_ctx_t *pipeline = rk_irfpa_pipeline_create_ctx();
    pipeline->cfg.width = w;
    pipeline->cfg.height = h;
    pipeline->cfg.bits = 14;
    pipeline->cfg.mode = RK_IRFPA_PIPELINE_MODE_HW_ALL;
    pipeline->cfg.isp_media = "/dev/media3";

    ret = rk_irfpa_pipeline_init(pipeline);
    if (!ret) {
        printf("rk_irfpa_pipeline_init error: %s\n", pipeline->error_str);
        goto FAIL;
    }
    rk_irfpa_pipeline_from_json(pipeline, "/data/irfpaisp.json");

    pipeline->sw_pseudo->params.sta.color = RK_IRFPA_PSEUDO_COLOR_IRON_RED;
    pipeline->sw_pseudo->reconfig = true;
    while (1) {
        frame = rawreader_getFrame();
        if (!frame)
            break;

        printf("process %d\n", seq);
        sleep(0.5);

        rk_irfpa_pipeline_process(pipeline, frame, seq);
        debug_save_nv12("res", pipeline->res_yuv, seq);
        debug_save_y8("res", pipeline->res_y8, seq);
        seq++;

    }
    rk_irfpa_pipeline_dump_json(pipeline, "/tmp/irfpaisp.json");
FAIL:
    rk_irfpa_pipeline_deinit(pipeline);
    rawreader_reset();
    return 0;
}
