/*
 * Copyright (c) 2021-2022 Rockchip Eletronics Co., Ltd.
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "sample_comm.h"
#include "sample_misc_modules.h"
#include "sample_ae_module.h"
#include "sample_awb_module.h"
#include "sample_accm_module.h"
#if USE_NEWSTRUCT && !defined(ISP_HW_V39)
#include "sample_hsv_module.h"
#else
#include "sample_a3dlut_module.h"
#endif
#include "sample_adrc_module.h"
#include "sample_amerge_module.h"
#include "sample_agamma_module.h"
#include "sample_adehaze_module.h"
#include "sample_af_module.h"
#include "sample_image_process.h"
#include "sample_aynr_module.h"
#include "sample_acnr_module.h"
#include "sample_asharp_module.h"
#include "sample_abayer2dnr_module.h"
#include "sample_abayertnr_module.h"
#include "sample_csm_module.h"
#include "sample_ablc_module.h"
#include "sample_agic_module.h"
#include "sample_aldc_module.h"
#ifndef USE_NEWSTRUCT
#include "sample_aldch_module.h"
#include "sample_aldch_v21_module.h"
#endif
#include "sample_adebayer_module.h"
#include "sample_alsc_module.h"
#include "sample_acp_module.h"
#include "sample_aie_module.h"
#include "sample_adpcc_module.h"
#include "sample_cgc_module.h"
#include "sample_cac_module.h"
#include "sample_again_module.h"
#include "sample_smartIr.h"
#include "sample_ainr_module.h"
#include "sample_trans_module.h"
#include "sample_rgbir_module.h"
#include "sample_sysctl.h"
#include "sample_aibnr_module.h"
#include "sample_aiynr_module.h"

struct module_sample_info {
    const char * const name;
    XCamReturn (*func)(const void *arg);
    void (*debug)(const void *arg);
};

#define MODULE_INFO(_modele_id, _func, _debug)              \
    [_modele_id] = {                                        \
        .name = #_modele_id,                                \
        .func = _func,                                      \
        .debug = _debug,                                    \
    }

static struct module_sample_info module_samples[] = {
    MODULE_INFO(RK_ISP_AE, sample_ae_module, sample_print_ae_info),
    MODULE_INFO(RK_ISP_AWB, sample_awb_module, sample_print_awb_info),
    MODULE_INFO(RK_ISP_AWB32, sample_awb_module, sample_print_awb_info),  
    MODULE_INFO(RK_ISP_AF, sample_af_module, sample_print_af_info),
#ifdef USE_NEWSTRUCT
    MODULE_INFO(RK_ISP_ACCM, sample_accm_v3_module, sample_print_accm_info),
#else
    MODULE_INFO(RK_ISP_ACCM, sample_accm_module, sample_print_accm_info),
#endif
#if USE_NEWSTRUCT && !defined(ISP_HW_V39)
    MODULE_INFO(RK_ISP_A3DLUT, sample_hsv_module, sample_print_hsv_info),
#else
    MODULE_INFO(RK_ISP_A3DLUT, sample_a3dlut_module, sample_print_a3dlut_info),
#endif
    MODULE_INFO(RK_ISP_ADRC, sample_adrc_module, sample_print_adrc_info),
    MODULE_INFO(RK_ISP_AMERGE, sample_amerge_module, sample_print_amerge_info),
    MODULE_INFO(RK_ISP_AGAMMA, sample_agamma_module, sample_print_agamma_info),
    MODULE_INFO(RK_ISP_ADEHAZE, sample_adehaze_module, sample_print_adehaze_info),
    MODULE_INFO(RK_ISP_AYNR, sample_aynr_module, sample_print_aynr_info),
    MODULE_INFO(RK_ISP_ACNR, sample_acnr_module, sample_print_acnr_info),
    MODULE_INFO(RK_ISP_ASHARP, sample_asharp_module, sample_print_asharp_info),
    MODULE_INFO(RK_ISP_ABAYER2DNR, sample_abayer2dnr_module, sample_print_abayer2dnr_info),
    MODULE_INFO(RK_ISP_ABAYERTNR, sample_abayertnr_module, sample_print_abayertnr_info),
    MODULE_INFO(RK_ISP_CSM, sample_csm_module, sample_print_csm_info),
    MODULE_INFO(RK_ISP_ABLC, sample_ablc_module, sample_print_ablc_info),
    MODULE_INFO(RK_ISP_AGIC, sample_agic_module, sample_print_agic_info),
#ifndef USE_NEWSTRUCT
    MODULE_INFO(RK_ISP_ALDCH, sample_aldch_module, sample_print_aldch_info),
#endif
    MODULE_INFO(RK_ISP_ADEBAYER, sample_adebayer_module, sample_print_adebayer_info),
    MODULE_INFO(RK_ISP_ACP, sample_acp_module, sample_print_acp_info),
    MODULE_INFO(RK_ISP_AIE, sample_aie_module, sample_print_aie_info),
    MODULE_INFO(RK_ISP_CGC, sample_cgc_module, sample_print_cgc_info),
    MODULE_INFO(RK_ISP_CAC, sample_cac_module, sample_print_cac_info),
    MODULE_INFO(RK_ISP_AGAIN, sample_again_module, sample_print_again_info),
    MODULE_INFO(RK_ISP_ACCM_V2, sample_accm_v2_module, sample_print_accm_v2_info),
#ifndef USE_NEWSTRUCT
    MODULE_INFO(RK_ISP_ALDCH_V21, sample_aldch_v21_module, sample_print_aldch_v21_info),
#endif
    MODULE_INFO(RK_SMARTIR, sample_smartIr_module, sample_print_smartIr_info),
    MODULE_INFO(RK_ISP_ALDC, sample_aldc_module, sample_print_aldc_info),
};

static void sample_usage()
{
    CLEAR ();

    printf("Usage : \n");
    printf("\t 0) AE:         module test sample.\n");
    printf("\t 1) AWB:        module test sample.\n");
    printf("\t 2) AF:         module test sample.\n");
    printf("\t 3) ACCM:       module test sample.\n");
    printf("\t 4) A3DLUT:     module test sample.\n");
    printf("\t 5) ADRC:       module test sample.\n");
    printf("\t 6) AMERGE:     module test sample.\n");
    printf("\t 7) AGAMMA:     module test sample.\n");
    printf("\t 8) ADEHAZE:    module test sample.\n");
    printf("\t 9) AYNR:       module test sample.\n");
    printf("\t a) ACNR:       module test sample.\n");
    printf("\t b) ASHARP:     module test sample.\n");
    printf("\t c) ABAYER2DNR: module test sample.\n");
    printf("\t d) ABAYERTNR:  module test sample.\n");
    printf("\t e) CSM:        module test sample.\n");
    printf("\t f) ABLC:       module test sample.\n");
    printf("\t g) AGIC:       module test sample.\n");
#ifndef USE_NEWSTRUCT
    printf("\t h) ALDCH:      module test sample.\n");
#endif
    printf("\t i) ADEBAYER:   module test sample.\n");
    printf("\t j) ALSC:       module test sample.\n");
    printf("\t k) ACP:        module test sample.\n");
    printf("\t l) AIE:        module test sample.\n");
    printf("\t m) CGC:        module test sample.\n");
    printf("\t n) CAC:        module test sample.\n");
    printf("\t o) AGAIN:      module test sample.\n");
    printf("\t p) AWBV32:     module test sample.\n");
    printf("\t r) CCMV2:      module test sample.\n");
#ifndef USE_NEWSTRUCT
    printf("\t s) ALDCHV32:  module test sample.\n");
#endif
    printf("\t t) SMARTIR:    module test sample.\n");
    printf("\t u) DPCC:       module test sample.\n");
    printf("\t v) AINR:       module test sample.\n");
    printf("\t w) TRANS:       module test sample.\n");
    printf("\t x) ALDC:       module test sample.\n");
    printf("\t y) RGBIR:       module test sample.\n");
    printf("\t A) SYSCTL:       module test sample.\n");
    printf("\t B) AIBNR:      module test sample.\n");
    printf("\t C) AIYNR:      module test sample.\n");
    printf("\t z) ALL:       module test sample.\n");
    printf("\n");
    printf("\t please press the key: ");

    return;
}

XCamReturn sample_main (const void *arg)
{
    if (arg == NULL) {
        ERR ("%s, arg is NULL\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    sample_usage ();

    int key = getchar ();
    while (key == '\n' || key == '\r')
        key = getchar ();

    switch (key)
    {
    case '0': {
        struct module_sample_info *info = &module_samples[RK_ISP_AE];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case '1': {
        struct module_sample_info *info = &module_samples[RK_ISP_AWB];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case '2': {
        struct module_sample_info *info = &module_samples[RK_ISP_AF];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case '3': {
        struct module_sample_info *info = &module_samples[RK_ISP_ACCM];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case '4': {
        struct module_sample_info *info = &module_samples[RK_ISP_A3DLUT];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case '5': {
        printf("enter DRC module test\n");
        struct module_sample_info *info = &module_samples[RK_ISP_ADRC];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case '6': {
        struct module_sample_info *info = &module_samples[RK_ISP_AMERGE];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case '7': {
        struct module_sample_info *info = &module_samples[RK_ISP_AGAMMA];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case '8': {
        struct module_sample_info *info = &module_samples[RK_ISP_ADEHAZE];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case '9': {
        printf("enter AYNR module test\n");
        struct module_sample_info *info = &module_samples[RK_ISP_AYNR];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case 'a': {
        printf("enter ACNR module test\n");
        struct module_sample_info *info = &module_samples[RK_ISP_ACNR];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case 'b': {
        printf("enter ASHARP module test\n");
        struct module_sample_info *info = &module_samples[RK_ISP_ASHARP];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case 'c': {
        printf("enter ABAYER2DNR module test\n");
        struct module_sample_info *info = &module_samples[RK_ISP_ABAYER2DNR];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case 'd': {
        printf("enter ABAYERTNR module test\n");
        struct module_sample_info *info = &module_samples[RK_ISP_ABAYERTNR];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case 'e': {
        printf("enter CSM module test\n");
        struct module_sample_info *info = &module_samples[RK_ISP_CSM];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case 'f': {
        printf("enter ABLC module test\n");
        struct module_sample_info *info = &module_samples[RK_ISP_ABLC];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case 'g': {
        printf("enter AGIC module test\n");
        struct module_sample_info *info = &module_samples[RK_ISP_AGIC];
        info->debug (NULL);
        info->func (arg);
        break;
    }
#ifndef USE_NEWSTRUCT
    case 'h': {
        struct module_sample_info *info = &module_samples[RK_ISP_ALDCH];
        info->debug (NULL);
        info->func (arg);
        break;
    }
#endif
    case 'i': {
        struct module_sample_info *info = &module_samples[RK_ISP_ADEBAYER];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case 'j': {
        printf("enter LSC module test\n");
        sample_alsc_module(arg);
        break;
    }
    case 'k': {
        struct module_sample_info *info = &module_samples[RK_ISP_ACP];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case 'l': {
        struct module_sample_info *info = &module_samples[RK_ISP_AIE];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case 'm': {
        printf("enter CGC module test\n");
        struct module_sample_info *info = &module_samples[RK_ISP_CGC];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case 'n': {
        struct module_sample_info *info = &module_samples[RK_ISP_CAC];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case 'o': {
        printf("enter GAIN module test\n");
        struct module_sample_info *info = &module_samples[RK_ISP_AGAIN];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case 'r': {
        struct module_sample_info *info = &module_samples[RK_ISP_ACCM_V2];
        info->debug (NULL);
        info->func (arg);
        break;
    }
#ifndef USE_NEWSTRUCT
    case 's': {
        struct module_sample_info *info = &module_samples[RK_ISP_ALDCH_V21];
        info->debug (NULL);
        info->func (arg);
        break;
    }
#endif
    case 't': {
        struct module_sample_info *info = &module_samples[RK_SMARTIR];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case 'u': {
        printf("enter DPCC module test\n");
        sample_adpcc_module(arg);
        break;
    }
    case 'v': {
        printf("enter AINR module test\n");
        sample_ainr_module(arg);
        break;
    }
    case 'w': {
        printf("enter TRANS module test\n");
        sample_trans_module(arg);
        break;
    }
    case 'x': {
        struct module_sample_info *info = &module_samples[RK_ISP_ALDC];
        info->debug (NULL);
        info->func (arg);
        break;
    }
    case 'y': {
        printf("enter RGBIR module test\n");
        sample_rgbir_module(arg);
        break;
    }
    case 'A': {
        printf("enter SYSCTL test\n");
        sample_sysctl(arg);
        break;
    }
    case 'B': {
        printf("enter AIBNR module test\n");
        sample_aibnr_module(arg);
        break;
    }
    case 'C': {
        printf("enter AIYNR module test\n");
        sample_aiynr_module(arg);
        break;
    }
    case 'z': {
        printf("enter all module test\n");
        const demo_context_t *demo_ctx = (demo_context_t *)arg;
        const rk_aiq_sys_ctx_t* ctx ;
        if (demo_ctx->camGroup) {
            ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->camgroup_ctx);
        } else {
            ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->aiq_ctx);
        } 
        #if USE_NEWSTRUCT
        for (int i = 0;i < 1000;i++) {
            // sample_ae_case(ctx);
            // sample_awb_case(ctx);
            // sample_af_case(ctx);
            // sample_3dlut_test(ctx);
            // sample_btnr_test(ctx);
            sample_new_blc(ctx);
            sample_ccm_test(ctx);
            // sample_cnr_reverseEn(ctx);
            sample_cp_test(ctx);
            sample_dm_test(ctx);
            // sample_new_dehaze(ctx);
            sample_dpc_test(ctx);
            sample_new_drc(ctx);
            sample_gain_test(ctx);
            sample_agamma_test(ctx);
            // sample_gic_test(ctx);
            sample_new_ie(ctx);
#if RKAIQ_HAVE_LDCH_V21
            sample_ldch_test(ctx);
#endif
            sample_lsc_test(ctx);
            sample_merge_test(ctx);
            // sample_sharp_reverseEn(ctx);
            // sample_ynr_test(ctx);
            // sample_cac_test(ctx);
            sample_cgc_test(ctx);
            sample_new_csm(ctx);
            // sample_new_rgbir(ctx);
            // sample_new_trans(ctx);
            
        }
        #endif
        break;
    }
    default:
        break;
    }

    return XCAM_RETURN_NO_ERROR;
}
