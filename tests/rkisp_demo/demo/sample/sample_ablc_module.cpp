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

static void sample_ablc_usage()
{
    printf("Usage : \n");
    printf("\t 0) ABLC:         get ablc attri on sync mode.\n");
    printf("\t 1) ABLC:         set ablc attri auto on sync mode.\n");
    printf("\t 2) ABLC:         set ablc attri manual on sync mode.\n");
    printf("\t 3) ABLC:         set ablc attri to default value on sync mode.\n");
    printf("\t 4) ABLC:         get ablc attri on async mode.\n");
    printf("\t 5) ABLC:         set ablc attri auto on async mode.\n");
    printf("\t 6) ABLC:         set ablc attri manual on async mode.\n");
    printf("\t 7) ABLC:         set ablc attri to default value on async mode.\n");
    printf("\t q) ABLC:         press key q or Q to quit.\n");
}

void sample_print_ablc_info(const void *arg)
{
    printf ("enter ABLC modult test!\n");
}

void sample_blc_attr_relese(rk_aiq_blc_attrib_t *pAttr)
{
    if(pAttr->stBlc0Auto.iso)
        free(pAttr->stBlc0Auto.iso);

    if(pAttr->stBlc0Auto.blc_b)
        free(pAttr->stBlc0Auto.blc_b);

    if(pAttr->stBlc0Auto.blc_b)
        free(pAttr->stBlc0Auto.blc_gb);

    if(pAttr->stBlc0Auto.blc_gr)
        free(pAttr->stBlc0Auto.blc_gr);

    if(pAttr->stBlc0Auto.blc_r)
        free(pAttr->stBlc0Auto.blc_r);


    if(pAttr->stBlc1Auto.iso)
        free(pAttr->stBlc1Auto.iso);

    if(pAttr->stBlc1Auto.blc_b)
        free(pAttr->stBlc1Auto.blc_b);

    if(pAttr->stBlc1Auto.blc_b)
        free(pAttr->stBlc1Auto.blc_gb);

    if(pAttr->stBlc1Auto.blc_gr)
        free(pAttr->stBlc1Auto.blc_gr);

    if(pAttr->stBlc1Auto.blc_r)
        free(pAttr->stBlc1Auto.blc_r);
}

