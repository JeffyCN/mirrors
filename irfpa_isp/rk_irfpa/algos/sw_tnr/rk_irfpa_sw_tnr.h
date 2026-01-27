#ifndef RK_IRFPA_SW_TNR_H_
#define RK_IRFPA_SW_TNR_H_

#include "rk_irfpa_sw_tnr_params.h"
#include "../sw_drc/rk_irfpa_sw_drc.h"

typedef struct {
    rk_irfpa_sw_tnr_params_t params;
    bool reconfig;
} rk_irfpa_sw_tnr_ctx_t;

rk_irfpa_sw_tnr_ctx_t *rk_irfpa_sw_tnr_init(int width, int height);
void rk_irfpa_sw_tnr_deinit(rk_irfpa_sw_tnr_ctx_t *ctx);
bool rk_irfpa_sw_tnr_process(rk_irfpa_sw_tnr_ctx_t *ctx, int16_t *datain, int16_t *dataout, sw_drc_info_t *drc_info);

#endif
