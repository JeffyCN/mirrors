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

static int sample_lsc_set_attr(const rk_aiq_sys_ctx_t* ctx, rk_aiq_uapi_mode_sync_e sync)
{
  XCamReturn ret = XCAM_RETURN_NO_ERROR;
  rk_aiq_lsc_attrib_t attr;
  memset(&attr, 0, sizeof(rk_aiq_lsc_attrib_t));

  ret = rk_aiq_user_api2_alsc_GetAttrib(ctx, &attr);
  RKAIQ_SAMPLE_CHECK_RET(ret, "setlscAttr failed in getting lsc attrib!");

  attr.sync.sync_mode = sync;
  if (attr.mode == RK_AIQ_LSC_MODE_AUTO) {
    attr.mode = RK_AIQ_LSC_MODE_MANUAL;
    attr.byPass = false;
    attr.stManual.r_data_tbl[0] = 4096;
  } else {
    attr.mode = RK_AIQ_LSC_MODE_AUTO;
    if (attr.stAuto.tbl.tableAll[0].lsc_samples_red.uCoeff[0] == 8191) {
      attr.stAuto.tbl.tableAll[0].lsc_samples_red.uCoeff[0] = 4096;
    } else {
      attr.stAuto.tbl.tableAll[0].lsc_samples_red.uCoeff[0] = 8191;
    }
  }
  //set
  ret = rk_aiq_user_api2_alsc_SetAttrib(ctx, attr);
  RKAIQ_SAMPLE_CHECK_RET(ret, "set lsc Attr failed!");
  printf("set lsc Attr\n\n");

  return 0;
}

static int sample_lsc_get_attr(const rk_aiq_sys_ctx_t* ctx)
{
  XCamReturn ret = XCAM_RETURN_NO_ERROR;
  rk_aiq_lsc_attrib_t attr;
  memset(&attr,0,sizeof(rk_aiq_lsc_attrib_t));
  //get
  ret = rk_aiq_user_api2_alsc_GetAttrib(ctx, &attr);
  RKAIQ_SAMPLE_CHECK_RET(ret, "get lsc Attr failed!");
  printf("get lsc Attr:\n\n");
  printf("\t sync = %d, done = %d\n", attr.sync.sync_mode, attr.sync.done);
  printf("\t enable = %s\n", (attr.byPass ? "false" : "true"));
  if (attr.mode == RK_AIQ_LSC_MODE_MANUAL){
    printf("\t mode = Manual\n");
    printf("\t manual table\n");
    printf("\t\t r: %d, %d, %d \n",
           attr.stManual.r_data_tbl[0],
           attr.stManual.r_data_tbl[1],
           attr.stManual.r_data_tbl[2]);
    printf("\t\t gr: %d, %d, %d \n",
           attr.stManual.gr_data_tbl[0],
           attr.stManual.gr_data_tbl[1],
           attr.stManual.gr_data_tbl[2]);
    printf("\t\t gb: %d, %d, %d \n",
           attr.stManual.gb_data_tbl[0],
           attr.stManual.gb_data_tbl[1],
           attr.stManual.gb_data_tbl[2]);
    printf("\t\t b: %d, %d, %d \n",
           attr.stManual.b_data_tbl[0],
           attr.stManual.b_data_tbl[1],
           attr.stManual.b_data_tbl[2]);
  } else if (attr.mode == RK_AIQ_LSC_MODE_AUTO) {
    printf("\t mode = Auto, tableAll[0].lsc_samples_red.uCoeff[0] = %d\n", \
            attr.stAuto.tbl.tableAll[0].lsc_samples_red.uCoeff[0]);
  } else {
    printf("\t mode is invalid");
  }
  return 0;
}

static int sample_lsc_set_attr_async(const rk_aiq_sys_ctx_t* ctx)
{
  sample_lsc_set_attr(ctx, RK_AIQ_UAPI_MODE_ASYNC);
  sample_lsc_get_attr(ctx);
  usleep(40*1000);
  sample_lsc_get_attr(ctx);

  return 0;
}

