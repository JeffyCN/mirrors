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


static void sample_hsv_usage()
{
    printf("Usage : \n");
    printf("  ImgProc API: \n");
    printf("\t 0) hsv:         Set hsv MANUAL Mode.\n");
    printf("\t 1) hsv:         Set hsv AUTO Mode.\n");
    printf("\t 2) hsv:         Get hsv Mode.\n");
    printf("\t 3) hsv:         Set hsv Manual Lut.\n");
    printf("\t 4) hsv:         Get hsv Lut.\n");
    printf("\t 5) hsv:         Get hsv Strength.\n");
    printf("\t 6) hsv:         Get hsv LutName.\n");
    printf("  Module API: \n");
    printf("\t o) hsv:         Set calib test.\n");
    printf("\t m) hsv:         Set and get attrib test.\n");
    printf("\n");
    printf("\t h) hsv:         help.\n");
    printf("\t q) hsv:         return to main sample screen.\n");

    printf("\n");
    printf("\t please press the key: ");

    return;
}

void sample_print_hsv_info(const void *arg)
{
    printf ("enter HSV modult test!\n");
}

/*
******************************
*
* ImgProc level API Sample Func
*
******************************
*/

static int sample_set_hsv_manual(const rk_aiq_sys_ctx_t* ctx)
{
    opMode_t mode = OP_MANUAL;
    rk_aiq_uapi2_setLut3dMode(ctx, mode);
    return 0;
}

static int sample_set_hsv_auto(const rk_aiq_sys_ctx_t* ctx)
{
    opMode_t mode = OP_AUTO;
    rk_aiq_uapi2_setLut3dMode(ctx, mode);
    return 0;
}

static int sample_get_hsv_mode(const rk_aiq_sys_ctx_t* ctx)
{
    opMode_t mode;
    rk_aiq_uapi2_getLut3dMode(ctx, &mode);
    if (mode == OP_MANUAL)
        printf("Get 3DLUT Mode is: Manual \n");
    else if (mode == OP_AUTO)
        printf("Get 3DLUT Mode is: AUTO\n");
    else
        printf("Get 3DLUT Mode is: INVALID\n");
    return 0;
}

static int sample_set_hsv_manual_lut(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_lut3d_table_t mlut;
    memset(&mlut, 0, sizeof(mlut));
    // create mlut
    rk_aiq_uapi2_getM3dLut(ctx, &mlut);
    memcpy(mlut.look_up_table_b, mlut.look_up_table_r, sizeof(unsigned short) * 729);
    for(int i = 0; i < 729; i++)
        mlut.look_up_table_g[i] = mlut.look_up_table_r[i]<<2;
    // set mlut
    rk_aiq_uapi2_setM3dLut(ctx, &mlut);
    printf("Set 3DLUT Manual Lut\n\n");
    return 0;
}

static int sample_get_hsv_lut(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_lut3d_table_t mlut;
    rk_aiq_uapi2_getM3dLut(ctx, &mlut);
    printf("Get 3DLUT Lut: ");
    printf("\t r: %d, %d, %d \n", mlut.look_up_table_r[0],
                                    mlut.look_up_table_r[1],
                                    mlut.look_up_table_r[2]);
    printf("\t g: %d, %d, %d \n", mlut.look_up_table_g[0],
                                    mlut.look_up_table_g[1],
                                    mlut.look_up_table_g[2]);
    printf("\t b: %d, %d, %d \n", mlut.look_up_table_b[0],
                                    mlut.look_up_table_b[1],
                                    mlut.look_up_table_b[2]);
    return 0;
}

static int sample_get_hsv_strength(const rk_aiq_sys_ctx_t* ctx)
{
    float alpha;
    rk_aiq_uapi2_getA3dLutStrth(ctx, &alpha);
    printf("Get A3DLUT Strength: %f\n\n", alpha);
    return 0;
}

static int sample_get_hsv_lutname(const rk_aiq_sys_ctx_t* ctx)
{
    char lutname[25];
    rk_aiq_uapi2_getA3dLutName(ctx, lutname);
    printf("Get A3DLUT LutName: %s\n\n", lutname);
    return 0;
}

