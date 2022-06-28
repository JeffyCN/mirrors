/*
 *  Copyright (c) 2021 Rockchip Corporation
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

static void sample_adebayer_usage()
{
    printf("Usage : \n");
    printf("  Module API: \n");
    printf("\t 0) ADEBAYER:         enable/disable with AUTO mode in sync.\n");
    printf("\t 1) ADEBAYER:         set sharp strength of 250 with AUTO modein sync.\n");
    printf("\t 2) ADEBAYER:         set sharp strength of 0 with AUTO modein sync.\n");
    printf("\t 3) ADEBAYER:         set high freq thresh of 250 with AUTO modein sync.\n");
    printf("\t 4) ADEBAYER:         set high freq thresh of 0 with AUTO mode in sync.\n");
    printf("\t 5) ADEBAYER:         set low freq thresh of 250 with AUTO mode in sync.\n");
    printf("\t 6) ADEBAYER:         set low freq thresh of 0 with AUTO mode in sync.\n");
    printf("\n");

    printf("\t 7) ADEBAYER:         enable/disable with MANUAL mode in sync.\n");
    printf("\t 8) ADEBAYER:         set manual params from json with MANUAL mode in sync, iso is 50.\n");
    printf("\t 9) ADEBAYER:         set manual params from json with MANUAL mode in sync, iso is 6400.\n");
    printf("\n");

    printf("\t h) ADEBAYER: help.\n");
    printf("\t q/Q) ADEBAYER:       return to main sample screen.\n");
    printf("\n");
    printf("\t please press the key: \n\n");

    return;
}

void sample_print_adebayer_info(const void *arg)
{
    printf ("enter ADEBAYER test!\n");
}

XCamReturn sample_adebayer_en(const rk_aiq_sys_ctx_t* ctx, rk_aiq_debayer_op_mode_t mode, bool en)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_SAMPLE_CHECK_RET(ret, "param error!");
    }

    adebayer_attrib_t attr;
    ret = rk_aiq_user_api2_adebayer_GetAttrib(ctx, &attr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "get debayer attrib failed!");
    attr.enable = en;
    attr.mode = mode;
    rk_aiq_user_api2_adebayer_SetAttrib(ctx, attr);

    ret = rk_aiq_user_api2_adebayer_GetAttrib(ctx, &attr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "get ldch attrib failed!");

    printf ("sync_mode: %d, done: %d\n", attr.sync.sync_mode, attr.sync.done);

    return ret;
}

XCamReturn sample_adebayer_setSharpStrength(const rk_aiq_sys_ctx_t* ctx, unsigned char *strength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_SAMPLE_CHECK_RET(ret, "param error!");
    }

    adebayer_attrib_t attr;
    ret = rk_aiq_user_api2_adebayer_GetAttrib(ctx, &attr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "get debayer attrib failed!");
    attr.mode = RK_AIQ_DEBAYER_MODE_AUTO;
    memcpy(attr.stAuto.sharp_strength, strength, sizeof(attr.stAuto.sharp_strength));
    rk_aiq_user_api2_adebayer_SetAttrib(ctx, attr);

    ret = rk_aiq_user_api2_adebayer_GetAttrib(ctx, &attr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "get ldch attrib failed!");

    printf ("mode: %d, sync_mode: %d, done: %d\n", attr.mode, attr.sync.sync_mode, attr.sync.done);

    return ret;
}

XCamReturn sample_adebayer_setLowFreqThresh(const rk_aiq_sys_ctx_t* ctx, __u8 thresh)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_SAMPLE_CHECK_RET(ret, "param error!");
    }

    adebayer_attrib_t attr;
    ret = rk_aiq_user_api2_adebayer_GetAttrib(ctx, &attr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "get debayer attrib failed!");
    attr.mode = RK_AIQ_DEBAYER_MODE_AUTO;
    attr.stAuto.low_freq_thresh = thresh;
    rk_aiq_user_api2_adebayer_SetAttrib(ctx, attr);

    ret = rk_aiq_user_api2_adebayer_GetAttrib(ctx, &attr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "get ldch attrib failed!");

    printf ("mode: %d, sync_mode: %d, done: %d\n", attr.mode, attr.sync.sync_mode, attr.sync.done);

    return ret;
}

XCamReturn sample_adebayer_setHighFreqThresh(const rk_aiq_sys_ctx_t* ctx, __u8 thresh)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_SAMPLE_CHECK_RET(ret, "param error!");
    }

    adebayer_attrib_t attr;
    ret = rk_aiq_user_api2_adebayer_GetAttrib(ctx, &attr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "get debayer attrib failed!");
    attr.mode = RK_AIQ_DEBAYER_MODE_AUTO;
    attr.stAuto.high_freq_thresh = thresh;
    rk_aiq_user_api2_adebayer_SetAttrib(ctx, attr);

    printf ("mode: %d, sync_mode: %d, done: %d\n", attr.mode, attr.sync.sync_mode, attr.sync.done);

    return ret;
}

/* debayer json params from os04a10_DH3588AVS6_default.json*/
XCamReturn
sample_adebayer_translate_params(adebayer_attrib_manual_t& stManual, int32_t ISO)
{
    int8_t filter1_coe[5] = {2, -6, 0, 6, -2};
    int8_t filter2_coe[5] = {2, -4, 4, -4, 2};

    for (int i = 0; i < 5; i++)
    {
        stManual.filter1[i] =  filter1_coe[i];
        stManual.filter2[i] =  filter2_coe[i];
    }
    stManual.gain_offset = 4;

    int sharp_strength_tmp[9];
    uint16_t iso[9]           = {50, 100, 200, 400, 800, 1600, 3200, 6400, 12800};
    uint8_t sharp_strength[9] = {1, 4, 4, 4, 4, 4, 4, 4, 4};
    uint8_t hf_offset[9]      = {1, 1, 1, 1, 1, 1, 1, 1, 1};

    for (int i = 0; i < 9; i ++)
    {
        float iso_index = iso[i];
        int gain = (int)(log((float)iso_index / 50) / log((float)2));
        sharp_strength_tmp[gain] = sharp_strength[i];
    }
    stManual.offset = 1;

    int hfOffset_tmp[9];
    for (int i = 0; i < 9; i ++)
    {
        float iso_index = iso[i];
        int gain = (int)(log((float)iso_index / 50) / log((float)2));
        hfOffset_tmp[gain]  = hf_offset[i];
    }
    stManual.clip_en        = 1;
    stManual.filter_g_en    = 1;
    stManual.filter_c_en    = 1;
    stManual.thed0          = 3;
    stManual.thed1          = 6;
    stManual.dist_scale     = 8;
    stManual.shift_num      = 2;

    //select sharp params
    int iso_low = ISO, iso_high = ISO;
    int gain_high, gain_low;
    float ratio = 0.0f;
    int iso_div             = 50;
    int max_iso_step        = 9;
    for (int i = max_iso_step - 1; i >= 0; i--)
    {
        if (ISO < iso_div * (2 << i))
        {
            iso_low = iso_div * (2 << (i)) / 2;
            iso_high = iso_div * (2 << i);
        }
    }
    ratio = (float)(ISO - iso_low) / (iso_high - iso_low);
    if (iso_low == ISO)
    {
        iso_high = ISO;
        ratio = 0;
    }
    if (iso_high == ISO )
    {
        iso_low = ISO;
        ratio = 1;
    }
    gain_high = (int)(log((float)iso_high / 50) / log((float)2));
    gain_low = (int)(log((float)iso_low / 50) / log((float)2));

    gain_low = MIN(MAX(gain_low, 0), 8);
    gain_high = MIN(MAX(gain_high, 0), 8);

    stManual.sharp_strength = ((ratio) * (sharp_strength_tmp[gain_high] - sharp_strength_tmp[gain_low]) + sharp_strength_tmp[gain_low]);
    stManual.hf_offset = ((ratio) * (hfOffset_tmp[gain_high] - hfOffset_tmp[gain_low]) + hfOffset_tmp[gain_low]);

    printf ("sharp_strength: %d, hf_offset: %d\n", stManual.sharp_strength, stManual.hf_offset);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn sample_adebayer_setManualAtrrib(const rk_aiq_sys_ctx_t* ctx, int32_t ISO)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_SAMPLE_CHECK_RET(ret, "param error!");
    }

    adebayer_attrib_t attr;
    ret = rk_aiq_user_api2_adebayer_GetAttrib(ctx, &attr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "get debayer attrib failed!");
    attr.mode = RK_AIQ_DEBAYER_MODE_MANUAL;
    sample_adebayer_translate_params(attr.stManual, ISO);
    rk_aiq_user_api2_adebayer_SetAttrib(ctx, attr);

    printf ("mode: %d, sync_mode: %d, done: %d\n", attr.mode, attr.sync.sync_mode, attr.sync.done);

    return ret;
}

XCamReturn sample_adebayer_module (const void *arg)
{
    int key = -1;
    CLEAR();

    const demo_context_t *demo_ctx = (demo_context_t *)arg;
    const rk_aiq_sys_ctx_t* ctx;
    if (demo_ctx->camGroup) {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->camgroup_ctx);
    } else {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->aiq_ctx);
    }

    if (ctx == nullptr) {
        ERR ("%s, ctx is nullptr\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    sample_adebayer_usage ();
    do {

        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar ();
        printf ("\n");

        switch (key)
        {
        case 'h':
            CLEAR();
            sample_adebayer_usage ();
            break;
        case '0': {
            static bool on = false;
            on = !on;
            sample_adebayer_en(ctx, RK_AIQ_DEBAYER_MODE_AUTO, on);
            printf("%s adebayer\n\n", on ? "enable" : "disable");
            break;
        }
        case '1': {
            unsigned char sharp_strength[9] = {250,250,250,250,250,250,250,250,250};
            sample_adebayer_setSharpStrength(ctx, sharp_strength);
            printf("test the sharp_strength of 255 in sync mode...\n");
            break;
        }
        case '2': {
            unsigned char sharp_strength[9] = {0,0,0,0,0,0,0,0,0};
            sample_adebayer_setSharpStrength(ctx, sharp_strength);
            printf("test the sharp_strength of 0 in sync mode...\n");
            break;
        }
        case '3':
            sample_adebayer_setHighFreqThresh(ctx, 250);
            printf("test the high freq thresh of 250 in sync mode...\n");
            break;
        case '4':
            sample_adebayer_setHighFreqThresh(ctx, 0);
            printf("test the high freq thresh of 0 in sync mode...\n");
            break;
        case '5':
            sample_adebayer_setLowFreqThresh(ctx, 250);
            printf("test the low freq thresh of 250 in sync mode...\n");
            break;
        case '6':
            sample_adebayer_setLowFreqThresh(ctx, 0);
            printf("test the low freq thresh of 0 in sync mode...\n");
            break;
        case '7':
            static bool on = false;
            on = !on;
            sample_adebayer_en(ctx, RK_AIQ_DEBAYER_MODE_MANUAL, on);
            printf("%s adebayer\n\n", on ? "enable" : "disable");
            break;
        case '8':
            sample_adebayer_setManualAtrrib(ctx, 50);
            printf("set manual params from json with MANUAL mode in sync, ISO: 50\n");
            break;
        case '9':
            sample_adebayer_setManualAtrrib(ctx, 6400);
            printf("set manual params from json with MANUAL mode in sync, ISO: 6400\n");
            break;
        default:
            break;
        }
    } while (key != 'q' && key != 'Q');

    return XCAM_RETURN_NO_ERROR;
}
