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

static void sample_adehaze_usage()
{
    printf("Usage : \n");
    printf("\t 0) ADEHAZE:         test test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_MANUAL Sync.\n");
    printf("\t 1) ADEHAZE:         test test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_MANUAL Async.\n");
    printf("\t 2) ADEHAZE:         test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_DEHAZE_AUTO Sync.\n");
    printf("\t 3) ADEHAZE:         test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_DEHAZE_AUTO Async.\n");
    printf("\t 4) ADEHAZE:         test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_DEHAZE_MANUAL Sync.\n");
    printf("\t 5) ADEHAZE:         test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_DEHAZE_MANUAL Async.\n");
    printf("\t 6) ADEHAZE:         test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_DEHAZE_OFF Sync.\n");
    printf("\t 7) ADEHAZE:         test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_DEHAZE_OFF Async.\n");
    printf("\t 8) ADEHAZE:         test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_ENHANCE_MANUAL Sync.\n");
    printf("\t 9) ADEHAZE:         test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_ENHANCE_MANUAL Async.\n");
    printf("\t a) ADEHAZE:         test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_ENHANCE_AUTO Sync.\n");
    printf("\t b) ADEHAZE:         test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_ENHANCE_AUTO Async.\n");
    printf("\t c) ADEHAZE:         test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_ENHANCE_OFF Sync.\n");
    printf("\t d) ADEHAZE:         test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_ENHANCE_OFF Async.\n");
    printf("\t e) ADEHAZE:         test rk_aiq_user_api2_adehaze_getSwAttrib.\n");
    printf("\t f) ADEHAZE:         test rk_aiq_uapi2_getMDehazeStrth.\n");
    printf("\t g) ADEHAZE:         test rk_aiq_uapi2_setMDehazeStrth.\n");
    printf("\t h) ADEHAZE:         test rk_aiq_uapi2_getMEnhanceStrth.\n");
    printf("\t i) ADEHAZE:         test rk_aiq_uapi2_setMEnhanceStrth.\n");
    printf("\t q) ADEHAZE:         return to main sample screen.\n");

    printf("\n");
    printf("\t please press the key: ");

    return;
}

void sample_print_adehaze_info(const void *arg)
{
    printf ("enter ADEHAZE modult test!\n");
}

