#ifndef __SAMPLE_ADPCC_MODULE_H__
#define __SAMPLE_ADPCC_MODULE_H__

#include "xcore/base/xcam_common.h"

#ifdef  __cplusplus
extern "C" {
#endif
XCamReturn sample_adpcc_module(const void* arg);
#ifdef USE_NEWSTRUCT
void sample_dpc_test(const rk_aiq_sys_ctx_t* ctx);
#endif
#ifdef  __cplusplus
}
#endif
#endif  /*__SAMPLE_ADPCC_MODULE_H__*/
