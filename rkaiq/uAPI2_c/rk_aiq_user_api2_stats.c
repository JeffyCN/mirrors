/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "uAPI2/rk_aiq_user_api2_stats.h"

RKAIQ_BEGIN_DECLARE

/*
 * timeout: -1 next, 0 current, > 0 wait next until timeout
 */
XCamReturn
rk_aiq_uapi2_stats_getIspStats(const rk_aiq_sys_ctx_t* ctx,
                              rk_aiq_isp_statistics_t* stats, int timeout_ms)
{
	RKAIQ_API_SMART_LOCK(ctx);
	return AiqCore_get3AStats(ctx->_analyzer, stats, timeout_ms);
}

RKAIQ_END_DECLARE
