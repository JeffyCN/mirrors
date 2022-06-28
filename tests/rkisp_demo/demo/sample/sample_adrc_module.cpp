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

static void sample_adrc_usage()
{
    printf("Usage : \n");
    printf("\t 0) ADRC:         test rk_aiq_user_api2_adrc_SetAttrib stManualV30 Sync.\n");
    printf("\t 1) ADRC:         test rk_aiq_user_api2_adrc_SetAttrib stManualV30 Async.\n");
    printf("\t 2) ADRC:         test rk_aiq_user_api2_adrc_SetAttrib stDrcGain Sync.\n");
    printf("\t 3) ADRC:         test rk_aiq_user_api2_adrc_SetAttrib stDrcGain Async.\n");
    printf("\t 4) ADRC:         test rk_aiq_user_api2_adrc_SetAttrib stHiLit Sync.\n");
    printf("\t 5) ADRC:         test rk_aiq_user_api2_adrc_SetAttrib stHiLit Async.\n");
    printf("\t 6) ADRC:         test rk_aiq_user_api2_adrc_SetAttrib stLocalDataV30 Sync.\n");
    printf("\t 7) ADRC:         test rk_aiq_user_api2_adrc_SetAttrib stLocalDataV30 Async.\n");
    printf("\t 8) ADRC:         test rk_aiq_user_api2_adrc_GetAttrib.\n");
    printf("\t 9) ADRC:         test rk_aiq_uapi2_getDrcGain.\n");
    printf("\t a) ADRC:         test rk_aiq_uapi2_setDrcGain.\n");
    printf("\t b) ADRC:         test rk_aiq_uapi2_getDrcHiLit.\n");
    printf("\t c) ADRC:         test rk_aiq_uapi2_setDrcHiLit.\n");
    printf("\t d) ADRC:         test rk_aiq_uapi2_getDrcLocalData.\n");
    printf("\t e) ADRC:         test rk_aiq_uapi2_setDrcLocalData.\n");
    printf("\t q) ADRC:         return to main sample screen.\n");

    printf("\n");
    printf("\t please press the key: ");

    return;
}

void sample_print_adrc_info(const void *arg)
{
    printf ("enter ADRC modult test!\n");
}

