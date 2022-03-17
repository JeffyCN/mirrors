
#ifndef _RK_AIQ_API_PRIVATE
#define _RK_AIQ_API_PRIVATE

extern "C" {

typedef struct rk_aiq_sys_ctx_s rk_aiq_sys_ctx_t;

rk_aiq_sys_ctx_t* get_next_ctx(const rk_aiq_sys_ctx_t* ctx);
bool is_ctx_need_bypass(const rk_aiq_sys_ctx_t* ctx);
void rk_aiq_ctx_set_tool_mode(const rk_aiq_sys_ctx_t* ctx, bool status);

#define CHECK_USER_API_ENABLE2(ctx) \
    if (is_ctx_need_bypass(ctx)) { return XCAM_RETURN_NO_ERROR; }

}

#endif
