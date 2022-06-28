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

static void sample_abayer2dnr_usage()
{
    printf("Usage : \n");
    printf("\t 0) ABAYER2DNR:         get abayer2dnr attri on sync mode.\n");
    printf("\t 1) ABAYER2DNR:         get abayer2dnr strength on sync mode.\n");
    printf("\t 2) ABAYER2DNR:         set abayer2dnr attri auto on sync mode.\n");
    printf("\t 3) ABAYER2DNR:         set abayer2dnr attri manual on sync mode.\n");
    printf("\t 4) ABAYER2DNR:         set abayer2dnr attri reg value on sync mode.\n");
    printf("\t 5) ABAYER2DNR:         set abayer2dnr strength max value 1.0 on sync mode, only on auto mode has effect.\n");
    printf("\t 6) ABAYER2DNR:         set abayer2dnr strength min value 0.0 on sync mode, only on auto mode has effect.\n");
    printf("\t 7) ABAYER2DNR:         set abayer2dnr strength med value 0.5 on sync mode, only on auto mode has effect.\n");
    printf("\t 8) ABAYER2DNR:         set abayer2dnr attri default vaule on sync mode.\n");
    printf("\t a) ABAYER2DNR:         get abayer2dnr attri on async mode.\n");
    printf("\t b) ABAYER2DNR:         get abayer2dnr strength on async mode.\n");
    printf("\t c) ABAYER2DNR:         set abayer2dnr attri auto on async mode.\n");
    printf("\t d) ABAYER2DNR:         set abayer2dnr attri manual on async mode.\n");
    printf("\t e) ABAYER2DNR:         set abayer2dnr attri reg value on async mode.\n");
    printf("\t f) ABAYER2DNR:         set abayer2dnr strength max value 1.0 on async mode, only on auto mode has effect.\n");
    printf("\t g) ABAYER2DNR:         set abayer2dnr strength min value 0.0 on async mode, only on auto mode has effect.\n");
    printf("\t h) ABAYER2DNR:         set abayer2dnr strength med value 0.5 on async mode, only on auto mode has effect.\n");
    printf("\t i) ABAYER2DNR:         set abayer2dnr attri default vaule on async mode.\n");
    printf("\t q) ABAYER2DNR:         press key q or Q to quit.\n");
}

void sample_print_abayer2dnr_info(const void *arg)
{
    printf ("enter ABAYER2DNR modult test!\n");
}


