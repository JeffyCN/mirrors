/*
 *  Copyright (c) 2022 Rockchip Electronics Co., Ltd
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

#include "algos/acac/rk_aiq_algo_acac_itf.h"

#include "algos/acac/cac_adaptor.h"
#include "algos/acac/rk_aiq_types_acac_algo_prvt.h"
#include "algos/rk_aiq_algo_types.h"
#include "iq_parser_v2/RkAiqCalibDbV2Helper.h"

using RkCam::CacAlgoAdaptor;

RKAIQ_BEGIN_DECLARE

static XCamReturn create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg) {
    auto* ctx = new RkAiqAlgoContext();
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

#if RKAIQ_HAVE_CAC_V03
    auto* calib_cac =
        (CalibDbV2_Cac_V03_t*)(CALIBDBV2_GET_MODULE_PTR(cfg->calibv2, cac_v03));
    XCAM_ASSERT(calib_cac != nullptr);
    adaptor->Config(cfg, calib_cac);
#elif RKAIQ_HAVE_CAC_V10
    auto* calib_cac =
        (CalibDbV2_Cac_V10_t*)(CALIBDBV2_GET_MODULE_PTR(cfg->calibv2, cac_v10));
    XCAM_ASSERT(calib_cac != nullptr);
    adaptor->Config(cfg, calib_cac);
#elif RKAIQ_HAVE_CAC_V11
    auto* calib_cac =
        (CalibDbV2_Cac_V11_t*)(CALIBDBV2_GET_MODULE_PTR(cfg->calibv2, cac_v11));
    XCAM_ASSERT(calib_cac != nullptr);
    adaptor->Config(cfg, calib_cac);
#endif

    ctx->handle = static_cast<void*>(adaptor);
    *context    = ctx;

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn destroy_context(RkAiqAlgoContext* context) {
    if (context != nullptr) {
        if (context->handle != nullptr) {
            auto* adaptor = static_cast<CacAlgoAdaptor*>(context->handle);
            delete adaptor;
            context->handle = nullptr;
        }
        delete context;
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn prepare(RkAiqAlgoCom* params) {
    auto* adaptor        = static_cast<CacAlgoAdaptor*>(params->ctx->handle);
    RkAiqAlgoConfigAcac* config = (RkAiqAlgoConfigAcac*)params;
    auto* calibv2 = config->com.u.prepare.calibv2;

    if (!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        auto* cfg = adaptor->GetConfig();
        const_cast<AlgoCtxInstanceCfg*>(cfg)->calibv2 = calibv2;
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR)
            return XCAM_RETURN_NO_ERROR;
        LOGD_ACAC("Re-config");
#if RKAIQ_HAVE_CAC_V03
        auto* calib_cac =
            (CalibDbV2_Cac_V03_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, cac_v03));
        if (calib_cac) {
            adaptor->Config(cfg, calib_cac);
        }
#elif RKAIQ_HAVE_CAC_V10
        auto* calib_cac = (CalibDbV2_Cac_V10_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, cac_v10));
        if (calib_cac) {
            adaptor->Config(cfg, calib_cac);
        }
#elif RKAIQ_HAVE_CAC_V11
        auto* calib_cac = (CalibDbV2_Cac_V11_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, cac_v11));
        if (calib_cac) {
            adaptor->Config(cfg, calib_cac);
        }
#endif
    }

    // prepare maybe called before hw prepared, may just intend to
    // update calib params before real prepare.
    if (config->width != 0 && config->height != 0)
        return adaptor->Prepare(config);
    else
        return XCAM_RETURN_NO_ERROR;
}

static XCamReturn pre_process(const RkAiqAlgoCom* /* inparams */, RkAiqAlgoResCom* /* outparams */) {
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    RkAiqAlgoProcAcac* input     = (RkAiqAlgoProcAcac*)(inparams);
    RkAiqAlgoProcResAcac* output = (RkAiqAlgoProcResAcac*)outparams;
    auto* adaptor         = static_cast<CacAlgoAdaptor*>(inparams->ctx->handle);

    adaptor->OnFrameEvent(input, output);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn post_process(const RkAiqAlgoCom* /* inparams */, RkAiqAlgoResCom* /* outparams */) {
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
    .pre_process  = NULL,
    .processing   = processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
