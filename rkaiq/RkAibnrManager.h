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

#ifndef _RK_AIBNR_MANAGER_H_
#define _RK_AIBNR_MANAGER_H_

#include "hwi_c/aiq_CamHwBase.h"
#include "rk-aiisp-config.h"
#include "c_base/aiq_mutex.h"
#include "algo_handlers/RkAiqAeHandler.h"
#include "algo_camgroup_handlers/RkAiqCamGroupAeHandle.h"


RKAIQ_BEGIN_DECLARE

#define AIBNR_IIR_BUF_CNT                3
#define AIBNR_GAIN_BUF_CNT               4
#define AIBNR_AIPRE_BUF_CNT              3
#define AIBNR_VPSL_BUF_CNT               3
#define AIBNR_AIISP_BUF_CNT              3
#define AIBNR_ISP_BUF_MAX                AIBNR_GAIN_BUF_CNT
#define AIBNR_AIISP_MODE                 BOTHEVENT_TO_AIQ
#define AIBNR_INBUF_MAX_WIDTH            (4096)

#if RKAIQ_HAVE_AIBNR

typedef struct rk_post_bnr_param_s {
    // rawInfo
    int rawHgt;
    int rawWid;
    int rawHgtStd;
    int rawWidStd;

    void* pRaw;

    // gainInfo
    int gainHgt;
    int gainWid;
    int gainHgtStd;
    int gainWidStd;

    int gainHborder;
    int gainWborder;

    void* pGain;

    float ISO;
    int gainMode;
    int bayerPattern; // 0--BGGR,1--GBRG,2--GRBG,3--RGGB

    char sns_name[32];
} rk_post_bnr_param;

typedef int (*rk_post_bnr_init)(rk_post_bnr_param* param);
typedef int (*rk_post_bnr_proc)(rk_post_bnr_param* param);
typedef int (*rk_post_bnr_deinit)(rk_post_bnr_param* param);

struct PostBnrOps {
    rk_post_bnr_init bnr_init;
    rk_post_bnr_proc bnr_proc;
    rk_post_bnr_deinit bnr_deinit;
};

struct PostBnrLibrary;
struct PostBnrLibrary {
    bool (*Init)(struct PostBnrLibrary* obj);
    bool (*LoadSymbols)(struct PostBnrLibrary* obj);
    void (*Deinit)(struct PostBnrLibrary* obj);

    void* handle_;
    struct PostBnrOps ops_;
};

typedef struct ispbuf_addr_s {
    char *iir_addr;
    char *gain_addr;
    char *aipre_addr;
    char *vpsl_addr;
    char *aiisp_addr;
} ispbuf_addr_t;

typedef struct AibnrModelBuf_s {
    bool valid;
    char model_file[AIBNR_ISO_STEP_MAX][AIBNR_MODEL_PATH_LEN];
    char *model_buf[AIBNR_ISO_STEP_MAX];
} AibnrModelBuf_t;

typedef struct AibnrManager_s AibnrManager_t;
typedef struct AibnrManager_s {
    bool is_enable;
    bool is_bypass;
    bool is_start;
    bool is_param_update;
    bool is_parambuf_prepare;
    bool is_state_error;
    int dump_raw_num;
    uint32_t model_max_runcnt;
    uint32_t isp_acq_width;
    uint32_t isp_acq_height;
    rk_aiq_isp_aibnr_params_t last_aibnr_params;
    struct rkaiisp_param_info param_info;
    struct rkaiisp_ispbuf_info ispbuf_info;
    struct rkisp_aiisp_st isp_out;
    struct rkisp_aiisp_st isp_in;
    struct rkisp_aiisp_cfg mAibnrCfg;
    AibnrModelBuf_t mAibnrModelBuf;
    RkAiqAibnrModelInfo_t mAibnrModelInfo;
    CamCalibDbV2Context_t* pCalibDbV2;
    AiqCamHwBase_t* pCamHw;
    AiqManager_t* pAiqManager;

    ispbuf_addr_t ispbuf_addr[AIBNR_ISP_BUF_MAX];
    bool aiisp_idx_pool[AIBNR_AIISP_BUF_CNT];
    AiqMutex_t idx_pool_mutex;
    AiqMutex_t iq_param_mutex;
    AiqMutex_t model_buf_mutex;

    struct PostBnrLibrary postBnrLib;
    rk_post_bnr_param postBnrParam;

    bool doAiisp_en;
    bool apiFrmRate_valid;
    ae_frmRate_t apiFrmRate;
    ae_frmRate_t iqFrmRate;
    AiqAlgoHandler_t* aeHandler;
    bool is_group;
    AiqMutex_t apiFrmRate_mutex;
    enum rkaiisp_mem_mode mMemMode;
    bool pBtnr2En;
} AibnrManager_t;

XCamReturn AibnrManager_init(AiqManager_t* pAiqManager, AibnrManager_t* pAibnrManager);
XCamReturn AibnrManager_setCalib(AibnrManager_t* pAibnrManager, CamCalibDbV2Context_t* pCalibDbV2);
XCamReturn AibnrManager_deinit(AibnrManager_t* pAibnrManager);
XCamReturn AibnrManager_prepare(AibnrManager_t* pAibnrManager, AiqCamHwBase_t* pCamHw, CamCalibDbV2Context_t* pCalibDbV2,
                               uint32_t isp_acq_width, uint32_t isp_acq_height);
XCamReturn AibnrManager_start(AibnrManager_t* pAibnrManager);
XCamReturn AibnrManager_stop(AibnrManager_t* pAibnrManager);
XCamReturn AibnrManager_updateParams(AibnrManager_t* pAibnrManager, rk_aiq_isp_aibnr_params_t *aibnr_params, uint32_t frame_id);
XCamReturn AibnrManager_hdlEvent(AibnrManager_t* pAibnrManager, AiqHwAinnEvt_t *event);
XCamReturn AibnrManager_setCalib(AibnrManager_t* pAibnrManager, CamCalibDbV2Context_t* pCalibDbV2);
bool AibnrManager_isNeedRknn(AibnrManager_t* pAibnrManager);
int AibnrManager_dumpRaw(AibnrManager_t* pAibnrManager, int dump_raw_num);
XCamReturn AibnrManager_setFrmRate(AiqAlgoHandler_t* aeHandler, bool is_group,
    AibnrManager_t* pAibnrManager, ae_api_expSwAttr_t *expSwAttr);

#endif

RKAIQ_END_DECLARE

#endif
