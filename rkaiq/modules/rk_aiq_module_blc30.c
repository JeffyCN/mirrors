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
 */

#include "algo_types_priv.h"
#include "rk_aiq_isp39_modules.h"
#include "c_base/aiq_base.h"

void rk_aiq_blc30_params_cvt(void* attr, isp_params_t* isp_params,
                             common_cvt_info_t* cvtinfo, blc_cvt_info_t* pBlcInfo, rk_aiq_wb_gain_v32_t* awb_gain_final)
{
#if ISP_HW_V35
    struct isp35_bls_cfg* phwcfg = &isp_params->isp_cfg->others.bls_cfg;
#else
    struct isp32_bls_cfg* phwcfg = &isp_params->isp_cfg->others.bls_cfg;
#endif
    blc_params_dyn_t* pdyn = &((blc_param_t*)attr)->dyn;
    rk_aiq_isp_blc_v33_t* blc_res = (rk_aiq_isp_blc_v33_t*)attr;
    int isp_ob_max = 0xfff;
    float pre_dgain = 1.0;
    float isp_dgain = 1.0;
    uint16_t autoOB_offset = CLIP(pdyn->obcPostTnr.sw_blcT_autoOB_offset, 0, 511);

#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
    blc_params_static_t* psta = &((blc_param_t*)attr)->sta;
    bool autoblc_en = psta->autoBlc.sw_blcT_autoBlc_en & pdyn->obcPostTnr.sw_blcT_obcPostTnr_en &
                      pBlcInfo->init_success & (cvtinfo->frameNum == 1) & cvtinfo->btnr_en;
    float damping_val = CLIP(psta->autoBlc.sw_blcT_damping_val, 0, 1);
    float totalGain = cvtinfo->ae_exp->LinearExp.exp_real_params.analog_gain
                      * cvtinfo->ae_exp->LinearExp.exp_real_params.digital_gain
                      * cvtinfo->ae_exp->LinearExp.exp_real_params.isp_dgain;
    int number = totalGain, gain_index = 0;
    while (number > 0) {
        number >>= 1;
        ++gain_index;
    }
    struct blcOps* ops;
    if (cvtinfo->isFirstFrame) {
        AiqBlc_Init(&pBlcInfo->lib_Blc_);
        char lib_info[128];
        if (!pBlcInfo->lib_Blc_.Init(&pBlcInfo->lib_Blc_)) {
            strcpy(lib_info, "blcLibrary init failed! Please check if 'libautoblc.so' exists in the system");
            pBlcInfo->init_success = false;
            goto lib_init_fail;
        }
        if (!pBlcInfo->lib_Blc_.LoadSymbols(&pBlcInfo->lib_Blc_)) {
            strcpy(lib_info, "blcLibrary LoadSymbols failed! Please check the correctness of 'libatuoblc.so'");
            pBlcInfo->init_success = false;
            goto lib_init_fail;
        }
        pBlcInfo->init_success = true;
        ops = AiqBlc_GetOps(&pBlcInfo->lib_Blc_);
        pBlcInfo->blc1_param.width = cvtinfo->rawWidth;
        pBlcInfo->blc1_param.height = cvtinfo->rawHeight;
        pBlcInfo->blc1_param.bayertnr_itransf_tbl = pBlcInfo->bayertnr_itransf_tbl;
        pBlcInfo->blc1_param.blc_adjust = pBlcInfo->blc_adjust;
        memset(&pBlcInfo->blc_adjust, 0, sizeof(pBlcInfo->blc_adjust));
        *blc_res->damping = false;
        ops->blc_init(&pBlcInfo->blc1_param);
        goto lib_init_success;
lib_init_fail:
        LOGE_ABLC("%s", lib_info);
lib_init_success:
        ;
    }
    else if (*blc_res->aeIsConverged && autoblc_en && totalGain >= psta->autoBlc.sw_blcT_autoBlcEn_thred) {
        *blc_res->damping = true;
        if (pBlcInfo->sigma_ratio > 0.7) {
            if (pBlcInfo->autoblc_count > 0 && (pBlcInfo->autoblc_count - 1) % 8 == 0) {
                ops = AiqBlc_GetOps(&pBlcInfo->lib_Blc_);
                pBlcInfo->blc1_param.isp_ob = autoOB_offset;

                pBlcInfo->blc1_param.rgain = awb_gain_final->rgain * 1024;
                pBlcInfo->blc1_param.bgain = awb_gain_final->bgain * 1024;
                pBlcInfo->blc1_param.grgain = awb_gain_final->grgain * 1024;
                pBlcInfo->blc1_param.gbgain = awb_gain_final->gbgain * 1024;

                pBlcInfo->blc1_param.bayertnr_autoblc_thre = CLIP(pdyn->obcPostTnr.autoBlc.sw_blcT_darkArea_thred, 0, 4095);
                if ((pdyn->obcPostTnr.autoBlc.sw_blcT_lumaR_wgt + pdyn->obcPostTnr.autoBlc.sw_blcT_lumaG_wgt + pdyn->obcPostTnr.autoBlc.sw_blcT_lumaB_wgt) == 1024) {
                    pBlcInfo->blc1_param.bayertnr_autoblc_rfact = pdyn->obcPostTnr.autoBlc.sw_blcT_lumaR_wgt;
                    pBlcInfo->blc1_param.bayertnr_autoblc_gfact = pdyn->obcPostTnr.autoBlc.sw_blcT_lumaG_wgt;
                    pBlcInfo->blc1_param.bayertnr_autoblc_bfact = pdyn->obcPostTnr.autoBlc.sw_blcT_lumaB_wgt;
                }
                else {
                    pBlcInfo->blc1_param.bayertnr_autoblc_rfact = 256;
                    pBlcInfo->blc1_param.bayertnr_autoblc_gfact = 512;
                    pBlcInfo->blc1_param.bayertnr_autoblc_bfact = 256;
                }
                pBlcInfo->blc1_param.ptr_rgb_pack = (unsigned char*)pBlcInfo->tnr_ds_buf;

                ops->blc_proc(&pBlcInfo->blc1_param);
            }
            pBlcInfo->autoblc_count++;
        }
    }
    else {
        pBlcInfo->autoblc_count = 0;
        *blc_res->damping = false;
    }
#endif

    pre_dgain = cvtinfo->preDGain;
    isp_dgain = cvtinfo->frameDGain[0];

    phwcfg->enable_auto = 0;
    phwcfg->en_windows  = 0;

    phwcfg->bls_window1.h_offs = 0;
    phwcfg->bls_window1.v_offs = 0;
    phwcfg->bls_window1.h_size = 0;
    phwcfg->bls_window1.v_size = 0;

    phwcfg->bls_window2.h_offs = 0;
    phwcfg->bls_window2.v_offs = 0;
    phwcfg->bls_window2.h_size = 0;
    phwcfg->bls_window2.v_size = 0;

    phwcfg->bls_samples = 0;

    // blc0
    phwcfg->fixed_val.r  = (short)pdyn->obcPreTnr.hw_blcC_obR_val;
    phwcfg->fixed_val.gr = (short)pdyn->obcPreTnr.hw_blcC_obGr_val;
    phwcfg->fixed_val.gb = (short)pdyn->obcPreTnr.hw_blcC_obGb_val;
    phwcfg->fixed_val.b  = (short)pdyn->obcPreTnr.hw_blcC_obB_val;

    // pre_dgain
    phwcfg->isp_ob_predgain = CLIP((int)(pre_dgain * (1 << 8)), 0, 65535);
    // blc1 aiq 6.0 does not support manualOBC, set to 0.
    phwcfg->bls1_val.r = 0;
    phwcfg->bls1_val.gr = 0;
    phwcfg->bls1_val.gb = 0;
    phwcfg->bls1_val.b = 0;

    if (pdyn->obcPostTnr.sw_blcT_obcPostTnr_en) {
        phwcfg->bls1_en = 1;
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
        int blc1_r = 0;
        int blc1_b = 0;
        if (pBlcInfo->autoblc_count <= 2 && autoblc_en && totalGain >= psta->autoBlc.sw_blcT_autoBlcEn_thred) {
            pBlcInfo->cur_adjust[0] = pBlcInfo->blc_adjust_tbl[gain_index * 2];
            pBlcInfo->cur_adjust[1] = pBlcInfo->blc_adjust_tbl[gain_index * 2 + 1];
            blc1_r = (int)pBlcInfo->cur_adjust[0];
            blc1_b = (int)pBlcInfo->cur_adjust[1];
        }
        if (*blc_res->aeIsConverged && autoblc_en && totalGain >= psta->autoBlc.sw_blcT_autoBlcEn_thred && pBlcInfo->autoblc_count > 1) {
            float r_add, b_add;
            r_add = (1 - damping_val) * pBlcInfo->cur_adjust[0] + (damping_val) * (float)(pBlcInfo->blc_adjust[0]);
            b_add = (1 - damping_val) * pBlcInfo->cur_adjust[1] + (damping_val) * (float)(pBlcInfo->blc_adjust[1]);
            pBlcInfo->cur_adjust[0] = r_add;
            pBlcInfo->cur_adjust[1] = b_add;
            blc1_r = (int)pBlcInfo->cur_adjust[0];
            blc1_b = (int)pBlcInfo->cur_adjust[1];
            LOGD_ABLC("target blc1 adjust %d %d, cur adjust %f %f, autoblc_count %d\n",
                      pBlcInfo->blc_adjust[0], pBlcInfo->blc_adjust[1], pBlcInfo->cur_adjust[0], pBlcInfo->cur_adjust[1],
                      pBlcInfo->autoblc_count);
            if (ABS(pBlcInfo->cur_adjust[0] - (float)pBlcInfo->blc_adjust[0]) > 0.5
                    || ABS(pBlcInfo->cur_adjust[1] - (float)pBlcInfo->blc_adjust[1]) > 0.5
                    || pBlcInfo->autoblc_count <= 1) {
                *blc_res->damping = true;
            }
            else {
                pBlcInfo->autoblc_count = 0;
                *blc_res->damping = false;
            }
            pBlcInfo->blc_adjust_tbl[gain_index * 2] = blc1_r;
            pBlcInfo->blc_adjust_tbl[gain_index * 2 + 1] = blc1_b;
        }
        else {
            pBlcInfo->cur_adjust[0] = 0;
            pBlcInfo->cur_adjust[1] = 0;
            pBlcInfo->blc_adjust[0] = 0;
            pBlcInfo->blc_adjust[1] = 0;
        }
        // phwcfg->bls1_val.r = CLIP(pdyn->obcPostTnr.hw_blcT_manualOBR_val * isp_dgain * pre_dgain, 0, 32767);
        // phwcfg->bls1_val.gr = CLIP(pdyn->obcPostTnr.hw_blcT_manualOBGr_val * isp_dgain * pre_dgain, 0, 32767);
        // phwcfg->bls1_val.gb = CLIP(pdyn->obcPostTnr.hw_blcT_manualOBGb_val * isp_dgain * pre_dgain, 0, 32767);
        // phwcfg->bls1_val.b = CLIP(pdyn->obcPostTnr.hw_blcT_manualOBB_val * isp_dgain * pre_dgain, 0, 32767);
#if ISP_HW_V33
        cvtinfo->offset2Blc1 = 0;
        cvtinfo->blc0_diff = 0;
        if (cvtinfo->isFirstFrame || cvtinfo->sw_btnrT_outFrmBase_mode == btnr_curBaseOut_mode) {
            pBlcInfo->pre_ob_offset = autoOB_offset;
            phwcfg->isp_ob_offset = autoOB_offset;
            phwcfg->bls1_val.r = blc1_r;
            phwcfg->bls1_val.b = blc1_b;
        }
        else {
            phwcfg->bls1_val.r = (int)pBlcInfo->pre_adjust[0];
            phwcfg->bls1_val.b = (int)pBlcInfo->pre_adjust[1];
            pBlcInfo->pre_adjust[0] = blc1_r;
            pBlcInfo->pre_adjust[1] = blc1_b;
            if (phwcfg->bls1_val.r != pBlcInfo->pre_adjust[0] || phwcfg->bls1_val.b != pBlcInfo->pre_adjust[1]) {
                *blc_res->damping = true;
            }

            if (psta->debug.obcPostTnr.sw_blcT_autoOBWkArd_en) {
                *blc_res->damping = *blc_res->damping | (pBlcInfo->pre_ob_offset != autoOB_offset);
                phwcfg->isp_ob_offset = autoOB_offset;
                int diff = pBlcInfo->pre_ob_offset - phwcfg->isp_ob_offset;
                phwcfg->bls1_val.r += diff;
                phwcfg->bls1_val.gr += diff;
                phwcfg->bls1_val.gb += diff;
                phwcfg->bls1_val.b += diff;
                cvtinfo->offset2Blc1 = diff;
                pBlcInfo->pre_ob_offset = autoOB_offset;
            }
            else {
                phwcfg->isp_ob_offset = autoOB_offset;
                pBlcInfo->pre_ob_offset = autoOB_offset;
            }
        }
#elif ISP_HW_V35
        if (cvtinfo->isFirstFrame || cvtinfo->sw_btnrT_outFrmBase_mode == btnr_curBaseOut_mode) {
            phwcfg->isp_ob_offset = autoOB_offset;
            phwcfg->isp_ob_offset1 = autoOB_offset;
            phwcfg->bls1_val.r = blc1_r;
            phwcfg->bls1_val.b = blc1_b;
        }
        else {
            phwcfg->bls1_val.r = (int)pBlcInfo->pre_adjust[0];
            phwcfg->bls1_val.b = (int)pBlcInfo->pre_adjust[1];
            pBlcInfo->pre_adjust[0] = blc1_r;
            pBlcInfo->pre_adjust[1] = blc1_b;
            if (phwcfg->bls1_val.r != pBlcInfo->pre_adjust[0] || phwcfg->bls1_val.b != pBlcInfo->pre_adjust[1]) {
                *blc_res->damping = true;
            }
            phwcfg->isp_ob_offset = autoOB_offset;
            phwcfg->isp_ob_offset1 = pBlcInfo->pre_ob_offset;
        }
        pBlcInfo->pre_ob_offset = autoOB_offset;
        if (phwcfg->isp_ob_offset != phwcfg->isp_ob_offset1) {
            *blc_res->damping = true;
        }
#endif
#else
        phwcfg->isp_ob_offset = autoOB_offset;
#endif
    }
    else {
        phwcfg->bls1_en = 0;
        phwcfg->isp_ob_offset = 0;
        pBlcInfo->pre_ob_offset = 0;
#if ISP_HW_V35
        phwcfg->isp_ob_offset1 = 0;
#endif
    }
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
    LOGD_ABLC("frameId %d blc_res->aeIsConverged %d totalGain %f phwcfg->bls1_val.r %d phwcfg->bls1_val.b %d phwcfg->isp_ob_offset %d",
        cvtinfo->frameId, *blc_res->aeIsConverged, totalGain, phwcfg->bls1_val.r, phwcfg->bls1_val.b, phwcfg->isp_ob_offset);
#endif

    if (cvtinfo->frameNum > 1) {
        if (phwcfg->bls1_en == 1 || phwcfg->isp_ob_offset > 0) {
            if (cvtinfo->blc_warning_count < 5) {
                LOGE_ABLC("When using HDR mode, obcPostTnr and ob_offset should be off to avoid data overflow.\n"
                          "So bls1_en, isp_ob_offset and isp_ob_predgain will be forcibly set to 0 in HWI.");
            }
            else if (cvtinfo->blc_warning_count % 300 == 0) {
                LOGE_ABLC("When using HDR mode, obcPostTnr and ob_offset should be off to avoid data overflow.\n"
                          "So bls1_en, isp_ob_offset and isp_ob_predgain will be forcibly set to 0 in HWI.");
            }
            cvtinfo->blc_warning_count++;
        }
        // hdr won't use blc1 and blc_ob
        phwcfg->bls1_en = 0;
        phwcfg->isp_ob_offset = 0;
        phwcfg->isp_ob_predgain = 0;
        pBlcInfo->pre_ob_offset = 0;
#if ISP_HW_V35
        phwcfg->isp_ob_offset1 = 0;
#endif
    }

#if ISP_HW_V35
    cvtinfo->isp_ob_offset = phwcfg->isp_ob_offset;
    cvtinfo->isp_ob_offset1 = phwcfg->isp_ob_offset1;
#endif

    if (phwcfg->isp_ob_predgain != 0 && fabs(phwcfg->isp_ob_predgain - (float)0x100) > FLT_EPSILON) {
        phwcfg->bls1_en = 1;
#if ISP_HW_V35
        phwcfg->isp_ob_max = CLIP((int)(4096 * cvtinfo->preDGain_drc) - phwcfg->isp_ob_offset1, 0, 1048575);
#else
        phwcfg->isp_ob_max = CLIP((int)(4096 * cvtinfo->preDGain_drc) - phwcfg->isp_ob_offset, 0, 1048575);
#endif
    }
    else {
        if (cvtinfo->frameNum == 1)
            phwcfg->isp_ob_max = 0xfff;
        else
            phwcfg->isp_ob_max = 0xfffff;
    }
    return;
}
