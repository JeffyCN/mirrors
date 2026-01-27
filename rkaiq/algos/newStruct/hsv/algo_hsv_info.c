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

#include "include/algo_hsv_info.h"

#include "hsv_types_prvt.h"
#include "rk_info_utils.h"

void hsv_dump_mod_param(const RkAiqAlgoCom* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "module param");

    HsvContext_t* ctx = (HsvContext_t*)(self->ctx);
    hsv_api_attrib_t* tunning = &ctx->hsv_calib->tunning;

    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-8s%-8s", "lut1d0", "lut1d1", "lut2d");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);

    snprintf(buffer, MAX_LINE_LENGTH,
             "%-8s%-8s%-8s",
             tunning->stAuto.sta.hsvCfg.hw_hsvT_lut1d0_en ? "on" : "off",
             tunning->stAuto.sta.hsvCfg.hw_hsvT_lut1d1_en ? "om" : "off",
             tunning->stAuto.sta.hsvCfg.hw_hsvT_lut2d_en ? "on" : "off");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}
#if RKAIQ_HAVE_HSV_V11
void hsv_v11_dump_mod_param(const RkAiqAlgoCom* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "module param");

    HsvContext_t* ctx = (HsvContext_t*)(self->ctx);
    hsv_api_attrib_t* tunning = &ctx->hsv_calib->tunning;

    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-8s%-8s%-8s%-8s%-8s", "lutmode", "lut1d0", "lut1d1", "lut2d", "lut2d1", "lut2d2");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);

    snprintf(buffer, MAX_LINE_LENGTH,
             "%-10s%-8s%-8s%-8s%-8s%-8s",
             tunning->stAuto.sta.hsvCfg.hw_hsvT_lut_mode ? "1dAnd2d" : "only2d",
             tunning->stAuto.sta.hsvCfg.hw_hsvT_lut1d0_en ? "on" : "off",
             tunning->stAuto.sta.hsvCfg.hw_hsvT_lut1d1_en ? "on" : "off",
             tunning->stAuto.sta.hsvCfg.hw_hsvT_lut2d_en ? "on" : "off",
             tunning->stAuto.sta.hsvCfg.hw_hsvT_lut2d1_en ? "on" : "off",
             tunning->stAuto.sta.hsvCfg.hw_hsvT_lut2d2_en ? "on" : "off");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}
#endif

void hsv_lut1d_dump_mod_attr(int16_t* lut, char* lutname, int mode, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    memset(buffer, 0, MAX_LINE_LENGTH);

    snprintf(buffer, MAX_LINE_LENGTH,
             "%-8s%-8s%-8s%-12d%-12d%-12s",
             lutname,
             mode > 2 ? "Sat" : "Hue",
             mode > 0 ? "Hue" : (mode % 2 ? "Value" : "Sat"),
             lut[4], lut[21], "--");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void hsv_lut2d_dump_mod_attr(int16_t* lut, char* lutname, int mode, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    memset(buffer, 0, MAX_LINE_LENGTH);

    char chl_out[5] = {0};
    char chl_in[5] = {0};
    if (mode >> 2) {
        snprintf(chl_out, 5, "%s", "Val");
        if (mode > 4) {
            snprintf(chl_in, 5, "%s", "SxV");
        } else {
            snprintf(chl_in, 5, "%s", "HxV");
        }
    } else if (mode >> 1) {
        snprintf(chl_out, 5, "%s", "Sat");
        if (mode > 2) {
            snprintf(chl_in, 5, "%s", "HxS");
        } else {
            snprintf(chl_in, 5, "%s", "SxV");
        }
    } else {
        snprintf(chl_out, 5, "%s", "Hue");
        if (mode > 0) {
            snprintf(chl_in, 5, "%s", "HxV");
        } else {
            snprintf(chl_in, 5, "%s", "HxS");
        }
    }

    snprintf(buffer, MAX_LINE_LENGTH,
             "%-8s%-8s%-8s%-12d%-12d%-12d",
             lutname, chl_in, chl_out,
             lut[4], lut[21], lut[177]);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void hsv_dump_mod_attr(const RkAiqAlgoCom* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "hsv attr");

    HsvContext_t* ctx = (HsvContext_t*)(self->ctx);

    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-8s%-8s%-12s%-12s%-12s",
             "--", "chl_in", "chl_out", "lut[4]", "lut[21]", "lut[177]");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    hsv_lut1d_dump_mod_attr(ctx->undamped_lut.lut0, "lut1d0",
                    ctx->calib_lut->lut1d0.hw_hsvT_lut1d_mode, result);

    hsv_lut1d_dump_mod_attr(ctx->undamped_lut.lut1, "lut1d1",
                    ctx->calib_lut->lut1d1.hw_hsvT_lut1d_mode, result);

    hsv_lut2d_dump_mod_attr(ctx->undamped_lut.lut2, "lut2d",
                            ctx->calib_lut->lut2d.hw_hsvT_lut2d_mode,
                            result);
#if RKAIQ_HAVE_HSV_V11
    hsv_lut2d_dump_mod_attr(ctx->undamped_lut.lut2d1, "lut2d1",
                            ctx->calib_lut->lut2d1.hw_hsvT_lut2d_mode,
                            result);

    hsv_lut2d_dump_mod_attr(ctx->undamped_lut.lut2d2, "lut2d2",
                            ctx->calib_lut->lut2d2.hw_hsvT_lut2d_mode,
                            result);
#endif
}

void hsv_dump_mod_status(const RkAiqAlgoCom* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "hsv status");

    HsvContext_t* ctx = (HsvContext_t*)(self->ctx);
    hsv_api_attrib_t* tunning = &ctx->hsv_calib->tunning;
    ahsv_param_illuLink_t *pIlluCase = &tunning->stAuto.dyn.illuLink[ctx->pre_illu_idx];

    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-12s%-12s%-12s%-12s%-12s%-12s",
             "phy_chn", "seq", "cur_rgain", "cur_bgain", "cur_illu", "cur_gain", "cur_alp");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    RkAiqAlgoProcHsv* proc_param = (RkAiqAlgoProcHsv*)self;
    illu_estm_info_t *swinfo = &proc_param->illu_info;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8d%-12d%-12.4f%-12.4f%-12s%-12.1f%-12.2f",
             self->cid, self->frame_id, swinfo->awbGain[0], swinfo->awbGain[1],
             pIlluCase->sw_hsvC_illu_name, swinfo->sensorGain, ctx->pre_alpha);

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
             tunning->stAuto.sta.sw_hsvT_damp_en ? "Y" : "N",
             ctx->damp_converged ? "Y" : "N", swinfo->awbIIRDampCoef);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}
