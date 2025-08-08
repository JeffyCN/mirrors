#ifndef __SAMPLE_AIE_MODULE_H__
#define __SAMPLE_AIE_MODULE_H__

#include "xcore/base/xcam_common.h"

#ifdef  __cplusplus
extern "C" {
#endif
void sample_print_aie_info(const void *arg);
XCamReturn sample_aie_module(const void* arg);
#ifdef USE_NEWSTRUCT
void sample_new_ie(const rk_aiq_sys_ctx_t* ctx);
#endif
#ifdef  __cplusplus
}
#endif
#endif  /*__SAMPLE_AIE_MODULE_H__*/
