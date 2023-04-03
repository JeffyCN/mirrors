/*
 * rk_aiq_algo_aeis_itf.c
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

#include "aeis/rk_aiq_algo_aeis_itf.h"

#include <memory>
#include <fstream>

#include "aeis/rk_aiq_types_aeis_algo_prvt.h"
#include "drm_device.h"
#include "drm_buffer.h"
//#include "dma_buffer.h"
#include "dma_video_buffer.h"
#include "dvs_app.h"
#include "remap_backend.h"
#include "unistd.h"
#include "xcam_common.h"
#include "xcam_log.h"
#include "imu_service.h"
#include "video_buffer.h"
#include "image_processor.h"
#include "scaler_service.h"
#include "eis_algo_service.h"
#include "RkAiqCalibDbV2Helper.h"

using namespace RkCam;
using namespace XCam;

RKAIQ_BEGIN_DECLARE

static XCamReturn create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg) {
    RkAiqAlgoContext* ctx = new RkAiqAlgoContext();
    if (ctx == nullptr) {
        LOGE_AEIS("create aeis context fail!");
        return XCAM_RETURN_ERROR_MEM;
    }

    auto adaptor = new EisAlgoAdaptor();
    if (adaptor == nullptr) {
        LOGE_AEIS("create aeis handle fail!");
        delete ctx;
        return XCAM_RETURN_ERROR_MEM;
    }

    CalibDbV2_Eis_t* calib_eis =
                (CalibDbV2_Eis_t*)(CALIBDBV2_GET_MODULE_PTR(cfg->calibv2, eis_calib));
    adaptor->Config(cfg, calib_eis);

    ctx->handle = static_cast<void*>(adaptor);
    *context = ctx;

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn destroy_context(RkAiqAlgoContext* context) {
    if (context) {
        if (context->handle) {
            EisAlgoAdaptor* adaptor = static_cast<EisAlgoAdaptor*>(context->handle);
            delete adaptor;
            context->handle = nullptr;
        }
        delete context;
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn prepare(RkAiqAlgoCom* params) {
    EisAlgoAdaptor* adaptor = static_cast<EisAlgoAdaptor*>(params->ctx->handle);
    RkAiqAlgoConfigAeis* config = (RkAiqAlgoConfigAeis*)params;

    return adaptor->Prepare(config->mems_sensor_intf, config->mem_ops);
}

static XCamReturn pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    RkAiqAlgoProcAeis* input = (RkAiqAlgoProcAeis*)(inparams);
    RkAiqAlgoProcResAeis* output = (RkAiqAlgoProcResAeis*)outparams;
    EisAlgoAdaptor* adaptor = static_cast<EisAlgoAdaptor*>(inparams->ctx->handle);

    if (!adaptor->IsEnabled()) {
        return XCAM_RETURN_NO_ERROR;
    }

    if (!adaptor->IsValid()) {
        return XCAM_RETURN_BYPASS;
    }

    adaptor->Start();

    output->fec_en = true;

    adaptor->GetProcResult(output);

    adaptor->OnFrameEvent(input);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAeis = {
    .common =
        {
            .version         = RKISP_ALGO_AEIS_VERSION,
            .vendor          = RKISP_ALGO_AEIS_VENDOR,
            .description     = RKISP_ALGO_AEIS_DESCRIPTION,
            .type            = RK_AIQ_ALGO_TYPE_AEIS,
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
