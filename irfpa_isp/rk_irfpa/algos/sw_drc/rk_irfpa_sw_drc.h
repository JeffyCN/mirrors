#ifndef RK_IRFPA_SW_DRC_H_
#define RK_IRFPA_SW_DRC_H_

#include <stdint.h>
#include <stdbool.h>
#include "rk_irfpa_sw_drc_params.h"

typedef struct sw_drc_info_s {
    int diff;
    float gain;
    int iso;
} sw_drc_info_t;

typedef struct {
    rk_irfpa_sw_drc_params_t params;
    sw_drc_info_t info;
} rk_irfpa_sw_drc_ctx_t;

rk_irfpa_sw_drc_ctx_t *rk_irfpa_sw_drc_init(int width, int height, uint8_t bits);
void rk_irfpa_sw_drc_deinit(rk_irfpa_sw_drc_ctx_t* ctx);
bool rk_irfpa_sw_drc_stat(rk_irfpa_sw_drc_ctx_t *ctx, int16_t *datain, int16_t* dataout);
bool rk_irfpa_sw_drc_process(rk_irfpa_sw_drc_ctx_t* ctx, int16_t* datain, int16_t* dataout);

#endif
