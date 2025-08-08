#include "socket_server.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rkaiq_tool_ae.cpp"

#ifndef USE_NEWSTRUCT
int setExpSwAttr(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_ae_setExpSwAttr(ctx, *(Uapi_ExpSwAttrV2_t*)data);
}

int getExpSwAttr(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_ae_getExpSwAttr(ctx, (Uapi_ExpSwAttrV2_t*)data);
}
int setLinAeRouteAttr(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_ae_setLinAeRouteAttr(ctx, *(Uapi_LinAeRouteAttr_t*)data);
}

int getLinAeRouteAttr(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_ae_getLinAeRouteAttr(ctx, (Uapi_LinAeRouteAttr_t*)data);
}

int setHdrAeRouteAttr(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_ae_setHdrAeRouteAttr(ctx, *(Uapi_HdrAeRouteAttr_t*)data);
}

int getHdrAeRouteAttr(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_ae_getHdrAeRouteAttr(ctx, (Uapi_HdrAeRouteAttr_t*)data);
}
int queryExpResInfo(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_ae_queryExpResInfo(ctx, (Uapi_ExpQueryInfo_t*)data);
}

int setLinExpAttr(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_ae_setLinExpAttr(ctx, *(Uapi_LinExpAttrV2_t*)data);
}

int getLinExpAttr(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_ae_getLinExpAttr(ctx, (Uapi_LinExpAttrV2_t*)data);
}

int setHdrExpAttr(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_ae_setHdrExpAttr(ctx, *(Uapi_HdrExpAttrV2_t*)data);
}

int getHdrExpAttr(rk_aiq_sys_ctx_t* ctx, char* data) {
    return rk_aiq_user_api2_ae_getHdrExpAttr(ctx, (Uapi_HdrExpAttrV2_t*)data);
}

#else
int setExpSwAttr(rk_aiq_sys_ctx_t* ctx, char* data)
{
    return rk_aiq_user_api2_ae_setExpSwAttr(ctx, *(ae_api_expSwAttr_t*)data);
}

int getExpSwAttr(rk_aiq_sys_ctx_t* ctx, char* data)
{
    return rk_aiq_user_api2_ae_getExpSwAttr(ctx, (ae_api_expSwAttr_t*)data);
}

int queryExpResInfo(rk_aiq_sys_ctx_t* ctx, char* data)
{
    return rk_aiq_user_api2_ae_queryExpResInfo(ctx, (ae_api_queryInfo_t*)data);
}

int setLinExpAttr(rk_aiq_sys_ctx_t* ctx, char* data)
{
    return rk_aiq_user_api2_ae_setLinExpAttr(ctx, *(ae_api_linExpAttr_t*)data);
}

int getLinExpAttr(rk_aiq_sys_ctx_t* ctx, char* data)
{
    return rk_aiq_user_api2_ae_getLinExpAttr(ctx, (ae_api_linExpAttr_t*)data);
}

int setHdrExpAttr(rk_aiq_sys_ctx_t* ctx, char* data)
{
    return rk_aiq_user_api2_ae_setHdrExpAttr(ctx, *(ae_api_hdrExpAttr_t*)data);
}

int getHdrExpAttr(rk_aiq_sys_ctx_t* ctx, char* data)
{
    return rk_aiq_user_api2_ae_getHdrExpAttr(ctx, (ae_api_hdrExpAttr_t*)data);
}
#endif
