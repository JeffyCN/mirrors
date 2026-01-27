#ifndef RK_IRFPA_SW_NUC_H_
#define RK_IRFPA_SW_NUC_H_

#include <stdint.h>
#include <stdbool.h>
#include "rk_irfpa_sw_nuc_params.h"

typedef struct {
    rk_irfpa_sw_nuc_params_t params;
    bool reconfig;

    uint16_t *base_buf;
    uint16_t *base_ave;
} rk_irfpa_sw_nuc_ctx_t;

rk_irfpa_sw_nuc_ctx_t *rk_irfpa_sw_nuc_init(int width, int height);
void rk_irfpa_sw_nuc_deinit(rk_irfpa_sw_nuc_ctx_t *ctx);
bool rk_irfpa_sw_nuc_process(rk_irfpa_sw_nuc_ctx_t *ctx, int16_t *datain, int16_t *dataout);

#endif
