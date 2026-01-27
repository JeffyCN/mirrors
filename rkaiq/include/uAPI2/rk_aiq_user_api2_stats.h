
#ifndef _RK_AIQ_USER_API2_AIQ_STATS_H_
#define _RK_AIQ_USER_API2_AIQ_STATS_H_

#ifdef ISP_HW_V39
#include "isp/rk_aiq_isp_stats_v39.h"
#endif
#ifdef ISP_HW_V33
#include "isp/rk_aiq_isp_stats_v33.h"
#endif
#ifdef ISP_HW_V35
#include "isp/rk_aiq_isp_stats_v35.h"
#endif

RKAIQ_BEGIN_DECLARE

#ifndef RK_AIQ_SYS_CTX_T
#define RK_AIQ_SYS_CTX_T
typedef struct rk_aiq_sys_ctx_s rk_aiq_sys_ctx_t;
#endif

/*!
 * \brief new 3a stats interace for C version
 *
 * \param[in] ctx             context
 * \param[in] timeout_ms      -1: wait until next stats comes
 *                             0: return current stats immediately
 *                           > 0: wait next stats until timeout
 * \param[out] stats          stats
 * \return void
 */
XCamReturn
rk_aiq_uapi2_stats_getIspStats(const rk_aiq_sys_ctx_t* ctx,
                              rk_aiq_isp_statistics_t *stats, int timeout_ms);

RKAIQ_END_DECLARE

#endif
