/*
 *  Copyright (c) 2024 Rockchip Corporation
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
 *
 */

#include "uAPI2/rk_aiq_user_api2_sysctl.h"

XCamReturn
rk_aiq_uapi_sysctl_preInit(const char* sns_ent_name,
                           rk_aiq_working_mode_t mode,
                           const char* force_iq_file)
{
    return rk_aiq_uapi2_sysctl_preInit(sns_ent_name, mode, force_iq_file);
}

XCamReturn
rk_aiq_uapi_sysctl_setReadBackMode(rk_aiq_sys_ctx_t* ctx, bool on)
{
    return rk_aiq_uapi2_sysctl_setReadBackMode(ctx, on);
}

rk_aiq_sys_ctx_t*
rk_aiq_uapi_sysctl_init(const char* sns_ent_name,
                        const char* config_file_dir,
                        rk_aiq_error_cb err_cb,
                        rk_aiq_metas_cb metas_cb)
{
    return rk_aiq_uapi2_sysctl_init(sns_ent_name, config_file_dir, err_cb, metas_cb);
}

void
rk_aiq_uapi_sysctl_deinit(rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_uapi2_sysctl_deinit(ctx);
}

void rk_aiq_uapi_setRawBufNum(rk_aiq_sys_ctx_t* ctx, uint16_t buf_num)
{
    rk_aiq_uapi2_setRawBufNum(ctx, buf_num);
}

XCamReturn
rk_aiq_uapi_sysctl_prepare(const rk_aiq_sys_ctx_t* ctx,
                           uint32_t  width, uint32_t  height,
                           rk_aiq_working_mode_t mode)
{
    return rk_aiq_uapi2_sysctl_prepare(ctx, width, height, mode);
}

XCamReturn
rk_aiq_uapi_sysctl_start(const rk_aiq_sys_ctx_t* ctx)
{
    return rk_aiq_uapi2_sysctl_start(ctx);
}

XCamReturn
rk_aiq_uapi_sysctl_stop(const rk_aiq_sys_ctx_t* ctx, bool keep_ext_hw_st)
{
    return rk_aiq_uapi2_sysctl_stop(ctx, keep_ext_hw_st);
}

XCamReturn
rk_aiq_uapi_sysctl_getStaticMetas(const char* sns_ent_name, rk_aiq_static_info_t* static_info)
{
    return rk_aiq_uapi2_sysctl_getStaticMetas(sns_ent_name, static_info);
}

XCamReturn rk_aiq_uapi_sysctl_enumStaticMetasByPhyId(
    int index, rk_aiq_static_info_t* static_info) {
    return rk_aiq_uapi2_sysctl_enumStaticMetasByPhyId(index, static_info);
}

XCamReturn
rk_aiq_uapi_sysctl_enumStaticMetas(int index, rk_aiq_static_info_t* static_info)
{
    return rk_aiq_uapi2_sysctl_enumStaticMetasByPhyId(index, static_info);
}

const char*
rk_aiq_uapi_sysctl_getBindedSnsEntNmByVd(const char* vd)
{
    return rk_aiq_uapi2_sysctl_getBindedSnsEntNmByVd(vd);
}

XCamReturn
rk_aiq_uapi_sysctl_getMetaData(const rk_aiq_sys_ctx_t* ctx, uint32_t frame_id, rk_aiq_metas_t* metas)
{
    // TODO
    return XCAM_RETURN_ERROR_FAILED;
}

XCamReturn
rk_aiq_uapi_sysctl_regLib(const rk_aiq_sys_ctx_t* ctx,
                          RkAiqAlgoDesComm* algo_lib_des)
{
    return XCAM_RETURN_ERROR_FAILED;
}

XCamReturn
rk_aiq_uapi_sysctl_unRegLib(const rk_aiq_sys_ctx_t* ctx,
                            const int algo_type,
                            const int lib_id)
{
    return XCAM_RETURN_ERROR_FAILED;
}

XCamReturn
rk_aiq_uapi_sysctl_enableAxlib(const rk_aiq_sys_ctx_t* ctx,
                               const int algo_type,
                               const int lib_id,
                               bool enable)
{
    return rk_aiq_uapi2_sysctl_enableAxlib(ctx, algo_type, lib_id, enable);
}

bool
rk_aiq_uapi_sysctl_getAxlibStatus(const rk_aiq_sys_ctx_t* ctx,
                                  const int algo_type,
                                  const int lib_id)
{
    return rk_aiq_uapi2_sysctl_getAxlibStatus(ctx, algo_type, lib_id);
}

RkAiqAlgoContext*
rk_aiq_uapi_sysctl_getEnabledAxlibCtx(const rk_aiq_sys_ctx_t* ctx, const int algo_type)
{
    return (RkAiqAlgoContext*)rk_aiq_uapi2_sysctl_getEnabledAxlibCtx(ctx, algo_type);
}

RkAiqAlgoContext*
rk_aiq_uapi_sysctl_getAxlibCtx(const rk_aiq_sys_ctx_t* ctx, const int algo_type, const int lib_id)
{
    return rk_aiq_uapi2_sysctl_getAxlibCtx(ctx, algo_type, lib_id);
}

XCamReturn
rk_aiq_uapi_sysctl_get3AStats(const rk_aiq_sys_ctx_t* ctx,
                              rk_aiq_isp_stats_t *stats)
{
    return rk_aiq_uapi2_sysctl_get3AStats(ctx, stats);
}

XCamReturn
rk_aiq_uapi_sysctl_get3AStatsBlk(const rk_aiq_sys_ctx_t* ctx,
                                 rk_aiq_isp_stats_t **stats, int timeout_ms)
{
    return rk_aiq_uapi2_sysctl_get3AStatsBlk(ctx, stats, timeout_ms);
}

void
rk_aiq_uapi_sysctl_release3AStatsRef(const rk_aiq_sys_ctx_t* ctx,
                                     rk_aiq_isp_stats_t *stats)
{
    return rk_aiq_uapi2_sysctl_release3AStatsRef(ctx, stats);
}

XCamReturn rk_aiq_uapi_sysctl_pause(rk_aiq_sys_ctx_t* sys_ctx, bool is_single_frame_mode)
{
    return rk_aiq_uapi2_sysctl_pause(sys_ctx, is_single_frame_mode);
}

XCamReturn rk_aiq_uapi_sysctl_resume(rk_aiq_sys_ctx_t* sys_ctx)
{
    return rk_aiq_uapi2_sysctl_resume(sys_ctx);
}

#include "rk_aiq_user_api_imgproc.c"
