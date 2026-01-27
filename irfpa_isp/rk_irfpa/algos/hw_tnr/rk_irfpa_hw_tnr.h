#ifndef RK_IRFPA_HW_TNR_H_
#define RK_IRFPA_HW_TNR_H_

#include <stdint.h>
#include "rk_irfpa_hw_tnr_params.h"
#include "../sw_drc/rk_irfpa_sw_drc.h"
#include "../../isphw/rk_irfpa_isphw.h"

typedef struct {
    rk_irfpa_hw_tnr_params_t params;
    bool reconfig;
} rk_irfpa_hw_tnr_ctx_t;

void rk_irfpa_hw_tnr_params_t_init(rk_irfpa_hw_tnr_params_t *p);
rk_irfpa_hw_tnr_ctx_t *rk_irfpa_hw_tnr_init(int width, int height);
void rk_irfpa_hw_tnr_deinit(rk_irfpa_hw_tnr_ctx_t* ctx);
bool rk_irfpa_hw_tnr_process(rk_irfpa_hw_tnr_ctx_t* ctx, rk_irfpa_isphw_ctx_t* isphw, uint16_t* data, sw_drc_info_t *drc_info);
void rk_irfpa_hw_tnr_postprocess(rk_irfpa_hw_tnr_ctx_t* ctx, rk_irfpa_isphw_ctx_t* isphw, uint16_t* data);
bool rk_irfpa_hw_tnr_process2(rk_irfpa_hw_tnr_ctx_t* ctx, rk_irfpa_isphw_ctx_t* isphw, uint16_t* data, uint8_t* mdwgt);

#endif
