#include <stdio.h>
#include "rk_irfpa_streamhw.h"
#include "rk_irfpa_log.h"

int main(void) {
    bool ret = false;
    int cnt = 0;
    rk_irfpa_streamhw_ctx_t *ctx = rk_irfpa_streamhw_create_ctx();
    rk_irfpa_log_ctx_t *log = rk_irfpa_log_create_ctx();

    log->cfg.use_stdio = true;
    log->cfg.stdio_color = true;
    log->cfg.level = RK_IRFPA_LOG_LEVEL_INFO;
    rk_irfpa_log_init(log);
    
    ctx->cfg.ooc_media = "/dev/media0";
    ctx->cfg.vicap_media = "/dev/media1";
    ctx->log = log;

    ret = rk_irfpa_streamhw_init(ctx);
    if (!ret) {
        goto FAIL;
    }

    ret = rk_irfpa_streamhw_start(ctx);
    if (!ret) {
        goto FAIL;
    }

    while (1) {
        ret = rk_irfpa_streamhw_getFrame(ctx);
        if (!ret) {
            break;
        }
        printf("getFrame seq %d\n", ctx->res.seq);
        cnt++;
        if (cnt > 20)
            break;
    }

    rk_irfpa_streamhw_stop(ctx);
FAIL:
    rk_irfpa_streamhw_deinit(ctx);
    return 0;
}
