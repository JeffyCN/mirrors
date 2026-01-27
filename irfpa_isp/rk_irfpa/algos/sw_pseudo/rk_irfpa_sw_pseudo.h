#ifndef RK_IRFPA_SW_pseudo_H_
#define RK_IRFPA_SW_pseudo_H_

#include <stdint.h>
#include <stdbool.h>
#include "rk_irfpa_sw_pseudo_params.h"

typedef struct {
    rk_irfpa_sw_pseudo_params_t params;
    bool reconfig;
} rk_irfpa_sw_pseudo_ctx_t;

rk_irfpa_sw_pseudo_ctx_t *rk_irfpa_sw_pseudo_init(int width, int height);
void rk_irfpa_sw_pseudo_deinit(rk_irfpa_sw_pseudo_ctx_t *ctx);
bool rk_irfpa_sw_pseudo_process(rk_irfpa_sw_pseudo_ctx_t *ctx, uint8_t *datain, uint8_t *dataout);

#endif
