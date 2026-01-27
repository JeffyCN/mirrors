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

#include <inttypes.h>
#include <sys/stat.h>

#include "algos/rk_aiq_api_types_ldc.h"
#include "sample_comm.h"
#include "uAPI2/rk_aiq_user_api2_ldc.h"

#ifdef USE_NEWSTRUCT
#include "uAPI2/rk_aiq_user_api2_helper.h"
#else
#include "uAPI2/rk_aiq_user_api2_aldc.h"
#endif

static void sample_aldc_usage()
{
    printf("Usage : \n");
    printf("\t --- ALDC:        module interface ---\n");
    printf("\n");
    printf("\t 0) ALDC:         enable/disable LDCH.\n");
    printf("\t 1) ALDC:         set the correction level for LDCH iteratively.\n");
    printf("\t 2) ALDC:         assigned correction level for LDCH.\n");
    printf("\t 3) ALDC:         set manual attrib for LDCH.\n");
    printf("\n");
    printf("\t --- ALDC:        imgProc interface ---\n");
    printf("\n");
    printf("\t 4) ALDC:         enable/disable LDCH.\n");
    printf("\t 5) ALDC:         set the correction level of LDCH iteratively\n");
    printf("\t 6) ALDC:         enable/disable LDCH && LDCV.\n");
    printf("\t 7) ALDC:         set the correction level of LDCH && LDCV iteratively\n");
    printf("\n");
    printf("\t --- ALDC:        tunning tool interface ---\n");
    printf("\n");
    printf("\t 8) ALDC:         enable/disable LDCH.\n");
    printf("\t 9) ALDC:         set the correction level of LDCH.\n");
    printf("\t 10) ALDC:        enable/disable LDCH && LDCV.\n");
    printf("\t 11) ALDC:        set the correction level of LDCH && LDCV.\n");
    printf("\n");

    printf("\t h) ALDC: help.\n");
    printf("\t q/Q) ALDC:       return to main sample screen.\n");
    printf("\n");
    printf("\t please press the key: \n\n");

    return;
}

void sample_print_aldc_info(const void *arg)
{
    printf ("enter ALDC test!\n");
}

static bool read_lut_from_file(const char* fileName, void** vaddr, uint32_t* size) {
    FILE* ofp;
    ofp = fopen(fileName, "rb");
    if (ofp != NULL) {
        struct stat file_stat;
        if (fstat(fileno(ofp), &file_stat) == -1) {
            ERR("Failed to get file size\n");
            return false;
        }

        *vaddr = malloc(file_stat.st_size);
        if (!*vaddr) {
            ERR("Failed to malloc size %" PRId64 "\n", file_stat.st_size);
            return false;
        }

        unsigned short hpic = 0, vpic = 0, hsize = 0, vsize = 0, hstep = 0, vstep = 0;

        int len = fread(&hpic, sizeof(unsigned short), 1, ofp);
        len = fread(&vpic, sizeof(unsigned short), 1, ofp);
        len = fread(&hsize, sizeof(unsigned short), 1, ofp);
        len = fread(&vsize, sizeof(unsigned short), 1, ofp);
        len = fread(&hstep, sizeof(unsigned short), 1, ofp);
        len = fread(&vstep, sizeof(unsigned short), 1, ofp);
        if (!len)
            ERR("Failed to read header info\n");
        else
            printf("lut info: [%d-%d-%d-%d-%d-%d]\n", hpic, vpic, hsize, vsize, hstep, vstep);

        *size            = file_stat.st_size - 6 * sizeof(unsigned short);
        unsigned int num = fread(*vaddr, 1, *size, ofp);
        fclose(ofp);

        if (num != *size) {
            printf("mismatched lut calib file\n");
            return false;
        }
        printf("check lut %s, size: %d, num: %d\n", fileName, *size, num);

        uint16_t* addr = (uint16_t*)(*vaddr);
        printf("lut[0:15]: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", addr[0], addr[1],
               addr[2], addr[3], addr[4], addr[5], addr[6], addr[7], addr[8], addr[9], addr[10],
               addr[11], addr[12], addr[13], addr[14], addr[15]);
    } else {
        printf("lut calib file %s not exist\n", fileName);
        return false;
    }

    return true;
}

