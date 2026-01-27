#include <stdio.h>
#include "rk_irfpa_streamhw.h"
#include "rk_irfpa_sensorhw.h"

void debug_save_x16(const char *name, uint16_t *data, int image_pixels, uint32_t seq)
{
    char fn[128];
    FILE *fp = NULL;
    sprintf(fn, "/tmp/%s_x16.raw", name);
    if (seq == 0) {
        printf("savefn %s\n", fn);
        fp = fopen(fn, "wb");
    } else {
        fp = fopen(fn, "ab");
    }
    fwrite(data, image_pixels, 2, fp);
    fclose(fp);
}

int main(void) {
    bool ret = false;
    int cnt = 0;
    rk_irfpa_streamhw_ctx_t *streamhw = rk_irfpa_streamhw_create_ctx();
    rk_irfpa_sensorhw_ctx_t *sensorhw = rk_irfpa_sensorhw_create_ctx();
    rk_irfpa_log_ctx_t *log = rk_irfpa_log_create_ctx();
    
    log->cfg.use_stdio = true;
    log->cfg.stdio_color = true;
    log->cfg.level = RK_IRFPA_LOG_LEVEL_TRACE;
    rk_irfpa_log_init(log);

    streamhw->cfg.ooc_media = "/dev/media0";
    streamhw->cfg.vicap_media = "/dev/media1";
    streamhw->log = log;

    ret = rk_irfpa_streamhw_init(streamhw);
    if (!ret) {
        goto FAIL;
    }

    ret = rk_irfpa_sensorhw_init(sensorhw, streamhw);
    if (!ret) {
        goto FAIL;
    }

    ret = rk_irfpa_streamhw_start(streamhw);
    if (!ret) {
        goto FAIL;
    }

    while (1) {
        ret = rk_irfpa_sensorhw_process(sensorhw);
        if (!ret) {
            break;
        }
        printf("getFrame seq %d\n", streamhw->res.seq);

        debug_save_x16("origin", streamhw->res.y16, streamhw->info.vi_width * streamhw->info.vi_height, cnt);
        cnt++;
        if (cnt > 20)
            break;
    }

    rk_irfpa_streamhw_stop(streamhw);
FAIL:
    rk_irfpa_sensorhw_deinit(sensorhw);
    rk_irfpa_streamhw_deinit(streamhw);
    rk_irfpa_log_deinit(log);
    return 0;
}
