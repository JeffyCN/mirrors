/*
 * Copyright (c) 2022 Rockchip Eletronics Co., Ltd.
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
 */


// #define USE_NEWSTRUCT
#include "sample_comm.h"
#include "sample_cac_module.h"
#include "uAPI2/rk_aiq_user_api2_acac.h"
#include "uAPI2/rk_aiq_user_api2_helper.h"
//#include <string>

static int g_cac_ver = 3;

static void sample_cac_usage() {
    printf("Usage : \n");
    printf("  Module API: \n");
    printf("\t a) CAC:         Set cac Attr & Sync .\n");
    printf("\t b) CAC:         Set cac Attr & Async .\n");
    printf("\t c) CAC:         sample_cac_test. \n");
    printf("\t v) CAC:         Version selection. \n");
    printf("\n");
    printf("\t h) CAC:         help.\n");
    printf("\t q) CAC:         return to main sample screen.\n");

    printf("\n");
    printf("\t please press the key: ");

    return;
}

static void sample_cac_version() {
    printf("Version list: \n");
    printf("\t 1) CAC V03\n");
    printf("\t 2) CAC V10\n");
    printf("\t 3) CAC V11\n");
    printf("\t please select a number: ");
}

void sample_print_cac_info(const void* arg) { printf("enter CAC modult test!\n"); }

/*
******************************
*
* Module level API Sample Func
*
******************************
*/
static void sample_cac_v03_fill_params(rkaiq_cac_v03_api_attr_t* param,
                                       rk_aiq_uapi_mode_sync_e sync) {
    param->sync.sync_mode                   = sync;
    param->op_mode                          = RKAIQ_CAC_API_OPMODE_MANUAL;
    param->enable                           = true;
    param->manual_param.bypass              = 1;
    param->manual_param.AutoHighLightDetect = true;
    param->manual_param.AutoHighLightOffset = 0;
    param->manual_param.FixHighLightBase    = 0;
    param->manual_param.YCompensate         = 0;
    param->manual_param.AutoStrengthU       = 0;
    param->manual_param.AutoStrengthV       = 0;
    param->manual_param.GrayStrengthU       = 0;
    param->manual_param.GrayStrengthV       = 0;
}

static void sample_cac_v10_fill_params(rkaiq_cac_v10_api_attr_t* param,
                                       rk_aiq_uapi_mode_sync_e sync) {
    param->sync.sync_mode                  = sync;
    param->op_mode                         = RKAIQ_CAC_API_OPMODE_MANUAL;
    param->enable                          = true;
    param->manual_param.bypass             = 0;
    param->manual_param.strength_table[0]  = 0.33;
    param->manual_param.strength_table[1]  = 0.33;
    param->manual_param.strength_table[2]  = 0.33;
    param->manual_param.strength_table[3]  = 0.33;
    param->manual_param.strength_table[4]  = 0.33;
    param->manual_param.strength_table[5]  = 0.33;
    param->manual_param.strength_table[6]  = 0.33;
    param->manual_param.strength_table[7]  = 0.33;
    param->manual_param.strength_table[8]  = 0.33;
    param->manual_param.strength_table[9]  = 0.33;
    param->manual_param.strength_table[10] = 0.33;
    param->manual_param.strength_table[11] = 0.33;
    param->manual_param.strength_table[12] = 0.33;
    param->manual_param.strength_table[13] = 0.33;
    param->manual_param.strength_table[14] = 0.33;
    param->manual_param.strength_table[15] = 0.33;
    param->manual_param.strength_table[16] = 0.33;
    param->manual_param.strength_table[17] = 0.33;
    param->manual_param.strength_table[18] = 0.33;
    param->manual_param.strength_table[19] = 0.33;
    param->manual_param.strength_table[20] = 0.33;
    param->manual_param.strength_table[21] = 0.33;
}

