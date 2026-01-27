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

#ifndef _RK_AIYNR_MANAGER_H_
#define _RK_AIYNR_MANAGER_H_

#include "hwi_c/aiq_CamHwBase.h"
#include "rk-aiisp-config.h"
#include "c_base/aiq_mutex.h"

RKAIQ_BEGIN_DECLARE
;
#define AIYNR_IIR_BUF_CNT                3
#define AIYNR_GAIN_BUF_CNT               4
#define AIYNR_AIPRE_BUF_CNT              3
#define AIYNR_VPSL_BUF_CNT               3
#define AIYNR_AIISP_BUF_CNT              3
#define AIYNR_YSRC_BUF_CNT               2
#define AIYNR_ISP_BUF_MAX                4
#define AIYNR_AIISP_MODE                 BOTHEVENT_TO_AIQ

#if RKAIQ_HAVE_AIYNR

typedef struct AiynrBufAddr_s {
    char *aipre_addr;
    char *vpsl_addr;
    char *ysrc_addr;
} AiynrBufAddr_t;

typedef struct AiynrModelBuf_s {
    bool valid;
    char model_file[AIYNR_ISO_STEP_MAX][AIYNR_MODEL_PATH_LEN];
    char *model_buf[AIYNR_ISO_STEP_MAX];
} AiynrModelBuf_t;

typedef struct AiynrManager_s AiynrManager_t;
typedef struct AiynrManager_s {
    bool is_enable;
    bool is_bypass;
    bool is_start;
    bool is_param_update;
    bool is_parambuf_prepare;
    bool is_aiynr_enable;
    bool is_state_error;
    bool is_get_ynrbuf;
    int dump_raw_num;
    uint32_t model_max_runcnt;
    uint32_t isp_acq_width;
    uint32_t isp_acq_height;
    rk_aiq_isp_aiynr_params_t last_aiynr_params;
    struct rkaiisp_param_info param_info;
    struct rkaiisp_ispbuf_info ispbuf_info;
    struct rkisp_aiisp_st isp_out;
    struct rkisp_aiisp_st isp_in;
    struct rkisp_aiisp_st last_isp_out;
    struct rkisp_aiisp_cfg mAiynrCfg;
    struct rkaiisp_ynrbuf_info ynrbuf_info;
    AiynrModelBuf_t mAiynrModelBuf;
    RkAiqAiynrModelInfo_t mAiynrModelInfo;
    CamCalibDbV2Context_t* pCalibDbV2;
    AiqCamHwBase_t* pCamHw;

    AiynrBufAddr_t ispbuf_addr[AIYNR_ISP_BUF_MAX];
    bool aiisp_idx_pool[AIYNR_AIISP_BUF_CNT];
    AiqMutex_t idx_pool_mutex;
    AiqMutex_t iq_param_mutex;
    AiqMutex_t model_buf_mutex;
} AiynrManager_t;

XCamReturn AiynrManager_init(AiynrManager_t* pAiynrManager);
XCamReturn AiynrManager_setCalib(AiynrManager_t* pAiynrManager, CamCalibDbV2Context_t* pCalibDbV2);
XCamReturn AiynrManager_deinit(AiynrManager_t* pAiynrManager);
XCamReturn AiynrManager_prepare(AiynrManager_t* pAiynrManager, AiqCamHwBase_t* pCamHw, CamCalibDbV2Context_t* pCalibDbV2,
                                uint32_t isp_acq_width, uint32_t isp_acq_height);
XCamReturn AiynrManager_start(AiynrManager_t* pAiynrManager);
XCamReturn AiynrManager_stop(AiynrManager_t* pAiynrManager);
XCamReturn AiynrManager_updateParams(AiynrManager_t* pAiynrManager, rk_aiq_isp_aiynr_params_t *aiynr_params, uint32_t frame_id);
XCamReturn AiynrManager_hdlEvent(AiynrManager_t* pAiynrManager, AiqHwAinnEvt_t *event);
XCamReturn AiynrManager_setCalib(AiynrManager_t* pAiynrManager, CamCalibDbV2Context_t* pCalibDbV2);
int AiynrManager_dumpRaw(AiynrManager_t* pAiynrManager, int dump_raw_num);

#endif

RKAIQ_END_DECLARE

#endif
