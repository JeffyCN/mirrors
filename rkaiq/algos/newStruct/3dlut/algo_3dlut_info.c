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

#include "include/algo_3dlut_info.h"

#include "3dlut_types_prvt.h"
#include "rk_info_utils.h"

void lut3d_dump_mod_param(const RkAiqAlgoCom* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "module param");

    aiq_string_printf(result, "\n\n");
}

void lut3d_dump_mod_attr(const RkAiqAlgoCom* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "3dlut attr");

    aiq_string_printf(result, "\n\n");
}

void lut3d_dump_mod_status(const RkAiqAlgoCom* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "3dlut status");

    Lut3dContext_t* ctx = (Lut3dContext_t*)(self->ctx);
    lut3d_api_attrib_t* tunning = &ctx->lut3d_attrib->tunning;
    alut3d_param_illuLink_t *pIlluCase = &tunning->stAuto.dyn.illuLink[ctx->pre_illu_idx];

    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-12s%-12s%-12s%-12s%-12s%-12s",
             "phy_chn", "seq", "cur_rgain", "cur_bgain", "cur_illu", "cur_gain", "cur_alp");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    RkAiqAlgoProcLut3d* proc_param = (RkAiqAlgoProcLut3d*)self;
    illu_estm_info_t *swinfo = &proc_param->illu_info;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8d%-12d%-12.4f%-12.4f%-12s%-12.1f%-12.2f",
             self->cid, self->frame_id, swinfo->awbGain[0], swinfo->awbGain[1],
             pIlluCase->sw_lut3dC_illu_name, swinfo->sensorGain, ctx->pre_alpha);

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
             tunning->stAuto.sta.sw_lut3dT_damp_en ? "Y" : "N",
             ctx->damp_converged ? "Y" : "N", swinfo->awbIIRDampCoef);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    if(tunning->stAuto.sta.sw_lut3dT_damp_en) {
        memset(buffer, 0, MAX_LINE_LENGTH);

        snprintf(buffer, MAX_LINE_LENGTH, "%-12s%-12s%-12s%-12s%-12s",
                "undampCoef", "lut[9]", "lut[82]", "lut[627]", "lut[727]");
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n");

        uint16_t* mat_tmp = ctx->undamped_matrix.hw_lut3dC_lutR_val;

        memset(buffer, 0, MAX_LINE_LENGTH);

        snprintf(buffer, MAX_LINE_LENGTH,
                 "%-12s%-12d%-12d%-12d%-12d",
                 "lutR_val:", mat_tmp[9], mat_tmp[82], 
                 mat_tmp[627], mat_tmp[727]);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n\n");

        mat_tmp = ctx->undamped_matrix.hw_lut3dC_lutG_val;

        memset(buffer, 0, MAX_LINE_LENGTH);

        snprintf(buffer, MAX_LINE_LENGTH,
                 "%-12s%-12d%-12d%-12d%-12d",
                 "lutG_val:", mat_tmp[9], mat_tmp[82], 
                 mat_tmp[627], mat_tmp[727]);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n\n");

        mat_tmp = ctx->undamped_matrix.hw_lut3dC_lutB_val;

        memset(buffer, 0, MAX_LINE_LENGTH);

        snprintf(buffer, MAX_LINE_LENGTH,
                 "%-12s%-12d%-12d%-12d%-12d",
                 "lutB_val:", mat_tmp[9], mat_tmp[82], 
                 mat_tmp[627], mat_tmp[727]);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n\n");
    }
}
