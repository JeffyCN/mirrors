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
#include "RkAiynrManager.h"

#if RKAIQ_HAVE_AIYNR

#define CEIL_DOWN(x, y)         (((x) + ((y) - 1)) / (y))
#define FLOOR_BY(v, r)          (((v) / (r)) * (r))
#define CEIL_BY(v, r)           FLOOR_BY(((v) + (r) - 1), (r))

#define AIYNR_DUMPBUF_DIR       "/data/aiynr_dump"

static void AiynrCalcIspBufSize(struct rkaiisp_ispbuf_info *ispbuf_info, int raw_wid, int raw_hgt)
{
    int gain_wid = raw_wid / 8;
    int gain_hgt = raw_hgt / 2;

    ispbuf_info->iir_width  = raw_wid;
    ispbuf_info->iir_height = raw_hgt;
    ispbuf_info->narmap_width  = CEIL_BY(raw_wid / 4, 16);
    ispbuf_info->narmap_height = gain_hgt;

    for (int i = 0; i < 7; i++) {
        if (i > 0) {
            ispbuf_info->raw_width[i - 1]  = raw_wid;
            ispbuf_info->raw_height[i - 1] = raw_hgt;
        }

        if (i < 5) {
            ispbuf_info->sig_width[i]  = raw_wid / 2;
            ispbuf_info->sig_height[i] = raw_hgt / 2;
        }

        raw_hgt = CEIL_BY(CEIL_DOWN(raw_hgt, 2), 2);
        raw_wid = CEIL_BY(CEIL_DOWN(raw_wid, 2), 2);
    }

}

static int AiynrGetKwtMaxSize(char *model_buf, uint32_t *kwt_size)
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

static int AiynrGetParamInfo(AiynrManager_t* pAiynrManager, AiynrModelBuf_t *pAiynrModelBuf, struct rkaiisp_param_info *param_info)
{
    uint32_t kwt_size, max_size;

    max_size = 0;
    for (int i = 0; i < AIYNR_ISO_STEP_MAX; i++) {
        if (AiynrGetKwtMaxSize(pAiynrModelBuf->model_buf[i], &kwt_size) != 0) {
            LOGE_AIYNR("%s: get param info(%s) failed!", __func__, pAiynrModelBuf->model_file[i]);
        }

        if (kwt_size > max_size)
            max_size = kwt_size;
    }

    param_info->exealgo    = AIYNR;
    param_info->exemode    = AIYNR_AIISP_MODE;
    param_info->max_runcnt = pAiynrManager->model_max_runcnt;
    param_info->para_size  = max_size + sizeof(struct rkaiisp_params);

    return 0;
}

static void AiynrParamsPrint(struct rkaiisp_params *aiisp_cfg)
{
    struct rkaiisp_other_cfg *other_cfg = &aiisp_cfg->other_cfg;
    struct rkaiisp_kwt_cfg *kwt_cfg = &aiisp_cfg->kwt_cfg;
    struct rkaiisp_model_cfg *model_cfg;
    u32 i, j;

    LOGD_AIYNR("aiisp: frame_id %d, module_update = 0x%x", aiisp_cfg->frame_id, aiisp_cfg->module_update);
    if (aiisp_cfg->module_update & RKAIISP_OTHER_UPDATE) {
        LOGD_AIYNR("sw_neg_noiselimit = 0x%x", other_cfg->sw_neg_noiselimit);
        LOGD_AIYNR("sw_pos_noiselimit = 0x%x", other_cfg->sw_pos_noiselimit);
        LOGD_AIYNR("sw_prev_blacklvl = 0x%x", other_cfg->sw_prev_blacklvl);
        LOGD_AIYNR("sw_post_blacklvl = 0x%x", other_cfg->sw_post_blacklvl);

        /*for (i = 0; i < 33; i++) {
            LOGD_AIYNR("sw_in_comp_y[%d] = 0x%x", i, other_cfg->sw_in_comp_y[i]);
        }*/
        for (i = 0; i < 33; i++) {
            LOGD_AIYNR("sw_out_decomp_y[%d] = 0x%x", i, other_cfg->sw_out_decomp_y[i]);
        }
    }

    if (aiisp_cfg->module_update & RKAIISP_MODEL_UPDATE) {
        for (i = 0; i < aiisp_cfg->model_runcnt; i++) {
            model_cfg = (struct rkaiisp_model_cfg *)&aiisp_cfg->model_cfg[i];

            LOGD_AIYNR("model_cfg %d:", i);
            for (j = 0; j < 7; j++) {
                LOGD_AIYNR("mi_chn_src[%d] = 0x%x", j, model_cfg->mi_chn_src[j]);
            }
            LOGD_AIYNR("sw_aiisp_mode = %d", model_cfg->sw_aiisp_mode);
            LOGD_AIYNR("sw_aiisp_level_num = %d", model_cfg->sw_aiisp_level_num);
            LOGD_AIYNR("sw_aiisp_l1_level_num = %d", model_cfg->sw_aiisp_l1_level_num);
            LOGD_AIYNR("sw_aiisp_op_mode = %d", model_cfg->sw_aiisp_op_mode);
            LOGD_AIYNR("sw_aiisp_drop_en = %d", model_cfg->sw_aiisp_drop_en);
            for (j = 0; j < 16; j++) {
                LOGD_AIYNR("sw_aiisp_lv_active[%d] = 0x%x", j, model_cfg->sw_aiisp_lv_active[j]);
            }
            for (j = 0; j < 16; j++) {
                LOGD_AIYNR("sw_aiisp_lv_mode[%d] = 0x%x", j, model_cfg->sw_aiisp_lv_mode[j]);
            }
            for (j = 0; j < 7; j++) {
                LOGD_AIYNR("sw_mi_chn_en[%d] = 0x%x", j, model_cfg->sw_mi_chn_en[j]);
            }
            for (j = 0; j < 7; j++) {
                LOGD_AIYNR("sw_mi_chn_mode[%d] = 0x%x", j, model_cfg->sw_mi_chn_mode[j]);
            }
            for (j = 0; j < 7; j++) {
                LOGD_AIYNR("sw_mi_chn_num[%d] = 0x%x", j, model_cfg->sw_mi_chn_num[j]);
            }
            for (j = 0; j < 7; j++) {
                LOGD_AIYNR("sw_mi_chn_data_mode[%d] = 0x%x", j, model_cfg->sw_mi_chn_data_mode[j]);
            }
            LOGD_AIYNR("sw_mi_chn1_sel = %d", model_cfg->sw_mi_chn1_sel);
            LOGD_AIYNR("sw_mi_chn3_sel = %d", model_cfg->sw_mi_chn3_sel);
            LOGD_AIYNR("sw_out_d2s_en = %d", model_cfg->sw_out_d2s_en);
            LOGD_AIYNR("sw_out_mode = %d", model_cfg->sw_out_mode);
            LOGD_AIYNR("sw_lastlvlm1_clip8bit = %d", model_cfg->sw_lastlvlm1_clip8bit);
        }

        for (i = 0; i < RKAIISP_MAX_RUNCNT; i++) {
            LOGD_AIYNR("kwt_offet[%d] = 0x%x", i, kwt_cfg->kwt_offet[i]);
        }
        for (i = 0; i < RKAIISP_MAX_RUNCNT; i++) {
            LOGD_AIYNR("kwt_size[%d] = 0x%x", i, kwt_cfg->kwt_size[i]);
        }
        for (i = 0; i < RKAIISP_MAX_RUNCNT; i++) {
            LOGD_AIYNR("kwt_pad_size[%d] = 0x%x", i, kwt_cfg->kwt_pad_size[i]);
        }
    }
}

