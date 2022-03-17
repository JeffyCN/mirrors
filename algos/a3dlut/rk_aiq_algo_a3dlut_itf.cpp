/*
 * rk_aiq_algo_a3dlut_itf.c
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

#include "a3dlut/rk_aiq_algo_a3dlut_itf.h"
#include "a3dlut/rk_aiq_a3dlut_algo.h"
#include "rk_aiq_algo_types.h"
#include "xcam_log.h"

RKAIQ_BEGIN_DECLARE


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOG1_A3DLUT( "%s: (enter)\n", __FUNCTION__);
    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_A3DLUT( "%s: create 3dlut context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }

    Alut3dInit(&ctx->a3dlut_para, cfg->calibv2);

    *context = ctx;

    LOG1_A3DLUT( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    LOG1_A3DLUT( "%s: (enter)\n", __FUNCTION__);

    Alut3dRelease((alut3d_handle_t)context->a3dlut_para);
    delete context;
    LOG1_A3DLUT( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_A3DLUT( "%s: (enter)\n", __FUNCTION__);
    alut3d_handle_t hAlut3d = (alut3d_handle_t)(params->ctx->a3dlut_para);
    RkAiqAlgoConfigA3dlut *para = (RkAiqAlgoConfigA3dlut*)params;
    hAlut3d->prepare_type = params->u.prepare.conf_type;
    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        hAlut3d->calibV2_lut3d =
                (CalibDbV2_Lut3D_Para_V2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(para->com.u.prepare.calibv2), lut3d_calib));
    }
           Alut3dPrepare((alut3d_handle_t)(params->ctx->a3dlut_para));

    LOG1_A3DLUT( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_A3DLUT( "%s: (enter)\n", __FUNCTION__);

    Alut3dPreProc((alut3d_handle_t)(inparams->ctx->a3dlut_para));

    LOG1_A3DLUT( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_A3DLUT( "%s: (enter)\n", __FUNCTION__);

    RkAiqAlgoProcA3dlut *procAlut3d = (RkAiqAlgoProcA3dlut*)inparams;
    RkAiqAlgoProcResA3dlut *proResAlut3d = (RkAiqAlgoProcResA3dlut*)outparams;
    alut3d_handle_t hAlut3d = (alut3d_handle_t)(inparams->ctx->a3dlut_para);

    hAlut3d->swinfo.sensorGain = procAlut3d->sensorGain;
    hAlut3d->swinfo.awbGain[0] = procAlut3d->awbGain[0];
    hAlut3d->swinfo.awbGain[1] = procAlut3d->awbGain[1];
    hAlut3d->swinfo.awbIIRDampCoef = procAlut3d->awbIIRDampCoef;

    Alut3dConfig(hAlut3d);
    memcpy(&proResAlut3d->lut3d_hw_conf, &hAlut3d->lut3d_hw_conf, sizeof(rk_aiq_lut3d_cfg_t));
    proResAlut3d->lut3d_update = hAlut3d->update ||hAlut3d->updateAtt || (!hAlut3d->swinfo.lut3dConverged);

    LOG1_A3DLUT( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_A3DLUT( "%s: (enter)\n", __FUNCTION__);

    LOG1_A3DLUT( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;

}

RkAiqAlgoDescription g_RkIspAlgoDescA3dlut = {
    .common = {
        .version = RKISP_ALGO_A3DLUT_VERSION,
        .vendor  = RKISP_ALGO_A3DLUT_VENDOR,
        .description = RKISP_ALGO_A3DLUT_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_A3DLUT,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = pre_process,
    .processing = processing,
    .post_process = post_process,
};

RKAIQ_END_DECLARE
