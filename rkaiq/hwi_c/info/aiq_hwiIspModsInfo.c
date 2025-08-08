/*
 *   Copyright (c) 2024 Rockchip CorporatiY
 *
 * Licensed under the Apache License, VersiY 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed Y an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CYDITIYS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissiYs and
 * limitatiYs under the License.
 *
 */

#include "aiq_hwiIspModsInfo.h"

#include "aiq_ispActiveParamsInfo.h"
#include "aiq_ispParamsCvtInfo.h"
#include "dumpcam_server/info/include/rk_info_utils.h"
#include "dumpcam_server/info/include/st_string.h"
#include "hwi_c/aiq_CamHwBase.h"

#define MOD_DUMP_INFO(_type, _func) \
    [_MODS_NUM(_type)] = {          \
        .type = _type,              \
        .dump = _func,              \
    }

struct mod_dump_info {
    enum aiq_notifier_match_type type;
    char name[32];
    int (*dump)(void* dumper, st_string* result, int argc, void* argv[]);
};

#define HWI_DUMP_MOD_INFO(dumper, type, result)               \
    do {                                                      \
        cvt_isp_params_dump_by_type(dumper, type, result);    \
        active_isp_params_dump_by_type(dumper, type, result); \
    } while (0)

int hwi_isp_mods_dump_debayer(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_AWB_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_ae(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_AEC_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_aestats(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_AESTATS_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_awb(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_AWB_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_awbgain(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_AWBGAIN_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_af(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_AF_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_ccm(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_CCM_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_tnr(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_TNR_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_ynr(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_YNR_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_uvnr(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_CNR_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_merge(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_MERGE_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_histeq(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_HISTEQ_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_sharp(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_SHARPEN_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_blc(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_BLC_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_csm(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_CSM_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_dpcc(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_DPCC_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_gamma(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_AGAMMA_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_lsc(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_LSC_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_drc(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_LSC_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_gic(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_GIC_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_cgc(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_CGC_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_ie(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_IE_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_gain(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_GAIN_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_dhaz(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_DEHAZE_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_rgbir(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_RGBIR_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_cp(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_CP_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_lut3d(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_LUT3D_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_ldc(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_LDC_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_cac(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_CAC_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_aibnr(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_AIBNR_PARAM, result);

    return 0;
}

static int hwi_isp_mods_dump_aiynr(void* dumper, st_string* result, int argc, void* argv[]) {
    HWI_DUMP_MOD_INFO(dumper, RESULT_TYPE_AIYNR_PARAM, result);

    return 0;
}

static struct mod_dump_info mod_dump[] = {
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_DEBAYER, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_AEC, hwi_isp_mods_dump_aestats),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_AWB, hwi_isp_mods_dump_awb),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_AWBGAIN, hwi_isp_mods_dump_awbgain),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_AF, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_CCM, hwi_isp_mods_dump_ccm),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_TNR, hwi_isp_mods_dump_tnr),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_YNR, hwi_isp_mods_dump_ynr),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_CNR, hwi_isp_mods_dump_uvnr),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_MERGE, hwi_isp_mods_dump_merge),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_HISTEQ, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_SHARPEN, hwi_isp_mods_dump_sharp),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_BLC, hwi_isp_mods_dump_blc),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_CSM, hwi_isp_mods_dump_csm),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_DPCC, hwi_isp_mods_dump_dpcc),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_AGAMMA, hwi_isp_mods_dump_gamma),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_LSC, hwi_isp_mods_dump_lsc),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_DRC, hwi_isp_mods_dump_drc),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_GIC, hwi_isp_mods_dump_gic),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_CGC, hwi_isp_mods_dump_cgc),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_IE, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_GAIN, hwi_isp_mods_dump_gain),
#if defined(ISP_HW_V39)
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_DEHAZE, hwi_isp_mods_dump_dhaz),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_RGBIR, hwi_isp_mods_dump_rgbir),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_CP, hwi_isp_mods_dump_cp),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_LUT3D, hwi_isp_mods_dump_lut3d),
#endif
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_LDC, hwi_isp_mods_dump_ldc),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_CAC, hwi_isp_mods_dump_cac),
#if defined(ISP_HW_V35)
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_AIBNR, hwi_isp_mods_dump_aibnr),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_AIYNR, hwi_isp_mods_dump_aiynr),
#endif
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif
#define MOD_DUMP_SIZE ARRAY_SIZE(mod_dump)

void hwi_isp_mods_add_subscriber(AiqCamHwBase_t* pCamHw) {
    if (!pCamHw) return;

    for (size_t i = 0; i < MOD_DUMP_SIZE; i++) {
        if (mod_dump[i].dump) {
            snprintf(mod_dump[i].name, 32, "HWI -> %s", NotifierMatchType2Str[mod_dump[i].type]);

            pCamHw->sub_isp_mods[i].match_type     = mod_dump[i].type;
            pCamHw->sub_isp_mods[i].name           = mod_dump[i].name;
            pCamHw->sub_isp_mods[i].dump.dump_fn_t = mod_dump[i].dump;
            pCamHw->sub_isp_mods[i].dump.dumper    = pCamHw;

            aiq_notifier_add_subscriber(&pCamHw->notifier, &pCamHw->sub_isp_mods[i]);
        }
    }
}
