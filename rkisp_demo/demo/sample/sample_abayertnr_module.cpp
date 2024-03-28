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

#ifdef ISP_HW_V39
#include "rk_aiq_user_api2_rk3576.h"
#elif  defined(ISP_HW_V32)
#include "rk_aiq_user_api2_rv1106.h"
#endif

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
    printf("\t j) BTNR:              test mode/en switch.\n");
    printf("\t q) ABAYERTNR:         press key q or Q to quit.\n");



}

void sample_print_abayertnr_info(const void *arg)
{
    printf ("enter ABAYERTNR modult test!\n");
}

XCamReturn sample_abayertnr_getAttr_v2(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_attrib_v2_t bayertnrV2_attr;
    bayertnrV2_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &bayertnrV2_attr);
    printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, bayertnrV2_attr.sync.done);

    return ret;
}

XCamReturn sample_abayertnr_getAttr_v23(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_attrib_v23_t bayertnr_attr;
    bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV23_GetAttrib(ctx, &bayertnr_attr);
    printf("get abayertnr v23 attri ret:%d done:%d \n\n", ret, bayertnr_attr.sync.done);

    return ret;
}

XCamReturn sample_abayertnr_getAttr_v23Lite(const rk_aiq_sys_ctx_t* ctx,
        rk_aiq_uapi_mode_sync_e sync_mode) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_attrib_v23L_t bayertnr_attr;
    bayertnr_attr.sync.sync_mode = sync_mode;
    ret                          = rk_aiq_user_api2_abayertnrV23Lite_GetAttrib(ctx, &bayertnr_attr);
    printf("get abayertnr v23Lite attri ret:%d done:%d \n\n", ret, bayertnr_attr.sync.done);

    return ret;
}

XCamReturn sample_abayertnr_getAttr_v30(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_attrib_v30_t bayertnr_attr;
    bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV30_GetAttrib(ctx, &bayertnr_attr);
    printf("get abayertnr v30 attri ret:%d done:%d \n\n", ret, bayertnr_attr.sync.done);

    return ret;
}

XCamReturn sample_abayertnr_getStrength_v2(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
    bayertnrV2Strenght.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV2_GetStrength(ctx, &bayertnrV2Strenght);
    printf("get abayertnr v2 attri ret:%d strength:%f done:%d\n\n",
           ret, bayertnrV2Strenght.percent, bayertnrV2Strenght.sync.done);


    return ret;
}

XCamReturn sample_abayertnr_getStrength_v23(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_strength_v23_t bayertnrStrenght;
    bayertnrStrenght.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV23_GetStrength(ctx, &bayertnrStrenght);
    printf("get abayertnr v23 attri ret:%d strength:%f done:%d\n\n",
           ret, bayertnrStrenght.percent, bayertnrStrenght.sync.done);


    return ret;
}


XCamReturn sample_abayertnr_getStrength_v30(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_strength_v30_t bayertnrStrenght;
    bayertnrStrenght.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV30_GetStrength(ctx, &bayertnrStrenght);
    printf("get abayertnr v30 attri ret:%d strength:%f done:%d\n\n",
           ret, bayertnrStrenght.percent, bayertnrStrenght.sync.done);


    return ret;
}

XCamReturn sample_abayertnr_setAuto_v2(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_attrib_v2_t bayertnrV2_attr;
    bayertnrV2_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &bayertnrV2_attr);
    bayertnrV2_attr.sync.sync_mode = sync_mode;
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
    get_bayertnrV2_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &get_bayertnrV2_attr);
    printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, get_bayertnrV2_attr.sync.done);
    return ret;
}

XCamReturn sample_abayertnr_setAuto_v23(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_attrib_v23_t bayertnr_attr;
    bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV23_GetAttrib(ctx, &bayertnr_attr);
    bayertnr_attr.sync.sync_mode = sync_mode;
    bayertnr_attr.eMode = ABAYERTNRV23_OP_MODE_AUTO;
    bayertnr_attr.stAuto.st3DParams.enable = 1;

    for(int i = 0; i < RK_BAYERNR_V23_MAX_ISO_NUM; i++) {
        bayertnr_attr.stAuto.st3DParams.iso[i] = 50 * pow(2, i);

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[0] = 512;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[1] = 1024;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[2] = 1536;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[3] = 2048;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[4] = 3072;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[5] = 4096;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[6] = 5120;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[7] = 6144;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[8] = 7168;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[9] = 8192;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[10] = 9216;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[11] = 10240;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[12] = 11264;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[13] = 12288;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[14] = 13312;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[15] = 14336;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[0] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[1] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[2] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[3] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[4] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[5] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[6] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[7] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[8] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[9] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[10] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[11] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[12] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[13] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[14] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[15] = 90;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[0] = 512;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[1] = 1024;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[2] = 1536;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[3] = 2048;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[4] = 3072;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[5] = 4096;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[6] = 5120;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[7] = 6144;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[8] = 7168;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[9] = 8192;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[10] = 9216;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[11] = 10240;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[12] = 11264;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[13] = 12288;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[14] = 13312;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[15] = 14336;


        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[0] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[1] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[2] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[3] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[4] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[5] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[6] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[7] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[8] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[9] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[10] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[11] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[12] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[13] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[14] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[15] = 90;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[0] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[1] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[2] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[3] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[4] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[5] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[6] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[7] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[8] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[9] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[10] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[11] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[12] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[13] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[14] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[15] = 90;


        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].thumbds_w = 8;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].thumbds_h = 4;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_enable = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_enable = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_med_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_gsbay_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_gslum_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_med_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_gslum_en = 1;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].global_pk_en = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].global_pksq = 1024;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_filter_strength = 1.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_filter_strength = 1.0;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].soft_threshold_ratio = 0.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_wgt_comp = 0.16;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_clipwgt = 0.03215;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hidif_th = 32767;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].trans_en = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].wgt_use_mode = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].wgt_mge_mode = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_guass = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].kl_guass = 0;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_filter_abs_ctrl = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_filter_filt_bay = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_filter_filt_avg = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_filter_filt_mode = 4;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_filter_rat0 = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_filter_thed0 = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_filter_rat0 = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_filter_thed0 = 256;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_filter_rat1 = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_filter_thed1 = 1024;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].guass_guide_coeff0 = 16;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].guass_guide_coeff1 = 8;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].guass_guide_coeff2 = 16;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].guass_guide_coeff3 = 8;

    }

    ret = rk_aiq_user_api2_abayertnrV23_SetAttrib(ctx, &bayertnr_attr);
    printf("set abayertnr attri auto ret:%d \n\n", ret);

    rk_aiq_bayertnr_attrib_v23_t get_bayertnr_attr;
    get_bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV23_GetAttrib(ctx, &get_bayertnr_attr);
    printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, get_bayertnr_attr.sync.done);
    return ret;
}

XCamReturn sample_abayertnr_setAuto_v23Lite(const rk_aiq_sys_ctx_t* ctx,
        rk_aiq_uapi_mode_sync_e sync_mode) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_attrib_v23L_t bayertnr_attr;
    bayertnr_attr.sync.sync_mode = sync_mode;
    ret                          = rk_aiq_user_api2_abayertnrV23Lite_GetAttrib(ctx, &bayertnr_attr);
    bayertnr_attr.sync.sync_mode = sync_mode;
    bayertnr_attr.eMode          = ABAYERTNRV23_OP_MODE_AUTO;
    bayertnr_attr.stAuto.st3DParams.enable = 1;

    for (int i = 0; i < RK_BAYERNR_V23_MAX_ISO_NUM; i++) {
        bayertnr_attr.stAuto.st3DParams.iso[i] = 50 * pow(2, i);

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[0]  = 512;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[1]  = 1024;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[2]  = 1536;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[3]  = 2048;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[4]  = 3072;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[5]  = 4096;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[6]  = 5120;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[7]  = 6144;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[8]  = 7168;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[9]  = 8192;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[10] = 9216;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[11] = 10240;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[12] = 11264;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[13] = 12288;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[14] = 13312;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint[15] = 14336;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[0]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[1]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[2]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[3]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[4]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[5]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[6]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[7]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[8]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[9]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[10] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[11] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[12] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[13] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[14] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sigma[15] = 90;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[0]  = 512;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[1]  = 1024;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[2]  = 1536;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[3]  = 2048;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[4]  = 3072;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[5]  = 4096;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[6]  = 5120;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[7]  = 6144;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[8]  = 7168;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[9]  = 8192;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[10] = 9216;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[11] = 10240;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[12] = 11264;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[13] = 12288;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[14] = 13312;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lumapoint2[15] = 14336;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[0]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[1]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[2]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[3]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[4]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[5]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[6]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[7]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[8]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[9]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[10] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[11] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[12] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[13] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[14] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_sigma[15] = 90;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[0]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[1]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[2]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[3]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[4]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[5]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[6]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[7]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[8]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[9]  = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[10] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[11] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[12] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[13] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[14] = 90;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_sigma[15] = 90;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].thumbds_w = 8;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].thumbds_h = 4;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_enable = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_enable = 1;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_gsbay_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_gslum_en = 1;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_gslum_en = 1;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].global_pk_en = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].global_pksq  = 1024;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_filter_strength = 1.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_filter_strength = 1.0;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].soft_threshold_ratio = 0.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_wgt_comp          = 0.16;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_clipwgt           = 0.03215;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hidif_th             = 32767;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].trans_en     = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].wgt_use_mode = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].wgt_mge_mode = 1;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_filter_abs_ctrl = 0;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_filter_rat0  = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].lo_filter_thed0 = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_filter_rat0  = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_filter_thed0 = 256;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_filter_rat1  = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hi_filter_thed1 = 1024;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].guass_guide_coeff0 = 16;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].guass_guide_coeff1 = 8;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].guass_guide_coeff2 = 4;
        // bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].guass_guide_coeff3 = 0;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].wgtmm_opt_en = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].wgtmm_sel_en = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].wgtmin       = 0;
    }

    ret = rk_aiq_user_api2_abayertnrV23Lite_SetAttrib(ctx, &bayertnr_attr);
    printf("set abayertnr attri v23Lite auto ret:%d \n\n", ret);

    rk_aiq_bayertnr_attrib_v23L_t get_bayertnr_attr;
    get_bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV23Lite_GetAttrib(ctx, &get_bayertnr_attr);
    printf("get abayertnr v23Lite attri ret:%d done:%d \n\n", ret, get_bayertnr_attr.sync.done);
    return ret;
}

