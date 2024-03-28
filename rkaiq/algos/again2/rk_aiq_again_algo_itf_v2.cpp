/*
 * rk_aiq_algo_anr_itf.c
 *
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

#include <fcntl.h> /* low-level i/o */
#include "again2/rk_aiq_again_algo_itf_v2.h"
#include "again2/rk_aiq_again_algo_v2.h"
#include "rk_aiq_algo_types.h"

RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    Again_Context_V2_t pAgainCtx;
} RkAiqAlgoContext;

/*******************************************************************************
 * useage for write local gain
 *
 * echo mode > /tmp/write_gain2ddr_flg
 *
 * mode:0/3 local gain mode 0/3 4*8 downscale
 * mode:1 local gain mode 1 2*8 downscale
 * mode:2 local gain mode 2 1*8 downscale
 *
 * file will be stored in /tmp/localgain_HxW_modeXX_frameXX.yuv
 ******************************************************************************/
char name_localgain_flag[] = "/tmp/write_gain2ddr_flg";

void set_gain2ddr_flg_func() {

    char *name          = name_localgain_flag;

    if (access(name, F_OK) == 0) {
        printf("%s remove /tmp/write_gain2ddr_flg name %s\n", __func__, name);
        remove(name);
    }
}

bool get_gain2ddr_flg_func(int* mode) {

    int write_localgain_flg = 0;
    int fp;
    char *name          = name_localgain_flag;
    char buffer[16]     = {0};
    const char *delim   = " ";

    if (access(name, F_OK) == 0) {
        write_localgain_flg = 1;
        fp = open(name, O_RDONLY | O_SYNC);

         if (read(fp, buffer, sizeof(buffer)) <= 0) {
                printf("%s read %s fail! empty\n", __func__, name);
                write_localgain_flg = 0;
                remove(name);
            } else {
                char *p = NULL;
                p = strtok(buffer, delim);
                if (p != NULL) {
                    int value = atoi(p);
                    if(value < 0 || value > 3) {
                        printf("%s not supported mode %d!\n", __func__, value);
                        remove(name);
                    } else {
                        printf("%s read success value %d\n", __func__, value);
                        *mode = value;
                    }
                }
            }
    } else {
        write_localgain_flg                 = 0;
    }

    return write_localgain_flg;
}