static int sample_lsc_set_attr_sync(const rk_aiq_sys_ctx_t* ctx)
{
  sample_lsc_set_attr(ctx, RK_AIQ_UAPI_MODE_DEFAULT);
  sample_lsc_get_attr(ctx);

  return 0;
}
#ifdef USE_NEWSTRUCT
static void sample_lsc_tuningtool_test(const rk_aiq_sys_ctx_t* ctx)
{
    char *ret_str = NULL;

    printf(">>> start tuning tool test: op attrib get ...\n");

    const char* json_lsc_status_str = " \n\
        [{ \n\
            \"op\":\"get\", \n\
            \"path\": \"/uapi/0/lsc_uapi/info\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 0,\"bypass\": 3} \n\
        }]";

    rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx),
                           json_lsc_status_str, &ret_str, RKAIQUAPI_OPMODE_GET);

    if (ret_str) {
        printf("lsc status json str: %s\n", ret_str);
    }

    printf("  start tuning tool test: op attrib set ...\n");
    const char* json_lsc_str = " \n\
        [{ \n\
            \"op\":\"replace\", \n\
            \"path\": \"/uapi/0/lsc_uapi/attr\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 1,\"bypass\": 1} \n\
        }]";
    printf("lsc json_cmd_str: %s\n", json_lsc_str);
    ret_str = NULL;
    rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx),
                           json_lsc_str, &ret_str, RKAIQUAPI_OPMODE_SET);

    // wait more than 2 frames
    usleep(90 * 1000);

    lsc_status_t status;
    memset(&status, 0, sizeof(lsc_status_t));

    rk_aiq_user_api2_lsc_QueryStatus(ctx, &status);

    if (status.opMode != RK_AIQ_OP_MODE_MANUAL || status.en != 1 || status.bypass != 1) {
        printf("lsc op set_attrib failed !\n");
        printf("lsc status: opmode:%d(EXP:%d), en:%d(EXP:%d), bypass:%d(EXP:%d)\n",
               status.opMode, RK_AIQ_OP_MODE_MANUAL, status.en, 1, status.bypass, 1);
    } else {
        printf("lsc op set_attrib success !\n");
    }

    printf(">>> tuning tool test done \n");
}

static void get_auto_attr(lsc_api_attrib_t* attr) {
    lsc_param_auto_t* stAuto = &attr->stAuto;
    for (int i = 0;i < 13;i++) {
    }
}

static void get_manual_attr(lsc_api_attrib_t* attr) {
    lsc_param_t* stMan = &attr->stMan;
}

int sample_lsc_test(const rk_aiq_sys_ctx_t* ctx)
{
    // sample_lsc_tuningtool_test(ctx);
    // get cur mode
    printf("+++++++ lsc module test start ++++++++\n");

    lsc_api_attrib_t attr;
    memset(&attr, 0, sizeof(attr));

    rk_aiq_user_api2_lsc_GetAttrib(ctx, &attr);

    printf("lsc attr: opmode:%d, en:%d, bypass:%d\n", attr.opMode, attr.en, attr.bypass);

    srand(time(0));
    int rand_num = rand() % 101;

    if (rand_num <70) {
        printf("update lsc arrrib!\n");
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
        printf("reverse lsc en!\n");
        attr.en = !attr.en;
    }

    rk_aiq_user_api2_lsc_SetAttrib(ctx, &attr);

    // wait more than 2 frames
    usleep(180 * 1000);

    lsc_status_t status;
    memset(&status, 0, sizeof(lsc_status_t));

    rk_aiq_user_api2_lsc_QueryStatus(ctx, &status);

    printf("lsc status: opmode:%d, en:%d, bypass:%d\n", status.opMode, status.en, status.bypass);

    if (status.opMode != attr.opMode || status.en != attr.en)
        printf("lsc test failed\n");
    printf("-------- lsc module test done --------\n");

    return 0;
}

