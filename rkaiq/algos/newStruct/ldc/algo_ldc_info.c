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

#include "include/algo_ldc_info.h"

#include "include/ldc_algo_adaptor.h"
#include "ldc_types_prvt.h"
#include "rk_info_utils.h"

void ldc_dump_mod_param(const RkAiqAlgoCom* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "module param");

    LdcContext_t* ctx   = (LdcContext_t*)(self->ctx);
    LdcAlgoAdaptor* adp = (LdcAlgoAdaptor*)(ctx->handler);

    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-8s", "ldch", "ldcv");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-8s", adp->ldch_en_ ? "on" : "off",
             adp->ldcv_en_ ? "on" : "off");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void ldc_dump_mod_attr(const RkAiqAlgoCom* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "ldc attr");

    LdcContext_t* ctx   = (LdcContext_t*)(self->ctx);
    LdcAlgoAdaptor* adp = (LdcAlgoAdaptor*)(ctx->handler);

    snprintf(buffer, MAX_LINE_LENGTH, "%-17s%-17s", "opticCenter_x", "opticCenter_y");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    ldc_lensDistorCoeff_static_t* lensDistorCoeff =
        &adp->calib->tunning.autoGenMesh.lensDistorCoeff;
    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-17.10f%-17.10f", lensDistorCoeff->sw_ldcC_opticCenter_x,
             lensDistorCoeff->sw_ldcC_opticCenter_y);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-24s%-24s%-24s%-24s", "coeff_0", "coeff_1", "coeff_2",
             "coeff_3");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(
        buffer, MAX_LINE_LENGTH, "%-24.16f%-24.16f%-24.10f%-24.16f",
        lensDistorCoeff->sw_ldcC_lensDistor_coeff[0], lensDistorCoeff->sw_ldcC_lensDistor_coeff[1],
        lensDistorCoeff->sw_ldcC_lensDistor_coeff[2], lensDistorCoeff->sw_ldcC_lensDistor_coeff[3]);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void ldc_dump_mod_status(const RkAiqAlgoCom* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "ldc status");

    LdcContext_t* ctx   = (LdcContext_t*)(self->ctx);
    LdcAlgoAdaptor* adp = (LdcAlgoAdaptor*)(ctx->handler);

    snprintf(buffer, MAX_LINE_LENGTH, "%-9s%-12s%-8s", "phy_chn", "seq", "correct_level");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-9d%-12d%-8d", self->cid, self->frame_id,
             adp->correct_level_);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}
