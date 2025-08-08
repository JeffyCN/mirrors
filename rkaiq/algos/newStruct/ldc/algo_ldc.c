/*
 * algo_ldc.c
 *
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

#include "algos/algo_types_priv.h"
#include "algos/newStruct/ldc/include/ldc_algo_adaptor.h"
#include "algos/newStruct/ldc/ldc_types_prvt.h"
#include "c_base/aiq_base.h"
#include "include/algos/rk_aiq_api_types_ldc.h"
#include "iq_parser_v2/RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"

#if RKAIQ_HAVE_DUMPSYS
#include "include/algo_ldc_info.h"
#include "rk_info_utils.h"
#endif

static XCamReturn create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg) {
    XCamReturn result                 = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t* pCalibDbV2 = cfg->calibv2;

    LdcContext_t* ctx = (LdcContext_t*)aiq_mallocz(sizeof(LdcContext_t));
    if (ctx == NULL) {
        LOGE_ALDC("%s: create Ldc context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }

    LdcAlgoAdaptor* adp = (LdcAlgoAdaptor*)aiq_mallocz(sizeof(LdcAlgoAdaptor));
    if (adp == NULL) {
        LOGE_ALDC("Failed to malloc Ldc adaptor fail!\n");
        return XCAM_RETURN_ERROR_MEM;
    }

    ldc_api_attrib_t* calib_ldc = (ldc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(cfg->calibv2, ldc));
    LdcAlgoAdaptor_init(adp);
    LdcAlgoAdaptor_config(adp, calib_ldc);

    ctx->handler = adp;
    *context     = (RkAiqAlgoContext*)ctx;
    LOGV_ALDC("%s: (exit)\n", __FUNCTION__);
    return result;
}

static XCamReturn destroy_context(RkAiqAlgoContext* context) {
    if (context != NULL) {
        LdcContext_t* ldc_ctx = (LdcContext_t*)context;
        if (ldc_ctx->handler) {
            LdcAlgoAdaptor_deinit((LdcAlgoAdaptor*)(ldc_ctx->handler));
            aiq_free(ldc_ctx->handler);
            ldc_ctx->handler = NULL;
        }
        aiq_free(ldc_ctx);
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn prepare(RkAiqAlgoCom* params) {
    LdcContext_t* ldc_ctx = (LdcContext_t*)(params->ctx);
    LdcAlgoAdaptor* adp   = (LdcAlgoAdaptor*)(ldc_ctx->handler);

    return LdcAlgoAdaptor_prepare(adp, (RkAiqAlgoConfigLdc*)params);
}

static XCamReturn processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    LdcAlgoAdaptor_onFrameEvent(inparams, outparams);

    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_DUMPSYS
static int dump(const RkAiqAlgoCom* config, st_string* result) {
    ldc_dump_mod_param(config, result);
    ldc_dump_mod_attr(config, result);
    ldc_dump_mod_status(config, result);

    return 0;
}
#endif

LdcLutBuffer* algo_ldc_getLdchFreeLutBuf(RkAiqAlgoContext* ctx) {
    LdcContext_t* ctx_  = (LdcContext_t*)ctx;
    LdcAlgoAdaptor* adp = (LdcAlgoAdaptor*)(ctx_->handler);

    return LdcAlgoAdaptor_getFreeLutBuf(adp, kLdch, 0);
}

#if RKAIQ_HAVE_LDCV
LdcLutBuffer* algo_ldc_getLdcvFreeLutBuf(RkAiqAlgoContext* ctx) {
    LdcContext_t* ctx_  = (LdcContext_t*)ctx;
    LdcAlgoAdaptor* adp = (LdcAlgoAdaptor*)(ctx_->handler);

    return LdcAlgoAdaptor_getFreeLutBuf(adp, kLdcv, 0);
}
#endif

#define RKISP_ALGO_LDC_VERSION     "v0.0.9"
#define RKISP_ALGO_LDC_VENDOR      "Rockchip"
#define RKISP_ALGO_LDC_DESCRIPTION "Rockchip Ldc algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescLdc = {
    .common =
        {
            .version         = RKISP_ALGO_LDC_VERSION,
            .vendor          = RKISP_ALGO_LDC_VENDOR,
            .description     = RKISP_ALGO_LDC_DESCRIPTION,
            .type            = RK_AIQ_ALGO_TYPE_ALDC,
            .id              = 0,
            .create_context  = create_context,
            .destroy_context = destroy_context,
        },
    .prepare    = prepare,
    .processing = processing,
#if RKAIQ_HAVE_DUMPSYS
    .dump = dump,
#endif
};