int sample_query_lsc_status(const rk_aiq_sys_ctx_t* ctx)
{
    lsc_status_t info;
    rk_aiq_user_api2_lsc_QueryStatus(ctx, &info);
    printf("Query lsc status:\n\n");
    printf("  opMode: %d, en: %d, bypass: %d,\n"
           "  stMan: {\n    sta: {mode: %d, [%f,%f,...], [%f,%f,...]}; "
           "    dyn: {r: [%d,...,%d], gr: [%d,...,%d], b: [%d,...,%d], gb: [%d,...,%d]}\n"
           "  }\n  astatus: {illu: %s, vig: %f}\n", 
            info.opMode, info.en, info.bypass,
            info.stMan.sta.sw_lscT_meshGrid_mode,
            info.stMan.sta.meshGrid.posX_f[0], 
            info.stMan.sta.meshGrid.posX_f[1],
            info.stMan.sta.meshGrid.posY_f[0],
            info.stMan.sta.meshGrid.posY_f[1],
            info.stMan.dyn.meshGain.hw_lscC_gainR_val[0],
            info.stMan.dyn.meshGain.hw_lscC_gainR_val[288],
            info.stMan.dyn.meshGain.hw_lscC_gainGr_val[0],
            info.stMan.dyn.meshGain.hw_lscC_gainGr_val[288],
            info.stMan.dyn.meshGain.hw_lscC_gainB_val[0],
            info.stMan.dyn.meshGain.hw_lscC_gainB_val[288],
            info.stMan.dyn.meshGain.hw_lscC_gainGb_val[0],
            info.stMan.dyn.meshGain.hw_lscC_gainGb_val[288],
            info.alscStatus.sw_lscC_illuUsed_name,
            info.alscStatus.sw_lscC_vignetting_val);
    return 0;
}

