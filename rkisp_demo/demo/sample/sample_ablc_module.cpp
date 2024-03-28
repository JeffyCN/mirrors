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

#ifdef USE_NEWSTRUCT
#ifdef ISP_HW_V39
#include "rk_aiq_user_api2_rk3576.h"
#elif  defined(ISP_HW_V32)
#include "rk_aiq_user_api2_rv1106.h"
#endif
#endif

#include "uAPI2/rk_aiq_user_api2_helper.h"
#include <string>

static void sample_ablc_usage()
{
    printf("Usage : \n");
    printf("\t 0) ABLC:         get ablc attri on sync mode.\n");
    printf("\t 1) ABLC:         set ablc attri auto on sync mode.\n");
    printf("\t 2) ABLC:         set ablc attri manual on sync mode.\n");
    printf("\t 3) ABLC:         set ablc attri to default value on sync mode.\n");
    printf("\t 4) ABLC:         get ablc attri on async mode.\n");
    printf("\t 5) ABLC:         set ablc attri auto on async mode.\n");
    printf("\t 6) ABLC:         set ablc attri manual on async mode.\n");
    printf("\t 7) ABLC:         set ablc attri to default value on async mode.\n");
    printf("\t q) ABLC:         press key q or Q to quit.\n");
}

void sample_print_ablc_info(const void *arg)
{
    printf ("enter ABLC modult test!\n");
}


XCamReturn sample_ablc_getAttr(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_blc_attrib_t blc_attr;
    blc_attr.sync.sync_mode = sync_mode;
    memset(&blc_attr, 0x00, sizeof(blc_attr));//important, need init first
    ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &blc_attr);
    printf("get ablc attri ret:%d done:%d \n\n", ret, blc_attr.sync.done);

    return ret;
}

XCamReturn sample_ablc_getAttr_v32(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_blc_attrib_V32_t blc_attr;
    blc_attr.sync.sync_mode = sync_mode;
    memset(&blc_attr, 0x00, sizeof(blc_attr));//important, need init first
    ret = rk_aiq_user_api2_ablcV32_GetAttrib(ctx, &blc_attr);
    printf("get ablc attri ret:%d done:%d \n\n", ret, blc_attr.sync.done);

    return ret;
}

XCamReturn sample_ablc_setAuto(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_blc_attrib_t blc_attr;
    blc_attr.sync.sync_mode = sync_mode;
    memset(&blc_attr, 0x00, sizeof(blc_attr));//important, need init first
    ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &blc_attr);
    blc_attr.sync.sync_mode = sync_mode;
    blc_attr.eMode = ABLC_OP_MODE_AUTO;
    blc_attr.stBlc0Auto.enable = 1;
    for(int i = 0; i < ABLC_MAX_ISO_LEVEL; i++) {
        blc_attr.stBlc0Auto.iso[i] = 50 * pow(2, i);
        blc_attr.stBlc0Auto.blc_r[i] = 255;
        blc_attr.stBlc0Auto.blc_gr[i] = 255;
        blc_attr.stBlc0Auto.blc_gb[i] = 255;
        blc_attr.stBlc0Auto.blc_b[i] = 255;
    }

    blc_attr.stBlc1Auto.enable = 1;
    for(int i = 0; i < ABLC_MAX_ISO_LEVEL; i++) {
        blc_attr.stBlc1Auto.iso[i] = 50 * pow(2, i);
        blc_attr.stBlc1Auto.blc_r[i] = 254;
        blc_attr.stBlc1Auto.blc_gr[i] = 254;
        blc_attr.stBlc1Auto.blc_gb[i] = 254;
        blc_attr.stBlc1Auto.blc_b[i] = 254;
    }

    ret = rk_aiq_user_api2_ablc_SetAttrib(ctx, &blc_attr);
    printf("set ablc attri auto ret:%d \n\n", ret);

    rk_aiq_blc_attrib_t get_blc_attr;
    get_blc_attr.sync.sync_mode = sync_mode;
    memset(&get_blc_attr, 0x00, sizeof(get_blc_attr));//important, need init first
    ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &get_blc_attr);
    printf("get ablc attri ret:%d done:%d \n\n", ret, get_blc_attr.sync.done);


    return ret;

}

XCamReturn sample_ablc_setAuto_v32(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_blc_attrib_V32_t blc_attr;
    blc_attr.sync.sync_mode = sync_mode;
    memset(&blc_attr, 0x00, sizeof(blc_attr));//important, need init first
    ret = rk_aiq_user_api2_ablcV32_GetAttrib(ctx, &blc_attr);
    blc_attr.sync.sync_mode = sync_mode;
    blc_attr.eMode = ABLC_V32_OP_MODE_AUTO;
    blc_attr.stBlc0Auto.enable = 1;
    for(int i = 0; i < ABLCV32_MAX_ISO_LEVEL; i++) {
        blc_attr.stBlc0Auto.iso[i] = 50 * pow(2, i);
        blc_attr.stBlc0Auto.blc_r[i] = 255;
        blc_attr.stBlc0Auto.blc_gr[i] = 255;
        blc_attr.stBlc0Auto.blc_gb[i] = 255;
        blc_attr.stBlc0Auto.blc_b[i] = 255;
    }

    blc_attr.stBlc1Auto.enable = 0;
    for(int i = 0; i < ABLCV32_MAX_ISO_LEVEL; i++) {
        blc_attr.stBlc1Auto.iso[i] = 50 * pow(2, i);
        blc_attr.stBlc1Auto.blc_r[i] = 254;
        blc_attr.stBlc1Auto.blc_gr[i] = 254;
        blc_attr.stBlc1Auto.blc_gb[i] = 254;
        blc_attr.stBlc1Auto.blc_b[i] = 254;
    }

    blc_attr.stBlcOBAuto.enable = 1;
    for(int i = 0; i < ABLCV32_MAX_ISO_LEVEL; i++) {
        blc_attr.stBlcOBAuto.iso[i] = 50 * pow(2, i);
        blc_attr.stBlcOBAuto.ob_offset[i] = 0;
        blc_attr.stBlcOBAuto.ob_predgain[i] = 1;
    }

    ret = rk_aiq_user_api2_ablcV32_SetAttrib(ctx, &blc_attr);
    printf("set ablc attri auto ret:%d \n\n", ret);

    rk_aiq_blc_attrib_V32_t get_blc_attr;
    get_blc_attr.sync.sync_mode = sync_mode;
    memset(&get_blc_attr, 0x00, sizeof(get_blc_attr));//important, need init first
    ret = rk_aiq_user_api2_ablcV32_GetAttrib(ctx, &get_blc_attr);
    printf("get ablc attri ret:%d done:%d \n\n", ret, get_blc_attr.sync.done);

    return ret;

}


XCamReturn sample_ablc_setManual(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_blc_attrib_t blc_attr;
    memset(&blc_attr, 0x00, sizeof(blc_attr));//important, need init first
    blc_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &blc_attr);
    blc_attr.sync.sync_mode = sync_mode;
    blc_attr.eMode = ABLC_OP_MODE_MANUAL;
    blc_attr.stBlc0Manual.enable = 1;
    blc_attr.stBlc0Manual.blc_r = 253;
    blc_attr.stBlc0Manual.blc_gr = 253;
    blc_attr.stBlc0Manual.blc_gb = 253;
    blc_attr.stBlc0Manual.blc_b = 253;

    blc_attr.stBlc1Manual.enable = 1;
    blc_attr.stBlc1Manual.blc_r = 253;
    blc_attr.stBlc1Manual.blc_gr = 253;
    blc_attr.stBlc1Manual.blc_gb = 253;
    blc_attr.stBlc1Manual.blc_b = 253;


    ret = rk_aiq_user_api2_ablc_SetAttrib(ctx, &blc_attr);
    printf("set blc attri manual ret:%d \n\n", ret);


    rk_aiq_blc_attrib_t get_blc_attr;
    get_blc_attr.sync.sync_mode = sync_mode;
    memset(&get_blc_attr, 0x00, sizeof(get_blc_attr));//important, need init first
    ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &get_blc_attr);
    printf("get ablc attri ret:%d done:%d \n\n", ret, get_blc_attr.sync.done);

    sleep(1);

    return ret;

}


XCamReturn sample_ablc_setManual_v32(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_blc_attrib_V32_t blc_attr;
    memset(&blc_attr, 0x00, sizeof(blc_attr));//important, need init first
    blc_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_ablcV32_GetAttrib(ctx, &blc_attr);
    blc_attr.sync.sync_mode = sync_mode;
    blc_attr.eMode = ABLC_V32_OP_MODE_MANUAL;
    blc_attr.stBlc0Manual.enable = 1;
    blc_attr.stBlc0Manual.blc_r = 253;
    blc_attr.stBlc0Manual.blc_gr = 253;
    blc_attr.stBlc0Manual.blc_gb = 253;
    blc_attr.stBlc0Manual.blc_b = 253;

    blc_attr.stBlc1Manual.enable = 1;
    blc_attr.stBlc1Manual.blc_r = 253;
    blc_attr.stBlc1Manual.blc_gr = 253;
    blc_attr.stBlc1Manual.blc_gb = 253;
    blc_attr.stBlc1Manual.blc_b = 253;

    blc_attr.stBlcOBManual.enable = 1;
    blc_attr.stBlcOBManual.ob_offset = 0;
    blc_attr.stBlcOBManual.ob_predgain = 1;

    ret = rk_aiq_user_api2_ablcV32_SetAttrib(ctx, &blc_attr);
    printf("set blc attri manual ret:%d \n\n", ret);


    rk_aiq_blc_attrib_V32_t get_blc_attr;
    get_blc_attr.sync.sync_mode = sync_mode;
    memset(&get_blc_attr, 0x00, sizeof(get_blc_attr));//important, need init first
    ret = rk_aiq_user_api2_ablcV32_GetAttrib(ctx, &get_blc_attr);
    printf("get ablc attri ret:%d done:%d \n\n", ret, get_blc_attr.sync.done);

    sleep(1);
    return ret;

}


XCamReturn sample_ablc_setDefault(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode, rk_aiq_blc_attrib_t &default_blc_attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    default_blc_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_ablc_SetAttrib(ctx, &default_blc_attr);
    printf("set blc attri auto default value ret:%d \n\n", ret);


    rk_aiq_blc_attrib_t get_blc_attr;
    get_blc_attr.sync.sync_mode = sync_mode;
    memset(&get_blc_attr, 0x00, sizeof(get_blc_attr));//important, need init first
    ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &get_blc_attr);
    printf("get ablc attri ret:%d done:%d \n\n", ret, get_blc_attr.sync.done);

    sleep(1);

    return ret;

}

XCamReturn sample_ablc_setDefault_v32(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync_mode, rk_aiq_blc_attrib_V32_t &default_blc_attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    default_blc_attr.sync.sync_mode = sync_mode;
    ret = rk_aiq_user_api2_ablcV32_SetAttrib(ctx, &default_blc_attr);
    printf("set blc attri auto default value ret:%d \n\n", ret);


    rk_aiq_blc_attrib_V32_t get_blc_attr;
    get_blc_attr.sync.sync_mode = sync_mode;
    memset(&get_blc_attr, 0x00, sizeof(get_blc_attr));//important, need init first
    ret = rk_aiq_user_api2_ablcV32_GetAttrib(ctx, &get_blc_attr);
    printf("get ablc attri ret:%d done:%d \n\n", ret, get_blc_attr.sync.done);

    sleep(1);
    return ret;

}

#if USE_NEWSTRUCT
static void sample_blc_tuningtool_test(const rk_aiq_sys_ctx_t* ctx)
{
    char *ret_str = NULL;

    printf(">>> start tuning tool test: op attrib get ...\n");

    std::string json_blc_status_str = " \n\
        [{ \n\
            \"op\":\"get\", \n\
            \"path\": \"/uapi/0/blc_uapi/info\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 0,\"bypass\": 3} \n\
        }]";

    rkaiq_uapi_unified_ctl(const_cast<rk_aiq_sys_ctx_t*>(ctx),
                           const_cast<char*>(json_blc_status_str.c_str()), &ret_str, RKAIQUAPI_OPMODE_GET);

    if (ret_str) {
        printf("blc status json str: %s\n", ret_str);
    }

    printf("  start tuning tool test: op attrib set ...\n");
    std::string json_blc_str = " \n\
        [{ \n\
            \"op\":\"replace\", \n\
            \"path\": \"/uapi/0/blc_uapi/attr\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 1,\"bypass\": 1} \n\
        }]";
    printf("blc json_cmd_str: %s\n", json_blc_str.c_str());
    ret_str = NULL;
    rkaiq_uapi_unified_ctl(const_cast<rk_aiq_sys_ctx_t*>(ctx),
                           const_cast<char*>(json_blc_str.c_str()), &ret_str, RKAIQUAPI_OPMODE_SET);

    // wait more than 2 frames
    usleep(90 * 1000);

    blc_status_t status;
    memset(&status, 0, sizeof(blc_status_t));

    rk_aiq_user_api2_blc_QueryStatus(ctx, &status);

    if (status.opMode != RK_AIQ_OP_MODE_MANUAL || status.en != 1 || status.bypass != 1) {
        printf("blc op set_attrib failed !\n");
        printf("blc status: opmode:%d(EXP:%d), en:%d(EXP:%d), bypass:%d(EXP:%d)\n",
               status.opMode, RK_AIQ_OP_MODE_MANUAL, status.en, 1, status.bypass, 1);
    } else {
        printf("blc op set_attrib success !\n");
    }

    printf(">>> tuning tool test done \n");
}

void sample_new_blc(const rk_aiq_sys_ctx_t* ctx) {
    // get cur mode
    printf("+++++++ BLC module test start ++++++++\n");

    sample_blc_tuningtool_test(ctx);

    blc_api_attrib_t attr;
    memset(&attr, 0, sizeof(attr));

    rk_aiq_user_api2_blc_GetAttrib(ctx, &attr);

    printf("blc attr: opmode:%d, en:%d, bypass:%d\n", attr.opMode, attr.en, attr.bypass);

    if (attr.opMode == RK_AIQ_OP_MODE_AUTO)
        attr.opMode = RK_AIQ_OP_MODE_MANUAL;
    else
        attr.opMode = RK_AIQ_OP_MODE_AUTO;

    // reverse en
    attr.en = !attr.en;

    rk_aiq_user_api2_blc_SetAttrib(ctx, &attr);

    // wait more than 2 frames
    usleep(90 * 1000);

    blc_status_t status;
    rk_aiq_user_api2_blc_QueryStatus(ctx, &status);

    printf("blc status: opmode:%d, en:%d, bypass:%d\n", status.opMode, status.en, status.bypass);

    if (status.opMode != attr.opMode || status.en != attr.en)
        printf("blc arrib api test failed\n");
    else
        printf("blc arrib api test success\n");

    printf("-------- BLC module test done --------\n");

}
#endif

XCamReturn sample_ablc_module (const void *arg)
{
    int key = -1;
    CLEAR();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    const demo_context_t *demo_ctx = (demo_context_t *)arg;
    const rk_aiq_sys_ctx_t* ctx ;

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

    rk_aiq_blc_attrib_t default_blc_attr;
    if (CHECK_ISP_HW_V30()) {
        memset(&default_blc_attr, 0x00, sizeof(default_blc_attr));//important, need init first
        rk_aiq_user_api2_ablc_GetAttrib(ctx, &default_blc_attr);
    }

    rk_aiq_blc_attrib_V32_t default_blc_attr_v32;
    if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        memset(&default_blc_attr_v32, 0x00, sizeof(default_blc_attr_v32));//important, need init first
        rk_aiq_user_api2_ablcV32_GetAttrib(ctx, &default_blc_attr_v32);
    }

    do {
        sample_ablc_usage ();

        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar ();
        printf ("\n");

        switch (key) {
        case '0':
            if (CHECK_ISP_HW_V30()) {
                sample_ablc_getAttr(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_ablc_getAttr_v32(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            break;
        case '1':
            if (CHECK_ISP_HW_V30()) {
                sample_ablc_setAuto(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_ablc_setAuto_v32(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            break;
        case '2':
            if (CHECK_ISP_HW_V30()) {
                sample_ablc_setManual(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_ablc_setManual_v32(ctx, RK_AIQ_UAPI_MODE_SYNC);
            }
            break;
        case '3':
            if (CHECK_ISP_HW_V30()) {
                sample_ablc_setDefault(ctx, RK_AIQ_UAPI_MODE_SYNC, default_blc_attr);
            }
            if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_ablc_setDefault_v32(ctx, RK_AIQ_UAPI_MODE_SYNC, default_blc_attr_v32);
            }
            break;
        case '4':
            if (CHECK_ISP_HW_V30()) {
                sample_ablc_getAttr(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_ablc_getAttr_v32(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            break;
        case '5':
            if (CHECK_ISP_HW_V30()) {
                sample_ablc_setAuto(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_ablc_setAuto_v32(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            break;
        case '6':
            if (CHECK_ISP_HW_V30()) {
                sample_ablc_setManual(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_ablc_setManual_v32(ctx, RK_AIQ_UAPI_MODE_ASYNC);
            }
            break;
        case '7':
            if (CHECK_ISP_HW_V30()) {
                sample_ablc_setDefault(ctx, RK_AIQ_UAPI_MODE_ASYNC, default_blc_attr);
            }
            if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
                sample_ablc_setDefault_v32(ctx, RK_AIQ_UAPI_MODE_ASYNC, default_blc_attr_v32);
            }
            break;
#if USE_NEWSTRUCT
        case '8':
            sample_new_blc(ctx);
            break;
#endif
        default:
            printf("not support test\n\n");
            break;
        }

    } while (key != 'q' && key != 'Q');

    return ret;
}

