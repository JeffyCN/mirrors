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

static void sample_adrc_usage()
{
    printf("Usage : \n");
    printf("\t 0) ADRC:         test rk_aiq_user_api2_adrc_v10_SetAttrib stManual Sync.\n");
    printf("\t 1) ADRC:         test rk_aiq_user_api2_adrc_v10_GetAttrib stManual Async.\n");
    printf("\t 2) ADRC:         test rk_aiq_user_api2_adrc_v10_GetAttrib.\n");
    printf("\t 3) ADRC:         test rk_aiq_user_api2_adrc_v11_SetAttrib stManual Sync.\n");
    printf("\t 4) ADRC:         test rk_aiq_user_api2_adrc_v11_GetAttrib stManual Async.\n");
    printf("\t 5) ADRC:         test rk_aiq_user_api2_adrc_v11_GetAttrib.\n");
    printf("\t 6) ADRC:         test rk_aiq_user_api2_adrc_v12_SetAttrib stManual Sync.\n");
    printf("\t 7) ADRC:         test rk_aiq_user_api2_adrc_v12_GetAttrib stManual Async.\n");
    printf("\t 8) ADRC:         test rk_aiq_user_api2_adrc_v12_GetAttrib.\n");
    printf("\t 9) ADRC:         test rk_aiq_user_api2_adrc_v12_lite_SetAttrib stManual Sync.\n");
    printf("\t a) ADRC:         test rk_aiq_user_api2_adrc_v12_lite_GetAttrib stManual Async.\n");
    printf("\t b) ADRC:         test rk_aiq_user_api2_adrc_v12_lite_GetAttrib.\n");
    printf("\t c) ADRC:         test rk_aiq_user_api2_adrc_SetAttrib stManual Sync.\n");
    printf("\t d) ADRC:         test rk_aiq_user_api2_adrc_SetAttrib stManual Async.\n");
    printf("\t e) ADRC:         test rk_aiq_user_api2_adrc_GetAttrib.\n");
    printf("\t f) ADRC:         test rk_aiq_uapi2_getDrcGain.\n");
    printf("\t g) ADRC:         test rk_aiq_uapi2_setDrcGain.\n");
    printf("\t h) ADRC:         test rk_aiq_uapi2_getDrcHiLit.\n");
    printf("\t i) ADRC:         test rk_aiq_uapi2_setDrcHiLit.\n");
    printf("\t j) ADRC:         test rk_aiq_uapi2_getDrcLocalData.\n");
    printf("\t k) ADRC:         test rk_aiq_uapi2_setDrcLocalData.\n");
    printf("\t l) ADRC:         sample_new_drc.\n");
    printf("\t q) ADRC:         return to main sample screen.\n");  

    printf("\n");
    printf("\t please press the key: ");

    return;
}

void sample_print_adrc_info(const void *arg)
{
    printf ("enter ADRC modult test!\n");
}

#ifdef USE_NEWSTRUCT
static void sample_drc_tuningtool_test(const rk_aiq_sys_ctx_t* ctx)
{
    char *ret_str = NULL;

    printf(">>> start tuning tool test: op attrib get ...\n");

    std::string json_drc_status_str = " \n\
        [{ \n\
            \"op\":\"get\", \n\
            \"path\": \"/uapi/0/drc_uapi/info\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 0,\"bypass\": 3} \n\
        }]";

    rkaiq_uapi_unified_ctl(const_cast<rk_aiq_sys_ctx_t*>(ctx),
                           const_cast<char*>(json_drc_status_str.c_str()), &ret_str, RKAIQUAPI_OPMODE_GET);

    if (ret_str) {
        printf("drc status json str: %s\n", ret_str);
    }

    printf("  start tuning tool test: op attrib set ...\n");
    std::string json_drc_str = " \n\
        [{ \n\
            \"op\":\"replace\", \n\
            \"path\": \"/uapi/0/drc_uapi/attr\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_MANUAL\", \"en\": 1,\"bypass\": 1} \n\
        }]";
    printf("drc json_cmd_str: %s\n", json_drc_str.c_str());
    ret_str = NULL;
    rkaiq_uapi_unified_ctl(const_cast<rk_aiq_sys_ctx_t*>(ctx),
                           const_cast<char*>(json_drc_str.c_str()), &ret_str, RKAIQUAPI_OPMODE_SET);

    // wait more than 2 frames
    usleep(90 * 1000);

    drc_status_t status;
    memset(&status, 0, sizeof(drc_status_t));

    rk_aiq_user_api2_drc_QueryStatus(ctx, &status);

    if (status.opMode != RK_AIQ_OP_MODE_MANUAL || status.en != 1 || status.bypass != 1) {
        printf("drc op set_attrib failed !\n");
        printf("drc status: opmode:%d(EXP:%d), en:%d(EXP:%d), bypass:%d(EXP:%d)\n",
               status.opMode, RK_AIQ_OP_MODE_MANUAL, status.en, 1, status.bypass, 1);
    } else {
        printf("drc op set_attrib success !\n");
    }

    printf(">>> tuning tool test done \n");
}

static void sample_new_drc(const rk_aiq_sys_ctx_t* ctx) {
    sample_drc_tuningtool_test(ctx);

    drc_api_attrib_t attr_drc;
    drc_status_t status;
    rk_aiq_user_api2_drc_GetAttrib(ctx, &attr_drc);
    printf("\t attr_drc.opMode:%d attr_drc.en:%d\n\n",
            attr_drc.opMode, attr_drc.en);

    // attr_drc.opMode = RK_AIQ_OP_MODE_MANUAL;
    // attr_drc.stMan.sta.Edge_Weit += 0.1;
    // rk_aiq_user_api2_drc_SetAttrib(ctx, &attr_drc);

    attr_drc.opMode = RK_AIQ_OP_MODE_AUTO;
    rk_aiq_user_api2_drc_SetAttrib(ctx, &attr_drc);

    rk_aiq_user_api2_drc_QueryStatus(ctx, &status);
    printf("\t status.opMode:%d status.en:%d\n\n",
            status.opMode, status.en);
}
#endif