XCamReturn sample_abayertnr_setAuto_v30(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_attrib_v30_t bayertnr_attr;
    bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV30_GetAttrib(ctx, &bayertnr_attr);
    bayertnr_attr.sync.sync_mode = sync_mode;
    bayertnr_attr.eMode = ABAYERTNRV30_OP_MODE_AUTO;
    bayertnr_attr.stAuto.st3DParams.enable = 1;

    for(int i = 0; i < RK_BAYERNR_V30_MAX_ISO_NUM; i++) {
        bayertnr_attr.stAuto.st3DParams.iso[i] = 50 * pow(2, i);

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_mdBypss_en = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_curFiltOut_en = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_iirSpnrOut_en = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_mdWgtOut_en = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_curFiltOut_en = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_loDetection_mode = 0;


        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[0] = 128;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[1] = 256;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[2] = 384;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[3] = 512;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[4] = 640;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[5] = 768;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[6] = 896;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[7] = 1024;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[8] = 1152;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[9] = 1280;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[10] = 1536;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[11] = 1792;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[12] = 2048;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[13] = 2304;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[14] = 2560;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[15] = 2816;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[16] = 3072;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[17] = 3328;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[18] = 3584;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[19] = 4095;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[0] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[1] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[2] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[3] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[4] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[5] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[6] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[7] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[8] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[9] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[10] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[11] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[12] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[13] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[14] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[15] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[16] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[17] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[18] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[19] = 32;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_luma2sigma_idx[0] = 256;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_luma2sigma_idx[1] = 512;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_luma2sigma_idx[2] = 768;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_luma2sigma_idx[3] = 1024;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_luma2sigma_idx[4] = 1280;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_luma2sigma_idx[5] = 1536;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_luma2sigma_idx[6] = 1792;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_luma2sigma_idx[7] = 2048;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_luma2sigma_idx[8] = 2304;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_luma2sigma_idx[9] = 2560;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_luma2sigma_idx[10] = 2816;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_luma2sigma_idx[11] = 3072;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_luma2sigma_idx[12] = 3328;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_luma2sigma_idx[13] = 3584;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_luma2sigma_idx[14] = 3840;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_luma2sigma_idx[15] = 4095;


        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[0] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[1] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[2] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[3] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[4] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[5] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[6] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[7] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[8] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[9] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[10] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[11] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[12] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[13] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[14] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[15] = 32;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_preluma2sigma_val[0] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_preluma2sigma_val[1] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_preluma2sigma_val[2] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_preluma2sigma_val[3] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_preluma2sigma_val[4] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_preluma2sigma_val[5] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_preluma2sigma_val[6] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_preluma2sigma_val[7] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_preluma2sigma_val[8] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_preluma2sigma_val[9] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_preluma2sigma_val[10] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_preluma2sigma_val[11] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_preluma2sigma_val[12] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_preluma2sigma_val[13] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_preluma2sigma_val[14] = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[15] = 32;


        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_curSpnr_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_preSpnr_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_btnr_tnrFilt_strg = 1.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_btnr_curSpnrFilt_strg = 1.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_btnr_preSpnrFilt_strg = 1.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_curSpnrSigmaIdxFilt_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_preSpnrSigmaIdxFilt_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_curSpnr_wgt = 1.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_preSpnr_wgt = 1.0;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_curSpnrWgtCal_scale = 0.15;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_curSpnrWgtCal_offset = 0.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_preSpnrWgtCal_scale = 0.15;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_preSpnrWgtCal_offset = 0.0;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_spnrPresigmaUse_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_curSpnrSigma_scale = 1.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_curSpnrSigma_offset = 0.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_curSpnrSigma_scale = 1.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_curSpnrSigma_offset = 0.0;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_curSpnrSigmaHdrS_scale = 1.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_curSpnrSigmaHdrS_offset = 0.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_preSpnrSigmaHdrS_scale = 1.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_preSpnrSigmaHdrS_offset = 0.0;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_transf_en = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_transf_mode = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_transfMode0_scale = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_btnr_transfMode0_offset = 256;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_btnr_itransfMode0_offset = 2048;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_btnr_transfMode1_offset = 256;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_btnr_itransfMode1_offset = 1024;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_transfData_maxLimit = 1048575;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_gKalman_en = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_gKalman_wgt = 1024;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_wgtCal_mode = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_lpfHi_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_btnr_lpfHi_sigma = 1.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_lpfLo_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_btnr_lpfLo_sigma = 1.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_sigmaIdxFilt_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_sigmaIdxFilt_mode = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_sigma_scale = 1.0;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_wgtFilt_en = 1;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_mode0LoWgt_scale = 2.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_mode0Base_ratio = 4.0;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_mode1LoWgt_scale = 1.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_mode1LoWgt_offset = 0.5;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_mode1Wgt_offset = 0.5;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_mode1Wgt_minLimit = 0.05;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_mode1Wgt_scale = 1.0;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_loDetection_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_loDiffVfilt_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_loDiffVfilt_mode = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_loDiffHfilt_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_loDiffWgtCal_scale = 2.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_loDiffWgtCal_offset = 0.125;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_loDiffFirstLine_scale = 0.125;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_loDiffVfilt_wgt = 0.875;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_sigmaHfilt_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_sigmaFirstLine_scale = 1.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_sigmaVfilt_wgt = 0.875;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_loWgtCal_maxLimit = 4;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_loWgtCal_scale = 3.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_loWgtCal_offset = 0.5;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_loWgtHfilt_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_loWgtVfilt_wgt = 0.875;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_sigmaHdrS_scale = 1.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_mode0LoWgtHdrS_scale = 1.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_mode1LoWgtHdrS_scale = 0.7;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_mode1LoWgtHdrS_offset = 0.5;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_mode1WgtHdrS_scale = 1.4;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_btnr_loMgeFrame_maxLimit = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_btnr_loMgeFrame_minLimit = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_btnr_hiMgeFrame_maxLimit = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_btnr_hiMgeFrame_minLimit = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_btnr_loMgeFrameHdrS_maxLimit = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_btnr_loMgeFrameHdrS_minLimit = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_btnr_hiMgeFrameHdrS_maxLimit = 32;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_btnr_hiMgeFrameHdrS_minLimit = 0;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_autoSigCount_en = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_autoSigCount_thred = 0.0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_autoSigCountWgt_thred = 0.8;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_autoSigCountFilt_wgt = 0.7;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_autoSigCountSpnr_en = 1;

        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_curSpnrHiWgt_minLimit = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_preSpnrHiWgt_minLimit = 0;


        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_gainOut_maxLimit = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_noiseBal_mode = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_btnr_noiseBalNr_strg = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_curSpnrSpaceWgt_sigma = 25;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].sw_iirSpnrSpaceWgt_sigma = 25;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_tnrWgtFltCoef[0] = 4;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_tnrWgtFltCoef[1] = 2;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_tnrWgtFltCoef[2] = 1;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_bay3d_lowgt_ctrl = 0;
        bayertnr_attr.stAuto.st3DParams.bayertnrParamISO[i].hw_bay3d_lowgt_offinit = 0;

    }

    ret = rk_aiq_user_api2_abayertnrV30_SetAttrib(ctx, &bayertnr_attr);
    printf("set abayertnr attri auto ret:%d \n\n", ret);

    rk_aiq_bayertnr_attrib_v30_t get_bayertnr_attr;
    get_bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV30_GetAttrib(ctx, &get_bayertnr_attr);
    printf("get abayertnr v30 attri ret:%d done:%d \n\n", ret, get_bayertnr_attr.sync.done);
    return ret;
}

XCamReturn sample_abayertnr_setManual_v2(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_attrib_v2_t bayertnrV2_attr;
    bayertnrV2_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &bayertnrV2_attr);
    bayertnrV2_attr.sync.sync_mode = sync_mode;
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
    get_bayertnrV2_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &get_bayertnrV2_attr);
    printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, get_bayertnrV2_attr.sync.done);
    return ret;
}


XCamReturn sample_abayertnr_setManual_v23(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_attrib_v23_t bayertnr_attr;
    bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV23_GetAttrib(ctx, &bayertnr_attr);
    bayertnr_attr.sync.sync_mode = sync_mode;
    bayertnr_attr.eMode = ABAYERTNRV23_OP_MODE_MANUAL;
    bayertnr_attr.stManual.st3DSelect.enable = 1;


    bayertnr_attr.stManual.st3DSelect.lumapoint[0] = 512;
    bayertnr_attr.stManual.st3DSelect.lumapoint[1] = 1024;
    bayertnr_attr.stManual.st3DSelect.lumapoint[2] = 1536;
    bayertnr_attr.stManual.st3DSelect.lumapoint[3] = 2048;
    bayertnr_attr.stManual.st3DSelect.lumapoint[4] = 3072;
    bayertnr_attr.stManual.st3DSelect.lumapoint[5] = 4096;
    bayertnr_attr.stManual.st3DSelect.lumapoint[6] = 5120;
    bayertnr_attr.stManual.st3DSelect.lumapoint[7] = 6144;
    bayertnr_attr.stManual.st3DSelect.lumapoint[8] = 7168;
    bayertnr_attr.stManual.st3DSelect.lumapoint[9] = 8192;
    bayertnr_attr.stManual.st3DSelect.lumapoint[10] = 9216;
    bayertnr_attr.stManual.st3DSelect.lumapoint[11] = 10240;
    bayertnr_attr.stManual.st3DSelect.lumapoint[12] = 11264;
    bayertnr_attr.stManual.st3DSelect.lumapoint[13] = 12288;
    bayertnr_attr.stManual.st3DSelect.lumapoint[14] = 13312;
    bayertnr_attr.stManual.st3DSelect.lumapoint[15] = 14336;

    bayertnr_attr.stManual.st3DSelect.sigma[0] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[1] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[2] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[3] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[4] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[5] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[6] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[7] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[8] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[9] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[10] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[11] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[12] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[13] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[14] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[15] = 90;

    bayertnr_attr.stManual.st3DSelect.lumapoint2[0] = 512;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[1] = 1024;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[2] = 1536;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[3] = 2048;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[4] = 3072;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[5] = 4096;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[6] = 5120;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[7] = 6144;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[8] = 7168;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[9] = 8192;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[10] = 9216;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[11] = 10240;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[12] = 11264;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[13] = 12288;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[14] = 13312;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[15] = 14336;


    bayertnr_attr.stManual.st3DSelect.lo_sigma[0] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[1] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[2] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[3] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[4] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[5] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[6] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[7] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[8] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[9] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[10] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[11] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[12] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[13] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[14] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[15] = 32;

    bayertnr_attr.stManual.st3DSelect.hi_sigma[0] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[1] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[2] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[3] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[4] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[5] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[6] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[7] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[8] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[9] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[10] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[11] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[12] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[13] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[14] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[15] = 256;

    bayertnr_attr.stManual.st3DSelect.thumbds_w = 8;
    bayertnr_attr.stManual.st3DSelect.thumbds_h = 4;
    bayertnr_attr.stManual.st3DSelect.lo_enable = 1;
    bayertnr_attr.stManual.st3DSelect.hi_enable = 1;
    bayertnr_attr.stManual.st3DSelect.lo_med_en = 1;
    bayertnr_attr.stManual.st3DSelect.lo_gsbay_en = 1;
    bayertnr_attr.stManual.st3DSelect.lo_gslum_en = 1;
    bayertnr_attr.stManual.st3DSelect.hi_med_en = 1;
    bayertnr_attr.stManual.st3DSelect.hi_gslum_en = 1;
    bayertnr_attr.stManual.st3DSelect.global_pk_en = 0;
    bayertnr_attr.stManual.st3DSelect.global_pksq = 1024;

    bayertnr_attr.stManual.st3DSelect.lo_filter_strength = 1.0;
    bayertnr_attr.stManual.st3DSelect.hi_filter_strength = 1.0;

    bayertnr_attr.stManual.st3DSelect.soft_threshold_ratio = 0.0;
    bayertnr_attr.stManual.st3DSelect.hi_wgt_comp = 0.16;
    bayertnr_attr.stManual.st3DSelect.lo_clipwgt = 0.03215;
    bayertnr_attr.stManual.st3DSelect.hidif_th = 32767;


    bayertnr_attr.stManual.st3DSelect.trans_en = 0;
    bayertnr_attr.stManual.st3DSelect.wgt_use_mode = 0;
    bayertnr_attr.stManual.st3DSelect.wgt_mge_mode = 1;
    bayertnr_attr.stManual.st3DSelect.hi_guass = 0;
    bayertnr_attr.stManual.st3DSelect.kl_guass = 0;

    bayertnr_attr.stManual.st3DSelect.hi_filter_abs_ctrl = 0;
    bayertnr_attr.stManual.st3DSelect.hi_filter_filt_bay = 0;
    bayertnr_attr.stManual.st3DSelect.hi_filter_filt_avg = 0;
    bayertnr_attr.stManual.st3DSelect.hi_filter_filt_mode = 4;

    bayertnr_attr.stManual.st3DSelect.lo_filter_rat0 = 1;
    bayertnr_attr.stManual.st3DSelect.lo_filter_thed0 = 0;
    bayertnr_attr.stManual.st3DSelect.hi_filter_rat0 = 0;
    bayertnr_attr.stManual.st3DSelect.hi_filter_thed0 = 256;
    bayertnr_attr.stManual.st3DSelect.hi_filter_rat1 = 0;
    bayertnr_attr.stManual.st3DSelect.hi_filter_thed1 = 1024;

    bayertnr_attr.stManual.st3DSelect.guass_guide_coeff0 = 16;
    bayertnr_attr.stManual.st3DSelect.guass_guide_coeff1 = 8;
    bayertnr_attr.stManual.st3DSelect.guass_guide_coeff2 = 16;
    bayertnr_attr.stManual.st3DSelect.guass_guide_coeff3 = 8;


    ret = rk_aiq_user_api2_abayertnrV23_SetAttrib(ctx, &bayertnr_attr);
    printf("set bayertnr v2 attri manual ret:%d \n\n", ret);

    rk_aiq_bayertnr_attrib_v23_t get_bayertnr_attr;
    get_bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV23_GetAttrib(ctx, &get_bayertnr_attr);
    printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, get_bayertnr_attr.sync.done);
    return ret;
}

