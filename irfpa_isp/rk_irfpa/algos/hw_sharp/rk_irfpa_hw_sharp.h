#ifndef RK_IRFPA_HW_SHARP_H_
#define RK_IRFPA_HW_SHARP_H_

#include "../../isphw/rk_irfpa_isphw.h"
#include "rk_irfpa_hw_sharp_params.h"
#include "rk_irfpa_hw_texEst_params.h"
#include "../hw_ynr/rk_irfpa_hw_ynr_params.h"
#include "../sw_drc/rk_irfpa_sw_drc.h"

typedef struct {
    rk_irfpa_hw_sharp_params_t sharp_params;
    rk_irfpa_hw_texEst_params_t texEst_params;
    bool reconfig;
    bool delaybypass;
} rk_irfpa_hw_sharp_ctx_t;

void rk_irfpa_hw_sharp_params_t_init(rk_irfpa_hw_sharp_params_t* p);
void rk_irfpa_hw_texEst_params_t_init(rk_irfpa_hw_texEst_params_t* p);
rk_irfpa_hw_sharp_ctx_t* rk_irfpa_hw_sharp_init(int width, int height);
void rk_irfpa_hw_sharp_deinit(rk_irfpa_hw_sharp_ctx_t *ctx);
bool rk_irfpa_hw_sharp_process(rk_irfpa_hw_sharp_ctx_t *ctx, rk_irfpa_hw_ynr_params_t* ynr, rk_irfpa_isphw_ctx_t *isphw, sw_drc_info_t *drc_info);

#endif