XCamReturn sample_ablc_module (const void *arg)
{
    int key = -1;
    CLEAR();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    const demo_context_t *demo_ctx = (demo_context_t *)arg;
    const rk_aiq_sys_ctx_t* ctx ;

    if (demo_ctx->camGroup) {
        ctx = (const rk_aiq_sys_ctx_t*)(demo_ctx->camgroup_ctx);
        printf("##################group !!!!########################\n");
    } else {
        ctx = (const rk_aiq_sys_ctx_t*)(demo_ctx->aiq_ctx);
        printf("##################sigle !!!!########################\n");
    }
    if (ctx == nullptr) {
        ERR ("%s, ctx is nullptr\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    rk_aiq_blc_attrib_t default_blc_attr;
    memset(&default_blc_attr, 0x00, sizeof(default_blc_attr));//important, need init first
    rk_aiq_user_api2_ablc_GetAttrib(ctx, &default_blc_attr);

    do {
        sample_ablc_usage ();

        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar ();
        printf ("\n");

        switch (key) {
        case '0':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_blc_attrib_t blc_attr;
                blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                memset(&blc_attr, 0x00, sizeof(blc_attr));//important, need init first
                ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &blc_attr);
                printf("get ablc attri ret:%d done:%d \n\n", ret, blc_attr.sync.done);
                sample_blc_attr_relese(&blc_attr);
            }
            break;
        case '1':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_blc_attrib_t blc_attr;
                blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                memset(&blc_attr, 0x00, sizeof(blc_attr));//important, need init first
                ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &blc_attr);
                blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                blc_attr.eMode = ABLC_OP_MODE_AUTO;
                blc_attr.stBlc0Auto.enable = 1;
                blc_attr.stBlc0Auto.len = 13;
                blc_attr.stBlc0Auto.iso = (float*)malloc(sizeof(float) * blc_attr.stBlc0Auto.len);
                blc_attr.stBlc0Auto.blc_r = (float*)malloc(sizeof(float) * blc_attr.stBlc0Auto.len);
                blc_attr.stBlc0Auto.blc_gr = (float*)malloc(sizeof(float) * blc_attr.stBlc0Auto.len);
                blc_attr.stBlc0Auto.blc_gb = (float*)malloc(sizeof(float) * blc_attr.stBlc0Auto.len);
                blc_attr.stBlc0Auto.blc_b = (float*)malloc(sizeof(float) * blc_attr.stBlc0Auto.len);
                for(int i = 0; i < blc_attr.stBlc0Auto.len; i++) {
                    blc_attr.stBlc0Auto.iso[i] = 50 * pow(2, i);
                    blc_attr.stBlc0Auto.blc_r[i] = 255;
                    blc_attr.stBlc0Auto.blc_gr[i] = 255;
                    blc_attr.stBlc0Auto.blc_gb[i] = 255;
                    blc_attr.stBlc0Auto.blc_b[i] = 255;
                }

                blc_attr.stBlc1Auto.enable = 1;
                blc_attr.stBlc1Auto.len = 13;
                blc_attr.stBlc1Auto.iso = (float*)malloc(sizeof(float) * blc_attr.stBlc1Auto.len);
                blc_attr.stBlc1Auto.blc_r = (float*)malloc(sizeof(float) * blc_attr.stBlc1Auto.len);
                blc_attr.stBlc1Auto.blc_gr = (float*)malloc(sizeof(float) * blc_attr.stBlc1Auto.len);
                blc_attr.stBlc1Auto.blc_gb = (float*)malloc(sizeof(float) * blc_attr.stBlc1Auto.len);
                blc_attr.stBlc1Auto.blc_b = (float*)malloc(sizeof(float) * blc_attr.stBlc1Auto.len);
                for(int i = 0; i < blc_attr.stBlc1Auto.len; i++) {
                    blc_attr.stBlc1Auto.iso[i] = 50 * pow(2, i);
                    blc_attr.stBlc1Auto.blc_r[i] = 254;
                    blc_attr.stBlc1Auto.blc_gr[i] = 254;
                    blc_attr.stBlc1Auto.blc_gb[i] = 254;
                    blc_attr.stBlc1Auto.blc_b[i] = 254;
                }

                ret = rk_aiq_user_api2_ablc_SetAttrib(ctx, &blc_attr);
                printf("set ablc attri auto ret:%d \n\n", ret);

                rk_aiq_blc_attrib_t get_blc_attr;
                get_blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                memset(&get_blc_attr, 0x00, sizeof(get_blc_attr));//important, need init first
                ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &get_blc_attr);
                printf("get ablc attri ret:%d done:%d \n\n", ret, get_blc_attr.sync.done);

                sample_blc_attr_relese(&blc_attr);
            }
            break;
        case '2':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_blc_attrib_t blc_attr;
                memset(&blc_attr, 0x00, sizeof(blc_attr));//important, need init first
                blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &blc_attr);
                blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                blc_attr.eMode = ABLC_OP_MODE_MANUAL;
                blc_attr.stBlc0Manual.enable = 1;
                blc_attr.stBlc0Manual.blc_r = 253;
                blc_attr.stBlc0Manual.blc_gr = 253;
                blc_attr.stBlc0Manual.blc_gb = 253;
                blc_attr.stBlc0Manual.blc_b = 253;

                blc_attr.stBlc1Manual.enable = 1;
                blc_attr.stBlc1Manual.blc_r = 253;
                blc_attr.stBlc1Manual.blc_gr = 253;
                blc_attr.stBlc1Manual.blc_gb = 253;
                blc_attr.stBlc1Manual.blc_b = 253;


                ret = rk_aiq_user_api2_ablc_SetAttrib(ctx, &blc_attr);
                printf("set blc attri manual ret:%d \n\n", ret);


                rk_aiq_blc_attrib_t get_blc_attr;
                get_blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                memset(&get_blc_attr, 0x00, sizeof(get_blc_attr));//important, need init first
                ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &get_blc_attr);
                printf("get ablc attri ret:%d done:%d \n\n", ret, get_blc_attr.sync.done);

            }
            break;
        case '3':
            if (CHECK_ISP_HW_V30()) {
                default_blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                ret = rk_aiq_user_api2_ablc_SetAttrib(ctx, &default_blc_attr);
                printf("set blc attri auto default value ret:%d \n\n", ret);


                rk_aiq_blc_attrib_t get_blc_attr;
                get_blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
                memset(&get_blc_attr, 0x00, sizeof(get_blc_attr));//important, need init first
                ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &get_blc_attr);
                printf("get ablc attri ret:%d done:%d \n\n", ret, get_blc_attr.sync.done);

            }
            break;
        case '4':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_blc_attrib_t blc_attr;
                blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                memset(&blc_attr, 0x00, sizeof(blc_attr));//important, need init first
                ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &blc_attr);
                printf("get ablc attri ret:%d done:%d \n\n", ret, blc_attr.sync.done);
                sample_blc_attr_relese(&blc_attr);
            }
            break;
        case '5':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_blc_attrib_t blc_attr;
                blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                memset(&blc_attr, 0x00, sizeof(blc_attr));//important, need init first
                ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &blc_attr);
                blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                blc_attr.eMode = ABLC_OP_MODE_AUTO;
                blc_attr.stBlc0Auto.enable = 1;
                blc_attr.stBlc0Auto.len = 13;
                blc_attr.stBlc0Auto.iso = (float*)malloc(sizeof(float) * blc_attr.stBlc0Auto.len);
                blc_attr.stBlc0Auto.blc_r = (float*)malloc(sizeof(float) * blc_attr.stBlc0Auto.len);
                blc_attr.stBlc0Auto.blc_gr = (float*)malloc(sizeof(float) * blc_attr.stBlc0Auto.len);
                blc_attr.stBlc0Auto.blc_gb = (float*)malloc(sizeof(float) * blc_attr.stBlc0Auto.len);
                blc_attr.stBlc0Auto.blc_b = (float*)malloc(sizeof(float) * blc_attr.stBlc0Auto.len);
                for(int i = 0; i < blc_attr.stBlc0Auto.len; i++) {
                    blc_attr.stBlc0Auto.iso[i] = 50 * pow(2, i);
                    blc_attr.stBlc0Auto.blc_r[i] = 255;
                    blc_attr.stBlc0Auto.blc_gr[i] = 255;
                    blc_attr.stBlc0Auto.blc_gb[i] = 255;
                    blc_attr.stBlc0Auto.blc_b[i] = 255;
                }

                blc_attr.stBlc1Auto.enable = 1;
                blc_attr.stBlc1Auto.len = 13;
                blc_attr.stBlc1Auto.iso = (float*)malloc(sizeof(float) * blc_attr.stBlc1Auto.len);
                blc_attr.stBlc1Auto.blc_r = (float*)malloc(sizeof(float) * blc_attr.stBlc1Auto.len);
                blc_attr.stBlc1Auto.blc_gr = (float*)malloc(sizeof(float) * blc_attr.stBlc1Auto.len);
                blc_attr.stBlc1Auto.blc_gb = (float*)malloc(sizeof(float) * blc_attr.stBlc1Auto.len);
                blc_attr.stBlc1Auto.blc_b = (float*)malloc(sizeof(float) * blc_attr.stBlc1Auto.len);
                for(int i = 0; i < blc_attr.stBlc1Auto.len; i++) {
                    blc_attr.stBlc1Auto.iso[i] = 50 * pow(2, i);
                    blc_attr.stBlc1Auto.blc_r[i] = 254;
                    blc_attr.stBlc1Auto.blc_gr[i] = 254;
                    blc_attr.stBlc1Auto.blc_gb[i] = 254;
                    blc_attr.stBlc1Auto.blc_b[i] = 254;
                }

                ret = rk_aiq_user_api2_ablc_SetAttrib(ctx, &blc_attr);
                printf("set ablc attri auto ret:%d \n\n", ret);


                rk_aiq_blc_attrib_t get_blc_attr;
                get_blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                memset(&get_blc_attr, 0x00, sizeof(get_blc_attr));//important, need init first
                ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &get_blc_attr);
                printf("get ablc attri ret:%d done:%d \n\n", ret, get_blc_attr.sync.done);

                //need sure sync.done is true, the can release the pointer
                usleep(100000);
                sample_blc_attr_relese(&blc_attr);
            }
            break;
        case '6':
            if (CHECK_ISP_HW_V30()) {
                rk_aiq_blc_attrib_t blc_attr;
                memset(&blc_attr, 0x00, sizeof(blc_attr));//important, need init first
                blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &blc_attr);
                blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                blc_attr.eMode = ABLC_OP_MODE_MANUAL;
                blc_attr.stBlc0Manual.enable = 1;
                blc_attr.stBlc0Manual.blc_r = 253;
                blc_attr.stBlc0Manual.blc_gr = 253;
                blc_attr.stBlc0Manual.blc_gb = 253;
                blc_attr.stBlc0Manual.blc_b = 253;

                blc_attr.stBlc1Manual.enable = 1;
                blc_attr.stBlc1Manual.blc_r = 253;
                blc_attr.stBlc1Manual.blc_gr = 253;
                blc_attr.stBlc1Manual.blc_gb = 253;
                blc_attr.stBlc1Manual.blc_b = 253;


                ret = rk_aiq_user_api2_ablc_SetAttrib(ctx, &blc_attr);
                printf("set blc attri manual ret:%d \n\n", ret);


                rk_aiq_blc_attrib_t get_blc_attr;
                get_blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                memset(&get_blc_attr, 0x00, sizeof(get_blc_attr));//important, need init first
                ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &get_blc_attr);
                printf("get ablc attri ret:%d done:%d \n\n", ret, get_blc_attr.sync.done);

            }
            break;
        case '7':
            if (CHECK_ISP_HW_V30()) {
                default_blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                ret = rk_aiq_user_api2_ablc_SetAttrib(ctx, &default_blc_attr);
                printf("set blc attri auto default value ret:%d \n\n", ret);


                rk_aiq_blc_attrib_t get_blc_attr;
                get_blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
                memset(&get_blc_attr, 0x00, sizeof(get_blc_attr));//important, need init first
                ret = rk_aiq_user_api2_ablc_GetAttrib(ctx, &get_blc_attr);
                printf("get ablc attri ret:%d done:%d \n\n", ret, get_blc_attr.sync.done);

            }
            break;
        default:
            printf("not support test\n\n");
            break;
        }

    } while (key != 'q' && key != 'Q');


    sample_blc_attr_relese(&default_blc_attr);

    return ret;
}

