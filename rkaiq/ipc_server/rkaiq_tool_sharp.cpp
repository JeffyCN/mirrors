#include "socket_server.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rkaiq_tool_sharp.cpp"

int setSharpAttrib(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_asharp_SetAttrib(ctx, (rk_aiq_sharp_attrib_t*)data);
}

int getSharpAttrib(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_asharp_GetAttrib(ctx, (rk_aiq_sharp_attrib_t*)data);
}

int setSharpIQPara(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_asharp_SetIQPara(ctx, (rk_aiq_sharp_IQpara_t *)data);
}

int getSharpIQPara(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_asharp_GetIQPara(ctx, (rk_aiq_sharp_IQpara_t *)data);
}

int setSharpStrength(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_asharp_SetStrength(ctx, *(float*)data);
}

int getSharpStrength(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_asharp_GetStrength(ctx, (float*)data);
}
