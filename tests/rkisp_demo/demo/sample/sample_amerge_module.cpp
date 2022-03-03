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

static void sample_amerge_usage()
{
    printf("Usage : \n");
    printf("\t 0) AMERGE:         test rk_aiq_user_api2_amerge_SetAttrib stManual BASEFRAME_SHORT Sync.\n");
    printf("\t 1) AMERGE:         test rk_aiq_user_api2_amerge_SetAttrib stManual BASEFRAME_SHORT Async.\n");
    printf("\t 2) AMERGE:         test rk_aiq_user_api2_amerge_SetAttrib stManual BASEFRAME_LONG Sync.\n");
    printf("\t 3) AMERGE:         test rk_aiq_user_api2_amerge_SetAttrib stManual BASEFRAME_LONG Async.\n");
    printf("\t 4) AMERGE:         test rk_aiq_user_api2_amerge_GetAttrib.\n");
    printf("\t q) AMERGE:         return to main sample screen.\n");

    printf("\n");
    printf("\t please press the key: ");

    return;
}

void sample_print_amerge_info(const void *arg)
{
    printf ("enter AMERGE modult test!\n");
}

XCamReturn sample_amerge_module(const void *arg)
{
    int key = -1;
    CLEAR();

    mergeAttr_t attr;
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


        switch (key)
        {
        case '0': {
            printf("\t AMERGE test rk_aiq_user_api2_amerge_SetAttrib stManual BASEFRAME_SHORT Sync\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            attr.sync.done = false;
            attr.attrV30.opMode = MERGE_OPMODE_MANU;
            attr.attrV30.stManual.BaseFrm = BASEFRAME_SHORT;
            attr.attrV30.stManual.ShortFrmModeData.OECurve.Smooth = 0.4;
            attr.attrV30.stManual.ShortFrmModeData.OECurve.Offset = 210;
            attr.attrV30.stManual.ShortFrmModeData.MDCurve.Coef = 0.05;
            attr.attrV30.stManual.ShortFrmModeData.MDCurve.ms_thd0 = 0.00;
            attr.attrV30.stManual.ShortFrmModeData.MDCurve.lm_thd0 = 0.00;
            rk_aiq_user_api2_amerge_SetAttrib(ctx, attr);
            break;
        }
        case '1': {
            printf("\t AMERGE test rk_aiq_user_api2_amerge_SetAttrib stManual BASEFRAME_SHORT Async\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
            attr.sync.done = false;
            attr.attrV30.opMode = MERGE_OPMODE_MANU;
            attr.attrV30.stManual.BaseFrm = BASEFRAME_SHORT;
            attr.attrV30.stManual.ShortFrmModeData.OECurve.Smooth = 0.5;
            attr.attrV30.stManual.ShortFrmModeData.OECurve.Offset = 230;
            attr.attrV30.stManual.ShortFrmModeData.MDCurve.Coef = 0.07;
            attr.attrV30.stManual.ShortFrmModeData.MDCurve.ms_thd0 = 0.00;
            attr.attrV30.stManual.ShortFrmModeData.MDCurve.lm_thd0 = 0.00;
            rk_aiq_user_api2_amerge_SetAttrib(ctx, attr);
            break;
        }
        case '2': {
            printf("\t AMERGE test rk_aiq_user_api2_amerge_SetAttrib stManual BASEFRAME_LONG Sync\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            attr.sync.done = false;
            attr.attrV30.opMode = MERGE_OPMODE_MANU;
            attr.attrV30.stManual.BaseFrm = BASEFRAME_LONG;
            attr.attrV30.stManual.LongFrmModeData.OECurve.Smooth = 0.6;
            attr.attrV30.stManual.LongFrmModeData.OECurve.Offset = 240;
            attr.attrV30.stManual.LongFrmModeData.MDCurve.LM_smooth = 0.8;
            attr.attrV30.stManual.LongFrmModeData.MDCurve.LM_offset = 38;
            attr.attrV30.stManual.LongFrmModeData.MDCurve.MS_smooth = 0.8;
            attr.attrV30.stManual.LongFrmModeData.MDCurve.MS_offset = 38;
            rk_aiq_user_api2_amerge_SetAttrib(ctx, attr);
            break;
        }
        case '3': {
            printf("\t AMERGE test rk_aiq_user_api2_amerge_SetAttrib stManual BASEFRAME_LONG Async\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
            attr.sync.done = false;
            attr.attrV30.opMode = MERGE_OPMODE_MANU;
            attr.attrV30.stManual.BaseFrm = BASEFRAME_LONG;
            attr.attrV30.stManual.LongFrmModeData.OECurve.Smooth = 0.7;
            attr.attrV30.stManual.LongFrmModeData.OECurve.Offset = 280;
            attr.attrV30.stManual.LongFrmModeData.MDCurve.LM_smooth = 0.8;
            attr.attrV30.stManual.LongFrmModeData.MDCurve.LM_offset = 38;
            attr.attrV30.stManual.LongFrmModeData.MDCurve.MS_smooth = 0.8;
            attr.attrV30.stManual.LongFrmModeData.MDCurve.MS_offset = 38;
            rk_aiq_user_api2_amerge_SetAttrib(ctx, attr);
            break;
        }
        case '4': {
            printf("\t AMERGE test rk_aiq_user_api2_amerge_GetAttrib\n\n");
            rk_aiq_user_api2_amerge_GetAttrib(ctx, &attr);
            printf("\t sync = %d, done = %d\n", attr.sync.sync_mode, attr.sync.done);
            printf("\t attr.mode:%d Envlv:%f MoveCoef:%f\n\n", attr.attrV30.opMode, attr.attrV30.CtlInfo.Envlv, attr.attrV30.CtlInfo.MoveCoef);
            printf("\t BaseFrm:%d \n\n", attr.attrV30.stManual.BaseFrm);
            printf("\t LongFrmModeData OECurve Smooth:%f Offset:%f\n\n", attr.attrV30.stManual.LongFrmModeData.OECurve.Smooth,
                   attr.attrV30.stManual.LongFrmModeData.OECurve.Offset);
            printf("\t LongFrmModeData MDCurve LM_smooth:%f LM_offset:%f MS_smooth%f MS_offset:%f\n\n", attr.attrV30.stManual.LongFrmModeData.MDCurve.LM_smooth,
                   attr.attrV30.stManual.LongFrmModeData.MDCurve.LM_offset, attr.attrV30.stManual.LongFrmModeData.MDCurve.MS_smooth,
                   attr.attrV30.stManual.LongFrmModeData.MDCurve.MS_offset);
            printf("\t ShortFrmModeData OECurve Smooth:%f Offset:%f\n\n", attr.attrV30.stManual.ShortFrmModeData.OECurve.Smooth,
                   attr.attrV30.stManual.ShortFrmModeData.OECurve.Offset);
            printf("\t ShortFrmModeData MDCurve Coef:%f ms_thd0:%f lm_thd0%f\n\n", attr.attrV30.stManual.ShortFrmModeData.MDCurve.Coef,
                   attr.attrV30.stManual.ShortFrmModeData.MDCurve.ms_thd0, attr.attrV30.stManual.ShortFrmModeData.MDCurve.lm_thd0);
            break;
        }
        default:
            break;
        }
    } while (key != 'q' && key != 'Q');

    return XCAM_RETURN_NO_ERROR;
}
