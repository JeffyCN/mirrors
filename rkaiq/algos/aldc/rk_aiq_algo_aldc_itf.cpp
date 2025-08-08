/*
 * rk_aiq_algo_aldc_itf.c
 *
 *  Copyright (c) 2023 Rockchip Corporation
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

#include "algos/aldc/rk_aiq_algo_aldc_itf.h"

#include "algos/aldc/ldc_algo_adaptor.h"
#include "algos/aldc/rk_aiq_types_aldc_algo_prvt.h"
#include "algos/rk_aiq_algo_types.h"
#include "iq_parser_v2/RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"

using RkCam::LdcAlgoAdaptor;

RKAIQ_BEGIN_DECLARE

static XCamReturn create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg) {
    auto* ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_ALDC("%s: create aldc context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    auto* adaptor = new LdcAlgoAdaptor();
    if (adaptor == nullptr) {
        LOGE_ALDC("create aldc handle fail!");
        delete ctx;
        return XCAM_RETURN_ERROR_MEM;
    }

    auto* calib_ldc = (CalibDbV2_LDC_t*)(CALIBDBV2_GET_MODULE_PTR(cfg->calibv2, ldc));
    XCAM_ASSERT(calib_ldc != nullptr);

    adaptor->Config(cfg, RK_AIQ_ALGO_CONFTYPE_UPDATECALIB, calib_ldc);

    ctx->handle = static_cast<void*>(adaptor);
    *context    = ctx;

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn destroy_context(RkAiqAlgoContext* context) {
    if (context != nullptr) {
        if (context->handle != nullptr) {
            auto* adaptor = static_cast<LdcAlgoAdaptor*>(context->handle);
            delete adaptor;
            context->handle = nullptr;
        }
        delete context;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn prepare(RkAiqAlgoCom* params) {
    auto* adaptor               = static_cast<LdcAlgoAdaptor*>(params->ctx->handle);
    RkAiqAlgoConfigAldc* config = (RkAiqAlgoConfigAldc*)params;
    auto* calibv2               = config->com.u.prepare.calibv2;
    auto* calib_ldc             = (CalibDbV2_LDC_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, ldc));

    if (!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB) &&
        !!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR)) {
        auto* cfg = adaptor->GetConfig();
        adaptor->Config(cfg, params->u.prepare.conf_type, calib_ldc);
    }

    return adaptor->Prepare(config);
}

static XCamReturn pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    RkAiqAlgoProcAldc* input     = (RkAiqAlgoProcAldc*)(inparams);
    RkAiqAlgoProcResAldc* output = (RkAiqAlgoProcResAldc*)outparams;
    auto* adaptor                = static_cast<LdcAlgoAdaptor*>(inparams->ctx->handle);

    adaptor->OnFrameEvent(input, output);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAldc = {
    .common =
        {
            .version         = RKISP_ALGO_ALDC_VERSION,
            .vendor          = RKISP_ALGO_ALDC_VENDOR,
            .description     = RKISP_ALGO_ALDC_DESCRIPTION,
            .type            = RK_AIQ_ALGO_TYPE_ALDC,
            .id              = 0,
            .create_context  = create_context,
            .destroy_context = destroy_context,
        },
    .prepare      = prepare,
    .pre_process  = pre_process,
    .processing   = processing,
    .post_process = post_process,
    .dump = NULL,
};

RKAIQ_END_DECLARE
