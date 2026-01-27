#include <stdio.h>
#include <unistd.h>
#include "rk_irfpa.h"

int main(void) {
    bool ret = false;
    int cnt = 0;
    int val = 0;

    rk_irfpa_simple_api_ctx_t* ctx = rk_irfpa_simple_api_create_ctx();
    ctx->cfg.ooc_media = "/dev/media0";
    ctx->cfg.vicap_media = "/dev/media1";
    ctx->cfg.isp_media = "/dev/media3";
    ctx->cfg.iqfile = "/data/irfpaisp12.json";
    ctx->cfg.log_mode = RK_IRFPA_SIMPLE_API_LOG_DEBUG;
    ctx->cfg.isp_mode = RK_IRFPA_SIMPLE_API_HW_ISP;

    ret = rk_irfpa_simple_api_init(ctx);
    if (!ret) {
        return -1;
    }

    rk_irfpa_simple_api_start(ctx);
    while (1) {
        char c = getchar();
        if (c == 'q') {
            break;
        }
    }

    rk_irfpa_simple_api_stop(ctx);
    rk_irfpa_simple_api_deinit(ctx);
    return 0;
}