static int AiynrCheckModelSum(char *model_file, char *model_buf, size_t file_len)
{
    uint32_t* pTmp = (unsigned int*)model_buf;
    uint32_t i, checksum, realsum;

    realsum = 0;
    checksum = *pTmp++;
    for (i = 1; i < file_len / 4; i++) {
        realsum += *pTmp++;
    }

    if (checksum != realsum) {
        LOGE_AIYNR("model(%s) sum check error 0x%x, 0x%x", model_file, checksum, realsum);
        return -1;
    }

    return 0;
}

static int AiynrCheckModelCfg(struct rkaiisp_model_cfg *model_cfg)
{
    uint32_t i;

    if (model_cfg->sw_aiisp_mode > 1) {
        LOGE_AIYNR("%s: sw_aiisp_mode %d", __func__, model_cfg->sw_aiisp_mode);
        return -1;
    }
    if (model_cfg->sw_aiisp_op_mode > 1) {
        LOGE_AIYNR("%s: sw_aiisp_op_mode %d", __func__, model_cfg->sw_aiisp_op_mode);
        return -1;
    }
    if (model_cfg->sw_out_mode > 2) {
        LOGE_AIYNR("%s: sw_out_mode %d", __func__, model_cfg->sw_out_mode);
        return -1;
    }
    if (model_cfg->sw_aiisp_level_num > 16) {
        LOGE_AIYNR("%s: sw_aiisp_level_num %d", __func__, model_cfg->sw_aiisp_level_num);
        return -1;
    }

    for (i = 0; i < 7; i++) {
        if (model_cfg->sw_mi_chn_mode[i] > 2) {
            LOGE_AIYNR("%s: sw_mi_chn_mode %d", __func__, model_cfg->sw_mi_chn_mode[i]);
            return -1;
        }
        if (model_cfg->sw_mi_chn_data_mode[i] > 2) {
            LOGE_AIYNR("%s: sw_mi_chn_data_mode %d", __func__, model_cfg->sw_mi_chn_data_mode[i]);
            return -1;
        }
    }

    for (i = 0; i < 16; i++) {
        if (model_cfg->sw_aiisp_lv_mode[i] > 4) {
            LOGE_AIYNR("%s: sw_aiisp_lv_mode %d", __func__, model_cfg->sw_aiisp_lv_mode[i]);
            return -1;
        }
    }

    return 0;
}

static XCamReturn AiynrReadModel(AiynrManager_t* pAiynrManager, CamCalibDbV2Context_t* pCalibDbV2,
                                 AiynrModelBuf_t* aiynrModelBuf, RkAiqAiynrModelInfo_t* aiynrModelInfo)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiynr_api_attrib_t *aiynr_attrib = (aiynr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, aiynr));
    aiynr_params_static_t* psta = &(aiynr_attrib->stAuto.sta);
    uint32_t model_max_runcnt = 0;

    if (!pAiynrManager->is_enable)
        return XCAM_RETURN_BYPASS;

    memset(aiynrModelBuf, 0, sizeof(*aiynrModelBuf));
    memset(aiynrModelInfo, 0, sizeof(*aiynrModelInfo));
    for (int i = 0; i < AIYNR_ISO_STEP_MAX; i++) {
        aiynr_param_dyn_t* pdyn = &(aiynr_attrib->stAuto.dyn[i]);
        aiynr_param_tunning_t *tunning = &pdyn->tunning;
        struct rkaiisp_model_info model_info;
        size_t file_size, correct_size, runcnt;
        struct rkaiisp_kwt_cfg *kwt_cfg;
        char model_file[256];
        FILE *fp;

        sprintf(model_file, "%s/%s", psta->model_dir.sw_aiYnrCfg_model_dir, tunning->sw_aiYnrT_model_file);
        fp = fopen(model_file, "rb");
        if (fp) {
            if (fread(&model_info, sizeof(model_info), 1, fp) != 1) {
                LOGE_AIYNR("%s(%d): %s: file head is error\n",
                           __FUNCTION__, __LINE__, model_file);
                return XCAM_RETURN_ERROR_FILE;
            }
            fseek(fp, 0, 2);
            file_size = ftell(fp);

            runcnt = model_info.model_runcnt;
            kwt_cfg = &model_info.kwt_cfg;
            /*if (runcnt != 3 && runcnt != 4 && runcnt != 8) {
                LOGE_AIYNR("%s(%d): %s: runcnt(%d) is error\n",
                           __FUNCTION__, __LINE__, model_file, runcnt);
                return XCAM_RETURN_ERROR_FILE;
            }*/

            if (runcnt > model_max_runcnt)
                model_max_runcnt = runcnt;

            correct_size = kwt_cfg->kwt_offet[runcnt - 1] + kwt_cfg->kwt_pad_size[runcnt - 1];
            if (file_size != correct_size) {
                LOGE_AIYNR("%s(%d): %s: file size(%d, %d) is error\n",
                           __FUNCTION__, __LINE__, model_file, file_size, correct_size);
                return XCAM_RETURN_ERROR_FILE;
            }

            for (uint32_t j = 0; j < model_info.model_runcnt; j++) {
                if (AiynrCheckModelCfg(&model_info.model_cfg[j]) != 0) {
                    LOGE_AIYNR("%s(%d): %s: model info check failure\n",
                               __FUNCTION__, __LINE__, model_file);
                    return XCAM_RETURN_ERROR_FILE;
                }
            }

            aiynrModelBuf->model_buf[i] = aiq_malloc(file_size);
            if (aiynrModelBuf->model_buf[i]) {
                fseek(fp, 0, 0);
                fread(aiynrModelBuf->model_buf[i], 1, file_size, fp);
                fclose(fp);
                strcpy(aiynrModelBuf->model_file[i], model_file);
                if (AiynrCheckModelSum(aiynrModelBuf->model_file[i], aiynrModelBuf->model_buf[i], file_size) != 0) {
                    aiq_free(aiynrModelBuf->model_buf[i]);
                    aiynrModelBuf->model_buf[i] = NULL;
                }
            }
        } else {
            LOGE_AIYNR("read aiisp model file(%s) failed !", model_file);
        }
    }

    aiynrModelBuf->valid = true;
    for (int i = 0; i < AIYNR_ISO_STEP_MAX; i++) {
        if (!aiynrModelBuf->model_buf[i])
            aiynrModelBuf->valid = false;
    }

    struct rkaiisp_model_info *model_info;
    if (aiynrModelBuf->valid) {
        for (int i = 0; i < AIYNR_ISO_STEP_MAX; i++) {
            model_info = (struct rkaiisp_model_info *)aiynrModelBuf->model_buf[i];
            strcpy(aiynrModelInfo->model_file[i], aiynrModelBuf->model_file[i]);
            aiynrModelInfo->quant_val[i] = model_info->model_qr;
            // model_info->model_cfg[j].sw_aiisp_mode = 0;
            // model_info->model_cfg[j].sw_out_mode = 2;
        }
    }

    pAiynrManager->model_max_runcnt = model_max_runcnt;

    return ret;
}

