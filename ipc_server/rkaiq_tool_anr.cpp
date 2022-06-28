#include "socket_server.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rkaiq_tool_anr.cpp"

int setAnrAttrib(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_anr_SetAttrib(ctx, (rk_aiq_nr_attrib_t *)data);
}

int getAnrAttrib(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_anr_GetAttrib(ctx, (rk_aiq_nr_attrib_t *)data);
}

int setAnrIQPara(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_anr_SetIQPara(ctx, (rk_aiq_nr_IQPara_t *) data);
}

int getAnrIQPara(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_anr_GetIQPara(ctx, (rk_aiq_nr_IQPara_t *) data);
}

int setLumaSFStrength(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_anr_SetLumaSFStrength(ctx, *(float *)data);
}

int setLumaTFStrength(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_anr_SetLumaTFStrength(ctx, *(float *)data);
}

int getLumaSFStrength(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_anr_GetLumaSFStrength(ctx, (float *)data);
}

int getLumaTFStrength(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_anr_GetLumaTFStrength(ctx, (float *)data);
}

int setChromaSFStrength(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_anr_SetChromaSFStrength(ctx, *(float *)data);
}

int setChromaTFStrength(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_anr_SetChromaTFStrength(ctx, *(float *)data);
}

int getChromaSFStrength(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_anr_GetChromaSFStrength(ctx, (float *)data);
}

int getChromaTFStrength(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_anr_GetChromaTFStrength(ctx, (float *)data);
}

int setRawnrSFStrength(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_anr_SetRawnrSFStrength(ctx, *(float *)data);
}

int getRawnrSFStrength(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_user_api_anr_GetRawnrSFStrength(ctx, (float *)data);
}
