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

#include "sample_comm.h"
#include "uAPI2/rk_aiq_user_api2_agic.h"
#include "sample_agic_module.h"
#include "uAPI2/rk_aiq_user_api2_helper.h"
//#include <string>

static void sample_agic_usage() {
    printf("Usage : \n");
    printf("  Module API: \n");
    printf("\t a) GIC:         Set gic Attr & Sync .\n");
    printf("\t b) GIC:         Set gic Attr & Async .\n");
    printf("\t c) GIC:         GIC new struct test.\n");
    printf("\n");
    printf("\t h) GIC:         help.\n");
    printf("\t q) GIC:         return to main sample screen.\n");

    printf("\n");
    printf("\t please press the key: ");

    return;
}

void sample_print_agic_info(const void* arg) { printf("enter GIC modult test!\n"); }

/*
******************************
*
* Module level API Sample Func
*
******************************
*/

static int sample_gic_setgicAttr(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rkaiq_gic_v2_api_attr_t attr;

    memset(&attr, 0, sizeof(rkaiq_gic_v2_api_attr_t));
    ret = rk_aiq_user_api2_agic_v2_GetAttrib(ctx, &attr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "setgicAttr failed in getting agic attrib!");

    memcpy(&attr.manual_param, &attr.auto_params[0], sizeof(rkaiq_gic_v2_param_selected_t));
    attr.sync.sync_mode = sync;
    attr.op_mode        = RKAIQ_GIC_API_OPMODE_MANUAL;
    attr.gic_en         = 0;
    attr.manual_param.globalStrength = 0.5;

    ret = rk_aiq_user_api2_agic_v2_SetAttrib(ctx, &attr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "set GIC Attr failed!");
    printf("set GIC Attr\n\n");

    return 0;
}

static int sample_gic_getgicAttr(const rk_aiq_sys_ctx_t* ctx) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rkaiq_gic_v2_api_attr_t attr;

    memset(&attr, 0, sizeof(rkaiq_gic_v2_api_attr_t));
    ret = rk_aiq_user_api2_agic_v2_GetAttrib(ctx, &attr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "get gic Attr failed!");
    printf("get GIC Attr:\n\n");
    printf("\t sync = %d, done = %d\n", attr.sync.sync_mode, attr.sync.done);
    printf("\t Mode = %s\n",
           (attr.op_mode == RKAIQ_GIC_API_OPMODE_AUTO
                ? "auto"
                : ((attr.op_mode == RKAIQ_GIC_API_OPMODE_MANUAL) ? "manual" : "off")));
    printf("\t globalStrength = %f\n", attr.manual_param.globalStrength);

    return 0;
}

#ifdef USE_NEWSTRUCT
static void sample_gic_tuningtool_test(const rk_aiq_sys_ctx_t* ctx)
{
    char *ret_str = NULL;

    printf(">>> start tuning tool test: op attrib get ...\n");

    const char* json_agic_status_str = " \n\
        [{ \n\
            \"op\":\"get\", \n\
            \"path\": \"/uapi/0/gic_uapi/info\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 0,\"bypass\": 3} \n\
        }]";

    rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx),
                           json_agic_status_str, &ret_str, RKAIQUAPI_OPMODE_GET);

    if (ret_str) {
        printf("agic status json str: %s\n", ret_str);
    }

    printf("  start tuning tool test: op attrib set ...\n");
    const char* json_agic_str = " \n\
        [{ \n\
            \"op\":\"replace\", \n\
            \"path\": \"/uapi/0/gic_uapi/attr\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 1,\"bypass\": 1} \n\
        }]";
    printf("agic json_cmd_str: %s\n", json_agic_str);
    ret_str = NULL;
    rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx),
                           json_agic_str, &ret_str, RKAIQUAPI_OPMODE_SET);

    // wait more than 2 frames
    usleep(90 * 1000);

    gic_status_t status;
    memset(&status, 0, sizeof(gic_status_t));

    rk_aiq_user_api2_gic_QueryStatus(ctx, &status);

    if (status.opMode != RK_AIQ_OP_MODE_MANUAL || status.en != 1 || status.bypass != 1) {
        printf("agic op set_attrib failed !\n");
        printf("agic status: opmode:%d(EXP:%d), en:%d(EXP:%d), bypass:%d(EXP:%d)\n",
               status.opMode, RK_AIQ_OP_MODE_MANUAL, status.en, 1, status.bypass, 1);
    } else {
        printf("agic op set_attrib success !\n");
    }

    printf(">>> tuning tool test done \n");
}

static void get_auto_attr(gic_api_attrib_t* attr) {
    gic_param_auto_t* stAuto = &attr->stAuto;
    for (int i = 0;i < 13;i++) {
    }
}

static void get_manual_attr(gic_api_attrib_t* attr) {
    gic_param_t* stMan = &attr->stMan;
}

void sample_gic_test(const rk_aiq_sys_ctx_t* ctx) {
    // sample_gic_tuningtool_test(ctx);
    // get cur mode
    printf("+++++++ GIC module test start ++++++++\n");

    gic_api_attrib_t attr;
    memset(&attr, 0, sizeof(attr));

    rk_aiq_user_api2_gic_GetAttrib(ctx, &attr);

    printf("gic attr: opmode:%d, en:%d, bypass:%d\n", attr.opMode, attr.en, attr.bypass);

    srand(time(0));
    int rand_num = rand() % 101;

    if (rand_num <70) {
        printf("update gic arrrib!\n");
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
        printf("reverse gic en!\n");
        attr.en = !attr.en;
    }

    rk_aiq_user_api2_gic_SetAttrib(ctx, &attr);

    // wait more than 2 frames
    usleep(90 * 1000);

    gic_status_t status;
    memset(&status, 0, sizeof(gic_status_t));

    rk_aiq_user_api2_gic_QueryStatus(ctx, &status);

    printf("gic status: opmode:%d, en:%d, bypass:%d\n", status.opMode, status.en, status.bypass);

    if (status.opMode != attr.opMode || status.en != attr.en)
        printf("gic test failed\n");
    printf("-------- gic module test done --------\n");
}
#endif

XCamReturn sample_agic_module(const void* arg) {
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

    sample_agic_usage();

    do {
        key = getchar();
        while (key == '\n' || key == '\r') key = getchar();
        printf("\n");

        switch (key) {
            case 'h':
                sample_agic_usage();
                CLEAR();
                break;
            case 'a':
                sample_gic_setgicAttr(ctx, RK_AIQ_UAPI_MODE_DEFAULT);
                sample_gic_getgicAttr(ctx);
                break;
            case 'b':
                sample_gic_setgicAttr(ctx, RK_AIQ_UAPI_MODE_ASYNC);
                sample_gic_getgicAttr(ctx);
                usleep(40 * 1000);
                sample_gic_getgicAttr(ctx);
                break;
#ifdef USE_NEWSTRUCT
            case 'c':
                sample_gic_test(ctx);
                break;
#endif
            default:
                CLEAR();
                sample_agic_usage();
                break;
        }
    } while (key != 'q' && key != 'Q');

    return XCAM_RETURN_NO_ERROR;
}
