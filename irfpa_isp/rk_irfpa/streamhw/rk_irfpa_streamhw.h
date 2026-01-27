#ifndef RK_IRFPA_STREAMHW_H_
#define RK_IRFPA_STREAMHW_H_

#include <stdint.h>
#include <stdbool.h>
#include "../common/rk_irfpa_log.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rk_irfpa_streamhw_ctx_s rk_irfpa_streamhw_ctx_t;

typedef struct {
    const char *ooc_media;
    const char *vicap_media;
} rk_irfpa_streamhw_config_t;

typedef struct {
    uint32_t seq;
    uint16_t *y16;
} rk_irfpa_streamhw_result_t;

typedef struct {
    uint16_t top;
    uint16_t left;
    uint16_t width;
    uint16_t height;
    uint32_t pixelformat;
} rk_irfpa_output_node_info_t;

typedef struct {
    uint8_t vi_bits;
    uint16_t vi_width;
    uint16_t vi_height;
    uint8_t ooc_bits;
    uint16_t ooc_width;
    uint16_t ooc_height;
    rk_irfpa_output_node_info_t output; 
    char sensor_name[32];
} rk_irfpa_streamhw_info_t;


typedef struct rk_irfpa_streamhw_ctx_s {
    rk_irfpa_log_ctx_t *log;
    uint8_t *oocbuf;
    rk_irfpa_streamhw_config_t cfg;
    rk_irfpa_streamhw_result_t res;
    rk_irfpa_streamhw_info_t info;
} rk_irfpa_streamhw_ctx_t;

rk_irfpa_streamhw_ctx_t* rk_irfpa_streamhw_create_ctx(void);
bool rk_irfpa_streamhw_init(rk_irfpa_streamhw_ctx_t *ctx);
void rk_irfpa_streamhw_deinit(rk_irfpa_streamhw_ctx_t *ctx);
bool rk_irfpa_streamhw_start(rk_irfpa_streamhw_ctx_t *ctx);
bool rk_irfpa_streamhw_stop(rk_irfpa_streamhw_ctx_t *ctx);
bool rk_irfpa_streamhw_getFrame(rk_irfpa_streamhw_ctx_t *ctx);
bool rk_irfpa_streamhw_setControl(rk_irfpa_streamhw_ctx_t *ctx, const char *ctrl, int val);
bool rk_irfpa_streamhw_getControl(rk_irfpa_streamhw_ctx_t *ctx, const char *ctrl, int *val);
bool rk_irfpa_streamhw_flush_ooc(rk_irfpa_streamhw_ctx_t *ctx);
bool rk_irfpa_streamhw_irfparx(rk_irfpa_streamhw_ctx_t *ctx, uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif
