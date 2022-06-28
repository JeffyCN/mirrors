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

static void sample_abayertnr_usage()
{
    printf("Usage : \n");
    printf("\t 0) ABAYERTNR:         get abayertnr attri on sync mode.\n");
    printf("\t 1) ABAYERTNR:         get abayertnr strength on sync mode.\n");
    printf("\t 2) ABAYERTNR:         set abayertnr attri auto on sync mode.\n");
    printf("\t 3) ABAYERTNR:         set abayertnr attri manual on sync mode.\n");
    printf("\t 4) ABAYERTNR:         set abayertnr attri reg value on sync mode.\n");
    printf("\t 5) ABAYERTNR:         set abayertnr strength max value 1.0 on sync mode, only on auto mode has effect.\n");
    printf("\t 6) ABAYERTNR:         set abayertnr strength min value 0.0 on sync mode, only on auto mode has effect.\n");
    printf("\t 7) ABAYERTNR:         set abayertnr strength med value 0.5 on sync mode, only on auto mode has effect.\n");
    printf("\t 8) ABAYERTNR:         set abayertnr attri to default value on sync mode.\n");
    printf("\t a) ABAYERTNR:         get abayertnr attri on async mode.\n");
    printf("\t b) ABAYERTNR:         get abayertnr strength on async mode.\n");
    printf("\t c) ABAYERTNR:         set abayertnr attri auto on async mode.\n");
    printf("\t d) ABAYERTNR:         set abayertnr attri manual on async mode.\n");
    printf("\t e) ABAYERTNR:         set abayertnr attri reg value on async mode.\n");
    printf("\t f) ABAYERTNR:         set abayertnr strength max value 1.0 on async mode, only on auto mode has effect.\n");
    printf("\t g) ABAYERTNR:         set abayertnr strength min value 0.0 on async mode, only on auto mode has effect.\n");
    printf("\t h) ABAYERTNR:         set abayertnr strength med value 0.5 on async mode, only on auto mode has effect.\n");
    printf("\t i) ABAYERTNR:         set abayertnr attri to default value on async mode.\n");
    printf("\t q) ABAYERTNR:         press key q or Q to quit.\n");



}

void sample_print_abayertnr_info(const void *arg)
{
    printf ("enter ABAYERTNR modult test!\n");
}


XCamReturn sample_abayertnr_module (const void *arg)
{
    int key = -1;
    CLEAR();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    const demo_context_t *demo_ctx = (demo_context_t *)arg;
    const rk_aiq_sys_ctx_t* ctx;

    if (demo_ctx->camGroup) {
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

    rk_aiq_bayertnr_attrib_v2_t default_bayertnrV2_attr;
    ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &default_bayertnrV2_attr);
    printf("get abayertnr v2 default attri ret:%d \n\n", ret);

    do {
        sample_abayertnr_usage ();

        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar ();
        printf ("\n");

        switch (key) {
        case '0':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayertnr_attrib_v2_t bayertnrV2_attr;
                bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &bayertnrV2_attr);
                printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, bayertnrV2_attr.sync.done);
            }
            break;
        case '1':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
                bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetStrength(ctx, &bayertnrV2Strenght);
                printf("get abayertnr v2 attri ret:%d strength:%f done:%d\n\n",
                       ret, bayertnrV2Strenght.percent, bayertnrV2Strenght.sync.done);
            }
            break;
        case '2':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayertnr_attrib_v2_t bayertnrV2_attr;
                bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &bayertnrV2_attr);
                bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                bayertnrV2_attr.eMode = ABAYERTNRV2_OP_MODE_AUTO;
                bayertnrV2_attr.stAuto.st3DParams.enable = 1;
                for(int i = 0; i < RK_BAYERNR_V2_MAX_ISO_NUM; i++) {
                    bayertnrV2_attr.stAuto.st3DParams.iso[i] = 50 * pow(2, i);

                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[0] = 512;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[1] = 1024;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[2] = 1536;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[3] = 2048;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[4] = 3072;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[5] = 4096;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[6] = 5120;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[7] = 6144;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[8] = 7168;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[9] = 8192;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[10] = 9216;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[11] = 10240;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[12] = 11264;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[13] = 12288;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[14] = 13312;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[15] = 14336;

                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][0] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][1] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][2] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][3] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][4] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][5] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][6] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][7] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][8] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][9] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][10] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][11] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][12] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][13] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][14] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][15] = 90;

                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[0] = 512;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[1] = 1024;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[2] = 1536;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[3] = 2048;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[4] = 3072;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[5] = 4096;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[6] = 5120;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[7] = 6144;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[8] = 7168;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[9] = 8192;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[10] = 9216;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[11] = 10240;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[12] = 11264;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[13] = 12288;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[14] = 13312;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[15] = 14336;


                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][0] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][1] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][2] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][3] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][4] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][5] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][6] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][7] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][8] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][9] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][10] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][11] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][12] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][13] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][14] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][15] = 90;

                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][0] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][1] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][2] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][3] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][4] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][5] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][6] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][7] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][8] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][9] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][10] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][11] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][12] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][13] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][14] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][15] = 90;

                    bayertnrV2_attr.stAuto.st3DParams.thumbds[i] = 8;
                    bayertnrV2_attr.stAuto.st3DParams.lo_enable[i] = 1;
                    bayertnrV2_attr.stAuto.st3DParams.hi_enable[i] = 1;
                    bayertnrV2_attr.stAuto.st3DParams.lo_med_en[i] = 1;
                    bayertnrV2_attr.stAuto.st3DParams.lo_gsbay_en[i] = 1;
                    bayertnrV2_attr.stAuto.st3DParams.lo_gslum_en[i] = 1;
                    bayertnrV2_attr.stAuto.st3DParams.hi_med_en[i] = 1;
                    bayertnrV2_attr.stAuto.st3DParams.hi_gslum_en[i] = 1;
                    bayertnrV2_attr.stAuto.st3DParams.global_pk_en[i] = 0;
                    bayertnrV2_attr.stAuto.st3DParams.global_pksq[i] = 1024;

                    bayertnrV2_attr.stAuto.st3DParams.lo_filter_strength[i] = 1.0;
                    bayertnrV2_attr.stAuto.st3DParams.hi_filter_strength[i] = 1.0;

                    bayertnrV2_attr.stAuto.st3DParams.soft_threshold_ratio[i] = 0.0;
                    bayertnrV2_attr.stAuto.st3DParams.hi_wgt_comp[i] = 0.16;
                    bayertnrV2_attr.stAuto.st3DParams.clipwgt[i] = 0.03215;
                    bayertnrV2_attr.stAuto.st3DParams.hidif_th[i] = 32767;

                }

                ret = rk_aiq_user_api2_abayertnrV2_SetAttrib(ctx, &bayertnrV2_attr);
                printf("set abayertnr attri auto ret:%d \n\n", ret);

                rk_aiq_bayertnr_attrib_v2_t get_bayertnrV2_attr;
                get_bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &get_bayertnrV2_attr);
                printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, get_bayertnrV2_attr.sync.done);
            }
            break;
        case '3':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayertnr_attrib_v2_t bayertnrV2_attr;
                bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &bayertnrV2_attr);
                bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                bayertnrV2_attr.eMode = ABAYERTNRV2_OP_MODE_MANUAL;
                bayertnrV2_attr.stManual.st3DSelect.enable = 1;


                bayertnrV2_attr.stManual.st3DSelect.lumapoint[0] = 512;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[1] = 1024;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[2] = 1536;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[3] = 2048;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[4] = 3072;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[5] = 4096;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[6] = 5120;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[7] = 6144;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[8] = 7168;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[9] = 8192;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[10] = 9216;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[11] = 10240;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[12] = 11264;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[13] = 12288;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[14] = 13312;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[15] = 14336;

                bayertnrV2_attr.stManual.st3DSelect.sigma[0] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[1] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[2] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[3] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[4] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[5] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[6] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[7] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[8] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[9] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[10] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[11] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[12] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[13] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[14] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[15] = 90;

                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[0] = 512;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[1] = 1024;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[2] = 1536;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[3] = 2048;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[4] = 3072;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[5] = 4096;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[6] = 5120;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[7] = 6144;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[8] = 7168;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[9] = 8192;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[10] = 9216;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[11] = 10240;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[12] = 11264;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[13] = 12288;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[14] = 13312;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[15] = 14336;


                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[0] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[1] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[2] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[3] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[4] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[5] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[6] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[7] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[8] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[9] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[10] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[11] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[12] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[13] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[14] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[15] = 32;

                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[0] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[1] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[2] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[3] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[4] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[5] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[6] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[7] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[8] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[9] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[10] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[11] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[12] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[13] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[14] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[15] = 256;

                bayertnrV2_attr.stManual.st3DSelect.thumbds = 8;
                bayertnrV2_attr.stManual.st3DSelect.lo_enable = 1;
                bayertnrV2_attr.stManual.st3DSelect.hi_enable = 1;
                bayertnrV2_attr.stManual.st3DSelect.lo_med_en = 1;
                bayertnrV2_attr.stManual.st3DSelect.lo_gsbay_en = 1;
                bayertnrV2_attr.stManual.st3DSelect.lo_gslum_en = 1;
                bayertnrV2_attr.stManual.st3DSelect.hi_med_en = 1;
                bayertnrV2_attr.stManual.st3DSelect.hi_gslum_en = 1;
                bayertnrV2_attr.stManual.st3DSelect.global_pk_en = 0;
                bayertnrV2_attr.stManual.st3DSelect.global_pksq = 1024;

                bayertnrV2_attr.stManual.st3DSelect.lo_filter_strength = 1.0;
                bayertnrV2_attr.stManual.st3DSelect.hi_filter_strength = 1.0;

                bayertnrV2_attr.stManual.st3DSelect.soft_threshold_ratio = 0.0;
                bayertnrV2_attr.stManual.st3DSelect.hi_wgt_comp = 0.16;
                bayertnrV2_attr.stManual.st3DSelect.clipwgt = 0.03215;
                bayertnrV2_attr.stManual.st3DSelect.hidif_th = 32767;


                ret = rk_aiq_user_api2_abayertnrV2_SetAttrib(ctx, &bayertnrV2_attr);
                printf("set bayertnr v2 attri manual ret:%d \n\n", ret);

                rk_aiq_bayertnr_attrib_v2_t get_bayertnrV2_attr;
                get_bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &get_bayertnrV2_attr);
                printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, get_bayertnrV2_attr.sync.done);
            }

            break;
        case '4':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayertnr_attrib_v2_t bayertnrV2_attr;
                bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &bayertnrV2_attr);
                bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                bayertnrV2_attr.eMode = ABAYERTNRV2_OP_MODE_REG_MANUAL;

                // BAY3D_BAY3D_CTRL 0x2c00
                bayertnrV2_attr.stManual.st3DFix.bay3d_exp_sel = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_soft_st = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_soft_mode = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_bwsaving_en = 0;

                bayertnrV2_attr.stManual.st3DFix.bay3d_loswitch_protect = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_glbpk_en = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_logaus3_bypass_en = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_logaus5_bypass_en = 0;

                bayertnrV2_attr.stManual.st3DFix.bay3d_lomed_bypass_en = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_hichnsplit_en = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_hiabs_pssel = 1;
                bayertnrV2_attr.stManual.st3DFix.bay3d_higaus_bypass_en = 0;

                bayertnrV2_attr.stManual.st3DFix.bay3d_himed_bypass_en = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_lobypass_en = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_hibypass_en = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_bypass_en = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_en_i = 1;

                // BAY3D_BAY3D_KALRATIO 0x2c04
                bayertnrV2_attr.stManual.st3DFix.bay3d_softwgt = 0x100;
                bayertnrV2_attr.stManual.st3DFix.bay3d_hidif_th = 0xffff;

                // BAY3D_BAY3D_GLBPK2 0x2c08
                bayertnrV2_attr.stManual.st3DFix.bay3d_glbpk2 = 0x0000800;

                // BAY3D_BAY3D_WGTLMT 0x2c10
                bayertnrV2_attr.stManual.st3DFix.bay3d_wgtlmt = 0x380;
                bayertnrV2_attr.stManual.st3DFix.bay3d_wgtratio = 0x000;

                // BAY3D_BAY3D_SIG_X0  0x2c14 - 0x2c30
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[0] = 0x200;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[1] = 0x400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[2] = 0x600;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[3] = 0x800;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[4] = 0xc00;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[5] = 0x1000;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[6] = 0x1400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[7] = 0x1800;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[8] = 0x1c00;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[9] = 0x2000;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[10] = 0x2400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[11] = 0x2800;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[12] = 0x2c00;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[13] = 0x3000;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[14] = 0x3400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[15] = 0x3800;

                // BAY3D_BAY3D_SIG0_Y0 0x2c34 - 0x2c50
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[0] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[1] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[2] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[3] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[4] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[5] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[6] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[7] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[8] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[9] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[10] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[11] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[12] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[13] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[14] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[15] = 0x0300;

                // BAY3D_BAY3D_SIG_X0  0x2c54 - 0x2c70
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[0] = 0x200;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[1] = 0x400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[2] = 0x600;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[3] = 0x800;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[4] = 0xc00;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[5] = 0x1000;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[6] = 0x1400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[7] = 0x1800;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[8] = 0x1c00;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[9] = 0x2000;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[10] = 0x2400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[11] = 0x2800;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[12] = 0x2c00;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[13] = 0x3000;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[14] = 0x3400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[15] = 0x3800;

                // ISP_BAYNR_3A00_SIGMAY0-15
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[0] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[1] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[2] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[3] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[4] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[5] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[6] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[7] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[8] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[9] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[10] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[11] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[12] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[13] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[14] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[15] = 0x0400;

                // ISP_BAYNR_3A00_SIGMAY0-15
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[0] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[1] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[2] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[3] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[4] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[5] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[6] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[7] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[8] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[9] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[10] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[11] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[12] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[13] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[14] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[15] = 0x080;


                //bayertnrV2_attr.stManual.st3DFix.ro_sum_lodif = 0x0;
                //bayertnrV2_attr.stManual.st3DFix.ro_sum_hidif0 = 0x0;

                //bayertnrV2_attr.stManual.st3DFix.sw_bay3dmi_st_linemode = 0x01;
                //bayertnrV2_attr.stManual.st3DFix.sw_bay3d_mi2cur_linecnt = 0x4e;


                ret = rk_aiq_user_api2_abayertnrV2_SetAttrib(ctx, &bayertnrV2_attr);
                printf("set bayertnr v2 attri manual ret:%d \n\n", ret);

                rk_aiq_bayertnr_attrib_v2_t get_bayertnrV2_attr;
                get_bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &get_bayertnrV2_attr);
                printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, get_bayertnrV2_attr.sync.done);
            }
            break;
        case '5':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
                bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                bayertnrV2Strenght.strength_enable = true;
                bayertnrV2Strenght.percent = 1.0;
                ret = rk_aiq_user_api2_abayertnrV2_SetStrength(ctx, &bayertnrV2Strenght);
                printf("Set abayertnr v2 set streangth ret:%d strength:%f \n\n", ret, bayertnrV2Strenght.percent);

                rk_aiq_bayertnr_strength_v2_t get_bayertnrV2Strenght;
                get_bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetStrength(ctx, &get_bayertnrV2Strenght);
                printf("get abayertnr v2 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_bayertnrV2Strenght.percent, get_bayertnrV2Strenght.sync.done);
            }
            break;
        case '6':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
                bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                bayertnrV2Strenght.strength_enable = true;
                bayertnrV2Strenght.percent = 0.0;
                ret = rk_aiq_user_api2_abayertnrV2_SetStrength(ctx, &bayertnrV2Strenght);
                printf("Set abayertnr v2 set streangth ret:%d strength:%f \n\n", ret, bayertnrV2Strenght.percent);

                rk_aiq_bayertnr_strength_v2_t get_bayertnrV2Strenght;
                get_bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetStrength(ctx, &get_bayertnrV2Strenght);
                printf("get abayertnr v2 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_bayertnrV2Strenght.percent, get_bayertnrV2Strenght.sync.done);
            }
            break;
        case '7':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
                bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                bayertnrV2Strenght.strength_enable = true;
                bayertnrV2Strenght.percent = 0.5;
                ret = rk_aiq_user_api2_abayertnrV2_SetStrength(ctx, &bayertnrV2Strenght);
                printf("Set abayertnr v2 set streangth ret:%d strength:%f \n\n", ret, bayertnrV2Strenght.percent);

                rk_aiq_bayertnr_strength_v2_t get_bayertnrV2Strenght;
                get_bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetStrength(ctx, &get_bayertnrV2Strenght);
                printf("get abayertnr v2 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_bayertnrV2Strenght.percent, get_bayertnrV2Strenght.sync.done);
            }
            break;
        case '8':
            if (CHECK_ISP_HW_V30()) {
                default_bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayertnrV2_SetAttrib(ctx, &default_bayertnrV2_attr);
                printf("set bayertnr v2 default attri ret:%d \n\n", ret);

                rk_aiq_bayertnr_attrib_v2_t get_bayertnrV2_attr;
                get_bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &get_bayertnrV2_attr);
                printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, get_bayertnrV2_attr.sync.done);
            }
            break;
        case 'a':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayertnr_attrib_v2_t bayertnrV2_attr;
                bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &bayertnrV2_attr);
                printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, bayertnrV2_attr.sync.done);
            }
            break;
        case 'b':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
                bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetStrength(ctx, &bayertnrV2Strenght);
                printf("get abayertnr v2 attri ret:%d strength:%f done:%d\n\n",
                       ret, bayertnrV2Strenght.percent, bayertnrV2Strenght.sync.done);
            }
            break;
        case 'c':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayertnr_attrib_v2_t bayertnrV2_attr;
                bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &bayertnrV2_attr);
                bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                bayertnrV2_attr.eMode = ABAYERTNRV2_OP_MODE_AUTO;
                bayertnrV2_attr.stAuto.st3DParams.enable = 1;
                for(int i = 0; i < RK_BAYERNR_V2_MAX_ISO_NUM; i++) {
                    bayertnrV2_attr.stAuto.st3DParams.iso[i] = 50 * pow(2, i);

                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[0] = 512;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[1] = 1024;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[2] = 1536;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[3] = 2048;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[4] = 3072;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[5] = 4096;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[6] = 5120;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[7] = 6144;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[8] = 7168;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[9] = 8192;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[10] = 9216;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[11] = 10240;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[12] = 11264;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[13] = 12288;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[14] = 13312;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint[15] = 14336;

                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][0] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][1] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][2] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][3] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][4] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][5] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][6] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][7] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][8] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][9] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][10] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][11] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][12] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][13] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][14] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.sigma[i][15] = 90;

                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[0] = 512;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[1] = 1024;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[2] = 1536;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[3] = 2048;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[4] = 3072;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[5] = 4096;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[6] = 5120;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[7] = 6144;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[8] = 7168;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[9] = 8192;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[10] = 9216;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[11] = 10240;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[12] = 11264;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[13] = 12288;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[14] = 13312;
                    bayertnrV2_attr.stAuto.st3DParams.lumapoint2[15] = 14336;


                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][0] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][1] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][2] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][3] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][4] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][5] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][6] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][7] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][8] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][9] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][10] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][11] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][12] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][13] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][14] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.lo_sigma[i][15] = 90;

                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][0] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][1] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][2] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][3] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][4] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][5] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][6] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][7] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][8] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][9] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][10] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][11] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][12] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][13] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][14] = 90;
                    bayertnrV2_attr.stAuto.st3DParams.hi_sigma[i][15] = 90;

                    bayertnrV2_attr.stAuto.st3DParams.thumbds[i] = 8;
                    bayertnrV2_attr.stAuto.st3DParams.lo_enable[i] = 1;
                    bayertnrV2_attr.stAuto.st3DParams.hi_enable[i] = 1;
                    bayertnrV2_attr.stAuto.st3DParams.lo_med_en[i] = 1;
                    bayertnrV2_attr.stAuto.st3DParams.lo_gsbay_en[i] = 1;
                    bayertnrV2_attr.stAuto.st3DParams.lo_gslum_en[i] = 1;
                    bayertnrV2_attr.stAuto.st3DParams.hi_med_en[i] = 1;
                    bayertnrV2_attr.stAuto.st3DParams.hi_gslum_en[i] = 1;
                    bayertnrV2_attr.stAuto.st3DParams.global_pk_en[i] = 0;
                    bayertnrV2_attr.stAuto.st3DParams.global_pksq[i] = 1024;

                    bayertnrV2_attr.stAuto.st3DParams.lo_filter_strength[i] = 1.0;
                    bayertnrV2_attr.stAuto.st3DParams.hi_filter_strength[i] = 1.0;

                    bayertnrV2_attr.stAuto.st3DParams.soft_threshold_ratio[i] = 0.0;
                    bayertnrV2_attr.stAuto.st3DParams.hi_wgt_comp[i] = 0.16;
                    bayertnrV2_attr.stAuto.st3DParams.clipwgt[i] = 0.03215;
                    bayertnrV2_attr.stAuto.st3DParams.hidif_th[i] = 32767;

                }

                ret = rk_aiq_user_api2_abayertnrV2_SetAttrib(ctx, &bayertnrV2_attr);
                printf("set abayertnr attri auto ret:%d \n\n", ret);

                rk_aiq_bayertnr_attrib_v2_t get_bayertnrV2_attr;
                get_bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &get_bayertnrV2_attr);
                printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, get_bayertnrV2_attr.sync.done);
            }
            break;
        case 'd':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayertnr_attrib_v2_t bayertnrV2_attr;
                bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &bayertnrV2_attr);
                bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                bayertnrV2_attr.eMode = ABAYERTNRV2_OP_MODE_MANUAL;
                bayertnrV2_attr.stManual.st3DSelect.enable = 1;


                bayertnrV2_attr.stManual.st3DSelect.lumapoint[0] = 512;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[1] = 1024;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[2] = 1536;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[3] = 2048;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[4] = 3072;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[5] = 4096;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[6] = 5120;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[7] = 6144;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[8] = 7168;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[9] = 8192;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[10] = 9216;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[11] = 10240;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[12] = 11264;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[13] = 12288;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[14] = 13312;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint[15] = 14336;

                bayertnrV2_attr.stManual.st3DSelect.sigma[0] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[1] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[2] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[3] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[4] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[5] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[6] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[7] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[8] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[9] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[10] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[11] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[12] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[13] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[14] = 90;
                bayertnrV2_attr.stManual.st3DSelect.sigma[15] = 90;

                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[0] = 512;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[1] = 1024;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[2] = 1536;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[3] = 2048;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[4] = 3072;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[5] = 4096;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[6] = 5120;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[7] = 6144;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[8] = 7168;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[9] = 8192;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[10] = 9216;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[11] = 10240;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[12] = 11264;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[13] = 12288;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[14] = 13312;
                bayertnrV2_attr.stManual.st3DSelect.lumapoint2[15] = 14336;


                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[0] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[1] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[2] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[3] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[4] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[5] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[6] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[7] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[8] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[9] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[10] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[11] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[12] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[13] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[14] = 32;
                bayertnrV2_attr.stManual.st3DSelect.lo_sigma[15] = 32;

                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[0] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[1] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[2] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[3] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[4] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[5] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[6] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[7] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[8] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[9] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[10] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[11] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[12] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[13] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[14] = 256;
                bayertnrV2_attr.stManual.st3DSelect.hi_sigma[15] = 256;

                bayertnrV2_attr.stManual.st3DSelect.thumbds = 8;
                bayertnrV2_attr.stManual.st3DSelect.lo_enable = 1;
                bayertnrV2_attr.stManual.st3DSelect.hi_enable = 1;
                bayertnrV2_attr.stManual.st3DSelect.lo_med_en = 1;
                bayertnrV2_attr.stManual.st3DSelect.lo_gsbay_en = 1;
                bayertnrV2_attr.stManual.st3DSelect.lo_gslum_en = 1;
                bayertnrV2_attr.stManual.st3DSelect.hi_med_en = 1;
                bayertnrV2_attr.stManual.st3DSelect.hi_gslum_en = 1;
                bayertnrV2_attr.stManual.st3DSelect.global_pk_en = 0;
                bayertnrV2_attr.stManual.st3DSelect.global_pksq = 1024;

                bayertnrV2_attr.stManual.st3DSelect.lo_filter_strength = 1.0;
                bayertnrV2_attr.stManual.st3DSelect.hi_filter_strength = 1.0;

                bayertnrV2_attr.stManual.st3DSelect.soft_threshold_ratio = 0.0;
                bayertnrV2_attr.stManual.st3DSelect.hi_wgt_comp = 0.16;
                bayertnrV2_attr.stManual.st3DSelect.clipwgt = 0.03215;
                bayertnrV2_attr.stManual.st3DSelect.hidif_th = 32767;


                ret = rk_aiq_user_api2_abayertnrV2_SetAttrib(ctx, &bayertnrV2_attr);
                printf("set bayertnr v2 attri manual ret:%d \n\n", ret);

                rk_aiq_bayertnr_attrib_v2_t get_bayertnrV2_attr;
                get_bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &get_bayertnrV2_attr);
                printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, get_bayertnrV2_attr.sync.done);
            }

            break;
        case 'e':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayertnr_attrib_v2_t bayertnrV2_attr;
                bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &bayertnrV2_attr);
                bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                bayertnrV2_attr.eMode = ABAYERTNRV2_OP_MODE_REG_MANUAL;

                // BAY3D_BAY3D_CTRL 0x2c00
                bayertnrV2_attr.stManual.st3DFix.bay3d_exp_sel = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_soft_st = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_soft_mode = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_bwsaving_en = 0;

                bayertnrV2_attr.stManual.st3DFix.bay3d_loswitch_protect = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_glbpk_en = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_logaus3_bypass_en = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_logaus5_bypass_en = 0;

                bayertnrV2_attr.stManual.st3DFix.bay3d_lomed_bypass_en = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_hichnsplit_en = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_hiabs_pssel = 1;
                bayertnrV2_attr.stManual.st3DFix.bay3d_higaus_bypass_en = 0;

                bayertnrV2_attr.stManual.st3DFix.bay3d_himed_bypass_en = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_lobypass_en = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_hibypass_en = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_bypass_en = 0;
                bayertnrV2_attr.stManual.st3DFix.bay3d_en_i = 1;

                // BAY3D_BAY3D_KALRATIO 0x2c04
                bayertnrV2_attr.stManual.st3DFix.bay3d_softwgt = 0x100;
                bayertnrV2_attr.stManual.st3DFix.bay3d_hidif_th = 0xffff;

                // BAY3D_BAY3D_GLBPK2 0x2c08
                bayertnrV2_attr.stManual.st3DFix.bay3d_glbpk2 = 0x0000800;

                // BAY3D_BAY3D_WGTLMT 0x2c10
                bayertnrV2_attr.stManual.st3DFix.bay3d_wgtlmt = 0x380;
                bayertnrV2_attr.stManual.st3DFix.bay3d_wgtratio = 0x000;

                // BAY3D_BAY3D_SIG_X0  0x2c14 - 0x2c30
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[0] = 0x200;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[1] = 0x400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[2] = 0x600;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[3] = 0x800;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[4] = 0xc00;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[5] = 0x1000;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[6] = 0x1400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[7] = 0x1800;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[8] = 0x1c00;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[9] = 0x2000;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[10] = 0x2400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[11] = 0x2800;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[12] = 0x2c00;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[13] = 0x3000;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[14] = 0x3400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_x[15] = 0x3800;

                // BAY3D_BAY3D_SIG0_Y0 0x2c34 - 0x2c50
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[0] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[1] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[2] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[3] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[4] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[5] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[6] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[7] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[8] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[9] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[10] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[11] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[12] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[13] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[14] = 0x0300;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig0_y[15] = 0x0300;

                // BAY3D_BAY3D_SIG_X0  0x2c54 - 0x2c70
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[0] = 0x200;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[1] = 0x400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[2] = 0x600;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[3] = 0x800;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[4] = 0xc00;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[5] = 0x1000;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[6] = 0x1400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[7] = 0x1800;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[8] = 0x1c00;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[9] = 0x2000;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[10] = 0x2400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[11] = 0x2800;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[12] = 0x2c00;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[13] = 0x3000;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[14] = 0x3400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_x[15] = 0x3800;

                // ISP_BAYNR_3A00_SIGMAY0-15
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[0] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[1] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[2] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[3] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[4] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[5] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[6] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[7] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[8] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[9] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[10] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[11] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[12] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[13] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[14] = 0x0400;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig1_y[15] = 0x0400;

                // ISP_BAYNR_3A00_SIGMAY0-15
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[0] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[1] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[2] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[3] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[4] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[5] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[6] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[7] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[8] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[9] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[10] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[11] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[12] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[13] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[14] = 0x080;
                bayertnrV2_attr.stManual.st3DFix.bay3d_sig2_y[15] = 0x080;


                //bayertnrV2_attr.stManual.st3DFix.ro_sum_lodif = 0x0;
                //bayertnrV2_attr.stManual.st3DFix.ro_sum_hidif0 = 0x0;

                //bayertnrV2_attr.stManual.st3DFix.sw_bay3dmi_st_linemode = 0x01;
                //bayertnrV2_attr.stManual.st3DFix.sw_bay3d_mi2cur_linecnt = 0x4e;


                ret = rk_aiq_user_api2_abayertnrV2_SetAttrib(ctx, &bayertnrV2_attr);
                printf("set bayertnr v2 attri manual ret:%d \n\n", ret);

                rk_aiq_bayertnr_attrib_v2_t get_bayertnrV2_attr;
                get_bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &get_bayertnrV2_attr);
                printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, get_bayertnrV2_attr.sync.done);
            }
            break;
        case 'f':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
                bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                bayertnrV2Strenght.strength_enable = true;
                bayertnrV2Strenght.percent = 1.0;
                ret = rk_aiq_user_api2_abayertnrV2_SetStrength(ctx, &bayertnrV2Strenght);
                printf("Set abayertnr v2 set streangth ret:%d strength:%f \n\n", ret, bayertnrV2Strenght.percent);

                rk_aiq_bayertnr_strength_v2_t get_bayertnrV2Strenght;
                get_bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetStrength(ctx, &get_bayertnrV2Strenght);
                printf("get abayertnr v2 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_bayertnrV2Strenght.percent, get_bayertnrV2Strenght.sync.done);
            }
            break;
        case 'g':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
                bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                bayertnrV2Strenght.strength_enable = true;
                bayertnrV2Strenght.percent = 0.0;
                ret = rk_aiq_user_api2_abayertnrV2_SetStrength(ctx, &bayertnrV2Strenght);
                printf("Set abayertnr v2 set streangth ret:%d strength:%f \n\n", ret, bayertnrV2Strenght.percent);

                rk_aiq_bayertnr_strength_v2_t get_bayertnrV2Strenght;
                get_bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetStrength(ctx, &get_bayertnrV2Strenght);
                printf("get abayertnr v2 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_bayertnrV2Strenght.percent, get_bayertnrV2Strenght.sync.done);
            }
            break;
        case 'h':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
                bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                bayertnrV2Strenght.strength_enable = true;
                bayertnrV2Strenght.percent = 0.5;
                ret = rk_aiq_user_api2_abayertnrV2_SetStrength(ctx, &bayertnrV2Strenght);
                printf("Set abayertnr v2 set streangth ret:%d strength:%f \n\n", ret, bayertnrV2Strenght.percent);

                rk_aiq_bayertnr_strength_v2_t get_bayertnrV2Strenght;
                get_bayertnrV2Strenght.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetStrength(ctx, &get_bayertnrV2Strenght);
                printf("get abayertnr v2 attri ret:%d strength:%f done:%d\n\n",
                       ret, get_bayertnrV2Strenght.percent, get_bayertnrV2Strenght.sync.done);
            }
            break;
        case 'i':
            if (CHECK_ISP_HW_V30()) {
                default_bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayertnrV2_SetAttrib(ctx, &default_bayertnrV2_attr);
                printf("set bayertnr v2 default attri ret:%d \n\n", ret);

                rk_aiq_bayertnr_attrib_v2_t get_bayertnrV2_attr;
                get_bayertnrV2_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &get_bayertnrV2_attr);
                printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, get_bayertnrV2_attr.sync.done);
            }
            break;
        default:
            printf("not support test\n\n");
            break;
        }

    } while (key != 'q' && key != 'Q');

    return ret;
}

