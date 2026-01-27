#ifndef RK_IRFPA_PIPELINE_H_
#define RK_IRFPA_PIPELINE_H_

#include <stdint.h>
#include <stdbool.h>

#include "../common/rk_irfpa_log.h"
#include "../algos/sw_nuc/rk_irfpa_sw_nuc.h"
#include "../algos/sw_destripe/rk_irfpa_sw_destripe.h"
#include "../algos/sw_decover/rk_irfpa_sw_decover.h"
#include "../algos/sw_tnr/rk_irfpa_sw_tnr.h"
#include "../algos/sw_spnr/rk_irfpa_sw_spnr.h"
#include "../algos/sw_sharp/rk_irfpa_sw_sharp.h"
#include "../algos/sw_drc/rk_irfpa_sw_drc.h"
#include "../algos/sw_pseudo/rk_irfpa_sw_pseudo.h"
#include "../isphw/rk_irfpa_isphw.h"
#include "../algos/hw_tnr/rk_irfpa_hw_tnr.h"
#include "../algos/hw_ynr/rk_irfpa_hw_ynr.h"
#include "../algos/hw_sharp/rk_irfpa_hw_sharp.h"
#include "../algos/hw_enh/rk_irfpa_hw_enh.h"
#include "../algos/hw_histeq/rk_irfpa_hw_histeq.h"
#include "../algos/sw_dpc/rk_irfpa_sw_dpc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RK_IRFPA_PIPELINE_MODE_SW_ALL,
    RK_IRFPA_PIPELINE_MODE_HW_TNR,
    RK_IRFPA_PIPELINE_MODE_HW_ALL
} rk_irfpa_pipeline_mode_e;

typedef struct {
    rk_irfpa_pipeline_mode_e mode;
    uint8_t bits;
    uint16_t width;
    uint16_t height;
    const char *isp_media;
} rk_irfpa_pipeline_config_t;

typedef struct {
    rk_irfpa_pipeline_config_t cfg;
    rk_irfpa_log_ctx_t *log;

    rk_irfpa_isphw_ctx_t *isphw;
    rk_irfpa_sw_nuc_ctx_t* sw_nuc;

    rk_irfpa_sw_dpc_ctx_t *sw_dpc;
    rk_irfpa_sw_destripe_ctx_t* sw_destripe;
    rk_irfpa_sw_decover_ctx_t *sw_decover;
    rk_irfpa_sw_tnr_ctx_t* sw_tnr;
    rk_irfpa_sw_spnr_ctx_t *sw_spnr;
    rk_irfpa_sw_sharp_ctx_t *sw_sharp;
    rk_irfpa_sw_drc_ctx_t *sw_drc;
    rk_irfpa_sw_pseudo_ctx_t *sw_pseudo;

    rk_irfpa_hw_tnr_ctx_t *hw_tnr;
    rk_irfpa_hw_sharp_ctx_t *hw_sharp;
    rk_irfpa_hw_ynr_ctx_t* hw_ynr;
    rk_irfpa_hw_enh_ctx_t* hw_enh;
    rk_irfpa_hw_histeq_ctx_t* hw_histeq;

    uint8_t  *res_y8;
    uint8_t  *res_yuv;
} rk_irfpa_pipeline_ctx_t;


rk_irfpa_pipeline_ctx_t *rk_irfpa_pipeline_create_ctx(void);
bool rk_irfpa_pipeline_init(rk_irfpa_pipeline_ctx_t *ctx);
void rk_irfpa_pipeline_deinit(rk_irfpa_pipeline_ctx_t *ctx);
bool rk_irfpa_pipeline_process(rk_irfpa_pipeline_ctx_t *ctx, uint16_t *datain, uint32_t seq);
bool rk_irfpa_pipeline_read_json(rk_irfpa_pipeline_ctx_t *ctx, const char *iqfile);

bool rk_irfpa_pipeline_dump_json(rk_irfpa_pipeline_ctx_t *ctx, const char *fn);
bool rk_irfpa_pipeline_from_json(rk_irfpa_pipeline_ctx_t *ctx, const char *iqfile);

#ifdef __cplusplus
}
#endif

#endif
