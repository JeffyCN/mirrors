/*
 *  Copyright (c) 2024 Rockchip Corporation
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/dma-buf.h>
#include <fcntl.h>
#include <dlfcn.h>
#include "RkAibnrManager.h"
#include "RkAiqManager_c.h"

#if RKAIQ_HAVE_AIBNR

#define CEIL_DOWN(x, y)                              (((x) + ((y) - 1)) / (y))
#define FLOOR_BY(v, r)                               (((v) / (r)) * (r))
#define CEIL_BY(v, r)                                FLOOR_BY(((v) + (r) - 1), (r))


static const char* PostBnrLibraryName = "librkpostbnr.so";

static bool PostBnrLibraryInit(struct PostBnrLibrary* obj) {
    obj->handle_ = dlopen(PostBnrLibraryName, RTLD_LAZY);
    char* error  = dlerror();

    if (obj->handle_ == NULL) {
        LOGI_AIBNR("Failed to dlopen library : %s, error: %s", PostBnrLibraryName, error);
        return false;
    }

    LOGK_AIBNR("dlopen library: %s success", PostBnrLibraryName);

    return true;
}

static void PostBnrLibraryDeinit(struct PostBnrLibrary* obj) {
    if (obj->handle_ != NULL) {
        dlclose(obj->handle_);
    }
}

static bool PostBnrLibraryLoadSymbols(struct PostBnrLibrary* obj) {
    char* error;

    obj->ops_.bnr_init = (rk_post_bnr_init)dlsym(obj->handle_, "rk_post_bnr_init");
    error               = dlerror();
    if (error != NULL) {
        LOGE_AIBNR("Failed to resolve symbol rk_post_bnr_init error: %s", error);
        goto error_out;
    }
    obj->ops_.bnr_proc = (rk_post_bnr_proc)dlsym(obj->handle_, "rk_post_bnr_proc");
    error               = dlerror();
    if (error != NULL) {
        LOGE_AIBNR("Failed to resolve symbol rk_post_bnr_proc error: %s", error);
        goto error_out;
    }
    obj->ops_.bnr_deinit = (rk_post_bnr_deinit)dlsym(obj->handle_, "rk_post_bnr_deinit");
    error                 = dlerror();
    if (error != NULL) {
        LOGE_AIBNR("Failed to resolve symbol rk_post_bnr_deinit error: %s", error);
        goto error_out;
    }

    return true;
error_out:
    dlclose(obj->handle_);
    obj->handle_ = NULL;
    return false;
}

static void calcIspBufSize(struct rkaiisp_ispbuf_info *ispbuf_info, int raw_wid, int raw_hgt)
{
    int gain_wid = raw_wid / 8;
    int gain_hgt = raw_hgt / 2;

    ispbuf_info->iir_width  = raw_wid;
    ispbuf_info->iir_height = raw_hgt;
    ispbuf_info->narmap_width  = CEIL_BY(raw_wid / 4, 16);
    ispbuf_info->narmap_height = gain_hgt;

    for (int i = 0; i < 4; i++) {
        if (i > 0) {
            ispbuf_info->raw_width[i - 1]  = raw_wid;
            ispbuf_info->raw_height[i - 1] = raw_hgt;
        }
        ispbuf_info->sig_width[i]  = raw_wid / 2;
        ispbuf_info->sig_height[i] = raw_hgt / 2;

        raw_hgt = CEIL_BY(CEIL_DOWN(raw_hgt, 2), 2);
        raw_wid = CEIL_BY(CEIL_DOWN(raw_wid, 2), 2);
    }

}

static int getKwtMaxSize(char *model_buf, uint32_t *kwt_size)
{
    struct rkaiisp_model_info *model_info = (struct rkaiisp_model_info *)model_buf;
    int ret = -1;

    *kwt_size = 0;
    if (model_info) {
        struct rkaiisp_kwt_cfg *kwt_cfg;
        uint32_t runcnt;

        runcnt = model_info->model_runcnt;
        kwt_cfg = &model_info->kwt_cfg;
        *kwt_size = kwt_cfg->kwt_offet[runcnt - 1] + kwt_cfg->kwt_pad_size[runcnt - 1];
        ret = 0;
    }

    return ret;
}

static int getParamInfo(AibnrManager_t* pAibnrManager, AibnrModelBuf_t *pAibnrModelBuf, struct rkaiisp_param_info *param_info)
{
    uint32_t kwt_size, max_size;

    max_size = 0;
    for (int i = 0; i < AIBNR_ISO_STEP_MAX; i++) {
        if (getKwtMaxSize(pAibnrModelBuf->model_buf[i], &kwt_size) != 0) {
            LOGE_AIBNR("%s: get param info(%s) failed!", __func__, pAibnrModelBuf->model_file[i]);
        }

        if (kwt_size > max_size)
            max_size = kwt_size;
    }

    param_info->exealgo    = AIBNR;
    param_info->exemode    = AIBNR_AIISP_MODE;
    param_info->max_runcnt = pAibnrManager->model_max_runcnt;
    param_info->para_size  = max_size + sizeof(struct rkaiisp_params);
    return 0;
}

static void aiisp_params_print(struct rkaiisp_params *aiisp_cfg)
{
    struct rkaiisp_other_cfg *other_cfg = &aiisp_cfg->other_cfg;
    struct rkaiisp_kwt_cfg *kwt_cfg = &aiisp_cfg->kwt_cfg;
    struct rkaiisp_model_cfg *model_cfg;
    u32 i, j;

    LOGD_AIBNR("aiisp: frame_id %d, module_update = 0x%x", aiisp_cfg->frame_id, aiisp_cfg->module_update);
    if (aiisp_cfg->module_update & RKAIISP_OTHER_UPDATE) {
        LOGD_AIBNR("sw_neg_noiselimit = 0x%x", other_cfg->sw_neg_noiselimit);
        LOGD_AIBNR("sw_pos_noiselimit = 0x%x", other_cfg->sw_pos_noiselimit);
        LOGD_AIBNR("sw_prev_blacklvl = 0x%x", other_cfg->sw_prev_blacklvl);
        LOGD_AIBNR("sw_post_blacklvl = 0x%x", other_cfg->sw_post_blacklvl);

        for (i = 0; i < 33; i++) {
            LOGD_AIBNR("sw_in_comp_y[%d] = 0x%x", i, other_cfg->sw_in_comp_y[i]);
        }
        for (i = 0; i < 33; i++) {
            LOGD_AIBNR("sw_out_decomp_y[%d] = 0x%x", i, other_cfg->sw_out_decomp_y[i]);
        }
    }

    if (aiisp_cfg->module_update & RKAIISP_MODEL_UPDATE) {
        for (i = 0; i < aiisp_cfg->model_runcnt; i++) {
            model_cfg = (struct rkaiisp_model_cfg *)&aiisp_cfg->model_cfg[i];

            LOGD_AIBNR("model_cfg %d:", i);
            for (j = 0; j < 7; j++) {
                LOGD_AIBNR("mi_chn_src[%d] = 0x%x", j, model_cfg->mi_chn_src[j]);
            }
            LOGD_AIBNR("sw_aiisp_mode = %d", model_cfg->sw_aiisp_mode);
            LOGD_AIBNR("sw_aiisp_level_num = %d", model_cfg->sw_aiisp_level_num);
            LOGD_AIBNR("sw_aiisp_l1_level_num = %d", model_cfg->sw_aiisp_l1_level_num);
            LOGD_AIBNR("sw_aiisp_op_mode = %d", model_cfg->sw_aiisp_op_mode);
            LOGD_AIBNR("sw_aiisp_drop_en = %d", model_cfg->sw_aiisp_drop_en);
            for (j = 0; j < 16; j++) {
                LOGD_AIBNR("sw_aiisp_lv_active[%d] = 0x%x", j, model_cfg->sw_aiisp_lv_active[j]);
            }
            for (j = 0; j < 16; j++) {
                LOGD_AIBNR("sw_aiisp_lv_mode[%d] = 0x%x", j, model_cfg->sw_aiisp_lv_mode[j]);
            }
            for (j = 0; j < 7; j++) {
                LOGD_AIBNR("sw_mi_chn_en[%d] = 0x%x", j, model_cfg->sw_mi_chn_en[j]);
            }
            for (j = 0; j < 7; j++) {
                LOGD_AIBNR("sw_mi_chn_mode[%d] = 0x%x", j, model_cfg->sw_mi_chn_mode[j]);
            }
            for (j = 0; j < 7; j++) {
                LOGD_AIBNR("sw_mi_chn_num[%d] = 0x%x", j, model_cfg->sw_mi_chn_num[j]);
            }
            for (j = 0; j < 7; j++) {
                LOGD_AIBNR("sw_mi_chn_data_mode[%d] = 0x%x", j, model_cfg->sw_mi_chn_data_mode[j]);
            }
            LOGD_AIBNR("sw_mi_chn1_sel = %d", model_cfg->sw_mi_chn1_sel);
            LOGD_AIBNR("sw_mi_chn3_sel = %d", model_cfg->sw_mi_chn3_sel);
            LOGD_AIBNR("sw_out_d2s_en = %d", model_cfg->sw_out_d2s_en);
            LOGD_AIBNR("sw_out_mode = %d", model_cfg->sw_out_mode);
            LOGD_AIBNR("sw_lastlvlm1_clip8bit = %d", model_cfg->sw_lastlvlm1_clip8bit);
        }

        for (i = 0; i < RKAIISP_MAX_RUNCNT; i++) {
            LOGD_AIBNR("kwt_offet[%d] = 0x%x", i, kwt_cfg->kwt_offet[i]);
        }
        for (i = 0; i < RKAIISP_MAX_RUNCNT; i++) {
            LOGD_AIBNR("kwt_size[%d] = 0x%x", i, kwt_cfg->kwt_size[i]);
        }
        for (i = 0; i < RKAIISP_MAX_RUNCNT; i++) {
            LOGD_AIBNR("kwt_pad_size[%d] = 0x%x", i, kwt_cfg->kwt_pad_size[i]);
        }
    }
}

static int checkModelSum(char *model_file, char *model_buf, size_t file_len)
{
    uint32_t* pTmp = (unsigned int*)model_buf;
    uint32_t i, checksum, realsum;

    realsum = 0;
    checksum = *pTmp++;
    for (i = 1; i < file_len / 4; i++) {
        realsum += *pTmp++;
    }

    if (checksum != realsum) {
        LOGE_AIBNR("model(%s) sum check error 0x%x, 0x%x", model_file, checksum, realsum);
        return -1;
    }

    return 0;
}

static int checkModelCfg(struct rkaiisp_model_cfg *model_cfg)
{
    uint32_t i;

    if (model_cfg->sw_aiisp_mode > 1) {
        LOGE_AIBNR("%s: sw_aiisp_mode %d", __func__, model_cfg->sw_aiisp_mode);
        return -1;
    }
    if (model_cfg->sw_aiisp_op_mode > 1) {
        LOGE_AIBNR("%s: sw_aiisp_op_mode %d", __func__, model_cfg->sw_aiisp_op_mode);
        return -1;
    }
    if (model_cfg->sw_out_mode > 2) {
        LOGE_AIBNR("%s: sw_out_mode %d", __func__, model_cfg->sw_out_mode);
        return -1;
    }
    if (model_cfg->sw_aiisp_level_num > 16) {
        LOGE_AIBNR("%s: sw_aiisp_level_num %d", __func__, model_cfg->sw_aiisp_level_num);
        return -1;
    }

    for (i = 0; i < 7; i++) {
        if (model_cfg->sw_mi_chn_mode[i] > 2) {
            LOGE_AIBNR("%s: sw_mi_chn_mode %d", __func__, model_cfg->sw_mi_chn_mode[i]);
            return -1;
        }
        if (model_cfg->sw_mi_chn_data_mode[i] > 2) {
            LOGE_AIBNR("%s: sw_mi_chn_data_mode %d", __func__, model_cfg->sw_mi_chn_data_mode[i]);
            return -1;
        }
    }

    for (i = 0; i < 16; i++) {
        if (model_cfg->sw_aiisp_lv_mode[i] > 4) {
            LOGE_AIBNR("%s: sw_aiisp_lv_mode %d", __func__, model_cfg->sw_aiisp_lv_mode[i]);
            return -1;
        }
    }

    return 0;
}

static XCamReturn readModel(AibnrManager_t* pAibnrManager, CamCalibDbV2Context_t* pCalibDbV2,
                            AibnrModelBuf_t* aibnrModelBuf, RkAiqAibnrModelInfo_t* aibnrModelInfo)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aibnr_api_attrib_t *aibnr_attrib = (aibnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, aibnr));
    aibnr_params_static_t* psta = &(aibnr_attrib->stAuto.sta);
    aiBnr_modelRunMode_t model_runmode = aiBnr_comboX1G8_mode;
    uint32_t model_max_runcnt = 0;

    if (!aibnr_attrib->en)
        return XCAM_RETURN_BYPASS;

    memset(aibnrModelBuf, 0, sizeof(*aibnrModelBuf));
    memset(aibnrModelInfo, 0, sizeof(*aibnrModelInfo));
    for (int i = 0; i < AIBNR_ISO_STEP_MAX; i++) {
        aibnr_param_dyn_t* pdyn = &(aibnr_attrib->stAuto.dyn[i]);
        aibnr_param_tunning_t *tunning = &pdyn->tunning;
        struct rkaiisp_model_info model_info;
        size_t file_size, correct_size, runcnt;
        struct rkaiisp_kwt_cfg *kwt_cfg;
        char model_file[256];
        FILE *fp;

        sprintf(model_file, "%s/%s", psta->model_dir.sw_aiBnrCfg_model_dir, tunning->sw_aiBnrT_model_file);
        fp = fopen(model_file, "rb");
        if (fp) {
            if (fread(&model_info, sizeof(model_info), 1, fp) != 1) {
                LOGE_AIBNR("%s(%d): %s: file head is error\n",
                           __FUNCTION__, __LINE__, model_file);
                return XCAM_RETURN_ERROR_FILE;
            }
            fseek(fp, 0, 2);
            file_size = ftell(fp);

            runcnt = model_info.model_runcnt;
            kwt_cfg = &model_info.kwt_cfg;
            if (runcnt != 3 && runcnt != 4 && runcnt != 8) {
                LOGE_AIBNR("%s(%d): %s: runcnt(%d) is error\n",
                           __FUNCTION__, __LINE__, model_file, runcnt);
                return XCAM_RETURN_ERROR_FILE;
            }

            if (runcnt > model_max_runcnt)
                model_max_runcnt = runcnt;

            correct_size = kwt_cfg->kwt_offet[runcnt - 1] + kwt_cfg->kwt_pad_size[runcnt - 1];
            if (file_size != correct_size) {
                LOGE_AIBNR("%s(%d): %s: file size(%d, %d) is error\n",
                           __FUNCTION__, __LINE__, model_file, file_size, correct_size);
                return XCAM_RETURN_ERROR_FILE;
            }

            if (model_info.model_mode == SINGLEX2_MODE)
                model_runmode = aiBnr_singleX2G8_mode;
            else
                model_runmode = aiBnr_comboX1G8_mode;

            if ((model_runmode == aiBnr_singleX2G8_mode) && psta->model_cfg.sw_aiBnrCfg_modelRun_mode != aiBnr_singleX2G8_mode) {
                LOGE_AIBNR("%s(%d): model_info.model_mode %d, sw_aiBnrCfg_modelRun_mode %d is conflicted",
                           __FUNCTION__, __LINE__, model_info.model_mode, psta->model_cfg.sw_aiBnrCfg_modelRun_mode);
                return XCAM_RETURN_ERROR_FILE;
            }

            for (uint32_t j = 0; j < model_info.model_runcnt; j++) {
                if (checkModelCfg(&model_info.model_cfg[j]) != 0) {
                    LOGE_AIBNR("%s(%d): %s: model info check failure\n",
                               __FUNCTION__, __LINE__, model_file);
                    return XCAM_RETURN_ERROR_FILE;
                }
            }

            aibnrModelBuf->model_buf[i] = aiq_malloc(file_size);
            if (aibnrModelBuf->model_buf[i]) {
                fseek(fp, 0, 0);
                fread(aibnrModelBuf->model_buf[i], 1, file_size, fp);
                fclose(fp);
                strcpy(aibnrModelBuf->model_file[i], model_file);
                if (checkModelSum(aibnrModelBuf->model_file[i], aibnrModelBuf->model_buf[i], file_size) != 0) {
                    aiq_free(aibnrModelBuf->model_buf[i]);
                    aibnrModelBuf->model_buf[i] = NULL;
                }
            }
        } else {
            LOGE_AIBNR("read aiisp model file(%s) failed !", model_file);
        }
    }
    aibnrModelBuf->valid = true;
    for (int i = 0; i < AIBNR_ISO_STEP_MAX; i++) {
        if (!aibnrModelBuf->model_buf[i])
            aibnrModelBuf->valid = false;
    }

    struct rkaiisp_model_info *model_info;
    if (aibnrModelBuf->valid) {
        for (int i = 0; i < AIBNR_ISO_STEP_MAX; i++) {
            model_info = (struct rkaiisp_model_info *)aibnrModelBuf->model_buf[i];

            strcpy(aibnrModelInfo->model_file[i], aibnrModelBuf->model_file[i]);
            aibnrModelInfo->quant_val[i] = model_info->model_qr;
        }
    }

    pAibnrManager->model_max_runcnt = model_max_runcnt;

    return ret;
}

XCamReturn AibnrManager_setCalib(AibnrManager_t* pAibnrManager, CamCalibDbV2Context_t* pCalibDbV2)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AibnrModelBuf_t *aibnrModelBuf = &pAibnrManager->mAibnrModelBuf;
    struct rkaiisp_param_info param_info;
    AibnrModelBuf_t tmpAibnrModelBuf;
    RkAiqAibnrModelInfo_t tmpAibnrModelInfo;

    pAibnrManager->pCalibDbV2 = pCalibDbV2;

    if (!pAibnrManager->is_enable) {
        LOGD_AIBNR("%s: aibnr is disabled.", __func__);
        return XCAM_RETURN_BYPASS;
    }

    readModel(pAibnrManager, pCalibDbV2, &tmpAibnrModelBuf, &tmpAibnrModelInfo);
    if (!tmpAibnrModelBuf.valid) {
        LOGE_AIBNR("%s: readModel is error.", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }
    getParamInfo(pAibnrManager, &tmpAibnrModelBuf, &param_info);

    if (param_info.para_size > pAibnrManager->param_info.para_size) {
        LOGE_AIBNR("para_size(%d) in new calib is too big!", param_info.para_size, pAibnrManager->param_info.para_size);
        return XCAM_RETURN_ERROR_FAILED;
    }

    aiqMutex_lock(&pAibnrManager->model_buf_mutex);
    for (int i = 0; i < AIBNR_ISO_STEP_MAX; i++) {
        if (aibnrModelBuf->model_buf[i] != NULL) {
            aiq_free(aibnrModelBuf->model_buf[i]);
            aibnrModelBuf->model_buf[i] = NULL;
        }
    }
    *aibnrModelBuf = tmpAibnrModelBuf;
    pAibnrManager->mAibnrModelInfo = tmpAibnrModelInfo;
    pAibnrManager->param_info = param_info;
    pAibnrManager->pCamHw->aibnr_setModelInf(pAibnrManager->pCamHw, &pAibnrManager->mAibnrModelInfo);
    aiqMutex_unlock(&pAibnrManager->model_buf_mutex);

    return ret;
}

XCamReturn AibnrManager_prepare(AibnrManager_t* pAibnrManager, AiqCamHwBase_t* pCamHw, CamCalibDbV2Context_t* pCalibDbV2,
                                uint32_t isp_acq_width, uint32_t isp_acq_height)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aibnr_api_attrib_t *aibnr_attrib = (aibnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, aibnr));
    aibnr_params_static_t* psta = &(aibnr_attrib->stAuto.sta);
    struct rkaiisp_ispbuf_info *ispbuf_info = &pAibnrManager->ispbuf_info;
    AibnrModelBuf_t *aibnrModelBuf = &pAibnrManager->mAibnrModelBuf;
    struct rkaiisp_param_info param_info;
    RkAiqAibnrModelInfo_t tmpAibnrModelInfo;
    AibnrModelBuf_t tmpAibnrModelBuf;
    btnr_api_attrib_t *btnr2_attr = (btnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, bayertnr));
    rk_aiq_aibnr_buffer_count_t aibnr_buf_cnt;
    uint16_t feBuf_cnt = 0;
    uint16_t beBuf_cnt = 0;

    ret = AiqCamHw_getAiBnrBufCnt(pCamHw, &aibnr_buf_cnt);
    if (ret == XCAM_RETURN_NO_ERROR) {
        feBuf_cnt = aibnr_buf_cnt.isp_fe;
        beBuf_cnt = aibnr_buf_cnt.isp_be;
        LOGK_AIBNR("aibnr buf cnt from hwi: fe %d, be %d", feBuf_cnt, beBuf_cnt);
    }

    if (!feBuf_cnt) feBuf_cnt = AIBNR_IIR_BUF_CNT;
    if (!beBuf_cnt) beBuf_cnt = AIBNR_AIISP_BUF_CNT;

    pAibnrManager->is_enable       = aibnr_attrib->en;
    pAibnrManager->is_param_update = false;
    pAibnrManager->pCamHw          = pCamHw;
    pAibnrManager->pCalibDbV2      = pCalibDbV2;
    pAibnrManager->isp_acq_width   = isp_acq_width;
    pAibnrManager->isp_acq_height  = isp_acq_height;
    pAibnrManager->is_bypass = aibnr_attrib->bypass;
    pAibnrManager->is_state_error  = false;
    pAibnrManager->aeHandler       = NULL;
    pAibnrManager->is_group        = false;
    pAibnrManager->apiFrmRate_valid = false;
    pAibnrManager->feBuf_cnt        = feBuf_cnt;
    pAibnrManager->beBuf_cnt        = beBuf_cnt;
    if (btnr2_attr && btnr2_attr->stAuto.sta.dLoMd_mode == btnr_dLoMdCurOrPre_mode) {
        pAibnrManager->pBtnr2En = true;
    } else {
        pAibnrManager->pBtnr2En = false;
    }

    pAibnrManager->iqFrmRate.sw_aeT_frmRate_mode = (ae_frmRate_mode_t)psta->frmRate.sw_aibnrT_frmRate_mode;
    pAibnrManager->iqFrmRate.sw_aeT_frmRate_val = psta->frmRate.sw_aibnrT_frmRate_val;
    LOGK_AIBNR("record iqFrmRate: mode %d, fps %f",
               pAibnrManager->iqFrmRate.sw_aeT_frmRate_mode,
               pAibnrManager->iqFrmRate.sw_aeT_frmRate_val);

    if (!pAibnrManager->is_enable) {
        LOGD_AIBNR("%s: aibnr is disabled.", __func__);
        return XCAM_RETURN_BYPASS;
    }

    if (isp_acq_width > AIBNR_INBUF_MAX_WIDTH) {
        LOGE_AIBNR("%s: input width(%d) is too big!", __func__, isp_acq_width);
        pAibnrManager->is_state_error = true;
        return XCAM_RETURN_ERROR_FAILED;
    }

    memset(&pAibnrManager->last_aibnr_params, 0, sizeof(pAibnrManager->last_aibnr_params));
    readModel(pAibnrManager, pCalibDbV2, &tmpAibnrModelBuf, &tmpAibnrModelInfo);
    if (!tmpAibnrModelBuf.valid) {
        LOGE_AIBNR("%s: readModel is error.", __func__);
        pAibnrManager->is_state_error = true;
        return XCAM_RETURN_ERROR_FAILED;
    }
    getParamInfo(pAibnrManager, &tmpAibnrModelBuf, &param_info);

    aiqMutex_lock(&pAibnrManager->model_buf_mutex);
    for (int i = 0; i < AIBNR_ISO_STEP_MAX; i++) {
        if (aibnrModelBuf->model_buf[i] != NULL) {
            aiq_free(aibnrModelBuf->model_buf[i]);
            aibnrModelBuf->model_buf[i] = NULL;
        }
    }
    *aibnrModelBuf = tmpAibnrModelBuf;
    pAibnrManager->mAibnrModelInfo = tmpAibnrModelInfo;
    pAibnrManager->param_info = param_info;
    aiqMutex_unlock(&pAibnrManager->model_buf_mutex);

    pAibnrManager->mAibnrCfg.mode = 0;
    pAibnrManager->mAibnrCfg.wr_linecnt = isp_acq_height;
    pAibnrManager->mAibnrCfg.rd_linecnt = isp_acq_height;
    ret = pAibnrManager->pCamHw->aibnr_init(pAibnrManager->pCamHw, pAibnrManager, &pAibnrManager->mAibnrCfg);
    if (ret) {
        LOGE_AIBNR("%s: can not init aibnr.", __func__);
        pAibnrManager->is_state_error = true;
    }
    pAibnrManager->pCamHw->aibnr_setModelInf(pAibnrManager->pCamHw, &pAibnrManager->mAibnrModelInfo);
    pAibnrManager->pCamHw->aibnr_setParamInf(pAibnrManager->pCamHw, &pAibnrManager->param_info);

    for (int i = 0; i < pAibnrManager->beBuf_cnt; i++) pAibnrManager->aiisp_idx_pool[i] = false;

    LOGD_AIBNR("iir_width %d, iir_height %d", isp_acq_width, isp_acq_height);
    calcIspBufSize(ispbuf_info, isp_acq_width, isp_acq_height);
    pAibnrManager->is_parambuf_prepare = false;

    return ret;
}

static XCamReturn mmap_ispbuf(AibnrManager_t* pAibnrManager, struct rkisp_bnr_buf_info* bnr_buf,
                              ispbuf_addr_t* ispbuf_addr) {
    struct rkisp_buf_info *gain_inf, *narmap_inf, *aiisp_inf;
    char *gain_addr, *narmap_addr, *aiisp_addr;

    for (int i = 0; i < AIBNR_ISP_BUF_MAX; i++) {
        ispbuf_addr[i].iir_addr = NULL;
        ispbuf_addr[i].aipre_addr = NULL;
        ispbuf_addr[i].vpsl_addr = NULL;

        ispbuf_addr[i].gain_addr = NULL;
        ispbuf_addr[i].aiisp_addr = NULL;

        if (i < AIBNR_GAIN_BUF_CNT) {
            gain_inf = &bnr_buf->u.v35.gain;
            gain_addr = (char*)mmap(NULL, gain_inf->buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, gain_inf->buf_fd[i], 0);
            if (MAP_FAILED == gain_addr) {
                LOGE_AIBNR("gain fd %d, size %d mmap failed", gain_inf->buf_fd[i], gain_inf->buf_size);
            } else {
                ispbuf_addr[i].gain_addr = gain_addr;
            }
        }

        if (i < pAibnrManager->feBuf_cnt) {
            narmap_inf = &bnr_buf->u.v35.aipre_gain;
            narmap_addr = (char*)mmap(NULL, narmap_inf->buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, narmap_inf->buf_fd[i], 0);
            if (MAP_FAILED == narmap_addr) {
                LOGE_AIBNR("aipre fd %d, size %d mmap failed", narmap_inf->buf_fd[i], narmap_inf->buf_size);
            } else {
                ispbuf_addr[i].aipre_addr = narmap_addr;
            }
        }

        if (i < pAibnrManager->beBuf_cnt) {
            aiisp_inf = &bnr_buf->u.v35.aiisp;
            aiisp_addr = (char*)mmap(NULL, aiisp_inf->buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, aiisp_inf->buf_fd[i], 0);
            if (MAP_FAILED == aiisp_addr) {
                LOGE_AIBNR("aiisp fd %d, size %d mmap failed", aiisp_inf->buf_fd[i], aiisp_inf->buf_size);
            } else {
                ispbuf_addr[i].aiisp_addr = aiisp_addr;
            }
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn munmap_ispbuf(struct rkisp_bnr_buf_info *bnr_buf, ispbuf_addr_t *ispbuf_addr)
{
    struct rkisp_buf_info *gain_inf, *narmap_inf, *aiisp_inf;

    for (int i = 0; i < AIBNR_ISP_BUF_MAX; i++) {
        if (ispbuf_addr[i].gain_addr != NULL) {
            gain_inf = &bnr_buf->u.v35.gain;
            munmap(ispbuf_addr[i].gain_addr, gain_inf->buf_size);
            ispbuf_addr[i].gain_addr = NULL;
        }
        if (ispbuf_addr[i].aipre_addr != NULL) {
            narmap_inf = &bnr_buf->u.v35.aipre_gain;
            munmap(ispbuf_addr[i].aipre_addr, narmap_inf->buf_size);
            ispbuf_addr[i].aipre_addr = NULL;
        }
        if (ispbuf_addr[i].aiisp_addr != NULL) {
            aiisp_inf = &bnr_buf->u.v35.aiisp;
            munmap(ispbuf_addr[i].aiisp_addr, aiisp_inf->buf_size);
            ispbuf_addr[i].aiisp_addr = NULL;
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn change_frmRate(AibnrManager_t* pAibnrManager)
{
    ae_api_expSwAttr_t expSwAttr;

    if (pAibnrManager->aeHandler) {
        if (pAibnrManager->is_group) {
            AiqAlgoCamGroupAeHandler_getExpSwAttr((AiqAlgoCamGroupAeHandler_t*)pAibnrManager->aeHandler, &expSwAttr);
        } else {
            AiqAlgoHandlerAe_getExpSwAttr((AiqAlgoHandlerAe_t*)pAibnrManager->aeHandler, &expSwAttr);
        }

        if (pAibnrManager->doAiisp_en) {
            expSwAttr.commCtrl.frmRate = pAibnrManager->iqFrmRate;
        } else {
            expSwAttr.commCtrl.frmRate = pAibnrManager->apiFrmRate;
        }

        LOGK_AIBNR("change frmRate to: mode %d, fps %f",
                   expSwAttr.commCtrl.frmRate.sw_aeT_frmRate_mode,
                   expSwAttr.commCtrl.frmRate.sw_aeT_frmRate_val);

        if (pAibnrManager->is_group) {
            AiqAlgoCamGroupAeHandler_setExpSwAttr((AiqAlgoCamGroupAeHandler_t*)pAibnrManager->aeHandler, expSwAttr);
        } else {
            AiqAlgoHandlerAe_setExpSwAttr((AiqAlgoHandlerAe_t*)pAibnrManager->aeHandler, expSwAttr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AibnrManager_updateParams(AibnrManager_t* pAibnrManager, rk_aiq_isp_aibnr_params_t *isp_aibnr_params, uint32_t frame_id)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct rkisp_bnr_buf_info *bnr_buf = &pAibnrManager->ispbuf_info.bnr_buf;
    uint32_t isp_acq_width  = pAibnrManager->isp_acq_width;
    uint32_t isp_acq_height = pAibnrManager->isp_acq_height;
    AiqV4l2Buffer_t* pV4l2Buf = NULL;
    bool first_param = false;
    char *pStrSt, *pStrEd;
    uint32_t val;

    if (!pAibnrManager->is_enable) {
        LOGD_AIBNR("%s: aibnr is disabled.", __func__);
        return XCAM_RETURN_BYPASS;
    }

    if (pAibnrManager->is_state_error) {
        LOGE_AIBNR("%s: aibnr in error state.", __func__);
        return XCAM_RETURN_BYPASS;
    }

    if ((pAibnrManager->is_bypass == isp_aibnr_params->is_bypass) && isp_aibnr_params->is_bypass && pAibnrManager->is_parambuf_prepare) {
        LOGD_AIBNR("%s: aibnr is bypass.", __func__);
        return XCAM_RETURN_BYPASS;
    }
    pAibnrManager->is_bypass = isp_aibnr_params->is_bypass;

    if (!pAibnrManager->is_parambuf_prepare) {
        ret = pAibnrManager->pCamHw->aibnr_init(pAibnrManager->pCamHw, pAibnrManager, &pAibnrManager->mAibnrCfg);
        if (ret) {
            pAibnrManager->is_state_error = true;
            LOGE_AIBNR("%s: can not init aibnr.", __func__);
            return ret;
        }
        aiBnr_modelRunMode_t sw_aiBnrCfg_modelRun_mode = isp_aibnr_params->aibnr_param.sta.model_cfg.sw_aiBnrCfg_modelRun_mode;
        pAibnrManager->mMemMode = sw_aiBnrCfg_modelRun_mode == aiBnr_singleX2G8_mode ? SINGLE_MEMODE : COMBO_MEMODE;
        pAibnrManager->pCamHw->aibnr_setMemMode(pAibnrManager->pCamHw, &pAibnrManager->mMemMode);
        pAibnrManager->pCamHw->aibnr_setIspBufInf(pAibnrManager->pCamHw, &pAibnrManager->ispbuf_info);
        ret = pAibnrManager->pCamHw->aibnr_prepare(pAibnrManager->pCamHw);
        if (ret) {
            pAibnrManager->is_state_error = true;
            LOGE_AIBNR("%s: can not prepare aibnr.", __func__);
            return ret;
        }
        mmap_ispbuf(pAibnrManager, &pAibnrManager->ispbuf_info.bnr_buf, pAibnrManager->ispbuf_addr);

        pAibnrManager->postBnrParam.rawHgt = isp_acq_height;
        pAibnrManager->postBnrParam.rawWid = isp_acq_width;
        pAibnrManager->postBnrParam.rawHgtStd = isp_acq_height;
        pAibnrManager->postBnrParam.rawWidStd = bnr_buf->u.v35.aiisp.buf_stride / 2;
        pAibnrManager->postBnrParam.pRaw = NULL;
        val = (isp_acq_width + 7) / 8;
        pAibnrManager->postBnrParam.gainWid = CEIL_BY(val, 16);
        pAibnrManager->postBnrParam.gainWidStd = bnr_buf->u.v35.gain.buf_stride;
        pAibnrManager->postBnrParam.gainHgt = isp_acq_height / 2;
        pAibnrManager->postBnrParam.gainHgtStd = isp_acq_height / 2;
        pAibnrManager->postBnrParam.pGain = NULL;
        pAibnrManager->postBnrParam.ISO = isp_aibnr_params->frameIso;
        pAibnrManager->postBnrParam.gainMode = 1; // fix use mode 1
        pAibnrManager->postBnrParam.bayerPattern = isp_aibnr_params->bayer_fmt;
        strcpy(pAibnrManager->postBnrParam.sns_name, "null");
        pStrSt = pAibnrManager->pCamHw->sns_name + strlen("m00_b_");
        if (pStrSt) {
            pStrEd = strchr(pStrSt, ' ');
            if (pStrEd && (pStrEd - pStrSt) < sizeof(pAibnrManager->postBnrParam.sns_name) - 1) {
                strncpy(pAibnrManager->postBnrParam.sns_name, pStrSt, pStrEd - pStrSt);
                pAibnrManager->postBnrParam.sns_name[pStrEd - pStrSt] = 0;
            }
        }
        if (pAibnrManager->postBnrLib.ops_.bnr_init)
            pAibnrManager->postBnrLib.ops_.bnr_init(&pAibnrManager->postBnrParam);
        pAibnrManager->is_parambuf_prepare = true;
        pAibnrManager->doAiisp_en = false;
        first_param = true;
    }

    if (!first_param) {
        aiqMutex_lock(&pAibnrManager->apiFrmRate_mutex);
        if (pAibnrManager->doAiisp_en != isp_aibnr_params->aibnr_param.sta.swOn_cfg.sw_aiBnrT_manualBnrHw_en) {
            if (pAibnrManager->doAiisp_en && pAibnrManager->doAiisp_delaycnt > 0) {
                pAibnrManager->doAiisp_en = isp_aibnr_params->aibnr_param.sta.swOn_cfg.sw_aiBnrT_manualBnrHw_en;
                LOGK_AIBNR("%s: switch on aiisp is break off, doAiisp_delaycnt %d",
                           __func__, pAibnrManager->doAiisp_delaycnt);
                pAibnrManager->mAibnrCfg.mode = 0;
                pAibnrManager->doAiisp_delaycnt = 0;
                pAibnrManager->doAiisp_framecnt = 0;
                change_frmRate(pAibnrManager);
                aiqMutex_unlock(&pAibnrManager->apiFrmRate_mutex);
                goto CONTINUE;
            }
            else if (pAibnrManager->doAiisp_en && pAibnrManager->doAiisp_delaycnt == 0 && pAibnrManager->doAiisp_framecnt == 0) {
                pAibnrManager->doAiisp_en = isp_aibnr_params->aibnr_param.sta.swOn_cfg.sw_aiBnrT_manualBnrHw_en;
                pAibnrManager->doAiisp_delaycnt = 1;
                pAibnrManager->doAiisp_framecnt = 0;
                LOGK_AIBNR("%s: switch on aiisp is not complete, doAiisp_delaycnt %d",
                           __func__, pAibnrManager->doAiisp_delaycnt);
                if (!pAibnrManager->pBtnr2En) {
                    pAibnrManager->mAibnrCfg.mode = pAibnrManager->doAiisp_en ? 1 : 0;
                } else {
                    pAibnrManager->mAibnrCfg.mode = pAibnrManager->doAiisp_en ? 2 : 0;
                }
                aiqMutex_unlock(&pAibnrManager->apiFrmRate_mutex);
                goto CONTINUE;
            }

            pAibnrManager->doAiisp_en = isp_aibnr_params->aibnr_param.sta.swOn_cfg.sw_aiBnrT_manualBnrHw_en;
            if (!pAibnrManager->pBtnr2En) {
                pAibnrManager->mAibnrCfg.mode = pAibnrManager->doAiisp_en ? 1 : 0;
            } else {
                pAibnrManager->mAibnrCfg.mode = pAibnrManager->doAiisp_en ? 2 : 0;
            }

            if (pAibnrManager->doAiisp_en && AIBNR_DOAIISP_ON_DELAYCNT > 0) {
                pAibnrManager->doAiisp_delaycnt = AIBNR_DOAIISP_ON_DELAYCNT;
                pAibnrManager->doAiisp_framecnt = 0;
            } else {
                pAibnrManager->doAiisp_delaycnt = 0;
                pAibnrManager->doAiisp_framecnt = 0;
                ret = pAibnrManager->pCamHw->aibnr_setLinecnt(pAibnrManager->pCamHw, &pAibnrManager->mAibnrCfg);
                if (ret) {
                    pAibnrManager->is_state_error = true;
                    LOGE_AIBNR("%s: aibnr_setLinecnt error.", __func__);
                    aiqMutex_unlock(&pAibnrManager->apiFrmRate_mutex);
                    return ret;
                }
            }

            if (pAibnrManager->pBtnr2En)
                pAibnrManager->pCamHw->btnrWgtReproc_updateEn(pAibnrManager->pCamHw, pAibnrManager->doAiisp_en);

            change_frmRate(pAibnrManager);
            if (pAibnrManager->doAiisp_en)
                pAibnrManager->pCamHw->aibnr_clrIqParam(pAibnrManager->pCamHw);
        }
        aiqMutex_unlock(&pAibnrManager->apiFrmRate_mutex);
    }

CONTINUE:
    if (!pAibnrManager->doAiisp_en && !first_param) {
        LOGD_AIBNR("%s: aiisp is auto skip, frameIso %d.", __func__, isp_aibnr_params->frameIso);
        return XCAM_RETURN_BYPASS;
    }

    aiqMutex_lock(&pAibnrManager->apiFrmRate_mutex);
    if (pAibnrManager->doAiisp_en && pAibnrManager->doAiisp_delaycnt > 0) {
        pAibnrManager->pCamHw->aibnr_clrIqParam(pAibnrManager->pCamHw);
    }
    aiqMutex_unlock(&pAibnrManager->apiFrmRate_mutex);

    pAibnrManager->pCamHw->aibnr_getParamsBuf(pAibnrManager->pCamHw, &pV4l2Buf);
    if (pV4l2Buf) {
        struct rkaiisp_params* aiisp_cfg = (struct rkaiisp_params*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);
        struct rkaiisp_other_cfg *other_cfg = &aiisp_cfg->other_cfg;
        aibnr_params_static_t *sta = &isp_aibnr_params->aibnr_param.sta;
        aibnr_param_tunning_t *tunning = &isp_aibnr_params->aibnr_param.dyn.tunning;
        char model_file[256];
        char used_model_file[256];

        aiisp_cfg->frame_id = frame_id;
        aiisp_cfg->module_update = RKAIISP_OTHER_UPDATE;
        other_cfg->sw_prev_blacklvl  = isp_aibnr_params->prev_blacklvl;
        other_cfg->sw_post_blacklvl  = isp_aibnr_params->post_blacklvl;
        other_cfg->sw_pos_noiselimit = pow(tunning->sw_aiBnrT_noiseAdd_limit, 2) * 32767;
        other_cfg->sw_neg_noiselimit = (0xFFFF - other_cfg->sw_pos_noiselimit + 1) | (1 << 15);

        for (int i = 0; i < 33; i++) {
            other_cfg->sw_in_comp_y[i] = isp_aibnr_params->sw_in_comp_y[i];
        }

        for (int i = 0; i < 33; i++) {
            other_cfg->sw_out_decomp_y[i] = isp_aibnr_params->sw_out_decomp_y[i];
        }

        sprintf(model_file, "%s/%s", sta->model_dir.sw_aiBnrCfg_model_dir, tunning->sw_aiBnrT_model_file);
        sprintf(used_model_file, "%s/%s", sta->model_dir.sw_aiBnrCfg_model_dir, pAibnrManager->last_aibnr_params.aibnr_param.dyn.tunning.sw_aiBnrT_model_file);
        if (strcmp((char *)model_file, used_model_file) != 0) {
            struct rkaiisp_model_info *model_info;
            int i;

            aiqMutex_lock(&pAibnrManager->model_buf_mutex);
            for (i = 0; i < AIBNR_ISO_STEP_MAX; i++) {
                if (strcmp(pAibnrManager->mAibnrModelBuf.model_file[i], model_file) == 0) {
                    model_info = (struct rkaiisp_model_info *)pAibnrManager->mAibnrModelBuf.model_buf[i];
                    break;
                }
            }

            if (i >= AIBNR_ISO_STEP_MAX) {
                LOGE_AIBNR("can not find model file %s", model_file);
                aiqMutex_unlock(&pAibnrManager->model_buf_mutex);
                return XCAM_RETURN_ERROR_FAILED;
            }

            aiisp_cfg->model_mode = model_info->model_mode;
            aiisp_cfg->model_runcnt = model_info->model_runcnt;
            memcpy(&aiisp_cfg->model_cfg[0], &model_info->model_cfg[0], sizeof(model_info->model_cfg));
            memcpy(&aiisp_cfg->kwt_cfg, &model_info->kwt_cfg, sizeof(model_info->kwt_cfg));
            for (uint32_t i = 0; i < model_info->model_runcnt; i++) {
                uint32_t offset = model_info->kwt_cfg.kwt_offet[i] - sizeof(struct rkaiisp_model_info) + sizeof(struct rkaiisp_params);
                char *pKwtBuffer;

                pKwtBuffer = (char *)aiisp_cfg + offset;
                aiisp_cfg->kwt_cfg.kwt_offet[i] = offset;
                memcpy(pKwtBuffer, (char *)model_info + model_info->kwt_cfg.kwt_offet[i], model_info->kwt_cfg.kwt_pad_size[i]);
            }
            aiqMutex_unlock(&pAibnrManager->model_buf_mutex);

            aiisp_cfg->module_update |= RKAIISP_MODEL_UPDATE;
        }

        aiisp_params_print(aiisp_cfg);

        aiqMutex_lock(&pAibnrManager->iq_param_mutex);
        pAibnrManager->last_aibnr_params = *isp_aibnr_params;
        aiqMutex_unlock(&pAibnrManager->iq_param_mutex);
        pAibnrManager->is_param_update = true;
        pAibnrManager->pCamHw->aibnr_updateParams(pAibnrManager->pCamHw, pV4l2Buf);
    } else {
        LOGE_AIBNR("%s: Can not get param buffer", __func__);
    }

    return ret;
}

XCamReturn AibnrManager_start(AibnrManager_t* pAibnrManager)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!pAibnrManager->is_enable) {
        LOGD_AIBNR("%s: aibnr is disabled.", __func__);
        return XCAM_RETURN_BYPASS;
    }

    pAibnrManager->is_start = true;
    ret = pAibnrManager->pCamHw->aibnr_start(pAibnrManager->pCamHw);
    if (ret) {
        pAibnrManager->is_state_error = true;
        LOGE_AIBNR("%s: can not start aibnr.", __func__);
    }

    return ret;
}

XCamReturn AibnrManager_stop(AibnrManager_t* pAibnrManager)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!pAibnrManager->is_enable) {
        LOGD_AIBNR("%s: aibnr is disabled.", __func__);
        return XCAM_RETURN_BYPASS;
    }

    ret = pAibnrManager->pCamHw->aibnr_stop(pAibnrManager->pCamHw);
    if (ret)
        LOGE_AIBNR("%s: can not stop aibnr.", __func__);
    pAibnrManager->is_start = false;
    pAibnrManager->is_parambuf_prepare = false;
    if (pAibnrManager->postBnrLib.ops_.bnr_deinit)
        pAibnrManager->postBnrLib.ops_.bnr_deinit(&pAibnrManager->postBnrParam);
    memset(&pAibnrManager->last_aibnr_params, 0, sizeof(pAibnrManager->last_aibnr_params));
    for (int i = 0; i < pAibnrManager->beBuf_cnt; i++) {
        pAibnrManager->aiisp_idx_pool[i] = false;
    }
    munmap_ispbuf(&pAibnrManager->ispbuf_info.bnr_buf, pAibnrManager->ispbuf_addr);

    return ret;
}

XCamReturn AibnrManager_deinit(AibnrManager_t* pAibnrManager)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AibnrModelBuf_t *aibnrModelBuf = &pAibnrManager->mAibnrModelBuf;

    PostBnrLibraryDeinit(&pAibnrManager->postBnrLib);
    for (int i = 0; i < AIBNR_ISO_STEP_MAX; i++) {
        if (aibnrModelBuf->model_buf[i] != NULL) {
            aiq_free(aibnrModelBuf->model_buf[i]);
            aibnrModelBuf->model_buf[i] = NULL;
        }
    }
    aiqMutex_deInit(&pAibnrManager->idx_pool_mutex);
    aiqMutex_deInit(&pAibnrManager->iq_param_mutex);
    aiqMutex_deInit(&pAibnrManager->model_buf_mutex);
    aiqMutex_deInit(&pAibnrManager->apiFrmRate_mutex);

    return ret;
}

static int dump_aibnr_iq_params(AibnrManager_t* pAibnrManager, FILE *fp)
{
    rk_aiq_isp_aibnr_params_t* params = &pAibnrManager->last_aibnr_params;
    aibnr_param_t* cur_param = &params->aibnr_param;
    aibnr_params_static_t *sta = &cur_param->sta;
    aibnr_param_dyn_t *dyn = &cur_param->dyn;
    aibnr_param_calib_t *calib = &dyn->calib;
    aibnr_param_tunning_t *tunning = &dyn->tunning;

    fprintf(fp, "%s\n", "aibnr iq params");

    fprintf(fp, "is_hdr=%d iso=%d\n",
            params->is_hdr, params->frameIso);

    fprintf(fp, "sw_aiBnrC_blkPrev_offset=%f\n",
            calib->sw_aiBnrC_blkPrev_offset);

    fprintf(fp, "sw_aiBnrC_blkPost_offset=%f\n",
            calib->sw_aiBnrC_blkPost_offset);

    fprintf(fp, "sw_aiBnrC_shotNoise_val=%f\n",
            calib->sw_aiBnrC_shotNoise_val);

    fprintf(fp, "sw_aiBnrC_readNoise_val=%f\n",
            calib->sw_aiBnrC_readNoise_val);

    fprintf(fp, "sw_aiBnrC_fixedNoise_val=%f\n",
            calib->sw_aiBnrC_fixedNoise_val);

    fprintf(fp, "sw_aiBnrT_totalNr_strg=%f\n",
            tunning->sw_aiBnrT_totalNr_strg);

    fprintf(fp, "sw_aiBnrT_shotNr_strg=%f\n",
            tunning->sw_aiBnrT_shotNr_strg);

    fprintf(fp, "sw_aiBnrT_readNr_strg=%f\n",
            tunning->sw_aiBnrT_readNr_strg);

    fprintf(fp, "sw_aiBnrT_fixedNr_strg=%f\n",
            tunning->sw_aiBnrT_fixedNr_strg);

    fprintf(fp, "sw_aiBnrT_motionBias_offset=%f\n",
            tunning->sw_aiBnrT_motionBias_offset);

    fprintf(fp, "sw_aiBnrT_motionGain_strg=%f\n",
            tunning->sw_aiBnrT_motionGain_strg);

    fprintf(fp, "sw_aiBnrT_noiseAdd_ratio=%f\n",
            tunning->sw_aiBnrT_noiseAdd_ratio);

    fprintf(fp, "sw_aiBnrT_noiseAdd_limit=%f\n",
            tunning->sw_aiBnrT_noiseAdd_limit);

    fprintf(fp, "sw_aiBnrT_noiseAddMot_offset=%f\n",
            tunning->sw_aiBnrT_noiseAddMot_offset);

    fprintf(fp, "sw_aiBnrT_noiseAddMot_coeff=%f\n",
            tunning->sw_aiBnrT_noiseAddMot_coeff);

    fprintf(fp, "sw_aiBnrT_noiseAddLuma_offset=%f\n",
            tunning->sw_aiBnrT_noiseAddLuma_offset);

    fprintf(fp, "sw_aiBnrT_noiseAddLuma_coeff=%f\n",
            tunning->sw_aiBnrT_noiseAddLuma_coeff);

    fprintf(fp, "sw_aiBnrT_noiseAddLumaClip_th=%f\n",
            tunning->sw_aiBnrT_noiseAddLumaClip_th);

    fprintf(fp, "sw_aiBnrT_noiseAddLumaStatic_th=%f\n",
            tunning->sw_aiBnrT_noiseAddLumaStatic_th);

    fprintf(fp, "sw_aiBnrT_nonLinear_scale=%f\n",
            tunning->sw_aiBnrT_nonLinear_scale);

    fprintf(fp, "sw_aiBnrT_nonLinear_ratio=%f\n",
            tunning->sw_aiBnrT_nonLinear_ratio);

    fprintf(fp, "sw_aiBnrT_nonLinear_adjust=%f\n",
            tunning->sw_aiBnrT_nonLinear_adjust);

    for (int i = 0; i < AIBNR_VALURENR_STRG_LEN; i++) {
        fprintf(fp, "sw_aiBnrT_bnrValueNr_strg[%d]=%f",
                i, tunning->sw_aiBnrT_bnrValueNr_strg[i]);
    }

    fprintf(fp, "sw_aiBnrCfg_model_dir=%s\n",
            sta->model_dir.sw_aiBnrCfg_model_dir);

    fprintf(fp, "sw_aiBnrT_model_file=%s\n",
            tunning->sw_aiBnrT_model_file);

    fprintf(fp, "sw_aiBnrT_debug_mode=%d\n",
            sta->debug.sw_aiBnrT_debug_mode);

    fprintf(fp, "sw_aiBnrT_autoSwOn_thred=%d\n",
            sta->swOn_cfg.sw_aiBnrT_autoSwOn_thred);

    fprintf(fp, "sw_aiBnrT_autoSwGap_thred=%d\n",
            sta->swOn_cfg.sw_aiBnrT_autoSwGap_thred);

    fprintf(fp, "sw_aiBnrT_manualBnrHw_en=%d\n",
            sta->swOn_cfg.sw_aiBnrT_manualBnrHw_en);

    return 0;
}

static void dump_aibnr_hwi_params(AibnrManager_t* pAibnrManager, FILE *fp)
{
    rk_aiq_isp_aibnr_params_t* params = &pAibnrManager->last_aibnr_params;
    struct isp35_ai_cfg *ai_cfg = &params->ai_cfg;

    fprintf(fp, "%s\n", "aipre hwi params");

    fprintf(fp, "%-15s%-13d%-13d\n", "AIPRE_CTRL",
            ai_cfg->aipre_luma2gain_dis, ai_cfg->aipre_narmap_inv);

    fprintf(fp, "%-15s%-13d%-13d%-13d\n", "AIPRE_NL_PRE",
            ai_cfg->aipre_scale, ai_cfg->aipre_zp,
            ai_cfg->aipre_black_lvl);

    fprintf(fp, "%-15s%-13d%-13d%-13d\n", "AIPRE_GAIN",
            ai_cfg->aipre_gain_alpha, ai_cfg->aipre_global_gain,
            ai_cfg->aipre_gain_ratio);

    fprintf(fp, "%-15s%-13d%-13d%-13d\n", "AIPRE_NOISE0",
            ai_cfg->aipre_noise_mot_offset, ai_cfg->aipre_noise_mot_gain,
            ai_cfg->aipre_noise_luma_offset);

    fprintf(fp, "%-15s%-13d%-13d%-13d\n", "AIPRE_NOISE1",
            ai_cfg->aipre_noise_luma_gain, ai_cfg->aipre_noise_luma_clip,
            ai_cfg->aipre_noise_luma_static);

    fprintf(fp, "%-15s%-13d%-13d\n", "AIPRE_NOISE2",
            ai_cfg->aipre_nar_manual, ai_cfg->aipre_nar_manual_alpha);

    for (int i = 0; i < ISP35_AI_SIGMA_NUM; i += 3) {
        fprintf(fp, "%-15s%-13d%-13d%-13d\n", "AIPRE_SIGMA",
                ai_cfg->aipre_sigma_y[i], ai_cfg->aipre_sigma_y[i + 1],
                ai_cfg->aipre_sigma_y[i + 2]);
    }

    fprintf(fp, "%-15s%-13d%-13d%-13d%-13d%-13d\n", "VPSL_PYR",
            ai_cfg->pyr_yraw_mode, ai_cfg->pyr_sigma_en, ai_cfg->pyr_yraw_sel,
            ai_cfg->pyr_gain_leftshift, ai_cfg->pyr_blacklvl_sig);

    for (int i = 0; i < ISP35_VPSL_SIGMA_NUM; i += 9) {
        fprintf(fp, "%-15s%-13d%-13d%-13d%-13d%-13d%-13d%-13d%-13d%-13d\n", "VPSL_SIGMA",
                ai_cfg->pyr_sigma_y[i + 0], ai_cfg->pyr_sigma_y[i + 1], ai_cfg->pyr_sigma_y[i + 2],
                ai_cfg->pyr_sigma_y[i + 3], ai_cfg->pyr_sigma_y[i + 4], ai_cfg->pyr_sigma_y[i + 5],
                ai_cfg->pyr_sigma_y[i + 6], ai_cfg->pyr_sigma_y[i + 7], ai_cfg->pyr_sigma_y[i + 8]);
    }
}

static void dump_aibnr_params(AibnrManager_t* pAibnrManager, char *filename)
{
    FILE *fp = NULL;

    fp = fopen(filename, "wb");
    if (!fp) {
        LOGE_AIBNR("can not open file %s", filename);
        return;
    }

    aiqMutex_lock(&pAibnrManager->iq_param_mutex);
    dump_aibnr_iq_params(pAibnrManager, fp);
    dump_aibnr_hwi_params(pAibnrManager, fp);
    aiqMutex_unlock(&pAibnrManager->iq_param_mutex);

    if (fp) {
        LOGK_AIBNR("dump aibnr params to filename %s", filename);
        fclose(fp);
    }
}

static XCamReturn dump_dmabuf(char *filename, u32 dma_fd, u32 size)
{
    FILE *fp;
    char *dump_addr;
    struct dma_buf_sync sync = { 0 };

    dump_addr = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, dma_fd, 0);
    if (MAP_FAILED == dump_addr) {
        LOGE_AIBNR("fd %d, size %d mmap failed", dma_fd, size);
        return XCAM_RETURN_ERROR_FAILED;
    }

    sync.flags = DMA_BUF_SYNC_READ | DMA_BUF_SYNC_START;
    ioctl(dma_fd, DMA_BUF_IOCTL_SYNC, &sync);

    fp = fopen(filename, "wb");
    if (fp) {
        LOGK_AIBNR("dump buffer to filename %s", filename);
        fwrite(dump_addr, 1, size, fp);
        fclose(fp);
    } else {
        LOGE_AIBNR("can not open file %s", filename);
    }

    sync.flags = DMA_BUF_SYNC_READ | DMA_BUF_SYNC_END;
    ioctl(dma_fd, DMA_BUF_IOCTL_SYNC, &sync);

    munmap(dump_addr, size);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn write_file(char *filename, char *addr, u32 size)
{
    FILE *fp;

    fp = fopen(filename, "wb");
    if (fp) {
        LOGD_AIBNR("dump buffer to filename %s", filename);
        fwrite(addr, 1, size, fp);
        fclose(fp);
    } else {
        LOGE_AIBNR("can not open file %s", filename);
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn copy_gain2aiispout(AibnrManager_t* pAibnrManager, int aiisp_buf_fd,
                                     int gain_index, int narmap_index, int aiisp_index, aibnr_debug_mode_t sw_aiBnrT_debug_mode)
{
    struct rkisp_bnr_buf_info *bnr_buf = &pAibnrManager->ispbuf_info.bnr_buf;
    ispbuf_addr_t *ispbuf_addr = pAibnrManager->ispbuf_addr;
    uint32_t aiisp_width = pAibnrManager->isp_acq_width;
    uint32_t aiisp_height = pAibnrManager->isp_acq_height;
    uint32_t gain_width, gain_height;
    uint32_t narmap_width, narmap_height;
    uint32_t aiisp_stride, gain_stride, narmap_stride;
    uint32_t g_offset = 0;
    struct dma_buf_sync sync = { 0 };

    if (gain_index >= AIBNR_GAIN_BUF_CNT || gain_index < 0) {
        LOGE_AIBNR("%s: gain_index %d is error", __func__, gain_index);
        return XCAM_RETURN_ERROR_FAILED;
    }
    if (narmap_index >= pAibnrManager->feBuf_cnt || narmap_index < 0) {
        LOGE_AIBNR("%s: narmap_index %d is error", __func__, narmap_index);
        return XCAM_RETURN_ERROR_FAILED;
    }
    if (aiisp_index >= pAibnrManager->beBuf_cnt || aiisp_index < 0) {
        LOGE_AIBNR("%s: aiisp_index %d is error", __func__, aiisp_index);
        return XCAM_RETURN_ERROR_FAILED;
    }
    if (ispbuf_addr[aiisp_index].aiisp_addr == NULL ||
            ispbuf_addr[narmap_index].aipre_addr == NULL ||
            ispbuf_addr[gain_index].gain_addr == NULL) {
        LOGE_AIBNR("%s: aiisp_addr or gain_addr or aipre_addr is error", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }

    aiisp_stride = bnr_buf->u.v35.aiisp.buf_stride / 2;
    gain_width = CEIL_BY(aiisp_width, 16) / 8;
    gain_height = aiisp_height / 2;
    gain_stride = bnr_buf->u.v35.gain.buf_stride;
    narmap_width = CEIL_BY(aiisp_width, 16) / 4;
    narmap_height = aiisp_height / 2;
    narmap_stride = bnr_buf->u.v35.aipre_gain.buf_stride;

    sync.flags = DMA_BUF_SYNC_RW | DMA_BUF_SYNC_START;
    ioctl(aiisp_buf_fd, DMA_BUF_IOCTL_SYNC, &sync);
    // fix use sw_aiisp_gain_mode 1
    uint16_t *aiisp_addr = (uint16_t *)ispbuf_addr[aiisp_index].aiisp_addr;
    uint16_t *gain_addr = (uint16_t *)ispbuf_addr[gain_index].gain_addr;
    uint8_t *aipre_addr = (uint8_t *)ispbuf_addr[narmap_index].aipre_addr;

    if (sw_aiBnrT_debug_mode == aibnr_debugGain_mode) {
        for (uint32_t i = 0; i < gain_height; i++) {
            for (uint32_t j = 0; j < gain_width; j++) {
                aiisp_addr[i * aiisp_stride + j] = (gain_addr[i * gain_stride + j] & 0xFF) << 8;
            }
        }
    } else {
        uint16_t max_narmap_val = 1;
        uint16_t debug_val = 0;

        for (uint32_t i = 0; i < narmap_height; i++) {
            for (uint32_t j = 0; j < narmap_width; j++) {
                if (aipre_addr[i * narmap_stride + j] > max_narmap_val) {
                    max_narmap_val = aipre_addr[i * narmap_stride + j];
                }
            }
        }
        for (uint32_t i = 0; i < narmap_height; i++) {
            for (uint32_t j = 0; j < narmap_width; j++) {
                debug_val = 255 * aipre_addr[i * narmap_stride + j] / max_narmap_val;
                aiisp_addr[i * aiisp_stride + j] = debug_val << 8;
            }
        }
    }

    sync.flags = DMA_BUF_SYNC_RW | DMA_BUF_SYNC_END;
    ioctl(aiisp_buf_fd, DMA_BUF_IOCTL_SYNC, &sync);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn do_post_bnr(AibnrManager_t* pAibnrManager, int aiisp_buf_fd, int gain_buf_fd, int aiisp_index, int gain_index)
{
    ispbuf_addr_t *ispbuf_addr = pAibnrManager->ispbuf_addr;
    struct dma_buf_sync sync = { 0 };
    bool sw_aiBnrT_borderHdl_en;

    if (!pAibnrManager->postBnrLib.ops_.bnr_proc)
        return XCAM_RETURN_NO_ERROR;

    if (gain_index >= AIBNR_GAIN_BUF_CNT || gain_index < 0) {
        LOGE_AIBNR("%s: gain_index %d is error", __func__, gain_index);
        return XCAM_RETURN_ERROR_FAILED;
    }
    if (aiisp_index >= pAibnrManager->beBuf_cnt || aiisp_index < 0) {
        LOGE_AIBNR("%s: aiisp_index %d is error", __func__, aiisp_index);
        return XCAM_RETURN_ERROR_FAILED;
    }
    if (ispbuf_addr[aiisp_index].aiisp_addr == NULL ||
            ispbuf_addr[gain_index].gain_addr == NULL) {
        LOGE_AIBNR("%s: aiisp_addr or gain_addr or aipre_addr is error", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }

    aiqMutex_lock(&pAibnrManager->iq_param_mutex);
    sw_aiBnrT_borderHdl_en =
        pAibnrManager->last_aibnr_params.aibnr_param.sta.border_cfg.sw_aiBnrT_borderHdl_en;
    if (!sw_aiBnrT_borderHdl_en) {
        aiqMutex_unlock(&pAibnrManager->iq_param_mutex);
        return XCAM_RETURN_NO_ERROR;
    }
    pAibnrManager->postBnrParam.ISO = pAibnrManager->last_aibnr_params.frameIso;
    pAibnrManager->postBnrParam.gainHborder =
        pAibnrManager->last_aibnr_params.aibnr_param.sta.border_cfg.sw_aiBnrT_gainHborder_val;
    pAibnrManager->postBnrParam.gainWborder =
        pAibnrManager->last_aibnr_params.aibnr_param.sta.border_cfg.sw_aiBnrT_gainWborder_val;
    aiqMutex_unlock(&pAibnrManager->iq_param_mutex);

    sync.flags = DMA_BUF_SYNC_RW | DMA_BUF_SYNC_START;
    ioctl(aiisp_buf_fd, DMA_BUF_IOCTL_SYNC, &sync);
    ioctl(gain_buf_fd, DMA_BUF_IOCTL_SYNC, &sync);

    pAibnrManager->postBnrParam.pRaw  = (void *)ispbuf_addr[aiisp_index].aiisp_addr;
    pAibnrManager->postBnrParam.pGain = (void *)ispbuf_addr[gain_index].gain_addr;

    LOGD_AIBNR("%s: rawHgt %d, rawWid %d, rawHgtStd %d, rawWidStd %d, gainHgt %d, gainWid %d, gainHgtStd %d, gainWidStd %d, ISO %f, gainMode %d, bayerPattern %d, pRaw %p, pGain %p, sns_name %s, gainHborder %d, gainWborder %d",
               __func__,
               pAibnrManager->postBnrParam.rawHgt,
               pAibnrManager->postBnrParam.rawWid,
               pAibnrManager->postBnrParam.rawHgtStd,
               pAibnrManager->postBnrParam.rawWidStd,
               pAibnrManager->postBnrParam.gainHgt,
               pAibnrManager->postBnrParam.gainWid,
               pAibnrManager->postBnrParam.gainHgtStd,
               pAibnrManager->postBnrParam.gainWidStd,
               pAibnrManager->postBnrParam.ISO,
               pAibnrManager->postBnrParam.gainMode,
               pAibnrManager->postBnrParam.bayerPattern,
               pAibnrManager->postBnrParam.pRaw,
               pAibnrManager->postBnrParam.pGain,
               pAibnrManager->postBnrParam.sns_name,
               pAibnrManager->postBnrParam.gainHborder,
               pAibnrManager->postBnrParam.gainWborder);
    pAibnrManager->postBnrLib.ops_.bnr_proc(&pAibnrManager->postBnrParam);

    sync.flags = DMA_BUF_SYNC_RW | DMA_BUF_SYNC_END;
    ioctl(aiisp_buf_fd, DMA_BUF_IOCTL_SYNC, &sync);
    ioctl(gain_buf_fd, DMA_BUF_IOCTL_SYNC, &sync);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AibnrManager_doIspBe(AibnrManager_t* pAibnrManager, struct rkisp_aiisp_st *isp_in)
{
    struct rkisp_bnr_buf_info *bnr_buf = &pAibnrManager->ispbuf_info.bnr_buf;
    rk_aiq_isp_aibnr_params_t *isp_aibnr_params;
    struct rkaiisp_model_info *model_info;
    struct rkisp_buf_info *buf_inf;
    char dump_file[256];
    u32 frame_id, idx;
    bool dump_flag = false;
    u32 i;

#define AIBNR_DUMPBUF_DIR        "/data/aibnr_dump"

    if (pAibnrManager->dump_raw_num >= -1 && pAibnrManager->dump_raw_num != 0) {
        dump_flag = true;
        if (pAibnrManager->dump_raw_num > -1)
            pAibnrManager->dump_raw_num--;
    }

    if (dump_flag && !pAibnrManager->is_bypass) {
        frame_id = isp_in->sequence;
        LOGD_AIBNR("aibnr dump frame %d, dump_raw_num %d", frame_id, pAibnrManager->dump_raw_num);

        idx = isp_in->iir_index;
        buf_inf = &bnr_buf->iir;
        sprintf(dump_file, "%s/%04d_%02d_iir.raw", AIBNR_DUMPBUF_DIR, frame_id, idx);
        dump_dmabuf(dump_file, buf_inf->buf_fd[idx], buf_inf->buf_size);

        idx = isp_in->gain_index;
        buf_inf = &bnr_buf->u.v35.gain;
        sprintf(dump_file, "%s/%04d_%02d_gain.raw", AIBNR_DUMPBUF_DIR, frame_id, idx);
        dump_dmabuf(dump_file, buf_inf->buf_fd[idx], buf_inf->buf_size);

        idx = isp_in->aipre_gain_index;
        buf_inf = &bnr_buf->u.v35.aipre_gain;
        sprintf(dump_file, "%s/%04d_%02d_aipre.raw", AIBNR_DUMPBUF_DIR, frame_id, idx);
        dump_dmabuf(dump_file, buf_inf->buf_fd[idx], buf_inf->buf_size);

        idx = isp_in->vpsl_index;
        buf_inf = &bnr_buf->u.v35.vpsl;
        sprintf(dump_file, "%s/%04d_%02d_yraw%d-%d_%d-%d_%d-%d_sig%d-%d_%d-%d_%d-%d_%d-%d_vpsl.raw",
                AIBNR_DUMPBUF_DIR, frame_id, idx,
                bnr_buf->u.v35.vpsl_yraw_offs[0],
                bnr_buf->u.v35.vpsl_yraw_stride[0],
                bnr_buf->u.v35.vpsl_yraw_offs[1],
                bnr_buf->u.v35.vpsl_yraw_stride[1],
                bnr_buf->u.v35.vpsl_yraw_offs[2],
                bnr_buf->u.v35.vpsl_yraw_stride[2],
                bnr_buf->u.v35.vpsl_sig_offs[0],
                bnr_buf->u.v35.vpsl_sig_stride[0],
                bnr_buf->u.v35.vpsl_sig_offs[1],
                bnr_buf->u.v35.vpsl_sig_stride[1],
                bnr_buf->u.v35.vpsl_sig_offs[2],
                bnr_buf->u.v35.vpsl_sig_stride[2],
                bnr_buf->u.v35.vpsl_sig_offs[3],
                bnr_buf->u.v35.vpsl_sig_stride[3]);
        dump_dmabuf(dump_file, buf_inf->buf_fd[idx], buf_inf->buf_size);

        idx = isp_in->aiisp_index;
        buf_inf = &bnr_buf->u.v35.aiisp;
        sprintf(dump_file, "%s/%04d_%02d_aiisp.raw", AIBNR_DUMPBUF_DIR, frame_id, idx);
        dump_dmabuf(dump_file, buf_inf->buf_fd[idx], buf_inf->buf_size);

        sprintf(dump_file, "%s/%04d_aibnr_params.txt", AIBNR_DUMPBUF_DIR, frame_id);
        dump_aibnr_params(pAibnrManager, dump_file);

        aiqMutex_lock(&pAibnrManager->iq_param_mutex);
        isp_aibnr_params = &pAibnrManager->last_aibnr_params;
        sprintf(dump_file, "%s/%s",
                isp_aibnr_params->aibnr_param.sta.model_dir.sw_aiBnrCfg_model_dir,
                isp_aibnr_params->aibnr_param.dyn.tunning.sw_aiBnrT_model_file);
        aiqMutex_unlock(&pAibnrManager->iq_param_mutex);

        aiqMutex_lock(&pAibnrManager->model_buf_mutex);
        for (i = 0; i < AIBNR_ISO_STEP_MAX; i++) {
            if (strcmp(pAibnrManager->mAibnrModelBuf.model_file[i], dump_file) == 0) {
                model_info = (struct rkaiisp_model_info *)pAibnrManager->mAibnrModelBuf.model_buf[i];
                break;
            }
        }

        if (i >= AIBNR_ISO_STEP_MAX) {
            LOGE_AIBNR("can not find model file %s in dumpbuf", dump_file);
        } else {
            for (i = 0; i < model_info->model_runcnt; i++) {
                char *pKwtBuffer = (char *)model_info + model_info->kwt_cfg.kwt_offet[i];

                sprintf(dump_file, "%s/%04d_%02d_%01d_%04d_kwt.bin",
                        AIBNR_DUMPBUF_DIR, frame_id, idx, i,
                        model_info->kwt_cfg.kwt_pad_size[i]);
                write_file(dump_file, pKwtBuffer, model_info->kwt_cfg.kwt_pad_size[i]);
            }
        }
        aiqMutex_unlock(&pAibnrManager->model_buf_mutex);
    }

    if (!pAibnrManager->is_bypass) {
        do_post_bnr(pAibnrManager, bnr_buf->u.v35.aiisp.buf_fd[isp_in->aiisp_index],
                    bnr_buf->u.v35.gain.buf_fd[isp_in->gain_index], isp_in->aiisp_index, isp_in->gain_index);
    }

    aiqMutex_lock(&pAibnrManager->iq_param_mutex);
    aibnr_debug_mode_t sw_aiBnrT_debug_mode = pAibnrManager->last_aibnr_params.aibnr_param.sta.debug.sw_aiBnrT_debug_mode;
    aiqMutex_unlock(&pAibnrManager->iq_param_mutex);

    if (!pAibnrManager->is_bypass && sw_aiBnrT_debug_mode != aibnr_debugOff_mode) {
        idx = isp_in->aiisp_index;
        buf_inf = &bnr_buf->u.v35.aiisp;
        copy_gain2aiispout(pAibnrManager, buf_inf->buf_fd[idx], isp_in->gain_index,
                           isp_in->aipre_gain_index, isp_in->aiisp_index, sw_aiBnrT_debug_mode);
    }

    LOGD_AIBNR("aibnr_doIspBe, frame id %d, iir_index %d, gain_index %d, aipre_gain_index %d, vpsl_index %d, aiisp_index %d, is_bypass %d, doAiisp_en %d",
               isp_in->sequence,
               isp_in->iir_index,
               isp_in->gain_index,
               isp_in->aipre_gain_index,
               isp_in->vpsl_index,
               isp_in->aiisp_index,
               pAibnrManager->is_bypass,
               pAibnrManager->doAiisp_en);
    pAibnrManager->pCamHw->aibnr_doIspBe(pAibnrManager->pCamHw, isp_in);
    pAibnrManager->doAiisp_framecnt++;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AibnrManager_hdlEvent(AibnrManager_t* pAibnrManager, AiqHwAinnEvt_t *event)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct rkisp_aiisp_st *isp_idxbuf = &event->queue_buf.aibnr_st;
    struct rkisp_bnr_buf_info *bnr_buf = &pAibnrManager->ispbuf_info.bnr_buf;
    int i, aiisp_idx = 0;

    if (!pAibnrManager->is_enable) {
        LOGD_AIBNR("%s: aibnr is disabled.", __func__);
        return XCAM_RETURN_BYPASS;
    }

    if (pAibnrManager->is_state_error) {
        LOGE_AIBNR("%s: aibnr in error state.", __func__);
        return XCAM_RETURN_BYPASS;
    }

    // isp event
    if (event->_base.type == ISP_POLL_AIISP) {
        if (event->_event_id == RKISP_AIISP_WR_LINECNT_ID) {
            // call aiisp ioctrl to do NRNN
            if (isp_idxbuf->iir_index < 0 || isp_idxbuf->iir_index >= bnr_buf->iir.buf_cnt ||
                    isp_idxbuf->gain_index < 0 || isp_idxbuf->gain_index >= bnr_buf->u.v35.gain.buf_cnt ||
                    isp_idxbuf->aipre_gain_index < 0 || isp_idxbuf->aipre_gain_index >= bnr_buf->u.v35.aipre_gain.buf_cnt ||
                    isp_idxbuf->vpsl_index < 0 || isp_idxbuf->vpsl_index >= bnr_buf->u.v35.vpsl.buf_cnt) {
                LOGE_AIBNR("WR_LINECNT_ID event error index, to check");
                return XCAM_RETURN_ERROR_FAILED;
            }

            if (pAibnrManager->is_bypass) {
                // skip nn, inform isp to do BE
                pAibnrManager->isp_in.timestamp = isp_idxbuf->timestamp;
                pAibnrManager->isp_in.sequence  = isp_idxbuf->sequence;
                pAibnrManager->isp_in.iir_index = isp_idxbuf->iir_index;
                pAibnrManager->isp_in.gain_index = isp_idxbuf->gain_index;
                pAibnrManager->isp_in.aipre_gain_index = isp_idxbuf->aipre_gain_index;
                pAibnrManager->isp_in.vpsl_index = isp_idxbuf->vpsl_index;
                pAibnrManager->isp_in.aiisp_index = -1;

                AibnrManager_doIspBe(pAibnrManager, &pAibnrManager->isp_in);
                return ret;
            }

            aiqMutex_lock(&pAibnrManager->idx_pool_mutex);
            for (i = 0; i < pAibnrManager->beBuf_cnt; i++) {
                if (pAibnrManager->aiisp_idx_pool[i] == 0) {
                    pAibnrManager->aiisp_idx_pool[i] = true;//buf to using
                    aiisp_idx = i;
                    break;
                }
            }
            if (i == pAibnrManager->beBuf_cnt) {
                aiisp_idx                        = 0;
                pAibnrManager->aiisp_idx_pool[0] = true;  // fixed use first buf
                LOGD_AIBNR("no free aiisp buf, use buf 0");
            }
            aiqMutex_unlock(&pAibnrManager->idx_pool_mutex);

            isp_idxbuf->aiisp_index = aiisp_idx;
            pAibnrManager->isp_out = event->queue_buf.aibnr_st;
            LOGD_AIBNR("aibnr_doNrnn, frame id %d, iir_index %d, gain_index %d, aipre_gain_index %d, vpsl_index %d, aiisp_index %d",
                       pAibnrManager->isp_out.sequence,
                       pAibnrManager->isp_out.iir_index,
                       pAibnrManager->isp_out.gain_index,
                       pAibnrManager->isp_out.aipre_gain_index,
                       pAibnrManager->isp_out.vpsl_index,
                       pAibnrManager->isp_out.aiisp_index);
            pAibnrManager->pCamHw->aibnr_doNrnn(pAibnrManager->pCamHw, &pAibnrManager->isp_out);
        } else {
            if (isp_idxbuf->aiisp_index < 0)
                return ret;

            if (isp_idxbuf->aiisp_index >= bnr_buf->u.v35.aiisp.buf_cnt) {
                LOGE_AIBNR("RD_LINECNT_ID event error index, to check, aiisp_index %d, buf_cnt %d",
                           isp_idxbuf->aiisp_index, bnr_buf->u.v35.aiisp.buf_cnt);
                return XCAM_RETURN_ERROR_FAILED;
            }
            aiqMutex_lock(&pAibnrManager->idx_pool_mutex);
            pAibnrManager->aiisp_idx_pool[isp_idxbuf->aiisp_index] = false;
            aiqMutex_unlock(&pAibnrManager->idx_pool_mutex);
            LOGD_AIBNR("aiisp_index %d is free by isp", isp_idxbuf->aiisp_index);
        }
    }
    // aiisp event
    else if (event->_base.type == ISP_POLL_AIBNR_DONE) {
        // inform isp to do BE
        pAibnrManager->isp_in = event->queue_buf.aibnr_st;
        AibnrManager_doIspBe(pAibnrManager, &pAibnrManager->isp_in);
    }
    // rknn event
    else if (event->_base.type == ISP_POLL_RKNN_DONE) {
        // inform isp to do BE
        pAibnrManager->isp_in = event->queue_buf.aibnr_st;
        AibnrManager_doIspBe(pAibnrManager, &pAibnrManager->isp_in);
    }

    return ret;
}

bool AibnrManager_isNeedRknn(AibnrManager_t* pAibnrManager)
{
    bool ret = false;

    return ret;
}

XCamReturn AibnrManager_init(AiqManager_t* pAiqManager, AibnrManager_t* pAibnrManager)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    memset(pAibnrManager, 0, sizeof(AibnrManager_t));
    aiqMutex_init(&pAibnrManager->idx_pool_mutex);
    aiqMutex_init(&pAibnrManager->iq_param_mutex);
    aiqMutex_init(&pAibnrManager->model_buf_mutex);
    aiqMutex_init(&pAibnrManager->apiFrmRate_mutex);
    pAibnrManager->pAiqManager = pAiqManager;
    if (PostBnrLibraryInit(&pAibnrManager->postBnrLib))
        PostBnrLibraryLoadSymbols(&pAibnrManager->postBnrLib);

    return ret;
}

int AibnrManager_dumpRaw(AibnrManager_t* pAibnrManager, int dump_raw_num)
{
    int ret = -1;

    if (pAibnrManager) {
        ret = 0;
        pAibnrManager->dump_raw_num = dump_raw_num;
    }

    return ret;
}

XCamReturn AibnrManager_setFrmRate(AiqAlgoHandler_t* aeHandler, bool is_group,
                                   AibnrManager_t* pAibnrManager, ae_api_expSwAttr_t *expSwAttr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ae_api_expSwAttr_t tmpExpSwAttr;

    if (!pAibnrManager->is_enable) {
        LOGD_AIBNR("%s: aibnr is disabled.", __func__);
        return XCAM_RETURN_BYPASS;
    }

    aiqMutex_lock(&pAibnrManager->apiFrmRate_mutex);
    pAibnrManager->apiFrmRate = expSwAttr->commCtrl.frmRate;
    pAibnrManager->apiFrmRate_valid = true;
    pAibnrManager->is_group = is_group;
    pAibnrManager->aeHandler = aeHandler;
    if (pAibnrManager->doAiisp_en) {
        expSwAttr->commCtrl.frmRate = pAibnrManager->iqFrmRate;
    }

    if (is_group) {
        AiqAlgoCamGroupAeHandler_setExpSwAttr((AiqAlgoCamGroupAeHandler_t*)aeHandler, *expSwAttr);
    } else {
        AiqAlgoHandlerAe_setExpSwAttr((AiqAlgoHandlerAe_t*)aeHandler, *expSwAttr);
    }
    aiqMutex_unlock(&pAibnrManager->apiFrmRate_mutex);

    LOGK_AIBNR("record apiFrmRate: mode %d, fps %f",
               pAibnrManager->apiFrmRate.sw_aeT_frmRate_mode,
               pAibnrManager->apiFrmRate.sw_aeT_frmRate_val);

    return ret;
}

//#define DUMP_BYFILE
#ifdef DUMP_BYFILE
static bool getValueFromFile(const char* path, int* pos) {
    const char* delim = " ";
    char buffer[16]   = {0};
    int fp;

    fp = open(path, O_RDONLY | O_SYNC);
    if (fp != -1) {
        if (read(fp, buffer, sizeof(buffer)) <= 0) {
            LOGE_AF("read %s failed!", path);
            goto OUT;
        } else {
            char* p = NULL;

            p = strtok(buffer, delim);
            if (p != NULL) {
                *pos = atoi(p);
            }
        }
        close(fp);
        return true;
    }

OUT:
    return false;
}
#endif

XCamReturn AibnrManager_notify_sof(AibnrManager_t* pAibnrManager)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pAibnrManager) {
#ifdef DUMP_BYFILE
        int aibnr_dump = 0;

        if (getValueFromFile("/data/.aibnr_dump", &aibnr_dump) == true) {
            if (aibnr_dump != 0) {
                AibnrManager_dumpRaw(pAibnrManager, aibnr_dump);
                remove("/data/.aibnr_dump");
            }
        }
#endif
        aiqMutex_lock(&pAibnrManager->apiFrmRate_mutex);
        if (pAibnrManager->doAiisp_delaycnt > 0) {
            pAibnrManager->doAiisp_delaycnt--;
            if (pAibnrManager->doAiisp_delaycnt == 0) {
                pAibnrManager->doAiisp_framecnt = 0;
                LOGK_AIBNR("%s: switch aiisp complete, doAiisp_en %d", __func__, pAibnrManager->doAiisp_en);
                ret = pAibnrManager->pCamHw->aibnr_setLinecnt(pAibnrManager->pCamHw, &pAibnrManager->mAibnrCfg);
                if (ret) {
                    pAibnrManager->is_state_error = true;
                    LOGE_AIBNR("%s: aibnr_setLinecnt error.", __func__);
                    aiqMutex_unlock(&pAibnrManager->apiFrmRate_mutex);
                    return ret;
                }

                if (pAibnrManager->doAiisp_en) {
                    aiqMutex_lock(&pAibnrManager->idx_pool_mutex);
                    for (int i = 0; i < pAibnrManager->beBuf_cnt; i++)
                        pAibnrManager->aiisp_idx_pool[i] = false;
                    aiqMutex_unlock(&pAibnrManager->idx_pool_mutex);
                }
            }
        }
        aiqMutex_unlock(&pAibnrManager->apiFrmRate_mutex);
    }

    return ret;
}

#endif

