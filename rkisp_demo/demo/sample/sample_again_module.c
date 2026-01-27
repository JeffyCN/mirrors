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

#include "uAPI2/rk_aiq_user_api2_helper.h"
//#include <string>

#define RK_AGAIN_GAIN2DDR_MODE_4X8      0
#define RK_AGAIN_GAIN2DDR_MODE_2X8      1
#define RK_AGAIN_GAIN2DDR_MODE_1X8      2

static void sample_again_usage()
{
    printf("Usage : \n");
    printf("\t 0) AGAIN:         get again attri on sync mode.\n");
    printf("\t 1) AGAIN:         set again attri auto on sync mode.\n");
    printf("\t 2) AGAIN:         set again attri manual on sync mode.\n");
    printf("\t 3) AGAIN:         set again attri to default vaule on sync mode.\n");
    printf("\t 4) AGAIN:         get again attri on async mode.\n");
    printf("\t 5) AGAIN:         set again attri auto on async mode.\n");
    printf("\t 6) AGAIN:         set again attri manual on async mode.\n");
    printf("\t 7) AGAIN:         set again attri to default vaule on async mode.\n");
    printf("\t 8) AGAIN:         set local gain write mode 4*8.\n");
    printf("\t 9) AGAIN:         set local gain write mode 2*8.\n");
    printf("\t a) AGAIN:         set local gain write mode 1*8.\n");
    printf("\t b) AGAIN:         newstruct test.\n");
    printf("\t q) AGAIN:         press key q or Q to quit.\n");

}

void sample_print_again_info(const void *arg)
{
    printf ("enter AGAIN modult test!\n");
}

XCamReturn sample_again_getAttr_v2 (const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_gain_attrib_v2_t gainV2_attr;
    gainV2_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_againV2_GetAttrib(ctx, &gainV2_attr);
    printf("get again v2 attri ret:%d done:%d\n\n", ret, gainV2_attr.sync.done);

    return ret;
}

XCamReturn sample_again_setAuto_v2 (const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_gain_attrib_v2_t gainV2_attr;
    gainV2_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_againV2_GetAttrib(ctx, &gainV2_attr);
    gainV2_attr.sync.sync_mode = sync_mode;
    gainV2_attr.eMode = AGAINV2_OP_MODE_AUTO;
    gainV2_attr.stAuto.stParams.hdrgain_ctrl_enable = 1;
    for(int i = 0; i < RK_GAIN_V2_MAX_ISO_NUM; i++) {
        gainV2_attr.stAuto.stParams.iso[i] = 50 * pow(2, i);

        gainV2_attr.stAuto.stParams.iso_params[i].hdr_gain_scale_s = 1.0;
        gainV2_attr.stAuto.stParams.iso_params[i].hdr_gain_scale_m = 1.0;
    }

    ret = rk_aiq_user_api2_againV2_SetAttrib(ctx, &gainV2_attr);
    printf("set again attri auto ret:%d \n\n", ret);

    rk_aiq_gain_attrib_v2_t get_gainV2_attr;
    get_gainV2_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_againV2_GetAttrib(ctx, &get_gainV2_attr);
    printf("get again v2 attri ret:%d done:%d\n\n", ret, get_gainV2_attr.sync.done);
    return ret;
}

XCamReturn sample_again_setManual_v2 (const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_gain_attrib_v2_t gainV2_attr;
    gainV2_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_againV2_GetAttrib(ctx, &gainV2_attr);
    gainV2_attr.sync.sync_mode = sync_mode;
    gainV2_attr.eMode = AGAINV2_OP_MODE_MANUAL;
    gainV2_attr.stManual.stSelect.hdrgain_ctrl_enable = 1;

    gainV2_attr.stManual.stSelect.hdr_gain_scale_s = 1.0;
    gainV2_attr.stManual.stSelect.hdr_gain_scale_m = 1.0;

    ret = rk_aiq_user_api2_againV2_SetAttrib(ctx, &gainV2_attr);
    printf("set gain v2 attri manual ret:%d \n\n", ret);

    rk_aiq_gain_attrib_v2_t get_gainV2_attr;
    get_gainV2_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_againV2_GetAttrib(ctx, &get_gainV2_attr);
    printf("get again v2 attri ret:%d done:%d\n\n", ret, get_gainV2_attr.sync.done);

    return ret;
}

XCamReturn sample_again_setDefault_v2 (const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode, rk_aiq_gain_attrib_v2_t default_gainV2_attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    default_gainV2_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_againV2_SetAttrib(ctx, &default_gainV2_attr);
    printf("Set again v2 set default attri ret:%d \n\n", ret);

    rk_aiq_gain_attrib_v2_t get_gainV2_attr;
    get_gainV2_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_againV2_GetAttrib(ctx, &get_gainV2_attr);
    printf("get again v2 attri ret:%d done:%d\n\n", ret, get_gainV2_attr.sync.done);

    return ret;
}

XCamReturn sample_agin_WriteLocalGain(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode, int mode)
{
    rk_aiq_uapiV2_again_wrtIn_attr_t attr;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    attr.call_cnt = 1;
    attr.enable = true;
    attr.mode = mode;
    sprintf(attr.path, "/tmp");
    ret = rk_aiq_user_api2_againV2_WriteInput(ctx, &attr, sync_mode);

    if (!ret) {
        if (mode == RK_AGAIN_GAIN2DDR_MODE_4X8)
            printf("Write local gain input 4X8_mode \n\n");
        if (mode == RK_AGAIN_GAIN2DDR_MODE_2X8)
            printf("Write local gain input 2X8_mode \n\n");
        if (mode == RK_AGAIN_GAIN2DDR_MODE_1X8)
            printf("Write local gain input 1X8_mode \n\n");
    }

    return ret;
}

#ifdef USE_NEWSTRUCT
static void sample_gain_tuningtool_test(const rk_aiq_sys_ctx_t* ctx)
{
    char *ret_str = NULL;

    printf(">>> start tuning tool test: op attrib get ...\n");

    const char* json_gain_status_str = " \n\
        [{ \n\
            \"op\":\"get\", \n\
            \"path\": \"/uapi/0/gain_uapi/info\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 0,\"bypass\": 3} \n\
        }]";

    rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx),
                           json_gain_status_str, &ret_str, RKAIQUAPI_OPMODE_GET);

    if (ret_str) {
        printf("gain status json str: %s\n", ret_str);
    }

    printf("  start tuning tool test: op attrib set ...\n");
    const char* json_gain_str = " \n\
        [{ \n\
            \"op\":\"replace\", \n\
            \"path\": \"/uapi/0/gain_uapi/attr\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 1,\"bypass\": 1} \n\
        }]";
    printf("gain json_cmd_str: %s\n", json_gain_str);
    ret_str = NULL;
    rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx),
                           json_gain_str, &ret_str, RKAIQUAPI_OPMODE_SET);

    // wait more than 2 frames
    usleep(90 * 1000);

    gain_status_t status;
    memset(&status, 0, sizeof(gain_status_t));

    rk_aiq_user_api2_gain_QueryStatus(ctx, &status);

    if (status.opMode != RK_AIQ_OP_MODE_MANUAL || status.en != 1 || status.bypass != 1) {
        printf("gain op set_attrib failed !\n");
        printf("gain status: opmode:%d(EXP:%d), en:%d(EXP:%d), bypass:%d(EXP:%d)\n",
               status.opMode, RK_AIQ_OP_MODE_MANUAL, status.en, 1, status.bypass, 1);
    } else {
        printf("gain op set_attrib success !\n");
    }

    printf(">>> tuning tool test done \n");
}

static void get_auto_attr(gain_api_attrib_t* attr) {
    gain_param_auto_t* stAuto = &attr->stAuto;
    for (int i = 0;i < 13;i++) {
    }
}

static void get_manual_attr(gain_api_attrib_t* attr) {
    gain_param_t* stMan = &attr->stMan;
}

int sample_gain_test(const rk_aiq_sys_ctx_t* ctx)
{
    // sample_gain_tuningtool_test(ctx);
    // get cur mode
    printf("+++++++ gain module test start ++++++++\n");

    gain_api_attrib_t attr;
    memset(&attr, 0, sizeof(attr));

    rk_aiq_user_api2_gain_GetAttrib(ctx, &attr);

    printf("gain attr: opmode:%d, en:%d, bypass:%d\n", attr.opMode, attr.en, attr.bypass);

    srand(time(0));
    int rand_num = rand() % 101;

    if (rand_num <70) {
        printf("update gain arrrib!\n");
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
        printf("reverse gain en!\n");
        attr.en = !attr.en;
    }

    rk_aiq_user_api2_gain_SetAttrib(ctx, &attr);

    // wait more than 2 frames
    usleep(90 * 1000);

    gain_status_t status;
    memset(&status, 0, sizeof(gain_status_t));

    rk_aiq_user_api2_gain_QueryStatus(ctx, &status);

    printf("gain status: opmode:%d, en:%d, bypass:%d\n", status.opMode, status.en, status.bypass);

    if (status.opMode != attr.opMode || status.en != attr.en)
        printf("gain test failed\n");
    printf("-------- gain module test done --------\n");

    return 0;
}
#endif

XCamReturn sample_again_module(const void* arg)
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

    if (ctx == NULL) {
        ERR ("%s, ctx is nullptr\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    rk_aiq_gain_attrib_v2_t default_gainV2_attr;
    if (CHECK_ISP_HW_V30() || CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        ret = rk_aiq_user_api2_againV2_GetAttrib(ctx, &default_gainV2_attr);
        printf("get again v2 default attri ret:%d \n\n", ret);
    }

    do {
        sample_again_usage ();

        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar ();
        printf ("\n");

        switch (key) {
        case '0':
            if (CHECK_ISP_HW_V30() || CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_again_getAttr_v2(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            break;
        case '1':
            if (CHECK_ISP_HW_V30() || CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_again_setAuto_v2(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            break;
        case '2':
            if (CHECK_ISP_HW_V30() || CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_again_setManual_v2(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            break;
        case '3':
            if (CHECK_ISP_HW_V30() || CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_again_setDefault_v2(ctx, RK_AIQ_UAPI_MODE_SYNC, default_gainV2_attr);
            }
            break;
        case '4':
            if (CHECK_ISP_HW_V30() || CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_again_getAttr_v2(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            break;
        case '5':
            if (CHECK_ISP_HW_V30() || CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_again_setAuto_v2(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            break;
        case '6':
            if (CHECK_ISP_HW_V30() || CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_again_setManual_v2(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            break;
        case '7':
            if (CHECK_ISP_HW_V30() || CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_again_setDefault_v2(ctx, RK_AIQ_UAPI_MODE_ASYNC, default_gainV2_attr);
            }
            break;
        case '8':
            if (CHECK_ISP_HW_V32()) {
                sample_agin_WriteLocalGain(ctx, RK_AIQ_UAPI_MODE_SYNC, RK_AGAIN_GAIN2DDR_MODE_4X8);
            }
            break;
        case '9':
            if (CHECK_ISP_HW_V32()) {
                sample_agin_WriteLocalGain(ctx, RK_AIQ_UAPI_MODE_SYNC, RK_AGAIN_GAIN2DDR_MODE_2X8);
            }
            break;
        case 'a':
            if (CHECK_ISP_HW_V32()) {
                sample_agin_WriteLocalGain(ctx, RK_AIQ_UAPI_MODE_SYNC, RK_AGAIN_GAIN2DDR_MODE_1X8);
            }
            break;
#ifdef USE_NEWSTRUCT
        case 'b':
            sample_gain_test(ctx);
            break;
#endif
        default:
            printf("not support test\n\n");
            break;
        }

    } while (key != 'q' && key != 'Q');

    return ret;
}

