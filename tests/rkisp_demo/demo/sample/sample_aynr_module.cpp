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

static void sample_aynr_usage()
{
    printf("Usage : \n");
    printf("\t 0) AYNR:         get aynr attri on sync mode.\n");
    printf("\t 1) AYNR:         get aynr strength on sync mode.\n");
    printf("\t 2) AYNR:         set aynr attri auto on sync mode.\n");
    printf("\t 3) AYNR:         set aynr attri manual on sync mode.\n");
    printf("\t 4) AYNR:         set aynr attri reg value on sync mode.\n");
    printf("\t 5) AYNR:         set aynr strength max value 1.0 on sync mode, only on auto mode has effect.\n");
    printf("\t 6) AYNR:         set aynr strength min value 0.0 on sync mode, only on auto mode has effect.\n");
    printf("\t 7) AYNR:         set aynr strength med value 0.5 on sync mode, only on auto mode has effect.\n");
    printf("\t 8) AYNR:         set aynr attri to default vaule on sync mode.\n");
    printf("\t a) AYNR:         get aynr attri on async mode.\n");
    printf("\t b) AYNR:         get aynr strength on async mode.\n");
    printf("\t c) AYNR:         set aynr attri auto on async mode.\n");
    printf("\t d) AYNR:         set aynr attri manual on async mode.\n");
    printf("\t e) AYNR:         set aynr attri reg value on async mode.\n");
    printf("\t f) AYNR:         set aynr strength max value 1.0 on async mode, only on auto mode has effect.\n");
    printf("\t g) AYNR:         set aynr strength min value 0.0 on async mode, only on auto mode has effect.\n");
    printf("\t h) AYNR:         set aynr strength med value 0.5 on async mode, only on auto mode has effect.\n");
    printf("\t i) AYNR:         set aynr attri to default vaule on async mode.\n");
    printf("\t q) AYNR:         press key q or Q to quit.\n");

}

void sample_print_aynr_info(const void *arg)
{
    printf ("enter AYNR modult test!\n");
}


