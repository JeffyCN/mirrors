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
 /*
 ******************************
 *
 * Module level API Sample Func
 *
 ******************************
 */

static int sample_dpcc_set_attr(const rk_aiq_sys_ctx_t *ctx,
                                rk_aiq_uapi_mode_sync_e sync) {
  XCamReturn ret = XCAM_RETURN_NO_ERROR;
  rk_aiq_dpcc_attrib_V20_t attr;
  memset(&attr, 0, sizeof(rk_aiq_dpcc_attrib_V20_t));

  ret = rk_aiq_user_api2_adpcc_GetAttrib(ctx, &attr);
  RKAIQ_SAMPLE_CHECK_RET(ret, "set dpccAttr failed in getting dpcc attrib!");

  printf("dpcc set config:[%d][%d][%d]\n",
         attr.stManual.stOnfly.expert_mode.stage1_use_set_1,
         attr.stManual.stOnfly.expert_mode.stage1_use_set_2,
         attr.stManual.stOnfly.expert_mode.stage1_use_set_3);

  attr.sync.sync_mode = sync;
  if (attr.eMode == ADPCC_OP_MODE_AUTO) {
    attr.eMode = ADPCC_OP_MODE_MANUAL;
    attr.stManual.stOnfly.mode = ADPCC_ONFLY_MODE_EXPERT;
    attr.stManual.stOnfly.expert_mode.stage1_use_set_1 = 1;
    attr.stManual.stOnfly.expert_mode.stage1_use_set_2 = 1;
    attr.stManual.stOnfly.expert_mode.stage1_use_set_3 = 1;
  } else {
    attr.eMode = ADPCC_OP_MODE_AUTO;
  }
  // set
  ret = rk_aiq_user_api2_adpcc_SetAttrib(ctx, &attr);
  RKAIQ_SAMPLE_CHECK_RET(ret, "set dpcc Attr failed!");
  printf("set dpcc Attr\n\n");

  return 0;
}

static int sample_dpcc_get_attr(const rk_aiq_sys_ctx_t *ctx) {
  XCamReturn ret = XCAM_RETURN_NO_ERROR;
  rk_aiq_dpcc_attrib_V20_t attr;
  memset(&attr, 0, sizeof(rk_aiq_dpcc_attrib_V20_t));
  // get
  ret = rk_aiq_user_api2_adpcc_GetAttrib(ctx, &attr);
  RKAIQ_SAMPLE_CHECK_RET(ret, "get dpcc Attr failed!");
  printf("get dpcc Attr:\n\n");
  printf("\t sync = %d, done = %d\n", attr.sync.sync_mode, attr.sync.done);
  if (attr.eMode == ADPCC_OP_MODE_MANUAL) {
    printf("\t mode = Manual\n");
    printf("dpcc set config:[%d][%d][%d]\n",
           attr.stManual.stOnfly.expert_mode.stage1_use_set_1,
           attr.stManual.stOnfly.expert_mode.stage1_use_set_2,
           attr.stManual.stOnfly.expert_mode.stage1_use_set_3);
  } else if (attr.eMode == ADPCC_OP_MODE_AUTO) {
    printf("\t mode = Auto\n");
  } else {
    printf("\t mode is unkown");
  }
  return 0;
}

static int sample_dpcc_set_attr_async(const rk_aiq_sys_ctx_t *ctx) {
  sample_dpcc_set_attr(ctx, RK_AIQ_UAPI_MODE_ASYNC);
  sample_dpcc_get_attr(ctx);
  usleep(40 * 1000);
  sample_dpcc_get_attr(ctx);

  return 0;
}

static int sample_dpcc_set_attr_sync(const rk_aiq_sys_ctx_t *ctx) {
  sample_dpcc_set_attr(ctx, RK_AIQ_UAPI_MODE_DEFAULT);
  sample_dpcc_get_attr(ctx);

  return 0;
}