XCamReturn sample_abayertnr_setManual_v23Lite(const rk_aiq_sys_ctx_t* ctx,
        rk_aiq_uapi_mode_sync_e sync_mode) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_attrib_v23L_t bayertnr_attr;
    bayertnr_attr.sync.sync_mode = sync_mode;
    ret                          = rk_aiq_user_api2_abayertnrV23Lite_GetAttrib(ctx, &bayertnr_attr);
    bayertnr_attr.sync.sync_mode = sync_mode;
    bayertnr_attr.eMode          = ABAYERTNRV23_OP_MODE_MANUAL;
    bayertnr_attr.stManual.st3DSelect.enable = 1;

    bayertnr_attr.stManual.st3DSelect.lumapoint[0]  = 512;
    bayertnr_attr.stManual.st3DSelect.lumapoint[1]  = 1024;
    bayertnr_attr.stManual.st3DSelect.lumapoint[2]  = 1536;
    bayertnr_attr.stManual.st3DSelect.lumapoint[3]  = 2048;
    bayertnr_attr.stManual.st3DSelect.lumapoint[4]  = 3072;
    bayertnr_attr.stManual.st3DSelect.lumapoint[5]  = 4096;
    bayertnr_attr.stManual.st3DSelect.lumapoint[6]  = 5120;
    bayertnr_attr.stManual.st3DSelect.lumapoint[7]  = 6144;
    bayertnr_attr.stManual.st3DSelect.lumapoint[8]  = 7168;
    bayertnr_attr.stManual.st3DSelect.lumapoint[9]  = 8192;
    bayertnr_attr.stManual.st3DSelect.lumapoint[10] = 9216;
    bayertnr_attr.stManual.st3DSelect.lumapoint[11] = 10240;
    bayertnr_attr.stManual.st3DSelect.lumapoint[12] = 11264;
    bayertnr_attr.stManual.st3DSelect.lumapoint[13] = 12288;
    bayertnr_attr.stManual.st3DSelect.lumapoint[14] = 13312;
    bayertnr_attr.stManual.st3DSelect.lumapoint[15] = 14336;

    bayertnr_attr.stManual.st3DSelect.sigma[0]  = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[1]  = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[2]  = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[3]  = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[4]  = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[5]  = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[6]  = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[7]  = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[8]  = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[9]  = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[10] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[11] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[12] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[13] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[14] = 90;
    bayertnr_attr.stManual.st3DSelect.sigma[15] = 90;

    bayertnr_attr.stManual.st3DSelect.lumapoint2[0]  = 512;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[1]  = 1024;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[2]  = 1536;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[3]  = 2048;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[4]  = 3072;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[5]  = 4096;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[6]  = 5120;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[7]  = 6144;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[8]  = 7168;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[9]  = 8192;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[10] = 9216;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[11] = 10240;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[12] = 11264;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[13] = 12288;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[14] = 13312;
    bayertnr_attr.stManual.st3DSelect.lumapoint2[15] = 14336;

    bayertnr_attr.stManual.st3DSelect.lo_sigma[0]  = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[1]  = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[2]  = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[3]  = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[4]  = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[5]  = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[6]  = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[7]  = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[8]  = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[9]  = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[10] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[11] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[12] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[13] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[14] = 32;
    bayertnr_attr.stManual.st3DSelect.lo_sigma[15] = 32;

    bayertnr_attr.stManual.st3DSelect.hi_sigma[0]  = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[1]  = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[2]  = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[3]  = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[4]  = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[5]  = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[6]  = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[7]  = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[8]  = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[9]  = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[10] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[11] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[12] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[13] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[14] = 256;
    bayertnr_attr.stManual.st3DSelect.hi_sigma[15] = 256;

    bayertnr_attr.stManual.st3DSelect.thumbds_w    = 8;
    bayertnr_attr.stManual.st3DSelect.thumbds_h    = 4;
    bayertnr_attr.stManual.st3DSelect.lo_enable    = 1;
    bayertnr_attr.stManual.st3DSelect.hi_enable    = 1;
    bayertnr_attr.stManual.st3DSelect.lo_gsbay_en  = 1;
    bayertnr_attr.stManual.st3DSelect.lo_gslum_en  = 1;
    bayertnr_attr.stManual.st3DSelect.hi_gslum_en  = 1;
    bayertnr_attr.stManual.st3DSelect.global_pk_en = 0;
    bayertnr_attr.stManual.st3DSelect.global_pksq  = 1024;

    bayertnr_attr.stManual.st3DSelect.lo_filter_strength = 1.0;
    bayertnr_attr.stManual.st3DSelect.hi_filter_strength = 1.0;

    bayertnr_attr.stManual.st3DSelect.soft_threshold_ratio = 0.0;
    bayertnr_attr.stManual.st3DSelect.hi_wgt_comp          = 0.16;
    bayertnr_attr.stManual.st3DSelect.lo_clipwgt           = 0.03215;
    bayertnr_attr.stManual.st3DSelect.hidif_th             = 32767;

    bayertnr_attr.stManual.st3DSelect.trans_en     = 0;
    bayertnr_attr.stManual.st3DSelect.wgt_use_mode = 1;
    bayertnr_attr.stManual.st3DSelect.wgt_mge_mode = 1;

    bayertnr_attr.stManual.st3DSelect.hi_filter_abs_ctrl = 0;

    bayertnr_attr.stManual.st3DSelect.lo_filter_rat0  = 1;
    bayertnr_attr.stManual.st3DSelect.lo_filter_thed0 = 0;
    bayertnr_attr.stManual.st3DSelect.hi_filter_rat0  = 0;
    bayertnr_attr.stManual.st3DSelect.hi_filter_thed0 = 256;
    bayertnr_attr.stManual.st3DSelect.hi_filter_rat1  = 0;
    bayertnr_attr.stManual.st3DSelect.hi_filter_thed1 = 1024;

    bayertnr_attr.stManual.st3DSelect.guass_guide_coeff0 = 16;
    bayertnr_attr.stManual.st3DSelect.guass_guide_coeff1 = 8;
    bayertnr_attr.stManual.st3DSelect.guass_guide_coeff2 = 4;
    // bayertnr_attr.stManual.st3DSelect.guass_guide_coeff3 = 8;

    bayertnr_attr.stManual.st3DSelect.wgtmm_opt_en = 0;
    bayertnr_attr.stManual.st3DSelect.wgtmm_sel_en = 0;
    bayertnr_attr.stManual.st3DSelect.wgtmin       = 0;

    ret = rk_aiq_user_api2_abayertnrV23Lite_SetAttrib(ctx, &bayertnr_attr);
    printf("set bayertnr v2 attri manual ret:%d \n\n", ret);

    rk_aiq_bayertnr_attrib_v23L_t get_bayertnr_attr;
    get_bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV23Lite_GetAttrib(ctx, &get_bayertnr_attr);
    printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, get_bayertnr_attr.sync.done);
    return ret;
}



XCamReturn sample_abayertnr_setManual_v30(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_attrib_v30_t bayertnr_attr;
    bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV30_GetAttrib(ctx, &bayertnr_attr);
    bayertnr_attr.sync.sync_mode = sync_mode;
    bayertnr_attr.eMode = ABAYERTNRV30_OP_MODE_MANUAL;
    bayertnr_attr.stManual.st3DSelect.enable = 1;

    bayertnr_attr.stManual.st3DSelect.hw_mdBypss_en = 0;
    bayertnr_attr.stManual.st3DSelect.hw_curFiltOut_en = 0;
    bayertnr_attr.stManual.st3DSelect.hw_iirSpnrOut_en = 0;
    bayertnr_attr.stManual.st3DSelect.hw_mdWgtOut_en = 0;
    bayertnr_attr.stManual.st3DSelect.hw_curFiltOut_en = 0;
    bayertnr_attr.stManual.st3DSelect.hw_loDetection_mode = 0;

    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[0] = 128;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[1] = 256;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[2] = 384;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[3] = 512;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[4] = 640;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[5] = 768;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[6] = 896;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[7] = 1024;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[8] = 1152;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[9] = 1280;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[10] = 1536;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[11] = 1792;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[12] = 2048;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[13] = 2304;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[14] = 2560;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[15] = 2816;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[16] = 3072;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[17] = 3328;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[18] = 3584;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_idx[19] = 4095;

    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[0] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[1] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[2] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[3] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[4] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[5] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[6] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[7] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[8] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[9] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[10] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[11] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[12] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[13] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[14] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[15] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[16] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[17] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[18] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_tnr_luma2sigma_val[19] = 32;

    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_luma2sigma_idx[0] = 256;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_luma2sigma_idx[1] = 512;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_luma2sigma_idx[2] = 768;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_luma2sigma_idx[3] = 1024;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_luma2sigma_idx[4] = 1280;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_luma2sigma_idx[5] = 1536;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_luma2sigma_idx[6] = 1792;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_luma2sigma_idx[7] = 2048;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_luma2sigma_idx[8] = 2304;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_luma2sigma_idx[9] = 2560;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_luma2sigma_idx[10] = 2816;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_luma2sigma_idx[11] = 3072;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_luma2sigma_idx[12] = 3328;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_luma2sigma_idx[13] = 3584;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_luma2sigma_idx[14] = 3840;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_luma2sigma_idx[15] = 4095;


    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_curluma2sigma_val[0] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_curluma2sigma_val[1] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_curluma2sigma_val[2] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_curluma2sigma_val[3] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_curluma2sigma_val[4] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_curluma2sigma_val[5] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_curluma2sigma_val[6] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_curluma2sigma_val[7] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_curluma2sigma_val[8] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_curluma2sigma_val[9] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_curluma2sigma_val[10] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_curluma2sigma_val[11] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_curluma2sigma_val[12] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_curluma2sigma_val[13] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_curluma2sigma_val[14] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_curluma2sigma_val[15] = 32;

    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_preluma2sigma_val[0] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_preluma2sigma_val[1] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_preluma2sigma_val[2] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_preluma2sigma_val[3] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_preluma2sigma_val[4] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_preluma2sigma_val[5] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_preluma2sigma_val[6] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_preluma2sigma_val[7] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_preluma2sigma_val[8] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_preluma2sigma_val[9] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_preluma2sigma_val[10] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_preluma2sigma_val[11] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_preluma2sigma_val[12] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_preluma2sigma_val[13] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_preluma2sigma_val[14] = 32;
    bayertnr_attr.stManual.st3DSelect.bayertnr_spnr_curluma2sigma_val[15] = 32;

    bayertnr_attr.stManual.st3DSelect.hw_btnr_curSpnr_en = 1;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_preSpnr_en = 1;
    bayertnr_attr.stManual.st3DSelect.sw_btnr_tnrFilt_strg = 1.0;
    bayertnr_attr.stManual.st3DSelect.sw_btnr_curSpnrFilt_strg = 1.0;
    bayertnr_attr.stManual.st3DSelect.sw_btnr_preSpnrFilt_strg = 1.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_curSpnrSigmaIdxFilt_en = 1;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_preSpnrSigmaIdxFilt_en = 1;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_curSpnr_wgt = 1.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_preSpnr_wgt = 1.0;

    bayertnr_attr.stManual.st3DSelect.hw_btnr_curSpnrWgtCal_scale = 0.15;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_curSpnrWgtCal_offset = 0.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_preSpnrWgtCal_scale = 0.15;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_preSpnrWgtCal_offset = 0.0;

    bayertnr_attr.stManual.st3DSelect.hw_btnr_spnrPresigmaUse_en = 1;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_curSpnrSigma_scale = 1.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_curSpnrSigma_offset = 0.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_curSpnrSigma_scale = 1.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_curSpnrSigma_offset = 0.0;

    bayertnr_attr.stManual.st3DSelect.hw_btnr_curSpnrSigmaHdrS_scale = 1.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_curSpnrSigmaHdrS_offset = 0.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_preSpnrSigmaHdrS_scale = 1.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_preSpnrSigmaHdrS_offset = 0.0;

    bayertnr_attr.stManual.st3DSelect.hw_btnr_transf_en = 0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_transf_mode = 0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_transfMode0_scale = 0;
    bayertnr_attr.stManual.st3DSelect.sw_btnr_transfMode0_offset = 256;
    bayertnr_attr.stManual.st3DSelect.sw_btnr_itransfMode0_offset = 2048;
    bayertnr_attr.stManual.st3DSelect.sw_btnr_transfMode1_offset = 256;
    bayertnr_attr.stManual.st3DSelect.sw_btnr_itransfMode1_offset = 1024;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_transfData_maxLimit = 1048575;

    bayertnr_attr.stManual.st3DSelect.hw_btnr_gKalman_en = 0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_gKalman_wgt = 1024;

    bayertnr_attr.stManual.st3DSelect.hw_btnr_wgtCal_mode = 1;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_lpfHi_en = 1;
    bayertnr_attr.stManual.st3DSelect.sw_btnr_lpfHi_sigma = 1.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_lpfLo_en = 1;
    bayertnr_attr.stManual.st3DSelect.sw_btnr_lpfLo_sigma = 1.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_sigmaIdxFilt_en = 1;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_sigmaIdxFilt_mode = 1;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_sigma_scale = 1.0;

    bayertnr_attr.stManual.st3DSelect.hw_btnr_wgtFilt_en = 1;

    bayertnr_attr.stManual.st3DSelect.hw_btnr_mode0LoWgt_scale = 2.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_mode0Base_ratio = 4.0;

    bayertnr_attr.stManual.st3DSelect.hw_btnr_mode1LoWgt_scale = 1.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_mode1LoWgt_offset = 0.5;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_mode1Wgt_offset = 0.5;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_mode1Wgt_minLimit = 0.05;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_mode1Wgt_scale = 1.0;

    bayertnr_attr.stManual.st3DSelect.hw_btnr_loDetection_en = 1;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_loDiffVfilt_en = 1;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_loDiffVfilt_mode = 0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_loDiffHfilt_en = 1;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_loDiffWgtCal_scale = 2.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_loDiffWgtCal_offset = 0.125;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_loDiffFirstLine_scale = 0.125;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_loDiffVfilt_wgt = 0.875;

    bayertnr_attr.stManual.st3DSelect.hw_btnr_sigmaHfilt_en = 1;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_sigmaFirstLine_scale = 1.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_sigmaVfilt_wgt = 0.875;

    bayertnr_attr.stManual.st3DSelect.hw_btnr_loWgtCal_maxLimit = 4;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_loWgtCal_scale = 3.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_loWgtCal_offset = 0.5;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_loWgtHfilt_en = 1;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_loWgtVfilt_wgt = 0.875;

    bayertnr_attr.stManual.st3DSelect.hw_btnr_sigmaHdrS_scale = 1.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_mode0LoWgtHdrS_scale = 1.0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_mode1LoWgtHdrS_scale = 0.7;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_mode1LoWgtHdrS_offset = 0.5;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_mode1WgtHdrS_scale = 1.4;

    bayertnr_attr.stManual.st3DSelect.sw_btnr_loMgeFrame_maxLimit = 32;
    bayertnr_attr.stManual.st3DSelect.sw_btnr_loMgeFrame_minLimit = 0;
    bayertnr_attr.stManual.st3DSelect.sw_btnr_hiMgeFrame_maxLimit = 32;
    bayertnr_attr.stManual.st3DSelect.sw_btnr_hiMgeFrame_minLimit = 1;
    bayertnr_attr.stManual.st3DSelect.sw_btnr_loMgeFrameHdrS_maxLimit = 32;
    bayertnr_attr.stManual.st3DSelect.sw_btnr_loMgeFrameHdrS_minLimit = 0;
    bayertnr_attr.stManual.st3DSelect.sw_btnr_hiMgeFrameHdrS_maxLimit = 32;
    bayertnr_attr.stManual.st3DSelect.sw_btnr_hiMgeFrameHdrS_minLimit = 0;

    bayertnr_attr.stManual.st3DSelect.hw_autoSigCount_en = 1;
    bayertnr_attr.stManual.st3DSelect.hw_autoSigCount_thred = 0.0;
    bayertnr_attr.stManual.st3DSelect.hw_autoSigCountWgt_thred = 0.8;
    bayertnr_attr.stManual.st3DSelect.sw_autoSigCountFilt_wgt = 0.7;
    bayertnr_attr.stManual.st3DSelect.sw_autoSigCountSpnr_en = 1;

    bayertnr_attr.stManual.st3DSelect.hw_btnr_curSpnrHiWgt_minLimit = 0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_preSpnrHiWgt_minLimit = 0;


    bayertnr_attr.stManual.st3DSelect.hw_btnr_gainOut_maxLimit = 1;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_noiseBal_mode = 0;
    bayertnr_attr.stManual.st3DSelect.hw_btnr_noiseBalNr_strg = 1;
    bayertnr_attr.stManual.st3DSelect.sw_curSpnrSpaceWgt_sigma = 25;
    bayertnr_attr.stManual.st3DSelect.sw_iirSpnrSpaceWgt_sigma = 25;
    bayertnr_attr.stManual.st3DSelect.hw_tnrWgtFltCoef[0] = 4;
    bayertnr_attr.stManual.st3DSelect.hw_tnrWgtFltCoef[1] = 2;
    bayertnr_attr.stManual.st3DSelect.hw_tnrWgtFltCoef[2] = 1;
    bayertnr_attr.stManual.st3DSelect.hw_bay3d_lowgt_ctrl = 0;
    bayertnr_attr.stManual.st3DSelect.hw_bay3d_lowgt_offinit = 0;

    ret = rk_aiq_user_api2_abayertnrV30_SetAttrib(ctx, &bayertnr_attr);
    printf("set bayertnr v30 attri manual ret:%d \n\n", ret);

    rk_aiq_bayertnr_attrib_v30_t get_bayertnr_attr;
    get_bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV30_GetAttrib(ctx, &get_bayertnr_attr);
    printf("get abayertnr v30 attri ret:%d done:%d \n\n", ret, get_bayertnr_attr.sync.done);
    return ret;
}

XCamReturn sample_abayertnr_setReg_v2(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_attrib_v2_t bayertnrV2_attr;
    bayertnrV2_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &bayertnrV2_attr);
    bayertnrV2_attr.sync.sync_mode = sync_mode;
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
    get_bayertnrV2_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &get_bayertnrV2_attr);
    printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, get_bayertnrV2_attr.sync.done);

    return ret;
}


XCamReturn sample_abayertnr_setReg_v23(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_attrib_v23_t bayertnr_attr;
    bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV23_GetAttrib(ctx, &bayertnr_attr);
    bayertnr_attr.sync.sync_mode = sync_mode;
    bayertnr_attr.eMode = ABAYERTNRV23_OP_MODE_REG_MANUAL;


    // BAY3D_BAY3D_CTRL 0x2c00
    bayertnr_attr.stManual.st3DFix.soft_st = 0;
    bayertnr_attr.stManual.st3DFix.soft_mode = 0;
    bayertnr_attr.stManual.st3DFix.bwsaving_en = 0;

    bayertnr_attr.stManual.st3DFix.loswitch_protect = 0;
    bayertnr_attr.stManual.st3DFix.glbpk_en = 0;
    bayertnr_attr.stManual.st3DFix.logaus3_bypass_en = 0;
    bayertnr_attr.stManual.st3DFix.logaus5_bypass_en = 0;

    bayertnr_attr.stManual.st3DFix.lomed_bypass_en = 0;
    bayertnr_attr.stManual.st3DFix.hichnsplit_en = 0;
    bayertnr_attr.stManual.st3DFix.hiabs_possel = 1;
    bayertnr_attr.stManual.st3DFix.higaus_bypass_en = 0;

    bayertnr_attr.stManual.st3DFix.himed_bypass_en = 0;
    bayertnr_attr.stManual.st3DFix.lobypass_en = 0;
    bayertnr_attr.stManual.st3DFix.hibypass_en = 0;
    bayertnr_attr.stManual.st3DFix.bypass_en = 0;
    bayertnr_attr.stManual.st3DFix.bay3d_en = 1;

    // BAY3D_BAY3D_KALRATIO 0x2c04
    bayertnr_attr.stManual.st3DFix.softwgt = 0x100;
    bayertnr_attr.stManual.st3DFix.hidif_th = 0xffff;

    // BAY3D_BAY3D_GLBPK2 0x2c08
    bayertnr_attr.stManual.st3DFix.glbpk2 = 0x0000800;

    // BAY3D_BAY3D_CTRL1 0x2c0c
    bayertnr_attr.stManual.st3DFix.hiwgt_opt_en = 0;
    bayertnr_attr.stManual.st3DFix.hichncor_en = 0;
    bayertnr_attr.stManual.st3DFix.bwopt_gain_dis = 0;
    bayertnr_attr.stManual.st3DFix.lo4x8_en = 1;
    bayertnr_attr.stManual.st3DFix.lo4x4_en = 0;

    bayertnr_attr.stManual.st3DFix.hisig_ind_sel = 0;
    bayertnr_attr.stManual.st3DFix.pksig_ind_sel = 0;
    bayertnr_attr.stManual.st3DFix.iirwr_rnd_en = 1;
    bayertnr_attr.stManual.st3DFix.curds_high_en = 0;
    bayertnr_attr.stManual.st3DFix.higaus3_mode = 0;
    bayertnr_attr.stManual.st3DFix.higaus5x5_en = 0;
    bayertnr_attr.stManual.st3DFix.wgtmix_opt_en = 0;

    // BAY3D_BAY3D_WGTLMT 0x2c10
    bayertnr_attr.stManual.st3DFix.wgtlmt = 0x380;
    bayertnr_attr.stManual.st3DFix.wgtratio = 0x000;

    // BAY3D_BAY3D_SIG_X0  0x2c14 - 0x2c30
    bayertnr_attr.stManual.st3DFix.sig0_x[0] = 0x200;
    bayertnr_attr.stManual.st3DFix.sig0_x[1] = 0x400;
    bayertnr_attr.stManual.st3DFix.sig0_x[2] = 0x600;
    bayertnr_attr.stManual.st3DFix.sig0_x[3] = 0x800;
    bayertnr_attr.stManual.st3DFix.sig0_x[4] = 0xc00;
    bayertnr_attr.stManual.st3DFix.sig0_x[5] = 0x1000;
    bayertnr_attr.stManual.st3DFix.sig0_x[6] = 0x1400;
    bayertnr_attr.stManual.st3DFix.sig0_x[7] = 0x1800;
    bayertnr_attr.stManual.st3DFix.sig0_x[8] = 0x1c00;
    bayertnr_attr.stManual.st3DFix.sig0_x[9] = 0x2000;
    bayertnr_attr.stManual.st3DFix.sig0_x[10] = 0x2400;
    bayertnr_attr.stManual.st3DFix.sig0_x[11] = 0x2800;
    bayertnr_attr.stManual.st3DFix.sig0_x[12] = 0x2c00;
    bayertnr_attr.stManual.st3DFix.sig0_x[13] = 0x3000;
    bayertnr_attr.stManual.st3DFix.sig0_x[14] = 0x3400;
    bayertnr_attr.stManual.st3DFix.sig0_x[15] = 0x3800;

    // BAY3D_BAY3D_SIG0_Y0 0x2c34 - 0x2c50
    bayertnr_attr.stManual.st3DFix.sig0_y[0] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[1] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[2] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[3] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[4] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[5] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[6] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[7] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[8] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[9] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[10] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[11] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[12] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[13] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[14] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[15] = 0x0300;

    // BAY3D_BAY3D_SIG_X0  0x2c54 - 0x2c70
    bayertnr_attr.stManual.st3DFix.sig1_x[0] = 0x200;
    bayertnr_attr.stManual.st3DFix.sig1_x[1] = 0x400;
    bayertnr_attr.stManual.st3DFix.sig1_x[2] = 0x600;
    bayertnr_attr.stManual.st3DFix.sig1_x[3] = 0x800;
    bayertnr_attr.stManual.st3DFix.sig1_x[4] = 0xc00;
    bayertnr_attr.stManual.st3DFix.sig1_x[5] = 0x1000;
    bayertnr_attr.stManual.st3DFix.sig1_x[6] = 0x1400;
    bayertnr_attr.stManual.st3DFix.sig1_x[7] = 0x1800;
    bayertnr_attr.stManual.st3DFix.sig1_x[8] = 0x1c00;
    bayertnr_attr.stManual.st3DFix.sig1_x[9] = 0x2000;
    bayertnr_attr.stManual.st3DFix.sig1_x[10] = 0x2400;
    bayertnr_attr.stManual.st3DFix.sig1_x[11] = 0x2800;
    bayertnr_attr.stManual.st3DFix.sig1_x[12] = 0x2c00;
    bayertnr_attr.stManual.st3DFix.sig1_x[13] = 0x3000;
    bayertnr_attr.stManual.st3DFix.sig1_x[14] = 0x3400;
    bayertnr_attr.stManual.st3DFix.sig1_x[15] = 0x3800;

    // ISP_BAYNR_3A00_SIGMAY0-15
    bayertnr_attr.stManual.st3DFix.sig1_y[0] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[1] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[2] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[3] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[4] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[5] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[6] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[7] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[8] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[9] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[10] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[11] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[12] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[13] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[14] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[15] = 0x0400;

    // ISP_BAYNR_3A00_SIGMAY0-15
    bayertnr_attr.stManual.st3DFix.sig2_y[0] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[1] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[2] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[3] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[4] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[5] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[6] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[7] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[8] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[9] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[10] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[11] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[12] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[13] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[14] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[15] = 0x080;


    /* BAY3D_HISIGRAT */
    bayertnr_attr.stManual.st3DFix.hisigrat0 = 0x00;
    bayertnr_attr.stManual.st3DFix.hisigrat1 = 0x00;

    /* BAY3D_HISIGOFF */
    bayertnr_attr.stManual.st3DFix.hisigoff0 = 0x100;
    bayertnr_attr.stManual.st3DFix.hisigoff1 = 0x400;

    /* BAY3D_LOSIG */
    bayertnr_attr.stManual.st3DFix.losigoff = 0x00;
    bayertnr_attr.stManual.st3DFix.losigrat = 0x100;

    /* BAY3D_SIGPK */
    bayertnr_attr.stManual.st3DFix.rgain_off = 0x00;
    bayertnr_attr.stManual.st3DFix.bgain_off = 0x00;

    /* BAY3D_SIGGAUS */
    bayertnr_attr.stManual.st3DFix.siggaus0 = 0x10;
    bayertnr_attr.stManual.st3DFix.siggaus1 = 0x8;
    bayertnr_attr.stManual.st3DFix.siggaus2 = 0x10;
    bayertnr_attr.stManual.st3DFix.siggaus3 = 0x8;


    ret = rk_aiq_user_api2_abayertnrV23_SetAttrib(ctx, &bayertnr_attr);
    printf("set bayertnr v2 attri manual ret:%d \n\n", ret);

    rk_aiq_bayertnr_attrib_v23_t get_bayertnr_attr;
    get_bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV23_GetAttrib(ctx, &get_bayertnr_attr);
    printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, get_bayertnr_attr.sync.done);

    return ret;
}

XCamReturn sample_abayertnr_setReg_v23Lite(const rk_aiq_sys_ctx_t* ctx,
        rk_aiq_uapi_mode_sync_e sync_mode) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_attrib_v23L_t bayertnr_attr;
    bayertnr_attr.sync.sync_mode = sync_mode;
    ret                          = rk_aiq_user_api2_abayertnrV23Lite_GetAttrib(ctx, &bayertnr_attr);
    bayertnr_attr.sync.sync_mode = sync_mode;
    bayertnr_attr.eMode          = ABAYERTNRV23_OP_MODE_REG_MANUAL;

    // BAY3D_BAY3D_CTRL 0x2c00
    bayertnr_attr.stManual.st3DFix.soft_st     = 0;
    bayertnr_attr.stManual.st3DFix.soft_mode   = 0;
    bayertnr_attr.stManual.st3DFix.bwsaving_en = 0;

    bayertnr_attr.stManual.st3DFix.loswitch_protect  = 0;
    bayertnr_attr.stManual.st3DFix.glbpk_en          = 0;
    bayertnr_attr.stManual.st3DFix.logaus3_bypass_en = 0;
    bayertnr_attr.stManual.st3DFix.logaus5_bypass_en = 0;

    bayertnr_attr.stManual.st3DFix.lomed_bypass_en  = 0;
    bayertnr_attr.stManual.st3DFix.hichnsplit_en    = 0;
    bayertnr_attr.stManual.st3DFix.hiabs_possel     = 1;
    bayertnr_attr.stManual.st3DFix.higaus_bypass_en = 0;

    bayertnr_attr.stManual.st3DFix.himed_bypass_en = 0;
    bayertnr_attr.stManual.st3DFix.lobypass_en     = 0;
    bayertnr_attr.stManual.st3DFix.hibypass_en     = 0;
    bayertnr_attr.stManual.st3DFix.bypass_en       = 0;
    bayertnr_attr.stManual.st3DFix.bay3d_en        = 1;

    // BAY3D_BAY3D_KALRATIO 0x2c04
    bayertnr_attr.stManual.st3DFix.softwgt  = 0x100;
    bayertnr_attr.stManual.st3DFix.hidif_th = 0xffff;

    // BAY3D_BAY3D_GLBPK2 0x2c08
    bayertnr_attr.stManual.st3DFix.glbpk2 = 0x0000800;

    // BAY3D_BAY3D_CTRL1 0x2c0c
    bayertnr_attr.stManual.st3DFix.hiwgt_opt_en   = 0;
    bayertnr_attr.stManual.st3DFix.hichncor_en    = 0;
    bayertnr_attr.stManual.st3DFix.bwopt_gain_dis = 0;
    bayertnr_attr.stManual.st3DFix.lo4x8_en       = 1;
    bayertnr_attr.stManual.st3DFix.lo4x4_en       = 0;

    bayertnr_attr.stManual.st3DFix.hisig_ind_sel = 0;
    bayertnr_attr.stManual.st3DFix.pksig_ind_sel = 0;
    bayertnr_attr.stManual.st3DFix.iirwr_rnd_en  = 1;
    bayertnr_attr.stManual.st3DFix.curds_high_en = 0;
    bayertnr_attr.stManual.st3DFix.higaus3_mode  = 0;
    bayertnr_attr.stManual.st3DFix.higaus5x5_en  = 0;
    bayertnr_attr.stManual.st3DFix.wgtmix_opt_en = 0;

    // BAY3D_BAY3D_WGTLMT 0x2c10
    bayertnr_attr.stManual.st3DFix.wgtlmt   = 0x380;
    bayertnr_attr.stManual.st3DFix.wgtratio = 0x000;

    // BAY3D_BAY3D_SIG_X0  0x2c14 - 0x2c30
    bayertnr_attr.stManual.st3DFix.sig0_x[0]  = 0x200;
    bayertnr_attr.stManual.st3DFix.sig0_x[1]  = 0x400;
    bayertnr_attr.stManual.st3DFix.sig0_x[2]  = 0x600;
    bayertnr_attr.stManual.st3DFix.sig0_x[3]  = 0x800;
    bayertnr_attr.stManual.st3DFix.sig0_x[4]  = 0xc00;
    bayertnr_attr.stManual.st3DFix.sig0_x[5]  = 0x1000;
    bayertnr_attr.stManual.st3DFix.sig0_x[6]  = 0x1400;
    bayertnr_attr.stManual.st3DFix.sig0_x[7]  = 0x1800;
    bayertnr_attr.stManual.st3DFix.sig0_x[8]  = 0x1c00;
    bayertnr_attr.stManual.st3DFix.sig0_x[9]  = 0x2000;
    bayertnr_attr.stManual.st3DFix.sig0_x[10] = 0x2400;
    bayertnr_attr.stManual.st3DFix.sig0_x[11] = 0x2800;
    bayertnr_attr.stManual.st3DFix.sig0_x[12] = 0x2c00;
    bayertnr_attr.stManual.st3DFix.sig0_x[13] = 0x3000;
    bayertnr_attr.stManual.st3DFix.sig0_x[14] = 0x3400;
    bayertnr_attr.stManual.st3DFix.sig0_x[15] = 0x3800;

    // BAY3D_BAY3D_SIG0_Y0 0x2c34 - 0x2c50
    bayertnr_attr.stManual.st3DFix.sig0_y[0]  = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[1]  = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[2]  = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[3]  = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[4]  = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[5]  = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[6]  = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[7]  = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[8]  = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[9]  = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[10] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[11] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[12] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[13] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[14] = 0x0300;
    bayertnr_attr.stManual.st3DFix.sig0_y[15] = 0x0300;

    // BAY3D_BAY3D_SIG_X0  0x2c54 - 0x2c70
    bayertnr_attr.stManual.st3DFix.sig1_x[0]  = 0x200;
    bayertnr_attr.stManual.st3DFix.sig1_x[1]  = 0x400;
    bayertnr_attr.stManual.st3DFix.sig1_x[2]  = 0x600;
    bayertnr_attr.stManual.st3DFix.sig1_x[3]  = 0x800;
    bayertnr_attr.stManual.st3DFix.sig1_x[4]  = 0xc00;
    bayertnr_attr.stManual.st3DFix.sig1_x[5]  = 0x1000;
    bayertnr_attr.stManual.st3DFix.sig1_x[6]  = 0x1400;
    bayertnr_attr.stManual.st3DFix.sig1_x[7]  = 0x1800;
    bayertnr_attr.stManual.st3DFix.sig1_x[8]  = 0x1c00;
    bayertnr_attr.stManual.st3DFix.sig1_x[9]  = 0x2000;
    bayertnr_attr.stManual.st3DFix.sig1_x[10] = 0x2400;
    bayertnr_attr.stManual.st3DFix.sig1_x[11] = 0x2800;
    bayertnr_attr.stManual.st3DFix.sig1_x[12] = 0x2c00;
    bayertnr_attr.stManual.st3DFix.sig1_x[13] = 0x3000;
    bayertnr_attr.stManual.st3DFix.sig1_x[14] = 0x3400;
    bayertnr_attr.stManual.st3DFix.sig1_x[15] = 0x3800;

    // ISP_BAYNR_3A00_SIGMAY0-15
    bayertnr_attr.stManual.st3DFix.sig1_y[0]  = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[1]  = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[2]  = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[3]  = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[4]  = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[5]  = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[6]  = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[7]  = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[8]  = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[9]  = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[10] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[11] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[12] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[13] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[14] = 0x0400;
    bayertnr_attr.stManual.st3DFix.sig1_y[15] = 0x0400;

    // ISP_BAYNR_3A00_SIGMAY0-15
    bayertnr_attr.stManual.st3DFix.sig2_y[0]  = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[1]  = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[2]  = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[3]  = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[4]  = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[5]  = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[6]  = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[7]  = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[8]  = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[9]  = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[10] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[11] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[12] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[13] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[14] = 0x080;
    bayertnr_attr.stManual.st3DFix.sig2_y[15] = 0x080;

    /* BAY3D_HISIGRAT */
    bayertnr_attr.stManual.st3DFix.hisigrat0 = 0x00;
    bayertnr_attr.stManual.st3DFix.hisigrat1 = 0x00;

    /* BAY3D_HISIGOFF */
    bayertnr_attr.stManual.st3DFix.hisigoff0 = 0x100;
    bayertnr_attr.stManual.st3DFix.hisigoff1 = 0x400;

    /* BAY3D_LOSIG */
    bayertnr_attr.stManual.st3DFix.losigoff = 0x00;
    bayertnr_attr.stManual.st3DFix.losigrat = 0x100;

    /* BAY3D_SIGPK */
    bayertnr_attr.stManual.st3DFix.rgain_off = 0x00;
    bayertnr_attr.stManual.st3DFix.bgain_off = 0x00;

    /* BAY3D_SIGGAUS */
    bayertnr_attr.stManual.st3DFix.siggaus0 = 0x10;
    bayertnr_attr.stManual.st3DFix.siggaus1 = 0x8;
    bayertnr_attr.stManual.st3DFix.siggaus2 = 0x4;
    bayertnr_attr.stManual.st3DFix.siggaus3 = 0x0;

    ret = rk_aiq_user_api2_abayertnrV23Lite_SetAttrib(ctx, &bayertnr_attr);
    printf("set bayertnr v23Lite attri manual ret:%d \n\n", ret);

    rk_aiq_bayertnr_attrib_v23L_t get_bayertnr_attr;
    get_bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV23Lite_GetAttrib(ctx, &get_bayertnr_attr);
    printf("get abayertnr v23Lite attri ret:%d done:%d \n\n", ret, get_bayertnr_attr.sync.done);

    return ret;
}