XCamReturn sample_adehaze_module(const void *arg)
{
    int key = -1;
    CLEAR();

    adehaze_sw_V2_t attr;
    const demo_context_t *demo_ctx = (demo_context_t *)arg;
    const rk_aiq_sys_ctx_t* ctx ;
    if (demo_ctx->camGroup) {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->camgroup_ctx);
    } else {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->aiq_ctx);
    }

    do {
        sample_adehaze_usage ();

        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar();
        printf ("\n");


        switch (key)
        {
        case '0': {
            printf("\t ADEHAZE test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_MANUAL Sync\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            attr.sync.done = false;
            attr.mode = DEHAZE_API_MANUAL;
            attr.stManual.Enable = true;
            attr.stManual.cfg_alpha = 1.0;
            attr.stManual.dehaze_setting.en = false;
            attr.stManual.dehaze_setting.air_lc_en = true;
            attr.stManual.dehaze_setting.stab_fnum = 8;
            attr.stManual.dehaze_setting.sigma = 6;
            attr.stManual.dehaze_setting.wt_sigma = 8;
            attr.stManual.dehaze_setting.air_sigma = 120;
            attr.stManual.dehaze_setting.tmax_sigma = 0.01;
            attr.stManual.dehaze_setting.pre_wet = 0.01;
            attr.stManual.dehaze_setting.DehazeData.dc_min_th = 64;
            attr.stManual.dehaze_setting.DehazeData.dc_max_th = 192;
            attr.stManual.dehaze_setting.DehazeData.yhist_th = 249;
            attr.stManual.dehaze_setting.DehazeData.yblk_th = 0.002;
            attr.stManual.dehaze_setting.DehazeData.dark_th = 250;
            attr.stManual.dehaze_setting.DehazeData.bright_min = 180;
            attr.stManual.dehaze_setting.DehazeData.bright_max = 240;
            attr.stManual.dehaze_setting.DehazeData.wt_max = 0.9;
            attr.stManual.dehaze_setting.DehazeData.air_min = 200;
            attr.stManual.dehaze_setting.DehazeData.air_max = 250;
            attr.stManual.dehaze_setting.DehazeData.tmax_base = 125;
            attr.stManual.dehaze_setting.DehazeData.tmax_off = 0.1;
            attr.stManual.dehaze_setting.DehazeData.tmax_max = 0.8;
            attr.stManual.dehaze_setting.DehazeData.cfg_wt = 0.8;
            attr.stManual.dehaze_setting.DehazeData.cfg_air = 210;
            attr.stManual.dehaze_setting.DehazeData.cfg_tmax = 0.2;
            attr.stManual.dehaze_setting.DehazeData.dc_weitcur = 1;
            attr.stManual.dehaze_setting.DehazeData.bf_weight = 0.5;
            attr.stManual.dehaze_setting.DehazeData.range_sigma = 0.14;
            attr.stManual.dehaze_setting.DehazeData.space_sigma_pre = 0.14;
            attr.stManual.dehaze_setting.DehazeData.space_sigma_cur = 0.14;

            attr.stManual.enhance_setting.en = true;
            attr.stManual.enhance_setting.EnhanceData.enhance_value = 1.0;
            attr.stManual.enhance_setting.EnhanceData.enhance_chroma = 1.0;
            attr.stManual.enhance_setting.enhance_curve[0] = 0;
            attr.stManual.enhance_setting.enhance_curve[1] = 64;
            attr.stManual.enhance_setting.enhance_curve[2] = 128;
            attr.stManual.enhance_setting.enhance_curve[3] = 192;
            attr.stManual.enhance_setting.enhance_curve[4] = 256;
            attr.stManual.enhance_setting.enhance_curve[5] = 320;
            attr.stManual.enhance_setting.enhance_curve[6] = 384;
            attr.stManual.enhance_setting.enhance_curve[7] = 448;
            attr.stManual.enhance_setting.enhance_curve[8] = 512;
            attr.stManual.enhance_setting.enhance_curve[9] = 576;
            attr.stManual.enhance_setting.enhance_curve[10] = 640;
            attr.stManual.enhance_setting.enhance_curve[11] = 704;
            attr.stManual.enhance_setting.enhance_curve[12] = 768;
            attr.stManual.enhance_setting.enhance_curve[13] = 832;
            attr.stManual.enhance_setting.enhance_curve[14] = 896;
            attr.stManual.enhance_setting.enhance_curve[15] = 960;
            attr.stManual.enhance_setting.enhance_curve[16] = 1023;

            attr.stManual.hist_setting.en = false;
            attr.stManual.hist_setting.hist_para_en = true;
            attr.stManual.hist_setting.HistData.hist_gratio = 2;
            attr.stManual.hist_setting.HistData.hist_th_off = 64;
            attr.stManual.hist_setting.HistData.hist_k = 2;
            attr.stManual.hist_setting.HistData.hist_min = 0.015;
            attr.stManual.hist_setting.HistData.hist_scale = 0.09;
            attr.stManual.hist_setting.HistData.cfg_gratio = 2;

            attr.stManual.sigma_curve[0] = -7.80229e-013;
            attr.stManual.sigma_curve[1] = -7.80229e-013;
            attr.stManual.sigma_curve[2] = -2.20431e-005;
            attr.stManual.sigma_curve[3] = 0.0298751;
            attr.stManual.sigma_curve[4] = 10.9382;
            rk_aiq_user_api2_adehaze_setSwAttrib(ctx, attr);
            break;
        }
        case '1': {
            printf("\t ADEHAZE test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_MANUAL Async\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
            attr.sync.done = false;
            attr.mode = DEHAZE_API_MANUAL;
            attr.stManual.Enable                                    = true;
            attr.stManual.cfg_alpha = 0.0;
            attr.stManual.dehaze_setting.en = false;
            attr.stManual.dehaze_setting.air_lc_en = true;
            attr.stManual.dehaze_setting.stab_fnum = 8;
            attr.stManual.dehaze_setting.sigma = 6;
            attr.stManual.dehaze_setting.wt_sigma = 8;
            attr.stManual.dehaze_setting.air_sigma = 120;
            attr.stManual.dehaze_setting.tmax_sigma = 0.01;
            attr.stManual.dehaze_setting.pre_wet = 0.01;
            attr.stManual.dehaze_setting.DehazeData.dc_min_th = 64;
            attr.stManual.dehaze_setting.DehazeData.dc_max_th = 192;
            attr.stManual.dehaze_setting.DehazeData.yhist_th = 249;
            attr.stManual.dehaze_setting.DehazeData.yblk_th = 0.002;
            attr.stManual.dehaze_setting.DehazeData.dark_th = 250;
            attr.stManual.dehaze_setting.DehazeData.bright_min = 180;
            attr.stManual.dehaze_setting.DehazeData.bright_max = 240;
            attr.stManual.dehaze_setting.DehazeData.wt_max = 0.9;
            attr.stManual.dehaze_setting.DehazeData.air_min = 200;
            attr.stManual.dehaze_setting.DehazeData.air_max = 250;
            attr.stManual.dehaze_setting.DehazeData.tmax_base = 125;
            attr.stManual.dehaze_setting.DehazeData.tmax_off = 0.1;
            attr.stManual.dehaze_setting.DehazeData.tmax_max = 0.8;
            attr.stManual.dehaze_setting.DehazeData.cfg_wt = 0.8;
            attr.stManual.dehaze_setting.DehazeData.cfg_air = 210;
            attr.stManual.dehaze_setting.DehazeData.cfg_tmax = 0.2;
            attr.stManual.dehaze_setting.DehazeData.dc_weitcur = 1;
            attr.stManual.dehaze_setting.DehazeData.bf_weight = 0.5;
            attr.stManual.dehaze_setting.DehazeData.range_sigma = 0.14;
            attr.stManual.dehaze_setting.DehazeData.space_sigma_pre = 0.14;
            attr.stManual.dehaze_setting.DehazeData.space_sigma_cur = 0.14;

            attr.stManual.enhance_setting.en = true;
            attr.stManual.enhance_setting.EnhanceData.enhance_value = 1.0;
            attr.stManual.enhance_setting.EnhanceData.enhance_chroma = 1.0;
            attr.stManual.enhance_setting.enhance_curve[0] = 0;
            attr.stManual.enhance_setting.enhance_curve[1] = 64;
            attr.stManual.enhance_setting.enhance_curve[2] = 128;
            attr.stManual.enhance_setting.enhance_curve[3] = 192;
            attr.stManual.enhance_setting.enhance_curve[4] = 256;
            attr.stManual.enhance_setting.enhance_curve[5] = 320;
            attr.stManual.enhance_setting.enhance_curve[6] = 384;
            attr.stManual.enhance_setting.enhance_curve[7] = 448;
            attr.stManual.enhance_setting.enhance_curve[8] = 512;
            attr.stManual.enhance_setting.enhance_curve[9] = 576;
            attr.stManual.enhance_setting.enhance_curve[10] = 640;
            attr.stManual.enhance_setting.enhance_curve[11] = 704;
            attr.stManual.enhance_setting.enhance_curve[12] = 768;
            attr.stManual.enhance_setting.enhance_curve[13] = 832;
            attr.stManual.enhance_setting.enhance_curve[14] = 896;
            attr.stManual.enhance_setting.enhance_curve[15] = 960;
            attr.stManual.enhance_setting.enhance_curve[16] = 1023;

            attr.stManual.hist_setting.en = false;
            attr.stManual.hist_setting.hist_para_en = true;
            attr.stManual.hist_setting.HistData.hist_gratio = 2;
            attr.stManual.hist_setting.HistData.hist_th_off = 64;
            attr.stManual.hist_setting.HistData.hist_k = 2;
            attr.stManual.hist_setting.HistData.hist_min = 0.015;
            attr.stManual.hist_setting.HistData.hist_scale = 0.09;
            attr.stManual.hist_setting.HistData.cfg_gratio = 2;

            attr.stManual.sigma_curve[0] = -7.80229e-013;
            attr.stManual.sigma_curve[1] = -7.80229e-013;
            attr.stManual.sigma_curve[2] = -2.20431e-005;
            attr.stManual.sigma_curve[3] = 0.0298751;
            attr.stManual.sigma_curve[4] = 18.7607;
            rk_aiq_user_api2_adehaze_setSwAttrib(ctx, attr);
            break;
        }
        case '2': {
            printf("\t ADEHAZE test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_DEHAZE_AUTO Sync\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            attr.sync.done = false;
            attr.mode = DEHAZE_API_DEHAZE_AUTO;
            rk_aiq_user_api2_adehaze_setSwAttrib(ctx, attr);
            break;
        }
        case '3': {
            printf("\t ADEHAZE test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_DEHAZE_AUTO Async\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
            attr.sync.done = false;
            attr.mode = DEHAZE_API_DEHAZE_AUTO;
            rk_aiq_user_api2_adehaze_setSwAttrib(ctx, attr);
            break;
        }
        case '4': {
            printf("\t ADEHAZE test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_DEHAZE_MANUAL Sync\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            attr.sync.done = false;
            attr.mode = DEHAZE_API_DEHAZE_MANUAL;
            attr.stDehazeManu.level = 75;
            rk_aiq_user_api2_adehaze_setSwAttrib(ctx, attr);
            break;
        }
        case '5': {
            printf("\t ADEHAZE test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_DEHAZE_MANUAL Async\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
            attr.sync.done = false;
            attr.mode = DEHAZE_API_DEHAZE_MANUAL;
            attr.stDehazeManu.level = 70;
            rk_aiq_user_api2_adehaze_setSwAttrib(ctx, attr);
            break;
        }
        case '6': {
            printf("\t ADEHAZE test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_DEHAZE_OFF Sync\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            attr.sync.done = false;
            attr.mode = DEHAZE_API_DEHAZE_OFF;
            rk_aiq_user_api2_adehaze_setSwAttrib(ctx, attr);
            break;
        }
        case '7': {
            printf("\t ADEHAZE test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_DEHAZE_OFF Async\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
            attr.sync.done = false;
            attr.mode = DEHAZE_API_DEHAZE_OFF;
            rk_aiq_user_api2_adehaze_setSwAttrib(ctx, attr);
            break;
        }
        case '8': {
            printf("\t ADEHAZE test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_ENHANCE_MANUAL Sync\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            attr.sync.done = false;
            attr.mode = DEHAZE_API_ENHANCE_MANUAL;
            attr.stEnhanceManu.level = 78;
            rk_aiq_user_api2_adehaze_setSwAttrib(ctx, attr);
            break;
        }
        case '9': {
            printf("\t ADEHAZE test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_ENHANCE_MANUAL Async\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
            attr.sync.done = false;
            attr.mode = DEHAZE_API_ENHANCE_MANUAL;
            attr.stEnhanceManu.level = 70;
            rk_aiq_user_api2_adehaze_setSwAttrib(ctx, attr);
            break;
        }
        case 'a': {
            printf("\t ADEHAZE test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_ENHANCE_AUTO Sync\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            attr.sync.done = false;
            attr.mode = DEHAZE_API_ENHANCE_AUTO;
            rk_aiq_user_api2_adehaze_setSwAttrib(ctx, attr);
            break;
        }
        case 'b': {
            printf("\t ADEHAZE test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_ENHANCE_AUTO Async\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
            attr.sync.done = false;
            attr.mode = DEHAZE_API_ENHANCE_AUTO;
            rk_aiq_user_api2_adehaze_setSwAttrib(ctx, attr);
            break;
        }
        case 'c': {
            printf("\t ADEHAZE test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_ENHANCE_OFF Sync\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            attr.sync.done = false;
            attr.mode = DEHAZE_API_ENHANCE_OFF;
            rk_aiq_user_api2_adehaze_setSwAttrib(ctx, attr);
            break;
        }
        case 'd': {
            printf("\t ADEHAZE test rk_aiq_user_api2_adehaze_setSwAttrib DEHAZE_API_ENHANCE_OFF Async\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
            attr.sync.done = false;
            attr.mode = DEHAZE_API_ENHANCE_OFF;
            rk_aiq_user_api2_adehaze_setSwAttrib(ctx, attr);
            break;
        }
        case 'e': {
            printf("\t ADEHAZE test rk_aiq_user_api2_adehaze_getSwAttrib\n\n");
            rk_aiq_user_api2_adehaze_getSwAttrib(ctx, &attr);
            printf("\t sync = %d, done = %d\n", attr.sync.sync_mode, attr.sync.done);
            printf("\t mode: %d\n\n", attr.mode);
            printf("\t stManual Enable: %d cfg_alpha:%f\n\n", attr.stManual.Enable, attr.stManual.cfg_alpha);
            printf("\t stDehazeManu level: %d\n\n", attr.stDehazeManu.level);
            printf("\t stEnhanceManu level: %d\n\n", attr.stEnhanceManu.level);
            printf("\t sigma_curve[3]:%f sigma_curve[4]:%f\n\n", attr.stManual.sigma_curve[3],
                   attr.stManual.sigma_curve[4]);
            break;
        }
        case 'f': {
            printf("\t ADEHAZE test rk_aiq_uapi2_getMDehazeStrth\n\n");
            unsigned int level = 60;
            rk_aiq_uapi2_getMDehazeStrth(ctx, &level);
            printf("\t rk_aiq_uapi2_getMDehazeStrth level: %d\n\n", level);
            break;
        }
        case 'g': {
            printf("\t ADEHAZE test rk_aiq_uapi2_setMDehazeStrth\n\n");
            unsigned int level = 70;
            rk_aiq_uapi2_setMDehazeStrth(ctx, level);
            printf("\t rk_aiq_uapi2_setMDehazeStrth level: %d\n\n", level);
            break;
        }
        case 'h': {
            printf("\t ADEHAZE test rk_aiq_uapi2_getMEnhanceStrth\n\n");
            unsigned int level = 60;
            rk_aiq_uapi2_getMEnhanceStrth(ctx, &level);
            printf("\t rk_aiq_uapi2_getMEnhanceStrth level: %d\n\n", level);
            break;
        }
        case 'i': {
            printf("\t ADEHAZE test rk_aiq_uapi2_setMEnhanceStrth\n\n");
            unsigned int level = 70;
            rk_aiq_uapi2_setMEnhanceStrth(ctx, level);
            printf("\t rk_aiq_uapi2_setMEnhanceStrth level: %d\n\n", level);
            break;
        }
        default:
            break;
        }
    } while (key != 'q' && key != 'Q');

    return XCAM_RETURN_NO_ERROR;
}