#if USE_NEWSTRUCT && !defined(ISP_HW_V39)
static int sample_hsv_test(const rk_aiq_sys_ctx_t* ctx)
{
    // get cur mode
    printf("+++++++ hsv module test start ++++++++\n");

    hsv_api_attrib_t attr;
    memset(&attr, 0, sizeof(attr));

    rk_aiq_user_api2_hsv_GetAttrib(ctx, &attr);

    printf("hsv attr: opmode:%d, en:%d, bypass:%d\n", attr.opMode, attr.en, attr.bypass);

    srand(time(0));
    int rand_num = rand() % 101;

    if (rand_num <70) {
        printf("update hsv arrrib!\n");
        if (attr.opMode == RK_AIQ_OP_MODE_AUTO) {
            attr.opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        else {
            attr.opMode = RK_AIQ_OP_MODE_AUTO;
        }
    }
    else {
        // reverse en
        printf("reverse hsv en!\n");
        attr.en = !attr.en;
    }

    rk_aiq_user_api2_hsv_SetAttrib(ctx, &attr);

    // wait more than 2 frames
    usleep(180 * 1000);

    hsv_status_t status;
    memset(&status, 0, sizeof(hsv_status_t));

    rk_aiq_user_api2_hsv_QueryStatus(ctx, &status);

    printf("hsv status: opmode:%d, en:%d, bypass:%d\n", status.opMode, status.en, status.bypass);

    if (status.opMode != attr.opMode || status.en != attr.en)
        printf("hsv test failed\n");
    printf("-------- hsv module test done --------\n");

    return 0;
}

static int sample_query_hsv_status(const rk_aiq_sys_ctx_t* ctx)
{
    hsv_status_t info;
    rk_aiq_user_api2_hsv_QueryStatus(ctx, &info);
    printf("Query hsv status:\n\n");
    printf("  opMode: %d, en: %d, bypass: %d,\n"
           "  stMan: {\n    sta: %d, %d, %d,\n    dyn: {lut1d0={mode: %d, [%d,...,%d,...]}, lut1d1={mode: %d, [%d,...,%d,...]}, lut2d={mode: %d, [...,%d,...,%d,...]}}\n"
           "  }\n  astatus: {illu: %s, alp: %f}\n", 
            info.opMode, info.en, info.bypass,
            info.stMan.sta.hw_hsvT_lut1d0_en,
            info.stMan.sta.hw_hsvT_lut1d1_en,
            info.stMan.sta.hw_hsvT_lut2d_en,
            info.stMan.dyn.lut1d0.hw_hsvT_lut1d_mode,
            info.stMan.dyn.lut1d0.hw_hsvT_lut1d_val[0],
            info.stMan.dyn.lut1d0.hw_hsvT_lut1d_val[32],
            info.stMan.dyn.lut1d1.hw_hsvT_lut1d_mode,
            info.stMan.dyn.lut1d1.hw_hsvT_lut1d_val[0],
            info.stMan.dyn.lut1d1.hw_hsvT_lut1d_val[32],
            info.stMan.dyn.lut2d.hw_hsvT_lut2d_mode,
            info.stMan.dyn.lut2d.hw_hsvT_lut2d_val[16],
            info.stMan.dyn.lut2d.hw_hsvT_lut2d_val[144],
            info.ahsvStatus.sw_hsvC_illuUsed_name,
            info.ahsvStatus.sw_hsvT_alpha_val);
    return 0;
}

static int sample_hsv_setCalib_test(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ahsv_hsvCalib_t calib;
    memset(&calib, 0, sizeof(ahsv_hsvCalib_t));
    //get
    ret = rk_aiq_user_api2_hsv_GetCalib(ctx, &calib);
    RKAIQ_SAMPLE_CHECK_RET(ret, "Get hsv CALIB failed!");
    printf("GetCALIB:\n\n");
    printf("\t effect Table_len = %d\n", calib.sw_hsvCfg_tblAll_len);
    for (int i = 0; i < calib.sw_hsvCfg_tblAll_len; i++) {
        printf("\t %s = {lut1d0={mode: %d, [%d,...,%d,...]}, lut1d1={mode: %d, [%d,...,%d,...]}, lut2d={mode: %d, [...,%d,...,%d,...]}}, \n",
                calib.tableAll[i].sw_hsvC_illu_name,
                calib.tableAll[i].meshGain.lut1d0.hw_hsvT_lut1d_mode,
                calib.tableAll[i].meshGain.lut1d0.hw_hsvT_lut1d_val[0],
                calib.tableAll[i].meshGain.lut1d0.hw_hsvT_lut1d_val[32],
                calib.tableAll[i].meshGain.lut1d1.hw_hsvT_lut1d_mode,
                calib.tableAll[i].meshGain.lut1d1.hw_hsvT_lut1d_val[0],
                calib.tableAll[i].meshGain.lut1d1.hw_hsvT_lut1d_val[32],
                calib.tableAll[i].meshGain.lut2d.hw_hsvT_lut2d_mode,
                calib.tableAll[i].meshGain.lut2d.hw_hsvT_lut2d_val[16],
                calib.tableAll[i].meshGain.lut2d.hw_hsvT_lut2d_val[144]);
    }
    //modify
    srand(time(0));
    int rand_num = rand() % 101;

    if (rand_num <70) {
        printf("update lsc calib!\n");
        calib.tableAll[0].meshGain.lut1d0.hw_hsvT_lut1d_val[0] += 32;
        calib.tableAll[0].meshGain.lut1d0.hw_hsvT_lut1d_val[32] += 32;
        calib.tableAll[0].meshGain.lut1d1.hw_hsvT_lut1d_val[0] -= 64;
        calib.tableAll[0].meshGain.lut1d1.hw_hsvT_lut1d_val[32] -= 64;
        calib.tableAll[0].meshGain.lut2d.hw_hsvT_lut2d_val[16] += 64;
        calib.tableAll[0].meshGain.lut2d.hw_hsvT_lut2d_val[144] +=64;

        calib.tableAll[1].meshGain.lut1d0.hw_hsvT_lut1d_val[0] += 64;
        calib.tableAll[1].meshGain.lut1d0.hw_hsvT_lut1d_val[32] += 64;
        calib.tableAll[1].meshGain.lut1d1.hw_hsvT_lut1d_val[0] -= 128;
        calib.tableAll[1].meshGain.lut1d1.hw_hsvT_lut1d_val[32] -= 128;
        calib.tableAll[1].meshGain.lut2d.hw_hsvT_lut2d_val[16] += 10;
        calib.tableAll[1].meshGain.lut2d.hw_hsvT_lut2d_val[144] +=10;
    } else {
        memcpy(&calib.tableAll[0], &calib.tableAll[calib.sw_hsvCfg_tblAll_len-1], sizeof(ahsv_tableAll_t));
        if (calib.sw_hsvCfg_tblAll_len > 1)
            calib.sw_hsvCfg_tblAll_len -= 1;
    }

    rk_aiq_user_api2_hsv_SetCalib(ctx, &calib);
    
    // wait more than 2 frames
    usleep(90 * 1000);

    ahsv_hsvCalib_t calib_new;
    memset(&calib_new, 0, sizeof(ahsv_hsvCalib_t));

    rk_aiq_user_api2_hsv_GetCalib(ctx, &calib_new);

    printf("\t new table_len = %d\n", calib_new.sw_hsvCfg_tblAll_len);
    for (int i = 0; i < calib_new.sw_hsvCfg_tblAll_len; i++) {
        printf("\t %s = {lut1d0={mode: %d, [%d,...,%d,...]}, lut1d1={mode: %d, [%d,...,%d,...]}, lut2d={mode: %d, [...,%d,...,%d,...]}}, \n",
                calib_new.tableAll[i].sw_hsvC_illu_name,
                calib_new.tableAll[i].meshGain.lut1d0.hw_hsvT_lut1d_mode,
                calib_new.tableAll[i].meshGain.lut1d0.hw_hsvT_lut1d_val[0],
                calib_new.tableAll[i].meshGain.lut1d0.hw_hsvT_lut1d_val[32],
                calib_new.tableAll[i].meshGain.lut1d1.hw_hsvT_lut1d_mode,
                calib_new.tableAll[i].meshGain.lut1d1.hw_hsvT_lut1d_val[0],
                calib_new.tableAll[i].meshGain.lut1d1.hw_hsvT_lut1d_val[32],
                calib_new.tableAll[i].meshGain.lut2d.hw_hsvT_lut2d_mode,
                calib_new.tableAll[i].meshGain.lut2d.hw_hsvT_lut2d_val[16],
                calib_new.tableAll[i].meshGain.lut2d.hw_hsvT_lut2d_val[144]);
    }
    if (calib_new.sw_hsvCfg_tblAll_len != calib.sw_hsvCfg_tblAll_len || 
        calib_new.tableAll[0].meshGain.lut1d0.hw_hsvT_lut1d_val[0] != calib.tableAll[0].meshGain.lut1d0.hw_hsvT_lut1d_val[0])
        printf("hsv calib test failed\n");
    printf("-------- hsv module calib test done --------\n");  

    return 0;
}
#endif

XCamReturn sample_hsv_module(const void* arg)
{
    int key = -1;
    CLEAR();

    const demo_context_t *demo_ctx = (demo_context_t *)arg;
    const rk_aiq_sys_ctx_t* ctx;
    if (demo_ctx->camGroup){
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->camgroup_ctx);
    } else {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->aiq_ctx);
    }

    /*TODO: when rkaiq_3A_server & rkisp_demo run in two different shell, rk_aiq_sys_ctx_t would be null?*/
    if (ctx == NULL) {
        ERR ("%s, ctx is nullptr\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    sample_hsv_usage ();

    do {

        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar();
        printf ("\n");

        switch (key)
        {
            case 'h':
                CLEAR();
                sample_hsv_usage ();
                break;
#if USE_NEWSTRUCT && !defined(ISP_HW_V39)
            case '0':
                sample_set_hsv_manual(ctx);
                printf("Set hsv MANUAL Mode\n\n");
                break;
            case '1':
                sample_set_hsv_auto(ctx);
                printf("Set hsv AUTO Mode\n\n");
                break;
            case '2':
                sample_get_hsv_mode(ctx);
                break;
            case '3':
                sample_set_hsv_manual_lut(ctx);
                break;
            case '4':
                sample_get_hsv_lut(ctx);
                break;
            case '5':
                sample_get_hsv_strength(ctx);
                break;
            case '6':
                sample_get_hsv_lutname(ctx);
                break;
            case 'm':
                sample_hsv_test(ctx);
                break;
            case 'o':
                sample_query_hsv_status(ctx);
                sample_hsv_setCalib_test(ctx);
                break;
#endif
            default:
                break;
        }
    } while (key != 'q' && key != 'Q');

    return XCAM_RETURN_NO_ERROR;
}
