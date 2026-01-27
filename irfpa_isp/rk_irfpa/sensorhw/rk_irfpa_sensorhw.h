#ifndef RK_IRFPA_SENSORHW_H_
#define RK_IRFPA_SENSORHW_H_

#include <stdint.h>
#include <stdbool.h>
#include "../streamhw/rk_irfpa_streamhw.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef float (*rk_irfpa_read_temp_func_t)(rk_irfpa_streamhw_ctx_t *);
typedef bool (*rk_irfpa_custom_process_func_t)(rk_irfpa_streamhw_ctx_t *);

typedef enum {
    RK_IRFPA_SENSORHW_OP_NONE,
    RK_IRFPA_SENSORHW_OP_SHUTTER_CLOSE,
    RK_IRFPA_SENSORHW_OP_SHUTTER_OPEN,
    RK_IRFPA_SENSORHW_OP_GAIN1,
    RK_IRFPA_SENSORHW_OP_GAIN2,
    RK_IRFPA_SENSORHW_OP_OOC,
    RK_IRFPA_SENSORHW_OP_BASE,
    RK_IRFPA_SENSORHW_OP_DONE,
} rk_irfpa_sensorhw_opertion_e;

typedef enum {
    RK_IRFPA_ADJUST_INCREASE_HIGHER,
    RK_IRFPA_ADJUST_INCREASE_LOWER
} rk_irfpa_adjust_mode_e;

typedef struct {
    uint8_t step;
    uint8_t delay;
} rk_irfpa_sensorhw_op_t;

typedef struct {
    rk_irfpa_adjust_mode_e mode;
    int max;
    int min;
    int step;
    int delay;
} rk_irfpa_sensorhw_adjust_t;

typedef struct {
    int lower;
    int upper;
} rk_irfpa_sensorhw_target_t;

typedef struct {
    const char *ctrl;
    int open_value;
    int close_value;
} rk_irfpa_sensorhw_shutter_t;

typedef struct {
    const char *ctrl;
    rk_irfpa_sensorhw_adjust_t adjust;
    rk_irfpa_sensorhw_target_t target;
} rk_irfpa_sensorhw_gain_t;

typedef struct {
    int start_x;
    int start_y;
    uint8_t init_val;
    uint8_t adj_step;
    rk_irfpa_sensorhw_adjust_t adjust;
    rk_irfpa_sensorhw_target_t target;
} rk_irfpa_sensorhw_ooc_t;

typedef struct {
    bool enable;
    int ave_num;
    int ave_delay;
} rk_irfpa_sensorhw_base_t;

typedef struct {
    bool enable;
    int read_interval;
    rk_irfpa_read_temp_func_t func;
} rk_irfpa_sensorhw_temp_t;

typedef struct {
    int delay;
    const rk_irfpa_sensorhw_op_t *steps;
} rk_irfpa_sensorhw_initial_t;

typedef struct {
    rk_irfpa_sensorhw_shutter_t shutter;
    rk_irfpa_sensorhw_gain_t gain1;
    rk_irfpa_sensorhw_gain_t gain2;
    rk_irfpa_sensorhw_ooc_t ooc;
    rk_irfpa_sensorhw_base_t base;

    rk_irfpa_sensorhw_temp_t vtemp;
    rk_irfpa_sensorhw_temp_t machine;

    rk_irfpa_sensorhw_initial_t initial;
} rk_irfpa_sensorhw_config_t;

typedef struct {
    rk_irfpa_sensorhw_config_t cfg;

    uint16_t *base_buf;
    uint16_t base_ave;
} rk_irfpa_sensorhw_ctx_t;

void rk_irfpa_sensorhw_register(const char *name, rk_irfpa_sensorhw_config_t cfg);

rk_irfpa_sensorhw_ctx_t* rk_irfpa_sensorhw_create_ctx(void);
bool rk_irfpa_sensorhw_init(rk_irfpa_sensorhw_ctx_t *ctx, rk_irfpa_streamhw_ctx_t *streamhw);
void rk_irfpa_sensorhw_deinit(rk_irfpa_sensorhw_ctx_t *ctx);
bool rk_irfpa_sensorhw_process(rk_irfpa_sensorhw_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif

