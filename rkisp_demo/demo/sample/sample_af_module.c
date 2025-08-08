/*
 *  Copyright (c) 2019 Rockchip Corporation
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

#include "sample_comm.h"
#include "iq_parser_v2/af_head.h"
#include "isp/rk_aiq_isp_af33.h"

#if !defined(ISP_HW_V33)
static void sample_af_usage()
{
    printf("\n\nUsage : \n");
    printf("\t 0) AF:         setFocusMode-OP_AUTO.\n");
    printf("\t 1) AF:         setFocusMode-OP_MANUAL.\n");
    printf("\t 2) AF:         setFocusMode-OP_SEMI_AUTO.\n");
    printf("\t 3) AF:         getFocusMode.\n");
    printf("\t 4) AF:         setFocusWin.\n");
    printf("\t 5) AF:         setFocusWin to defalut.\n");
    printf("\t 6) AF:         getFocusWin.\n");
    printf("\t 7) AF:         lockFocus.\n");
    printf("\t 8) AF:         unlockFocus.\n");
    printf("\t 9) AF:         oneshotFocus.\n");
    printf("\t a) AF:         manualTrigerFocus.\n");
    printf("\t b) AF:         trackingFocus.\n");
    printf("\t c) AF:         startZoomCalib.\n");
    printf("\t d) AF:         resetZoom.\n");
    printf("\t e) AF:         setAngleZ.\n");
    printf("\t f) AF:         + ZoomPosition.\n");
    printf("\t g) AF:         - ZoomPosition.\n");
    printf("\t i) AF:         + FocusPosition.\n");
    printf("\t j) AF:         - FocusPosition.\n");
    printf("\t k) AF:         getSearchResult.\n");
    printf("\t l) AF:         getAfAttrib.\n");
    printf("\t m) AF:         getAfCalib.\n");
    printf("\t n) AF:         setAfCalib.\n");
    printf("\t q) AF:         return to main sample screen.\n");

    printf("\n");
    printf("\t please press the key: ");

    return;
}

void sample_set_focus_automode(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_uapi2_setFocusMode(ctx, OP_AUTO);
    printf("setFocusMode auto\n");
}

void sample_set_focus_manualmode(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_uapi2_setFocusMode(ctx, OP_MANUAL);
    printf("setFocusMode manual\n");
}

void sample_set_focus_semiautomode(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_uapi2_setFocusMode(ctx, OP_SEMI_AUTO);
    printf("setFocusMode semi-auto\n");
}

void sample_get_focus_mode(const rk_aiq_sys_ctx_t* ctx)
{
    opMode_t mode;

    rk_aiq_uapi2_getFocusMode(ctx, &mode);
    if (mode == OP_AUTO) {
        printf("getFocusMode auto\n");
    } else if (mode == OP_MANUAL) {
        printf("getFocusMode manual\n");
    } else if (mode == OP_SEMI_AUTO) {
        printf("getFocusMode semi-auto\n");
    } else {
        printf("getFocusMode unknow mode %d\n", mode);
    }
}

void sample_set_focus_win(const rk_aiq_sys_ctx_t* ctx)
{
    paRect_t rect;

    rect.x = 4;
    rect.y = 4;
    rect.w = 800;
    rect.h = 600;
    rk_aiq_uapi2_setFocusWin(ctx, &rect);
    printf("setFocusWin rect: x %d, y %d, w %d, h %d\n", rect.x, rect.y, rect.w, rect.h);
}

void sample_set_focus_defwin(const rk_aiq_sys_ctx_t* ctx)
{
    paRect_t rect;

    memset(&rect, 0, sizeof(rect));
    rk_aiq_uapi2_setFocusWin(ctx, &rect);
    printf("setFocusWin rect to default value\n");
}

void sample_get_focus_win(const rk_aiq_sys_ctx_t* ctx)
{
    paRect_t rect;

    memset(&rect, 0, sizeof(rect));
    rk_aiq_uapi2_getFocusWin(ctx, &rect);
    printf("getFocusWin rect: x %d, y %d, w %d, h %d\n", rect.x, rect.y, rect.w, rect.h);
}

void sample_lock_focus(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_uapi2_lockFocus(ctx);
    printf("lockFocus\n");
}

void sample_unlock_focus(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_uapi2_unlockFocus(ctx);
    printf("unlockFocus\n");
}

void sample_oneshot_focus(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_uapi2_oneshotFocus(ctx);
    printf("oneshotFocus\n");
}

void sample_manual_triger_focus(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_uapi2_manualTrigerFocus(ctx);
    printf("manualTrigerFocus\n");
}

void sample_tracking_focus(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_uapi2_trackingFocus(ctx);
    printf("trackingFocus\n");
}

void sample_start_zoomcalib(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_uapi2_startZoomCalib(ctx);
    printf("startZoomCalib\n");
}

void sample_reset_zoom(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_uapi2_resetZoom(ctx);
    printf("resetZoom\n");
}

void sample_set_anglez(const rk_aiq_sys_ctx_t* ctx)
{
    float angleZ = 90;
    rk_aiq_uapi2_setAngleZ(ctx, angleZ);
    printf("setAngleZ %f degree\n", angleZ);
}

void sample_add_zoom_position(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_af_zoomrange range;
    int code;

    rk_aiq_uapi2_getZoomRange(ctx, &range);
    printf("zoom.min_pos %d, zoom.max_pos %d\n", range.min_pos, range.max_pos);

    rk_aiq_uapi2_getOpZoomPosition(ctx, &code);
    printf("getOpZoomPosition code %d\n", code);

    code += 20;
    if (code > range.max_pos)
        code = range.max_pos;
    rk_aiq_uapi2_setOpZoomPosition(ctx, code);
    rk_aiq_uapi2_endOpZoomChange(ctx);
    printf("setOpZoomPosition %d\n", code);
}

void sample_sub_zoom_position(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_af_zoomrange range;
    int code;

    rk_aiq_uapi2_getZoomRange(ctx, &range);
    printf("zoom.min_pos %d, zoom.max_pos %d\n", range.min_pos, range.max_pos);

    rk_aiq_uapi2_getOpZoomPosition(ctx, &code);
    printf("getOpZoomPosition code %d\n", code);

    code -= 20;
    if (code < range.min_pos)
        code = range.min_pos;
    rk_aiq_uapi2_setOpZoomPosition(ctx, code);
    rk_aiq_uapi2_endOpZoomChange(ctx);
    printf("setOpZoomPosition %d\n", code);
}

void sample_add_focus_position(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_af_focusrange range;
    short code;

    rk_aiq_uapi2_getFocusRange(ctx, &range);
    printf("focus.min_pos %d, focus.max_pos %d\n", range.min_pos, range.max_pos);

    rk_aiq_uapi2_getFocusPosition(ctx, &code);

    code++;
    if (code > range.max_pos)
        code = range.max_pos;
    rk_aiq_uapi2_setFocusPosition(ctx, code);
    printf("set Focus Code %d\n", code);
}

void sample_sub_focus_position(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_af_focusrange range;
    short code;

    rk_aiq_uapi2_getFocusRange(ctx, &range);
    printf("focus.min_pos %d, focus.max_pos %d\n", range.min_pos, range.max_pos);

    rk_aiq_uapi2_getFocusPosition(ctx, &code);

    code--;
    if (code < range.min_pos)
        code = range.min_pos;
    rk_aiq_uapi2_setFocusPosition(ctx, code);
    printf("set Focus Code %d\n", code);
}

void sample_get_af_search_result(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_af_result_t result;
    rk_aiq_uapi2_getSearchResult(ctx, &result);
    printf("get search result, stat: %d, final_pos: %d\n", result.stat, result.final_pos);
}

void sample_get_af_attrib(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_af_attrib_t attr;
    memset(&attr, 0, sizeof(attr));
#if RKAIQ_HAVE_AF_V33
    rk_aiq_user_api2_af_GetAttrib(ctx, &attr);
#endif
    printf("get sync.done %d, sync_mode %d\n", attr.sync.done, attr.sync.sync_mode);
}

void sample_get_af_calib(const rk_aiq_sys_ctx_t* ctx)
{
    if (CHECK_ISP_HW_V20() || CHECK_ISP_HW_V21()) {
        CalibDbV2_AF_Tuning_Para_t af_calib_cfg_v20;
        rk_aiq_user_api2_af_GetCalib(ctx, (void *)&af_calib_cfg_v20);
    } else if (CHECK_ISP_HW_V30()) {
        CalibDbV2_AFV30_Tuning_Para_t af_calib_cfg_v30;
        rk_aiq_user_api2_af_GetCalib(ctx, (void *)&af_calib_cfg_v30);
    } else if (CHECK_ISP_HW_V32()) {
        CalibDbV2_AFV31_Tuning_Para_t af_calib_cfg_v31;
        rk_aiq_user_api2_af_GetCalib(ctx, (void *)&af_calib_cfg_v31);
    } else if (CHECK_ISP_HW_V32_LITE()) {
        CalibDbV2_AFV32_Tuning_Para_t af_calib_cfg_v32;
        rk_aiq_user_api2_af_GetCalib(ctx, (void *)&af_calib_cfg_v32);
    } else if (CHECK_ISP_HW_V39()) {
        CalibDbV2_AFV33_t af_calib_cfg_v33;
        rk_aiq_user_api2_af_GetCalib(ctx, (void *)&af_calib_cfg_v33);
    } else if (CHECK_ISP_HW_V35()) {
        af_param_t af_calib;
        rk_aiq_user_api2_af_GetCalib(ctx, (void *)&af_calib);
    }
}

void sample_set_af_calib(const rk_aiq_sys_ctx_t* ctx)
{
    if (CHECK_ISP_HW_V20() || CHECK_ISP_HW_V21()) {
        CalibDbV2_AF_Tuning_Para_t af_calib_cfg_v20;
        memset(&af_calib_cfg_v20, 0, sizeof(af_calib_cfg_v20));
        rk_aiq_user_api2_af_GetCalib(ctx, (void *)&af_calib_cfg_v20);
        af_calib_cfg_v20.af_mode = CalibDbV2_AFMODE_AUTO;
        rk_aiq_user_api2_af_SetCalib(ctx, (void *)&af_calib_cfg_v20);
    } else if (CHECK_ISP_HW_V30()) {
        CalibDbV2_AFV30_Tuning_Para_t af_calib_cfg_v30;
        memset(&af_calib_cfg_v30, 0, sizeof(af_calib_cfg_v30));
        rk_aiq_user_api2_af_GetCalib(ctx, (void *)&af_calib_cfg_v30);
        af_calib_cfg_v30.af_mode = CalibDbV2_AFMODE_AUTO;
        rk_aiq_user_api2_af_SetCalib(ctx, (void *)&af_calib_cfg_v30);
    } else if (CHECK_ISP_HW_V32()) {
        CalibDbV2_AFV31_Tuning_Para_t af_calib_cfg_v31;
        memset(&af_calib_cfg_v31, 0, sizeof(af_calib_cfg_v31));
        rk_aiq_user_api2_af_GetCalib(ctx, (void *)&af_calib_cfg_v31);
        af_calib_cfg_v31.af_mode = CalibDbV2_AFMODE_AUTO;
        rk_aiq_user_api2_af_SetCalib(ctx, (void *)&af_calib_cfg_v31);
    } else if (CHECK_ISP_HW_V32_LITE()) {
        CalibDbV2_AFV32_Tuning_Para_t af_calib_cfg_v32;
        memset(&af_calib_cfg_v32, 0, sizeof(af_calib_cfg_v32));
        rk_aiq_user_api2_af_GetCalib(ctx, (void *)&af_calib_cfg_v32);
        af_calib_cfg_v32.af_mode = CalibDbV2_AFMODE_AUTO;
        rk_aiq_user_api2_af_SetCalib(ctx, (void *)&af_calib_cfg_v32);
    } else if (CHECK_ISP_HW_V39()) {
        CalibDbV2_AFV33_t af_calib_cfg_v33;
        memset(&af_calib_cfg_v33, 0, sizeof(af_calib_cfg_v33));
        rk_aiq_user_api2_af_GetCalib(ctx, (void *)&af_calib_cfg_v33);
        af_calib_cfg_v33.Common.AfMode = CalibDbV2_AFMODE_AUTO;
        rk_aiq_user_api2_af_SetCalib(ctx, (void *)&af_calib_cfg_v33);
    } else if (CHECK_ISP_HW_V35()) {
        af_param_t af_calib;
        memset(&af_calib, 0, sizeof(af_calib));
        rk_aiq_user_api2_af_GetCalib(ctx, (void *)&af_calib);
        af_calib.common.sw_afT_af_mode = af_mode_auto;
        rk_aiq_user_api2_af_SetCalib(ctx, (void *)&af_calib);
    }
}
#endif
void sample_print_af_info(const void *arg)
{
    printf ("enter AF modult test!\n");
}
#if !defined(ISP_HW_V33)
void sample_af_case(const rk_aiq_sys_ctx_t* ctx) {
    
    sample_set_focus_automode(ctx);
    sample_set_focus_manualmode(ctx);
    sample_set_focus_semiautomode(ctx);
    sample_get_focus_mode(ctx);
    sample_set_focus_win(ctx);
    sample_set_focus_defwin(ctx);
    sample_get_focus_win(ctx);
    sample_lock_focus(ctx);
    sample_unlock_focus(ctx);
    sample_oneshot_focus(ctx);
    sample_manual_triger_focus(ctx);
    sample_tracking_focus(ctx);
    sample_start_zoomcalib(ctx);
    sample_reset_zoom(ctx);
    sample_set_anglez(ctx);
    sample_add_zoom_position(ctx);
    sample_sub_zoom_position(ctx);
    sample_add_focus_position(ctx);
    sample_sub_focus_position(ctx);
    sample_get_af_search_result(ctx);
    sample_get_af_attrib(ctx);
    sample_get_af_calib(ctx);
    sample_set_af_calib(ctx);
}
#endif
XCamReturn sample_af_module(const void *arg)
{
#if !defined(ISP_HW_V33)
    int key = -1;
    CLEAR();
    rk_aiq_wb_scene_t scene;
    rk_aiq_wb_gain_t gain;
    rk_aiq_wb_cct_t ct;
    opMode_t mode;
    const demo_context_t *demo_ctx = (demo_context_t *)arg;
    const rk_aiq_sys_ctx_t* ctx = (const rk_aiq_sys_ctx_t*)(demo_ctx->aiq_ctx);
    if (ctx == NULL) {
        ERR ("%s, ctx is nullptr\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    do {
        sample_af_usage();

        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar();
        printf ("\n\n");

        switch (key)
        {
            case 'h':
                sample_af_usage();
                break;
            case '0':
                sample_set_focus_automode(ctx);
                break;
            case '1':
                sample_set_focus_manualmode(ctx);
                break;
            case '2':
                sample_set_focus_semiautomode(ctx);
                break;
            case '3':
                sample_get_focus_mode(ctx);
                break;
            case '4':
                sample_set_focus_win(ctx);
                break;
            case '5':
                sample_set_focus_defwin(ctx);
                break;
            case '6':
                sample_get_focus_win(ctx);
                break;
            case '7':
                sample_lock_focus(ctx);
                break;
            case '8':
                sample_unlock_focus(ctx);
                break;
            case '9':
                sample_oneshot_focus(ctx);
                break;
            case 'a':
                sample_manual_triger_focus(ctx);
                break;
            case 'b':
                sample_tracking_focus(ctx);
                break;
            case 'c':
                sample_start_zoomcalib(ctx);
                break;
            case 'd':
                sample_reset_zoom(ctx);
                break;
            case 'e':
                sample_set_anglez(ctx);
                break;
            case 'f':
                sample_add_zoom_position(ctx);
                break;
            case 'g':
                sample_sub_zoom_position(ctx);
                break;
            case 'i':
                sample_add_focus_position(ctx);
                break;
            case 'j':
                sample_sub_focus_position(ctx);
                break;
            case 'k':
                sample_get_af_search_result(ctx);
                break;
            case 'l':
                sample_get_af_attrib(ctx);
                break;
            case 'm':
                sample_get_af_calib(ctx);
                break;
            case 'n':
                sample_set_af_calib(ctx);
                break;

            default:
                break;
        }
    } while (key != 'q' && key != 'Q');
#endif
    return XCAM_RETURN_NO_ERROR;
}