static void sample_cac_v11_fill_params(rkaiq_cac_v11_api_attr_t* param,
                                       rk_aiq_uapi_mode_sync_e sync) {
    param->sync.sync_mode                         = sync;
    param->op_mode                                = RKAIQ_CAC_API_OPMODE_MANUAL;
    param->enable                                 = true;
    param->manual_param.bypass                    = 0;
    param->manual_param.strength_table[0]         = 0.33;
    param->manual_param.strength_table[1]         = 0.33;
    param->manual_param.strength_table[2]         = 0.33;
    param->manual_param.strength_table[3]         = 0.33;
    param->manual_param.strength_table[4]         = 0.33;
    param->manual_param.strength_table[5]         = 0.33;
    param->manual_param.strength_table[6]         = 0.33;
    param->manual_param.strength_table[7]         = 0.33;
    param->manual_param.strength_table[8]         = 0.33;
    param->manual_param.strength_table[9]         = 0.33;
    param->manual_param.strength_table[10]        = 0.33;
    param->manual_param.strength_table[11]        = 0.33;
    param->manual_param.strength_table[12]        = 0.33;
    param->manual_param.strength_table[13]        = 0.33;
    param->manual_param.strength_table[14]        = 0.33;
    param->manual_param.strength_table[15]        = 0.33;
    param->manual_param.strength_table[16]        = 0.33;
    param->manual_param.strength_table[17]        = 0.33;
    param->manual_param.strength_table[18]        = 0.33;
    param->manual_param.strength_table[19]        = 0.33;
    param->manual_param.strength_table[20]        = 0.33;
    param->manual_param.strength_table[21]        = 0.33;
    param->manual_param.clip_g_mode               = RKAIQ_CAC_CLIP_G_MODE_MINIMAL;
    param->manual_param.neg_clip0_enable          = true;
    param->manual_param.edge_detect_en            = true;
    param->manual_param.flat_thed_b               = 1.0;
    param->manual_param.flat_thed_r               = 1.0;
    param->manual_param.offset_b                  = 0.0;
    param->manual_param.offset_r                  = 0.0;
    param->manual_param.expo_det_b_en             = true;
    param->manual_param.expo_det_r_en             = true;
    // (hdr ratio * (1 << raw_bits))
    param->manual_param.expo_thed_b = 1 << 10;
    param->manual_param.expo_thed_r = 1 << 10;
    param->manual_param.expo_adj_b  = 1 << 10;
    param->manual_param.expo_adj_r  = 1 << 10;
}

#ifdef USE_NEWSTRUCT
static void sample_cac_tuningtool_test(const rk_aiq_sys_ctx_t* ctx)
{
    char *ret_str = NULL;

    printf(">>> start tuning tool test: op attrib get ...\n");

    const char* json_cac_status_str = " \n\
        [{ \n\
            \"op\":\"get\", \n\
            \"path\": \"/uapi/0/cac_uapi/info\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 0,\"bypass\": 3} \n\
        }]";

    rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx),
                           json_cac_status_str, &ret_str, RKAIQUAPI_OPMODE_GET);

    if (ret_str) {
        printf("cac status json str: %s\n", ret_str);
    }

    printf("  start tuning tool test: op attrib set ...\n");
    const char* json_cac_str = " \n\
        [{ \n\
            \"op\":\"replace\", \n\
            \"path\": \"/uapi/0/cac_uapi/attr\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 1,\"bypass\": 1} \n\
        }]";
    printf("cac json_cmd_str: %s\n", json_cac_str);
    ret_str = NULL;
    rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx),
                           json_cac_str, &ret_str, RKAIQUAPI_OPMODE_SET);

    // wait more than 2 frames
    usleep(90 * 1000);

    cac_status_t status;
    memset(&status, 0, sizeof(cac_status_t));

    rk_aiq_user_api2_cac_QueryStatus(ctx, &status);

    if (status.opMode != RK_AIQ_OP_MODE_MANUAL || status.en != 1 || status.bypass != 1) {
        printf("cac op set_attrib failed !\n");
        printf("cac status: opmode:%d(EXP:%d), en:%d(EXP:%d), bypass:%d(EXP:%d)\n",
               status.opMode, RK_AIQ_OP_MODE_MANUAL, status.en, 1, status.bypass, 1);
    } else {
        printf("cac op set_attrib success !\n");
    }

    printf(">>> tuning tool test done \n");
}

static void get_auto_attr(cac_api_attrib_t* attr) {
    cac_param_auto_t* stAuto = &attr->stAuto;
    for (int i = 0;i < 13;i++) {
    }
}

