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
#include "rk_aiq_types_priv_c.h"

#include "algo_types_priv.h"
#include "airms_types_prvt.h"
#include "xcam_log.h"

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "interpolation.h"
#include "c_base/aiq_base.h"
#include "newStruct/algo_common.h"
#include "common/rk-aiisp-config.h"

#include "rk_info_utils.h"

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    AirmsContext_t* ctx = aiq_mallocz(sizeof(AirmsContext_t));

    if (ctx == NULL) {
        LOGE_AIRMS("%s create context failed!", __func__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->airms_attrib =
        (airms_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, airms));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_AIRMS("%s: Airms (exit)\n", __FUNCTION__ );

    *context = (RkAiqAlgoContext*)ctx;
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    AirmsContext_t* pAirmsCtx = (AirmsContext_t*)context;
    aiq_free(pAirmsCtx);
    return result;
}

static XCamReturn prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    AirmsContext_t* pAirmsCtx = (AirmsContext_t *)params->ctx;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pAirmsCtx->airms_attrib =
                (airms_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, airms));
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pAirmsCtx->working_mode = params->u.prepare.working_mode;
    pAirmsCtx->airms_attrib =
        (airms_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, airms));

    return result;
}

XCamReturn Airms_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso)
{
    AirmsContext_t* pAirmsCtx = (AirmsContext_t *)inparams->ctx;
    airms_api_attrib_t* airms_attrib = pAirmsCtx->airms_attrib;
    airms_param_t* airms_res = outparams->algoRes;

    if (airms_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_AIRMS("mode is %d, not auto mode, ignore", airms_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (airms_attrib->en == false) {
        if (inparams->u.proc.is_attrib_update) {
            outparams->cfg_update = true;
            outparams->en = airms_attrib->en;
            outparams->bypass = airms_attrib->bypass;
        }
        LOGD_AIRMS("AIRMS not enable, skip!");
        return XCAM_RETURN_NO_ERROR;
    }
    bool need_recal = false;

    bool init = inparams->u.proc.init;
    if (inparams->u.proc.is_attrib_update || inparams->u.proc.init) {
        need_recal = true;
    }

    outparams->cfg_update = false;
    if (need_recal) {
        airms_res->sta = airms_attrib->stAuto.sta;
        pAirmsCtx->cur_param = *airms_res;

        outparams->cfg_update = true;
        outparams->en = airms_attrib->en;
        outparams->bypass = airms_attrib->bypass;
        LOGD_AIRMS("AIRMS processing: iso %d, airms en:%d, bypass:%d", iso, outparams->en, outparams->bypass);
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    int iso = inparams->u.proc.iso;
    Airms_processing(inparams, outparams, iso);
    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_DUMPSYS
static int dump(const RkAiqAlgoCom* self, st_string* result)
{
    AirmsContext_t* pAirmsCtx = (AirmsContext_t *)self->ctx;
    airms_param_t *cur_param = &pAirmsCtx->cur_param;
    airms_params_static_t *sta = &cur_param->sta;
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "airms status");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "Cam%d FrmId=%d working mode=%d",
             self->cid, self->frame_id, pAirmsCtx->working_mode);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiRmsCfg_model_file=%s",
            sta->model_path.sw_aiRmsCfg_model_file);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    return 0;
}
#endif

#define RKISP_ALGO_AIRMS_VERSION     "v0.0.9"
#define RKISP_ALGO_AIRMS_VENDOR      "Rockchip"
#define RKISP_ALGO_AIRMS_DESCRIPTION "Rockchip airms algo for ISP35"

RkAiqAlgoDescription g_RkIspAlgoDescAirms = {
    .common = {
        .version = RKISP_ALGO_AIRMS_VERSION,
        .vendor  = RKISP_ALGO_AIRMS_VENDOR,
        .description = RKISP_ALGO_AIRMS_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AIRMS,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
#if RKAIQ_HAVE_DUMPSYS
    .dump = dump,
#endif
};

