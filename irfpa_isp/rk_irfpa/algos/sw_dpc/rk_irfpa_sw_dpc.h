#ifndef RK_IRFPA_SW_DPC_H_
#define RK_IRFPA_SW_DPC_H_

#include "rk_irfpa_sw_dpc_params.h"
typedef struct {
    rk_irfpa_sw_dpc_params_t params;
    bool reconfig;
} rk_irfpa_sw_dpc_ctx_t;

rk_irfpa_sw_dpc_ctx_t *rk_irfpa_sw_dpc_init(int width, int height);
void rk_irfpa_sw_dpc_deinit(rk_irfpa_sw_dpc_ctx_t *ctx);
bool rk_irfpa_sw_dpc_process(rk_irfpa_sw_dpc_ctx_t *ctx, int16_t *datain, int16_t *dataout);

#endif