XCamReturn AiynrManager_setCalib(AiynrManager_t* pAiynrManager, CamCalibDbV2Context_t* pCalibDbV2)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiynrModelBuf_t *aiynrModelBuf = &pAiynrManager->mAiynrModelBuf;
    struct rkaiisp_param_info param_info;
    AiynrModelBuf_t tmpAiynrModelBuf;
    RkAiqAiynrModelInfo_t tmpAiynrModelInfo;

    pAiynrManager->pCalibDbV2 = pCalibDbV2;

    if (!pAiynrManager->is_enable) {
        LOGD_AIYNR("%s: aiynr is disabled.", __func__);
        return XCAM_RETURN_BYPASS;
    }

    AiynrReadModel(pAiynrManager, pCalibDbV2, &tmpAiynrModelBuf, &tmpAiynrModelInfo);
    if (!tmpAiynrModelBuf.valid) {
        LOGE_AIYNR("%s: readModel is error.", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }
    AiynrGetParamInfo(pAiynrManager, &tmpAiynrModelBuf, &param_info);

    if (param_info.para_size > pAiynrManager->param_info.para_size) {
        LOGE_AIYNR("para_size(%d) in new calib is too big!", param_info.para_size, pAiynrManager->param_info.para_size);
        return XCAM_RETURN_ERROR_FAILED;
    }

    aiqMutex_lock(&pAiynrManager->model_buf_mutex);
    for (int i = 0; i < AIYNR_ISO_STEP_MAX; i++) {
        if (aiynrModelBuf->model_buf[i] != NULL) {
            aiq_free(aiynrModelBuf->model_buf[i]);
            aiynrModelBuf->model_buf[i] = NULL;
        }
    }
    *aiynrModelBuf = tmpAiynrModelBuf;
    pAiynrManager->mAiynrModelInfo = tmpAiynrModelInfo;
    pAiynrManager->param_info = param_info;
    pAiynrManager->pCamHw->aiynr_setModelInf(pAiynrManager->pCamHw, &pAiynrManager->mAiynrModelInfo);
    aiqMutex_unlock(&pAiynrManager->model_buf_mutex);

    return ret;
}

XCamReturn AiynrManager_prepare(AiynrManager_t* pAiynrManager, AiqCamHwBase_t* pCamHw, CamCalibDbV2Context_t* pCalibDbV2,
                                uint32_t isp_acq_width, uint32_t isp_acq_height)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiynr_api_attrib_t *aiynr_attrib = (aiynr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, aiynr));
    struct rkaiisp_ispbuf_info *ispbuf_info = &pAiynrManager->ispbuf_info;
    AiynrModelBuf_t *aiynrModelBuf = &pAiynrManager->mAiynrModelBuf;
    struct rkaiisp_param_info param_info;
    RkAiqAiynrModelInfo_t tmpAiynrModelInfo;
    AiynrModelBuf_t tmpAiynrModelBuf;

    pAiynrManager->is_enable       = aiynr_attrib->en && pAiynrManager->is_aiynr_enable;
    pAiynrManager->is_param_update = false;
    pAiynrManager->pCamHw          = pCamHw;
    pAiynrManager->pCalibDbV2      = pCalibDbV2;
    pAiynrManager->isp_acq_width   = isp_acq_width;
    pAiynrManager->isp_acq_height  = isp_acq_height;
    pAiynrManager->is_bypass = aiynr_attrib->bypass;
    pAiynrManager->is_state_error  = false;

    if (!pAiynrManager->is_enable) {
        LOGD_AIYNR("%s: aiynr is disabled.", __func__);
        return XCAM_RETURN_BYPASS;
    }

    memset(&pAiynrManager->last_aiynr_params, 0, sizeof(pAiynrManager->last_aiynr_params));
    AiynrReadModel(pAiynrManager, pCalibDbV2, &tmpAiynrModelBuf, &tmpAiynrModelInfo);
    if (!tmpAiynrModelBuf.valid) {
        LOGE_AIYNR("%s: readModel is error.", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }
    AiynrGetParamInfo(pAiynrManager, &tmpAiynrModelBuf, &param_info);

    aiqMutex_lock(&pAiynrManager->model_buf_mutex);
    for (int i = 0; i < AIYNR_ISO_STEP_MAX; i++) {
        if (aiynrModelBuf->model_buf[i] != NULL) {
            aiq_free(aiynrModelBuf->model_buf[i]);
            aiynrModelBuf->model_buf[i] = NULL;
        }
    }
    *aiynrModelBuf = tmpAiynrModelBuf;
    pAiynrManager->mAiynrModelInfo = tmpAiynrModelInfo;
    pAiynrManager->param_info = param_info;
    aiqMutex_unlock(&pAiynrManager->model_buf_mutex);

    pAiynrManager->mAiynrCfg.mode = 0;
    pAiynrManager->mAiynrCfg.wr_linecnt = isp_acq_height;
    pAiynrManager->mAiynrCfg.rd_linecnt = 0;
    pAiynrManager->mAiynrCfg.wr_mode = 0;
    ret = pAiynrManager->pCamHw->aiynr_init(pAiynrManager->pCamHw, pAiynrManager, &pAiynrManager->mAiynrCfg);
    if (ret) {
        LOGE_AIYNR("%s: can not init aiynr.", __func__);
        pAiynrManager->is_state_error = true;
    }
    pAiynrManager->pCamHw->aiynr_setModelInf(pAiynrManager->pCamHw, &pAiynrManager->mAiynrModelInfo);
    pAiynrManager->pCamHw->aiynr_setParamInf(pAiynrManager->pCamHw, &pAiynrManager->param_info);

    pAiynrManager->last_isp_out.iir_index =  -1;
    pAiynrManager->last_isp_out.gain_index =  -1;
    pAiynrManager->last_isp_out.aipre_gain_index = -1;
    pAiynrManager->last_isp_out.vpsl_index =  -1;
    pAiynrManager->last_isp_out.aiisp_index = -1;

    for (int i = 0; i < AIYNR_AIISP_BUF_CNT; i++)
        pAiynrManager->aiisp_idx_pool[i] = false;

    LOGD_AIYNR("iir_width %d, iir_height %d", isp_acq_width, isp_acq_height);
    AiynrCalcIspBufSize(ispbuf_info, isp_acq_width, isp_acq_height);
    pAiynrManager->is_parambuf_prepare = false;

    return ret;
}

