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

#include "aiq_coreIspModsInfo.h"

#include "aiq_core.h"
#include "info/aiq_coreAlgosInfo.h"
#include "info/aiq_coreIspParamsInfo.h"
#include "rk_info_utils.h"
#include "st_string.h"

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

#define CORE_DUMP_MOD_INFO(dumper, algo_type, algo_res_type, result) \
    do {                                                             \
        core_alogs_dump_by_type(dumper, algo_type, result);          \
        core_isp_params_dump_by_type(dumper, algo_res_type, result); \
    } while (0)

int core_isp_mods_dump_debayer(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_ADEBAYER, RESULT_TYPE_DEBAYER_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_ae(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_AE, RESULT_TYPE_AEC_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_aestats(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_AE, RESULT_TYPE_AESTATS_PARAM, result);
    return 0;
}

static int core_isp_mods_dump_awb(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_AWB, RESULT_TYPE_AWB_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_awbgain(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_AWB, RESULT_TYPE_AWBGAIN_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_af(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_AF, RESULT_TYPE_AF_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_ccm(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_ACCM, RESULT_TYPE_CCM_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_tnr(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_AMFNR, RESULT_TYPE_TNR_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_ynr(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_AYNR, RESULT_TYPE_YNR_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_uvnr(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_ACNR, RESULT_TYPE_CNR_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_merge(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_AMERGE, RESULT_TYPE_MERGE_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_histeq(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_AHISTEQ, RESULT_TYPE_HISTEQ_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_sharp(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_ASHARP, RESULT_TYPE_SHARPEN_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_blc(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_ABLC, RESULT_TYPE_BLC_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_csm(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_ACSM, RESULT_TYPE_CSM_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_dpcc(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_ADPCC, RESULT_TYPE_DPCC_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_gamma(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_AGAMMA, RESULT_TYPE_AGAMMA_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_lsc(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_ALSC, RESULT_TYPE_LSC_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_drc(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_ADRC, RESULT_TYPE_DRC_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_gic(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_AGIC, RESULT_TYPE_GIC_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_cgc(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_ACGC, RESULT_TYPE_CGC_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_ie(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_AIE, RESULT_TYPE_IE_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_gain(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_AGAIN, RESULT_TYPE_GAIN_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_dhaz(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_ADHAZ, RESULT_TYPE_DEHAZE_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_rgbir(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_ARGBIR, RESULT_TYPE_RGBIR_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_cp(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_ACP, RESULT_TYPE_CP_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_lut3d(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_A3DLUT, RESULT_TYPE_LUT3D_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_ldc(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_ALDC, RESULT_TYPE_LDC_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_cac(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_ACAC, RESULT_TYPE_CAC_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_aibnr(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_AIBNR, RESULT_TYPE_AIBNR_PARAM, result);

    return 0;
}

static int core_isp_mods_dump_aiynr(void* dumper, st_string* result, int argc, void* argv[]) {
    CORE_DUMP_MOD_INFO(dumper, RK_AIQ_ALGO_TYPE_AIYNR, RESULT_TYPE_AIYNR_PARAM, result);

    return 0;
}

static struct mod_dump_info mod_dump[] = {
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_AEC, core_isp_mods_dump_aestats),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_AWB, core_isp_mods_dump_awb),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_DEBAYER, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_AF, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_CCM, core_isp_mods_dump_ccm),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_TNR, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_YNR, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_CNR, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_MERGE, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_HISTEQ, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_SHARPEN, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_BLC, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_CSM, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_DPCC, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_AGAMMA, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_LSC, core_isp_mods_dump_lsc),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_DRC, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_GIC, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_CGC, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_IE, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_GAIN, NULL),
#if defined(ISP_HW_V39)
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_DEHAZE, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_RGBIR, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_CP, NULL),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_LUT3D, core_isp_mods_dump_lut3d),
#endif
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_LDC, core_isp_mods_dump_ldc),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_CAC, core_isp_mods_dump_cac),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_AIBNR, core_isp_mods_dump_aibnr),
    MOD_DUMP_INFO(AIQ_NOTIFIER_MATCH_AIYNR, core_isp_mods_dump_aiynr),
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif
#define MOD_DUMP_SIZE ARRAY_SIZE(mod_dump)

void core_isp_mods_add_subscriber(AiqCore_t* pAiqCore) {
    if (!pAiqCore) return;

    for (size_t i = 0; i < MOD_DUMP_SIZE; i++) {
        if (mod_dump[i].dump) {
            snprintf(mod_dump[i].name, 32, "ALGO -> %s", NotifierMatchType2Str[mod_dump[i].type]);

            pAiqCore->sub_isp_mods[i].match_type     = mod_dump[i].type;
            pAiqCore->sub_isp_mods[i].name           = mod_dump[i].name;
            pAiqCore->sub_isp_mods[i].dump.dump_fn_t = mod_dump[i].dump;
            pAiqCore->sub_isp_mods[i].dump.dumper    = pAiqCore;

            aiq_notifier_add_subscriber(&pAiqCore->notifier, &pAiqCore->sub_isp_mods[i]);
        }
    }
}