XCamReturn sample_abayertnr_setReg_v30(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_attrib_v30_t bayertnr_attr;
    bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV30_GetAttrib(ctx, &bayertnr_attr);
    bayertnr_attr.sync.sync_mode = sync_mode;
    bayertnr_attr.eMode = ABAYERTNRV30_OP_MODE_REG_MANUAL;


    // BAY3D_BAY3D_CTRL 0x2c00
    bayertnr_attr.stManual.st3DFix.bypass_en = 0;
    bayertnr_attr.stManual.st3DFix.iirsparse_en = 0;

    // BAY3D_BAY3D_KALRATIO 0x2c04
    bayertnr_attr.stManual.st3DFix.transf_bypass_en = 1;
    bayertnr_attr.stManual.st3DFix.sigma_curve_double_en = 0;
    bayertnr_attr.stManual.st3DFix.cur_spnr_bypass_en = 0;
    bayertnr_attr.stManual.st3DFix.cur_spnr_sigma_idxfilt_bypass_en = 0;
    bayertnr_attr.stManual.st3DFix.cur_spnr_sigma_curve_double_en = 0;
    bayertnr_attr.stManual.st3DFix.pre_spnr_bypass_en = 0;
    bayertnr_attr.stManual.st3DFix.pre_spnr_sigma_idxfilt_bypass_en = 0;
    bayertnr_attr.stManual.st3DFix.pre_spnr_sigma_curve_double_en = 1;
    bayertnr_attr.stManual.st3DFix.lpf_hi_bypass_en = 0;
    bayertnr_attr.stManual.st3DFix.lo_diff_vfilt_bypass_en = 0;
    bayertnr_attr.stManual.st3DFix.lpf_lo_bypass_en = 0;
    bayertnr_attr.stManual.st3DFix.lo_wgt_hfilt_en = 1;
    bayertnr_attr.stManual.st3DFix.lo_diff_hfilt_en = 1;
    bayertnr_attr.stManual.st3DFix.sig_hfilt_en = 1;
    bayertnr_attr.stManual.st3DFix.gkalman_en = 1;
    bayertnr_attr.stManual.st3DFix.spnr_pre_sigma_use_en = 1;
    bayertnr_attr.stManual.st3DFix.lo_detection_mode = 0;
    bayertnr_attr.stManual.st3DFix.md_wgt_out_en = 0;
    bayertnr_attr.stManual.st3DFix.md_bypass_en = 0;
    bayertnr_attr.stManual.st3DFix.iirspnr_out_en = 0;
    bayertnr_attr.stManual.st3DFix.lomdwgt_dbg_en = 0;
    bayertnr_attr.stManual.st3DFix.curdbg_out_en = 0;
    bayertnr_attr.stManual.st3DFix.noisebal_mode = 0;


    // BAY3D_CTRL2 0x2c08
    bayertnr_attr.stManual.st3DFix.transf_mode = 0;
    bayertnr_attr.stManual.st3DFix.wgt_cal_mode = 1;
    bayertnr_attr.stManual.st3DFix.mge_wgt_ds_mode = 0;
    bayertnr_attr.stManual.st3DFix.kalman_wgt_ds_mode = 0;
    bayertnr_attr.stManual.st3DFix.mge_wgt_hdr_sht_thred = 0x3f;
    bayertnr_attr.stManual.st3DFix.sigma_calc_mge_wgt_hdr_sht_thred = 0x3f;

    // BAY3D_TRANS0 0x2c0c
    bayertnr_attr.stManual.st3DFix.transf_mode_offset = 0x100;
    bayertnr_attr.stManual.st3DFix.transf_mode_scale = 0;
    bayertnr_attr.stManual.st3DFix.itransf_mode_offset = 0x800;

    // BAY3D_TRANS1 0x2c10
    bayertnr_attr.stManual.st3DFix.transf_data_max_limit = 0xfffff;

    // BAY3D_CURDGAIN  0x2c14 - 0x2c30
    bayertnr_attr.stManual.st3DFix.cur_spnr_sigma_hdr_sht_scale = 0x100;
    bayertnr_attr.stManual.st3DFix.cur_spnr_sigma_scale = 0x100;


    /* BAY3D_CURSIG_X */
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_x[0] = 0x100;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_x[1] = 0x200;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_x[2] = 0x300;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_x[3] = 0x400;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_x[4] = 0x500;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_x[5] = 0x600;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_x[6] = 0x700;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_x[7] = 0x800;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_x[8] = 0x900;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_x[9] = 0xa00;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_x[10] = 0xb00;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_x[11] = 0xc00;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_x[12] = 0xd00;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_x[13] = 0xe00;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_x[14] = 0xf00;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_x[15] = 0xfff;

    // BAY3D_BAY3D_SIG0_Y0 0x2c34 - 0x2c50
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_y[0] = 0x80;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_y[1] = 0x80;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_y[2] = 0x80;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_y[3] = 0x80;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_y[4] = 0x80;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_y[5] = 0x80;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_y[6] = 0x80;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_y[7] = 0x80;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_y[8] = 0x80;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_y[9] = 0x80;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_y[10] = 0x80;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_y[11] = 0x80;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_y[12] = 0x80;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_y[13] = 0x80;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_y[14] = 0x80;
    bayertnr_attr.stManual.st3DFix.cur_spnr_luma_sigma_y[15] = 0x80;

    /* BAY3D_CURGAIN_OFF */
    bayertnr_attr.stManual.st3DFix.cur_spnr_sigma_rgain_offset = 0x00;
    bayertnr_attr.stManual.st3DFix.cur_spnr_sigma_bgain_offset = 0x00;

    /* BAY3D_CURSIG_OFF */
    bayertnr_attr.stManual.st3DFix.cur_spnr_sigma_hdr_sht_offset = 0x00;
    bayertnr_attr.stManual.st3DFix.cur_spnr_sigma_offset = 0x00;

    /* BAY3D_CURWTH */
    bayertnr_attr.stManual.st3DFix.cur_spnr_pix_diff_max_limit = 0xfff;
    bayertnr_attr.stManual.st3DFix.cur_spnr_wgt_cal_offset = 0x00;

    /* BAY3D_CURBFALP */
    bayertnr_attr.stManual.st3DFix.cur_spnr_wgt = 0x3ff;
    bayertnr_attr.stManual.st3DFix.pre_spnr_wgt = 0x3ff;

    /* BAY3D_CURWDC */
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_rb_wgt[0] = 0x3ff;
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_rb_wgt[1] = 0x3fc;
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_rb_wgt[2] = 0x3f9;
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_rb_wgt[3] = 0x3f2;
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_rb_wgt[4] = 0x3ef;
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_rb_wgt[5] = 0x3e6;
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_rb_wgt[6] = 0x3d6;
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_rb_wgt[7] = 0x3df;
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_rb_wgt[8] = 0x3e2;

    /* BAY3D_CURWDY */
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_gg_wgt[0] = 0x3ff;
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_gg_wgt[1] = 0x3fc;
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_gg_wgt[2] = 0x3f9;
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_gg_wgt[3] = 0x3f2;
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_gg_wgt[4] = 0x3ef;
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_gg_wgt[5] = 0x3e6;
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_gg_wgt[6] = 0x3d6;
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_gg_wgt[7] = 0x3df;
    bayertnr_attr.stManual.st3DFix.cur_spnr_space_gg_wgt[8] = 0x3e2;

    /* BAY3D_IIRDGAIN */
    bayertnr_attr.stManual.st3DFix.pre_spnr_sigma_hdr_sht_scale = 0x100;
    bayertnr_attr.stManual.st3DFix.pre_spnr_sigma_scale = 0x100;

    /* BAY3D_IIRSIG_X */
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_x[0] = 0x100;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_x[1] = 0x200;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_x[2] = 0x300;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_x[3] = 0x400;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_x[4] = 0x500;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_x[5] = 0x600;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_x[6] = 0x700;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_x[7] = 0x800;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_x[8] = 0x900;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_x[9] = 0xa00;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_x[10] = 0xb00;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_x[11] = 0xc00;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_x[12] = 0xd00;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_x[13] = 0xe00;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_x[14] = 0xf00;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_x[15] = 0xfff;

    /* BAY3D_IIRSIG_Y */
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_y[0] = 0x80;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_y[1] = 0x80;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_y[2] = 0x80;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_y[3] = 0x80;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_y[4] = 0x80;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_y[5] = 0x80;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_y[6] = 0x80;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_y[7] = 0x80;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_y[8] = 0x80;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_y[9] = 0x80;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_y[10] = 0x80;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_y[11] = 0x80;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_y[12] = 0x80;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_y[13] = 0x80;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_y[14] = 0x80;
    bayertnr_attr.stManual.st3DFix.pre_spnr_luma_sigma_y[15] = 0x80;

    /* BAY3D_IIRGAIN_OFF */
    bayertnr_attr.stManual.st3DFix.pre_spnr_sigma_rgain_offset = 0x00;
    bayertnr_attr.stManual.st3DFix.pre_spnr_sigma_bgain_offset = 0x00;

    /* BAY3D_IIRSIG_OFF */
    bayertnr_attr.stManual.st3DFix.pre_spnr_sigma_hdr_sht_offset = 0x00;
    bayertnr_attr.stManual.st3DFix.pre_spnr_sigma_offset = 0x00;

    /* BAY3D_IIRWTH */
    bayertnr_attr.stManual.st3DFix.pre_spnr_pix_diff_max_limit = 0xfff;
    bayertnr_attr.stManual.st3DFix.pre_spnr_wgt_cal_offset = 0x00;

    /* BAY3D_IIRWDC */
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_rb_wgt[0] = 0x3ff;
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_rb_wgt[1] = 0x3fc;
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_rb_wgt[2] = 0x3f9;
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_rb_wgt[3] = 0x3f2;
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_rb_wgt[4] = 0x3ef;
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_rb_wgt[5] = 0x3e6;
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_rb_wgt[6] = 0x3d6;
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_rb_wgt[7] = 0x3df;
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_rb_wgt[8] = 0x3e2;

    /* BAY3D_IIRWDY */
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_gg_wgt[0] = 0x3ff;
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_gg_wgt[1] = 0x3fc;
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_gg_wgt[2] = 0x3f9;
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_gg_wgt[3] = 0x3f2;
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_gg_wgt[4] = 0x3ef;
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_gg_wgt[5] = 0x3e6;
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_gg_wgt[6] = 0x3d6;
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_gg_wgt[7] = 0x3df;
    bayertnr_attr.stManual.st3DFix.pre_spnr_space_gg_wgt[8] = 0x3e2;

    /* BAY3D_BFCOEF */
    bayertnr_attr.stManual.st3DFix.cur_spnr_wgt_cal_scale = 0x100;
    bayertnr_attr.stManual.st3DFix.pre_spnr_wgt_cal_scale = 0x100;


    /* BAY3D_TNRSIG_X */
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[0] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[1] = 0x100;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[2] = 0x180;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[3] = 0x200;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[4] = 0x280;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[5] = 0x300;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[6] = 0x380;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[7] = 0x400;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[8] = 0x480;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[9] = 0x500;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[10] = 0x600;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[11] = 0x700;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[12] = 0x800;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[13] = 0x900;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[14] = 0xa00;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[15] = 0xb00;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[16] = 0xc00;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[17] = 0xd00;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[18] = 0xe00;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_x[19] = 0xfff;



    /* BAY3D_TNRHIW */
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[0] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[1] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[2] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[3] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[4] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[5] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[6] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[7] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[8] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[9] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[10] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[11] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[12] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[13] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[14] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[15] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[16] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[17] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[18] = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_luma_sigma_y[19] = 0x80;

    /* BAY3D_TNRSIG_Y */
    bayertnr_attr.stManual.st3DFix.tnr_lpf_hi_coeff[0] = 0xb2;
    bayertnr_attr.stManual.st3DFix.tnr_lpf_hi_coeff[1] = 0x63;
    bayertnr_attr.stManual.st3DFix.tnr_lpf_hi_coeff[2] = 0x3c;
    bayertnr_attr.stManual.st3DFix.tnr_lpf_hi_coeff[3] = 0x16;
    bayertnr_attr.stManual.st3DFix.tnr_lpf_hi_coeff[4] = 0xd;
    bayertnr_attr.stManual.st3DFix.tnr_lpf_hi_coeff[5] = 0x3;
    bayertnr_attr.stManual.st3DFix.tnr_lpf_hi_coeff[6] = 0x0;
    bayertnr_attr.stManual.st3DFix.tnr_lpf_hi_coeff[7] = 0x1;
    bayertnr_attr.stManual.st3DFix.tnr_lpf_hi_coeff[8] = 0x1;

    /* BAY3D_TNRLOW*/
    bayertnr_attr.stManual.st3DFix.tnr_lpf_lo_coeff[0] = 0xb2;
    bayertnr_attr.stManual.st3DFix.tnr_lpf_lo_coeff[1] = 0x63;
    bayertnr_attr.stManual.st3DFix.tnr_lpf_lo_coeff[2] = 0x3c;
    bayertnr_attr.stManual.st3DFix.tnr_lpf_lo_coeff[3] = 0x16;
    bayertnr_attr.stManual.st3DFix.tnr_lpf_lo_coeff[4] = 0xd;
    bayertnr_attr.stManual.st3DFix.tnr_lpf_lo_coeff[5] = 0x3;
    bayertnr_attr.stManual.st3DFix.tnr_lpf_lo_coeff[6] = 0x0;
    bayertnr_attr.stManual.st3DFix.tnr_lpf_lo_coeff[7] = 0x1;
    bayertnr_attr.stManual.st3DFix.tnr_lpf_lo_coeff[8] = 0x1;


    /* BAY3D_TNRGF3 */
    bayertnr_attr.stManual.st3DFix.tnr_wgt_filt_coeff0 = 0x100;
    bayertnr_attr.stManual.st3DFix.tnr_wgt_filt_coeff1 = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_wgt_filt_coeff2 = 0x40;

    /* BAY3D_TNRSIGSCL */
    bayertnr_attr.stManual.st3DFix.tnr_sigma_scale = 0x400;
    bayertnr_attr.stManual.st3DFix.tnr_sigma_hdr_sht_scale = 0x400;

    /* BAY3D_TNRVIIR */
    bayertnr_attr.stManual.st3DFix.tnr_sig_vfilt_wgt = 0xe;
    bayertnr_attr.stManual.st3DFix.tnr_lo_diff_vfilt_wgt = 0xe;
    bayertnr_attr.stManual.st3DFix.tnr_lo_wgt_vfilt_wgt = 0xe;
    bayertnr_attr.stManual.st3DFix.tnr_sig_first_line_scale = 0x10;
    bayertnr_attr.stManual.st3DFix.tnr_lo_diff_first_line_scale = 0x2;

    /* BAY3D_TNRLFSCL */
    bayertnr_attr.stManual.st3DFix.tnr_lo_wgt_cal_offset = 0x1cc;
    bayertnr_attr.stManual.st3DFix.tnr_lo_wgt_cal_scale = 0xc00;
    /* BAY3D_TNRLFSCLTH */
    bayertnr_attr.stManual.st3DFix.tnr_low_wgt_cal_max_limit = 0x1000;
    bayertnr_attr.stManual.st3DFix.tnr_mode0_base_ratio = 0x1000;

    /* BAY3D_TNRDSWGTSCL */
    bayertnr_attr.stManual.st3DFix.tnr_lo_diff_wgt_cal_offset = 0x80;
    bayertnr_attr.stManual.st3DFix.tnr_lo_diff_wgt_cal_scale = 0x800;

    /* BAY3D_TNRWLSTSCL */
    bayertnr_attr.stManual.st3DFix.tnr_lo_mge_pre_wgt_offset = 0x0;
    bayertnr_attr.stManual.st3DFix.tnr_lo_mge_pre_wgt_scale = 0x800;

    /* BAY3D_TNRWGT0SCL0 */
    bayertnr_attr.stManual.st3DFix.tnr_mode0_lo_wgt_scale = 0x800;
    bayertnr_attr.stManual.st3DFix.tnr_mode0_lo_wgt_hdr_sht_scale = 0x400;

    /* BAY3D_TNRWGT1SCL1 */
    bayertnr_attr.stManual.st3DFix.tnr_mode1_lo_wgt_scale = 0x5a7;
    bayertnr_attr.stManual.st3DFix.tnr_mode1_lo_wgt_hdr_sht_scale = 0x2cc;

    /* BAY3D_TNRWGT1SCL2 */
    bayertnr_attr.stManual.st3DFix.tnr_mode1_wgt_scale = 0x599;
    bayertnr_attr.stManual.st3DFix.tnr_mode1_wgt_hdr_sht_scale = 0x999;

    /* BAY3D_TNRWGTOFF */
    bayertnr_attr.stManual.st3DFix.tnr_mode1_lo_wgt_offset = 0x200;
    bayertnr_attr.stManual.st3DFix.tnr_mode1_lo_wgt_hdr_sht_offset = 0x200;


    /* BAY3D_TNRWGT1OFF */
    bayertnr_attr.stManual.st3DFix.tnr_auto_sigma_count_wgt_thred = 0x333;
    bayertnr_attr.stManual.st3DFix.tnr_mode1_wgt_min_limit = 0x33;
    bayertnr_attr.stManual.st3DFix.tnr_mode1_wgt_offset = 0x2d3;

    /* BAY3D_TNRSIGORG */
    bayertnr_attr.stManual.st3DFix.tnr_out_sigma_sq = 0x3fffffff;

    /* BAY3D_TNRWLO_THL */
    bayertnr_attr.stManual.st3DFix.tnr_lo_wgt_clip_min_limit = 0x0;
    bayertnr_attr.stManual.st3DFix.tnr_lo_wgt_clip_hdr_sht_min_limit = 0x0;

    /* BAY3D_TNRWLO_THH */
    bayertnr_attr.stManual.st3DFix.tnr_lo_wgt_clip_max_limit = 0xf80;
    bayertnr_attr.stManual.st3DFix.tnr_lo_wgt_clip_hdr_sht_max_limit = 0xf80;

    /* BAY3D_TNRWHI_THL */
    bayertnr_attr.stManual.st3DFix.tnr_hi_wgt_clip_min_limit = 0x00;
    bayertnr_attr.stManual.st3DFix.tnr_hi_wgt_clip_hdr_sht_min_limit = 0x00;

    /* BAY3D_TNRKEEP */
    bayertnr_attr.stManual.st3DFix.tnr_hi_wgt_clip_max_limit = 0xf80;
    bayertnr_attr.stManual.st3DFix.tnr_hi_wgt_clip_hdr_sht_max_limit = 0xf80;

    /* BAY3D_TNRWHI_THH */
    bayertnr_attr.stManual.st3DFix.tnr_cur_spnr_hi_wgt_min_limit = 0x26;
    bayertnr_attr.stManual.st3DFix.tnr_pre_spnr_hi_wgt_min_limit = 0x0;

    /* BAY3D_PIXMAX */
    bayertnr_attr.stManual.st3DFix.tnr_pix_max = 0xfff;
    bayertnr_attr.stManual.st3DFix.hw_bay3d_lowgt_ctrl = 0x0;
    bayertnr_attr.stManual.st3DFix.hw_bay3d_lowgt_offinit = 0x0;

    /* BAY3D_SIGNUMTH */
    bayertnr_attr.stManual.st3DFix.tnr_auto_sigma_count_th = 0x0;

    /* BAY3D_TNRMO_STR */
    bayertnr_attr.stManual.st3DFix.tnr_motion_nr_strg = 0x7ff;
    bayertnr_attr.stManual.st3DFix.tnr_gain_max = 0xff;

    ret = rk_aiq_user_api2_abayertnrV30_SetAttrib(ctx, &bayertnr_attr);
    printf("set bayertnr v30 attri manual ret:%d \n\n", ret);

    rk_aiq_bayertnr_attrib_v30_t get_bayertnr_attr;
    get_bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV30_GetAttrib(ctx, &get_bayertnr_attr);
    printf("get abayertnr v30 attri ret:%d done:%d \n\n", ret, get_bayertnr_attr.sync.done);

    return ret;
}