XCamReturn sample_aynr_module (const void *arg)
{
    int key = -1;
    CLEAR();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    const demo_context_t *demo_ctx = (demo_context_t *)arg;
    const rk_aiq_sys_ctx_t* ctx ;

    if(demo_ctx->camGroup) {
        ctx = (const rk_aiq_sys_ctx_t*)(demo_ctx->camgroup_ctx);
        printf("##################group !!!!########################\n");
    } else {
        ctx = (const rk_aiq_sys_ctx_t*)(demo_ctx->aiq_ctx);
        printf("##################sigle !!!!########################\n");
    }

    if (ctx == nullptr) {
        ERR ("%s, ctx is nullptr\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    rk_aiq_ynr_attrib_v3_t default_ynrV3_attr;
    rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &default_ynrV3_attr);

    do {
        sample_aynr_usage ();

        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar ();
        printf ("\n");

        switch (key) {
        case '0':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_ynr_attrib_v3_t ynrV3_attr;
                ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &ynrV3_attr);
                printf("get aynr v3 attri ret:%d done:%d \n\n", ret, ynrV3_attr.sync.done);
            }
            break;
        case '1':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_ynr_strength_v3_t ynrV3_strength;
                ynrV3_strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_aynrV3_GetStrength(ctx, &ynrV3_strength);
                printf("get aynr v3 attri ret:%d strength:%f done:%d\n\n",
                       ret, ynrV3_strength.percent, ynrV3_strength.sync.done);
            }
            break;
        case '2':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_ynr_attrib_v3_t ynrV3_attr;
                ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &ynrV3_attr);
                ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ynrV3_attr.eMode = AYNRV3_OP_MODE_AUTO;
                ynrV3_attr.stAuto.stParams.enable = 1;
                for(int i = 0; i < RK_YNR_V3_MAX_ISO_NUM; i++) {
                    ynrV3_attr.stAuto.stParams.iso[i] = 50 * pow(2, i);

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[0] = 0x000;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[1] = 0x040;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[2] = 0x080;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[3] = 0x0c0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[4] = 0x100;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[5] = 0x140;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[6] = 0x180;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[7] = 0x1c0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[8] = 0x200;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[9] = 0x240;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[10] = 0x280;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[11] = 0x2c0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[12] = 0x300;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[13] = 0x340;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[14] = 0x380;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[15] = 0x3c0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[16] = 0x400;

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[0] = 16;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[1] = 20;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[2] = 24;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[3] = 30;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[4] = 40;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[5] = 60;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[6] = 40;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[7] = 38;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[8] = 36;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[9] = 32;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[10] = 30;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[11] = 28;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[12] = 24;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[13] = 24;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[14] = 20;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[15] = 20;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[16] = 16;

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lci = 1.0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].hci = 1.0;

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_bft3x3_bypass = 0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_lbft5x5_bypass = 0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_lgft3x3_bypass = 0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_flt1x1_bypass = 0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_sft5x5_bypass = 0;

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_global_gain_alpha = 0.0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_global_gain = 1.0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_adjust_thresh = 0.3;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_adjust_scale = 1.0;

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_bf1 = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_bf2 = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_thred_adj = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_peak_supress = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_edge_adj_thresh = 7;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_lbf_weight_thresh = 0.25;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_center_weight = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_dist_adj = 0.8;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_weight = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_filt1_strength = 0.7;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_filt2_strength = 0.85;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_bi_weight = 0.3;

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].base_filter_weight1 = 0.28;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].base_filter_weight1 = 0.46;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].base_filter_weight1 = 0.26;

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_thred_adj = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_weight = 0.78;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].hi_min_adj = 0.9;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].hi_edge_thed = 100;

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_direction_weight[0] = 1.0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_direction_weight[1] = 1.0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_direction_weight[2] = 1.0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_direction_weight[3] = 1.0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_direction_weight[4] = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_direction_weight[5] = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_direction_weight[6] = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_direction_weight[7] = 0.5;

                    for(int j = 0; j < 6; j++) {
                        ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lo_ratio[j] = 1.0;
                        ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].hi_ratio[j] = 1.0;
                    }

                    for(int j = 0; j < 17; j++) {
                        ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].rnr_strength[j] = 1.0;
                    }
                }

                ret = rk_aiq_user_api2_aynrV3_SetAttrib(ctx, &ynrV3_attr);
                printf("set aynr attri auto ret:%d \n\n", ret);

                rk_aiq_ynr_attrib_v3_t get_ynrV3_attr;
                get_ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &get_ynrV3_attr);
                printf("get aynr v3 attri ret:%d done:%d \n\n", ret, get_ynrV3_attr.sync.done);
            }
            break;
        case '3':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_ynr_attrib_v3_t ynrV3_attr;
                ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &ynrV3_attr);
                ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ynrV3_attr.eMode = AYNRV3_OP_MODE_MANUAL;
                ynrV3_attr.stManual.stSelect.enable = 1;

                ynrV3_attr.stManual.stSelect.lumaPoint[0] = 0x000;
                ynrV3_attr.stManual.stSelect.lumaPoint[1] = 0x040;
                ynrV3_attr.stManual.stSelect.lumaPoint[2] = 0x080;
                ynrV3_attr.stManual.stSelect.lumaPoint[3] = 0x0c0;
                ynrV3_attr.stManual.stSelect.lumaPoint[4] = 0x100;
                ynrV3_attr.stManual.stSelect.lumaPoint[5] = 0x140;
                ynrV3_attr.stManual.stSelect.lumaPoint[6] = 0x180;
                ynrV3_attr.stManual.stSelect.lumaPoint[7] = 0x1c0;
                ynrV3_attr.stManual.stSelect.lumaPoint[8] = 0x200;
                ynrV3_attr.stManual.stSelect.lumaPoint[9] = 0x240;
                ynrV3_attr.stManual.stSelect.lumaPoint[10] = 0x280;
                ynrV3_attr.stManual.stSelect.lumaPoint[11] = 0x2c0;
                ynrV3_attr.stManual.stSelect.lumaPoint[12] = 0x300;
                ynrV3_attr.stManual.stSelect.lumaPoint[13] = 0x340;
                ynrV3_attr.stManual.stSelect.lumaPoint[14] = 0x380;
                ynrV3_attr.stManual.stSelect.lumaPoint[15] = 0x3c0;
                ynrV3_attr.stManual.stSelect.lumaPoint[16] = 0x400;

                ynrV3_attr.stManual.stSelect.sigma[0] = 16;
                ynrV3_attr.stManual.stSelect.sigma[1] = 20;
                ynrV3_attr.stManual.stSelect.sigma[2] = 24;
                ynrV3_attr.stManual.stSelect.sigma[3] = 30;
                ynrV3_attr.stManual.stSelect.sigma[4] = 40;
                ynrV3_attr.stManual.stSelect.sigma[5] = 60;
                ynrV3_attr.stManual.stSelect.sigma[6] = 40;
                ynrV3_attr.stManual.stSelect.sigma[7] = 38;
                ynrV3_attr.stManual.stSelect.sigma[8] = 36;
                ynrV3_attr.stManual.stSelect.sigma[9] = 32;
                ynrV3_attr.stManual.stSelect.sigma[10] = 30;
                ynrV3_attr.stManual.stSelect.sigma[11] = 28;
                ynrV3_attr.stManual.stSelect.sigma[12] = 24;
                ynrV3_attr.stManual.stSelect.sigma[13] = 24;
                ynrV3_attr.stManual.stSelect.sigma[14] = 20;
                ynrV3_attr.stManual.stSelect.sigma[15] = 20;
                ynrV3_attr.stManual.stSelect.sigma[16] = 16;

                ynrV3_attr.stManual.stSelect.lci = 1.0;
                ynrV3_attr.stManual.stSelect.hci = 1.0;

                ynrV3_attr.stManual.stSelect.ynr_bft3x3_bypass = 0;
                ynrV3_attr.stManual.stSelect.ynr_lbft5x5_bypass = 0;
                ynrV3_attr.stManual.stSelect.ynr_lgft3x3_bypass = 0;
                ynrV3_attr.stManual.stSelect.ynr_flt1x1_bypass = 0;
                ynrV3_attr.stManual.stSelect.ynr_sft5x5_bypass = 0;

                ynrV3_attr.stManual.stSelect.ynr_global_gain_alpha = 0.0;
                ynrV3_attr.stManual.stSelect.ynr_global_gain = 1.0;
                ynrV3_attr.stManual.stSelect.ynr_adjust_thresh = 0.3;
                ynrV3_attr.stManual.stSelect.ynr_adjust_scale = 1.0;

                ynrV3_attr.stManual.stSelect.low_bf1 = 0.5;
                ynrV3_attr.stManual.stSelect.low_bf2 = 0.5;
                ynrV3_attr.stManual.stSelect.low_thred_adj = 0.5;
                ynrV3_attr.stManual.stSelect.low_peak_supress = 0.5;
                ynrV3_attr.stManual.stSelect.low_edge_adj_thresh = 7;
                ynrV3_attr.stManual.stSelect.low_lbf_weight_thresh = 0.25;
                ynrV3_attr.stManual.stSelect.low_center_weight = 0.5;
                ynrV3_attr.stManual.stSelect.low_dist_adj = 0.8;
                ynrV3_attr.stManual.stSelect.low_weight = 0.5;
                ynrV3_attr.stManual.stSelect.low_filt1_strength = 0.7;
                ynrV3_attr.stManual.stSelect.low_filt2_strength = 0.85;
                ynrV3_attr.stManual.stSelect.low_bi_weight = 0.3;

                ynrV3_attr.stManual.stSelect.base_filter_weight1 = 0.28;
                ynrV3_attr.stManual.stSelect.base_filter_weight1 = 0.46;
                ynrV3_attr.stManual.stSelect.base_filter_weight1 = 0.26;

                ynrV3_attr.stManual.stSelect.high_thred_adj = 0.5;
                ynrV3_attr.stManual.stSelect.high_weight = 0.78;
                ynrV3_attr.stManual.stSelect.hi_min_adj = 0.9;
                ynrV3_attr.stManual.stSelect.hi_edge_thed = 100;

                ynrV3_attr.stManual.stSelect.high_direction_weight[0] = 1.0;
                ynrV3_attr.stManual.stSelect.high_direction_weight[1] = 1.0;
                ynrV3_attr.stManual.stSelect.high_direction_weight[2] = 1.0;
                ynrV3_attr.stManual.stSelect.high_direction_weight[3] = 1.0;
                ynrV3_attr.stManual.stSelect.high_direction_weight[4] = 0.5;
                ynrV3_attr.stManual.stSelect.high_direction_weight[5] = 0.5;
                ynrV3_attr.stManual.stSelect.high_direction_weight[6] = 0.5;
                ynrV3_attr.stManual.stSelect.high_direction_weight[7] = 0.5;

                for(int j = 0; j < 6; j++) {
                    ynrV3_attr.stManual.stSelect.lo_ratio[j] = 1.0;
                    ynrV3_attr.stManual.stSelect.hi_ratio[j] = 1.0;
                }

                for(int j = 0; j < 17; j++) {
                    ynrV3_attr.stManual.stSelect.rnr_strength[j] = 1.0;
                }

                ret = rk_aiq_user_api2_aynrV3_SetAttrib(ctx, &ynrV3_attr);
                printf("set ynr v3 attri manual ret:%d \n\n", ret);

                rk_aiq_ynr_attrib_v3_t get_ynrV3_attr;
                get_ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &get_ynrV3_attr);
                printf("get aynr v3 attri ret:%d done:%d \n\n", ret, get_ynrV3_attr.sync.done);
            }

            break;
        case '4':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_ynr_attrib_v3_t ynrV3_attr;
                ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &ynrV3_attr);
                ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ynrV3_attr.eMode = AYNRV3_OP_MODE_REG_MANUAL;

                // YNR_2700_GLOBAL_CTRL (0x0000)
                ynrV3_attr.stManual.stFix.ynr_rnr_en = 1;
                ynrV3_attr.stManual.stFix.ynr_gate_dis = 0;
                ynrV3_attr.stManual.stFix.ynr_thumb_mix_cur_en = 0;
                ynrV3_attr.stManual.stFix.ynr_global_gain_alpha = 0x0;
                ynrV3_attr.stManual.stFix.ynr_global_gain = 0x10;
                ynrV3_attr.stManual.stFix.ynr_flt1x1_bypass_sel = 0;
                ynrV3_attr.stManual.stFix.ynr_sft5x5_bypass = 0;
                ynrV3_attr.stManual.stFix.ynr_flt1x1_bypass = 0;
                ynrV3_attr.stManual.stFix.ynr_lgft3x3_bypass = 0;
                ynrV3_attr.stManual.stFix.ynr_lbft5x5_bypass = 0;
                ynrV3_attr.stManual.stFix.ynr_bft3x3_bypass = 0;
                ynrV3_attr.stManual.stFix.ynr_en = 1;

                // YNR_2700_RNR_MAX_R  (0x0004)
                ynrV3_attr.stManual.stFix.ynr_local_gainscale = 0x80;
                ynrV3_attr.stManual.stFix.ynr_rnr_max_r = 0x3754;

                // YNR_2700_RNR_MAX_R  (0x0008)
                ynrV3_attr.stManual.stFix.ynr_rnr_center_coorv = 0x0000;
                ynrV3_attr.stManual.stFix.ynr_rnr_center_coorh = 0x0001;

                // YNR_2700_RNR_MAX_R  (0x000c)
                ynrV3_attr.stManual.stFix.ynr_localgain_adj = 0x10;
                ynrV3_attr.stManual.stFix.ynr_localgain_adj_thresh = 0x10;

                // YNR_2700_LOWNR_CTRL0 (0x0010)
                ynrV3_attr.stManual.stFix.ynr_low_bf_inv[1] = 0x100;
                ynrV3_attr.stManual.stFix.ynr_low_bf_inv[0] = 0x100;

                // YNR_2700_LOWNR_CTRL1  (0x0014)
                ynrV3_attr.stManual.stFix.ynr_low_peak_supress = 0x00;
                ynrV3_attr.stManual.stFix.ynr_low_thred_adj = 0x80;

                // YNR_2700_LOWNR_CTRL2 (0x0018)
                ynrV3_attr.stManual.stFix.ynr_low_dist_adj = 0x010;
                ynrV3_attr.stManual.stFix.ynr_low_edge_adj_thresh = 0x007;

                // YNR_2700_LOWNR_CTRL3 (0x001c)
                ynrV3_attr.stManual.stFix.ynr_low_bi_weight = 0x40;
                ynrV3_attr.stManual.stFix.ynr_low_weight = 0x64;
                ynrV3_attr.stManual.stFix.ynr_low_center_weight = 0x133;

                // YNR_2700_HIGHNR_CTRL0 (0x0020)
                ynrV3_attr.stManual.stFix.ynr_hi_min_adj = 0x3a;
                ynrV3_attr.stManual.stFix.ynr_high_thred_adj = 0x080;

                // YNR_2700_HIGHNR_CTRL1  (0x0024)
                ynrV3_attr.stManual.stFix.ynr_high_retain_weight = 0x6e;
                ynrV3_attr.stManual.stFix.ynr_hi_edge_thed = 0x64;

                // YNR_2700_HIGHNR_BASE_FILTER_WEIGHT  (0x0028)
                ynrV3_attr.stManual.stFix.ynr_base_filter_weight[0] = 0x12;
                ynrV3_attr.stManual.stFix.ynr_base_filter_weight[1] = 0x0f;
                ynrV3_attr.stManual.stFix.ynr_base_filter_weight[2] = 0x8;

                // YNR_2700_HIGHNR_CTRL1  (0x002c)
                ynrV3_attr.stManual.stFix.ynr_frame_full_size = 0x0000;
                ynrV3_attr.stManual.stFix.ynr_lbf_weight_thres = 0x000;

                // YNR_2700_GAUSS1_COEFF  (0x0030)
                ynrV3_attr.stManual.stFix.ynr_low_gauss1_coeff[0] = 0x0b;
                ynrV3_attr.stManual.stFix.ynr_low_gauss1_coeff[1] = 0x20;
                ynrV3_attr.stManual.stFix.ynr_low_gauss1_coeff[2] = 0x054;

                // YNR_2700_GAUSS2_COEFF  (0x0034)
                ynrV3_attr.stManual.stFix.ynr_low_gauss2_coeff[0] = 0x10;
                ynrV3_attr.stManual.stFix.ynr_low_gauss2_coeff[1] = 0x20;
                ynrV3_attr.stManual.stFix.ynr_low_gauss2_coeff[2] = 0x040;

                // YNR_2700_DIRECTION_W_0_3  (0x0038 - 0x003c)
                ynrV3_attr.stManual.stFix.ynr_direction_weight[0] = 0x08;
                ynrV3_attr.stManual.stFix.ynr_direction_weight[1] = 0x08;
                ynrV3_attr.stManual.stFix.ynr_direction_weight[2] = 0x08;
                ynrV3_attr.stManual.stFix.ynr_direction_weight[3] = 0x08;
                ynrV3_attr.stManual.stFix.ynr_direction_weight[4] = 0x04;
                ynrV3_attr.stManual.stFix.ynr_direction_weight[5] = 0x04;
                ynrV3_attr.stManual.stFix.ynr_direction_weight[6] = 0x04;
                ynrV3_attr.stManual.stFix.ynr_direction_weight[7] = 0x04;


                // YNR_2700_SGM_DX_0_1 (0x0040 - 0x0060)
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[0] = 0x000;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[1] = 0x040;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[2] = 0x080;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[3] = 0x0c0;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[4] = 0x100;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[5] = 0x140;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[6] = 0x180;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[7] = 0x1c0;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[8] = 0x200;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[9] = 0x240;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[10] = 0x280;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[11] = 0x2c0;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[12] = 0x300;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[13] = 0x340;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[14] = 0x380;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[15] = 0x3c0;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[16] = 0x400;


                // YNR_2700_LSGM_Y_0_1 (0x0070- 0x0090)
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[0] = 0x04c;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[1] = 0x0a0;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[2] = 0x0d0;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[3] = 0x0e4;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[4] = 0x0e8;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[5] = 0x0e4;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[6] = 0x0d8;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[7] = 0x0cc;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[8] = 0x0c4;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[9] = 0x0c0;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[10] = 0x0bc;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[11] = 0x0b8;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[12] = 0x0b4;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[13] = 0x0a4;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[14] = 0x088;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[15] = 0x054;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[16] = 0x028;

                // YNR_2700_HSGM_Y_0_1 (0x00a0- 0x00c0)
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[0] = 0x04c;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[1] = 0x0a0;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[2] = 0x0d0;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[3] = 0x0e4;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[4] = 0x0e8;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[5] = 0x0e4;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[6] = 0x0d8;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[7] = 0x0cc;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[8] = 0x0c4;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[9] = 0x0c0;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[10] = 0x0bc;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[11] = 0x0b8;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[12] = 0x0b4;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[13] = 0x0a4;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[14] = 0x088;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[15] = 0x054;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[16] = 0x028;

                for(int j = 0; j < 17; j++) {
                    // YNR_2700_RNR_STRENGTH03 (0x00d0- 0x00e0)
                    ynrV3_attr.stManual.stFix.ynr_rnr_strength[j] = 0x10;
                }

                ret = rk_aiq_user_api2_aynrV3_SetAttrib(ctx, &ynrV3_attr);
                printf("set ynr v3 attri manual ret:%d \n\n", ret);

                rk_aiq_ynr_attrib_v3_t get_ynrV3_attr;
                get_ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &get_ynrV3_attr);
                printf("get aynr v3 attri ret:%d done:%d \n\n", ret, get_ynrV3_attr.sync.done);
            }
            break;
        case '5':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_ynr_strength_v3_t ynrV3_strength;
                ynrV3_strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ynrV3_strength.strength_enable = true;
                ynrV3_strength.percent = 1.0;
                ret = rk_aiq_user_api2_aynrV3_SetStrength(ctx, &ynrV3_strength);
                printf("Set aynr v3 set streangth ret:%d strength:%f \n\n", ret, ynrV3_strength.percent);

                rk_aiq_ynr_strength_v3_t get_ynrV3_strength;
                get_ynrV3_strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_aynrV3_GetStrength(ctx, &get_ynrV3_strength);
                printf("get aynr v3 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_ynrV3_strength.percent, get_ynrV3_strength.sync.done);
            }
            break;
        case '6':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_ynr_strength_v3_t ynrV3_strength;
                ynrV3_strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ynrV3_strength.strength_enable = true;
                ynrV3_strength.percent = 0.0;
                ret = rk_aiq_user_api2_aynrV3_SetStrength(ctx, &ynrV3_strength);
                printf("Set aynr v3 set streangth ret:%d strength:%f \n\n", ret, ynrV3_strength.percent);

                rk_aiq_ynr_strength_v3_t get_ynrV3_strength;
                get_ynrV3_strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_aynrV3_GetStrength(ctx, &get_ynrV3_strength);
                printf("get aynr v3 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_ynrV3_strength.percent, get_ynrV3_strength.sync.done);
            }
            break;
        case '7':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_ynr_strength_v3_t ynrV3_strength;
                ynrV3_strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ynrV3_strength.strength_enable = true;
                ynrV3_strength.percent = 0.5;
                ret = rk_aiq_user_api2_aynrV3_SetStrength(ctx, &ynrV3_strength);
                printf("Set aynr v3 set streangth ret:%d strength:%f \n\n", ret, ynrV3_strength.percent);

                rk_aiq_ynr_strength_v3_t get_ynrV3_strength;
                get_ynrV3_strength.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_aynrV3_GetStrength(ctx, &get_ynrV3_strength);
                printf("get aynr v3 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_ynrV3_strength.percent, get_ynrV3_strength.sync.done);
            }
            break;
        case '8':
            if (CHECK_ISP_HW_V30()) {
                default_ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_aynrV3_SetAttrib(ctx, &default_ynrV3_attr);
                printf("set ynr v3 attri auto default value ret:%d \n\n", ret);

                ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &default_ynrV3_attr);
                printf("get aynr v3 attri ret:%d done:%d \n\n", ret, default_ynrV3_attr.sync.done);
            }
            break;
        case 'a':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_ynr_attrib_v3_t ynrV3_attr;
                ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &ynrV3_attr);
                printf("get aynr v3 attri ret:%d done:%d \n\n", ret, ynrV3_attr.sync.done);
            }
            break;
        case 'b':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_ynr_strength_v3_t ynrV3_strength;
                ynrV3_strength.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_aynrV3_GetStrength(ctx, &ynrV3_strength);
                printf("get aynr v3 attri ret:%d strength:%f done:%d\n\n",
                       ret, ynrV3_strength.percent, ynrV3_strength.sync.done);
            }
            break;
        case 'c':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_ynr_attrib_v3_t ynrV3_attr;
                ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &ynrV3_attr);
                ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ynrV3_attr.eMode = AYNRV3_OP_MODE_AUTO;
                ynrV3_attr.stAuto.stParams.enable = 1;
                for(int i = 0; i < RK_YNR_V3_MAX_ISO_NUM; i++) {
                    ynrV3_attr.stAuto.stParams.iso[i] = 50 * pow(2, i);

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[0] = 0x000;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[1] = 0x040;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[2] = 0x080;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[3] = 0x0c0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[4] = 0x100;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[5] = 0x140;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[6] = 0x180;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[7] = 0x1c0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[8] = 0x200;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[9] = 0x240;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[10] = 0x280;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[11] = 0x2c0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[12] = 0x300;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[13] = 0x340;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[14] = 0x380;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[15] = 0x3c0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lumaPoint[16] = 0x400;

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[0] = 16;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[1] = 20;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[2] = 24;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[3] = 30;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[4] = 40;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[5] = 60;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[6] = 40;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[7] = 38;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[8] = 36;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[9] = 32;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[10] = 30;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[11] = 28;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[12] = 24;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[13] = 24;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[14] = 20;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[15] = 20;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].sigma[16] = 16;

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lci = 1.0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].hci = 1.0;

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_bft3x3_bypass = 0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_lbft5x5_bypass = 0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_lgft3x3_bypass = 0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_flt1x1_bypass = 0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_sft5x5_bypass = 0;

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_global_gain_alpha = 0.0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_global_gain = 1.0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_adjust_thresh = 0.3;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].ynr_adjust_scale = 1.0;

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_bf1 = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_bf2 = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_thred_adj = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_peak_supress = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_edge_adj_thresh = 7;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_lbf_weight_thresh = 0.25;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_center_weight = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_dist_adj = 0.8;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_weight = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_filt1_strength = 0.7;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_filt2_strength = 0.85;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].low_bi_weight = 0.3;

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].base_filter_weight1 = 0.28;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].base_filter_weight1 = 0.46;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].base_filter_weight1 = 0.26;

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_thred_adj = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_weight = 0.78;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].hi_min_adj = 0.9;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].hi_edge_thed = 100;

                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_direction_weight[0] = 1.0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_direction_weight[1] = 1.0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_direction_weight[2] = 1.0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_direction_weight[3] = 1.0;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_direction_weight[4] = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_direction_weight[5] = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_direction_weight[6] = 0.5;
                    ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].high_direction_weight[7] = 0.5;

                    for(int j = 0; j < 6; j++) {
                        ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].lo_ratio[j] = 1.0;
                        ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].hi_ratio[j] = 1.0;
                    }

                    for(int j = 0; j < 17; j++) {
                        ynrV3_attr.stAuto.stParams.arYnrParamsISO[i].rnr_strength[j] = 1.0;
                    }
                }

                ret = rk_aiq_user_api2_aynrV3_SetAttrib(ctx, &ynrV3_attr);
                printf("set aynr attri auto ret:%d \n\n", ret);

                rk_aiq_ynr_attrib_v3_t get_ynrV3_attr;
                get_ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &get_ynrV3_attr);
                printf("get aynr v3 attri ret:%d done:%d \n\n", ret, get_ynrV3_attr.sync.done);
            }
            break;
        case 'd':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_ynr_attrib_v3_t ynrV3_attr;
                ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &ynrV3_attr);
                ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ynrV3_attr.eMode = AYNRV3_OP_MODE_MANUAL;
                ynrV3_attr.stManual.stSelect.enable = 1;

                ynrV3_attr.stManual.stSelect.lumaPoint[0] = 0x000;
                ynrV3_attr.stManual.stSelect.lumaPoint[1] = 0x040;
                ynrV3_attr.stManual.stSelect.lumaPoint[2] = 0x080;
                ynrV3_attr.stManual.stSelect.lumaPoint[3] = 0x0c0;
                ynrV3_attr.stManual.stSelect.lumaPoint[4] = 0x100;
                ynrV3_attr.stManual.stSelect.lumaPoint[5] = 0x140;
                ynrV3_attr.stManual.stSelect.lumaPoint[6] = 0x180;
                ynrV3_attr.stManual.stSelect.lumaPoint[7] = 0x1c0;
                ynrV3_attr.stManual.stSelect.lumaPoint[8] = 0x200;
                ynrV3_attr.stManual.stSelect.lumaPoint[9] = 0x240;
                ynrV3_attr.stManual.stSelect.lumaPoint[10] = 0x280;
                ynrV3_attr.stManual.stSelect.lumaPoint[11] = 0x2c0;
                ynrV3_attr.stManual.stSelect.lumaPoint[12] = 0x300;
                ynrV3_attr.stManual.stSelect.lumaPoint[13] = 0x340;
                ynrV3_attr.stManual.stSelect.lumaPoint[14] = 0x380;
                ynrV3_attr.stManual.stSelect.lumaPoint[15] = 0x3c0;
                ynrV3_attr.stManual.stSelect.lumaPoint[16] = 0x400;

                ynrV3_attr.stManual.stSelect.sigma[0] = 16;
                ynrV3_attr.stManual.stSelect.sigma[1] = 20;
                ynrV3_attr.stManual.stSelect.sigma[2] = 24;
                ynrV3_attr.stManual.stSelect.sigma[3] = 30;
                ynrV3_attr.stManual.stSelect.sigma[4] = 40;
                ynrV3_attr.stManual.stSelect.sigma[5] = 60;
                ynrV3_attr.stManual.stSelect.sigma[6] = 40;
                ynrV3_attr.stManual.stSelect.sigma[7] = 38;
                ynrV3_attr.stManual.stSelect.sigma[8] = 36;
                ynrV3_attr.stManual.stSelect.sigma[9] = 32;
                ynrV3_attr.stManual.stSelect.sigma[10] = 30;
                ynrV3_attr.stManual.stSelect.sigma[11] = 28;
                ynrV3_attr.stManual.stSelect.sigma[12] = 24;
                ynrV3_attr.stManual.stSelect.sigma[13] = 24;
                ynrV3_attr.stManual.stSelect.sigma[14] = 20;
                ynrV3_attr.stManual.stSelect.sigma[15] = 20;
                ynrV3_attr.stManual.stSelect.sigma[16] = 16;

                ynrV3_attr.stManual.stSelect.lci = 1.0;
                ynrV3_attr.stManual.stSelect.hci = 1.0;

                ynrV3_attr.stManual.stSelect.ynr_bft3x3_bypass = 0;
                ynrV3_attr.stManual.stSelect.ynr_lbft5x5_bypass = 0;
                ynrV3_attr.stManual.stSelect.ynr_lgft3x3_bypass = 0;
                ynrV3_attr.stManual.stSelect.ynr_flt1x1_bypass = 0;
                ynrV3_attr.stManual.stSelect.ynr_sft5x5_bypass = 0;

                ynrV3_attr.stManual.stSelect.ynr_global_gain_alpha = 0.0;
                ynrV3_attr.stManual.stSelect.ynr_global_gain = 1.0;
                ynrV3_attr.stManual.stSelect.ynr_adjust_thresh = 0.3;
                ynrV3_attr.stManual.stSelect.ynr_adjust_scale = 1.0;

                ynrV3_attr.stManual.stSelect.low_bf1 = 0.5;
                ynrV3_attr.stManual.stSelect.low_bf2 = 0.5;
                ynrV3_attr.stManual.stSelect.low_thred_adj = 0.5;
                ynrV3_attr.stManual.stSelect.low_peak_supress = 0.5;
                ynrV3_attr.stManual.stSelect.low_edge_adj_thresh = 7;
                ynrV3_attr.stManual.stSelect.low_lbf_weight_thresh = 0.25;
                ynrV3_attr.stManual.stSelect.low_center_weight = 0.5;
                ynrV3_attr.stManual.stSelect.low_dist_adj = 0.8;
                ynrV3_attr.stManual.stSelect.low_weight = 0.5;
                ynrV3_attr.stManual.stSelect.low_filt1_strength = 0.7;
                ynrV3_attr.stManual.stSelect.low_filt2_strength = 0.85;
                ynrV3_attr.stManual.stSelect.low_bi_weight = 0.3;

                ynrV3_attr.stManual.stSelect.base_filter_weight1 = 0.28;
                ynrV3_attr.stManual.stSelect.base_filter_weight1 = 0.46;
                ynrV3_attr.stManual.stSelect.base_filter_weight1 = 0.26;

                ynrV3_attr.stManual.stSelect.high_thred_adj = 0.5;
                ynrV3_attr.stManual.stSelect.high_weight = 0.78;
                ynrV3_attr.stManual.stSelect.hi_min_adj = 0.9;
                ynrV3_attr.stManual.stSelect.hi_edge_thed = 100;

                ynrV3_attr.stManual.stSelect.high_direction_weight[0] = 1.0;
                ynrV3_attr.stManual.stSelect.high_direction_weight[1] = 1.0;
                ynrV3_attr.stManual.stSelect.high_direction_weight[2] = 1.0;
                ynrV3_attr.stManual.stSelect.high_direction_weight[3] = 1.0;
                ynrV3_attr.stManual.stSelect.high_direction_weight[4] = 0.5;
                ynrV3_attr.stManual.stSelect.high_direction_weight[5] = 0.5;
                ynrV3_attr.stManual.stSelect.high_direction_weight[6] = 0.5;
                ynrV3_attr.stManual.stSelect.high_direction_weight[7] = 0.5;

                for(int j = 0; j < 6; j++) {
                    ynrV3_attr.stManual.stSelect.lo_ratio[j] = 1.0;
                    ynrV3_attr.stManual.stSelect.hi_ratio[j] = 1.0;
                }

                for(int j = 0; j < 17; j++) {
                    ynrV3_attr.stManual.stSelect.rnr_strength[j] = 1.0;
                }

                ret = rk_aiq_user_api2_aynrV3_SetAttrib(ctx, &ynrV3_attr);
                printf("set ynr v3 attri manual ret:%d \n\n", ret);

                rk_aiq_ynr_attrib_v3_t get_ynrV3_attr;
                get_ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &get_ynrV3_attr);
                printf("get aynr v3 attri ret:%d done:%d \n\n", ret, get_ynrV3_attr.sync.done);
            }

            break;
        case 'e':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_ynr_attrib_v3_t ynrV3_attr;
                ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &ynrV3_attr);
                ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ynrV3_attr.eMode = AYNRV3_OP_MODE_REG_MANUAL;

                // YNR_2700_GLOBAL_CTRL (0x0000)
                ynrV3_attr.stManual.stFix.ynr_rnr_en = 1;
                ynrV3_attr.stManual.stFix.ynr_gate_dis = 0;
                ynrV3_attr.stManual.stFix.ynr_thumb_mix_cur_en = 0;
                ynrV3_attr.stManual.stFix.ynr_global_gain_alpha = 0x0;
                ynrV3_attr.stManual.stFix.ynr_global_gain = 0x10;
                ynrV3_attr.stManual.stFix.ynr_flt1x1_bypass_sel = 0;
                ynrV3_attr.stManual.stFix.ynr_sft5x5_bypass = 0;
                ynrV3_attr.stManual.stFix.ynr_flt1x1_bypass = 0;
                ynrV3_attr.stManual.stFix.ynr_lgft3x3_bypass = 0;
                ynrV3_attr.stManual.stFix.ynr_lbft5x5_bypass = 0;
                ynrV3_attr.stManual.stFix.ynr_bft3x3_bypass = 0;
                ynrV3_attr.stManual.stFix.ynr_en = 1;

                // YNR_2700_RNR_MAX_R  (0x0004)
                ynrV3_attr.stManual.stFix.ynr_local_gainscale = 0x80;
                ynrV3_attr.stManual.stFix.ynr_rnr_max_r = 0x3754;

                // YNR_2700_RNR_MAX_R  (0x0008)
                ynrV3_attr.stManual.stFix.ynr_rnr_center_coorv = 0x0000;
                ynrV3_attr.stManual.stFix.ynr_rnr_center_coorh = 0x0001;

                // YNR_2700_RNR_MAX_R  (0x000c)
                ynrV3_attr.stManual.stFix.ynr_localgain_adj = 0x10;
                ynrV3_attr.stManual.stFix.ynr_localgain_adj_thresh = 0x10;

                // YNR_2700_LOWNR_CTRL0 (0x0010)
                ynrV3_attr.stManual.stFix.ynr_low_bf_inv[1] = 0x100;
                ynrV3_attr.stManual.stFix.ynr_low_bf_inv[0] = 0x100;

                // YNR_2700_LOWNR_CTRL1  (0x0014)
                ynrV3_attr.stManual.stFix.ynr_low_peak_supress = 0x00;
                ynrV3_attr.stManual.stFix.ynr_low_thred_adj = 0x80;

                // YNR_2700_LOWNR_CTRL2 (0x0018)
                ynrV3_attr.stManual.stFix.ynr_low_dist_adj = 0x010;
                ynrV3_attr.stManual.stFix.ynr_low_edge_adj_thresh = 0x007;

                // YNR_2700_LOWNR_CTRL3 (0x001c)
                ynrV3_attr.stManual.stFix.ynr_low_bi_weight = 0x40;
                ynrV3_attr.stManual.stFix.ynr_low_weight = 0x64;
                ynrV3_attr.stManual.stFix.ynr_low_center_weight = 0x133;

                // YNR_2700_HIGHNR_CTRL0 (0x0020)
                ynrV3_attr.stManual.stFix.ynr_hi_min_adj = 0x3a;
                ynrV3_attr.stManual.stFix.ynr_high_thred_adj = 0x080;

                // YNR_2700_HIGHNR_CTRL1  (0x0024)
                ynrV3_attr.stManual.stFix.ynr_high_retain_weight = 0x6e;
                ynrV3_attr.stManual.stFix.ynr_hi_edge_thed = 0x64;

                // YNR_2700_HIGHNR_BASE_FILTER_WEIGHT  (0x0028)
                ynrV3_attr.stManual.stFix.ynr_base_filter_weight[0] = 0x12;
                ynrV3_attr.stManual.stFix.ynr_base_filter_weight[1] = 0x0f;
                ynrV3_attr.stManual.stFix.ynr_base_filter_weight[2] = 0x8;

                // YNR_2700_HIGHNR_CTRL1  (0x002c)
                ynrV3_attr.stManual.stFix.ynr_frame_full_size = 0x0000;
                ynrV3_attr.stManual.stFix.ynr_lbf_weight_thres = 0x000;

                // YNR_2700_GAUSS1_COEFF  (0x0030)
                ynrV3_attr.stManual.stFix.ynr_low_gauss1_coeff[0] = 0x0b;
                ynrV3_attr.stManual.stFix.ynr_low_gauss1_coeff[1] = 0x20;
                ynrV3_attr.stManual.stFix.ynr_low_gauss1_coeff[2] = 0x054;

                // YNR_2700_GAUSS2_COEFF  (0x0034)
                ynrV3_attr.stManual.stFix.ynr_low_gauss2_coeff[0] = 0x10;
                ynrV3_attr.stManual.stFix.ynr_low_gauss2_coeff[1] = 0x20;
                ynrV3_attr.stManual.stFix.ynr_low_gauss2_coeff[2] = 0x040;

                // YNR_2700_DIRECTION_W_0_3  (0x0038 - 0x003c)
                ynrV3_attr.stManual.stFix.ynr_direction_weight[0] = 0x08;
                ynrV3_attr.stManual.stFix.ynr_direction_weight[1] = 0x08;
                ynrV3_attr.stManual.stFix.ynr_direction_weight[2] = 0x08;
                ynrV3_attr.stManual.stFix.ynr_direction_weight[3] = 0x08;
                ynrV3_attr.stManual.stFix.ynr_direction_weight[4] = 0x04;
                ynrV3_attr.stManual.stFix.ynr_direction_weight[5] = 0x04;
                ynrV3_attr.stManual.stFix.ynr_direction_weight[6] = 0x04;
                ynrV3_attr.stManual.stFix.ynr_direction_weight[7] = 0x04;


                // YNR_2700_SGM_DX_0_1 (0x0040 - 0x0060)
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[0] = 0x000;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[1] = 0x040;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[2] = 0x080;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[3] = 0x0c0;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[4] = 0x100;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[5] = 0x140;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[6] = 0x180;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[7] = 0x1c0;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[8] = 0x200;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[9] = 0x240;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[10] = 0x280;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[11] = 0x2c0;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[12] = 0x300;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[13] = 0x340;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[14] = 0x380;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[15] = 0x3c0;
                ynrV3_attr.stManual.stFix.ynr_luma_points_x[16] = 0x400;


                // YNR_2700_LSGM_Y_0_1 (0x0070- 0x0090)
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[0] = 0x04c;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[1] = 0x0a0;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[2] = 0x0d0;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[3] = 0x0e4;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[4] = 0x0e8;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[5] = 0x0e4;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[6] = 0x0d8;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[7] = 0x0cc;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[8] = 0x0c4;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[9] = 0x0c0;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[10] = 0x0bc;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[11] = 0x0b8;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[12] = 0x0b4;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[13] = 0x0a4;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[14] = 0x088;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[15] = 0x054;
                ynrV3_attr.stManual.stFix.ynr_lsgm_y[16] = 0x028;

                // YNR_2700_HSGM_Y_0_1 (0x00a0- 0x00c0)
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[0] = 0x04c;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[1] = 0x0a0;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[2] = 0x0d0;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[3] = 0x0e4;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[4] = 0x0e8;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[5] = 0x0e4;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[6] = 0x0d8;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[7] = 0x0cc;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[8] = 0x0c4;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[9] = 0x0c0;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[10] = 0x0bc;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[11] = 0x0b8;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[12] = 0x0b4;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[13] = 0x0a4;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[14] = 0x088;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[15] = 0x054;
                ynrV3_attr.stManual.stFix.ynr_hsgm_y[16] = 0x028;

                for(int j = 0; j < 17; j++) {
                    // YNR_2700_RNR_STRENGTH03 (0x00d0- 0x00e0)
                    ynrV3_attr.stManual.stFix.ynr_rnr_strength[j] = 0x10;
                }

                ret = rk_aiq_user_api2_aynrV3_SetAttrib(ctx, &ynrV3_attr);
                printf("set ynr v3 attri manual ret:%d \n\n", ret);

                rk_aiq_ynr_attrib_v3_t get_ynrV3_attr;
                get_ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &get_ynrV3_attr);
                printf("get aynr v3 attri ret:%d done:%d \n\n", ret, get_ynrV3_attr.sync.done);
            }
            break;
        case 'f':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_ynr_strength_v3_t ynrV3_strength;
                ynrV3_strength.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ynrV3_strength.strength_enable = true;
                ynrV3_strength.percent = 1.0;
                ret = rk_aiq_user_api2_aynrV3_SetStrength(ctx, &ynrV3_strength);
                printf("Set aynr v3 set streangth ret:%d strength:%f \n\n", ret, ynrV3_strength.percent);

                rk_aiq_ynr_strength_v3_t get_ynrV3_strength;
                get_ynrV3_strength.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_aynrV3_GetStrength(ctx, &get_ynrV3_strength);
                printf("get aynr v3 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_ynrV3_strength.percent, get_ynrV3_strength.sync.done);
            }
            break;
        case 'g':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_ynr_strength_v3_t ynrV3_strength;
                ynrV3_strength.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ynrV3_strength.strength_enable = true;
                ynrV3_strength.percent = 0.0;
                ret = rk_aiq_user_api2_aynrV3_SetStrength(ctx, &ynrV3_strength);
                printf("Set aynr v3 set streangth ret:%d strength:%f \n\n", ret, ynrV3_strength.percent);

                rk_aiq_ynr_strength_v3_t get_ynrV3_strength;
                get_ynrV3_strength.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_aynrV3_GetStrength(ctx, &get_ynrV3_strength);
                printf("get aynr v3 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_ynrV3_strength.percent, get_ynrV3_strength.sync.done);
            }
            break;
        case 'h':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_ynr_strength_v3_t ynrV3_strength;
                ynrV3_strength.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ynrV3_strength.strength_enable = true;
                ynrV3_strength.percent = 0.5;
                ret = rk_aiq_user_api2_aynrV3_SetStrength(ctx, &ynrV3_strength);
                printf("Set aynr v3 set streangth ret:%d strength:%f \n\n", ret, ynrV3_strength.percent);

                rk_aiq_ynr_strength_v3_t get_ynrV3_strength;
                get_ynrV3_strength.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_aynrV3_GetStrength(ctx, &get_ynrV3_strength);
                printf("get aynr v3 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_ynrV3_strength.percent, get_ynrV3_strength.sync.done);
            }
            break;
        case 'i':
            if (CHECK_ISP_HW_V30()) {
                default_ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_aynrV3_SetAttrib(ctx, &default_ynrV3_attr);
                printf("set ynr v3 attri auto default value ret:%d \n\n", ret);

                rk_aiq_ynr_attrib_v3_t get_ynrV3_attr;
                get_ynrV3_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_aynrV3_GetAttrib(ctx, &get_ynrV3_attr);
                printf("get aynr v3 attri ret:%d done:%d \n\n", ret, get_ynrV3_attr.sync.done);
            }
            break;
        default:
            printf("not support test\n\n");
            break;
        }

    } while (key != 'q' && key != 'Q');

    return ret;
}

