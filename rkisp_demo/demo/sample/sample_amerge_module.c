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

static void sample_amerge_usage()
{
    printf("Usage : \n");
    printf("\t 0) AMERGE:         test rk_aiq_user_api2_amerge_v10_SetAttrib stManual Sync.\n");
    printf("\t 1) AMERGE:         test rk_aiq_user_api2_amerge_v10_SetAttrib stManual Async.\n");
    printf("\t 2) AMERGE:         test rk_aiq_user_api2_amerge_v10_GetAttrib.\n");
    printf(
        "\t 3) AMERGE:         test rk_aiq_user_api2_amerge_v11_SetAttrib stManual BASEFRAME_SHORT "
        "Sync.\n");
    printf(
        "\t 4) AMERGE:         test rk_aiq_user_api2_amerge_v11_SetAttrib stManual BASEFRAME_SHORT "
        "Async.\n");
    printf(
        "\t 5) AMERGE:         test rk_aiq_user_api2_amerge_v11_SetAttrib stManual BASEFRAME_LONG "
        "Sync.\n");
    printf(
        "\t 6) AMERGE:         test rk_aiq_user_api2_amerge_v11_SetAttrib stManual BASEFRAME_LONG "
        "Async.\n");
    printf("\t 7) AMERGE:         test rk_aiq_user_api2_amerge_v11_GetAttrib.\n");
    printf(
        "\t 8) AMERGE:         test rk_aiq_user_api2_amerge_v12_SetAttrib stManual BASEFRAME_SHORT "
        "Sync.\n");
    printf(
        "\t 9) AMERGE:         test rk_aiq_user_api2_amerge_v12_SetAttrib stManual BASEFRAME_SHORT "
        "Async.\n");
    printf(
        "\t a) AMERGE:         test rk_aiq_user_api2_amerge_v12_SetAttrib stManual BASEFRAME_LONG "
        "EnableEachChn off Sync.\n");
    printf(
        "\t b) AMERGE:         test rk_aiq_user_api2_amerge_v12_SetAttrib stManual BASEFRAME_LONG "
        "EnableEachChn off Async.\n");
    printf(
        "\t c) AMERGE:         test rk_aiq_user_api2_amerge_v12_SetAttrib stManual BASEFRAME_LONG "
        "EnableEachChn on Sync.\n");
    printf(
        "\t d) AMERGE:         test rk_aiq_user_api2_amerge_v12_SetAttrib stManual BASEFRAME_LONG "
        "EnableEachChn on Async.\n");
    printf("\t e) AMERGE:         test rk_aiq_user_api2_amerge_v12_GetAttrib.\n");
    printf(
        "\t f) AMERGE:         test rk_aiq_user_api2_amerge_SetAttrib stManual BASEFRAME_SHORT "
        "Sync.\n");
    printf(
        "\t g) AMERGE:         test rk_aiq_user_api2_amerge_SetAttrib stManual BASEFRAME_SHORT "
        "Async.\n");
    printf(
        "\t h) AMERGE:         test rk_aiq_user_api2_amerge_SetAttrib stManual BASEFRAME_LONG "
        "Sync.\n");
    printf(
        "\t i) AMERGE:         test rk_aiq_user_api2_amerge_SetAttrib stManual BASEFRAME_LONG "
        "Async.\n");
    printf("\t j) AMERGE:         test rk_aiq_user_api2_amerge_GetAttrib.\n");
    printf("\t k) AMERGE:         Merge Sample test.\n");

    printf("\t q) AMERGE:         return to main sample screen.\n");
    printf("\n");
    printf("\t please press the key: ");

    return;
}
#ifdef USE_NEWSTRUCT
static void sample_merge_tuningtool_test(const rk_aiq_sys_ctx_t* ctx)
{
    char *ret_str = NULL;

    printf(">>> start tuning tool test: op attrib get ...\n");

    const char* json_merge_status_str = " \n\
        [{ \n\
            \"op\":\"get\", \n\
            \"path\": \"/uapi/0/mge_uapi/info\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 0,\"bypass\": 3} \n\
        }]";

    rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx),
                           json_merge_status_str, &ret_str, RKAIQUAPI_OPMODE_GET);

    if (ret_str) {
        printf("merge status json str: %s\n", ret_str);
    }

    printf("  start tuning tool test: op attrib set ...\n");
    const char* json_merge_str = " \n\
        [{ \n\
            \"op\":\"replace\", \n\
            \"path\": \"/uapi/0/mge_uapi/attr\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 1,\"bypass\": 1} \n\
        }]";
    printf("merge json_cmd_str: %s\n", json_merge_str);
    ret_str = NULL;
    rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx),
                           json_merge_str, &ret_str, RKAIQUAPI_OPMODE_SET);

    // wait more than 2 frames
    usleep(90 * 1000);

    mge_status_t status;
    memset(&status, 0, sizeof(mge_status_t));

    rk_aiq_user_api2_merge_QueryStatus(ctx, &status);

    if (status.opMode != RK_AIQ_OP_MODE_MANUAL || status.en != 1 || status.bypass != 1) {
        printf("merge op set_attrib failed !\n");
        printf("merge status: opmode:%d(EXP:%d), en:%d(EXP:%d), bypass:%d(EXP:%d)\n",
               status.opMode, RK_AIQ_OP_MODE_MANUAL, status.en, 1, status.bypass, 1);
    } else {
        printf("merge op set_attrib success !\n");
    }

    printf(">>> tuning tool test done \n");
}

static void get_auto_attr(mge_api_attrib_t* attr) {
    mge_param_auto_t* stAuto = &attr->stAuto;
    for (int i = 0;i < 13;i++) {
    }
}

static void get_manual_attr(mge_api_attrib_t* attr) {
    mge_param_t* stMan = &attr->stMan;
}

void sample_merge_test(const rk_aiq_sys_ctx_t* ctx)
{
    // sample_merge_tuningtool_test(ctx);
    // get cur mode
    printf("+++++++ MERGE module test start ++++++++\n");

    mge_api_attrib_t attr;
    memset(&attr, 0, sizeof(attr));

    rk_aiq_user_api2_merge_GetAttrib(ctx, &attr);

    printf("merge attr: opmode:%d, en:%d, bypass:%d\n", attr.opMode, attr.en, attr.bypass);

    srand(time(0));
    int rand_num = rand() % 101;

    if (rand_num <70) {
        printf("update merge arrrib!\n");
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
        printf("reverse merge en!\n");
        attr.en = !attr.en;
    }

    rk_aiq_user_api2_merge_SetAttrib(ctx, &attr);

    // wait more than 2 frames
    usleep(90 * 1000);

    mge_status_t status;
    memset(&status, 0, sizeof(mge_status_t));

    rk_aiq_user_api2_merge_QueryStatus(ctx, &status);

    printf("merge status: opmode:%d, en:%d, bypass:%d\n", status.opMode, status.en, status.bypass);

    if (status.opMode != attr.opMode || status.en != attr.en)
        printf("merge test failed\n");
    printf("-------- BTNR module test done --------\n");
}
#endif
void sample_print_amerge_info(const void *arg)
{
    printf ("enter AMERGE modult test!\n");
}

XCamReturn sample_amerge_module(const void *arg)
{
    int key = -1;
    CLEAR();

    mergeAttrV10_t attr_v10;
    memset(&attr_v10, 0x0, sizeof(mergeAttrV10_t));
    mergeAttrV11_t attr_v11;
    memset(&attr_v11, 0x0, sizeof(mergeAttrV11_t));
    mergeAttrV12_t attr_v12;
    memset(&attr_v12, 0x0, sizeof(mergeAttrV12_t));
    amerge_attrib_t attr_v2;
    memset(&attr_v2, 0x0, sizeof(amerge_attrib_t));
    const demo_context_t *demo_ctx = (demo_context_t *)arg;
    const rk_aiq_sys_ctx_t* ctx ;
    if (demo_ctx->camGroup) {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->camgroup_ctx);
    } else {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->aiq_ctx);
    }

    do {
        sample_amerge_usage ();

        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar();
        printf ("\n");

        switch (key) {
            case '0': {
                printf(
                    "\t AMERGE test rk_aiq_user_api2_amerge_v10_SetAttrib stManual BASEFRAME_SHORT "
                    "Sync\n\n");
                attr_v10.sync.sync_mode             = RK_AIQ_UAPI_MODE_DEFAULT;
                attr_v10.sync.done                  = false;
                attr_v10.opMode                     = MERGE_OPMODE_MANUAL;
                attr_v10.stManual.OECurve.Smooth    = 0.4f;
                attr_v10.stManual.OECurve.Offset    = 210.0f;
                attr_v10.stManual.MDCurve.LM_smooth = 0.4f;
                attr_v10.stManual.MDCurve.LM_offset = 0.38f;
                attr_v10.stManual.MDCurve.MS_smooth = 0.4f;
                attr_v10.stManual.MDCurve.MS_offset = 0.38f;
                rk_aiq_user_api2_amerge_v10_SetAttrib(ctx, &attr_v10);
                break;
            }
            case '1': {
                printf(
                    "\t AMERGE test rk_aiq_user_api2_amerge_v10_SetAttrib stManual BASEFRAME_SHORT "
                    "Async\n\n");
                attr_v10.sync.sync_mode             = RK_AIQ_UAPI_MODE_ASYNC;
                attr_v10.sync.done                  = false;
                attr_v10.opMode                     = MERGE_OPMODE_MANUAL;
                attr_v10.stManual.OECurve.Smooth    = 0.4f;
                attr_v10.stManual.OECurve.Offset    = 210.0f;
                attr_v10.stManual.MDCurve.LM_smooth = 0.4f;
                attr_v10.stManual.MDCurve.LM_offset = 0.38f;
                attr_v10.stManual.MDCurve.MS_smooth = 0.4f;
                attr_v10.stManual.MDCurve.MS_offset = 0.38f;
                rk_aiq_user_api2_amerge_v10_SetAttrib(ctx, &attr_v10);
                break;
            }
            case '2': {
                printf("\t AMERGE test rk_aiq_user_api2_amerge_v10_GetAttrib\n\n");
                rk_aiq_user_api2_amerge_v10_GetAttrib(ctx, &attr_v10);
                printf("\t sync = %d, done = %d\n", attr_v10.sync.sync_mode, attr_v10.sync.done);
                printf("\t attr_v10.mode:%d Envlv:%f MoveCoef:%f\n\n", attr_v10.opMode,
                       attr_v10.Info.Envlv, attr_v10.Info.MoveCoef);
                printf("\t OECurve Smooth:%f Offset:%f\n\n", attr_v10.stManual.OECurve.Smooth,
                       attr_v10.stManual.OECurve.Offset);
                printf(
                    "\t MDCurve LM_smooth:%f LM_offset:%f MS_smooth%f "
                    "MS_offset:%f\n\n",
                    attr_v10.stManual.MDCurve.LM_smooth, attr_v10.stManual.MDCurve.LM_offset,
                    attr_v10.stManual.MDCurve.MS_smooth, attr_v10.stManual.MDCurve.MS_offset);
                break;
            }
            case '3': {
                printf(
                    "\t AMERGE test rk_aiq_user_api2_amerge_v11_SetAttrib stManual BASEFRAME_SHORT "
                    "Sync\n\n");
                attr_v11.sync.sync_mode                            = RK_AIQ_UAPI_MODE_DEFAULT;
                attr_v11.sync.done                                 = false;
                attr_v11.opMode                                    = MERGE_OPMODE_MANUAL;
                attr_v11.stManual.BaseFrm                          = BASEFRAME_SHORT;
                attr_v11.stManual.ShortFrmModeData.OECurve.Smooth  = 0.4f;
                attr_v11.stManual.ShortFrmModeData.OECurve.Offset  = 210.0f;
                attr_v11.stManual.ShortFrmModeData.MDCurve.Coef    = 0.05f;
                attr_v11.stManual.ShortFrmModeData.MDCurve.ms_thd0 = 0.00f;
                attr_v11.stManual.ShortFrmModeData.MDCurve.lm_thd0 = 0.00f;
                rk_aiq_user_api2_amerge_v11_SetAttrib(ctx, &attr_v11);
                break;
            }
            case '4': {
                printf(
                    "\t AMERGE test rk_aiq_user_api2_amerge_v11_SetAttrib stManual BASEFRAME_SHORT "
                    "Async\n\n");
                attr_v11.sync.sync_mode                            = RK_AIQ_UAPI_MODE_ASYNC;
                attr_v11.sync.done                                 = false;
                attr_v11.opMode                                    = MERGE_OPMODE_MANUAL;
                attr_v11.stManual.BaseFrm                          = BASEFRAME_SHORT;
                attr_v11.stManual.ShortFrmModeData.OECurve.Smooth  = 0.5f;
                attr_v11.stManual.ShortFrmModeData.OECurve.Offset  = 230.0f;
                attr_v11.stManual.ShortFrmModeData.MDCurve.Coef    = 0.07f;
                attr_v11.stManual.ShortFrmModeData.MDCurve.ms_thd0 = 0.00f;
                attr_v11.stManual.ShortFrmModeData.MDCurve.lm_thd0 = 0.00f;
                rk_aiq_user_api2_amerge_v11_SetAttrib(ctx, &attr_v11);
                break;
            }
            case '5': {
                printf(
                    "\t AMERGE test rk_aiq_user_api2_amerge_v11_SetAttrib stManual BASEFRAME_LONG "
                    "Sync\n\n");
                attr_v11.sync.sync_mode                             = RK_AIQ_UAPI_MODE_DEFAULT;
                attr_v11.sync.done                                  = false;
                attr_v11.opMode                                     = MERGE_OPMODE_MANUAL;
                attr_v11.stManual.BaseFrm                           = BASEFRAME_LONG;
                attr_v11.stManual.LongFrmModeData.OECurve.Smooth    = 0.6f;
                attr_v11.stManual.LongFrmModeData.OECurve.Offset    = 240.0f;
                attr_v11.stManual.LongFrmModeData.MDCurve.LM_smooth = 0.8f;
                attr_v11.stManual.LongFrmModeData.MDCurve.LM_offset = 0.38f;
                attr_v11.stManual.LongFrmModeData.MDCurve.MS_smooth = 0.8f;
                attr_v11.stManual.LongFrmModeData.MDCurve.MS_offset = 0.38f;
                rk_aiq_user_api2_amerge_v11_SetAttrib(ctx, &attr_v11);
                break;
            }
            case '6': {
                printf(
                    "\t AMERGE test rk_aiq_user_api2_amerge_v11_SetAttrib stManual BASEFRAME_LONG "
                    "Async\n\n");
                attr_v11.sync.sync_mode                             = RK_AIQ_UAPI_MODE_ASYNC;
                attr_v11.sync.done                                  = false;
                attr_v11.opMode                                     = MERGE_OPMODE_MANUAL;
                attr_v11.stManual.BaseFrm                           = BASEFRAME_LONG;
                attr_v11.stManual.LongFrmModeData.OECurve.Smooth    = 0.7f;
                attr_v11.stManual.LongFrmModeData.OECurve.Offset    = 280.0f;
                attr_v11.stManual.LongFrmModeData.MDCurve.LM_smooth = 0.8f;
                attr_v11.stManual.LongFrmModeData.MDCurve.LM_offset = 0.38f;
                attr_v11.stManual.LongFrmModeData.MDCurve.MS_smooth = 0.8f;
                attr_v11.stManual.LongFrmModeData.MDCurve.MS_offset = 0.38f;
                rk_aiq_user_api2_amerge_v11_SetAttrib(ctx, &attr_v11);
                break;
            }
            case '7': {
                printf("\t AMERGE test rk_aiq_user_api2_amerge_v11_GetAttrib\n\n");
                rk_aiq_user_api2_amerge_v11_GetAttrib(ctx, &attr_v11);
                printf("\t sync = %d, done = %d\n", attr_v11.sync.sync_mode, attr_v11.sync.done);
                printf("\t attr_v11.mode:%d Envlv:%f MoveCoef:%f\n\n", attr_v11.opMode,
                       attr_v11.Info.Envlv, attr_v11.Info.MoveCoef);
                printf("\t BaseFrm:%d \n\n", attr_v11.stManual.BaseFrm);
                printf("\t LongFrmModeData OECurve Smooth:%f Offset:%f\n\n",
                       attr_v11.stManual.LongFrmModeData.OECurve.Smooth,
                       attr_v11.stManual.LongFrmModeData.OECurve.Offset);
                printf(
                    "\t LongFrmModeData MDCurve LM_smooth:%f LM_offset:%f MS_smooth%f "
                    "MS_offset:%f\n\n",
                    attr_v11.stManual.LongFrmModeData.MDCurve.LM_smooth,
                    attr_v11.stManual.LongFrmModeData.MDCurve.LM_offset,
                    attr_v11.stManual.LongFrmModeData.MDCurve.MS_smooth,
                    attr_v11.stManual.LongFrmModeData.MDCurve.MS_offset);
                printf("\t ShortFrmModeData OECurve Smooth:%f Offset:%f\n\n",
                       attr_v11.stManual.ShortFrmModeData.OECurve.Smooth,
                       attr_v11.stManual.ShortFrmModeData.OECurve.Offset);
                printf("\t ShortFrmModeData MDCurve Coef:%f ms_thd0:%f lm_thd0%f\n\n",
                       attr_v11.stManual.ShortFrmModeData.MDCurve.Coef,
                       attr_v11.stManual.ShortFrmModeData.MDCurve.ms_thd0,
                       attr_v11.stManual.ShortFrmModeData.MDCurve.lm_thd0);
                break;
            }
            case '8': {
                printf(
                    "\t AMERGE test rk_aiq_user_api2_amerge_v12_SetAttrib stManual BASEFRAME_SHORT "
                    "Sync\n\n");
                attr_v12.sync.sync_mode                            = RK_AIQ_UAPI_MODE_DEFAULT;
                attr_v12.sync.done                                 = false;
                attr_v12.opMode                                    = MERGE_OPMODE_MANUAL;
                attr_v12.stManual.BaseFrm                          = BASEFRAME_SHORT;
                attr_v12.stManual.ShortFrmModeData.OECurve.Smooth  = 0.4f;
                attr_v12.stManual.ShortFrmModeData.OECurve.Offset  = 210.0f;
                attr_v12.stManual.ShortFrmModeData.MDCurve.Coef    = 0.05f;
                attr_v12.stManual.ShortFrmModeData.MDCurve.ms_thd0 = 0.00f;
                attr_v12.stManual.ShortFrmModeData.MDCurve.lm_thd0 = 0.00f;
                rk_aiq_user_api2_amerge_v12_SetAttrib(ctx, &attr_v12);
                break;
            }
            case '9': {
                printf(
                    "\t AMERGE test rk_aiq_user_api2_amerge_v12_SetAttrib stManual BASEFRAME_SHORT "
                    "Async\n\n");
                attr_v12.sync.sync_mode                            = RK_AIQ_UAPI_MODE_ASYNC;
                attr_v12.sync.done                                 = false;
                attr_v12.opMode                                    = MERGE_OPMODE_MANUAL;
                attr_v12.stManual.BaseFrm                          = BASEFRAME_SHORT;
                attr_v12.stManual.ShortFrmModeData.OECurve.Smooth  = 0.5f;
                attr_v12.stManual.ShortFrmModeData.OECurve.Offset  = 230.0f;
                attr_v12.stManual.ShortFrmModeData.MDCurve.Coef    = 0.07f;
                attr_v12.stManual.ShortFrmModeData.MDCurve.ms_thd0 = 0.00f;
                attr_v12.stManual.ShortFrmModeData.MDCurve.lm_thd0 = 0.00f;
                rk_aiq_user_api2_amerge_v12_SetAttrib(ctx, &attr_v12);
                break;
            }
            case 'a': {
                printf(
                    "\t AMERGE test rk_aiq_user_api2_amerge_v12_SetAttrib stManual BASEFRAME_LONG "
                    "EnableEachChn off"
                    "Sync\n\n");
                attr_v12.sync.sync_mode                             = RK_AIQ_UAPI_MODE_DEFAULT;
                attr_v12.sync.done                                  = false;
                attr_v12.opMode                                     = MERGE_OPMODE_MANUAL;
                attr_v12.stManual.BaseFrm                           = BASEFRAME_LONG;
                attr_v12.stManual.LongFrmModeData.EnableEachChn     = false;
                attr_v12.stManual.LongFrmModeData.OECurve.Smooth    = 0.6f;
                attr_v12.stManual.LongFrmModeData.OECurve.Offset    = 240.0f;
                attr_v12.stManual.LongFrmModeData.MDCurve.LM_smooth = 0.8f;
                attr_v12.stManual.LongFrmModeData.MDCurve.LM_offset = 0.38f;
                attr_v12.stManual.LongFrmModeData.MDCurve.MS_smooth = 0.8f;
                attr_v12.stManual.LongFrmModeData.MDCurve.MS_offset = 0.38f;
                rk_aiq_user_api2_amerge_v12_SetAttrib(ctx, &attr_v12);
                break;
            }
            case 'b': {
                printf(
                    "\t AMERGE test rk_aiq_user_api2_amerge_v12_SetAttrib stManual BASEFRAME_LONG "
                    "EnableEachChn off"
                    "Async\n\n");
                attr_v12.sync.sync_mode                             = RK_AIQ_UAPI_MODE_ASYNC;
                attr_v12.sync.done                                  = false;
                attr_v12.opMode                                     = MERGE_OPMODE_MANUAL;
                attr_v12.stManual.BaseFrm                           = BASEFRAME_LONG;
                attr_v12.stManual.LongFrmModeData.EnableEachChn     = false;
                attr_v12.stManual.LongFrmModeData.OECurve.Smooth    = 0.7f;
                attr_v12.stManual.LongFrmModeData.OECurve.Offset    = 280.0f;
                attr_v12.stManual.LongFrmModeData.MDCurve.LM_smooth = 0.8f;
                attr_v12.stManual.LongFrmModeData.MDCurve.LM_offset = 0.38f;
                attr_v12.stManual.LongFrmModeData.MDCurve.MS_smooth = 0.8f;
                attr_v12.stManual.LongFrmModeData.MDCurve.MS_offset = 0.38f;
                rk_aiq_user_api2_amerge_v12_SetAttrib(ctx, &attr_v12);
                break;
            }
            case 'c': {
                printf(
                    "\t AMERGE test rk_aiq_user_api2_amerge_v12_SetAttrib stManual BASEFRAME_LONG "
                    "EnableEachChn on"
                    "Sync\n\n");
                attr_v12.sync.sync_mode                               = RK_AIQ_UAPI_MODE_DEFAULT;
                attr_v12.sync.done                                    = false;
                attr_v12.opMode                                       = MERGE_OPMODE_MANUAL;
                attr_v12.stManual.BaseFrm                             = BASEFRAME_LONG;
                attr_v12.stManual.LongFrmModeData.EnableEachChn       = true;
                attr_v12.stManual.LongFrmModeData.OECurve.Smooth      = 0.6f;
                attr_v12.stManual.LongFrmModeData.OECurve.Offset      = 240.0f;
                attr_v12.stManual.LongFrmModeData.MDCurve.LM_smooth   = 0.8f;
                attr_v12.stManual.LongFrmModeData.MDCurve.LM_offset   = 0.38f;
                attr_v12.stManual.LongFrmModeData.MDCurve.MS_smooth   = 0.8f;
                attr_v12.stManual.LongFrmModeData.MDCurve.MS_offset   = 0.38f;
                attr_v12.stManual.LongFrmModeData.EachChnCurve.Smooth = 0.5f;
                attr_v12.stManual.LongFrmModeData.EachChnCurve.Offset = 0.5f;
                rk_aiq_user_api2_amerge_v12_SetAttrib(ctx, &attr_v12);
                break;
            }
            case 'd': {
                printf(
                    "\t AMERGE test rk_aiq_user_api2_amerge_v12_SetAttrib stManual BASEFRAME_LONG "
                    "EnableEachChn on"
                    "Async\n\n");
                attr_v12.sync.sync_mode                               = RK_AIQ_UAPI_MODE_ASYNC;
                attr_v12.sync.done                                    = false;
                attr_v12.opMode                                       = MERGE_OPMODE_MANUAL;
                attr_v12.stManual.BaseFrm                             = BASEFRAME_LONG;
                attr_v12.stManual.LongFrmModeData.EnableEachChn       = true;
                attr_v12.stManual.LongFrmModeData.OECurve.Smooth      = 0.7f;
                attr_v12.stManual.LongFrmModeData.OECurve.Offset      = 280.0f;
                attr_v12.stManual.LongFrmModeData.MDCurve.LM_smooth   = 0.8f;
                attr_v12.stManual.LongFrmModeData.MDCurve.LM_offset   = 0.38f;
                attr_v12.stManual.LongFrmModeData.MDCurve.MS_smooth   = 0.8f;
                attr_v12.stManual.LongFrmModeData.MDCurve.MS_offset   = 0.38f;
                attr_v12.stManual.LongFrmModeData.EachChnCurve.Smooth = 0.4f;
                attr_v12.stManual.LongFrmModeData.EachChnCurve.Offset = 0.38f;
                rk_aiq_user_api2_amerge_v12_SetAttrib(ctx, &attr_v12);
                break;
            }
            case 'e': {
                printf("\t AMERGE test rk_aiq_user_api2_amerge_v12_GetAttrib\n\n");
                rk_aiq_user_api2_amerge_v12_GetAttrib(ctx, &attr_v12);
                printf("\t sync = %d, done = %d\n", attr_v12.sync.sync_mode, attr_v12.sync.done);
                printf("\t attr_v12.mode:%d Envlv:%f MoveCoef:%f\n\n", attr_v12.opMode,
                       attr_v12.Info.Envlv, attr_v12.Info.MoveCoef);
                printf("\t stManual BaseFrm:%d \n\n", attr_v12.stManual.BaseFrm);
                printf("\t stManual LongFrmModeData OECurve Smooth:%f Offset:%f\n\n",
                       attr_v12.stManual.LongFrmModeData.OECurve.Smooth,
                       attr_v12.stManual.LongFrmModeData.OECurve.Offset);
                printf(
                    "\t stManual LongFrmModeData MDCurve LM_smooth:%f LM_offset:%f MS_smooth%f "
                    "MS_offset:%f\n\n",
                    attr_v12.stManual.LongFrmModeData.MDCurve.LM_smooth,
                    attr_v12.stManual.LongFrmModeData.MDCurve.LM_offset,
                    attr_v12.stManual.LongFrmModeData.MDCurve.MS_smooth,
                    attr_v12.stManual.LongFrmModeData.MDCurve.MS_offset);
                printf(
                    "\t stManual LongFrmModeData EnableEachChn:%d EachChnCurve Smooth:%f Offset%f ",
                    attr_v12.stManual.LongFrmModeData.EnableEachChn,
                    attr_v12.stManual.LongFrmModeData.EachChnCurve.Smooth,
                    attr_v12.stManual.LongFrmModeData.EachChnCurve.Offset);
                printf("\t stManual ShortFrmModeData OECurve Smooth:%f Offset:%f\n\n",
                       attr_v12.stManual.ShortFrmModeData.OECurve.Smooth,
                       attr_v12.stManual.ShortFrmModeData.OECurve.Offset);
                printf("\t stManual ShortFrmModeData MDCurve Coef:%f ms_thd0:%f lm_thd0%f\n\n",
                       attr_v12.stManual.ShortFrmModeData.MDCurve.Coef,
                       attr_v12.stManual.ShortFrmModeData.MDCurve.ms_thd0,
                       attr_v12.stManual.ShortFrmModeData.MDCurve.lm_thd0);
                break;
            }
            case 'f': {
                printf(
                    "\t AMERGE test rk_aiq_user_api2_amerge_SetAttrib stManual BASEFRAME_SHORT "
                    "Sync\n\n");
                attr_v2.sync.sync_mode                                   = RK_AIQ_UAPI_MODE_DEFAULT;
                attr_v2.sync.done                                        = false;
                attr_v2.attrV30.opMode                                   = MERGE_OPMODE_MANU;
                attr_v2.attrV30.stManual.BaseFrm                         = BASEFRAME_SHORT;
                attr_v2.attrV30.stManual.ShortFrmModeData.OECurve.Smooth  = 0.4f;
                attr_v2.attrV30.stManual.ShortFrmModeData.OECurve.Offset  = 210.0f;
                attr_v2.attrV30.stManual.ShortFrmModeData.MDCurve.Coef    = 0.05f;
                attr_v2.attrV30.stManual.ShortFrmModeData.MDCurve.ms_thd0 = 0.00f;
                attr_v2.attrV30.stManual.ShortFrmModeData.MDCurve.lm_thd0 = 0.00f;
                rk_aiq_user_api2_amerge_SetAttrib(ctx, attr_v2);
                break;
            }
            case 'g': {
                printf(
                    "\t AMERGE test rk_aiq_user_api2_amerge_SetAttrib stManual BASEFRAME_SHORT "
                    "Async\n\n");
                attr_v2.sync.sync_mode                                    = RK_AIQ_UAPI_MODE_ASYNC;
                attr_v2.sync.done                                         = false;
                attr_v2.attrV30.opMode                                    = MERGE_OPMODE_MANU;
                attr_v2.attrV30.stManual.BaseFrm                          = BASEFRAME_SHORT;
                attr_v2.attrV30.stManual.ShortFrmModeData.OECurve.Smooth  = 0.5f;
                attr_v2.attrV30.stManual.ShortFrmModeData.OECurve.Offset  = 230.0f;
                attr_v2.attrV30.stManual.ShortFrmModeData.MDCurve.Coef    = 0.07f;
                attr_v2.attrV30.stManual.ShortFrmModeData.MDCurve.ms_thd0 = 0.00f;
                attr_v2.attrV30.stManual.ShortFrmModeData.MDCurve.lm_thd0 = 0.00f;
                rk_aiq_user_api2_amerge_SetAttrib(ctx, attr_v2);
                break;
            }
            case 'h': {
                printf(
                    "\t AMERGE test rk_aiq_user_api2_amerge_SetAttrib stManual BASEFRAME_LONG "
                    "Sync\n\n");
                attr_v2.sync.sync_mode                                  = RK_AIQ_UAPI_MODE_DEFAULT;
                attr_v2.sync.done                                       = false;
                attr_v2.attrV30.opMode                                  = MERGE_OPMODE_MANU;
                attr_v2.attrV30.stManual.BaseFrm                        = BASEFRAME_LONG;
                attr_v2.attrV30.stManual.LongFrmModeData.OECurve.Smooth = 0.6f;
                attr_v2.attrV30.stManual.LongFrmModeData.OECurve.Offset = 240.0f;
                attr_v2.attrV30.stManual.LongFrmModeData.MDCurve.LM_smooth = 0.8f;
                attr_v2.attrV30.stManual.LongFrmModeData.MDCurve.LM_offset = 0.38f;
                attr_v2.attrV30.stManual.LongFrmModeData.MDCurve.MS_smooth = 0.8f;
                attr_v2.attrV30.stManual.LongFrmModeData.MDCurve.MS_offset = 0.38f;
                rk_aiq_user_api2_amerge_SetAttrib(ctx, attr_v2);
                break;
            }
            case 'i': {
                printf(
                    "\t AMERGE test rk_aiq_user_api2_amerge_SetAttrib stManual BASEFRAME_LONG "
                    "Async\n\n");
                attr_v2.sync.sync_mode                                     = RK_AIQ_UAPI_MODE_ASYNC;
                attr_v2.sync.done                                          = false;
                attr_v2.attrV30.opMode                                     = MERGE_OPMODE_MANU;
                attr_v2.attrV30.stManual.BaseFrm                           = BASEFRAME_LONG;
                attr_v2.attrV30.stManual.LongFrmModeData.OECurve.Smooth    = 0.7f;
                attr_v2.attrV30.stManual.LongFrmModeData.OECurve.Offset    = 280.0f;
                attr_v2.attrV30.stManual.LongFrmModeData.MDCurve.LM_smooth = 0.8f;
                attr_v2.attrV30.stManual.LongFrmModeData.MDCurve.LM_offset = 0.38f;
                attr_v2.attrV30.stManual.LongFrmModeData.MDCurve.MS_smooth = 0.8f;
                attr_v2.attrV30.stManual.LongFrmModeData.MDCurve.MS_offset = 0.38f;
                rk_aiq_user_api2_amerge_SetAttrib(ctx, attr_v2);
                break;
            }
            case 'j': {
                printf("\t AMERGE test rk_aiq_user_api2_amerge_GetAttrib\n\n");
                rk_aiq_user_api2_amerge_GetAttrib(ctx, &attr_v2);
                printf("\t sync = %d, done = %d\n", attr_v2.sync.sync_mode, attr_v2.sync.done);
                printf("\t attr_v2.mode:%d Envlv:%f MoveCoef:%f\n\n", attr_v2.attrV30.opMode,
                       attr_v2.attrV30.CtlInfo.Envlv, attr_v2.attrV30.CtlInfo.MoveCoef);
                printf("\t BaseFrm:%d \n\n", attr_v2.attrV30.stManual.BaseFrm);
                printf("\t LongFrmModeData OECurve Smooth:%f Offset:%f\n\n",
                       attr_v2.attrV30.stManual.LongFrmModeData.OECurve.Smooth,
                       attr_v2.attrV30.stManual.LongFrmModeData.OECurve.Offset);
                printf(
                    "\t LongFrmModeData MDCurve LM_smooth:%f LM_offset:%f MS_smooth%f "
                    "MS_offset:%f\n\n",
                    attr_v2.attrV30.stManual.LongFrmModeData.MDCurve.LM_smooth,
                    attr_v2.attrV30.stManual.LongFrmModeData.MDCurve.LM_offset,
                    attr_v2.attrV30.stManual.LongFrmModeData.MDCurve.MS_smooth,
                    attr_v2.attrV30.stManual.LongFrmModeData.MDCurve.MS_offset);
                printf("\t ShortFrmModeData OECurve Smooth:%f Offset:%f\n\n",
                       attr_v2.attrV30.stManual.ShortFrmModeData.OECurve.Smooth,
                       attr_v2.attrV30.stManual.ShortFrmModeData.OECurve.Offset);
                printf("\t ShortFrmModeData MDCurve Coef:%f ms_thd0:%f lm_thd0%f\n\n",
                       attr_v2.attrV30.stManual.ShortFrmModeData.MDCurve.Coef,
                       attr_v2.attrV30.stManual.ShortFrmModeData.MDCurve.ms_thd0,
                       attr_v2.attrV30.stManual.ShortFrmModeData.MDCurve.lm_thd0);
                break;
            }
#ifdef USE_NEWSTRUCT
            case 'k': {
                sample_merge_test(ctx);
                break;
            }
#endif
            default:
                break;
        }
    } while (key != 'q' && key != 'Q');

    return XCAM_RETURN_NO_ERROR;
}
