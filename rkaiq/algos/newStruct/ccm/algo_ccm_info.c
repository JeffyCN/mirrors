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

#include "include/algo_ccm_info.h"

#include "ccm_types_prvt.h"
#include "rk_info_utils.h"

void ccm_dump_mod_param(const RkAiqAlgoCom* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "module param");

    aiq_string_printf(result, "\n\n");
}

void ccm_dump_mod_attr(const RkAiqAlgoCom* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "ccm attr");

    aiq_string_printf(result, "\n\n");
}

void ccm_dump_mod_status(const RkAiqAlgoCom* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "ccm status");

    CcmContext_t* ctx = (CcmContext_t*)(self->ctx);
    ccm_api_attrib_t* tunning = &ctx->ccm_attrib->tunning;
    accm_param_illuLink_t *pIlluCase = &tunning->stAuto.dyn.illuLink[ctx->pre_illu_idx];

    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-12s%-12s%-12s%-12s%-12s",
             "phy_chn", "seq", "cur_rgain", "cur_bgain", "cur_illu", "cur_sat");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    RkAiqAlgoProcCcm* proc_param = (RkAiqAlgoProcCcm*)self;
    illu_estm_info_t *swinfo = &proc_param->illu_info;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8d%-12d%-12.4f%-12.4f%-12s%-12.2f",
             self->cid, self->frame_id, swinfo->awbGain[0], swinfo->awbGain[1],
             pIlluCase->sw_ccmC_illu_name, ctx->pre_saturation);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);

    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-12s%-12s%-12s%-12s%-12s",
             "phy_chn", "seq", "cur_iso", "last_scale", "cur_scale", "used_scale");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8d%-12d%-12d%-12.2f%-12.2f%-12.2f",
             self->cid, self->frame_id, ctx->pre_iso, ctx->pre_scale, ctx->fScale,
             fabs(ctx->pre_scale - ctx->fScale) > 0.01 ? ctx->fScale : ctx->pre_scale);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);

    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-12s%-8s%-16s%-10s",
             "phy_chn", "seq", "damp_en", "damp_converged", "dampCoef");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8d%-12d%-8s%-16s%-10.4f",
             self->cid, self->frame_id,
             tunning->stAuto.sta.sw_ccmT_damp_en ? "Y" : "N",
             ctx->damp_converged ? "Y" : "N", swinfo->awbIIRDampCoef);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    if(tunning->stAuto.sta.sw_ccmT_damp_en && (!ctx->damp_converged)) {
        float* mat_tmp = ctx->undamped_matrix.hw_ccmC_matrix_coeff;

        memset(buffer, 0, MAX_LINE_LENGTH);

        snprintf(buffer, MAX_LINE_LENGTH,
                 "%-12s%-8.3f%-8.3f%-8.3f%-8.3f%-8.3f%-8.3f%-8.3f%-8.3f%-8.3f",
                 "undampMat",
                 mat_tmp[0], mat_tmp[1], mat_tmp[2],
                 mat_tmp[3], mat_tmp[4], mat_tmp[5],
                 mat_tmp[6], mat_tmp[7], mat_tmp[8]);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n\n");

        aiq_info_dump_title(result, "undampOffset");

        mat_tmp = ctx->undamped_matrix.hw_ccmC_matrix_offset;

        memset(buffer, 0, MAX_LINE_LENGTH);
        snprintf(buffer, MAX_LINE_LENGTH, "%-8.3f%-8.3f%-8.3f",
                 mat_tmp[0], mat_tmp[1], mat_tmp[2]);

        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n\n");
    }
}