#ifdef USE_NEWSTRUCT
static void sample_dpc_tuningtool_test(const rk_aiq_sys_ctx_t* ctx)
{
    char *ret_str = NULL;

    printf(">>> start tuning tool test: op attrib get ...\n");

    const char* json_dpc_status_str = " \n\
        [{ \n\
            \"op\":\"get\", \n\
            \"path\": \"/uapi/0/dpc_uapi/info\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 0,\"bypass\": 3} \n\
        }]";

    rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx),
                           json_dpc_status_str, &ret_str, RKAIQUAPI_OPMODE_GET);

    if (ret_str) {
        printf("dpc status json str: %s\n", ret_str);
    }

    printf("  start tuning tool test: op attrib set ...\n");
    const char* json_dpc_str = " \n\
        [{ \n\
            \"op\":\"replace\", \n\
            \"path\": \"/uapi/0/dpc_uapi/attr\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 1,\"bypass\": 1} \n\
        }]";
    printf("dpc json_cmd_str: %s\n", json_dpc_str);
    ret_str = NULL;
    rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx),
                           json_dpc_str, &ret_str, RKAIQUAPI_OPMODE_SET);

    // wait more than 2 frames
    usleep(90 * 1000);

    dpc_status_t status;
    memset(&status, 0, sizeof(dpc_status_t));

    rk_aiq_user_api2_dpc_QueryStatus(ctx, &status);

    if (status.opMode != RK_AIQ_OP_MODE_MANUAL || status.en != 1 || status.bypass != 1) {
        printf("dpc op set_attrib failed !\n");
        printf("dpc status: opmode:%d(EXP:%d), en:%d(EXP:%d), bypass:%d(EXP:%d)\n",
               status.opMode, RK_AIQ_OP_MODE_MANUAL, status.en, 1, status.bypass, 1);
    } else {
        printf("dpc op set_attrib success !\n");
    }

    printf(">>> tuning tool test done \n");
}

static void get_auto_attr(dpc_api_attrib_t* attr) {
    dpc_param_auto_t* stAuto = &attr->stAuto;
    for (int i = 0;i < 13;i++) {
    }
}

static void get_manual_attr(dpc_api_attrib_t* attr) {
    dpc_param_t* stMan = &attr->stMan;
}

int sample_dpc_test(const rk_aiq_sys_ctx_t* ctx)
{
    // sample_dpc_tuningtool_test(ctx);
    // get cur mode
    printf("+++++++ DPCC module test start ++++++++\n");

    dpc_api_attrib_t attr;
    memset(&attr, 0, sizeof(attr));

    rk_aiq_user_api2_dpc_GetAttrib(ctx, &attr);

    printf("dpc attr: opmode:%d, en:%d, bypass:%d\n", attr.opMode, attr.en, attr.bypass);

    srand(time(0));
    int rand_num = rand() % 101;

    if (rand_num <70) {
        printf("update dpc arrrib!\n");
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
        printf("reverse dpc en!\n");
        attr.en = !attr.en;
    }

    rk_aiq_user_api2_dpc_SetAttrib(ctx, &attr);

    // wait more than 2 frames
    usleep(90 * 1000);

    dpc_status_t status;
    memset(&status, 0, sizeof(dpc_status_t));

    rk_aiq_user_api2_dpc_QueryStatus(ctx, &status);

    printf("dpc status: opmode:%d, en:%d, bypass:%d\n", status.opMode, status.en, status.bypass);

    if (status.opMode != attr.opMode || status.en != attr.en)
        printf("dpc test failed\n");
    printf("-------- DPC module test done --------\n");

    return 0;
}
#endif

uapi_case_t dpcc_uapi_list[] = {
  {
    .desc = "Adpcc: set dpcc config async",
    .func = (uapi_case_func)sample_dpcc_set_attr_async
  },
  {
    .desc = "Adpcc: set dpcc config sync",
    .func = (uapi_case_func)sample_dpcc_set_attr_sync
  },
  {
    .desc = "Adpcc: get dpcc config",
    .func = (uapi_case_func)sample_dpcc_get_attr
  },
  {
    #ifdef USE_NEWSTRUCT
    .desc = "Dpcc: get dpcc config",
    .func = (uapi_case_func)sample_dpc_test
    #endif
  },
  {
    .desc = NULL,
    .func = NULL,
  }
};

XCamReturn sample_adpcc_module(const void *arg) {
  CLEAR();
  const demo_context_t *demo_ctx = (demo_context_t *)arg;
  const rk_aiq_sys_ctx_t *ctx;
  if (demo_ctx->camGroup) {
    ctx = (rk_aiq_sys_ctx_t *)(demo_ctx->camgroup_ctx);
  } else {
    ctx = (rk_aiq_sys_ctx_t *)(demo_ctx->aiq_ctx);
  }

  if (ctx == NULL) {
    ERR("%s, ctx is nullptr\n", __FUNCTION__);
    return XCAM_RETURN_ERROR_PARAM;
  }

  uapi_process_loop(ctx, dpcc_uapi_list);

  return XCAM_RETURN_NO_ERROR;
}
