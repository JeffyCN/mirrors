/*
 * rk_aiq_algo_acac_itf.c
 *
 *  Copyright (c) 2021 Rockchip Electronics Co., Ltd
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
 * Author: Cody Xie <cody.xie@rock-chips.com>
 */

#include "rk_aiq_algo_acac_itf.h"

#include "cac_adaptor.h"
#include "rk_aiq_types_acac_algo_prvt.h"
#include "rk_aiq_algo_types.h"
#include "RkAiqCalibDbV2Helper.h"

using namespace RkCam;
using namespace XCam;

RKAIQ_BEGIN_DECLARE

static XCamReturn create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg) {
    RkAiqAlgoContext* ctx = new RkAiqAlgoContext();
    if (ctx == nullptr) {
        LOGE_ACAC("create acac context fail!");
        return XCAM_RETURN_ERROR_MEM;
    }

    auto* adaptor = new CacAlgoAdaptor();
    if (adaptor == nullptr) {
        LOGE_ACAC("create acac handle fail!");
        delete ctx;
        return XCAM_RETURN_ERROR_MEM;
    }

    CalibDbV2_Cac_t* calib_cac =
        (CalibDbV2_Cac_t*)(CALIBDBV2_GET_MODULE_PTR(cfg->calibv2, cac_calib));
    XCAM_ASSERT(calib_cac != nullptr);
    adaptor->Config(cfg, calib_cac);

    ctx->handle = static_cast<void*>(adaptor);
    *context    = ctx;

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn destroy_context(RkAiqAlgoContext* context) {
    if (context) {
        if (context->handle) {
            CacAlgoAdaptor* adaptor = static_cast<CacAlgoAdaptor*>(context->handle);
            delete adaptor;
            context->handle = nullptr;
        }
        delete context;
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn prepare(RkAiqAlgoCom* params) {
    CacAlgoAdaptor* adaptor        = static_cast<CacAlgoAdaptor*>(params->ctx->handle);
    RkAiqAlgoConfigAcac* config = (RkAiqAlgoConfigAcac*)params;

    return adaptor->Prepare(config);
}

static XCamReturn pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    RkAiqAlgoProcAcac* input     = (RkAiqAlgoProcAcac*)(inparams);
    RkAiqAlgoProcResAcac* output = (RkAiqAlgoProcResAcac*)outparams;
    CacAlgoAdaptor* adaptor         = static_cast<CacAlgoAdaptor*>(inparams->ctx->handle);

    adaptor->OnFrameEvent(input, output);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAcac = {
    .common =
        {
            .version         = RKISP_ALGO_ACAC_VERSION,
            .vendor          = RKISP_ALGO_ACAC_VENDOR,
            .description     = RKISP_ALGO_ACAC_DESCRIPTION,
            .type            = RK_AIQ_ALGO_TYPE_ACAC,
            .id              = 0,
            .create_context  = create_context,
            .destroy_context = destroy_context,
        },
    .prepare      = prepare,
    .pre_process  = pre_process,
    .processing   = processing,
    .post_process = post_process,
};

RKAIQ_END_DECLARE
