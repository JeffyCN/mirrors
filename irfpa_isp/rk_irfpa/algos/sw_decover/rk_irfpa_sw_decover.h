#ifndef RK_IRFPA_SW_DECOVER_H_
#define RK_IRFPA_SW_DECOVER_H_

#include <stdint.h>
#include <stdbool.h>
#include "rk_irfpa_sw_decover_params.h"

typedef struct {
    rk_irfpa_sw_decover_params_t params;
    bool reconfig;
} rk_irfpa_sw_decover_ctx_t;

rk_irfpa_sw_decover_ctx_t *rk_irfpa_sw_decover_init(int width, int height);
void rk_irfpa_sw_decover_deinit(rk_irfpa_sw_decover_ctx_t* ctx);
bool rk_irfpa_sw_decover_build(rk_irfpa_sw_decover_ctx_t *ctx, int16_t *datain);
bool rk_irfpa_sw_decover_process(rk_irfpa_sw_decover_ctx_t* ctx, int16_t* datain, int16_t* dataout);

#endif