int sample_lsc_setCalib_test(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    alsc_lscCalib_t calib;
    memset(&calib, 0, sizeof(alsc_lscCalib_t));
    //get
    ret = rk_aiq_user_api2_lsc_GetCalib(ctx, &calib);
    RKAIQ_SAMPLE_CHECK_RET(ret, "Get LSC CALIB failed!");
    printf("GetCALIB:\n\n");
    printf("\t effect Table_len = %d\n", calib.tableAll_len);
    for (int i = 0; i < calib.tableAll_len; i++) {
        printf("\t %s_%f = {r = [%d,...,%d], gr = [%d,...,%d], b = [%d,...,%d], gb = [%d,...,%d]}, \n",
                calib.tableAll[i].sw_lscC_illu_name,
                calib.tableAll[i].sw_lscC_vignetting_val,
                calib.tableAll[i].meshGain.hw_lscC_gainR_val[0],
                calib.tableAll[i].meshGain.hw_lscC_gainR_val[288],
                calib.tableAll[i].meshGain.hw_lscC_gainGr_val[0],
                calib.tableAll[i].meshGain.hw_lscC_gainGr_val[288],
                calib.tableAll[i].meshGain.hw_lscC_gainB_val[0],
                calib.tableAll[i].meshGain.hw_lscC_gainB_val[288],
                calib.tableAll[i].meshGain.hw_lscC_gainGb_val[0],
                calib.tableAll[i].meshGain.hw_lscC_gainGb_val[288]);
    }
    //modify
    srand(time(0));
    int rand_num = rand() % 101;

    if (rand_num <70) {
        printf("update lsc calib!\n");
        calib.tableAll[0].meshGain.hw_lscC_gainR_val[0] = 8191;
        calib.tableAll[0].meshGain.hw_lscC_gainR_val[16] = 8191;
        calib.tableAll[0].meshGain.hw_lscC_gainR_val[272] = 8191;
        calib.tableAll[0].meshGain.hw_lscC_gainR_val[288] = 8191;
        calib.tableAll[0].meshGain.hw_lscC_gainGr_val[0] = 1024;
        calib.tableAll[0].meshGain.hw_lscC_gainGr_val[16] = 1024;
        calib.tableAll[0].meshGain.hw_lscC_gainGr_val[272] = 1024;
        calib.tableAll[0].meshGain.hw_lscC_gainGr_val[288] = 1024;
        calib.tableAll[0].meshGain.hw_lscC_gainB_val[0] = 8191;
        calib.tableAll[0].meshGain.hw_lscC_gainB_val[16] = 8191;
        calib.tableAll[0].meshGain.hw_lscC_gainB_val[272] = 8191;
        calib.tableAll[0].meshGain.hw_lscC_gainB_val[288] = 8191;
        calib.tableAll[0].meshGain.hw_lscC_gainGb_val[0] = 1024;
        calib.tableAll[0].meshGain.hw_lscC_gainGb_val[16] = 1024;
        calib.tableAll[0].meshGain.hw_lscC_gainGb_val[272] = 1024;
        calib.tableAll[0].meshGain.hw_lscC_gainGb_val[288] = 1024;

        calib.tableAll[1].meshGain.hw_lscC_gainR_val[0] = 1024;
        calib.tableAll[1].meshGain.hw_lscC_gainR_val[16] = 1024;
        calib.tableAll[1].meshGain.hw_lscC_gainR_val[272] = 1024;
        calib.tableAll[1].meshGain.hw_lscC_gainR_val[288] = 1024;
        calib.tableAll[1].meshGain.hw_lscC_gainGr_val[0] = 4096;
        calib.tableAll[1].meshGain.hw_lscC_gainGr_val[16] = 4096;
        calib.tableAll[1].meshGain.hw_lscC_gainGr_val[272] = 4096;
        calib.tableAll[1].meshGain.hw_lscC_gainGr_val[288] = 4096;
        calib.tableAll[1].meshGain.hw_lscC_gainB_val[0] = 1024;
        calib.tableAll[1].meshGain.hw_lscC_gainB_val[16] = 1024;
        calib.tableAll[1].meshGain.hw_lscC_gainB_val[272] = 1024;
        calib.tableAll[1].meshGain.hw_lscC_gainB_val[288] = 1024;
        calib.tableAll[1].meshGain.hw_lscC_gainGb_val[0] = 4096;
        calib.tableAll[1].meshGain.hw_lscC_gainGb_val[16] = 4096;
        calib.tableAll[1].meshGain.hw_lscC_gainGb_val[272] = 4096;
        calib.tableAll[1].meshGain.hw_lscC_gainGb_val[288] = 4096;
    } else {
        memcpy(&calib.tableAll[0], &calib.tableAll[calib.tableAll_len-1], sizeof(alsc_tableAll_t));
        if (calib.tableAll_len > 1)
          calib.tableAll_len -= 1;
    }

    rk_aiq_user_api2_lsc_SetCalib(ctx, &calib);
    
    // wait more than 2 frames
    usleep(90 * 1000);

    alsc_lscCalib_t calib_new;
    memset(&calib_new, 0, sizeof(alsc_lscCalib_t));

    rk_aiq_user_api2_lsc_GetCalib(ctx, &calib_new);

    printf("\t new table_len = %d\n", calib_new.tableAll_len);
    for (int i = 0; i < calib_new.tableAll_len; i++) {
        printf("\t %s_%f = {r = [%d,...,%d], gr = [%d,...,%d], b = [%d,...,%d], gb = [%d,...,%d]}, \n",
                calib_new.tableAll[i].sw_lscC_illu_name,
                calib_new.tableAll[i].sw_lscC_vignetting_val,
                calib_new.tableAll[i].meshGain.hw_lscC_gainR_val[0],
                calib_new.tableAll[i].meshGain.hw_lscC_gainR_val[288],
                calib_new.tableAll[i].meshGain.hw_lscC_gainGr_val[0],
                calib_new.tableAll[i].meshGain.hw_lscC_gainGr_val[288],
                calib_new.tableAll[i].meshGain.hw_lscC_gainB_val[0],
                calib_new.tableAll[i].meshGain.hw_lscC_gainB_val[288],
                calib_new.tableAll[i].meshGain.hw_lscC_gainGb_val[0],
                calib_new.tableAll[i].meshGain.hw_lscC_gainGb_val[288]);
    }
    if (calib_new.tableAll_len != calib.tableAll_len || 
        calib_new.tableAll[0].meshGain.hw_lscC_gainR_val[0] != calib.tableAll[0].meshGain.hw_lscC_gainR_val[0])
        printf("lsc calib test failed\n");
    printf("-------- lsc module calib test done --------\n");  

    return 0;
}
#endif
uapi_case_t lsc_uapi_list[] = {
  { .desc = "ALSC: set lsc gain table async",
    .func = (uapi_case_func)sample_lsc_set_attr_async
  },
  { .desc = "ALSC: set lsc gain table sync",
    .func = (uapi_case_func)sample_lsc_set_attr_sync
  },
  { .desc = "ALSC: get lsc gain table",
    .func = (uapi_case_func)sample_lsc_get_attr
  },
#ifdef USE_NEWSTRUCT
  { .desc = "LSC: sample_lsc_test",
    .func = (uapi_case_func)sample_lsc_test
  },
  { .desc = "LSC: sample_query_lsc_status",
    .func = (uapi_case_func)sample_query_lsc_status
  },
  { .desc = "LSC: sample_lsc_setCalib_test",
    .func = (uapi_case_func)sample_lsc_setCalib_test
  },
  #endif
  {
    .desc = NULL,
    .func = NULL,
  }
};

XCamReturn sample_alsc_module(const void* arg)
{
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

  uapi_process_loop(ctx, lsc_uapi_list);

  return XCAM_RETURN_NO_ERROR;
}
