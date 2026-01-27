#ifndef RK_IRFPA_ISPHW_H_
#define RK_IRFPA_ISPHW_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "rk_irfpa_hw_isp_interface.h"

typedef struct {
    uint16_t width;
    uint16_t height;
    const char *isp_media;
} rk_irfpa_isphw_config_t;

typedef struct {
    uint32_t seq;
    uint16_t *iir;
    uint8_t *y8;
} rk_irfpa_isphw_result_t;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint32_t seq;
} rk_irfpa_isphw_info_t;

typedef struct {
    char error_str[128];
    rk_irfpa_isphw_config_t cfg;
    rk_irfpa_isphw_result_t res;
    rk_irfpa_isphw_info_t info;
    isphw_params_t params;
    isphw_stats_t stats;
} rk_irfpa_isphw_ctx_t;

rk_irfpa_isphw_ctx_t* rk_irfpa_isphw_create_ctx(void);
bool rk_irfpa_isphw_init(rk_irfpa_isphw_ctx_t* ctx);
void rk_irfpa_isphw_postProcess(rk_irfpa_isphw_ctx_t* ctx);
bool rk_irfpa_isphw_process(rk_irfpa_isphw_ctx_t* ctx, uint16_t* data, uint32_t seq);
bool rk_irfpa_isphw_save_stats(rk_irfpa_isphw_ctx_t* ctx);
void rk_irfpa_isphw_preProcess(rk_irfpa_isphw_ctx_t* ctx, uint32_t seq);
void rk_irfpa_isphw_deinit(rk_irfpa_isphw_ctx_t* ctx);

#ifdef __cplusplus
}
#endif
#endif

