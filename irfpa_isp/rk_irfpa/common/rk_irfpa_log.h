#ifndef RK_IRFPA_LOG_H_
#define RK_IRFPA_LOG_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RK_IRFPA_LOG_LEVEL_TRACE,
    RK_IRFPA_LOG_LEVEL_DEBUG,
    RK_IRFPA_LOG_LEVEL_INFO,
    RK_IRFPA_LOG_LEVEL_WARN,
    RK_IRFPA_LOG_LEVEL_ERROR,
    RK_IRFPA_LOG_LEVEL_FATAL
} rk_irfpa_log_level_e;

typedef struct {
    bool use_stdio;
    bool stdio_color;
    const char *file;
    rk_irfpa_log_level_e level;
} rk_irfpa_log_config_t;

typedef struct {
    rk_irfpa_log_config_t cfg;
} rk_irfpa_log_ctx_t;

rk_irfpa_log_ctx_t *rk_irfpa_log_create_ctx(void);
rk_irfpa_log_ctx_t *rk_irfpa_log_get_default(void);
bool rk_irfpa_log_init(rk_irfpa_log_ctx_t *ctx);
void rk_irfpa_log_deinit(rk_irfpa_log_ctx_t *ctx);
void rk_irfpa_log(rk_irfpa_log_ctx_t * ctx, int level, const char *file, int line, const char *fmt, ...);


#define rk_irfpa_log_trace(ctx, ...) rk_irfpa_log(ctx, RK_IRFPA_LOG_LEVEL_TRACE, __FILE_NAME__, __LINE__, __VA_ARGS__)
#define rk_irfpa_log_debug(ctx, ...) rk_irfpa_log(ctx, RK_IRFPA_LOG_LEVEL_DEBUG, __FILE_NAME__, __LINE__, __VA_ARGS__)
#define rk_irfpa_log_info(ctx, ...)  rk_irfpa_log(ctx, RK_IRFPA_LOG_LEVEL_INFO,  __FILE_NAME__, __LINE__, __VA_ARGS__)
#define rk_irfpa_log_warn(ctx, ...)  rk_irfpa_log(ctx, RK_IRFPA_LOG_LEVEL_WARN,  __FILE_NAME__, __LINE__, __VA_ARGS__)
#define rk_irfpa_log_error(ctx, ...) rk_irfpa_log(ctx, RK_IRFPA_LOG_LEVEL_ERROR, __FILE_NAME__, __LINE__, __VA_ARGS__)
#define rk_irfpa_log_fatal(ctx, ...) rk_irfpa_log(ctx, RK_IRFPA_LOG_LEVEL_FATAL, __FILE_NAME__, __LINE__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif

