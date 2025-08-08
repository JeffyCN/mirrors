#ifndef __SAMPLE_ACP_MODULE_H__
#define __SAMPLE_ACP_MODULE_H__

#include "xcore/base/xcam_common.h"

#ifdef  __cplusplus
extern "C" {
#endif
void sample_print_acp_info(const void *arg);
XCamReturn sample_acp_module(const void* arg);
#ifdef USE_NEWSTRUCT
void sample_cp_test(const rk_aiq_sys_ctx_t* ctx);
#endif
#ifdef  __cplusplus
}
#endif
#endif  /*__SAMPLE_ACP_MODULE_H__*/