XCamReturn sample_abayertnr_setStrength_v2(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode, float fStrength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_strength_v2_t bayertnrV2Strenght;
    bayertnrV2Strenght.sync.sync_mode = sync_mode;
    bayertnrV2Strenght.percent = fStrength;
    ret = rk_aiq_user_api2_abayertnrV2_SetStrength(ctx, &bayertnrV2Strenght);
    printf("Set abayertnr v2 set streangth ret:%d strength:%f \n\n", ret, bayertnrV2Strenght.percent);

    rk_aiq_bayertnr_strength_v2_t get_bayertnrV2Strenght;
    get_bayertnrV2Strenght.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV2_GetStrength(ctx, &get_bayertnrV2Strenght);
    printf("get abayertnr v2 attri ret:%d strength:%f done:%d\n\n",
           ret, get_bayertnrV2Strenght.percent, get_bayertnrV2Strenght.sync.done);
    return ret;
}

XCamReturn sample_abayertnr_setStrength_v23(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode, float fStrength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_strength_v23_t bayertnrStrength;
    bayertnrStrength.sync.sync_mode = sync_mode;
    bayertnrStrength.percent = fStrength;
    bayertnrStrength.strength_enable = true;
    ret = rk_aiq_user_api2_abayertnrV23_SetStrength(ctx, &bayertnrStrength);
    printf("Set abayertnr v23 set streangth ret:%d strength:%f \n\n", ret, bayertnrStrength.percent);

    rk_aiq_bayertnr_strength_v23_t get_bayertnrStrength;
    get_bayertnrStrength.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV23_GetStrength(ctx, &get_bayertnrStrength);
    printf("get abayertnr v23 attri ret:%d strength:%f done:%d\n\n",
           ret, get_bayertnrStrength.percent, get_bayertnrStrength.sync.done);
    return ret;
}