static XCamReturn AiynrMmapIspbuf(struct rkisp_bnr_buf_info *ynr_buf, AiynrBufAddr_t *ispbuf_addr)
{
    struct rkisp_buf_info *aipre_inf, *vpsl_inf, *ysrc_inf;
    char *aipre_addr, *vpsl_addr, *ysrc_addr;

    for (int i = 0; i < AIYNR_ISP_BUF_MAX; i++) {
        ispbuf_addr[i].aipre_addr = NULL;
        ispbuf_addr[i].vpsl_addr = NULL;
        ispbuf_addr[i].ysrc_addr = NULL;

        if (i < AIYNR_AIPRE_BUF_CNT) {
            aipre_inf = &ynr_buf->u.v35.vpsl;
            aipre_addr = (char*)mmap(NULL, aipre_inf->buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, aipre_inf->buf_fd[i], 0);
            if (MAP_FAILED == aipre_addr) {
                LOGE_AIYNR("aiisp fd %d, size %d mmap failed", aipre_inf->buf_fd[i], aipre_inf->buf_size);
            } else {
                ispbuf_addr[i].aipre_addr = aipre_addr;
            }
        }

        if (i < AIYNR_GAIN_BUF_CNT) {
            vpsl_inf = &ynr_buf->u.v35.y_src;
            vpsl_addr = (char*)mmap(NULL, vpsl_inf->buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, vpsl_inf->buf_fd[i], 0);
            if (MAP_FAILED == vpsl_addr) {
                LOGE_AIYNR("gain fd %d, size %d mmap failed", vpsl_inf->buf_fd[i], vpsl_inf->buf_size);
            } else {
                ispbuf_addr[i].vpsl_addr = vpsl_addr;
            }
        }

        if (i < AIYNR_AIISP_BUF_CNT) {
            ysrc_inf = &ynr_buf->u.v35.y_src;
            ysrc_addr = (char*)mmap(NULL, ysrc_inf->buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, ysrc_inf->buf_fd[i], 0);
            if (MAP_FAILED == ysrc_addr) {
                LOGE_AIYNR("aiisp fd %d, size %d mmap failed", ysrc_inf->buf_fd[i], ysrc_inf->buf_size);
            } else {
                ispbuf_addr[i].ysrc_addr = ysrc_addr;
            }
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiynrMunmapIspbuf(struct rkisp_bnr_buf_info *ynr_buf, AiynrBufAddr_t *ispbuf_addr)
{
    struct rkisp_buf_info *aipre_inf, *vpsl_inf, *ysrc_inf;

    for (int i = 0; i < AIYNR_ISP_BUF_MAX; i++) {
        if (ispbuf_addr[i].aipre_addr != NULL) {
            aipre_inf = &ynr_buf->u.v35.aipre_gain;
            munmap(ispbuf_addr[i].aipre_addr, aipre_inf->buf_size);
            ispbuf_addr[i].aipre_addr = NULL;
        }
        if (ispbuf_addr[i].vpsl_addr != NULL) {
            vpsl_inf = &ynr_buf->u.v35.vpsl;
            munmap(ispbuf_addr[i].vpsl_addr, vpsl_inf->buf_size);
            ispbuf_addr[i].vpsl_addr = NULL;
        }
        if (ispbuf_addr[i].ysrc_addr != NULL) {
            ysrc_inf = &ynr_buf->u.v35.y_src;
            munmap(ispbuf_addr[i].ysrc_addr, ysrc_inf->buf_size);
            ispbuf_addr[i].ysrc_addr = NULL;
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiynrManager_updateParams(AiynrManager_t* pAiynrManager, rk_aiq_isp_aiynr_params_t *isp_aiynr_params, uint32_t frame_id)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqV4l2Buffer_t* pV4l2Buf = NULL;

    if (!pAiynrManager->is_enable) {
        LOGD_AIYNR("%s: aiynr is disabled.", __func__);
        return XCAM_RETURN_BYPASS;
    }

    if (pAiynrManager->is_state_error) {
        LOGE_AIYNR("%s: aiynr in error state.", __func__);
        return XCAM_RETURN_BYPASS;
    }

    if ((pAiynrManager->is_bypass == isp_aiynr_params->is_bypass) && isp_aiynr_params->is_bypass && pAiynrManager->is_parambuf_prepare) {
        LOGD_AIYNR("%s: aiynr is bypass.", __func__);
        return XCAM_RETURN_BYPASS;
    }
    pAiynrManager->is_bypass = isp_aiynr_params->is_bypass;

    if (!pAiynrManager->is_parambuf_prepare) {
        ret = pAiynrManager->pCamHw->aiynr_init(pAiynrManager->pCamHw, pAiynrManager, &pAiynrManager->mAiynrCfg);
        if (ret) {
            pAiynrManager->is_state_error = true;
            LOGE_AIYNR("%s: can not init aiynr.", __func__);
            return ret;
        }
        pAiynrManager->pCamHw->aiynr_setIspBufInf(pAiynrManager->pCamHw, &pAiynrManager->ispbuf_info);
        ret = pAiynrManager->pCamHw->aiynr_prepare(pAiynrManager->pCamHw);
        if (ret) {
            pAiynrManager->is_state_error = true;
            LOGE_AIYNR("%s: can not prepare aiynr.", __func__);
            return ret;
        }
        //AiynrMmapIspbuf(&pAiynrManager->ispbuf_info.bnr_buf, pAiynrManager->ispbuf_addr);
        pAiynrManager->is_parambuf_prepare = true;
    }
    pAiynrManager->pCamHw->aiynr_getParamsBuf(pAiynrManager->pCamHw, &pV4l2Buf);
    if (pV4l2Buf) {
        struct rkaiisp_params* aiisp_cfg = (struct rkaiisp_params*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);
        struct rkaiisp_other_cfg *other_cfg = &aiisp_cfg->other_cfg;
        aiynr_params_static_t *sta = &isp_aiynr_params->aiynr_param.sta;
        aiynr_param_tunning_t *tunning = &isp_aiynr_params->aiynr_param.dyn.tunning;
        char model_file[256];
        char used_model_file[256];

        aiisp_cfg->frame_id = frame_id;
        aiisp_cfg->module_update = RKAIISP_OTHER_UPDATE;
        other_cfg->sw_prev_blacklvl  = 0;
        other_cfg->sw_post_blacklvl  = 0;

        other_cfg->sw_pos_noiselimit = isp_aiynr_params->sw_pos_noiselimit;
        other_cfg->sw_neg_noiselimit = isp_aiynr_params->sw_neg_noiselimit;
        for (int i = 0; i < 33; i++) {
            other_cfg->sw_in_comp_y[i] = isp_aiynr_params->sw_in_comp_y[i];
            other_cfg->sw_out_decomp_y[i] = isp_aiynr_params->sw_out_decomp_y[i];
        }

        sprintf(model_file, "%s/%s", sta->model_dir.sw_aiYnrCfg_model_dir, tunning->sw_aiYnrT_model_file);
        sprintf(used_model_file, "%s/%s", sta->model_dir.sw_aiYnrCfg_model_dir, pAiynrManager->last_aiynr_params.aiynr_param.dyn.tunning.sw_aiYnrT_model_file);
        if (strcmp((char *)model_file, used_model_file) != 0) {
            struct rkaiisp_model_info *model_info;
            int i;

            aiqMutex_lock(&pAiynrManager->model_buf_mutex);
            for (i = 0; i < AIYNR_ISO_STEP_MAX; i++) {
                if (strcmp(pAiynrManager->mAiynrModelBuf.model_file[i], model_file) == 0) {
                    model_info = (struct rkaiisp_model_info *)pAiynrManager->mAiynrModelBuf.model_buf[i];
                    break;
                }
            }

            if (i >= AIYNR_ISO_STEP_MAX) {
                LOGE_AIYNR("can not find model file %s", model_file);
                aiqMutex_unlock(&pAiynrManager->model_buf_mutex);
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

            if (aiisp_cfg->model_cfg[0].sw_aiisp_mode == 0 && aiisp_cfg->model_cfg[0].sw_out_mode == 2) {
                // sw_m0_diff_merge = 1 in aiisp driver
                if (isp_aiynr_params->sw_nar_inverse == true)
                    LOGE_AIYNR("sw_nar_inverse error config(1), to check");
            } else {
                // sw_m0_diff_merge = 0 in aiisp driver
                if (isp_aiynr_params->sw_nar_inverse == false)
                    LOGE_AIYNR("sw_nar_inverse error config(0), to check");
            }

            aiqMutex_unlock(&pAiynrManager->model_buf_mutex);

            aiisp_cfg->module_update |= RKAIISP_MODEL_UPDATE;
        }

        AiynrParamsPrint(aiisp_cfg);

        aiqMutex_lock(&pAiynrManager->iq_param_mutex);
        pAiynrManager->last_aiynr_params = *isp_aiynr_params;
        aiqMutex_unlock(&pAiynrManager->iq_param_mutex);
        pAiynrManager->is_param_update = true;
        pAiynrManager->pCamHw->aiynr_updateParams(pAiynrManager->pCamHw, pV4l2Buf);
    } else {
        LOGE_AIYNR("%s: Can not get param buffer", __func__);
    }

    return ret;
}

XCamReturn AiynrManager_start(AiynrManager_t* pAiynrManager)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!pAiynrManager->is_enable) {
        LOGD_AIYNR("%s: aiynr is disabled.", __func__);
        return XCAM_RETURN_BYPASS;
    }

    pAiynrManager->is_start = true;
    ret = pAiynrManager->pCamHw->aiynr_start(pAiynrManager->pCamHw);
    if (ret) {
        pAiynrManager->is_state_error = true;
        LOGE_AIYNR("%s: can not start aiynr.", __func__);
    }

    return ret;
}

XCamReturn AiynrManager_stop(AiynrManager_t* pAiynrManager)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!pAiynrManager->is_enable) {
        LOGD_AIYNR("%s: aiynr is disabled.", __func__);
        return XCAM_RETURN_BYPASS;
    }

    if (pAiynrManager->is_get_ynrbuf) {
        for (int i = 0; i < pAiynrManager->ynrbuf_info.buf_cnt; i++) {
            close(pAiynrManager->ynrbuf_info.dma_fd[i]);
        }
        pAiynrManager->is_get_ynrbuf = false;
    }

    ret = pAiynrManager->pCamHw->aiynr_stop(pAiynrManager->pCamHw);
    if (ret)
        LOGE_AIYNR("%s: can not stop aiynr.", __func__);

    pAiynrManager->is_start = false;
    pAiynrManager->is_parambuf_prepare = false;
    memset(&pAiynrManager->last_aiynr_params, 0, sizeof(pAiynrManager->last_aiynr_params));
    for (int i = 0; i < AIYNR_AIISP_BUF_CNT; i++) {
        pAiynrManager->aiisp_idx_pool[i] = false;
    }
    //AiynrMunmapIspbuf(&pAiynrManager->ispbuf_info.bnr_buf, pAiynrManager->ispbuf_addr);

    return ret;
}

XCamReturn AiynrManager_deinit(AiynrManager_t* pAiynrManager)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiynrModelBuf_t *aiynrModelBuf = &pAiynrManager->mAiynrModelBuf;

    for (int i = 0; i < AIYNR_ISO_STEP_MAX; i++) {
        if (aiynrModelBuf->model_buf[i] != NULL) {
            aiq_free(aiynrModelBuf->model_buf[i]);
            aiynrModelBuf->model_buf[i] = NULL;
        }
    }
    aiqMutex_deInit(&pAiynrManager->idx_pool_mutex);
    aiqMutex_deInit(&pAiynrManager->iq_param_mutex);
    aiqMutex_deInit(&pAiynrManager->model_buf_mutex);

    return ret;
}

static XCamReturn AiynrDumpDmabuf(char *filename, u32 dma_fd, u32 size)
{
    FILE *fp;
    char *dump_addr;
    struct dma_buf_sync sync = { 0 };

    dump_addr = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, dma_fd, 0);
    if (MAP_FAILED == dump_addr) {
        LOGE_AIYNR("fd %d, size %d mmap failed", dma_fd, size);
        return XCAM_RETURN_ERROR_FAILED;
    }

    sync.flags = DMA_BUF_SYNC_READ | DMA_BUF_SYNC_START;
    ioctl(dma_fd, DMA_BUF_IOCTL_SYNC, &sync);

    fp = fopen(filename, "wb");
    if (fp) {
        LOGK_AIYNR("dump buffer to filename %s", filename);
        fwrite(dump_addr, 1, size, fp);
        fclose(fp);
    } else {
        LOGE_AIYNR("can not open file %s", filename);
    }

    sync.flags = DMA_BUF_SYNC_READ | DMA_BUF_SYNC_END;
    ioctl(dma_fd, DMA_BUF_IOCTL_SYNC, &sync);

    munmap(dump_addr, size);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiynrWriteFile(char *filename, char *addr, u32 size)
{
    FILE *fp;

    fp = fopen(filename, "wb");
    if (fp) {
        LOGD_AIYNR("dump buffer to filename %s", filename);
        fwrite(addr, 1, size, fp);
        fclose(fp);
    } else {
        LOGE_AIYNR("can not open file %s", filename);
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiynrChangeOutBuf(AiynrManager_t* pAiynrManager, u32 dma_fd, u32 size,
                                        aiynr_debug_mode_t sw_aiYnrT_debug_mode)
{
    uint32_t isp_acq_width  = pAiynrManager->isp_acq_width;
    uint32_t isp_acq_height = pAiynrManager->isp_acq_height;
    char *dump_addr;
    struct dma_buf_sync sync = { 0 };

    dump_addr = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, dma_fd, 0);
    if (MAP_FAILED == dump_addr) {
        LOGE_AIYNR("fd %d, size %d mmap failed", dma_fd, size);
        return XCAM_RETURN_ERROR_FAILED;
    }

    sync.flags = DMA_BUF_SYNC_WRITE | DMA_BUF_SYNC_START;
    ioctl(dma_fd, DMA_BUF_IOCTL_SYNC, &sync);

    if (sw_aiYnrT_debug_mode == aiynr_debugYsrc_mode) {
        for (int i = 0; i < isp_acq_height / 4; i++) {
            for (int j = 0; j < isp_acq_width / 4; j++) {
                dump_addr[i * isp_acq_width + j] = 255;
            }
        }

    } else if (sw_aiYnrT_debug_mode == aiynr_debugYdest_mode) {
        for (int i = isp_acq_height * 3 / 4; i < isp_acq_height; i++) {
            for (int j = isp_acq_width * 3 / 4; j < isp_acq_width; j++) {
                dump_addr[i * isp_acq_width + j] = 128;
            }
        }
    }

    sync.flags = DMA_BUF_SYNC_WRITE | DMA_BUF_SYNC_END;
    ioctl(dma_fd, DMA_BUF_IOCTL_SYNC, &sync);

    munmap(dump_addr, size);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiynrManager_doDumpRaw(AiynrManager_t* pAiynrManager, struct rkisp_aiisp_st *isp_in)
{
    struct rkisp_bnr_buf_info *ynr_buf = &pAiynrManager->ispbuf_info.bnr_buf;
    rk_aiq_isp_aiynr_params_t *isp_aiynr_params;
    struct rkaiisp_model_info *model_info;
    struct rkisp_buf_info *buf_inf;
    char dump_file[256];
    u32 frame_id, idx;
    bool dump_flag = false;
    u32 i;

    if (pAiynrManager->dump_raw_num >= -1 && pAiynrManager->dump_raw_num != 0) {
        dump_flag = true;
        if (pAiynrManager->dump_raw_num > -1)
            pAiynrManager->dump_raw_num--;
    }

    if (dump_flag) {
        frame_id = isp_in->sequence;
        LOGD_AIYNR("aiynr dump frame %d, dump_raw_num %d", frame_id, pAiynrManager->dump_raw_num);

        idx = isp_in->aipre_gain_index;
        buf_inf = &ynr_buf->u.v35.aipre_gain;
        sprintf(dump_file, "%s/%05d_aipre.raw", AIYNR_DUMPBUF_DIR, frame_id);
        AiynrDumpDmabuf(dump_file, buf_inf->buf_fd[idx], buf_inf->buf_size);

        idx = isp_in->vpsl_index;
        buf_inf = &ynr_buf->u.v35.vpsl;
        sprintf(dump_file, "%s/%05d_yraw-%d-%d_%d-%d_%d-%d_%d-%d_%d-%d_%d-%d_sig-%d-%d_%d-%d_%d-%d_%d-%d_%d-%d_vpsl.raw",
                AIYNR_DUMPBUF_DIR, frame_id,
                ynr_buf->u.v35.vpsl_yraw_offs[0],
                ynr_buf->u.v35.vpsl_yraw_stride[0],
                ynr_buf->u.v35.vpsl_yraw_offs[1],
                ynr_buf->u.v35.vpsl_yraw_stride[1],
                ynr_buf->u.v35.vpsl_yraw_offs[2],
                ynr_buf->u.v35.vpsl_yraw_stride[2],
                ynr_buf->u.v35.vpsl_yraw_offs[3],
                ynr_buf->u.v35.vpsl_yraw_stride[3],
                ynr_buf->u.v35.vpsl_yraw_offs[4],
                ynr_buf->u.v35.vpsl_yraw_stride[4],
                ynr_buf->u.v35.vpsl_yraw_offs[5],
                ynr_buf->u.v35.vpsl_yraw_stride[5],
                ynr_buf->u.v35.vpsl_sig_offs[0],
                ynr_buf->u.v35.vpsl_sig_stride[0],
                ynr_buf->u.v35.vpsl_sig_offs[1],
                ynr_buf->u.v35.vpsl_sig_stride[1],
                ynr_buf->u.v35.vpsl_sig_offs[2],
                ynr_buf->u.v35.vpsl_sig_stride[2],
                ynr_buf->u.v35.vpsl_sig_offs[3],
                ynr_buf->u.v35.vpsl_sig_stride[3],
                ynr_buf->u.v35.vpsl_sig_offs[4],
                ynr_buf->u.v35.vpsl_sig_stride[4]);
        AiynrDumpDmabuf(dump_file, buf_inf->buf_fd[idx], buf_inf->buf_size);

        idx = isp_in->y_src_index;
        buf_inf = &ynr_buf->u.v35.y_src;
        sprintf(dump_file, "%s/%05d_y_src.raw", AIYNR_DUMPBUF_DIR, frame_id);
        AiynrDumpDmabuf(dump_file, buf_inf->buf_fd[idx], buf_inf->buf_size);

#if 0
        aiqMutex_lock(&pAiynrManager->iq_param_mutex);
        isp_aiynr_params = &pAiynrManager->last_aiynr_params;
        sprintf(dump_file, "%s/%s",
                isp_aiynr_params->aiynr_param.sta.model_dir.sw_aiYnrCfg_model_dir,
                isp_aiynr_params->aiynr_param.dyn.tunning.sw_aiYnrT_model_file);
        aiqMutex_unlock(&pAiynrManager->iq_param_mutex);

        aiqMutex_lock(&pAiynrManager->model_buf_mutex);
        for (i = 0; i < AIYNR_ISO_STEP_MAX; i++) {
            if (strcmp(pAiynrManager->mAiynrModelBuf.model_file[i], dump_file) == 0) {
                model_info = (struct rkaiisp_model_info *)pAiynrManager->mAiynrModelBuf.model_buf[i];
                break;
            }
        }

        if (i >= AIYNR_ISO_STEP_MAX) {
            LOGE_AIYNR("can not find model file %s in dumpbuf", dump_file);
        } else {
            for (i = 0; i < model_info->model_runcnt; i++) {
                char *pKwtBuffer = (char *)model_info + model_info->kwt_cfg.kwt_offet[i];

                sprintf(dump_file, "%s/%04d_%02d_%01d_%04d_kwt.bin",
                        AIYNR_DUMPBUF_DIR, frame_id, idx, i,
                        model_info->kwt_cfg.kwt_pad_size[i]);
                AiynrWriteFile(dump_file, pKwtBuffer, model_info->kwt_cfg.kwt_pad_size[i]);
            }
        }
        aiqMutex_unlock(&pAiynrManager->model_buf_mutex);
#endif
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiynrManager_doIspBe(AiynrManager_t* pAiynrManager, struct rkisp_aiisp_st *isp_in)
{
    struct rkisp_bnr_buf_info *ynr_buf = &pAiynrManager->ispbuf_info.bnr_buf;
    struct rkaiisp_ynrbuf_info *ynrbuf_info = &pAiynrManager->ynrbuf_info;
    struct rkisp_buf_info *buf_inf;
    u32 idx;

    AiynrManager_doDumpRaw(pAiynrManager, isp_in);

    aiqMutex_lock(&pAiynrManager->iq_param_mutex);
    aiynr_debug_mode_t sw_aiYnrT_debug_mode = pAiynrManager->last_aiynr_params.aiynr_param.sta.debug.sw_aiYnrT_debug_mode;
    aiqMutex_unlock(&pAiynrManager->iq_param_mutex);
    if (!pAiynrManager->is_bypass && sw_aiYnrT_debug_mode == aiynr_debugYdest_mode) {
        idx = pAiynrManager->isp_out.y_dest_index;
        AiynrChangeOutBuf(pAiynrManager, ynrbuf_info->dma_fd[idx], ynrbuf_info->width * ynrbuf_info->height, sw_aiYnrT_debug_mode);
    }

    LOGD_AIYNR("aiynr_doDump:frameid=%d,aipre_gain_index=%d,vpsl_index=%d,y_src_index=%d,y_dest_index=%d,bypass=%d",
               isp_in->sequence,
               isp_in->aipre_gain_index,
               isp_in->vpsl_index,
               isp_in->y_src_index,
               isp_in->y_dest_index,
               pAiynrManager->is_bypass);

    pAiynrManager->pCamHw->aiynr_doIspBe(pAiynrManager->pCamHw, isp_in);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiynrManager_hdlEvent(AiynrManager_t* pAiynrManager, AiqHwAinnEvt_t *event)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct rkisp_aiisp_st *isp_idxbuf = &event->queue_buf.aibnr_st;
    struct rkisp_bnr_buf_info *ynr_buf = &pAiynrManager->ispbuf_info.bnr_buf;
    struct rkisp_buf_info *buf_inf;
    u32 idx;
    int i, aiisp_idx = 0;

    if (!pAiynrManager->is_enable) {
        LOGD_AIYNR("%s: aiynr is disabled.", __func__);
        return XCAM_RETURN_BYPASS;
    }

    if (pAiynrManager->is_state_error) {
        LOGE_AIYNR("%s: aiynr in error state.", __func__);
        return XCAM_RETURN_BYPASS;
    }

    // isp event
    if (event->_base.type == ISP_POLL_AIISP) {
        if (event->_event_id == RKISP_AIISP_WR_LINECNT_ID) {
            // call aiisp ioctrl to do NRNN
            /*if (isp_idxbuf->iir_index < 0 || isp_idxbuf->iir_index >= ynr_buf->iir.buf_cnt ||
                    isp_idxbuf->gain_index < 0 || isp_idxbuf->gain_index >= ynr_buf->u.v35.gain.buf_cnt ||
                    isp_idxbuf->aipre_gain_index < 0 || isp_idxbuf->aipre_gain_index >= ynr_buf->u.v35.aipre_gain.buf_cnt ||
                    isp_idxbuf->vpsl_index < 0 || isp_idxbuf->vpsl_index >= ynr_buf->u.v35.vpsl.buf_cnt) {
                LOGE_AIYNR("WR_LINECNT_ID event error index, to check");
                return XCAM_RETURN_ERROR_FAILED;
            }*/

            if (!pAiynrManager->is_get_ynrbuf) {
                pAiynrManager->pCamHw->aiynr_getYnrbufInf(pAiynrManager->pCamHw, &pAiynrManager->ynrbuf_info);
                pAiynrManager->is_get_ynrbuf = true;
                for (int i = 0; i < pAiynrManager->ynrbuf_info.buf_cnt; i++) {
                    LOGE_AIYNR("%s: i %d, get fd %d.", __func__, i, pAiynrManager->ynrbuf_info.dma_fd[i]);
                }
            }

            if (pAiynrManager->is_bypass) {
                // skip nn, inform isp to do BE
                pAiynrManager->isp_in.timestamp = isp_idxbuf->timestamp;
                pAiynrManager->isp_in.sequence  = isp_idxbuf->sequence;
                pAiynrManager->isp_in.iir_index = isp_idxbuf->iir_index;
                pAiynrManager->isp_in.gain_index = isp_idxbuf->gain_index;
                pAiynrManager->isp_in.aipre_gain_index = isp_idxbuf->aipre_gain_index;
                pAiynrManager->isp_in.vpsl_index = isp_idxbuf->vpsl_index;
                pAiynrManager->isp_in.y_src_index = isp_idxbuf->y_src_index;
                pAiynrManager->isp_in.y_dest_index = isp_idxbuf->y_dest_index;
                pAiynrManager->isp_in.aiisp_index = -1;
                aiqMutex_lock(&pAiynrManager->idx_pool_mutex);
                if (pAiynrManager->last_isp_out.aipre_gain_index != -1) {
                    pAiynrManager->isp_in.aipre_gain_index = pAiynrManager->last_isp_out.aipre_gain_index;
                    pAiynrManager->last_isp_out.aipre_gain_index = -1;
                }
                if (pAiynrManager->last_isp_out.vpsl_index != -1) {
                    pAiynrManager->isp_in.vpsl_index = pAiynrManager->last_isp_out.vpsl_index;
                    pAiynrManager->last_isp_out.vpsl_index = -1;
                }
                aiqMutex_unlock(&pAiynrManager->idx_pool_mutex);

                AiynrManager_doIspBe(pAiynrManager, &pAiynrManager->isp_in);
                return ret;
            }

#if 0
            aiqMutex_lock(&pAiynrManager->idx_pool_mutex);
            for (i = 0; i < AIYNR_AIISP_BUF_CNT; i++) {
                if (pAiynrManager->aiisp_idx_pool[i] == 0) {
                    pAiynrManager->aiisp_idx_pool[i] = true;//buf to using
                    aiisp_idx = i;
                    break;
                }
            }
            if (i == AIYNR_AIISP_BUF_CNT) {
                LOGE_AIYNR("no free aiisp buf, to check");
                aiqMutex_unlock(&pAiynrManager->idx_pool_mutex);
                return XCAM_RETURN_ERROR_FAILED;
            }
            aiqMutex_unlock(&pAiynrManager->idx_pool_mutex);
#endif
            isp_idxbuf->aiisp_index = aiisp_idx;
            pAiynrManager->isp_out = event->queue_buf.aibnr_st;
            LOGD_AIYNR("aiynr_doNrnn:frameid=%d,aipre_gain_index=%d,vpsl_index=%d,y_src_index=%d,y_dest_index=%d,bypass=%d",
                       pAiynrManager->isp_out.sequence,
                       pAiynrManager->isp_out.aipre_gain_index,
                       pAiynrManager->isp_out.vpsl_index,
                       pAiynrManager->isp_out.y_src_index,
                       pAiynrManager->isp_out.y_dest_index,
                       pAiynrManager->is_bypass);

            aiqMutex_lock(&pAiynrManager->iq_param_mutex);
            aiynr_debug_mode_t sw_aiYnrT_debug_mode = pAiynrManager->last_aiynr_params.aiynr_param.sta.debug.sw_aiYnrT_debug_mode;
            aiqMutex_unlock(&pAiynrManager->iq_param_mutex);
            if (!pAiynrManager->is_bypass && sw_aiYnrT_debug_mode == aiynr_debugYsrc_mode) {
                idx = pAiynrManager->isp_out.y_src_index;
                buf_inf = &ynr_buf->u.v35.y_src;
                AiynrChangeOutBuf(pAiynrManager, buf_inf->buf_fd[idx], buf_inf->buf_size, sw_aiYnrT_debug_mode);
            }

            pAiynrManager->pCamHw->aiynr_doNrnn(pAiynrManager->pCamHw, &pAiynrManager->isp_out);

        } else {
            /*if (isp_idxbuf->aiisp_index < 0)
                return ret;

            if (isp_idxbuf->aiisp_index >= ynr_buf->u.v35.aiisp.buf_cnt) {
                LOGE_AIYNR("RD_LINECNT_ID event error index, to check, aiisp_index %d, buf_cnt %d",
                    isp_idxbuf->aiisp_index, ynr_buf->u.v35.aiisp.buf_cnt);
                return XCAM_RETURN_ERROR_FAILED;
            }

            aiqMutex_lock(&pAiynrManager->idx_pool_mutex);
            pAiynrManager->aiisp_idx_pool[isp_idxbuf->aiisp_index] = false;
            aiqMutex_unlock(&pAiynrManager->idx_pool_mutex);
            LOGD_AIYNR("aiisp_index %d is free by isp", isp_idxbuf->aiisp_index);*/

            aiqMutex_lock(&pAiynrManager->idx_pool_mutex);
            pAiynrManager->last_isp_out = *isp_idxbuf;
            aiqMutex_unlock(&pAiynrManager->idx_pool_mutex);
            LOGD_AIYNR("RD_LINECNT_ID:frameid=%d,aipre_gain_index=%d,vpsl_index=%d,y_src_index=%d,y_dest_index=%d,bypass=%d",
                       isp_idxbuf->sequence,
                       isp_idxbuf->aipre_gain_index,
                       isp_idxbuf->vpsl_index,
                       isp_idxbuf->y_src_index,
                       isp_idxbuf->y_dest_index,
                       pAiynrManager->is_bypass);
        }
    }
    // aiisp event
    else if (event->_base.type == ISP_POLL_AIYNR_DONE) {
        // inform isp to do BE
        pAiynrManager->isp_in = event->queue_buf.aibnr_st;
        AiynrManager_doIspBe(pAiynrManager, &pAiynrManager->isp_in);
    }
    // rknn event
    else if (event->_base.type == ISP_POLL_RKNN_DONE) {
        // inform isp to do BE
        pAiynrManager->isp_in = event->queue_buf.aibnr_st;
        AiynrManager_doIspBe(pAiynrManager, &pAiynrManager->isp_in);
    }

    return ret;
}

XCamReturn AiynrManager_init(AiynrManager_t* pAiynrManager)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    memset(pAiynrManager, 0, sizeof(AiynrManager_t));
    aiqMutex_init(&pAiynrManager->idx_pool_mutex);
    aiqMutex_init(&pAiynrManager->iq_param_mutex);
    aiqMutex_init(&pAiynrManager->model_buf_mutex);

    return ret;
}

int AiynrManager_dumpRaw(AiynrManager_t* pAiynrManager, int dump_raw_num)
{
    int ret = -1;

    if (pAiynrManager) {
        ret = 0;
        pAiynrManager->dump_raw_num = dump_raw_num;
    }

    return ret;
}

#endif