static void get_manual_attr(cac_api_attrib_t* attr) {
    cac_param_t* stMan = &attr->stMan;
}

int sample_cac_test(const rk_aiq_sys_ctx_t* ctx)
{
    // sample_cac_tuningtool_test(ctx);
    // get cur mode
    printf("+++++++ cac module test start ++++++++\n");

    cac_api_attrib_t attr;
    memset(&attr, 0, sizeof(attr));

    rk_aiq_user_api2_cac_GetAttrib(ctx, &attr);

    printf("cac attr: opmode:%d, en:%d, bypass:%d\n", attr.opMode, attr.en, attr.bypass);

    srand(time(0));
    int rand_num = rand() % 101;

    if (rand_num <70) {
        printf("update cac arrrib!\n");
        if (attr.opMode == RK_AIQ_OP_MODE_AUTO) {
            attr.opMode = RK_AIQ_OP_MODE_MANUAL;
            get_manual_attr(&attr);
        }
        else {
            get_auto_attr(&attr);
            attr.opMode = RK_AIQ_OP_MODE_AUTO;
        }
    }
    else {
        // reverse en
        printf("reverse cac en!\n");
        attr.en = !attr.en;
    }


    rk_aiq_user_api2_cac_SetAttrib(ctx, &attr);

    // wait more than 2 frames
    usleep(90 * 1000);

    cac_status_t status;
    memset(&status, 0, sizeof(cac_status_t));

    rk_aiq_user_api2_cac_QueryStatus(ctx, &status);

    printf("cac status: opmode:%d, en:%d, bypass:%d\n", status.opMode, status.en, status.bypass);

    if (status.opMode != attr.opMode || status.en != attr.en)
        printf("cac test failed\n");
    printf("-------- cac module test done --------\n");

    return 0;
}
#endif

static int sample_cac_setcacAttr(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rkaiq_cac_v03_api_attr_t attr_v03;
    rkaiq_cac_v10_api_attr_t attr_v10;
    rkaiq_cac_v11_api_attr_t attr_v11;

    if (g_cac_ver == 3) {
        memset(&attr_v03, 0, sizeof(rkaiq_cac_v03_api_attr_t));
        ret = rk_aiq_user_api2_acac_v03_GetAttrib(ctx, &attr_v03);
        RKAIQ_SAMPLE_CHECK_RET(ret, "setcacAttr failed in getting acac attrib!");

        sample_cac_v03_fill_params(&attr_v03, sync);

        ret = rk_aiq_user_api2_acac_v03_SetAttrib(ctx, &attr_v03);
        RKAIQ_SAMPLE_CHECK_RET(ret, "set CAC Attr failed!");
    } else if (g_cac_ver == 10) {
        memset(&attr_v10, 0, sizeof(rkaiq_cac_v10_api_attr_t));
        ret = rk_aiq_user_api2_acac_v10_GetAttrib(ctx, &attr_v10);
        RKAIQ_SAMPLE_CHECK_RET(ret, "setcacAttr failed in getting acac attrib!");

        sample_cac_v10_fill_params(&attr_v10, sync);

        ret = rk_aiq_user_api2_acac_v10_SetAttrib(ctx, &attr_v10);
        RKAIQ_SAMPLE_CHECK_RET(ret, "set CAC Attr failed!");
    } else if (g_cac_ver == 11) {
        memset(&attr_v11, 0, sizeof(rkaiq_cac_v11_api_attr_t));
        ret = rk_aiq_user_api2_acac_v11_GetAttrib(ctx, &attr_v11);
        RKAIQ_SAMPLE_CHECK_RET(ret, "setcacAttr failed in getting acac attrib!");

        sample_cac_v11_fill_params(&attr_v11, sync);

        ret = rk_aiq_user_api2_acac_v11_SetAttrib(ctx, &attr_v11);
        RKAIQ_SAMPLE_CHECK_RET(ret, "set CAC Attr failed!");
    }
    printf("set CAC Attr\n\n");

    return 0;
}