XCamReturn sample_abayer2dnr_module (const void *arg)
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

    rk_aiq_bayer2dnr_attrib_v2_t default_bayer2dnrV2_attr;
    ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &default_bayer2dnrV2_attr);
    printf("get abayer2dnr v2 default auto attri ret:%d \n\n", ret);

    do {
        sample_abayer2dnr_usage ();

        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar ();
        printf ("\n");

        switch (key) {
        case '0':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayer2dnr_attrib_v2_t bayer2dnrV2_attr;
                bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &bayer2dnrV2_attr);
                printf("get abayer2dnr v2 attri ret:%d done:%d \n\n", ret, bayer2dnrV2_attr.sync.done);
            }
            break;
        case '1':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayer2dnr_strength_v2_t bayer2dnrV2Strenght;
                bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetStrength(ctx, &bayer2dnrV2Strenght);
                printf("get abayer2dnr v2 attri ret:%d strength:%f done:%d\n\n",
                       ret, bayer2dnrV2Strenght.percent, bayer2dnrV2Strenght.sync.done);
            }
            break;
        case '2':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayer2dnr_attrib_v2_t bayer2dnrV2_attr;
                bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &bayer2dnrV2_attr);
                bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                bayer2dnrV2_attr.eMode = ABAYER2DNR_OP_MODE_AUTO;
                bayer2dnrV2_attr.stAuto.st2DParams.enable = 1;
                bayer2dnrV2_attr.stAuto.st2DParams.hdrdgain_ctrl_en = 1;

                for(int i = 0; i < RK_BAYER2DNR_V2_MAX_ISO_NUM; i++) {
                    bayer2dnrV2_attr.stAuto.st2DParams.iso[i] = 50 * pow(2, i);

                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[0] = 512;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[1] = 1024;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[2] = 1536;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[3] = 2048;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[4] = 3072;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[5] = 4096;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[6] = 5120;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[7] = 6144;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[8] = 7168;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[9] = 8192;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[10] = 9216;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[11] = 10240;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[12] = 11264;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[13] = 12288;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[14] = 13312;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[15] = 14336;

                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][0] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][1] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][2] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][3] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][4] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][5] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][6] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][7] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][8] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][9] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][10] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][11] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][12] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][13] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][14] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][15] = 90;

                    bayer2dnrV2_attr.stAuto.st2DParams.filter_strength[i] = 1.0;
                    bayer2dnrV2_attr.stAuto.st2DParams.edgesofts[i] = 1.0;
                    bayer2dnrV2_attr.stAuto.st2DParams.ratio[i] = 0.01;
                    bayer2dnrV2_attr.stAuto.st2DParams.weight[i] = 0.7;
                    bayer2dnrV2_attr.stAuto.st2DParams.gauss_guide[i] = 0;
                    bayer2dnrV2_attr.stAuto.st2DParams.pix_diff[i] = 16383;
                    bayer2dnrV2_attr.stAuto.st2DParams.diff_thld[i] = 1024;

                    bayer2dnrV2_attr.stAuto.st2DParams.hdr_dgain_scale_s[i] = 1.0;
                    bayer2dnrV2_attr.stAuto.st2DParams.hdr_dgain_scale_m[i] = 1.0;

                }

                ret = rk_aiq_user_api2_abayer2dnrV2_SetAttrib(ctx, &bayer2dnrV2_attr);
                printf("set abayer2dnr attri auto ret:%d \n\n", ret);

                rk_aiq_bayer2dnr_attrib_v2_t get_bayer2dnrV2_attr;
                get_bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &get_bayer2dnrV2_attr);
                printf("get abayer2dnr v2 attri ret:%d done:%d \n\n", ret, get_bayer2dnrV2_attr.sync.done);
            }
            break;
        case '3':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayer2dnr_attrib_v2_t bayer2dnrV2_attr;
                bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &bayer2dnrV2_attr);
                bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                bayer2dnrV2_attr.eMode = ABAYER2DNR_OP_MODE_MANUAL;
                bayer2dnrV2_attr.stManual.st2DSelect.enable = 1;
                bayer2dnrV2_attr.stManual.st2DSelect.hdrdgain_ctrl_en = 1;

                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[0] = 512;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[1] = 1024;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[2] = 1536;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[3] = 2048;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[4] = 3072;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[5] = 4096;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[6] = 5120;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[7] = 6144;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[8] = 7168;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[9] = 8192;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[10] = 9216;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[11] = 10240;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[12] = 11264;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[13] = 12288;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[14] = 13312;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[15] = 14336;

                bayer2dnrV2_attr.stManual.st2DSelect.sigma[0] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[1] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[2] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[3] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[4] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[5] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[6] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[7] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[8] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[9] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[10] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[11] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[12] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[13] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[14] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[15] = 90;

                bayer2dnrV2_attr.stManual.st2DSelect.filter_strength = 1.0;
                bayer2dnrV2_attr.stManual.st2DSelect.edgesofts = 1.0;
                bayer2dnrV2_attr.stManual.st2DSelect.ratio = 0.01;
                bayer2dnrV2_attr.stManual.st2DSelect.weight = 0.7;
                bayer2dnrV2_attr.stManual.st2DSelect.gauss_guide = 0;
                bayer2dnrV2_attr.stManual.st2DSelect.pix_diff = 16383;
                bayer2dnrV2_attr.stManual.st2DSelect.diff_thld = 1023;

                bayer2dnrV2_attr.stManual.st2DSelect.hdr_dgain_scale_s = 1.0;
                bayer2dnrV2_attr.stManual.st2DSelect.hdr_dgain_scale_m = 1.0;

                ret = rk_aiq_user_api2_abayer2dnrV2_SetAttrib(ctx, &bayer2dnrV2_attr);
                printf("set bayer2dnr v2 attri manual ret:%d \n\n", ret);

                rk_aiq_bayer2dnr_attrib_v2_t get_bayer2dnrV2_attr;
                get_bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &get_bayer2dnrV2_attr);
                printf("get abayer2dnr v2 attri ret:%d done:%d \n\n", ret, get_bayer2dnrV2_attr.sync.done);
            }

            break;
        case '4':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayer2dnr_attrib_v2_t bayer2dnrV2_attr;
                bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &bayer2dnrV2_attr);
                bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                bayer2dnrV2_attr.eMode = ABAYER2DNR_OP_MODE_REG_MANUAL;

                //ISP_BAYNR_3A00_CTRL
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_lg2_mode = 0;
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_gauss_en = 1;
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_log_bypass = 0;
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_en = 1;

                // ISP_BAYNR_3A00_DGAIN0-2
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_dgain[0] = 0x4000;
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_dgain[1] = 0x4000;
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_dgain[2] = 0x4000;

                // ISP_BAYNR_3A00_PIXDIFF
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_pix_diff = 0x3fff;

                // ISP_BAYNR_3A00_THLD
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_diff_thld = 0x3fff;
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_softthld = 0x00a;

                // ISP_BAYNR_3A00_W1_STRENG
                bayer2dnrV2_attr.stManual.st2Dfix.bltflt_streng = 0x0a3;
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_reg_w1 = 0x3ff;

                // ISP_BAYNR_3A00_SIGMAX0-15
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[0] = 0x1010;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[1] = 0x1210;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[2] = 0x1410;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[3] = 0x1610;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[4] = 0x1810;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[5] = 0x1c10;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[6] = 0x2010;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[7] = 0x2410;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[8] = 0x2810;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[9] = 0x2c10;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[10] = 0x3010;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[11] = 0x3410;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[12] = 0x3810;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[13] = 0x3a10;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[14] = 0x3c10;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[15] = 0x4010;

                // ISP_BAYNR_3A00_SIGMAY0-15
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[0] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[1] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[2] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[3] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[4] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[5] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[6] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[7] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[8] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[9] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[10] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[11] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[12] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[13] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[14] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[15] = 0x0300;

                // ISP_BAYNR_3A00_WRIT_D
                bayer2dnrV2_attr.stManual.st2Dfix.weit_d[0] = 0x178;
                bayer2dnrV2_attr.stManual.st2Dfix.weit_d[1] = 0x26d;
                bayer2dnrV2_attr.stManual.st2Dfix.weit_d[2] = 0x31d;


                ret = rk_aiq_user_api2_abayer2dnrV2_SetAttrib(ctx, &bayer2dnrV2_attr);
                printf("set bayer2dnr v2 attri manual ret:%d \n\n", ret);

                rk_aiq_bayer2dnr_attrib_v2_t get_bayer2dnrV2_attr;
                get_bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &get_bayer2dnrV2_attr);
                printf("get abayer2dnr v2 attri ret:%d done:%d \n\n", ret, get_bayer2dnrV2_attr.sync.done);
            }
            break;
        case '5':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayer2dnr_strength_v2_t bayer2dnrV2Strenght;
                bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                bayer2dnrV2Strenght.strength_enable = true;
                bayer2dnrV2Strenght.percent = 1.0;
                ret = rk_aiq_user_api2_abayer2dnrV2_SetStrength(ctx, &bayer2dnrV2Strenght);
                printf("Set abayer2dnr v2 set strength ret:%d strength:%f \n\n", ret, bayer2dnrV2Strenght.percent);

                rk_aiq_bayer2dnr_strength_v2_t get_bayer2dnrV2Strenght;
                get_bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetStrength(ctx, &get_bayer2dnrV2Strenght);
                printf("get abayer2dnr v2 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_bayer2dnrV2Strenght.percent, get_bayer2dnrV2Strenght.sync.done);
            }
            break;
        case '6':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayer2dnr_strength_v2_t bayer2dnrV2Strenght;
                bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                bayer2dnrV2Strenght.strength_enable = true;
                bayer2dnrV2Strenght.percent = 0.0;
                ret = rk_aiq_user_api2_abayer2dnrV2_SetStrength(ctx, &bayer2dnrV2Strenght);
                printf("Set abayer2dnr v2 set strength ret:%d strength:%f \n\n", ret, bayer2dnrV2Strenght.percent);

                rk_aiq_bayer2dnr_strength_v2_t get_bayer2dnrV2Strenght;
                get_bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetStrength(ctx, &get_bayer2dnrV2Strenght);
                printf("get abayer2dnr v2 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_bayer2dnrV2Strenght.percent, get_bayer2dnrV2Strenght.sync.done);
            }
            break;
        case '7':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayer2dnr_strength_v2_t bayer2dnrV2Strenght;
                bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                bayer2dnrV2Strenght.strength_enable = true;
                bayer2dnrV2Strenght.percent = 0.5;
                ret = rk_aiq_user_api2_abayer2dnrV2_SetStrength(ctx, &bayer2dnrV2Strenght);
                printf("Set abayer2dnr v2 set strength ret:%d strength:%f \n\n", ret, bayer2dnrV2Strenght.percent);

                rk_aiq_bayer2dnr_strength_v2_t get_bayer2dnrV2Strenght;
                get_bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetStrength(ctx, &get_bayer2dnrV2Strenght);
                printf("get abayer2dnr v2 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_bayer2dnrV2Strenght.percent, get_bayer2dnrV2Strenght.sync.done);
            }
            break;
        case '8':
            if (CHECK_ISP_HW_V30()) {
                default_bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_SetAttrib(ctx, &default_bayer2dnrV2_attr);
                printf("set bayer2dnr v2 default auto attri ret:%d \n\n", ret);

                rk_aiq_bayer2dnr_attrib_v2_t get_bayer2dnrV2_attr;
                get_bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &get_bayer2dnrV2_attr);
                printf("get abayer2dnr v2 attri ret:%d done:%d \n\n", ret, get_bayer2dnrV2_attr.sync.done);
            }
            break;
        case 'a':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayer2dnr_attrib_v2_t bayer2dnrV2_attr;
                bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &bayer2dnrV2_attr);
                printf("get abayer2dnr v2 attri ret:%d done:%d \n\n", ret, bayer2dnrV2_attr.sync.done);
            }
            break;
        case 'b':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayer2dnr_strength_v2_t bayer2dnrV2Strenght;
                bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetStrength(ctx, &bayer2dnrV2Strenght);
                printf("get abayer2dnr v2 attri ret:%d strength:%f done:%d\n\n",
                       ret, bayer2dnrV2Strenght.percent, bayer2dnrV2Strenght.sync.done);
            }
            break;
        case 'c':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayer2dnr_attrib_v2_t bayer2dnrV2_attr;
                bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &bayer2dnrV2_attr);
                bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                bayer2dnrV2_attr.eMode = ABAYER2DNR_OP_MODE_AUTO;
                bayer2dnrV2_attr.stAuto.st2DParams.enable = 1;
                bayer2dnrV2_attr.stAuto.st2DParams.hdrdgain_ctrl_en = 1;
                for(int i = 0; i < RK_BAYER2DNR_V2_MAX_ISO_NUM; i++) {
                    bayer2dnrV2_attr.stAuto.st2DParams.iso[i] = 50 * pow(2, i);

                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[0] = 512;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[1] = 1024;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[2] = 1536;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[3] = 2048;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[4] = 3072;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[5] = 4096;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[6] = 5120;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[7] = 6144;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[8] = 7168;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[9] = 8192;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[10] = 9216;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[11] = 10240;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[12] = 11264;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[13] = 12288;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[14] = 13312;
                    bayer2dnrV2_attr.stAuto.st2DParams.lumapoint[15] = 14336;

                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][0] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][1] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][2] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][3] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][4] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][5] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][6] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][7] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][8] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][9] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][10] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][11] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][12] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][13] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][14] = 90;
                    bayer2dnrV2_attr.stAuto.st2DParams.sigma[i][15] = 90;

                    bayer2dnrV2_attr.stAuto.st2DParams.filter_strength[i] = 1.0;
                    bayer2dnrV2_attr.stAuto.st2DParams.edgesofts[i] = 1.0;
                    bayer2dnrV2_attr.stAuto.st2DParams.ratio[i] = 0.01;
                    bayer2dnrV2_attr.stAuto.st2DParams.weight[i] = 0.7;
                    bayer2dnrV2_attr.stAuto.st2DParams.gauss_guide[i] = 0;
                    bayer2dnrV2_attr.stAuto.st2DParams.pix_diff[i] = 16383;
                    bayer2dnrV2_attr.stAuto.st2DParams.diff_thld[i] = 1024;

                    bayer2dnrV2_attr.stAuto.st2DParams.hdr_dgain_scale_s[i] = 1.0;
                    bayer2dnrV2_attr.stAuto.st2DParams.hdr_dgain_scale_m[i] = 1.0;

                }

                ret = rk_aiq_user_api2_abayer2dnrV2_SetAttrib(ctx, &bayer2dnrV2_attr);
                printf("set abayer2dnr attri auto ret:%d \n\n", ret);

                rk_aiq_bayer2dnr_attrib_v2_t get_bayer2dnrV2_attr;
                get_bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &get_bayer2dnrV2_attr);
                printf("get abayer2dnr v2 attri ret:%d done:%d \n\n", ret, get_bayer2dnrV2_attr.sync.done);
            }
            break;
        case 'd':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayer2dnr_attrib_v2_t bayer2dnrV2_attr;
                bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &bayer2dnrV2_attr);
                bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                bayer2dnrV2_attr.eMode = ABAYER2DNR_OP_MODE_MANUAL;
                bayer2dnrV2_attr.stManual.st2DSelect.enable = 1;
                bayer2dnrV2_attr.stManual.st2DSelect.hdrdgain_ctrl_en = 1;

                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[0] = 512;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[1] = 1024;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[2] = 1536;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[3] = 2048;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[4] = 3072;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[5] = 4096;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[6] = 5120;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[7] = 6144;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[8] = 7168;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[9] = 8192;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[10] = 9216;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[11] = 10240;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[12] = 11264;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[13] = 12288;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[14] = 13312;
                bayer2dnrV2_attr.stManual.st2DSelect.lumapoint[15] = 14336;

                bayer2dnrV2_attr.stManual.st2DSelect.sigma[0] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[1] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[2] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[3] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[4] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[5] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[6] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[7] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[8] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[9] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[10] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[11] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[12] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[13] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[14] = 90;
                bayer2dnrV2_attr.stManual.st2DSelect.sigma[15] = 90;

                bayer2dnrV2_attr.stManual.st2DSelect.filter_strength = 1.0;
                bayer2dnrV2_attr.stManual.st2DSelect.edgesofts = 1.0;
                bayer2dnrV2_attr.stManual.st2DSelect.ratio = 0.01;
                bayer2dnrV2_attr.stManual.st2DSelect.weight = 0.7;
                bayer2dnrV2_attr.stManual.st2DSelect.gauss_guide = 0;
                bayer2dnrV2_attr.stManual.st2DSelect.pix_diff = 16383;
                bayer2dnrV2_attr.stManual.st2DSelect.diff_thld = 1023;

                bayer2dnrV2_attr.stManual.st2DSelect.hdr_dgain_scale_s = 1.0;
                bayer2dnrV2_attr.stManual.st2DSelect.hdr_dgain_scale_m = 1.0;

                ret = rk_aiq_user_api2_abayer2dnrV2_SetAttrib(ctx, &bayer2dnrV2_attr);
                printf("set bayer2dnr v2 attri manual ret:%d \n\n", ret);

                rk_aiq_bayer2dnr_attrib_v2_t get_bayer2dnrV2_attr;
                get_bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &get_bayer2dnrV2_attr);
                printf("get abayer2dnr v2 attri ret:%d done:%d \n\n", ret, get_bayer2dnrV2_attr.sync.done);
            }

            break;
        case 'e':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayer2dnr_attrib_v2_t bayer2dnrV2_attr;
                bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &bayer2dnrV2_attr);
                bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                bayer2dnrV2_attr.eMode = ABAYER2DNR_OP_MODE_REG_MANUAL;

                //ISP_BAYNR_3A00_CTRL
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_lg2_mode = 0;
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_gauss_en = 1;
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_log_bypass = 0;
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_en = 1;

                // ISP_BAYNR_3A00_DGAIN0-2
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_dgain[0] = 0x4000;
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_dgain[1] = 0x4000;
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_dgain[2] = 0x4000;

                // ISP_BAYNR_3A00_PIXDIFF
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_pix_diff = 0x3fff;

                // ISP_BAYNR_3A00_THLD
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_diff_thld = 0x3fff;
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_softthld = 0x00a;

                // ISP_BAYNR_3A00_W1_STRENG
                bayer2dnrV2_attr.stManual.st2Dfix.bltflt_streng = 0x0a3;
                bayer2dnrV2_attr.stManual.st2Dfix.baynr_reg_w1 = 0x3ff;

                // ISP_BAYNR_3A00_SIGMAX0-15
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[0] = 0x1010;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[1] = 0x1210;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[2] = 0x1410;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[3] = 0x1610;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[4] = 0x1810;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[5] = 0x1c10;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[6] = 0x2010;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[7] = 0x2410;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[8] = 0x2810;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[9] = 0x2c10;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[10] = 0x3010;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[11] = 0x3410;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[12] = 0x3810;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[13] = 0x3a10;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[14] = 0x3c10;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_x[15] = 0x4010;

                // ISP_BAYNR_3A00_SIGMAY0-15
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[0] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[1] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[2] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[3] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[4] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[5] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[6] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[7] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[8] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[9] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[10] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[11] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[12] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[13] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[14] = 0x0300;
                bayer2dnrV2_attr.stManual.st2Dfix.sigma_y[15] = 0x0300;

                // ISP_BAYNR_3A00_WRIT_D
                bayer2dnrV2_attr.stManual.st2Dfix.weit_d[0] = 0x178;
                bayer2dnrV2_attr.stManual.st2Dfix.weit_d[1] = 0x26d;
                bayer2dnrV2_attr.stManual.st2Dfix.weit_d[2] = 0x31d;


                ret = rk_aiq_user_api2_abayer2dnrV2_SetAttrib(ctx, &bayer2dnrV2_attr);
                printf("set bayer2dnr v2 attri manual ret:%d \n\n", ret);

                rk_aiq_bayer2dnr_attrib_v2_t get_bayer2dnrV2_attr;
                get_bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &get_bayer2dnrV2_attr);
                printf("get abayer2dnr v2 attri ret:%d done:%d \n\n", ret, get_bayer2dnrV2_attr.sync.done);
            }
            break;
        case 'f':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayer2dnr_strength_v2_t bayer2dnrV2Strenght;
                bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                bayer2dnrV2Strenght.strength_enable = true;
                bayer2dnrV2Strenght.percent = 1.0;
                ret = rk_aiq_user_api2_abayer2dnrV2_SetStrength(ctx, &bayer2dnrV2Strenght);
                printf("Set abayer2dnr v2 set strength ret:%d strength:%f \n\n", ret, bayer2dnrV2Strenght.percent);

                rk_aiq_bayer2dnr_strength_v2_t get_bayer2dnrV2Strenght;
                get_bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetStrength(ctx, &get_bayer2dnrV2Strenght);
                printf("get abayer2dnr v2 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_bayer2dnrV2Strenght.percent, get_bayer2dnrV2Strenght.sync.done);
            }
            break;
        case 'g':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayer2dnr_strength_v2_t bayer2dnrV2Strenght;
                bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                bayer2dnrV2Strenght.strength_enable = true;
                bayer2dnrV2Strenght.percent = 0.0;
                ret = rk_aiq_user_api2_abayer2dnrV2_SetStrength(ctx, &bayer2dnrV2Strenght);
                printf("Set abayer2dnr v2 set strength ret:%d strength:%f \n\n", ret, bayer2dnrV2Strenght.percent);

                rk_aiq_bayer2dnr_strength_v2_t get_bayer2dnrV2Strenght;
                get_bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetStrength(ctx, &get_bayer2dnrV2Strenght);
                printf("get abayer2dnr v2 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_bayer2dnrV2Strenght.percent, get_bayer2dnrV2Strenght.sync.done);
            }
            break;
        case 'h':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayer2dnr_strength_v2_t bayer2dnrV2Strenght;
                bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                bayer2dnrV2Strenght.strength_enable = true;
                bayer2dnrV2Strenght.percent = 0.5;
                ret = rk_aiq_user_api2_abayer2dnrV2_SetStrength(ctx, &bayer2dnrV2Strenght);
                printf("Set abayer2dnr v2 set strength ret:%d strength:%f \n\n", ret, bayer2dnrV2Strenght.percent);

                rk_aiq_bayer2dnr_strength_v2_t get_bayer2dnrV2Strenght;
                get_bayer2dnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetStrength(ctx, &get_bayer2dnrV2Strenght);
                printf("get abayer2dnr v2 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_bayer2dnrV2Strenght.percent, get_bayer2dnrV2Strenght.sync.done);
            }
            break;
        case 'i':
            if (CHECK_ISP_HW_V30()) {
                default_bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_SetAttrib(ctx, &default_bayer2dnrV2_attr);
                printf("set bayer2dnr v2 default auto attri ret:%d \n\n", ret);

                rk_aiq_bayer2dnr_attrib_v2_t get_bayer2dnrV2_attr;
                get_bayer2dnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayer2dnrV2_GetAttrib(ctx, &get_bayer2dnrV2_attr);
                printf("get abayer2dnr v2 attri ret:%d done:%d \n\n", ret, get_bayer2dnrV2_attr.sync.done);
            }
            break;
        default:
            printf("not support test\n\n");
            break;
        }

    } while (key != 'q' && key != 'Q');

    return ret;
}

