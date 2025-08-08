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

#include "sample_sysctl.h"
#include "sample_comm.h"

void sample_print_sysctl_info(const void *arg)
{
    printf("test sysctl !\n");
}

static void sample_sysctl_usage()
{
    printf("Usage : \n");
    printf("\t 0) SYSCTL:         sample_pause_resume.\n");
    printf("\t 1) SYSCTL:         sample_mirror_flip.\n");
    printf("\t 2) SYSCTL:         sample_getHdrComprCurve.\n");
    printf("\n");
    printf("\t please press the key: ");

    return;
}

static void sample_pause_resume(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    static bool g_pause = false;
    if (!g_pause) {
        ret = rk_aiq_uapi2_sysctl_pause((rk_aiq_sys_ctx_t*)ctx, true);
    } else {
        ret = rk_aiq_uapi2_sysctl_resume((rk_aiq_sys_ctx_t*)ctx);
    }
    g_pause = !g_pause;
    printf("%s done, ret: %d, pause:%d\n", __func__, ret, g_pause);
}

static void sample_mirrflip(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    static int g_mirrflip = 0;
    bool flip = g_mirrflip & 0x1;
    bool mirror = g_mirrflip & 0x2;
    ret = rk_aiq_uapi2_setMirrorFlip((rk_aiq_sys_ctx_t*)ctx, mirror, flip, 3);
    g_mirrflip++;
    g_mirrflip %= 4;
    printf("%s done, ret: %d, flip :%d, mirror:%d\n", __func__, ret, flip, mirror);
}

static void sample_getHdrComprCurve(const rk_aiq_sys_ctx_t* ctx) {
#ifdef USE_NEWSTRUCT
    XCamReturn ret        = XCAM_RETURN_NO_ERROR;
    RkAiqHdrCompr_t compr = {0};
    ret                   = rk_aiq_uapi2_sysctl_getHdrComprCurve((rk_aiq_sys_ctx_t*)ctx, &compr);
    if (ret != XCAM_RETURN_NO_ERROR) {
        printf("get hdr compr curve failed, ret: %d\n", ret);
        return;
    }
#endif
}

XCamReturn sample_sysctl(const void *arg)
{
    int key = -1;
    CLEAR();

    const demo_context_t *demo_ctx = (demo_context_t *)arg;
    const rk_aiq_sys_ctx_t* ctx ;
    if (demo_ctx->camGroup) {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->camgroup_ctx);
    } else {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->aiq_ctx);
    }

    do {
        sample_sysctl_usage ();

        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar();
        printf ("\n");

        switch (key) {
            case '0': {
                printf("\t sample_pause_resume\n\n");
                sample_pause_resume(ctx);
                break;
            }
            case '1': {
                printf("\t sample_mirrflip\n\n");
                sample_mirrflip(ctx);
                break;
            }
            case '2': {
                printf("\t sample_getHdrComprCurve\n\n");
                sample_getHdrComprCurve(ctx);
                break;
            }
            default:
                break;
        }
    } while (key != 'q' && key != 'Q');

    return XCAM_RETURN_NO_ERROR;
}