static void sample_cac_v03_dump_attr(const rkaiq_cac_v03_api_attr_t* attr) {
    printf("Dump CAC V03 attributes:\n");
    printf("\t sync = %d, done = %d\n", attr->sync.sync_mode, attr->sync.done);
    printf("\t Mode = %s\n",
           (attr->op_mode == RKAIQ_CAC_API_OPMODE_AUTO
                ? "auto"
                : ((attr->op_mode == RKAIQ_CAC_API_OPMODE_MANUAL) ? "manual" : "invalid")));
    if (attr->op_mode == RKAIQ_CAC_API_OPMODE_MANUAL) {
        printf("\t bypass = %d", attr->manual_param.bypass);
    }
}

static void sample_cac_v10_dump_attr(const rkaiq_cac_v10_api_attr_t* attr) {
    printf("Dump CAC V10 attributes:\n");
    printf("\t sync = %d, done = %d\n", attr->sync.sync_mode, attr->sync.done);
    printf("\t Mode = %s\n",
           (attr->op_mode == RKAIQ_CAC_API_OPMODE_AUTO
                ? "auto"
                : ((attr->op_mode == RKAIQ_CAC_API_OPMODE_MANUAL) ? "manual" : "invalid")));
    printf("\t enable = %d\n", attr->enable);
    printf("\t default enable : %d\n", attr->persist_params.enable);
    printf("\t psf path: %s\n", attr->persist_params.psf_path);
    printf("\t psf shift bits: %d\n", attr->persist_params.psf_shift_bits);
    printf("\t center en : %d\n", attr->persist_params.center_en);
    printf("\t center x : %d\n", attr->persist_params.center_x);
    printf("\t center y : %d\n", attr->persist_params.center_y);
    const rkaiq_cac_v10_param_selected_t* param = NULL;
    if (attr->op_mode == RKAIQ_CAC_API_OPMODE_MANUAL) {
        param = &attr->manual_param;
    } else if (attr->op_mode == RKAIQ_CAC_API_OPMODE_MANUAL) {
        param = &attr->auto_params[0];
    } else {
        printf("\t mode invalide\n");
        return;
    }
    printf("\t bypass = %d\n", param->bypass);
    int strength_cnt = sizeof(param->strength_table) / sizeof(param->strength_table[0]);
    for (int i = 0; i < strength_cnt; i++) {
        printf("\t strength %d = %f\n", i, param->strength_table[i]);
    }
}

static void sample_cac_v11_dump_attr(const rkaiq_cac_v11_api_attr_t* attr) {
    printf("Dump CAC V11 attributes:\n");
    printf("\t sync = %d, done = %d\n", attr->sync.sync_mode, attr->sync.done);
    printf("\t Mode = %s\n",
           (attr->op_mode == RKAIQ_CAC_API_OPMODE_AUTO
                ? "auto"
                : ((attr->op_mode == RKAIQ_CAC_API_OPMODE_MANUAL) ? "manual" : "invalid")));
    printf("\t enable = %d\n", attr->enable);
    printf("\t default enable : %d\n", attr->persist_params.enable);
    printf("\t psf path: %s\n", attr->persist_params.psf_path);
    printf("\t psf shift bits: %d\n", attr->persist_params.psf_shift_bits);
    printf("\t center en : %d\n", attr->persist_params.center_en);
    printf("\t center x : %d\n", attr->persist_params.center_x);
    printf("\t center y : %d\n", attr->persist_params.center_y);

    const rkaiq_cac_v11_param_selected_t* param = NULL;
    if (attr->op_mode == RKAIQ_CAC_API_OPMODE_MANUAL) {
        param = &attr->manual_param;
    } else if (attr->op_mode == RKAIQ_CAC_API_OPMODE_MANUAL) {
        param = &attr->auto_params[0];
    } else {
        printf("\t mode invalide\n");
        return;
    }
    printf("\t bypass = %d\n", param->bypass);
    int strength_cnt = sizeof(param->strength_table) / sizeof(param->strength_table[0]);
    for (int i = 0; i < strength_cnt; i++) {
        printf("\t strength %d = %f\n", i, param->strength_table[i]);
    }
    printf("\t clip_g_mode : %d\n", param->clip_g_mode);
    printf("\t neg_clip0_enable : %d\n", param->neg_clip0_enable);
    printf("\t edge_detect_en : %d\n", param->edge_detect_en);
    printf("\t flat_thed_b : %f\n", param->flat_thed_b);
    printf("\t flat_thed_r : %f \n", param->flat_thed_r);
    printf("\t offset_b : %u\n", param->offset_b);
    printf("\t offset_r : %u\n", param->offset_r);
    printf("\t expo_det_b_en : %d\n", param->expo_det_b_en);
    printf("\t expo_det_r_en : %d\n", param->expo_det_r_en);
    printf("\t expo_thed_b : %u\n", param->expo_thed_b);
    printf("\t expo_thed_r : %u\n", param->expo_thed_r);
    printf("\t expo_adj_b : %u\n", param->expo_adj_b);
    printf("\t expo_adj_r : %u\n", param->expo_adj_r);
}