#ifdef USE_NEWSTRUCT
XCamReturn sample_aldc_en(const rk_aiq_sys_ctx_t* ctx, bool en) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ldc_api_attrib_t attr;

    ret = rk_aiq_user_api2_ldc_GetAttrib(ctx, &attr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "Failed to get ldc attrib.");

    printf("Get ldc attr: en %d, bypass %d, opMode %d.\n", attr.en, attr.bypass, attr.opMode);
    printf("Get ldc attr tunning: level %d, level_max %d.\n",
           attr.tunning.autoGenMesh.sw_ldcT_correctStrg_val,
           attr.tunning.autoGenMesh.sw_ldcC_correctStrg_maxLimit);

    attr.en     = en;
    attr.opMode = RK_AIQ_OP_MODE_AUTO;
    attr.bypass = 0;

    ret = rk_aiq_user_api2_ldc_SetAttrib(ctx, &attr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "Failed to set ldc attrib.");

    return ret;
}

XCamReturn sample_aldc_setCorrectLevel(const rk_aiq_sys_ctx_t* ctx, int level) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ldc_api_attrib_t attr;

    ret = rk_aiq_user_api2_ldc_GetAttrib(ctx, &attr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "Failed to get ldc attrib.");

    printf("Get ldc attr: en %d, bypass %d, opMode %d.\n", attr.en, attr.bypass, attr.opMode);
    printf("Get ldc attr tunning: level %d, level_max %d.\n",
           attr.tunning.autoGenMesh.sw_ldcT_correctStrg_val,
           attr.tunning.autoGenMesh.sw_ldcC_correctStrg_maxLimit);

    attr.bypass                                      = 0;
    attr.tunning.autoGenMesh.sw_ldcT_correctStrg_val = level;

    ret = rk_aiq_user_api2_ldc_SetAttrib(ctx, &attr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "Failed to set ldc attrib.");

    return ret;
}

#define LDCH_MESH_1 "/tmp/ldch_mesh_1.bin"
#define LDCH_MESH_2 "/tmp/ldch_mesh_2.bin"
XCamReturn sample_aldc_setManualAttrib(const rk_aiq_sys_ctx_t* ctx, bool en) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_SAMPLE_CHECK_RET(ret, "param error!");
    }

    ldc_param_t attr;
    if (rk_aiq_user_api2_ldc_GetManualAttrib(ctx, &attr) < 0)
        ERR("Failed ro get manual Attrib from LDC");

    if (en) {
        static bool flag   = false;
        char filename[512] = "\0";
        if (flag)
            sprintf(filename, "%s", LDCH_MESH_1);
        else
            sprintf(filename, "%s", LDCH_MESH_2);
        flag = !flag;

        uint32_t size = 0;
        if (read_lut_from_file(filename, &attr.sta.ldchCfg.lutMapCfg.sw_ldcT_lutMapBuf_vaddr[0],
                               &size)) {
            attr.sta.ldchCfg.en                            = true;
            attr.sta.ldchCfg.lutMapCfg.sw_ldcT_lutMap_size = size;

            ret = rk_aiq_user_api2_ldc_SetManualAttrib(ctx, &attr);
            RKAIQ_SAMPLE_CHECK_RET(ret, "Ldc set manual attrib error!\n");
        }
    }

    return ret;
}

