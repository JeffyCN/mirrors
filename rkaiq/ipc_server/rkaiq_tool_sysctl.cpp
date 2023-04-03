#include "socket_server.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rkaiq_tool_sysctl.cpp"

int setCpsLtCfg(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_uapi_sysctl_setCpsLtCfg(ctx, (rk_aiq_cpsl_cfg_t*) data);
}

int getCpsLtInfo(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_uapi_sysctl_getCpsLtInfo(ctx, (rk_aiq_cpsl_info_t *) data);
}

int queryCpsLtCap(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_uapi_sysctl_queryCpsLtCap(ctx, (rk_aiq_cpsl_cap_t *) data);
}

int setWorkingModeDyn(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_uapi_sysctl_swWorkingModeDyn(ctx, *(rk_aiq_working_mode_t*) data);
}

int getVersionInfo(rk_aiq_sys_ctx_t* ctx, char* data) {
    rk_aiq_uapi_get_version_info((rk_aiq_ver_info_t *)data);
    return 0;
}

#if RKAIQ_HAVE_MERGE_V10
int setMergeAttribV10(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_amerge_v10_SetAttrib(ctx, (mergeAttrV10_t*)data);
}

int getMergeAttribV10(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_amerge_v10_GetAttrib(ctx, (mergeAttrV10_t*)data);
}
#endif
#if RKAIQ_HAVE_MERGE_V11
int setMergeAttribV11(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_amerge_v11_SetAttrib(ctx, (mergeAttrV11_t*)data);
}

int getMergeAttribV11(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_amerge_v11_GetAttrib(ctx, (mergeAttrV11_t*)data);
}
#endif
#if RKAIQ_HAVE_MERGE_V12
int setMergeAttribV12(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_amerge_v12_SetAttrib(ctx, (mergeAttrV12_t*)data);
}

int getMergeAttribV12(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_amerge_v12_GetAttrib(ctx, (mergeAttrV12_t*)data);
}
#endif

int setTmoAttrib(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_atmo_SetAttrib(ctx, *(atmo_attrib_t*)data);
}

int getTmoAttrib(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_atmo_GetAttrib(ctx, (atmo_attrib_t*)data);
}

int setGammaAttrib(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api_agamma_SetAttrib(ctx, *(rk_aiq_gamma_attrib_t*)data);
}

int getGammaAttrib(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api_agamma_GetAttrib(ctx, (rk_aiq_gamma_attrib_t*)data);
}

int setDpccAttrib(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_adpcc_SetAttrib(ctx, (rk_aiq_dpcc_attrib_V20_t *)data);
}

int getDpccAttrib(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_adpcc_GetAttrib(ctx, (rk_aiq_dpcc_attrib_V20_t *)data);
}
/*
int setDehazeAttrib(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api_adehaze_setSwAttrib(ctx, *(adehaze_sw_t *)data);
}

int getDehazeAttrib(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api_adehaze_getSwAttrib(ctx, (adehaze_sw_t *)data);
}
*/
int setCcmAttrib(rk_aiq_sys_ctx_t* ctx, char* data)
{
#if RKAIQ_HAVE_CCM_V1
    return rk_aiq_user_api_accm_SetAttrib(ctx, (rk_aiq_ccm_attrib_t*) data);
#elif RKAIQ_HAVE_CCM_V2
    return rk_aiq_user_api_accm_v2_SetAttrib(ctx, (rk_aiq_ccm_v2_attrib_t*) data);
#else
    return -1;
#endif
}

int getCcmAttrib(rk_aiq_sys_ctx_t* ctx, char* data)
{
#if RKAIQ_HAVE_CCM_V1
    return rk_aiq_user_api_accm_GetAttrib(ctx, (rk_aiq_ccm_attrib_t *)data);
#elif RKAIQ_HAVE_CCM_V2
    return rk_aiq_user_api_accm_v2_GetAttrib(ctx, (rk_aiq_ccm_v2_attrib_t *)data);
#else
    return -1;
#endif
}

int queryCCMInfo(rk_aiq_sys_ctx_t* ctx, char* data)
{
    return rk_aiq_user_api_accm_QueryCcmInfo(ctx, (rk_aiq_ccm_querry_info_t *)data);
}
int setAwbAttrib(rk_aiq_sys_ctx_t* ctx, char* data)
{
    return rk_aiq_user_api_awb_SetAttrib(ctx, *(rk_aiq_wb_attrib_t*)data);
}

int getAwbAttrib(rk_aiq_sys_ctx_t* ctx, char* data)
{
    return rk_aiq_user_api_awb_GetAttrib(ctx, (rk_aiq_wb_attrib_t*)data);
}

int getAwbCCT(rk_aiq_sys_ctx_t* ctx, char* data)
{
    return rk_aiq_user_api_awb_GetCCT(ctx, (rk_aiq_wb_cct_t *)data);
}

int queryWBInfo(rk_aiq_sys_ctx_t* ctx, char* data)
{
    return rk_aiq_user_api_awb_QueryWBInfo(ctx, (rk_aiq_wb_querry_info_t*) data);
}

int setAcpAttrib(rk_aiq_sys_ctx_t* ctx, char* data)
{
    return  rk_aiq_user_api_acp_SetAttrib(ctx, (acp_attrib_t*) data);
}

int getAcpAttrib(rk_aiq_sys_ctx_t* ctx, char* data)
{
    return rk_aiq_user_api_acp_GetAttrib(ctx, (acp_attrib_t *) data);
}

int enqueueRkRawFile(rk_aiq_sys_ctx_t* ctx, char* data)
{
    return rk_aiq_uapi_sysctl_enqueueRkRawFile(ctx, data);
}

int get3AStats(rk_aiq_sys_ctx_t* ctx, char* data)
{
    return rk_aiq_uapi_sysctl_get3AStats(ctx, (rk_aiq_isp_stats_t*)data);
}

int get3AStatsBlk(rk_aiq_sys_ctx_t* ctx, char* data)
{
    rk_aiq_isp_stats_t* new_stats = NULL;
    rk_aiq_uapi_sysctl_get3AStatsBlk(ctx, &new_stats, -1);
    if (new_stats) {
      memcpy(data, new_stats, sizeof(rk_aiq_isp_stats_t));
      rk_aiq_uapi_sysctl_release3AStatsRef(ctx, new_stats);
      return 0;
    } else {
      return -1;
    }
}

int writeAwbIn(rk_aiq_sys_ctx_t* ctx, char* data)
{
    static int call_cnt = 0;
    rk_aiq_uapiV2_awb_wrtIn_attr_t attr;
    memset(&attr,0,sizeof(rk_aiq_uapiV2_awb_wrtIn_attr_t));
    attr.en = true;
    attr.mode = 1; // 1 means rgb ,0 means raw
    attr.call_cnt = call_cnt++;
    sprintf(attr.path,"/tmp");
    return rk_aiq_user_api2_awb_WriteAwbIn(ctx, attr);
}
