#ifndef RK_IRFPA_SIMPLE_API_H_
#define RK_IRFPA_SIMPLE_API_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RK_IRFPA_SIMPLE_API_LOG_QUIET,
    RK_IRFPA_SIMPLE_API_LOG_ERROR,
    RK_IRFPA_SIMPLE_API_LOG_INFO,
    RK_IRFPA_SIMPLE_API_LOG_DEBUG,
    RK_IRFPA_SIMPLE_API_LOG_TRACE
} rk_irfpa_simple_api_log_mode_e;

typedef enum {
    RK_IRFPA_SIMPLE_API_HW_ISP,
    RK_IRFPA_SIMPLE_API_SW_ISP
} rk_irfpa_simple_api_isp_mode_e;

typedef struct {
    const char *isp_media;
    const char *ooc_media;
    const char *vicap_media;
    const char *iqfile;

    rk_irfpa_simple_api_log_mode_e log_mode;
    rk_irfpa_simple_api_isp_mode_e isp_mode;
} rk_irfpa_simple_api_config_t;

typedef struct {
    bool ok;
    rk_irfpa_simple_api_config_t cfg;
} rk_irfpa_simple_api_ctx_t;

rk_irfpa_simple_api_ctx_t* rk_irfpa_simple_api_create_ctx(void);
bool rk_irfpa_simple_api_init(rk_irfpa_simple_api_ctx_t *ctx);
void rk_irfpa_simple_api_deinit(rk_irfpa_simple_api_ctx_t *ctx);
bool rk_irfpa_simple_api_start(rk_irfpa_simple_api_ctx_t *ctx);
bool rk_irfpa_simple_api_stop(rk_irfpa_simple_api_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif

