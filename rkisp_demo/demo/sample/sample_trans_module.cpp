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
#include "uAPI2/rk_aiq_user_api2_helper.h"
#include <string>

static void sample_trans_usage()
{
    printf("Usage : \n");
    printf("\t 0) TRANS:         sample_new_trans.\n");
    printf("\n");
    printf("\t please press the key: ");

    return;
}

#ifdef USE_NEWSTRUCT
static void sample_trans_tuningtool_test(const rk_aiq_sys_ctx_t* ctx)
{
    char *ret_str = NULL;

    printf(">>> start tuning tool test: op attrib get ...\n");

    std::string json_trans_status_str = " \n\
        [{ \n\
            \"op\":\"get\", \n\
            \"path\": \"/uapi/0/trans_uapi/info\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 0,\"bypass\": 3} \n\
        }]";

    rkaiq_uapi_unified_ctl(const_cast<rk_aiq_sys_ctx_t*>(ctx),
                           const_cast<char*>(json_trans_status_str.c_str()), &ret_str, RKAIQUAPI_OPMODE_GET);

    if (ret_str) {
        printf("trans status json str: %s\n", ret_str);
    }

    printf("  start tuning tool test: op attrib set ...\n");
    std::string json_trans_str = " \n\
        [{ \n\
            \"op\":\"replace\", \n\
            \"path\": \"/uapi/0/trans_uapi/attr\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 1,\"bypass\": 1} \n\
        }]";
    printf("trans json_cmd_str: %s\n", json_trans_str.c_str());
    ret_str = NULL;
    rkaiq_uapi_unified_ctl(const_cast<rk_aiq_sys_ctx_t*>(ctx),
                           const_cast<char*>(json_trans_str.c_str()), &ret_str, RKAIQUAPI_OPMODE_SET);

    // wait more than 2 frames
    usleep(90 * 1000);

    trans_status_t status;
    memset(&status, 0, sizeof(trans_status_t));

    rk_aiq_user_api2_trans_QueryStatus(ctx, &status);

    if (status.opMode != RK_AIQ_OP_MODE_MANUAL || status.en != 1 || status.bypass != 1) {
        printf("trans op set_attrib failed !\n");
        printf("trans status: opmode:%d(EXP:%d), en:%d(EXP:%d), bypass:%d(EXP:%d)\n",
               status.opMode, RK_AIQ_OP_MODE_MANUAL, status.en, 1, status.bypass, 1);
    } else {
        printf("trans op set_attrib success !\n");
    }

    printf(">>> tuning tool test done \n");
}

static void sample_new_trans(const rk_aiq_sys_ctx_t* ctx) {
    sample_trans_tuningtool_test(ctx);

    trans_api_attrib_t attr_trans;
    trans_status_t status;
    rk_aiq_user_api2_trans_GetAttrib(ctx, &attr_trans);
    printf("\t attr_trans.opMode:%d attr_trans.en:%d\n\n",
            attr_trans.opMode, attr_trans.en);

    attr_trans.opMode = RK_AIQ_OP_MODE_MANUAL;
    attr_trans.stMan.sta.hw_transCfg_transOfDrc_offset += 1;
    rk_aiq_user_api2_trans_SetAttrib(ctx, &attr_trans);

    rk_aiq_user_api2_trans_QueryStatus(ctx, &status);
    printf("\t status.opMode:%d status.en:%d\n\n",
            status.opMode, status.en);
}
#endif

XCamReturn sample_trans_module(const void *arg)
{
    int key = -1;
    CLEAR();

    const demo_context_t *demo_ctx = (demo_context_t *)arg;
    const rk_aiq_sys_ctx_t* ctx ;
    if (demo_ctx->camGroup) {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->camgroup_ctx);
    } else {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->aiq_ctx);
    }

    do {
        sample_trans_usage ();

        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar();
        printf ("\n");

        switch (key) {
#ifdef USE_NEWSTRUCT
            case '0': {
                printf("\t sample_new_trans\n\n");
                sample_new_trans(ctx);
                break;
            }
#endif
            default:
                break;
        }
    } while (key != 'q' && key != 'Q');

    return XCAM_RETURN_NO_ERROR;
}
