#ifndef RK_IRFPA_SW_SPNR_H_
#define RK_IRFPA_SW_SPNR_H_

#include <stdint.h>
#include <stdbool.h>
#include "rk_irfpa_sw_spnr_params.h"
#include "../sw_drc/rk_irfpa_sw_drc.h"

typedef struct {
    rk_irfpa_sw_spnr_params_t params;
    bool reconfig;
} rk_irfpa_sw_spnr_ctx_t;

rk_irfpa_sw_spnr_ctx_t *rk_irfpa_sw_spnr_init(int width, int height);
void rk_irfpa_sw_spnr_deinit(rk_irfpa_sw_spnr_ctx_t *ctx);
bool rk_irfpa_sw_spnr_process(rk_irfpa_sw_spnr_ctx_t *ctx, int16_t *datain, int16_t *dataout, sw_drc_info_t *drc_info);

#endif
