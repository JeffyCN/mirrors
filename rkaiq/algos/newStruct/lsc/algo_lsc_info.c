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

#include "include/algo_lsc_info.h"

#include "lsc_types_prvt.h"
#include "rk_info_utils.h"

void lsc_dump_mod_param(const RkAiqAlgoCom* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "module param");

    aiq_string_printf(result, "\n\n");
}

void lsc_dump_mod_attr(const RkAiqAlgoCom* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "lsc attr");

    LscContext_t* ctx = (LscContext_t*)(self->ctx);
    lsc_api_attrib_t* tunning = &ctx->lsc_attrib->tunning;

    snprintf(buffer, MAX_LINE_LENGTH, "%-16s", "meshGrid_mode");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-16s",
             tunning->stAuto.sta.lscCfg.sw_lscT_meshGrid_mode > lsc_usrConfig_mode ? 
             "equal" : "userCfg");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void lsc_dump_mod_status(const RkAiqAlgoCom* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "lsc status");

    LscContext_t* ctx = (LscContext_t*)(self->ctx);
    lsc_api_attrib_t* tunning = &ctx->lsc_attrib->tunning;
    alsc_param_illuLink_t *pIlluCase = &tunning->stAuto.dyn.illuLink[ctx->pre_illu_idx];

    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-12s%-12s%-12s%-12s%-12s%-12s",
             "phy_chn", "seq", "cur_rgain", "cur_bgain", "cur_illu", "cur_gain", "cur_vig");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    RkAiqAlgoProcLsc* proc_param = (RkAiqAlgoProcLsc*)self;
    illu_estm_info_t *swinfo = &proc_param->illu_info;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8d%-12d%-12.4f%-12.4f%-12s%-12.1f%-12.2f",
             self->cid, self->frame_id, swinfo->awbGain[0], swinfo->awbGain[1],
             pIlluCase->sw_lscC_illu_name, swinfo->sensorGain, ctx->pre_vignetting);

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
             tunning->stAuto.sta.sw_lscT_damp_en ? "Y" : "N",
             ctx->damp_converged ? "Y" : "N", swinfo->awbIIRDampCoef);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    if(tunning->stAuto.sta.sw_lscT_damp_en) {
        memset(buffer, 0, MAX_LINE_LENGTH);

        snprintf(buffer, MAX_LINE_LENGTH, "%-12s%-8s%-8s%-8s%-8s%-8s%-8s",
                "undampCoef", "lut[0]", "lut[1]", "lut[2]", "lut[3]", "lut[4]", "lut[5]");
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n");

        uint16_t* mat_tmp = ctx->undamped_matrix.hw_lscC_gainR_val;

        memset(buffer, 0, MAX_LINE_LENGTH);

        snprintf(buffer, MAX_LINE_LENGTH,
                 "%-12s%-8d%-8d%-8d%-8d%-8d%-8d",
                 "gainR_val",
                 mat_tmp[0], mat_tmp[1], mat_tmp[2],
                 mat_tmp[3], mat_tmp[4], mat_tmp[5]);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n\n");

        mat_tmp = ctx->undamped_matrix.hw_lscC_gainGr_val;

        memset(buffer, 0, MAX_LINE_LENGTH);

        snprintf(buffer, MAX_LINE_LENGTH,
                 "%-12s%-8d%-8d%-8d%-8d%-8d%-8d",
                 "gainGr_val",
                 mat_tmp[0], mat_tmp[1], mat_tmp[2],
                 mat_tmp[3], mat_tmp[4], mat_tmp[5]);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n\n");

        mat_tmp = ctx->undamped_matrix.hw_lscC_gainGb_val;

        memset(buffer, 0, MAX_LINE_LENGTH);

        snprintf(buffer, MAX_LINE_LENGTH,
                 "%-12s%-8d%-8d%-8d%-8d%-8d%-8d",
                 "gainGb_val",
                 mat_tmp[0], mat_tmp[1], mat_tmp[2],
                 mat_tmp[3], mat_tmp[4], mat_tmp[5]);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n\n");

        mat_tmp = ctx->undamped_matrix.hw_lscC_gainB_val;

        memset(buffer, 0, MAX_LINE_LENGTH);

        snprintf(buffer, MAX_LINE_LENGTH,
                 "%-12s%-8d%-8d%-8d%-8d%-8d%-8d",
                 "gainB_val",
                 mat_tmp[0], mat_tmp[1], mat_tmp[2],
                 mat_tmp[3], mat_tmp[4], mat_tmp[5]);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n\n");
    }
}