XCamReturn sample_adrc_module(const void *arg)
{
    int key = -1;
    CLEAR();

    drcAttrV10_t attr_v10;
    drcAttrV11_t attr_v11;
    drcAttrV12_t attr_v12;
    drcAttrV12Lite_t attr_v12_lite;
    drc_attrib_t attr_v2;
    const demo_context_t *demo_ctx = (demo_context_t *)arg;
    const rk_aiq_sys_ctx_t* ctx ;
    if (demo_ctx->camGroup) {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->camgroup_ctx);
    } else {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->aiq_ctx);
    }

    do {
        sample_adrc_usage ();

        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar();
        printf ("\n");

        switch (key) {
            case '0': {
                printf("\t ADRC test rk_aiq_user_api2_adrc_v10_SetAttrib stManual Sync\n\n");
                attr_v10.sync.sync_mode                                  = RK_AIQ_UAPI_MODE_DEFAULT;
                attr_v10.sync.done                                       = false;
                attr_v10.opMode                                          = DRC_OPMODE_MANUAL;
                attr_v10.stManual.Enable                                 = true;
                attr_v10.stManual.DrcGain.Alpha                          = 0.2f;
                attr_v10.stManual.DrcGain.DrcGain                        = 2.0f;
                attr_v10.stManual.DrcGain.Clip                           = 1.0f;
                attr_v10.stManual.HiLight.Strength                       = 0.5f;
                attr_v10.stManual.LocalTMOSetting.LocalTMOData.LocalWeit = 1.0f;
                attr_v10.stManual.LocalTMOSetting.LocalTMOData.GlobalContrast = 0.0f;
                attr_v10.stManual.LocalTMOSetting.LocalTMOData.LoLitContrast  = 0.0f;
                attr_v10.stManual.LocalTMOSetting.curPixWeit                  = 0.376471f;
                attr_v10.stManual.LocalTMOSetting.preFrameWeit                = 0.8f;
                attr_v10.stManual.LocalTMOSetting.Range_force_sgm             = 0.0f;
                attr_v10.stManual.LocalTMOSetting.Range_sgm_cur               = 0.2f;
                attr_v10.stManual.LocalTMOSetting.Range_sgm_pre               = 0.2f;
                attr_v10.stManual.LocalTMOSetting.Space_sgm_cur               = 4068;
                attr_v10.stManual.LocalTMOSetting.Space_sgm_pre               = 3068;
                attr_v10.stManual.CompressSetting.Mode                        = COMPRESS_AUTO;
                attr_v10.stManual.CompressSetting.Manual_curve[0]             = 0;
                attr_v10.stManual.CompressSetting.Manual_curve[1]             = 558;
                attr_v10.stManual.CompressSetting.Manual_curve[2]             = 1087;
                attr_v10.stManual.CompressSetting.Manual_curve[3]             = 1588;
                attr_v10.stManual.CompressSetting.Manual_curve[4]             = 2063;
                attr_v10.stManual.CompressSetting.Manual_curve[5]             = 2515;
                attr_v10.stManual.CompressSetting.Manual_curve[6]             = 2944;
                attr_v10.stManual.CompressSetting.Manual_curve[7]             = 3353;
                attr_v10.stManual.CompressSetting.Manual_curve[8]             = 3744;
                attr_v10.stManual.CompressSetting.Manual_curve[9]             = 4473;
                attr_v10.stManual.CompressSetting.Manual_curve[10]            = 5139;
                attr_v10.stManual.CompressSetting.Manual_curve[11]            = 5751;
                attr_v10.stManual.CompressSetting.Manual_curve[12]            = 6316;
                attr_v10.stManual.CompressSetting.Manual_curve[13]            = 6838;
                attr_v10.stManual.CompressSetting.Manual_curve[14]            = 7322;
                attr_v10.stManual.CompressSetting.Manual_curve[15]            = 7772;
                attr_v10.stManual.CompressSetting.Manual_curve[16]            = 8192;
                attr_v10.stManual.Scale_y[0]                                  = 0;
                attr_v10.stManual.Scale_y[1]                                  = 2;
                attr_v10.stManual.Scale_y[2]                                  = 20;
                attr_v10.stManual.Scale_y[3]                                  = 76;
                attr_v10.stManual.Scale_y[4]                                  = 193;
                attr_v10.stManual.Scale_y[5]                                  = 381;
                attr_v10.stManual.Scale_y[6]                                  = 631;
                attr_v10.stManual.Scale_y[7]                                  = 772;
                attr_v10.stManual.Scale_y[8]                                  = 919;
                attr_v10.stManual.Scale_y[9]                                  = 1066;
                attr_v10.stManual.Scale_y[10]                                 = 1210;
                attr_v10.stManual.Scale_y[11]                                 = 1479;
                attr_v10.stManual.Scale_y[12]                                 = 1700;
                attr_v10.stManual.Scale_y[13]                                 = 1863;
                attr_v10.stManual.Scale_y[14]                                 = 1968;
                attr_v10.stManual.Scale_y[15]                                 = 2024;
                attr_v10.stManual.Scale_y[16]                                 = 2048;
                attr_v10.stManual.Edge_Weit                                   = 0.02f;
                attr_v10.stManual.OutPutLongFrame                             = false;
                attr_v10.stManual.IIR_frame                                   = 2;
                rk_aiq_user_api2_adrc_v10_SetAttrib(ctx, &attr_v10);
                break;
            }
            case '1': {
                printf("\t ADRC test rk_aiq_user_api2_adrc_v10_SetAttrib stManual Async\n\n");
                attr_v10.sync.sync_mode                                  = RK_AIQ_UAPI_MODE_ASYNC;
                attr_v10.sync.done                                       = false;
                attr_v10.opMode                                          = DRC_OPMODE_MANUAL;
                attr_v10.stManual.Enable                                 = true;
                attr_v10.stManual.DrcGain.Alpha                          = 0.1f;
                attr_v10.stManual.DrcGain.DrcGain                        = 2.0f;
                attr_v10.stManual.DrcGain.Clip                           = 1.0f;
                attr_v10.stManual.HiLight.Strength                       = 0.5f;
                attr_v10.stManual.LocalTMOSetting.LocalTMOData.LocalWeit = 1.0f;
                attr_v10.stManual.LocalTMOSetting.LocalTMOData.GlobalContrast = 0.5f;
                attr_v10.stManual.LocalTMOSetting.LocalTMOData.LoLitContrast  = 0.5f;
                attr_v10.stManual.LocalTMOSetting.curPixWeit                  = 0.376471f;
                attr_v10.stManual.LocalTMOSetting.preFrameWeit                = 0.8f;
                attr_v10.stManual.LocalTMOSetting.Range_force_sgm             = 0.0f;
                attr_v10.stManual.LocalTMOSetting.Range_sgm_cur               = 0.2f;
                attr_v10.stManual.LocalTMOSetting.Range_sgm_pre               = 0.2f;
                attr_v10.stManual.LocalTMOSetting.Space_sgm_cur               = 4068;
                attr_v10.stManual.LocalTMOSetting.Space_sgm_pre               = 3068;
                attr_v10.stManual.CompressSetting.Mode                        = COMPRESS_AUTO;
                attr_v10.stManual.CompressSetting.Manual_curve[0]             = 0;
                attr_v10.stManual.CompressSetting.Manual_curve[1]             = 558;
                attr_v10.stManual.CompressSetting.Manual_curve[2]             = 1087;
                attr_v10.stManual.CompressSetting.Manual_curve[3]             = 1588;
                attr_v10.stManual.CompressSetting.Manual_curve[4]             = 2063;
                attr_v10.stManual.CompressSetting.Manual_curve[5]             = 2515;
                attr_v10.stManual.CompressSetting.Manual_curve[6]             = 2944;
                attr_v10.stManual.CompressSetting.Manual_curve[7]             = 3353;
                attr_v10.stManual.CompressSetting.Manual_curve[8]             = 3744;
                attr_v10.stManual.CompressSetting.Manual_curve[9]             = 4473;
                attr_v10.stManual.CompressSetting.Manual_curve[10]            = 5139;
                attr_v10.stManual.CompressSetting.Manual_curve[11]            = 5751;
                attr_v10.stManual.CompressSetting.Manual_curve[12]            = 6316;
                attr_v10.stManual.CompressSetting.Manual_curve[13]            = 6838;
                attr_v10.stManual.CompressSetting.Manual_curve[14]            = 7322;
                attr_v10.stManual.CompressSetting.Manual_curve[15]            = 7772;
                attr_v10.stManual.CompressSetting.Manual_curve[16]            = 8192;
                attr_v10.stManual.Scale_y[0]                                  = 0;
                attr_v10.stManual.Scale_y[1]                                  = 2;
                attr_v10.stManual.Scale_y[2]                                  = 20;
                attr_v10.stManual.Scale_y[3]                                  = 76;
                attr_v10.stManual.Scale_y[4]                                  = 193;
                attr_v10.stManual.Scale_y[5]                                  = 381;
                attr_v10.stManual.Scale_y[6]                                  = 631;
                attr_v10.stManual.Scale_y[7]                                  = 772;
                attr_v10.stManual.Scale_y[8]                                  = 919;
                attr_v10.stManual.Scale_y[9]                                  = 1066;
                attr_v10.stManual.Scale_y[10]                                 = 1210;
                attr_v10.stManual.Scale_y[11]                                 = 1479;
                attr_v10.stManual.Scale_y[12]                                 = 1700;
                attr_v10.stManual.Scale_y[13]                                 = 1863;
                attr_v10.stManual.Scale_y[14]                                 = 1968;
                attr_v10.stManual.Scale_y[15]                                 = 2024;
                attr_v10.stManual.Scale_y[16]                                 = 2048;
                attr_v10.stManual.Edge_Weit                                   = 0.02f;
                attr_v10.stManual.OutPutLongFrame                             = false;
                attr_v10.stManual.IIR_frame                                   = 2;
                rk_aiq_user_api2_adrc_v10_SetAttrib(ctx, &attr_v10);
                break;
            }
            case '2': {
                printf("\t ADRC test rk_aiq_user_api2_adrc_GetAttrib\n\n");
                rk_aiq_user_api2_adrc_v10_GetAttrib(ctx, &attr_v10);
                printf("\t sync = %d, done = %d\n", attr_v10.sync.sync_mode, attr_v10.sync.done);
                printf("\t opMode:%d EnvLv:%f ISO:%f\n\n", attr_v10.opMode,
                       attr_v10.Info.CtrlInfo.EnvLv, attr_v10.Info.CtrlInfo.ISO);
                printf("\t stManual DrcGain:%f Alpha:%f Clip:%f\n\n",
                       attr_v10.stManual.DrcGain.DrcGain, attr_v10.stManual.DrcGain.Alpha,
                       attr_v10.stManual.DrcGain.Clip);
                break;
            }
            case '3': {
                printf("\t ADRC test rk_aiq_user_api2_adrc_v11_SetAttrib stManual Sync\n\n");
                attr_v11.sync.sync_mode                                  = RK_AIQ_UAPI_MODE_DEFAULT;
                attr_v11.sync.done                                       = false;
                attr_v11.opMode                                          = DRC_OPMODE_MANUAL;
                attr_v11.stManual.Enable                                 = true;
                attr_v11.stManual.DrcGain.Alpha                          = 0.2f;
                attr_v11.stManual.DrcGain.DrcGain                        = 2.0f;
                attr_v11.stManual.DrcGain.Clip                           = 1.0f;
                attr_v11.stManual.HiLight.Strength                       = 0.5f;
                attr_v11.stManual.LocalSetting.LocalData.LocalWeit       = 1.0f;
                attr_v11.stManual.LocalSetting.LocalData.LocalAutoEnable = 1;
                attr_v11.stManual.LocalSetting.LocalData.LocalAutoWeit   = 0.5f;
                attr_v11.stManual.LocalSetting.LocalData.GlobalContrast  = 0.5f;
                attr_v11.stManual.LocalSetting.LocalData.LoLitContrast   = 0.5f;
                attr_v11.stManual.LocalSetting.curPixWeit                = 0.376471f;
                attr_v11.stManual.LocalSetting.preFrameWeit              = 0.8f;
                attr_v11.stManual.LocalSetting.Range_force_sgm           = 0.0f;
                attr_v11.stManual.LocalSetting.Range_sgm_cur             = 0.2f;
                attr_v11.stManual.LocalSetting.Range_sgm_pre             = 0.2f;
                attr_v11.stManual.LocalSetting.Space_sgm_cur             = 4068;
                attr_v11.stManual.LocalSetting.Space_sgm_pre             = 3068;
                attr_v11.stManual.CompressSetting.Mode                   = COMPRESS_AUTO;
                attr_v11.stManual.CompressSetting.Manual_curve[0]        = 0;
                attr_v11.stManual.CompressSetting.Manual_curve[1]        = 558;
                attr_v11.stManual.CompressSetting.Manual_curve[2]        = 1087;
                attr_v11.stManual.CompressSetting.Manual_curve[3]        = 1588;
                attr_v11.stManual.CompressSetting.Manual_curve[4]        = 2063;
                attr_v11.stManual.CompressSetting.Manual_curve[5]        = 2515;
                attr_v11.stManual.CompressSetting.Manual_curve[6]        = 2944;
                attr_v11.stManual.CompressSetting.Manual_curve[7]        = 3353;
                attr_v11.stManual.CompressSetting.Manual_curve[8]        = 3744;
                attr_v11.stManual.CompressSetting.Manual_curve[9]        = 4473;
                attr_v11.stManual.CompressSetting.Manual_curve[10]       = 5139;
                attr_v11.stManual.CompressSetting.Manual_curve[11]       = 5751;
                attr_v11.stManual.CompressSetting.Manual_curve[12]       = 6316;
                attr_v11.stManual.CompressSetting.Manual_curve[13]       = 6838;
                attr_v11.stManual.CompressSetting.Manual_curve[14]       = 7322;
                attr_v11.stManual.CompressSetting.Manual_curve[15]       = 7772;
                attr_v11.stManual.CompressSetting.Manual_curve[16]       = 8192;
                attr_v11.stManual.Scale_y[0]                             = 0;
                attr_v11.stManual.Scale_y[1]                             = 2;
                attr_v11.stManual.Scale_y[2]                             = 20;
                attr_v11.stManual.Scale_y[3]                             = 76;
                attr_v11.stManual.Scale_y[4]                             = 193;
                attr_v11.stManual.Scale_y[5]                             = 381;
                attr_v11.stManual.Scale_y[6]                             = 631;
                attr_v11.stManual.Scale_y[7]                             = 772;
                attr_v11.stManual.Scale_y[8]                             = 919;
                attr_v11.stManual.Scale_y[9]                             = 1066;
                attr_v11.stManual.Scale_y[10]                            = 1211;
                attr_v11.stManual.Scale_y[11]                            = 1479;
                attr_v11.stManual.Scale_y[12]                            = 1700;
                attr_v11.stManual.Scale_y[13]                            = 1863;
                attr_v11.stManual.Scale_y[14]                            = 1968;
                attr_v11.stManual.Scale_y[15]                            = 2024;
                attr_v11.stManual.Scale_y[16]                            = 2048;
                attr_v11.stManual.Edge_Weit                              = 0.02f;
                attr_v11.stManual.OutPutLongFrame                        = false;
                attr_v11.stManual.IIR_frame                              = 2;
                rk_aiq_user_api2_adrc_v11_SetAttrib(ctx, &attr_v11);
                break;
            }
            case '4': {
                printf("\t ADRC test rk_aiq_user_api2_adrc_v11_SetAttrib stManual Async\n\n");
                attr_v11.sync.sync_mode                                  = RK_AIQ_UAPI_MODE_ASYNC;
                attr_v11.sync.done                                       = false;
                attr_v11.opMode                                          = DRC_OPMODE_MANUAL;
                attr_v11.stManual.Enable                                 = true;
                attr_v11.stManual.DrcGain.Alpha                          = 0.1f;
                attr_v11.stManual.DrcGain.DrcGain                        = 2.0f;
                attr_v11.stManual.DrcGain.Clip                           = 1.0f;
                attr_v11.stManual.HiLight.Strength                       = 0.5f;
                attr_v11.stManual.LocalSetting.LocalData.LocalWeit       = 1.0f;
                attr_v11.stManual.LocalSetting.LocalData.LocalAutoEnable = 1;
                attr_v11.stManual.LocalSetting.LocalData.LocalAutoWeit   = 0.5f;
                attr_v11.stManual.LocalSetting.LocalData.GlobalContrast  = 0.5f;
                attr_v11.stManual.LocalSetting.LocalData.LoLitContrast   = 0.5f;
                attr_v11.stManual.LocalSetting.curPixWeit                = 0.376471f;
                attr_v11.stManual.LocalSetting.preFrameWeit              = 0.8f;
                attr_v11.stManual.LocalSetting.Range_force_sgm           = 0.0f;
                attr_v11.stManual.LocalSetting.Range_sgm_cur             = 0.2f;
                attr_v11.stManual.LocalSetting.Range_sgm_pre             = 0.2f;
                attr_v11.stManual.LocalSetting.Space_sgm_cur             = 4068;
                attr_v11.stManual.LocalSetting.Space_sgm_pre             = 3068;
                attr_v11.stManual.CompressSetting.Mode                   = COMPRESS_AUTO;
                attr_v11.stManual.CompressSetting.Manual_curve[0]        = 0;
                attr_v11.stManual.CompressSetting.Manual_curve[1]        = 558;
                attr_v11.stManual.CompressSetting.Manual_curve[2]        = 1087;
                attr_v11.stManual.CompressSetting.Manual_curve[3]        = 1588;
                attr_v11.stManual.CompressSetting.Manual_curve[4]        = 2063;
                attr_v11.stManual.CompressSetting.Manual_curve[5]        = 2515;
                attr_v11.stManual.CompressSetting.Manual_curve[6]        = 2944;
                attr_v11.stManual.CompressSetting.Manual_curve[7]        = 3353;
                attr_v11.stManual.CompressSetting.Manual_curve[8]        = 3744;
                attr_v11.stManual.CompressSetting.Manual_curve[9]        = 4473;
                attr_v11.stManual.CompressSetting.Manual_curve[10]       = 5139;
                attr_v11.stManual.CompressSetting.Manual_curve[11]       = 5751;
                attr_v11.stManual.CompressSetting.Manual_curve[12]       = 6316;
                attr_v11.stManual.CompressSetting.Manual_curve[13]       = 6838;
                attr_v11.stManual.CompressSetting.Manual_curve[14]       = 7322;
                attr_v11.stManual.CompressSetting.Manual_curve[15]       = 7772;
                attr_v11.stManual.CompressSetting.Manual_curve[16]       = 8192;
                attr_v11.stManual.Scale_y[0]                             = 0;
                attr_v11.stManual.Scale_y[1]                             = 2;
                attr_v11.stManual.Scale_y[2]                             = 20;
                attr_v11.stManual.Scale_y[3]                             = 76;
                attr_v11.stManual.Scale_y[4]                             = 193;
                attr_v11.stManual.Scale_y[5]                             = 381;
                attr_v11.stManual.Scale_y[6]                             = 631;
                attr_v11.stManual.Scale_y[7]                             = 772;
                attr_v11.stManual.Scale_y[8]                             = 919;
                attr_v11.stManual.Scale_y[9]                             = 1066;
                attr_v11.stManual.Scale_y[10]                            = 1211;
                attr_v11.stManual.Scale_y[11]                            = 1479;
                attr_v11.stManual.Scale_y[12]                            = 1700;
                attr_v11.stManual.Scale_y[13]                            = 1863;
                attr_v11.stManual.Scale_y[14]                            = 1968;
                attr_v11.stManual.Scale_y[15]                            = 2024;
                attr_v11.stManual.Scale_y[16]                            = 2048;
                attr_v11.stManual.Edge_Weit                              = 0.02f;
                attr_v11.stManual.OutPutLongFrame                        = false;
                attr_v11.stManual.IIR_frame                              = 2;
                rk_aiq_user_api2_adrc_v11_SetAttrib(ctx, &attr_v11);
                break;
            }
            case '5': {
                printf("\t ADRC test rk_aiq_user_api2_adrc_GetAttrib\n\n");
                rk_aiq_user_api2_adrc_v11_GetAttrib(ctx, &attr_v11);
                printf("\t sync = %d, done = %d\n", attr_v11.sync.sync_mode, attr_v11.sync.done);
                printf("\t opMode:%d EnvLv:%f ISO:%f\n\n", attr_v11.opMode,
                       attr_v11.Info.CtrlInfo.EnvLv, attr_v11.Info.CtrlInfo.ISO);
                printf("\t stManual DrcGain:%f Alpha:%f Clip:%f\n\n",
                       attr_v11.stManual.DrcGain.DrcGain, attr_v11.stManual.DrcGain.Alpha,
                       attr_v11.stManual.DrcGain.Clip);
                break;
            }
            case '6': {
                printf("\t ADRC test rk_aiq_user_api2_adrc_v12_SetAttrib stManual Sync\n\n");
                attr_v12.sync.sync_mode                                  = RK_AIQ_UAPI_MODE_DEFAULT;
                attr_v12.sync.done                                       = false;
                attr_v12.opMode                                          = DRC_OPMODE_MANUAL;
                attr_v12.stManual.Enable                                 = true;
                attr_v12.stManual.DrcGain.Alpha                          = 0.2f;
                attr_v12.stManual.DrcGain.DrcGain                        = 2.0f;
                attr_v12.stManual.DrcGain.Clip                           = 1.0f;
                attr_v12.stManual.HiLight.HiLightData.Strength           = 0.5f;
                attr_v12.stManual.HiLight.HiLightData.gas_t              = 0.5f;
                attr_v12.stManual.HiLight.gas_l0                         = 24;
                attr_v12.stManual.HiLight.gas_l1                         = 10;
                attr_v12.stManual.HiLight.gas_l2                         = 10;
                attr_v12.stManual.HiLight.gas_l3                         = 5;
                attr_v12.stManual.LocalSetting.LocalData.LocalWeit       = 1.0f;
                attr_v12.stManual.LocalSetting.LocalData.LocalAutoEnable = 1;
                attr_v12.stManual.LocalSetting.LocalData.LocalAutoWeit   = 0.5f;
                attr_v12.stManual.LocalSetting.LocalData.GlobalContrast  = 0.5f;
                attr_v12.stManual.LocalSetting.LocalData.LoLitContrast   = 0.5f;
                attr_v12.stManual.LocalSetting.MotionData.MotionStr      = 0.0f;
                attr_v12.stManual.LocalSetting.curPixWeit                = 0.376471f;
                attr_v12.stManual.LocalSetting.preFrameWeit              = 0.8f;
                attr_v12.stManual.LocalSetting.Range_force_sgm           = 0.0f;
                attr_v12.stManual.LocalSetting.Range_sgm_cur             = 0.2f;
                attr_v12.stManual.LocalSetting.Range_sgm_pre             = 0.2f;
                attr_v12.stManual.LocalSetting.Space_sgm_cur             = 4068;
                attr_v12.stManual.LocalSetting.Space_sgm_pre             = 3068;
                attr_v12.stManual.CompressSetting.Mode                   = COMPRESS_AUTO;
                attr_v12.stManual.CompressSetting.Manual_curve[0]        = 0;
                attr_v12.stManual.CompressSetting.Manual_curve[1]        = 558;
                attr_v12.stManual.CompressSetting.Manual_curve[2]        = 1087;
                attr_v12.stManual.CompressSetting.Manual_curve[3]        = 1588;
                attr_v12.stManual.CompressSetting.Manual_curve[4]        = 2063;
                attr_v12.stManual.CompressSetting.Manual_curve[5]        = 2515;
                attr_v12.stManual.CompressSetting.Manual_curve[6]        = 2944;
                attr_v12.stManual.CompressSetting.Manual_curve[7]        = 3353;
                attr_v12.stManual.CompressSetting.Manual_curve[8]        = 3744;
                attr_v12.stManual.CompressSetting.Manual_curve[9]        = 4473;
                attr_v12.stManual.CompressSetting.Manual_curve[10]       = 5139;
                attr_v12.stManual.CompressSetting.Manual_curve[11]       = 5751;
                attr_v12.stManual.CompressSetting.Manual_curve[12]       = 6316;
                attr_v12.stManual.CompressSetting.Manual_curve[13]       = 6838;
                attr_v12.stManual.CompressSetting.Manual_curve[14]       = 7322;
                attr_v12.stManual.CompressSetting.Manual_curve[15]       = 7772;
                attr_v12.stManual.CompressSetting.Manual_curve[16]       = 8192;
                attr_v12.stManual.Scale_y[0]                             = 0;
                attr_v12.stManual.Scale_y[1]                             = 2;
                attr_v12.stManual.Scale_y[2]                             = 20;
                attr_v12.stManual.Scale_y[3]                             = 76;
                attr_v12.stManual.Scale_y[4]                             = 193;
                attr_v12.stManual.Scale_y[5]                             = 381;
                attr_v12.stManual.Scale_y[6]                             = 631;
                attr_v12.stManual.Scale_y[7]                             = 772;
                attr_v12.stManual.Scale_y[8]                             = 919;
                attr_v12.stManual.Scale_y[9]                             = 1066;
                attr_v12.stManual.Scale_y[10]                            = 1212;
                attr_v12.stManual.Scale_y[11]                            = 1479;
                attr_v12.stManual.Scale_y[12]                            = 1700;
                attr_v12.stManual.Scale_y[13]                            = 1863;
                attr_v12.stManual.Scale_y[14]                            = 1968;
                attr_v12.stManual.Scale_y[15]                            = 2024;
                attr_v12.stManual.Scale_y[16]                            = 2048;
                attr_v12.stManual.Edge_Weit                              = 0.02f;
                attr_v12.stManual.OutPutLongFrame                        = false;
                attr_v12.stManual.IIR_frame                              = 2;
                rk_aiq_user_api2_adrc_v12_SetAttrib(ctx, &attr_v12);
                break;
            }
            case '7': {
                printf("\t ADRC test rk_aiq_user_api2_adrc_v12_SetAttrib stManual Async\n\n");
                attr_v12.sync.sync_mode                                  = RK_AIQ_UAPI_MODE_ASYNC;
                attr_v12.sync.done                                       = false;
                attr_v12.opMode                                          = DRC_OPMODE_MANUAL;
                attr_v12.stManual.Enable                                 = true;
                attr_v12.stManual.DrcGain.Alpha                          = 0.1f;
                attr_v12.stManual.DrcGain.DrcGain                        = 2.0f;
                attr_v12.stManual.DrcGain.Clip                           = 1.0f;
                attr_v12.stManual.HiLight.HiLightData.Strength           = 0.5f;
                attr_v12.stManual.HiLight.HiLightData.gas_t              = 0.5f;
                attr_v12.stManual.HiLight.gas_l0                         = 24;
                attr_v12.stManual.HiLight.gas_l1                         = 10;
                attr_v12.stManual.HiLight.gas_l2                         = 10;
                attr_v12.stManual.HiLight.gas_l3                         = 5;
                attr_v12.stManual.LocalSetting.LocalData.LocalWeit       = 1.0f;
                attr_v12.stManual.LocalSetting.LocalData.LocalAutoEnable = 1;
                attr_v12.stManual.LocalSetting.LocalData.LocalAutoWeit   = 0.5f;
                attr_v12.stManual.LocalSetting.LocalData.GlobalContrast  = 0.5f;
                attr_v12.stManual.LocalSetting.LocalData.LoLitContrast   = 0.5f;
                attr_v12.stManual.LocalSetting.MotionData.MotionStr      = 0.0f;
                attr_v12.stManual.LocalSetting.curPixWeit                = 0.376471f;
                attr_v12.stManual.LocalSetting.preFrameWeit              = 0.8f;
                attr_v12.stManual.LocalSetting.Range_force_sgm           = 0.0f;
                attr_v12.stManual.LocalSetting.Range_sgm_cur             = 0.2f;
                attr_v12.stManual.LocalSetting.Range_sgm_pre             = 0.2f;
                attr_v12.stManual.LocalSetting.Space_sgm_cur             = 4068;
                attr_v12.stManual.LocalSetting.Space_sgm_pre             = 3068;
                attr_v12.stManual.CompressSetting.Mode                   = COMPRESS_AUTO;
                attr_v12.stManual.CompressSetting.Manual_curve[0]        = 0;
                attr_v12.stManual.CompressSetting.Manual_curve[1]        = 558;
                attr_v12.stManual.CompressSetting.Manual_curve[2]        = 1087;
                attr_v12.stManual.CompressSetting.Manual_curve[3]        = 1588;
                attr_v12.stManual.CompressSetting.Manual_curve[4]        = 2063;
                attr_v12.stManual.CompressSetting.Manual_curve[5]        = 2515;
                attr_v12.stManual.CompressSetting.Manual_curve[6]        = 2944;
                attr_v12.stManual.CompressSetting.Manual_curve[7]        = 3353;
                attr_v12.stManual.CompressSetting.Manual_curve[8]        = 3744;
                attr_v12.stManual.CompressSetting.Manual_curve[9]        = 4473;
                attr_v12.stManual.CompressSetting.Manual_curve[10]       = 5139;
                attr_v12.stManual.CompressSetting.Manual_curve[11]       = 5751;
                attr_v12.stManual.CompressSetting.Manual_curve[12]       = 6316;
                attr_v12.stManual.CompressSetting.Manual_curve[13]       = 6838;
                attr_v12.stManual.CompressSetting.Manual_curve[14]       = 7322;
                attr_v12.stManual.CompressSetting.Manual_curve[15]       = 7772;
                attr_v12.stManual.CompressSetting.Manual_curve[16]       = 8192;
                attr_v12.stManual.Scale_y[0]                             = 0;
                attr_v12.stManual.Scale_y[1]                             = 2;
                attr_v12.stManual.Scale_y[2]                             = 20;
                attr_v12.stManual.Scale_y[3]                             = 76;
                attr_v12.stManual.Scale_y[4]                             = 193;
                attr_v12.stManual.Scale_y[5]                             = 381;
                attr_v12.stManual.Scale_y[6]                             = 631;
                attr_v12.stManual.Scale_y[7]                             = 772;
                attr_v12.stManual.Scale_y[8]                             = 919;
                attr_v12.stManual.Scale_y[9]                             = 1066;
                attr_v12.stManual.Scale_y[10]                            = 1212;
                attr_v12.stManual.Scale_y[11]                            = 1479;
                attr_v12.stManual.Scale_y[12]                            = 1700;
                attr_v12.stManual.Scale_y[13]                            = 1863;
                attr_v12.stManual.Scale_y[14]                            = 1968;
                attr_v12.stManual.Scale_y[15]                            = 2024;
                attr_v12.stManual.Scale_y[16]                            = 2048;
                attr_v12.stManual.Edge_Weit                              = 0.02f;
                attr_v12.stManual.OutPutLongFrame                        = false;
                attr_v12.stManual.IIR_frame                              = 2;
                rk_aiq_user_api2_adrc_v12_SetAttrib(ctx, &attr_v12);
                break;
            }
            case '8': {
                printf("\t ADRC test rk_aiq_user_api2_adrc_GetAttrib\n\n");
                rk_aiq_user_api2_adrc_v12_GetAttrib(ctx, &attr_v12);
                printf("\t sync = %d, done = %d\n", attr_v12.sync.sync_mode, attr_v12.sync.done);
                printf("\t opMode:%d EnvLv:%f ISO:%f\n\n", attr_v12.opMode,
                       attr_v12.Info.CtrlInfo.EnvLv, attr_v12.Info.CtrlInfo.ISO);
                printf("\t stManual DrcGain:%f Alpha:%f Clip:%f\n\n",
                       attr_v12.stManual.DrcGain.DrcGain, attr_v12.stManual.DrcGain.Alpha,
                       attr_v12.stManual.DrcGain.Clip);
                break;
            }
            case '9': {
                printf("\t ADRC test rk_aiq_user_api2_adrc_v12_lite_SetAttrib stManual Sync\n\n");
                attr_v12_lite.sync.sync_mode                            = RK_AIQ_UAPI_MODE_DEFAULT;
                attr_v12_lite.sync.done                                 = false;
                attr_v12_lite.opMode                                    = DRC_OPMODE_MANUAL;
                attr_v12_lite.stManual.Enable                           = true;
                attr_v12_lite.stManual.DrcGain.Alpha                    = 0.2f;
                attr_v12_lite.stManual.DrcGain.DrcGain                  = 2.0f;
                attr_v12_lite.stManual.DrcGain.Clip                     = 1.0f;
                attr_v12_lite.stManual.HiLight.HiLightData.Strength     = 0.5f;
                attr_v12_lite.stManual.HiLight.HiLightData.gas_t        = 0.5f;
                attr_v12_lite.stManual.HiLight.gas_l0                   = 24;
                attr_v12_lite.stManual.HiLight.gas_l1                   = 10;
                attr_v12_lite.stManual.HiLight.gas_l2                   = 10;
                attr_v12_lite.stManual.HiLight.gas_l3                   = 5;
                attr_v12_lite.stManual.LocalSetting.LocalData.LocalWeit = 1.0f;
                attr_v12_lite.stManual.LocalSetting.LocalData.LocalAutoEnable = 1;
                attr_v12_lite.stManual.LocalSetting.LocalData.LocalAutoWeit   = 0.5f;
                attr_v12_lite.stManual.LocalSetting.LocalData.GlobalContrast  = 0.5f;
                attr_v12_lite.stManual.LocalSetting.LocalData.LoLitContrast   = 0.5f;
                attr_v12_lite.stManual.LocalSetting.MotionData.MotionStr      = 0.0f;
                attr_v12_lite.stManual.LocalSetting.curPixWeit                = 0.376471f;
                attr_v12_lite.stManual.LocalSetting.Range_force_sgm           = 0.0f;
                attr_v12_lite.stManual.LocalSetting.Range_sgm_cur             = 0.2f;
                attr_v12_lite.stManual.LocalSetting.Space_sgm_cur             = 4068;
                attr_v12_lite.stManual.CompressSetting.Mode                   = COMPRESS_AUTO;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[0]        = 0;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[1]        = 558;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[2]        = 1087;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[3]        = 1588;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[4]        = 2063;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[5]        = 2515;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[6]        = 2944;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[7]        = 3353;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[8]        = 3744;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[9]        = 4473;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[10]       = 5139;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[11]       = 5751;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[12]       = 6316;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[13]       = 6838;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[14]       = 7322;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[15]       = 7772;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[16]       = 8192;
                attr_v12_lite.stManual.Scale_y[0]                             = 0;
                attr_v12_lite.stManual.Scale_y[1]                             = 2;
                attr_v12_lite.stManual.Scale_y[2]                             = 20;
                attr_v12_lite.stManual.Scale_y[3]                             = 76;
                attr_v12_lite.stManual.Scale_y[4]                             = 193;
                attr_v12_lite.stManual.Scale_y[5]                             = 381;
                attr_v12_lite.stManual.Scale_y[6]                             = 631;
                attr_v12_lite.stManual.Scale_y[7]                             = 772;
                attr_v12_lite.stManual.Scale_y[8]                             = 919;
                attr_v12_lite.stManual.Scale_y[9]                             = 1066;
                attr_v12_lite.stManual.Scale_y[10]                            = 1212;
                attr_v12_lite.stManual.Scale_y[11]                            = 1479;
                attr_v12_lite.stManual.Scale_y[12]                            = 1700;
                attr_v12_lite.stManual.Scale_y[13]                            = 1863;
                attr_v12_lite.stManual.Scale_y[14]                            = 1968;
                attr_v12_lite.stManual.Scale_y[15]                            = 2024;
                attr_v12_lite.stManual.Scale_y[16]                            = 2048;
                attr_v12_lite.stManual.Edge_Weit                              = 0.02f;
                attr_v12_lite.stManual.OutPutLongFrame                        = false;
                attr_v12_lite.stManual.IIR_frame                              = 2;
                rk_aiq_user_api2_adrc_v12_lite_SetAttrib(ctx, &attr_v12_lite);
                break;
            }
            case 'a': {
                printf("\t ADRC test rk_aiq_user_api2_adrc_v12_lite_SetAttrib stManual Async\n\n");
                attr_v12_lite.sync.sync_mode                            = RK_AIQ_UAPI_MODE_ASYNC;
                attr_v12_lite.sync.done                                 = false;
                attr_v12_lite.opMode                                    = DRC_OPMODE_MANUAL;
                attr_v12_lite.stManual.Enable                           = true;
                attr_v12_lite.stManual.DrcGain.Alpha                    = 0.1f;
                attr_v12_lite.stManual.DrcGain.DrcGain                  = 2.0f;
                attr_v12_lite.stManual.DrcGain.Clip                     = 1.0f;
                attr_v12_lite.stManual.HiLight.HiLightData.Strength     = 0.5f;
                attr_v12_lite.stManual.HiLight.HiLightData.gas_t        = 0.5f;
                attr_v12_lite.stManual.HiLight.gas_l0                   = 24;
                attr_v12_lite.stManual.HiLight.gas_l1                   = 10;
                attr_v12_lite.stManual.HiLight.gas_l2                   = 10;
                attr_v12_lite.stManual.HiLight.gas_l3                   = 5;
                attr_v12_lite.stManual.LocalSetting.LocalData.LocalWeit = 1.0f;
                attr_v12_lite.stManual.LocalSetting.LocalData.LocalAutoEnable = 1;
                attr_v12_lite.stManual.LocalSetting.LocalData.LocalAutoWeit   = 0.5f;
                attr_v12_lite.stManual.LocalSetting.LocalData.GlobalContrast  = 0.5f;
                attr_v12_lite.stManual.LocalSetting.LocalData.LoLitContrast   = 0.5f;
                attr_v12_lite.stManual.LocalSetting.MotionData.MotionStr      = 0.0f;
                attr_v12_lite.stManual.LocalSetting.curPixWeit                = 0.376471f;
                attr_v12_lite.stManual.LocalSetting.Range_force_sgm           = 0.0f;
                attr_v12_lite.stManual.LocalSetting.Range_sgm_cur             = 0.2f;
                attr_v12_lite.stManual.LocalSetting.Space_sgm_cur             = 4068;
                attr_v12_lite.stManual.CompressSetting.Mode                   = COMPRESS_AUTO;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[0]        = 0;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[1]        = 558;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[2]        = 1087;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[3]        = 1588;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[4]        = 2063;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[5]        = 2515;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[6]        = 2944;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[7]        = 3353;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[8]        = 3744;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[9]        = 4473;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[10]       = 5139;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[11]       = 5751;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[12]       = 6316;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[13]       = 6838;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[14]       = 7322;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[15]       = 7772;
                attr_v12_lite.stManual.CompressSetting.Manual_curve[16]       = 8192;
                attr_v12_lite.stManual.Scale_y[0]                             = 0;
                attr_v12_lite.stManual.Scale_y[1]                             = 2;
                attr_v12_lite.stManual.Scale_y[2]                             = 20;
                attr_v12_lite.stManual.Scale_y[3]                             = 76;
                attr_v12_lite.stManual.Scale_y[4]                             = 193;
                attr_v12_lite.stManual.Scale_y[5]                             = 381;
                attr_v12_lite.stManual.Scale_y[6]                             = 631;
                attr_v12_lite.stManual.Scale_y[7]                             = 772;
                attr_v12_lite.stManual.Scale_y[8]                             = 919;
                attr_v12_lite.stManual.Scale_y[9]                             = 1066;
                attr_v12_lite.stManual.Scale_y[10]                            = 1212;
                attr_v12_lite.stManual.Scale_y[11]                            = 1479;
                attr_v12_lite.stManual.Scale_y[12]                            = 1700;
                attr_v12_lite.stManual.Scale_y[13]                            = 1863;
                attr_v12_lite.stManual.Scale_y[14]                            = 1968;
                attr_v12_lite.stManual.Scale_y[15]                            = 2024;
                attr_v12_lite.stManual.Scale_y[16]                            = 2048;
                attr_v12_lite.stManual.Edge_Weit                              = 0.02f;
                attr_v12_lite.stManual.OutPutLongFrame                        = false;
                attr_v12_lite.stManual.IIR_frame                              = 2;
                rk_aiq_user_api2_adrc_v12_lite_SetAttrib(ctx, &attr_v12_lite);
                break;
            }
            case 'b': {
                printf("\t ADRC test rk_aiq_user_api2_lite_adrc_GetAttrib\n\n");
                rk_aiq_user_api2_adrc_v12_lite_GetAttrib(ctx, &attr_v12_lite);
                printf("\t sync = %d, done = %d\n", attr_v12_lite.sync.sync_mode,
                       attr_v12_lite.sync.done);
                printf("\t opMode:%d EnvLv:%f ISO:%f\n\n", attr_v12_lite.opMode,
                       attr_v12_lite.Info.CtrlInfo.EnvLv, attr_v12_lite.Info.CtrlInfo.ISO);
                printf("\t stManual DrcGain:%f Alpha:%f Clip:%f\n\n",
                       attr_v12_lite.stManual.DrcGain.DrcGain, attr_v12_lite.stManual.DrcGain.Alpha,
                       attr_v12_lite.stManual.DrcGain.Clip);
                break;
            }
            case 'c': {
                printf("\t ADRC test rk_aiq_user_api2_adrc_SetAttrib stManual Sync\n\n");
                attr_v2.sync.sync_mode                               = RK_AIQ_UAPI_MODE_DEFAULT;
                attr_v2.sync.done                                    = false;
                attr_v2.opMode                                       = DRC_OPMODE_MANUAL;
                attr_v2.stManualV30.Enable                           = true;
                attr_v2.stManualV30.DrcGain.Alpha                    = 0.2f;
                attr_v2.stManualV30.DrcGain.DrcGain                  = 2.0f;
                attr_v2.stManualV30.DrcGain.Clip                     = 1.0f;
                attr_v2.stManualV30.HiLight.Strength                 = 0.5f;
                attr_v2.stManualV30.LocalSetting.LocalData.LocalWeit = 1.0f;
                attr_v2.stManualV30.LocalSetting.LocalData.LocalAutoEnable = 1;
                attr_v2.stManualV30.LocalSetting.LocalData.LocalAutoWeit   = 0.5f;
                attr_v2.stManualV30.LocalSetting.LocalData.GlobalContrast  = 0.5f;
                attr_v2.stManualV30.LocalSetting.LocalData.LoLitContrast   = 0.5f;
                attr_v2.stManualV30.LocalSetting.curPixWeit                = 0.376471f;
                attr_v2.stManualV30.LocalSetting.preFrameWeit              = 0.8f;
                attr_v2.stManualV30.LocalSetting.Range_force_sgm           = 0.0f;
                attr_v2.stManualV30.LocalSetting.Range_sgm_cur             = 0.2f;
                attr_v2.stManualV30.LocalSetting.Range_sgm_pre             = 0.2f;
                attr_v2.stManualV30.LocalSetting.Space_sgm_cur             = 4068;
                attr_v2.stManualV30.LocalSetting.Space_sgm_pre             = 3068;
                attr_v2.stManualV30.LocalSetting.Space_sgm_pre             = 3068;
                attr_v2.stManualV30.CompressSetting.Mode                   = COMPRESS_AUTO;
                attr_v2.stManualV30.CompressSetting.Manual_curve[0]        = 0;
                attr_v2.stManualV30.CompressSetting.Manual_curve[1]        = 558;
                attr_v2.stManualV30.CompressSetting.Manual_curve[2]        = 1087;
                attr_v2.stManualV30.CompressSetting.Manual_curve[3]        = 1588;
                attr_v2.stManualV30.CompressSetting.Manual_curve[4]        = 2063;
                attr_v2.stManualV30.CompressSetting.Manual_curve[5]        = 2515;
                attr_v2.stManualV30.CompressSetting.Manual_curve[6]        = 2944;
                attr_v2.stManualV30.CompressSetting.Manual_curve[7]        = 3353;
                attr_v2.stManualV30.CompressSetting.Manual_curve[8]        = 3744;
                attr_v2.stManualV30.CompressSetting.Manual_curve[9]        = 4473;
                attr_v2.stManualV30.CompressSetting.Manual_curve[10]       = 5139;
                attr_v2.stManualV30.CompressSetting.Manual_curve[11]       = 5751;
                attr_v2.stManualV30.CompressSetting.Manual_curve[12]       = 6316;
                attr_v2.stManualV30.CompressSetting.Manual_curve[13]       = 6838;
                attr_v2.stManualV30.CompressSetting.Manual_curve[14]       = 7322;
                attr_v2.stManualV30.CompressSetting.Manual_curve[15]       = 7772;
                attr_v2.stManualV30.CompressSetting.Manual_curve[16]       = 8192;
                attr_v2.stManualV30.Scale_y[0]                             = 0;
                attr_v2.stManualV30.Scale_y[1]                             = 2;
                attr_v2.stManualV30.Scale_y[2]                             = 20;
                attr_v2.stManualV30.Scale_y[3]                             = 76;
                attr_v2.stManualV30.Scale_y[4]                             = 193;
                attr_v2.stManualV30.Scale_y[5]                             = 381;
                attr_v2.stManualV30.Scale_y[6]                             = 631;
                attr_v2.stManualV30.Scale_y[7]                             = 772;
                attr_v2.stManualV30.Scale_y[8]                             = 919;
                attr_v2.stManualV30.Scale_y[9]                             = 1066;
                attr_v2.stManualV30.Scale_y[10]                            = 1212;
                attr_v2.stManualV30.Scale_y[11]                            = 1479;
                attr_v2.stManualV30.Scale_y[12]                            = 1700;
                attr_v2.stManualV30.Scale_y[13]                            = 1863;
                attr_v2.stManualV30.Scale_y[14]                            = 1968;
                attr_v2.stManualV30.Scale_y[15]                            = 2024;
                attr_v2.stManualV30.Scale_y[16]                            = 2048;
                attr_v2.stManualV30.Edge_Weit                              = 0.02f;
                attr_v2.stManualV30.OutPutLongFrame                        = false;
                attr_v2.stManualV30.IIR_frame                              = 2;
                rk_aiq_user_api2_adrc_SetAttrib(ctx, attr_v2);
                break;
            }
            case 'd': {
                printf("\t ADRC test rk_aiq_user_api2_adrc_SetAttrib stManual Async\n\n");
                attr_v2.sync.sync_mode                                     = RK_AIQ_UAPI_MODE_ASYNC;
                attr_v2.sync.done                                          = false;
                attr_v2.opMode                                             = DRC_OPMODE_MANUAL;
                attr_v2.stManualV30.Enable                                 = true;
                attr_v2.stManualV30.DrcGain.Alpha                          = 0.1f;
                attr_v2.stManualV30.DrcGain.DrcGain                        = 2.0f;
                attr_v2.stManualV30.DrcGain.Clip                           = 1.0f;
                attr_v2.stManualV30.HiLight.Strength                       = 0.5f;
                attr_v2.stManualV30.LocalSetting.LocalData.LocalWeit       = 1.0f;
                attr_v2.stManualV30.LocalSetting.LocalData.LocalAutoEnable = 1;
                attr_v2.stManualV30.LocalSetting.LocalData.LocalAutoWeit   = 0.5f;
                attr_v2.stManualV30.LocalSetting.LocalData.GlobalContrast  = 0.5f;
                attr_v2.stManualV30.LocalSetting.LocalData.LoLitContrast   = 0.5f;
                attr_v2.stManualV30.LocalSetting.curPixWeit                = 0.376471f;
                attr_v2.stManualV30.LocalSetting.preFrameWeit              = 0.8f;
                attr_v2.stManualV30.LocalSetting.Range_force_sgm           = 0.0f;
                attr_v2.stManualV30.LocalSetting.Range_sgm_cur             = 0.2f;
                attr_v2.stManualV30.LocalSetting.Range_sgm_pre             = 0.2f;
                attr_v2.stManualV30.LocalSetting.Space_sgm_cur             = 4068;
                attr_v2.stManualV30.LocalSetting.Space_sgm_pre             = 3068;
                attr_v2.stManualV30.LocalSetting.Space_sgm_pre             = 3068;
                attr_v2.stManualV30.CompressSetting.Mode                   = COMPRESS_AUTO;
                attr_v2.stManualV30.CompressSetting.Manual_curve[0]        = 0;
                attr_v2.stManualV30.CompressSetting.Manual_curve[1]        = 558;
                attr_v2.stManualV30.CompressSetting.Manual_curve[2]        = 1087;
                attr_v2.stManualV30.CompressSetting.Manual_curve[3]        = 1588;
                attr_v2.stManualV30.CompressSetting.Manual_curve[4]        = 2063;
                attr_v2.stManualV30.CompressSetting.Manual_curve[5]        = 2515;
                attr_v2.stManualV30.CompressSetting.Manual_curve[6]        = 2944;
                attr_v2.stManualV30.CompressSetting.Manual_curve[7]        = 3353;
                attr_v2.stManualV30.CompressSetting.Manual_curve[8]        = 3744;
                attr_v2.stManualV30.CompressSetting.Manual_curve[9]        = 4473;
                attr_v2.stManualV30.CompressSetting.Manual_curve[10]       = 5139;
                attr_v2.stManualV30.CompressSetting.Manual_curve[11]       = 5751;
                attr_v2.stManualV30.CompressSetting.Manual_curve[12]       = 6316;
                attr_v2.stManualV30.CompressSetting.Manual_curve[13]       = 6838;
                attr_v2.stManualV30.CompressSetting.Manual_curve[14]       = 7322;
                attr_v2.stManualV30.CompressSetting.Manual_curve[15]       = 7772;
                attr_v2.stManualV30.CompressSetting.Manual_curve[16]       = 8192;
                attr_v2.stManualV30.Scale_y[0]                             = 0;
                attr_v2.stManualV30.Scale_y[1]                             = 2;
                attr_v2.stManualV30.Scale_y[2]                             = 20;
                attr_v2.stManualV30.Scale_y[3]                             = 76;
                attr_v2.stManualV30.Scale_y[4]                             = 193;
                attr_v2.stManualV30.Scale_y[5]                             = 381;
                attr_v2.stManualV30.Scale_y[6]                             = 631;
                attr_v2.stManualV30.Scale_y[7]                             = 772;
                attr_v2.stManualV30.Scale_y[8]                             = 919;
                attr_v2.stManualV30.Scale_y[9]                             = 1066;
                attr_v2.stManualV30.Scale_y[10]                            = 1212;
                attr_v2.stManualV30.Scale_y[11]                            = 1479;
                attr_v2.stManualV30.Scale_y[12]                            = 1700;
                attr_v2.stManualV30.Scale_y[13]                            = 1863;
                attr_v2.stManualV30.Scale_y[14]                            = 1968;
                attr_v2.stManualV30.Scale_y[15]                            = 2024;
                attr_v2.stManualV30.Scale_y[16]                            = 2048;
                attr_v2.stManualV30.Edge_Weit                              = 0.02f;
                attr_v2.stManualV30.OutPutLongFrame                        = false;
                attr_v2.stManualV30.IIR_frame                              = 2;
                rk_aiq_user_api2_adrc_SetAttrib(ctx, attr_v2);
                break;
            }
            case 'e': {
                printf("\t ADRC test rk_aiq_user_api2_adrc_GetAttrib\n\n");
                rk_aiq_user_api2_adrc_GetAttrib(ctx, &attr_v2);
                printf("\t sync = %d, done = %d Version:%d\n", attr_v2.sync.sync_mode,
                       attr_v2.sync.done, attr_v2.Version);
                printf("\t opMode:%d EnvLv:%f ISO:%f\n\n", attr_v2.opMode,
                       attr_v2.Info.CtrlInfo.EnvLv, attr_v2.Info.CtrlInfo.ISO);
                printf("\t stManual DrcGain:%f Alpha:%f Clip:%f\n\n",
                       attr_v2.stManualV30.DrcGain.DrcGain, attr_v2.stManualV30.DrcGain.Alpha,
                       attr_v2.stManualV30.DrcGain.Clip);
                break;
            }
            case 'f': {
                printf("\t ADRC test rk_aiq_uapi2_getDrcGain\n\n");
                float Gain  = 0.0f;
                float Alpha = 0.0f;
                float Clip  = 0.0f;
                rk_aiq_uapi2_getDrcGain(ctx, &Gain, &Alpha, &Clip);
                printf("\t ADRC rk_aiq_uapi2_getDrcGain: Gain:%f Alpha:%f Clip:%f\n\n", Gain, Alpha,
                       Clip);
                break;
            }
            case 'g': {
                printf("\t ADRC test rk_aiq_uapi2_setDrcGain\n\n");
                rk_aiq_uapi2_setDrcGain(ctx, 3.5f, 0.4f, 3.0f);
                break;
            }
            case 'h': {
                printf("\t ADRC test rk_aiq_uapi2_getDrcHiLit\n\n");
                float Strength = 0.0f;
                rk_aiq_uapi2_getDrcHiLit(ctx, &Strength);
                printf("\t ADRC rk_aiq_uapi2_getDrcHiLit: Strength:%f\n\n", Strength);
                break;
            }
            case 'i': {
                printf("\t ADRC test rk_aiq_uapi2_setDrcHiLit\n\n");
                rk_aiq_uapi2_setDrcHiLit(ctx, 0.8f);
                break;
            }
            case 'j': {
                printf("\t ADRC test rk_aiq_uapi2_getDrcLocalData\n\n");
                float LocalWeit      = 0.0f;
                float GlobalContrast = 0.0f;
                float LoLitContrast  = 0.0f;
                int LocalAutoEnable  = 0;
                float LocalAutoWeit  = 0.0f;
                rk_aiq_uapi2_getDrcLocalData(ctx, &LocalWeit, &GlobalContrast, &LoLitContrast,
                                             &LocalAutoEnable, &LocalAutoWeit);
                printf(
                    "\t ADRC rk_aiq_uapi2_getDrcLocalData: LocalWeit:%f GlobalContrast:%f "
                    "LoLitContrast:%f LocalAutoEnable:%d LocalAutoWeit:%f\n\n",
                    LocalWeit, GlobalContrast, LoLitContrast, LocalAutoEnable, LocalAutoWeit);
                break;
            }
            case 'k': {
                printf("\t ADRC test rk_aiq_uapi2_setDrcLocalData\n\n");
                rk_aiq_uapi2_setDrcLocalData(ctx, 1.0f, 0.5f, 0.5f, 1, 0.9f);
                break;
            }
#ifdef USE_NEWSTRUCT
            case 'l': {
                printf("\t sample_new_drc\n\n");
                sample_new_drc(ctx);
                break;
            }
#endif
            default:
                break;
        }
    } while (key != 'q' && key != 'Q');

    return XCAM_RETURN_NO_ERROR;
}
