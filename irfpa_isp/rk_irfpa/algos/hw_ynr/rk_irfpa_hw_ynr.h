#ifndef RK_IRFPA_HW_YNR_H_
#define RK_IRFPA_HW_YNR_H_

#include <stdint.h>
#include "rk_irfpa_hw_ynr_params.h"
#include "../sw_drc/rk_irfpa_sw_drc.h"
#include "../../isphw/rk_irfpa_isphw.h"

typedef struct {
    rk_irfpa_hw_ynr_params_t params;
    bool reconfig;
    bool delaybypass;
} rk_irfpa_hw_ynr_ctx_t;

void rk_irfpa_hw_ynr_params_t_init(rk_irfpa_hw_ynr_params_t *p);
rk_irfpa_hw_ynr_ctx_t *rk_irfpa_hw_ynr_init(int width, int height);
void rk_irfpa_hw_ynr_deinit(rk_irfpa_hw_ynr_ctx_t *ctx);
bool rk_irfpa_hw_ynr_process(rk_irfpa_hw_ynr_ctx_t *ctx, rk_irfpa_isphw_ctx_t *isphw, sw_drc_info_t *drc_info);

#endif
