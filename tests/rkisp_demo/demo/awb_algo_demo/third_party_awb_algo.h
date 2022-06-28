#include "rk_aiq_types.h"
#include "rk_aiq_user_api2_custom_awb.h"


int32_t custom_awb_init(void* ctx);
int32_t custom_awb_run(void* ctx, const rk_aiq_customAwb_stats_t* pstAwbInfo,
                      rk_aiq_customeAwb_results_t* pstAwbResult);
int32_t custom_awb_ctrl(void* ctx, uint32_t u32Cmd, void *pValue);
int32_t custom_awb_exit(void* ctx);