static int sample_cac_getcacAttr(const rk_aiq_sys_ctx_t* ctx) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rkaiq_cac_v03_api_attr_t attr_v03;
    rkaiq_cac_v10_api_attr_t attr_v10;
    rkaiq_cac_v11_api_attr_t attr_v11;

    if (g_cac_ver == 3) {
        memset(&attr_v03, 0, sizeof(rkaiq_cac_v03_api_attr_t));
        ret = rk_aiq_user_api2_acac_v03_GetAttrib(ctx, &attr_v03);
        RKAIQ_SAMPLE_CHECK_RET(ret, "get cac Attr failed!");
        printf("get CAC Attr:\n\n");
        sample_cac_v03_dump_attr(&attr_v03);
    } else if (g_cac_ver == 10) {
        memset(&attr_v10, 0, sizeof(rkaiq_cac_v10_api_attr_t));
        ret = rk_aiq_user_api2_acac_v10_GetAttrib(ctx, &attr_v10);
        RKAIQ_SAMPLE_CHECK_RET(ret, "get cac Attr failed!");
        printf("get CAC Attr:\n\n");
        sample_cac_v10_dump_attr(&attr_v10);
    } else if (g_cac_ver == 11) {
        memset(&attr_v11, 0, sizeof(rkaiq_cac_v11_api_attr_t));
        ret = rk_aiq_user_api2_acac_v11_GetAttrib(ctx, &attr_v11);
        RKAIQ_SAMPLE_CHECK_RET(ret, "get cac Attr failed!");
        printf("get CAC Attr:\n\n");
        sample_cac_v11_dump_attr(&attr_v11);
    }

    return 0;
}

XCamReturn sample_cac_module(const void* arg) {
    int key = -1;
    CLEAR();

    const demo_context_t* demo_ctx = (demo_context_t*)arg;
    const rk_aiq_sys_ctx_t* ctx;
    if (demo_ctx->camGroup) {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->camgroup_ctx);
    } else {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->aiq_ctx);
    }

    if (ctx == NULL) {
        ERR("%s, ctx is nullptr\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    sample_cac_usage();

    do {
        key = getchar();
        while (key == '\n' || key == '\r') key = getchar();
        printf("\n");

        switch (key) {
            case 'h':
                sample_cac_usage();
                CLEAR();
                break;
            case 'v':
                sample_cac_version();
                break;
            case '1':
                g_cac_ver = 3;
                printf("Using CAC V03\n");
                sample_cac_usage();
                break;
            case '2':
                g_cac_ver = 10;
                printf("Using CAC V10\n");
                sample_cac_usage();
                break;
            case '3':
                g_cac_ver = 11;
                printf("Using CAC V11\n");
                sample_cac_usage();
                break;
            case 'a':
                sample_cac_setcacAttr(ctx, RK_AIQ_UAPI_MODE_DEFAULT);
                sample_cac_getcacAttr(ctx);
                break;
            case 'b':
                sample_cac_setcacAttr(ctx, RK_AIQ_UAPI_MODE_ASYNC);
                sample_cac_getcacAttr(ctx);
                usleep(40 * 1000);
                sample_cac_getcacAttr(ctx);
                break;
#ifdef USE_NEWSTRUCT
            case 'c':
                sample_cac_test(ctx);
                break;
#endif
            default:
                CLEAR();
                sample_cac_usage();
                break;
        }
    } while (key != 'q' && key != 'Q');

    return XCAM_RETURN_NO_ERROR;
}