static void sample_ldc_tuningtool_en(const rk_aiq_sys_ctx_t* ctx, bool en) {
    char* ret_str = NULL;
    const char* json_ldc_attr_str =
        " \n\
        [{ \n\
            \"op\":\"get\", \n\
            \"path\": \"/uapi/0/ldc_uapi/info/\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_AUTO\", \"en\": 0,\"bypass\": 0} \n\
        }]";

    rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx), json_ldc_attr_str, &ret_str,
                           RKAIQUAPI_OPMODE_GET);

    if (ret_str) {
        printf("get ldc_uapi/attr str:\n%s\n", ret_str);
    }

    printf("set en to %s...\n", en ? "on" : "off");

    const char* json_ldc_on_str =
        " \n\
        [{ \n\
            \"op\":\"replace\", \n\
            \"path\": \"/uapi/0/ldc_uapi/attr/en\", \n\
            \"value\": 1 \n\
        }]";

    const char* json_ldc_off_str =
        " \n\
        [{ \n\
            \"op\":\"replace\", \n\
            \"path\": \"/uapi/0/ldc_uapi/attr/en\", \n\
            \"value\": 0 \n\
        }]";

    ret_str = NULL;
    if (en)
        rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx), json_ldc_on_str, &ret_str,
                               RKAIQUAPI_OPMODE_SET);
    else
        rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx), json_ldc_off_str, &ret_str,
                               RKAIQUAPI_OPMODE_SET);
}

void sample_ldc_tuningtool_correctLevel(const rk_aiq_sys_ctx_t* ctx, int bypass) {
    char* ret_str = NULL;
    const char* json_ldc_attr_str =
        " \n\
        [{ \n\
            \"op\":\"get\", \n\
            \"path\": \"/uapi/0/ldc_uapi/info/\", \n\
            \"value\": \n\
            { \"opMode\": \"RK_AIQ_OP_MODE_AUTO\", \"en\": 0,\"bypass\": 0} \n\
        }]";

    rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx), json_ldc_attr_str, &ret_str,
                           RKAIQUAPI_OPMODE_GET);

    if (ret_str) {
        printf("get ldc_uapi/attr str:\n%s\n", ret_str);
    }

    const char* json_ldc_level0_str =
        " \n\
        [{ \n\
            \"op\":\"replace\", \n\
            \"path\": \"/uapi/0/ldc_uapi/attr/tunning/autoGenMesh/sw_ldcT_correctStrg_val\", \n\
            \"value\": 0 \n\
        }]";

    const char* json_ldc_level200_str =
        " \n\
        [{ \n\
            \"op\":\"replace\", \n\
            \"path\": \"/uapi/0/ldc_uapi/attr/tunning/autoGenMesh/sw_ldcT_correctStrg_val\", \n\
            \"value\": 200 \n\
        }]";

    ret_str = NULL;
    if (bypass)
        rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx), json_ldc_level0_str, &ret_str,
                               RKAIQUAPI_OPMODE_SET);
    else
        rkaiq_uapi_unified_ctl((rk_aiq_sys_ctx_t*)(ctx), json_ldc_level200_str, &ret_str,
                               RKAIQUAPI_OPMODE_SET);
}

#else

XCamReturn sample_aldc_en(const rk_aiq_sys_ctx_t* ctx, bool en)
{
    XCamReturn ret = XCAM_RETURN_BYPASS;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_SAMPLE_CHECK_RET(ret, "param error!");
    }
    return ret;
}

XCamReturn sample_aldc_setCorrectLevel(const rk_aiq_sys_ctx_t* ctx, int correctLevel,  rk_aiq_uapi_mode_sync_e sync)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_SAMPLE_CHECK_RET(ret, "param error!");
    }
    rk_aiq_ldc_attrib_t ldcAttr;
    memset(&ldcAttr, 0, sizeof(ldcAttr));
    ret = rk_aiq_user_api2_aldc_GetAttrib(ctx, &ldcAttr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "get ldc attrib failed!");
    ldcAttr.sync.sync_mode = sync;
    ldcAttr.ldch.correct_level = correctLevel;
    ldcAttr.ldcv.correct_level = correctLevel;
    ret = rk_aiq_user_api2_aldc_SetAttrib(ctx, &ldcAttr);
    ret = rk_aiq_user_api2_aldc_GetAttrib(ctx, &ldcAttr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "get ldch attrib failed!");

    printf ("sync_mode: %d, level: %d, done: %d\n", ldcAttr.sync.sync_mode, correctLevel, ldcAttr.sync.done);

    return ret;
}
#endif

