#include "socket_server.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "rkaiq_tool_imgproc.cpp"


int setGrayMode(rk_aiq_sys_ctx_t* ctx, char* data) {
  return rk_aiq_uapi_setGrayMode(ctx, *(rk_aiq_gray_mode_t *)data);
}

int getGrayMode(rk_aiq_sys_ctx_t* ctx) {
  return rk_aiq_uapi_getGrayMode(ctx);
}