XCamReturn sample_abayertnr_setStrength_v30(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode, float fStrength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_bayertnr_strength_v30_t bayertnrStrength;
    bayertnrStrength.sync.sync_mode = sync_mode;
    bayertnrStrength.percent = fStrength;
    bayertnrStrength.strength_enable = true;
    ret = rk_aiq_user_api2_abayertnrV30_SetStrength(ctx, &bayertnrStrength);
    printf("Set abayertnr v30 set streangth ret:%d strength:%f \n\n", ret, bayertnrStrength.percent);

    rk_aiq_bayertnr_strength_v30_t get_bayertnrStrength;
    get_bayertnrStrength.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV30_GetStrength(ctx, &get_bayertnrStrength);
    printf("get abayertnr v30 attri ret:%d strength:%f done:%d\n\n",
           ret, get_bayertnrStrength.percent, get_bayertnrStrength.sync.done);
    return ret;
}


XCamReturn sample_abayertnr_setDefault_v2(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode, rk_aiq_bayertnr_attrib_v2_t &default_bayertnrV2_attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    default_bayertnrV2_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV2_SetAttrib(ctx, &default_bayertnrV2_attr);
    printf("set bayertnr v2 default attri ret:%d \n\n", ret);

    rk_aiq_bayertnr_attrib_v2_t get_bayertnrV2_attr;
    get_bayertnrV2_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &get_bayertnrV2_attr);
    printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, get_bayertnrV2_attr.sync.done);
    return ret;
}

XCamReturn sample_abayertnr_setDefault_v23(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode, rk_aiq_bayertnr_attrib_v23_t & default_bayertnr_attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    default_bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV23_SetAttrib(ctx, &default_bayertnr_attr);
    printf("set bayertnr v2 default attri ret:%d \n\n", ret);

    rk_aiq_bayertnr_attrib_v23_t get_bayertnr_attr;
    get_bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV23_GetAttrib(ctx, &get_bayertnr_attr);
    printf("get abayertnr v2 attri ret:%d done:%d \n\n", ret, get_bayertnr_attr.sync.done);
    return ret;
}

XCamReturn sample_abayertnr_setDefault_v23Lite(
    const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode,
    rk_aiq_bayertnr_attrib_v23L_t& default_bayertnr_attr) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    default_bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV23Lite_SetAttrib(ctx, &default_bayertnr_attr);
    printf("set bayertnr v23Lite default attri ret:%d \n\n", ret);

    rk_aiq_bayertnr_attrib_v23L_t get_bayertnr_attr;
    get_bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV23Lite_GetAttrib(ctx, &get_bayertnr_attr);
    printf("get abayertnr v23Lite attri ret:%d done:%d \n\n", ret, get_bayertnr_attr.sync.done);
    return ret;
}

XCamReturn sample_abayertnr_setDefault_v30(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode, rk_aiq_bayertnr_attrib_v30_t & default_bayertnr_attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    default_bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV30_SetAttrib(ctx, &default_bayertnr_attr);
    printf("set bayertnr v30 default attri ret:%d \n\n", ret);

    rk_aiq_bayertnr_attrib_v30_t get_bayertnr_attr;
    get_bayertnr_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_abayertnrV30_GetAttrib(ctx, &get_bayertnr_attr);
    printf("get abayertnr v30 attri ret:%d done:%d \n\n", ret, get_bayertnr_attr.sync.done);
    return ret;
}

#ifdef USE_NEWSTRUCT
static void sample_btnr_test(const rk_aiq_sys_ctx_t* ctx)
{
    // get cur mode
    printf("+++++++ BTNR module test start ++++++++\n");

    btnr_api_attrib_t attr;
    memset(&attr, 0, sizeof(attr));

    rk_aiq_user_api2_btnr_GetAttrib(ctx, &attr);

    printf("btnr attr: opmode:%d, en:%d, bypass:%d\n", attr.opMode, attr.en, attr.bypass);

    if (attr.opMode == RK_AIQ_OP_MODE_AUTO)
        attr.opMode = RK_AIQ_OP_MODE_MANUAL;
    else
        attr.opMode = RK_AIQ_OP_MODE_AUTO;

    // reverse en
    attr.en = !attr.en;

    rk_aiq_user_api2_btnr_SetAttrib(ctx, &attr);

    // wait more than 2 frames
    usleep(90 * 1000);

    btnr_status_t status;
    memset(&status, 0, sizeof(btnr_status_t));

    rk_aiq_user_api2_btnr_QueryStatus(ctx, &status);

    printf("btnr status: opmode:%d, en:%d, bypass:%d\n", status.opMode, status.en, status.bypass);

    if (status.opMode != attr.opMode || status.en != attr.en)
        printf("btnr test failed\n");
    printf("-------- BTNR module test done --------\n");
}
#endif

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
    if (CHECK_ISP_HW_V30()) {
        ret = rk_aiq_user_api2_abayertnrV2_GetAttrib(ctx, &default_bayertnrV2_attr);
        printf("get abayertnr v2 default attri ret:%d \n\n", ret);
    }


    rk_aiq_bayertnr_attrib_v23_t default_bayertnrV23_attr;
    if ( CHECK_ISP_HW_V32()) {
        ret = rk_aiq_user_api2_abayertnrV23_GetAttrib(ctx, &default_bayertnrV23_attr);
        printf("get abayertnr v23 default attri ret:%d \n\n", ret);
    }

    rk_aiq_bayertnr_attrib_v23L_t default_bayertnrV23_attr_lite;
    if (CHECK_ISP_HW_V32_LITE()) {
        ret = rk_aiq_user_api2_abayertnrV23Lite_GetAttrib(ctx, &default_bayertnrV23_attr_lite);
        printf("get abayertnr v23Lite default attri ret:%d \n\n", ret);
    }

    rk_aiq_bayertnr_attrib_v30_t default_bayertnrV30_attr;
    if (CHECK_ISP_HW_V39()) {
        ret = rk_aiq_user_api2_abayertnrV30_GetAttrib(ctx, &default_bayertnrV30_attr);
        printf("get abayertnr v30 default attri ret:%d \n\n", ret);
    }

    do {
        sample_abayertnr_usage ();

        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar ();
        printf ("\n");

        switch (key) {
        case '0':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_getAttr_v2(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            if (CHECK_ISP_HW_V32()) {
                sample_abayertnr_getAttr_v23(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            if (CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_getAttr_v23Lite(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_getAttr_v30(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            break;
        case '1':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_getStrength_v2(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            if (CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_getStrength_v23(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_getStrength_v30(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            break;
        case '2':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_setAuto_v2(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            if (CHECK_ISP_HW_V32()) {
                sample_abayertnr_setAuto_v23(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            if (CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_setAuto_v23Lite(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_setAuto_v30(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            break;
        case '3':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_setManual_v2(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            if (CHECK_ISP_HW_V32()) {
                sample_abayertnr_setManual_v23(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            if (CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_setManual_v23Lite(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_setManual_v30(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            break;
        case '4':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_setReg_v2(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            if (CHECK_ISP_HW_V32()) {
                sample_abayertnr_setReg_v23(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            if (CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_setReg_v23Lite(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_setReg_v30(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            break;
        case '5':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_setStrength_v2(ctx, RK_AIQ_UAPI_MODE_SYNC, 1.0);
            }
            if (CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_setStrength_v23(ctx, RK_AIQ_UAPI_MODE_SYNC, 1.0);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_setStrength_v30(ctx, RK_AIQ_UAPI_MODE_SYNC, 1.0);
            }
            break;
        case '6':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_setStrength_v2(ctx, RK_AIQ_UAPI_MODE_SYNC, 0.0);
            }
            if (CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_setStrength_v23(ctx, RK_AIQ_UAPI_MODE_SYNC, 0.0);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_setStrength_v30(ctx, RK_AIQ_UAPI_MODE_SYNC, 0.0);
            }
            break;
        case '7':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_setStrength_v2(ctx, RK_AIQ_UAPI_MODE_SYNC, 0.5);
            }
            if (CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_setStrength_v23(ctx, RK_AIQ_UAPI_MODE_SYNC, 0.5);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_setStrength_v30(ctx, RK_AIQ_UAPI_MODE_SYNC, 0.5);
            }
            break;
        case '8':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_setDefault_v2(ctx, RK_AIQ_UAPI_MODE_SYNC, default_bayertnrV2_attr);
            }
            if (CHECK_ISP_HW_V32()) {
                sample_abayertnr_setDefault_v23(ctx, RK_AIQ_UAPI_MODE_SYNC, default_bayertnrV23_attr);
            }
            if (CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_setDefault_v23Lite(ctx, RK_AIQ_UAPI_MODE_SYNC,
                                                    default_bayertnrV23_attr_lite);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_setDefault_v30(ctx, RK_AIQ_UAPI_MODE_SYNC, default_bayertnrV30_attr);
            }
            break;
        case 'a':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_getAttr_v2(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            if (CHECK_ISP_HW_V32()) {
                sample_abayertnr_getAttr_v23(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            if (CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_getAttr_v23Lite(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_getAttr_v30(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            break;
        case 'b':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_getStrength_v2(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            if (CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_getStrength_v23(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_getStrength_v30(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            break;
        case 'c':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_setAuto_v2(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            if (CHECK_ISP_HW_V32()) {
                sample_abayertnr_setAuto_v23(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            if (CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_setAuto_v23Lite(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_setAuto_v30(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            break;
        case 'd':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_setManual_v2(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            if (CHECK_ISP_HW_V32()) {
                sample_abayertnr_setManual_v23(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            if (CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_setManual_v23Lite(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_setManual_v30(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            break;
        case 'e':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_setReg_v2(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            if (CHECK_ISP_HW_V32()) {
                sample_abayertnr_setReg_v23(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            if (CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_setReg_v23Lite(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_setReg_v30(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            break;
        case 'f':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_setStrength_v2(ctx, RK_AIQ_UAPI_MODE_ASYNC, 1.0);
            }
            if (CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_setStrength_v23(ctx, RK_AIQ_UAPI_MODE_ASYNC, 1.0);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_setStrength_v30(ctx, RK_AIQ_UAPI_MODE_ASYNC, 1.0);
            }
            break;
        case 'g':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_setStrength_v2(ctx, RK_AIQ_UAPI_MODE_ASYNC, 0.0);
            }
            if (CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_setStrength_v23(ctx, RK_AIQ_UAPI_MODE_ASYNC, 0.0);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_setStrength_v30(ctx, RK_AIQ_UAPI_MODE_ASYNC, 0.0);
            }
            break;
        case 'h':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_setStrength_v2(ctx, RK_AIQ_UAPI_MODE_ASYNC, 0.5);
            }
            if (CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_setStrength_v23(ctx, RK_AIQ_UAPI_MODE_ASYNC, 0.5);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_setStrength_v30(ctx, RK_AIQ_UAPI_MODE_ASYNC, 0.5);
            }
            break;
        case 'i':
            if (CHECK_ISP_HW_V30()) {
                sample_abayertnr_setDefault_v2(ctx, RK_AIQ_UAPI_MODE_ASYNC, default_bayertnrV2_attr);
            }
            if (CHECK_ISP_HW_V32()) {
                sample_abayertnr_setDefault_v23(ctx, RK_AIQ_UAPI_MODE_ASYNC, default_bayertnrV23_attr);
            }
            if (CHECK_ISP_HW_V32_LITE()) {
                sample_abayertnr_setDefault_v23Lite(ctx, RK_AIQ_UAPI_MODE_ASYNC,
                                                    default_bayertnrV23_attr_lite);
            }
            if (CHECK_ISP_HW_V39()) {
                sample_abayertnr_setDefault_v30(ctx, RK_AIQ_UAPI_MODE_ASYNC, default_bayertnrV30_attr);
            }
            break;
#ifdef USE_NEWSTRUCT
        case 'j':
            sample_btnr_test(ctx);
            break;
#endif
        default:
            printf("not support test\n\n");
            break;
        }

    } while (key != 'q' && key != 'Q');

    return ret;
}

