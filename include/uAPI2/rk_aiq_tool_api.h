
#ifndef _RK_AIQ_TOOL_API_H_
#define _RK_AIQ_TOOL_API_H_

#include "rk_aiq_user_api2_imgproc.h"
#include "rk_aiq_api_private.h"

XCamReturn rk_aiq_tool_api_ae_setExpSwAttr
    (const rk_aiq_sys_ctx_t* sys_ctx, const Uapi_ExpSwAttrV2_t expSwAttr)
{
    const rk_aiq_sys_ctx_t* tool_ctx = get_next_ctx(sys_ctx);
    return rk_aiq_user_api2_ae_setExpSwAttr(tool_ctx, expSwAttr);
}

XCamReturn rk_aiq_tool_api_setMWBGain
    (const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_gain_t *gain)
{
    const rk_aiq_sys_ctx_t* tool_ctx = get_next_ctx(sys_ctx);
    return rk_aiq_uapi2_setMWBGain(tool_ctx, gain);
}

XCamReturn rk_aiq_tool_api_setWBMode
    (const rk_aiq_sys_ctx_t* sys_ctx, opMode_t mode)
{
    const rk_aiq_sys_ctx_t* tool_ctx = get_next_ctx(sys_ctx);
    return rk_aiq_uapi2_setWBMode(tool_ctx, mode);
}

int rk_aiq_tool_api_amerge_SetAttrib2
    (const rk_aiq_sys_ctx_t* sys_ctx, uapiMergeCurrCtlData_t* ctldata)
{
    const rk_aiq_sys_ctx_t* tool_ctx = get_next_ctx(sys_ctx);
    return rk_aiq_user_api2_amerge_SetAttrib2(tool_ctx, ctldata);
}

XCamReturn rk_aiq_tool_api_adrc_SetAttrib
    (const rk_aiq_sys_ctx_t* sys_ctx, drc_attrib_t attr)
{
    const rk_aiq_sys_ctx_t* tool_ctx = get_next_ctx(sys_ctx);
    return rk_aiq_user_api2_adrc_SetAttrib(tool_ctx, attr);
}

XCamReturn rk_aiq_tool_api_sysctl_swWorkingModeDyn
    (const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_working_mode_t mode)
{
    const rk_aiq_sys_ctx_t* tool_ctx = get_next_ctx(sys_ctx);
    return rk_aiq_uapi_sysctl_swWorkingModeDyn(tool_ctx, mode);
}

int rk_aiq_tool_api_set_scene
    (const rk_aiq_sys_ctx_t* sys_ctx, aiq_scene_t* scene)
{
    const rk_aiq_sys_ctx_t* tool_ctx = get_next_ctx(sys_ctx);
    return rk_aiq_user_api2_set_scene(tool_ctx, scene);
}

#endif