static XCamReturn release_dbg_buf(Again_Context_V2_t* again_contex)
{
    if (again_contex->wrt2ddr.mem_ctx)
        again_contex->wrt2ddr.mem_ops->release_mem(0, again_contex->wrt2ddr.mem_ctx);
    again_contex->wrt2ddr.mem_ctx = nullptr;
    for(int i = 0; i < again_contex->wrt2ddr.buf_cnt; i++){
        again_contex->wrt2ddr.store_addr[i] = nullptr;
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn restore_dbg_buf(Again_Context_V2_t* again_contex)
{
    if(again_contex->wrt2ddr.store_addr[0] != nullptr){
        LOGE_ANR("restore_dbg_buf return");
        return XCAM_RETURN_NO_ERROR;
    }
    for(int i = 0; i < again_contex->wrt2ddr.buf_cnt; i++){
        again_contex->wrt2ddr.mem_info = (rk_aiq_dbg_share_mem_info_t *)
                again_contex->wrt2ddr.mem_ops->get_free_item(0, again_contex->wrt2ddr.mem_ctx);
        if (again_contex->wrt2ddr.mem_info == NULL) {
            LOGE_ANR( "%s(%d): no free dbg buf", __FUNCTION__, __LINE__);
            return XCAM_RETURN_ERROR_MEM;
        } else {
            again_contex->wrt2ddr.store_addr[i] = again_contex->wrt2ddr.mem_info->map_addr;
            again_contex->wrt2ddr.store_fd[i] = again_contex->wrt2ddr.mem_info->fd;
            again_contex->wrt2ddr.size[i] = again_contex->wrt2ddr.mem_info->size;
            LOGD_ANR("get stored fd: %d(%d)", again_contex->wrt2ddr.store_fd[i], i);
        }
    }
    for(int i = 0; i < again_contex->wrt2ddr.buf_cnt; i++){
        if (again_contex->wrt2ddr.store_addr[i] != NULL) {
            unsigned int* tmp1 = (unsigned int* )again_contex->wrt2ddr.store_addr[i];
            tmp1[0] = RKISP_INFO2DDR_BUF_INIT;
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn alloc_dbg_buf(Again_Context_V2_t* again_contex)
{
    if(again_contex->wrt2ddr.mem_ctx){
        LOGD_ANR("no need to alloc");
        return XCAM_RETURN_NO_ERROR;//no need to alloc
    }
    rk_aiq_share_mem_config_t dbg_mem_config;
    again_contex->wrt2ddr.buf_cnt = 1;
    dbg_mem_config.alloc_param.width = 0;
    dbg_mem_config.alloc_param.height = 0;
    /* 0 or 3: 4x8mode, 1: 2x8 mode, 2: 1x8mode */
    if (again_contex->wrt2ddr.again2ddr_mode < RK_AIQ_AGAIN_DS_4X8 || again_contex->wrt2ddr.again2ddr_mode > RK_AIQ_AGAIN_DS_MAX) {
        LOGE_ANR("not supported again2ddr_mode %d \n", again_contex->wrt2ddr.again2ddr_mode);
        return XCAM_RETURN_ERROR_PARAM;
    }
    dbg_mem_config.mem_type = MEM_TYPE_DBG_INFO;
    dbg_mem_config.alloc_param.reserved[0] = RKISP_INFO2DRR_OWNER_GAIN;
    dbg_mem_config.alloc_param.reserved[1] = again_contex->wrt2ddr.again2ddr_mode;
    dbg_mem_config.alloc_param.reserved[2] = again_contex->wrt2ddr.buf_cnt;
    again_contex->wrt2ddr.mem_ops->alloc_mem(0, again_contex->wrt2ddr.mem_ops,
                                      &dbg_mem_config,
                                      &again_contex->wrt2ddr.mem_ctx);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn alloc_dbg_buf_mode(Again_Context_V2_t* again_contex, int mode)
{
    if(again_contex->wrt2ddr.mem_ctx){
        LOGD_ANR("no need to alloc");
        return XCAM_RETURN_NO_ERROR;//no need to alloc
    }
    rk_aiq_share_mem_config_t dbg_mem_config;
    again_contex->wrt2ddr.buf_cnt = 1;
    dbg_mem_config.alloc_param.width = 0;
    dbg_mem_config.alloc_param.height = 0;
    /* 0 or 3: 4x8mode, 1: 2x8 mode, 2: 1x8mode */
    if (mode < RK_AIQ_AGAIN_DS_4X8 || mode > RK_AIQ_AGAIN_DS_MAX) {
        LOGE_ANR("not supported again2ddr_mode %d \n", mode);
        return XCAM_RETURN_ERROR_PARAM;
    }
    dbg_mem_config.mem_type = MEM_TYPE_DBG_INFO;
    dbg_mem_config.alloc_param.reserved[0] = RKISP_INFO2DRR_OWNER_GAIN;
    dbg_mem_config.alloc_param.reserved[1] = mode;
    dbg_mem_config.alloc_param.reserved[2] = again_contex->wrt2ddr.buf_cnt;
    again_contex->wrt2ddr.mem_ops->alloc_mem(0, again_contex->wrt2ddr.mem_ops,
                                      &dbg_mem_config,
                                      &again_contex->wrt2ddr.mem_ctx);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn write_dbg_buf_and_release(Again_Context_V2_t* again_contex, int fd, int fd2)
{
    int ds_width = 0, ds_hight = 0;

    for(int i=0; i < again_contex->wrt2ddr.buf_cnt; i++){
        unsigned int* tmp2 = (unsigned int* )again_contex->wrt2ddr.store_addr[i];
        if (tmp2 != NULL) {
            if(again_contex->wrt2ddr.store_fd[i] == fd) {
                int totalpixel;
                if (again_contex->wrt2ddr.again2ddr_mode == RK_AIQ_AGAIN_DS_4X8 || again_contex->wrt2ddr.again2ddr_mode == RK_AIQ_AGAIN_DS_MAX) {
                    ds_width = (again_contex->rawWidth + 7) / 8;
                    ds_hight = again_contex->rawHeight / 4;
                    totalpixel = ds_width * ds_hight;
                } else if (again_contex->wrt2ddr.again2ddr_mode == RK_AIQ_AGAIN_DS_2X8) {
                    ds_width = (again_contex->rawWidth + 7) / 8;
                    ds_hight = again_contex->rawHeight / 2;
                    totalpixel = ds_width * ds_hight;
                } else if (again_contex->wrt2ddr.again2ddr_mode == RK_AIQ_AGAIN_DS_1X8){
                    ds_width = (again_contex->rawWidth + 7) / 8;
                    ds_hight = again_contex->rawHeight;
                    totalpixel = ds_width * ds_hight;
                } else {
                    LOGE_ANR( "not supported mode:%d\n", again_contex->wrt2ddr.again2ddr_mode);
                    release_dbg_buf(again_contex);
                    return XCAM_RETURN_ERROR_FAILED;
                }
                char fname[200];
                sprintf(fname, "/tmp/localgain_%dx%d_mode%d_frame%d.yuv", //again_contex->wrt2ddr.path_name,
                        ds_width, ds_hight, again_contex->wrt2ddr.again2ddr_mode, fd2);

                FILE *fp = fopen(fname, "wb");
                if(fp) {
                    fwrite(tmp2, again_contex->wrt2ddr.size[i], 1, fp);
                    fclose(fp);
                    LOGE_ANR( "suceed to write %s", fname);
                    again_contex->wrt2ddr.need2wrt = false;
                    release_dbg_buf(again_contex);
                }
                break;
            }
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn write_dbg_buf_and_release_mode(Again_Context_V2_t* again_contex,
                                                 int fd, int fd2, int* mode)
{
    int ds_width = 0, ds_hight = 0;

    for(int i=0; i < again_contex->wrt2ddr.buf_cnt; i++){
        unsigned int* tmp2 = (unsigned int* )again_contex->wrt2ddr.store_addr[i];
        if (tmp2 != NULL) {
            if(again_contex->wrt2ddr.store_fd[i] == fd) {
                int totalpixel;
                if (*mode == RK_AIQ_AGAIN_DS_4X8 || *mode == RK_AIQ_AGAIN_DS_MAX) {
                    ds_width = (again_contex->rawWidth + 7) / 8;
                    ds_hight = again_contex->rawHeight / 4;
                    totalpixel = ds_width * ds_hight;
                } else if (*mode == RK_AIQ_AGAIN_DS_2X8) {
                    ds_width = (again_contex->rawWidth + 7) / 8;
                    ds_hight = again_contex->rawHeight / 2;
                    totalpixel = ds_width * ds_hight;
                } else if (*mode == RK_AIQ_AGAIN_DS_1X8){
                    ds_width = (again_contex->rawWidth + 7) / 8;
                    ds_hight = again_contex->rawHeight;
                    totalpixel = ds_width * ds_hight;
                } else {
                    LOGE_ANR( "not supported mode:%d\n", mode);
                    release_dbg_buf(again_contex);
                    return XCAM_RETURN_ERROR_FAILED;
                }
                char fname[200];
                sprintf(fname, "/tmp/localgain_%dx%d_mode%d_frame%d.yuv", //again_contex->wrt2ddr.path_name,
                        ds_width, ds_hight, *mode, fd2);

                FILE *fp = fopen(fname, "wb");
                if(fp) {
                    fwrite(tmp2, again_contex->wrt2ddr.size[i], 1, fp);
                    fclose(fp);
                    LOGE_ANR( "suceed to write %s", fname);
                    again_contex->wrt2ddr.need2wrt = false;
                    release_dbg_buf(again_contex);
                }
                *mode = -1;
                break;
            }
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn init_dbg_buf(Again_Context_V2_t* again_contex,int fd)
{
    for(int i=0; i < again_contex->wrt2ddr.buf_cnt; i++){
        unsigned int* tmp1 = (unsigned int* )again_contex->wrt2ddr.store_addr[i];
        if (tmp1 != NULL) {
            if(again_contex->wrt2ddr.store_fd[i] == fd){
                unsigned int* tmp1 = (unsigned int* )again_contex->wrt2ddr.store_addr[i];
                tmp1[0] = RKISP_INFO2DDR_BUF_INIT;
                break;
            }
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{

    XCamReturn result = XCAM_RETURN_NO_ERROR;
    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    Again_Context_V2_t* pAgainCtx = NULL;

#if(AGAIN_USE_JSON_FILE_V2)
    Again_result_V2_t ret = Again_Init_V2(&pAgainCtx, cfg->calibv2);
#endif

    if(ret != AGAINV2_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
    } else {
        *context = (RkAiqAlgoContext *)(pAgainCtx);
    }
#endif

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    Again_Context_V2_t* pAgainCtx = (Again_Context_V2_t*)context;
    Again_result_V2_t ret = Again_Release_V2(pAgainCtx);
    if(ret != AGAINV2_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: release ANR failed (%d)\n", __FUNCTION__, ret);
    }
#endif

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

    Again_Context_V2_t* pAgainCtx = (Again_Context_V2_t *)params->ctx;
    RkAiqAlgoConfigAgainV2* pCfgParam = (RkAiqAlgoConfigAgainV2*)params;
    pAgainCtx->prepare_type = params->u.prepare.conf_type;

    pAgainCtx->wrt2ddr.mem_ops = pCfgParam->mem_ops_ptr;
    pAgainCtx->rawHeight = params->u.prepare.sns_op_height;
    pAgainCtx->rawWidth = params->u.prepare.sns_op_width;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
#if AGAIN_USE_JSON_FILE_V2
#if 1
        void *pCalibDbV2 = (void*)(pCfgParam->com.u.prepare.calibv2);
        CalibDbV2_GainV2_t * pcalibdbV2_gain_v2 =
            (CalibDbV2_GainV2_t *)(CALIBDBV2_GET_MODULE_PTR((CamCalibDbV2Context_t*)pCalibDbV2, gain_v2));

        pAgainCtx->gain_v2 = *pcalibdbV2_gain_v2;
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR)
            return XCAM_RETURN_NO_ERROR;
        pAgainCtx->isIQParaUpdate = true;
        pAgainCtx->isReCalculate |= 1;
        LOGE_ANR("enter!!\n");
#endif
#endif
    }

    Again_result_V2_t ret = Again_Prepare_V2(pAgainCtx, &pCfgParam->stAgainConfig);
    if(ret != AGAINV2_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: config ANR failed (%d)\n", __FUNCTION__, ret);
    }

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return result;
}
#if 0
static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );
    Again_Context_V2_t* pAgainCtx = (Again_Context_V2_t *)inparams->ctx;

    RkAiqAlgoPreAgainV2* pAnrPreParams = (RkAiqAlgoPreAgainV2*)inparams;

    if (pAnrPreParams->com.u.proc.gray_mode) {
        pAgainCtx->isGrayMode = true;
    } else {
        pAgainCtx->isGrayMode = false;
    }

    Again_result_V2_t ret = Again_PreProcess_V2(pAgainCtx);
    if(ret != AGAINV2_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: ANRPreProcess failed (%d)\n", __FUNCTION__, ret);
    }

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return result;
}
#endif
static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    static int get_gain2ddr_mode = -1;

    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    int deltaIso = 0;
    RkAiqAlgoProcAgainV2* pAgainProcParams = (RkAiqAlgoProcAgainV2*)inparams;
    RkAiqAlgoProcResAgainV2* pAgainProcResParams = (RkAiqAlgoProcResAgainV2*)outparams;
    Again_Context_V2_t* pAgainCtx = (Again_Context_V2_t *)inparams->ctx;
    Again_ExpInfo_V2_t stExpInfo;
    memset(&stExpInfo, 0x00, sizeof(Again_ExpInfo_V2_t));

    LOGD_ANR("%s:%d init:%d hdr mode:%d  \n",
             __FUNCTION__, __LINE__,
             inparams->u.proc.init,
             pAgainProcParams->hdr_mode);

    if (inparams->u.proc.gray_mode) {
        pAgainCtx->isGrayMode = true;
    } else {
        pAgainCtx->isGrayMode = false;
    }

    Again_result_V2_t ret = Again_PreProcess_V2(pAgainCtx);
    if(ret != AGAINV2_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: ANRPreProcess failed (%d)\n", __FUNCTION__, ret);
    }

    stExpInfo.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    for(int i = 0; i < 3; i++) {
        stExpInfo.arIso[i] = 50;
        stExpInfo.arAGain[i] = 1.0;
        stExpInfo.arDGain[i] = 1.0;
        stExpInfo.arTime[i] = 0.01;
    }

    if(pAgainProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        stExpInfo.hdr_mode = 0;
    } else if(pAgainProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
              || pAgainProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR ) {
        stExpInfo.hdr_mode = 1;
    } else if(pAgainProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
              || pAgainProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR ) {
        stExpInfo.hdr_mode = 2;
    }
    stExpInfo.snr_mode = 0;

    RKAiqAecExpInfo_t *curExp = pAgainProcParams->com.u.proc.curExp;

    if(curExp != NULL) {
        stExpInfo.snr_mode = curExp->CISFeature.SNR;
        if(pAgainProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfo.hdr_mode = 0;
            if(curExp->LinearExp.exp_real_params.analog_gain < 1.0) {
                stExpInfo.arAGain[0] = 1.0;
                LOGW_ANR("leanr mode again is wrong, use 1.0 instead\n");
            } else {
                stExpInfo.arAGain[0] = curExp->LinearExp.exp_real_params.analog_gain;
            }
            if(curExp->LinearExp.exp_real_params.digital_gain < 1.0) {
                stExpInfo.arDGain[0] = 1.0;
                LOGW_ANR("leanr mode dgain is wrong, use 1.0 instead\n");
            } else {
                stExpInfo.arDGain[0] = curExp->LinearExp.exp_real_params.digital_gain;
            }
            if(curExp->LinearExp.exp_real_params.isp_dgain < 1.0) {
                stExpInfo.isp_dgain[0] = 1.0;
                LOGW_ANR("leanr mode isp_dgain is wrong, use 1.0 instead\n");
            } else {
                stExpInfo.isp_dgain[0] = curExp->LinearExp.exp_real_params.isp_dgain;
            }
            // stExpInfo.arAGain[0] = 64.0;
            stExpInfo.arTime[0] = curExp->LinearExp.exp_real_params.integration_time;
            stExpInfo.arIso[0] = stExpInfo.arAGain[0] * stExpInfo.arDGain[0] * 50 * stExpInfo.isp_dgain[0];
            LOGD_ANR("anr: %s-%d curExp(%f, %f, %f, %f %d, %d)\n",
                     __FUNCTION__, __LINE__,
                     curExp->LinearExp.exp_real_params.analog_gain,
                     curExp->LinearExp.exp_real_params.integration_time,
                     curExp->LinearExp.exp_real_params.digital_gain,
                     curExp->LinearExp.exp_real_params.isp_dgain,
                     curExp->LinearExp.exp_real_params.dcg_mode,
                     curExp->CISFeature.SNR);
        } else {
            for(int i = 0; i < 3; i++) {
                if(curExp->HdrExp[i].exp_real_params.analog_gain < 1.0) {
                    stExpInfo.arAGain[i] = 1.0;
                    LOGW_ANR("hdr mode again is wrong, use 1.0 instead\n");
                } else {
                    stExpInfo.arAGain[i] = curExp->HdrExp[i].exp_real_params.analog_gain;
                }
                if(curExp->HdrExp[i].exp_real_params.digital_gain < 1.0) {
                    stExpInfo.arDGain[i] = 1.0;
                    LOGW_ANR("hdr mode dgain is wrong, use 1.0 instead\n");
                } else {
                    stExpInfo.arDGain[i] = curExp->HdrExp[i].exp_real_params.digital_gain;
                }
                if(curExp->HdrExp[i].exp_real_params.isp_dgain < 1.0) {
                    stExpInfo.isp_dgain[i] = 1.0;
                    LOGW_ANR("hdr mode isp_dgain is wrong, use 1.0 instead\n");
                } else {
                    stExpInfo.isp_dgain[i] = curExp->HdrExp[i].exp_real_params.isp_dgain;
                }
                stExpInfo.arTime[i] = curExp->HdrExp[i].exp_real_params.integration_time;
                stExpInfo.arIso[i] = stExpInfo.arAGain[i] * stExpInfo.arDGain[i] * 50 * stExpInfo.isp_dgain[i];

                LOGD_ANR("%s:%d index:%d again:%f dgain:%f isp_dgain:%f time:%f  iso:%d  hdr_mode:%d  \n",
                         __FUNCTION__, __LINE__,
                         i,
                         stExpInfo.arAGain[i],
                         stExpInfo.arDGain[i],
                         stExpInfo.isp_dgain[i],
                         stExpInfo.arTime[i],
                         stExpInfo.arIso[i],
                         stExpInfo.hdr_mode);
            }
        }
    } else {
        LOGE_ANR("%s:%d  curExp(%p) is NULL, so use default instead \n",
                 __FUNCTION__, __LINE__,  curExp);
    }

    deltaIso = abs(stExpInfo.arIso[stExpInfo.hdr_mode] - pAgainCtx->stExpInfo.arIso[stExpInfo.hdr_mode]);
    if(deltaIso > AGAINV2_RECALCULATE_DELTA_ISO) {
        pAgainCtx->isReCalculate |= 1;
    }

    if(pAgainCtx->isReCalculate) {
        Again_result_V2_t ret = Again_Process_V2(pAgainCtx, &stExpInfo);
        if(ret != AGAINV2_RET_SUCCESS) {
            result = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
        }

        outparams->cfg_update = true;
    } else {
        outparams->cfg_update = false;
    }
    Again_GetProcResult_V2(pAgainCtx, &pAgainProcResParams->stAgainProcResult);
    pAgainCtx->isReCalculate = 0;
#endif

#if defined(ISP_HW_V32) && RK_GAIN_V2_ENABLE_GAIN2DDR

    if (get_gain2ddr_flg_func(&get_gain2ddr_mode)) {
        alloc_dbg_buf_mode(pAgainCtx, get_gain2ddr_mode);
        restore_dbg_buf(pAgainCtx);
        set_gain2ddr_flg_func();
    }
    if (get_gain2ddr_mode != -1) {
        write_dbg_buf_and_release_mode(pAgainCtx, pAgainProcParams->stats.dbginfo_fd, inparams->frame_id, &get_gain2ddr_mode);
        init_dbg_buf(pAgainCtx, pAgainProcParams->stats.dbginfo_fd);
    }

    if(pAgainCtx->wrt2ddr.need2wrt) {
        LOGE_ANR(">>> start alloc frame %d", inparams->frame_id);
        pAgainCtx->wrt2ddr.need2wrt = false;
        alloc_dbg_buf(pAgainCtx);
        restore_dbg_buf(pAgainCtx);
    }

    if (pAgainProcParams->stats.stats_true && pAgainProcParams->stats.dbginfo_fd != -1) {
        LOGE_ANR(">>> start write fd %d ", pAgainProcParams->stats.dbginfo_fd);
        write_dbg_buf_and_release(pAgainCtx, pAgainProcParams->stats.dbginfo_fd, inparams->frame_id);
        init_dbg_buf(pAgainCtx, pAgainProcParams->stats.dbginfo_fd);
    }

#endif // defined(ISP_HW_V32) && RK_GAIN_V2_ENABLE_GAIN2DDR

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

    //nothing todo now

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAgainV2 = {
    .common = {
        .version = RKISP_ALGO_AGAIN_VERSION_V2,
        .vendor  = RKISP_ALGO_AGAIN_VENDOR_V2,
        .description = RKISP_ALGO_AGAIN_DESCRIPTION_V2,
        .type    = RK_AIQ_ALGO_TYPE_AGAIN,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