XCamReturn sample_adrc_module(const void *arg)
{
    int key = -1;
    CLEAR();

    drc_attrib_t attr;
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


        switch (key)
        {
        case '0': {
            printf("\t ADRC test rk_aiq_user_api2_adrc_SetAttrib stManualV30 Sync\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            attr.sync.done = false;
            attr.opMode = DRC_OPMODE_MANU;
            attr.stManualV30.Enable = true;
            attr.stManualV30.DrcGain.Alpha = 0.2;
            attr.stManualV30.DrcGain.DrcGain = 2;
            attr.stManualV30.DrcGain.Clip = 1;
            attr.stManualV30.HiLight.Strength = 0.5;
            attr.stManualV30.LocalSetting.LocalData.LocalWeit = 1;
            attr.stManualV30.LocalSetting.LocalData.LocalAutoEnable = 1;
            attr.stManualV30.LocalSetting.LocalData.LocalAutoWeit = 0.5;
            attr.stManualV30.LocalSetting.LocalData.GlobalContrast = 0.5;
            attr.stManualV30.LocalSetting.LocalData.LoLitContrast = 0.5;
            attr.stManualV30.LocalSetting.curPixWeit = 0.376471;
            attr.stManualV30.LocalSetting.preFrameWeit = 0.8;
            attr.stManualV30.LocalSetting.Range_force_sgm = 0.0;
            attr.stManualV30.LocalSetting.Range_sgm_cur = 0.2;
            attr.stManualV30.LocalSetting.Range_sgm_pre = 0.2;
            attr.stManualV30.LocalSetting.Space_sgm_cur = 4068;
            attr.stManualV30.LocalSetting.Space_sgm_pre = 3068;
            attr.stManualV30.LocalSetting.Space_sgm_pre = 3068;
            attr.stManualV30.CompressSetting.Mode = COMPRESS_AUTO;
            attr.stManualV30.CompressSetting.Manual_curve[0] = 0;
            attr.stManualV30.CompressSetting.Manual_curve[1] = 558;
            attr.stManualV30.CompressSetting.Manual_curve[2] = 1087;
            attr.stManualV30.CompressSetting.Manual_curve[3] = 1588;
            attr.stManualV30.CompressSetting.Manual_curve[4] = 2063;
            attr.stManualV30.CompressSetting.Manual_curve[5] = 2515;
            attr.stManualV30.CompressSetting.Manual_curve[6] = 2944;
            attr.stManualV30.CompressSetting.Manual_curve[7] = 3353;
            attr.stManualV30.CompressSetting.Manual_curve[8] = 3744;
            attr.stManualV30.CompressSetting.Manual_curve[9] = 4473;
            attr.stManualV30.CompressSetting.Manual_curve[10] = 5139;
            attr.stManualV30.CompressSetting.Manual_curve[11] = 5751;
            attr.stManualV30.CompressSetting.Manual_curve[12] = 6316;
            attr.stManualV30.CompressSetting.Manual_curve[13] = 6838;
            attr.stManualV30.CompressSetting.Manual_curve[14] = 7322;
            attr.stManualV30.CompressSetting.Manual_curve[15] = 7772;
            attr.stManualV30.CompressSetting.Manual_curve[16] = 8192;
            attr.stManualV30.Scale_y[0] = 0;
            attr.stManualV30.Scale_y[1] = 2;
            attr.stManualV30.Scale_y[2] = 20;
            attr.stManualV30.Scale_y[3] = 76;
            attr.stManualV30.Scale_y[4] = 193;
            attr.stManualV30.Scale_y[5] = 381;
            attr.stManualV30.Scale_y[6] = 631;
            attr.stManualV30.Scale_y[7] = 772;
            attr.stManualV30.Scale_y[8] = 919;
            attr.stManualV30.Scale_y[9] = 1066;
            attr.stManualV30.Scale_y[10] = 1211;
            attr.stManualV30.Scale_y[11] = 1479;
            attr.stManualV30.Scale_y[12] = 1700;
            attr.stManualV30.Scale_y[13] = 1863;
            attr.stManualV30.Scale_y[14] = 1968;
            attr.stManualV30.Scale_y[15] = 2024;
            attr.stManualV30.Scale_y[16] = 2048;
            attr.stManualV30.Edge_Weit = 0.02;
            attr.stManualV30.OutPutLongFrame = false;
            attr.stManualV30.IIR_frame = 2;
            rk_aiq_user_api2_adrc_SetAttrib(ctx, attr);
            break;
        }
        case '1': {
            printf("\t ADRC test rk_aiq_user_api2_adrc_SetAttrib stManualV30 Async\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
            attr.sync.done = false;
            attr.opMode = DRC_OPMODE_MANU;
            attr.stManualV30.Enable = true;
            attr.stManualV30.DrcGain.Alpha = 0.1;
            attr.stManualV30.DrcGain.DrcGain = 2;
            attr.stManualV30.DrcGain.Clip = 1;
            attr.stManualV30.HiLight.Strength = 0.5;
            attr.stManualV30.LocalSetting.LocalData.LocalWeit = 1;
            attr.stManualV30.LocalSetting.LocalData.LocalAutoEnable = 1;
            attr.stManualV30.LocalSetting.LocalData.LocalAutoWeit = 0.5;
            attr.stManualV30.LocalSetting.LocalData.GlobalContrast = 0.5;
            attr.stManualV30.LocalSetting.LocalData.LoLitContrast = 0.5;
            attr.stManualV30.LocalSetting.curPixWeit = 0.376471;
            attr.stManualV30.LocalSetting.preFrameWeit = 0.8;
            attr.stManualV30.LocalSetting.Range_force_sgm = 0.0;
            attr.stManualV30.LocalSetting.Range_sgm_cur = 0.2;
            attr.stManualV30.LocalSetting.Range_sgm_pre = 0.2;
            attr.stManualV30.LocalSetting.Space_sgm_cur = 4068;
            attr.stManualV30.LocalSetting.Space_sgm_pre = 3068;
            attr.stManualV30.LocalSetting.Space_sgm_pre = 3068;
            attr.stManualV30.CompressSetting.Mode = COMPRESS_AUTO;
            attr.stManualV30.CompressSetting.Manual_curve[0] = 0;
            attr.stManualV30.CompressSetting.Manual_curve[1] = 558;
            attr.stManualV30.CompressSetting.Manual_curve[2] = 1087;
            attr.stManualV30.CompressSetting.Manual_curve[3] = 1588;
            attr.stManualV30.CompressSetting.Manual_curve[4] = 2063;
            attr.stManualV30.CompressSetting.Manual_curve[5] = 2515;
            attr.stManualV30.CompressSetting.Manual_curve[6] = 2944;
            attr.stManualV30.CompressSetting.Manual_curve[7] = 3353;
            attr.stManualV30.CompressSetting.Manual_curve[8] = 3744;
            attr.stManualV30.CompressSetting.Manual_curve[9] = 4473;
            attr.stManualV30.CompressSetting.Manual_curve[10] = 5139;
            attr.stManualV30.CompressSetting.Manual_curve[11] = 5751;
            attr.stManualV30.CompressSetting.Manual_curve[12] = 6316;
            attr.stManualV30.CompressSetting.Manual_curve[13] = 6838;
            attr.stManualV30.CompressSetting.Manual_curve[14] = 7322;
            attr.stManualV30.CompressSetting.Manual_curve[15] = 7772;
            attr.stManualV30.CompressSetting.Manual_curve[16] = 8192;
            attr.stManualV30.Scale_y[0] = 0;
            attr.stManualV30.Scale_y[1] = 2;
            attr.stManualV30.Scale_y[2] = 20;
            attr.stManualV30.Scale_y[3] = 76;
            attr.stManualV30.Scale_y[4] = 193;
            attr.stManualV30.Scale_y[5] = 381;
            attr.stManualV30.Scale_y[6] = 631;
            attr.stManualV30.Scale_y[7] = 772;
            attr.stManualV30.Scale_y[8] = 919;
            attr.stManualV30.Scale_y[9] = 1066;
            attr.stManualV30.Scale_y[10] = 1211;
            attr.stManualV30.Scale_y[11] = 1479;
            attr.stManualV30.Scale_y[12] = 1700;
            attr.stManualV30.Scale_y[13] = 1863;
            attr.stManualV30.Scale_y[14] = 1968;
            attr.stManualV30.Scale_y[15] = 2024;
            attr.stManualV30.Scale_y[16] = 2048;
            attr.stManualV30.Edge_Weit = 0.02;
            attr.stManualV30.OutPutLongFrame = false;
            attr.stManualV30.IIR_frame = 2;
            rk_aiq_user_api2_adrc_SetAttrib(ctx, attr);
            break;
        }
        case '2': {
            printf("\t ADRC test rk_aiq_user_api2_adrc_SetAttrib stDrcGain Sync\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            attr.sync.done = false;
            attr.opMode = DRC_OPMODE_DRC_GAIN;
            attr.stDrcGain.DrcGain = 3;
            attr.stDrcGain.Clip = 3.2;
            attr.stDrcGain.Alpha = 0.2;
            rk_aiq_user_api2_adrc_SetAttrib(ctx, attr);
            break;
        }
        case '3': {
            printf("\t ADRC test rk_aiq_user_api2_adrc_SetAttrib stDrcGain Async\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
            attr.sync.done = false;
            attr.opMode = DRC_OPMODE_DRC_GAIN;
            attr.stDrcGain.DrcGain = 3;
            attr.stDrcGain.Clip = 3;
            attr.stDrcGain.Alpha = 0.2;
            rk_aiq_user_api2_adrc_SetAttrib(ctx, attr);
            break;
        }
        case '4': {
            printf("\t ADRC test rk_aiq_user_api2_adrc_SetAttrib stHiLit Sync\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            attr.sync.done = false;
            attr.opMode = DRC_OPMODE_HILIT;
            attr.stHiLit.Strength = 0.51;
            rk_aiq_user_api2_adrc_SetAttrib(ctx, attr);
            break;
        }
        case '5': {
            printf("\t ADRC test rk_aiq_user_api2_adrc_SetAttrib stHiLit Async\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
            attr.sync.done = false;
            attr.opMode = DRC_OPMODE_HILIT;
            attr.stHiLit.Strength = 0.5;
            rk_aiq_user_api2_adrc_SetAttrib(ctx, attr);
            break;
        }
        case '6': {
            printf("\t ADRC test rk_aiq_user_api2_adrc_SetAttrib stLocalDataV30 Sync\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            attr.sync.done = false;
            attr.opMode = DRC_OPMODE_LOCAL_TMO;
            attr.stLocalDataV30.LocalWeit = 1;
            attr.stLocalDataV30.LocalAutoEnable = 1;
            attr.stLocalDataV30.LocalAutoWeit = 0.6;
            attr.stLocalDataV30.GlobalContrast = 0.5;
            attr.stLocalDataV30.LoLitContrast = 0.5;
            rk_aiq_user_api2_adrc_SetAttrib(ctx, attr);
            break;
        }
        case '7': {
            printf("\t ADRC test rk_aiq_user_api2_adrc_SetAttrib stLocalDataV30 Async\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
            attr.sync.done = false;
            attr.opMode = DRC_OPMODE_LOCAL_TMO;
            attr.stLocalDataV30.LocalWeit = 1;
            attr.stLocalDataV30.LocalAutoEnable = 1;
            attr.stLocalDataV30.LocalAutoWeit = 0.5;
            attr.stLocalDataV30.GlobalContrast = 0.5;
            attr.stLocalDataV30.LoLitContrast = 0.5;
            rk_aiq_user_api2_adrc_SetAttrib(ctx, attr);
            break;
        }
        case '8': {
            printf("\t ADRC test rk_aiq_user_api2_adrc_GetAttrib\n\n");
            rk_aiq_user_api2_adrc_GetAttrib(ctx, &attr);
            printf("\t sync = %d, done = %d\n", attr.sync.sync_mode, attr.sync.done);
            printf("\t opMode:%d EnvLv:%f\n\n", attr.opMode, attr.Info.EnvLv);
            printf("\t stManualV30 DrcGain:%f Alpha:%f Clip:%f\n\n", attr.stManualV30.DrcGain.DrcGain, attr.stManualV30.DrcGain.Alpha, attr.stManualV30.DrcGain.Clip);
            printf("\t stDrcGain DrcGain:%f Alpha:%f Clip:%f\n\n", attr.stDrcGain.DrcGain, attr.stDrcGain.Alpha, attr.stDrcGain.Clip);
            printf("\t stHiLit Strength:%f\n\n", attr.stHiLit.Strength);
            printf("\t stLocalDataV30 LocalWeit:%f LocalAutoEnable:%d LocalAutoWeit:%f GlobalContrast:%f LoLitContrast:%f\n\n", attr.stLocalDataV30.LocalWeit,
                   attr.stLocalDataV30.LocalAutoEnable, attr.stLocalDataV30.LocalAutoWeit, attr.stLocalDataV30.GlobalContrast, attr.stLocalDataV30.LoLitContrast);
            break;
        }
        case '9': {
            printf("\t ADRC test rk_aiq_uapi2_getDrcGain\n\n");
            float Gain = 0;
            float Alpha = 0;
            float Clip = 0;
            rk_aiq_uapi2_getDrcGain(ctx, &Gain, &Alpha, &Clip);
            printf("\t ADRC rk_aiq_uapi2_getDrcGain: Gain:%f Alpha:%f Clip:%f\n\n", Gain, Alpha, Clip);
            break;
        }
        case 'a': {
            printf("\t ADRC test rk_aiq_uapi2_setDrcGain\n\n");
            rk_aiq_uapi2_setDrcGain(ctx, 3.5, 0.4, 3);
            break;
        }
        case 'b': {
            printf("\t ADRC test rk_aiq_uapi2_getDrcHiLit\n\n");
            float Strength = 0;
            rk_aiq_uapi2_getDrcHiLit(ctx, &Strength);
            printf("\t ADRC rk_aiq_uapi2_getDrcHiLit: Strength:%f\n\n", Strength);
            break;
        }
        case 'c': {
            printf("\t ADRC test rk_aiq_uapi2_setDrcHiLit\n\n");
            rk_aiq_uapi2_setDrcHiLit(ctx, 0.8);
            break;
        }
        case 'd': {
            printf("\t ADRC test rk_aiq_uapi2_getDrcLocalData\n\n");
            float LocalWeit = 0;
            float GlobalContrast = 0;
            float LoLitContrast = 0;
            int LocalAutoEnable = 0;
            float LocalAutoWeit = 0;
            rk_aiq_uapi2_getDrcLocalData(ctx, &LocalWeit, &GlobalContrast, &LoLitContrast, &LocalAutoEnable, &LocalAutoWeit);
            printf("\t ADRC rk_aiq_uapi2_getDrcGain: LocalWeit:%f GlobalContrast:%f LoLitContrast:%f LocalAutoEnable:%d LocalAutoWeit:%f\n\n",
                   LocalWeit, GlobalContrast, LoLitContrast, LocalAutoEnable, LocalAutoWeit);
            break;
        }
        case 'e': {
            printf("\t ADRC test rk_aiq_uapi2_setDrcLocalData\n\n");
            rk_aiq_uapi2_setDrcLocalData(ctx, 1.0, 0.5, 0.5, 1, 0.9);
            break;
        }
        default:
            break;
        }
    } while (key != 'q' && key != 'Q');

    return XCAM_RETURN_NO_ERROR;
}
