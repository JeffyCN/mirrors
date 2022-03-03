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

#define LIMIT_VALUE(value,max_value,min_value)      (value > max_value? max_value : value < min_value ? min_value : value)


static void sample_agamma_usage()
{
    printf("Usage : \n");
    printf("\t 0) AGAMMA:         test rk_aiq_user_api2_agamma_SetAttrib stManual Sync.\n");
    printf("\t 1) AGAMMA:         test rk_aiq_user_api2_agamma_SetAttrib stManual Async.\n");
    printf("\t 2) AGAMMA:         test rk_aiq_user_api2_agamma_SetAttrib stFast Sync.\n");
    printf("\t 3) AGAMMA:         test rk_aiq_user_api2_agamma_SetAttrib stFast Async.\n");
    printf("\t 4) AGAMMA:         test rk_aiq_user_api2_agamma_GetAttrib.\n");
    printf("\t 5) AGAMMA:         test rk_aiq_uapi2_setGammaCoef.\n");
    printf("\t q) AGAMMA:         return to main sample screen.\n");

    printf("\n");
    printf("\t please press the key: ");

    return;
}

void sample_print_agamma_info(const void *arg)
{
    printf ("enter AGAMMA modult test!\n");
}

XCamReturn sample_agamma_module(const void *arg)
{
    int key = -1;
    CLEAR();

    rk_aiq_gamma_attr_t attr;
    const demo_context_t *demo_ctx = (demo_context_t *)arg;
    const rk_aiq_sys_ctx_t* ctx ;
    if (demo_ctx->camGroup) {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->camgroup_ctx);
    } else {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->aiq_ctx);
    }

    do {
        sample_agamma_usage ();

        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar();
        printf ("\n");


        switch (key)
        {
        case '0': {
            printf("\t AGAMMA test rk_aiq_user_api2_agamma_SetAttrib stManual Sync\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            attr.sync.done = false;
            attr.atrrV30.mode = RK_AIQ_GAMMA_MODE_MANUAL;
            attr.atrrV30.stManual.Gamma_en = true;
            attr.atrrV30.stManual.Gamma_out_offset = 0;
            float X_isp30[CALIBDB_AGAMMA_KNOTS_NUM_V30] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 28, 32, 40, 48, 56,
                                                            64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 512, 640, 768, 896, 1024,
                                                            1280, 1536, 1792, 2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840, 4095
                                                          };
            float Y_isp30[CALIBDB_AGAMMA_KNOTS_NUM_V30];
            for(int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V30; i++) {
                Y_isp30[i] = 4095 * pow(X_isp30[i] / 4095, 1 / 1 + 0.0);
                Y_isp30[i] = LIMIT_VALUE(Y_isp30[i], 4095, 0);
                attr.atrrV30.stManual.Gamma_curve[i] = (int)(Y_isp30[i] + 0.5);
            }
            rk_aiq_user_api2_agamma_SetAttrib(ctx, attr);
            break;
        }
        case '1': {
            printf("\t AGAMMA test rk_aiq_user_api2_agamma_SetAttrib stManual Async\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
            attr.sync.done = false;
            attr.atrrV30.mode = RK_AIQ_GAMMA_MODE_MANUAL;
            attr.atrrV30.stManual.Gamma_en = true;
            attr.atrrV30.stManual.Gamma_out_offset = 1024;
            float X_isp30[CALIBDB_AGAMMA_KNOTS_NUM_V30] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 28, 32, 40, 48, 56,
                                                            64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 512, 640, 768, 896, 1024,
                                                            1280, 1536, 1792, 2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840, 4095
                                                          };
            float Y_isp30[CALIBDB_AGAMMA_KNOTS_NUM_V30];
            for(int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V30; i++) {
                Y_isp30[i] = 4095 * pow(X_isp30[i] / 4095, 1 / 2.2 + 0.0);
                Y_isp30[i] = LIMIT_VALUE(Y_isp30[i], 4095, 0);
                attr.atrrV30.stManual.Gamma_curve[i] = (int)(Y_isp30[i] + 0.5);
            }
            rk_aiq_user_api2_agamma_SetAttrib(ctx, attr);
            break;
        }
        case '2': {
            printf("\t AGAMMA test rk_aiq_user_api2_agamma_SetAttrib stFast Sync\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            attr.sync.done = false;
            attr.atrrV30.mode = RK_AIQ_GAMMA_MODE_FAST;
            attr.atrrV30.stFast.en = true;
            attr.atrrV30.stFast.GammaCoef = 3;
            attr.atrrV30.stFast.SlopeAtZero = 0.0;
            rk_aiq_user_api2_agamma_SetAttrib(ctx, attr);
            break;
        }
        case '3': {
            printf("\t AGAMMA test rk_aiq_user_api2_agamma_SetAttrib stFast Async\n\n");
            attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
            attr.sync.done = false;
            attr.atrrV30.mode = RK_AIQ_GAMMA_MODE_FAST;
            attr.atrrV30.stFast.en = true;
            attr.atrrV30.stFast.GammaCoef = 4;
            attr.atrrV30.stFast.SlopeAtZero = 0.0;
            rk_aiq_user_api2_agamma_SetAttrib(ctx, attr);
            break;
        }
        case '4': {
            printf("\t AGAMMA test rk_aiq_user_api2_agamma_GetAttrib\n\n");
            rk_aiq_user_api2_agamma_GetAttrib(ctx, &attr);
            printf("\t sync = %d, done = %d\n", attr.sync.sync_mode, attr.sync.done);
            printf("\t attr.mode:%d stManual.Gamma_en:%d stManual.Gamma_out_offset:%d\n\n", attr.atrrV30.mode, attr.atrrV30.stManual.Gamma_en, attr.atrrV30.stManual.Gamma_out_offset);
            printf("\t stManual.Gamma_curve:%d %d %d %d %d %d\n\n", attr.atrrV30.stManual.Gamma_curve[0], attr.atrrV30.stManual.Gamma_curve[1],
                   attr.atrrV30.stManual.Gamma_curve[2], attr.atrrV30.stManual.Gamma_curve[3], attr.atrrV30.stManual.Gamma_curve[4], attr.atrrV30.stManual.Gamma_curve[5]);
            printf("\t stFast.en:%d stFast.GammaCoef:%f stFast.SlopeAtZero:%f\n\n", attr.atrrV30.stFast.en, attr.atrrV30.stFast.GammaCoef, attr.atrrV30.stFast.SlopeAtZero);
            break;
        }
        case '5': {
            printf("\t AGAMMA test rk_aiq_uapi2_setGammaCoef\n\n");
            rk_aiq_uapi2_setGammaCoef(ctx, 5, 0);
            break;
        }
        default:
            break;
        }
    } while (key != 'q' && key != 'Q');

    return XCAM_RETURN_NO_ERROR;
}
