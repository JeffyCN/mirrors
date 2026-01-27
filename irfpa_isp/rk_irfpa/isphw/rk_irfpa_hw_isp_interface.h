#pragma once

#define RK_IRFPA_PLATFORM_RV1126B 1

#if RK_IRFPA_PLATFORM_RV1126B
#include "rk_isp35_config.h"
typedef struct isp35_isp_params_cfg isphw_params_t;
typedef struct isp35_stat isphw_stats_t;

typedef struct isp35_bay3d_cfg isphw_tnr_params_t;
typedef struct isp35_ynr_cfg isphw_ynr_params_t;
typedef struct isp35_sharp_cfg isphw_sharp_params_t;
#else
#error "must select soc platform!"
#endif