XCamReturn sample_aldc_module (const void *arg)
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

    if (ctx == NULL) {
        ERR ("%s, ctx is nullptr\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    sample_aldc_usage ();
    do {
        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar ();
        printf ("\n");

        switch (key)
        {
        case 'h':
            CLEAR();
            sample_aldc_usage ();
            break;
        case '0': {
            static bool on = false;
            on             = !on;
            sample_aldc_en(ctx, on);
            printf("%s aldc\n\n", on ? "enable" : "disable");
            break;
        }
        case '1':
            printf("test the correction level of ALDCH iteratively in sync mode...\n");
            for (int level = 1; level <= 255; level++) {
                usleep(100*1000);
#ifdef USE_NEWSTRUCT
                sample_aldc_setCorrectLevel(ctx, level);
#else
                sample_aldc_setCorrectLevel(ctx, level, RK_AIQ_UAPI_MODE_DEFAULT);
#endif
            }
            printf("end of the test\n\n");
            break;
        case '2':
            {
                static int32_t level = 0;
                if (level == 0)
                    level = 200;
                else
                    level = 0;

                printf("test the correction level : %d\n", level);
#ifdef USE_NEWSTRUCT
                sample_aldc_setCorrectLevel(ctx, level);
#else
                sample_aldc_setCorrectLevel(ctx, level, RK_AIQ_UAPI_MODE_DEFAULT);
#endif
                break;
            }
            case '3': {
#ifdef USE_NEWSTRUCT
                static bool on = false;
                on             = !on;
                sample_aldc_setManualAttrib(ctx, on);
                printf("%s aldc in manual mode\n\n", on ? "enable" : "disable");
                break;
#endif
            }
            case '4': {
                static bool on = false;
                on             = !on;
                rk_aiq_uapi2_setLdchEn(ctx, on);
                printf("%s aldc\n\n", on ? "enable" : "disable");
                break;
            }
            case '5':
#ifdef USE_NEWSTRUCT
                printf("test the correction level of LDCH iteratively by imgproc interface.\n");
                for (int level = 1; level <= 255; level++) {
                    usleep(100 * 1000);
                    rk_aiq_uapi2_setLdchCorrectLevel(ctx, level);
                }
                printf("end of the test\n\n");
#endif
                break;

            case '6': {
#ifdef USE_NEWSTRUCT
                static bool on = false;
                on             = !on;
                printf("%s LDCH && LDCV\n\n", on ? "enable" : "disable");
                rk_aiq_uapi2_setLdchLdcvEn(ctx, on);
#endif
                break;
            }
            case '7':
#ifdef USE_NEWSTRUCT
                printf(
                    "test the correction level of LDCH && LDCV iteratively by imgproc "
                    "interface.\n");
                for (int level = 1; level <= 255; level++) {
                    usleep(100 * 1000);
                    rk_aiq_uapi2_setLdchLdcvCorrectLevel(ctx, level);
                }
                printf("end of the test\n\n");
#endif
                break;

            case '8': {
#ifdef USE_NEWSTRUCT
                static bool on = false;
                on             = !on;
                sample_ldc_tuningtool_en(ctx, on);
#endif
                break;
            }
            case '9': {
#ifdef USE_NEWSTRUCT
                static bool bypass = false;
                bypass             = !bypass;
                sample_ldc_tuningtool_correctLevel(ctx, bypass);
#endif
                break;
            }

            default:
                break;
        }
    } while (key != 'q' && key != 'Q');

    return XCAM_RETURN_NO_ERROR;
}
